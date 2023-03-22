
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



int io_init(BUS *bus) {

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
		// todo CRTC
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
		// todo CRTC
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

