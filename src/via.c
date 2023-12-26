
#include <string.h>

#include "types.h"
#include "log.h"
#include "alarm.h"
#include "bus.h"
#include "via.h"
#include "cpu.h"


static void t1alarm_cb(alarm_t *, CLOCK);

/* timer values do not depend on a certain value here, but PB7 does... */
#define TAUOFFSET       (-1)

// copied from VICE viacore.c

inline static void update_int(VIA *via_context, CLOCK rclk)
{
	// VICE update_myviairq_rclk()
#if 0
	(via->set_int)(via_context, via_context->int_num,
		(via_context->ifr & via_context->ier & 0x7f)
		? via_context->irq_line : 0, rclk);
#endif
}

#define	FULL_CYCLE_2	2
/*
 * Get the current Timer 1 value.
 *
 * The datasheet claims:
 * Only in continuous mode, T1 reloads from the latch after it shows underflow (FFFF).
 * A full cycle of T1 therefore takes (tal + 2) cycles: N, N-1, ..., 1, , 0, FFFF.
 * FULL_CYCLE_2 is the name of this 2, to make it clear where it occurs.
 *      
 * In practice it seems that this happens also in one-shot mode.
 *       
 * tau is used to indicate when T1 gets/was reloaded from the latch.
 */
inline static CLOCK viata(VIA *via_context, CLOCK rclk)
{
	// VICE viacore_t1()

    if (rclk < via_context->tau - TAUOFFSET) {
        return via_context->tau - TAUOFFSET - rclk - FULL_CYCLE_2;
    } else {
	unsigned int full_cycle = via_context->tal + FULL_CYCLE_2;
	CLOCK time_past_last_reload = rclk - via_context->tau;
	unsigned int partial_cycle = time_past_last_reload % full_cycle;

        return (via_context->tal - partial_cycle);
    }
}

inline static CLOCK viatb(VIA *via_context, CLOCK rclk)
{
    CLOCK t2;

    if (via_context->acr & 0x20) {
        t2 = (via_context->t2ch << 8) | via_context->t2cl;
    } else {
        t2 = via_context->tbu - rclk - 2;

        if (via_context->tbi) {
            uint8_t t2hi = via_context->t2ch;

            if (rclk == via_context->tbi + 1) {
                t2hi--;
            }

            t2 = (t2hi << 8) | (t2 & 0xff);
        }
    }

    return t2;
}

inline static void update_viatal(VIA *via_context, CLOCK rclk)
{
	// VICE update_via_t1_latch()

    if (rclk >= via_context->tau) {
	unsigned int full_cycle = via_context->tal + FULL_CYCLE_2;
	CLOCK time_past_last_reload = rclk - via_context->tau;
	/*
	 * Calculate the number of full T1 cycles, rounded way up
	 * to get tau ahead of rclk, but at most by 1 full_cycle
	 */

        CLOCK nuf = 1 + (time_past_last_reload / full_cycle);
	via_context->tau += nuf * full_cycle;
    }

    via_context->tal = via_context->t1ll
                       + (via_context->t1lh << 8);
}

/*
 * This alarm takes care of generating the T1 IRQ: VIA_IM_T1.
 * It also toggles PB7.
 * It runs after the T1 reads 0000.
 */
static void t1alarm_cb(alarm_t *alarm, CLOCK rclk)
{
	VIA *via_context = (VIA *)alarm->data;

logout(0, "t1alarm_cb");

	if (!(via_context->acr & 0x40)) {     /* one-shot mode */
		clr_alarm_clock(alarm);
		via_context->tai = 0;
	} else {                    /* continuous mode */
		unsigned int full_cycle = via_context->tal + FULL_CYCLE_2;
		/* load counter with latch value */
		via_context->tai += full_cycle;
		set_alarm_clock(alarm, via_context->tai);

		/* Let tau also keep up with the cpu clock
	  	   this should avoid "% (via_context->tal + 2)" case */
		via_context->tau += full_cycle;
	}

	via_context->ifr |= VIA_IM_T1;
	update_int(via_context, rclk + 1);

	via_context->t1pb7 ^= 0x80;
}


