
#include <string.h>

#include "types.h"
#include "via.h"


// read/write to VIA registers by the CPU
void via_wr(VIA *via, uchar reg, uchar val) {

	switch(reg & 0x0f) {
	case 0:
		via->data_b = val;
		if (via->set_port_b_out) {
			via->set_port_b_out(via->data_b, via->ddir_b);
		}
		break;
	case 1:
	case 15:	// TODO: no handshake
		via->data_a = val;
		if (via->set_port_a_out) {
			via->set_port_a_out(via->data_a, via->ddir_a);
		}
		break;
	case 2:
		via->ddir_b = val;
		break;
	case 3:
		via->ddir_a = val;
		break;
		// TODO timer stuff
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		break;
	case 10:
		via->sr = val;
		break;
	case 11: 
		via->acr = val;
		break;
	case 12:
		via->pcr = val;
		break;
	case 13:
		via->ifr = val;
		break;
	case 14:
		via->ier = val;
		break;	
	}
}

uchar via_rd(VIA *via, uchar reg) {

	register uchar rv = 0xff;	// default if get function not set
	
	switch(reg & 0x03) {
	case 0:
		if (via->get_port_b_in) {
			rv = via->get_port_b_in(via->data_b);
		}
		rv = (via->ddir_b & via->data_b) | ((via->ddir_b ^ 0xff) & rv);
		break;
	case 1:
	case 15:	// TODO no handshake
		if (via->get_port_a_in) {
			rv = via->get_port_a_in(via->data_a);
		}
		rv = (via->ddir_a & via->data_a) | ((via->ddir_a ^ 0xff) & rv);
		break;
	case 2:
		rv = via->ddir_b;	
		break;
	case 3:
		rv = via->ddir_a;
		break;
	case 4:
		rv = via->t1cl;
		break;
	case 5:
		rv = via->t1ch;
		break;
	case 6: 
		rv = via->t1ll;
		break;
	case 7:
		rv = via->t1lh;
		break;
	case 8:
		rv = via->t2cl;
		break;
	case 9:
		rv = via->t2ch;
		break;
	case 10:
		rv = via->sr;
		break;
	case 11:
		rv = via->acr;
		break;
	case 12:
		rv = via->pcr;
		break;
	case 13:
		rv = via->ifr;
		break;
	case 14:
		rv = via->ier;
		break;
	}
	return rv;
}

// extern trigger to set CA1 / CA2 input state
void via_ca1(VIA *via, uchar flag);

void via_ca2(VIA *via, uchar flag);

void via_cb1(VIA *via, uchar flag);

void via_cb2(VIA *via, uchar flag);


void via_init(VIA *p) {

	memset(p, 0, sizeof(VIA));

}

