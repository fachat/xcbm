
#include <string.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "emu6502.h"

#include "timer.h"
#include "video.h"
#include "keys.h"
#include "io.h"

#include "pia.h"
#include "via.h"


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

PIA pia1;

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

void io_set_vdrive(uchar flag) {
	pia_ca1(&pia1, flag ? PIA_CX1_HIGH : PIA_CX1_LOW);
}

// PIA2
//
// port A:
//	0-7: IEEE data in
//
// CA1: ATN in
// CA2: NDAC out
//
// port B:
//	0-7: IEEE data out
//
// CB1: SRQ in
// CB2: DAV out
//

PIA pia2;

static uchar pia2_get_porta(uchar origdata) {
	return 0xff;
}

static void pia2_set_ca2(uchar flag) {
}

static void pia2_set_portb(uchar data, uchar dir) {
}

static void pia2_set_cb2(uchar flag) {
}


// VIA
//
// port A:
// 	0-7: userport output
//
// CA1: userport input
// CA2: graphics mode selector
//
// port B:
//	0: NDAC out
//	1: NRFD out
//	2: ATN out
//	3: Cassette write (#1 + #2 shared)
//	4: Cassette #2 motor
//	5: vertical drive input
//	6: NRFD in
//	7: DAV in
//
// CB1: Cassette #2 read
// CB2: userport out / shift register / sound
//

VIA via;

static uchar via_get_portb(uchar origdata) {
	return 0xff;
}

static void via_set_portb(uchar data, uchar dir) {
}


//------------------------------------------------------

int io_init(void) {

	// PIA1
	pia_init(&pia1);

	pia1.get_port_a_in = pia1_get_porta;
	pia1.set_port_a_out = pia1_set_porta;
	pia1.set_ca2_out = pia1_set_ca2;

	pia1.get_port_b_in = pia1_get_portb;

	// PIA2
	pia_init(&pia2);

	// IEEE data
	pia2.get_port_a_in = pia2_get_porta;
	pia2.set_port_b_out = pia2_set_portb;
	pia2.set_ca2_out = pia2_set_ca2;
	pia2.set_cb2_out = pia2_set_cb2;

	// VIA
	via_init(&via);

	via.get_port_b_in = via_get_portb;
	via.set_port_b_out = via_set_portb;

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
		via_wr(&via, a, val);
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
		return via_rd(&via, adr);
	case 8:
		// todo CRTC
	default:
		return adr >> 8;
	}
}