// read/write to VIA registers by the CPU
void via_wr(VIA *via, scnt addr, uchar val) {

	CLOCK rclk = via->bus->actx.clk;

	uchar reg = addr & 0x0f;

	logout(0, "% 6d - %04x: %s write %02x to reg %02x", rclk, cpu_pc(via->bus->cpu), via->name, val, reg);

	switch(reg) {
	case VIA_PRB:
		via->data_b = val;
		if (via->set_port_b_out) {
			via->set_port_b_out(via->data_b, via->ddir_b);
		}
		break;
	case VIA_PRA:
	case VIA_PRA_NHS:	// TODO: handshake
		via->data_a = val;
		if (via->set_port_a_out) {
			via->set_port_a_out(via->data_a, via->ddir_a);
		}
		break;
	case VIA_DDRB:
		via->ddir_b = val;
		break;
	case VIA_DDRA:
		via->ddir_a = val;
		break;
		// TODO timer stuff
	case VIA_T1CL:
	case VIA_T1LL:
		via->t1ll = val;
		update_viatal(via, rclk);
		break;
	case VIA_T1CH:
		via->t1lh = val;
		update_viatal(via, rclk);
		// load counter with latch value
		via->tau = rclk + via->tal + 3 + TAUOFFSET;
		via->tai = rclk + via->tal + 1 + TAUOFFSET;
		// set alarm at via->tai
	logout(1, "set T1 alarm to %d", via->tai);
		set_alarm_clock(&via->t1alarm, via->tai);
		// set PB7 state
		via->t1pb7 = 0;
		// clear T1 interrupt
		via->ifr &= ~VIA_IM_T1;
		update_int(via, rclk);
		break;
	case VIA_T1LH:
		via->t1lh = val;
		update_viatal(via, rclk);
		// update interrupt flag (Note Synertek datasheet seems wrong here)
		via->ifr &= VIA_IM_T1;
		update_int(via, rclk);
		break;
	case VIA_T2LL:	// T2CL is read only
		via->t2ll = val;
		break;
	case VIA_T2LH:	// T2CH is read only
		via->t2lh = val;
		via->t2cl = via->t2ll;
		via->t2ch = via->t2lh;

		/* start T2 only in timer mode, leave unchanged in pulse counting mode */
		if (!(via->acr & 0x20)) {
			/* set the next alarm to the low latch value as timer cascading mode change 
			matters at each underflow of the T2 low counter */
			via->tbu = rclk + via->t2cl + 3;
			via->tbi = rclk + via->t2cl + 1;
			//alarm_set(via_context->t2_alarm, via_context->tbi);
		}

		/* Clear T2 interrupt */
		via->ifr &= ~VIA_IM_T2;
		update_int(via, rclk);
		break;
	case VIA_SR:
		via->sr = val;
		break;
	case VIA_ACR: 
		via->acr = val;
		break;
	case VIA_PCR:
		via->pcr = val;
		break;
	case VIA_IFR:
		via->ifr = val;
		break;
	case VIA_IER:
		if (val & VIA_IM_IRQ) {
			/* set interrupts */
			via->ier |= val & 0x7f;
		} else {
			/* clear interrupts */
			via->ier &= ~val;
		}
		break;	
	}
}

uchar via_rd(VIA *via, scnt addr) {

	register uchar rv = 0xff;	// default if get function not set

	uchar reg = addr & 0x0f;
	
	CLOCK rclk = via->bus->actx.clk;


	switch(reg) {
	case VIA_PRB:
		if (via->get_port_b_in) {
			rv = via->get_port_b_in(via->data_b);
		}
		rv = (via->ddir_b & via->data_b) | ((via->ddir_b ^ 0xff) & rv);
		break;
	case VIA_PRA:
	case VIA_PRA_NHS:	// TODO no handshake
		if (via->get_port_a_in) {
			rv = via->get_port_a_in(via->data_a);
		}
		rv = (via->ddir_a & via->data_a) | ((via->ddir_a ^ 0xff) & rv);
		break;
	case VIA_DDRB:
		rv = via->ddir_b;	
		break;
	case VIA_DDRA:
		rv = via->ddir_a;
		break;
	case VIA_T1CL:
		via->ifr &= ~VIA_IM_T1;
		update_int(via, rclk);
		rv = (uchar) (viata(via, rclk) & 0xff);
		break;
	case VIA_T1CH:
		rv = (uchar) ((viata(via, rclk) >> 8) & 0xff);
		break;
	case VIA_T1LL: 
		rv = via->t1ll;
		break;
	case VIA_T1LH:
		rv = via->t1lh;
		break;
	case VIA_T2CL:
		via->ifr &= ~VIA_IM_T2;
		update_int(via, rclk);
		rv = (uchar) (viatb(via, rclk) & 0xff);
		break;
	case VIA_T2CH:
		rv = (uchar) ((viatb(via, rclk) >> 8) & 0xff);
		break;
	case VIA_SR:
		rv = via->sr;
		break;
	case VIA_ACR:
		rv = via->acr;
		break;
	case VIA_PCR:
		rv = via->pcr;
		break;
	case VIA_IFR:
		rv = via->ifr;
		if (via->ifr & via->ier) {
			rv |= 0x80;
		}
		break;
	case VIA_IER:
		rv = via->ier | 0x80;
		break;
	}

	logout(0, "%04x: %s read from reg %02x as %02x", cpu_pc(via->bus->cpu), via->name, reg, rv);
	return rv;
}

// extern trigger to set CA1 / CA2 input state
void via_ca1(VIA *via, uchar flag);

void via_ca2(VIA *via, uchar flag);

void via_cb1(VIA *via, uchar flag);

void via_cb2(VIA *via, uchar flag);


void via_init(VIA *via, BUS *bus, const char *name) {

	memset(via, 0, sizeof(VIA));

	via->bus = bus;
	via->name = name;

	via->tal = 0xffff;
	via->t2cl = 0xff;
	via->t2ch = 0xff;

	alarm_init(&via->t1alarm, "VIA T1", &bus->actx, t1alarm_cb, via);
	alarm_init(&via->t2alarm, "VIA T2", &bus->actx, NULL, via);
	alarm_init(&via->sralarm, "VIA SR", &bus->actx, NULL, via);

	update_int(via, bus->actx.clk);
}

