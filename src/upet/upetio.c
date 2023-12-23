
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

#include "ctrl.h"



int io_init(BUS *bus) {

	return piavia_init(bus);
}

void io_wr(scnt adr, scnt val) {

	//logout(0, "io_wr %02x to %04x", val, adr);

	register uchar a = (adr & 0xf0);
	switch(a) {
	case 0x00:
		if (adr & 0x08 == 0) {
			// e800-e807
			ctrl_wr(adr, val);
		} else {
			// e808-e80f
			//spi_wr(adr, val);
		}
		break;
	case 0x10:
		pia_wr(&pia1, adr, val);
		break;
	case 0x20:
		pia_wr(&pia2, adr, val);
		break;
	case 0x30:
		//dac_wr(&dac, adr, val);
		break;
	case 0x40: 
		via_wr(&via, adr, val);
		break;
	case 0x80:
		// todo CRTC
		break;
	}
}

scnt io_rd(scnt adr) {

	//logout(0, "io_rd from %04x", adr);

	register uchar a = (adr & 0xf0);
	switch(a) {
        case 0x00:
                if (adr & 0x08 == 0) {
                        // e800-e807
                        return ctrl_rd(adr);
                } else {
                        // e808-e80f
                        //spi_wr(adr, val);
                }
                break;
	case 0x10:
		return pia_rd(&pia1, adr);
	case 0x20:
		return pia_rd(&pia2, adr);
	case 0x40:
		return via_rd(&via, adr);
	case 0x80:
		// todo CRTC
		break;
	default:
		break;
	}
	return adr >> 8;
}

// note: PET I/O chips do not change state on read
scnt io_peek(scnt addr) {
	return io_rd(addr);
}


