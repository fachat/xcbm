
#include <string.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "cpu.h"

#include "timer.h"
#include "video.h"
#include "keys.h"
#include "io.h"

#include "petio.h"
#include "pia.h"
#include "via.h"
#include "parallel.h"
#include "piavia.h"

#include "ctrl.h"
#include "spi.h"



int io_init(BUS *bus) {

	return piavia_init(bus);
}

void io_wr(scnt addr, scnt val) {

	//logout(0, "io_wr %02x to %04x", val, addr);

	register uchar a = (addr & 0xf0);
	switch(a) {
	case 0x00:
		//logout(0, "io_wr(0) %02x to %04x", val, addr);

		if ((addr & 0x08) == 0) {
			logout(0, "to ctrl_wr(0) %02x to %04x", val, addr);
			// e800-e807
			ctrl_wr(addr, val);
		} else {
			// e808-e80f
			spi_wr(addr, val);
		}
		break;
	case 0x10:
		pia_wr(&pia1, addr, val);
		break;
	case 0x20:
		pia_wr(&pia2, addr, val);
		break;
	case 0x30:
		//dac_wr(&dac, addr, val);
		break;
	case 0x40: 
		via_wr(&via, addr, val);
		break;
	case 0x80:
		// todo CRTC
		break;
	default:
		break;
	}
}

scnt io_rd(scnt addr) {

	//logout(0, "io_rd from %04x", addr);

	register uchar a = (addr & 0xf0);
	switch(a) {
        case 0x00:
                if (addr & 0x08 == 0) {
                        // e800-e807
                        return ctrl_rd(addr);
                } else {
                        // e808-e80f
                        return spi_rd(addr);
                }
                break;
	case 0x10:
		return pia_rd(&pia1, addr);
	case 0x20:
		return pia_rd(&pia2, addr);
	case 0x40:
		return via_rd(&via, addr);
	case 0x80:
		// todo CRTC
		break;
	default:
		break;
	}
	return addr >> 8;
}

// note: PET I/O chips do not change state on read
scnt io_peek(scnt addr) {

	register uchar a = (addr & 0xf0);
	switch(a) {
	case 0x10:
	case 0x20:
	case 0x40:
		return io_rd(addr);
	}
	return 0;
}


