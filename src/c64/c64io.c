
#include <string.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"

#include "emu64.h"
#include "timer.h"
#include "video.h"
#include "keys.h"
#include "sound.h"
#include "mem.h"
#include "c64io.h"
#include "vicii.h"

typedef struct {
		byte 	pra;
		byte	prb;
		byte	ddra;
		byte	ddrb;
		byte	talo;
		byte	tahi;
		byte	tblo;
		byte	tbhi;
		byte	tod_tenth;
		byte	tod_sec;
		byte	tod_min;
		byte	tod_hr;
		byte	sdr;
		byte	icr;
		byte 	cra;
		byte	crb;

		byte	pra_w;
		byte	prb_w;
		byte	talo_w;
		byte	tahi_w;
		byte	tblo_w;
		byte	tbhi_w;
		byte	imr;
		byte	al_tenth;
		byte	al_sec;
		byte	al_min;
		byte	al_hr;
		int	timera;
		int 	timerb;
} CIA;

CIA	cia1, cia2;

void cia1_tia(int); 
void cia1_tib(int);
void cia2_tia(int);
void cia2_tib(int);

void cia1_wr(scnt,scnt);
void cia2_wr(scnt,scnt);
scnt cia1_rd(scnt);
scnt cia2_rd(scnt);
scnt cia1_peek(scnt);
scnt cia2_peek(scnt);

int io_init(void) {
	iec_init();

	memset(&cia1,0,sizeof(CIA));
	memset(&cia2,0,sizeof(CIA));
	cia1.timera=time_register(cia1_tia,"cia1 timera");
	cia1.timerb=time_register(cia1_tib,"cia1 timerb");
	cia2.timera=time_register(cia2_tia,"cia2 timera");
	cia2.timerb=time_register(cia2_tib,"cia2 timerb");

	return(0);
}

void io_wr(meminfo_t *inf, scnt adr, scnt val) {
	register scnt a=(adr&0x0c00);
	switch(a) {
	case 0:
		video_wr(adr&0x3f,val);
		break;
	case 0x400:
		sid_wr(adr&0x1f,val);
		break;
	case 0x800:
		colram_wr(adr&0x3ff,val);
		break;
	case 0xc00:	
		a=adr&0x300;
		switch(a) {
		case 0:
			cia1_wr(adr&0x0f,val);
			break;
		case 0x100:
			cia2_wr(adr&0x0f,val);
			break;
		}
		break;
	}
}

scnt io_rd(meminfo_t *inf, scnt adr) {
        register scnt a=(adr&0x0c00);
        switch(a) {
        case 0:
                return(video_rd(adr&0x3f));
                break;
        case 0x400:
                return(sid_rd(adr&0x1f));
                break;
        case 0x800:
                return(colram_rd(adr&0x3ff));
                break;
        case 0xc00:
                a=adr&0x300;
                switch(a) {
                case 0:
                        return(cia1_rd(adr&0x0f));
                        break;
                case 0x100:
                        return(cia2_rd(adr&0x0f));
                        break;
                default:
                        return((adr&0xff00)>>8);
                }
 	}
	return(0);
}

scnt io_peek(meminfo_t *inf, scnt adr) {
        register scnt a=(adr&0x0c00);
        switch(a) {
        case 0:
		// VIC-II does not auto-clear interrupts on read
                return(video_rd(adr&0x3f));
                break;
        case 0x400:
                return(sid_rd(adr&0x1f));
                break;
        case 0x800:
                return(colram_rd(adr&0x3ff));
                break;
        case 0xc00:
                a=adr&0x300;
                switch(a) {
                case 0:
                        return(cia1_peek(adr&0x0f));
                        break;
                case 0x100:
                        return(cia2_peek(adr&0x0f));
                        break;
                default:
                        return((adr&0xff00)>>8);
                }
 	}
	return(0);
}

/****************************************************************************/


#define	cia1_setirq()	\
 if(!(cia1.icr&0x80)) {if(cia1.icr&cia1.imr&0x7f){cia1.icr|=0x80;cpu_set_irq(IRQ_CIA1, 1);}}
#define	cia2_setirq()	\
 if(!(cia2.icr&0x80)) {if(cia2.icr&cia2.imr&0x7f){cia2.icr|=0x80;cpu_set_nmi(IRQ_CIA2, 1);}}

void cia1_tia(int endval) {
	cia1.talo = cia1.talo_w;
	cia1.tahi = cia1.tahi_w;
	if(cia1.cra&8) { /* single shot */
		cia1.cra &= 0xfe;	/* timer a stop */
		time_reset(cia1.timera);
	} else {	/* continous mode */
		time_setval(cia1.timera, cia1.talo_w+256*cia1.tahi_w);
	}
	cia1.icr |= 1;
	cia1_setirq();
/*printf("\ncia1_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia1.icr, cia1.imr,hirq);*/
}

void cia1_tib(int endval) {
        cia1.tblo = cia1.tblo_w;
        cia1.tbhi = cia1.tbhi_w;
        if(cia1.crb&8) { /* single shot */
                cia1.crb &= 0xfe;       /* timer a stop */
                time_reset(cia1.timerb);
        } else {        /* continous mode */
                time_setval(cia1.timerb, cia1.tblo_w+256*cia1.tbhi_w+endval);
        }
        cia1.icr |= 2;
        cia1_setirq();
/*printf("\ncia1_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia1.icr, cia1.imr,hirq);*/
}

void cia2_tia(int endval) {
        cia2.talo = cia2.talo_w;
        cia2.tahi = cia2.tahi_w;
        if(cia2.cra&8) { /* single shot */
                cia2.cra &= 0xfe;       /* timer a stop */
                time_reset(cia2.timera);
        } else {        /* continous mode */
                time_setval(cia2.timera, cia2.talo_w+256*cia2.tahi_w+endval);
        }
        cia2.icr |= 1;
        cia2_setirq();
/*printf("\ncia2_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia2.icr, cia2.imr,hnmi);*/
}

