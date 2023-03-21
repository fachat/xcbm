
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

static uchar key_row = 0;

static uchar pia1_get_porta(uchar origdata) {
	uchar rv = key_row;

	rv |= 0x30;	// unused cassette inputs

	if (!parallel_get_eoi()) {
		rv |= 0x40;
	}

	rv |= 0x80;	// unused DIAG inputs

	return rv; 
}

static void pia1_set_porta(uchar data, uchar dir) {

	// all input pins read as high
	key_row = (data | ~dir) & 0x0f;
}

static void pia1_set_ca2(uchar flag) {
	parallel_cpu_set_eoi(!flag);
}

static uchar pia1_get_portb(uchar origdata) {

	uchar rv = key_read_cols(key_row);
	
	logout(0, "PIA1 read port B on row=%d as %02x", key_row, rv);

	return rv;
}

void io_set_vdrive(uchar flag) {
	pia_cb1(&pia1, flag ? PIA_CX1_HIGH : PIA_CX1_LOW);
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
	return parallel_get_bus();
}

static void pia2_set_ca2(uchar flag) {
	parallel_cpu_set_ndac(!flag);
}

static void pia2_set_portb(uchar data, uchar dir) {
	parallel_cpu_set_bus(data | ~dir);
}

static void pia2_set_cb2(uchar flag) {
	parallel_cpu_set_dav(!flag);
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
//	0: NDAC in
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

uchar portb;

static uchar via_get_portb(uchar outdata) {

	uchar rv = outdata & 0x3e;

	if (!parallel_get_ndac()) {
		rv |= 0x01;
	}
	if (!parallel_get_nrfd()) {
		rv |= 0x40;
	}
	if (!parallel_get_dav()) {
		rv |= 0x80;
	}
	return rv;
}

static void via_set_portb(uchar data, uchar dir) {

	data |= ~dir;

	parallel_cpu_set_nrfd(!(data & 0x02));
	parallel_cpu_set_atn(!(data & 0x04));
}


//------------------------------------------------------

int piavia_init(BUS *bus) {

	parallel_init();

	// ----------
	// PIA1
	pia_init(&pia1, bus, "PIA1");

	pia1.get_port_a_in = pia1_get_porta;
	pia1.set_port_a_out = pia1_set_porta;
	pia1.set_ca2_out = pia1_set_ca2;

	pia1.get_port_b_in = pia1_get_portb;
	
	// interrupts
	pia1.set_interrupt = cpu_set_irq;
	pia1.int_num = PIA1_INT_MASK;

	// ----------
	// PIA2
	pia_init(&pia2, bus, "PIA2");

	// IEEE data
	pia2.get_port_a_in = pia2_get_porta;
	pia2.set_port_b_out = pia2_set_portb;
	pia2.set_ca2_out = pia2_set_ca2;
	pia2.set_cb2_out = pia2_set_cb2;

	// interrupts
	pia2.set_interrupt = cpu_set_irq;
	pia2.int_num = PIA2_INT_MASK;

	// ----------
	// VIA
	via_init(&via, bus, "VIA");

	via.get_port_b_in = via_get_portb;
	via.set_port_b_out = via_set_portb;

	// interrupts
	via.set_interrupt = cpu_set_irq;
	via.int_num = VIA_INT_MASK;

	return(0);
}



