
#include <string.h>

#include "types.h"
#include "emu6502.h"

#include "timer.h"
#include "video.h"
#include "keys.h"
#include "io.h"

#include "pia.h"

PIA pia1;
PIA pia2;

// PIA1
//
// port A:
// 	0-3: keyboard row select out
// 	4: cassette #1 switch in
// 	5: cassette #2 switch in
// 	6: EOI in
// 	7: DIAG in
//
// CA1:	Cassette #1 read in
// CA2: EOI out
//
// port B:
// 	0-7: keyboard col input
//
// CB1: vertical drive in (interrupt generator)
// CB2: cassette #1 motor out
//

static uchar pia1_get_porta(uchar origdata) {
	return 0xff;
}

static void pia1_set_porta(uchar data, uchar dir) {
}

static void pia1_set_ca2(uchar flag) {
}

static uchar pia1_get_portb(uchar origdata) {
	return 0xff;
}


int io_init(void) {

	pia_init(&pia1);
	pia_init(&pia2);

	pia1.get_port_a_in = pia1_get_porta;
	pia1.set_port_a_out = pia1_set_porta;
	pia1.set_ca2_out = pia1_set_ca2;

	pia1.get_port_b_in = pia1_get_portb;

#if 0
	key_init(0);
#endif
	return(0);
}

void io_wr(scnt adr, scnt val) {

	register uchar a = (adr & 0xf0);
	switch(a) {
	case 1:
		pia_wr(&pia1, a, val);
		break;
	case 2:
		pia_wr(&pia2, a, val);
		break;
	case 4: 
		// todo VIA
		break;
	case 8:
		// todo CRTC
		break;
	}
}

scnt io_rd(scnt adr) {

	register uchar a = (adr & 0xf0);
	switch(a) {
	case 1:
		return pia_rd(&pia1, adr);
	case 2:
		return pia_rd(&pia2, adr);
	case 4:
		// todo VIA
	case 8:
		// todo CRTC
	default:
		return adr >> 8;
	}
}