void cia2_tib(int endval) {
        cia2.tblo = cia2.tblo_w;
        cia2.tbhi = cia2.tbhi_w;
        if(cia2.crb&8) { /* single shot */
                cia2.crb &= 0xfe;       /* timer a stop */
                time_reset(cia2.timerb);
        } else {        /* continous mode */
                time_setval(cia2.timerb, cia2.tblo_w+256*cia2.tbhi_w+endval);
        }
        cia2.icr |= 2;
        cia2_setirq();
/*printf("\ncia2_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia2.icr, cia2.imr,hnmi);*/
}

void cia1_wr(scnt reg, scnt val) {
	switch(reg) {
	case 0:
		cia1.pra = val;
		break;
	case 1:
		cia1.prb = val;
		break;
	case 2:
		cia1.ddra = val;
		break;
	case 3:
		cia1.ddrb = val;
		break;
	case 4:
		cia1.talo_w = val;
		break;
	case 5:
		cia1.tahi_w = val;
		break;
	case 6:
		cia1.tblo_w = val;
		break;
	case 7:
		cia1.tbhi_w = val;
		break;
	case 13:
		if(val&0x80) { 	/* set mask bits */
			cia1.imr |= (val&0x7f);
		} else {
			cia1.imr &= (val^0x7f);
		}
		break;
	case 14:
		if(val&1) { 
			time_seton(cia1.timera);
			if(!(cia1.cra&1))
				time_setval(cia1.timera,
					cia1.talo_w+256*cia1.tahi_w);
		}
		cia1.cra = val;
		break;
	case 15:
                if(val&1) {
                        time_seton(cia1.timerb);
                        if(!(cia1.crb&1))
                                time_setval(cia1.timerb,
					cia1.tblo_w+256*cia1.tbhi_w);
                }
		cia1.crb = val;
		break;
	}
}

void cia2_wr(scnt reg, scnt val) {
        switch(reg) {
        case 0:
		if((val^cia2.pra)&0x3) 
			setvideopage((val&0x3)^3);
                cia2.pra = val;
                break;
        case 1:
                cia2.prb = val;
                break;
        case 2:
                cia2.ddra = val;
                break;
        case 3:
                cia2.ddrb = val;
                break;
        case 4:
                cia2.talo_w = val;
                break;
        case 5:
                cia2.tahi_w = val;
                break;
        case 6:
                cia2.tblo_w = val;
                break;
        case 7:
                cia2.tbhi_w = val;
                break;
        case 13:
                if(val&0x80) {  /* set mask bits */
                        cia2.imr |= (val&0x7f);
                } else {
                        cia2.imr &= (val^0x7f);
                }
                break;
        case 14:
                if(val&1) {
                        time_seton(cia2.timera);
                        if(!(cia2.cra&1))
                                time_setval(cia2.timera,
					cia2.talo_w+256*cia2.tahi_w);
                }
                cia2.cra = val;
                break;
        case 15:
                if(val&1) {
                        time_seton(cia2.timerb);
                        if(!(cia2.crb&1))
                                time_setval(cia2.timerb, 
					cia2.tblo_w+256*cia2.tbhi_w);
                }
                cia2.crb = val;
                break;
        }
} 

 
static scnt cia1_rd_int(scnt reg, int ispeek) {
	scnt i; 
	switch(reg) {
	case 0:
		return(cia1.pra);
	case 1:
		return(prb[cia1.pra]);
	case 2:
		return(cia1.ddra);
	case 3:
		return(cia1.ddrb);
	case 4:
		return(time_get(cia1.timera)&0xff);
	case 5:
		return((time_get(cia1.timera)&0xff00)>>8);
	case 6:
		return(time_get(cia1.timerb)&0xff);
	case 7:
		return((time_get(cia1.timerb)&0xff00)>>8);
	case 13:
		if(cia1.icr&0x80) {
			cpu_set_irq(IRQ_CIA1, 0);
		};
		i=cia1.icr;
		if (!ispeek) {
			cia1.icr=0;
		}
		return(i);
	case 14:
		return(cia1.cra);
	case 15:
		return(cia1.crb);
	}
	return(0);
}	

scnt cia1_rd(scnt reg) {
	return cia1_rd_int(reg, 0);
}

scnt cia1_peek(scnt reg) {
	return cia1_rd_int(reg, 1);
}

static scnt cia2_rd_int(scnt reg, int ispeek) {
        scnt i;
        switch(reg) {
        case 0:
                return(cia2.pra);
        case 1:
                return(cia2.prb);
        case 2:
                return(cia2.ddra);
        case 3:
                return(cia2.ddrb);
        case 4:
                return(time_get(cia2.timera)&0xff);
        case 5:
                return((time_get(cia2.timera)&0xff00)>>8);
        case 6:
                return(time_get(cia2.timerb)&0xff);
        case 7:
                return((time_get(cia2.timerb)&0xff00)>>8);
        case 13:
                if(cia2.icr&0x80) {
			cpu_set_nmi(IRQ_CIA2, 0);
		}
                i=cia2.icr;
		if (!ispeek) {
                	cia2.icr=0;
		}
                return(i);
        case 14:
                return(cia2.cra);
        case 15:
                return(cia2.crb);
        }
        return(0);
}

scnt cia2_rd(scnt reg) {
	return cia2_rd_int(reg, 0);
}

scnt cia2_peek(scnt reg) {
	return cia2_rd_int(reg, 1);
}

 
