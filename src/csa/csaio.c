
#include <string.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"

#include "timer.h"
#include "video.h"
#include "keys.h"
#include "io.h"

#include "petio.h"
#include "pia.h"
#include "via.h"
#include "parallel.h"
#include "piavia.h"
#include "csamem.h"
#include "mem.h"
#include "petvideo.h"


static uchar ctrl_port = 0;
static uchar ctrl_irq = 0;

static void bios_ctrl_cb(struct alarm_s *alarm, CLOCK current);


static alarm_t bios_ctrl_irq = {
        "50Hz IRQ",
        NULL,
        bios_ctrl_cb,
        NULL,
        CLOCK_MAX
};

static void set_ctrl_irq(uchar fl) {

	if (fl) {
		// set
		if ((ctrl_irq == 0)
			&& (ctrl_port & 0x02)) {
			// not yet set, and clkirqen is set
			cpu_set_irq(CTRL_IRQ_MASK, 1);
		}
	} else {
		// clr
		cpu_set_irq(CTRL_IRQ_MASK, 0);
	}

	ctrl_irq = fl;
}

static void ctrl_wr(scnt val) {

	set_ctrl_irq(0);
	
	ctrl_port = val;
}

static scnt ctrl_rd() {

	return (ctrl_port & 0x3e) | 0x40 | (ctrl_irq ? 0x80 : 0) | (cpu_is_irq() ? 0 : 0x01);
}

static void bios_ctrl_cb(struct alarm_s *alarm, CLOCK current) {

	set_ctrl_irq(1);

	// toggle 50hz
        set_alarm_clock_plus(&bios_ctrl_irq, 50000);
}

int io_init(BUS *bus) {

        alarm_register(&bus->actx, &bios_ctrl_irq);
        set_alarm_clock(&bios_ctrl_irq, 0);

	return piavia_init(bus);
}

void io_wr(scnt adr, scnt val) {

	//logout(0, "io_wr %02x to %04x", val, adr);

	scnt a = (adr & 0x7f0);
	switch(a) {
	case 0x10:
		pia_wr(&pia1, adr, val);
		break;
	case 0x20:
		pia_wr(&pia2, adr, val);
		break;
	case 0x40: 
		via_wr(&via, adr, val);
		break;
	case 0x80:
		crtc_wr(adr, val);
		break;
	case 0x7e0:
		ctrl_wr(val);
		break;
	case 0x7f0:
		mmu_wr(adr, val);
		break;
	}
}

scnt io_rd(scnt adr) {

	//logout(0, "io_rd from %04x", adr);

	scnt a = (adr & 0x7f0);
	switch(a) {
	case 0x10:
		return pia_rd(&pia1, adr);
	case 0x20:
		return pia_rd(&pia2, adr);
	case 0x40:
		return via_rd(&via, adr);
	case 0x80:
		return crtc_rd(adr);
	case 0x7e0:
		return ctrl_rd();
	case 0x7f0:
		return mmu_rd(adr);
	default:
		return adr >> 8;
	}
}

// PET I/O chips don't change state on read
scnt io_peek(scnt addr) {
	return io_rd(addr);
}

