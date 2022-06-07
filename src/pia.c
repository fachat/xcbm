
#include <string.h>

#include "log.h"
#include "types.h"
#include "pia.h"


// read/write to PIA registers by the CPU
void pia_wr(PIA *pia, uchar reg, uchar val) {

	switch(reg & 0x03) {
	case 0:
		if (pia->ctrl_a & PIA_CR_DATA) {
			// port data register selected in CR
			pia->data_a = val;
		} else {
			// data direction register selected in CR
			pia->ddir_a = val;
		}
		if (pia->set_port_a_out) {
			pia->set_port_a_out(pia->data_a, pia->ddir_a);
		}
		break;
	case 1:
		if (pia->set_ca2_out) {
			if (val & PIA_CR_CX2) {
				// output
				if (val & PIA_CR_CX2OUT) {
					// follow bit 3
					pia->set_ca2_out( (val & PIA_CR_CX2SD) ? PIA_CX2_HIGH : PIA_CX2_LOW);
				}
			} else {
				// input
				pia->set_ca2_out(PIA_CX2_INPUT);
			}
		}
		pia->ctrl_a = val;
		break;
	case 2:
		if (pia->ctrl_b & PIA_CR_DATA) {
			// port data register selected in CR
			pia->data_b = val;
		} else {
			// data direction register selected in CR
			pia->ddir_b = val;
		}
		if (pia->set_port_b_out) {
			pia->set_port_b_out(pia->data_b, pia->ddir_b);
		}
		break;
	case 3:
		if (pia->set_cb2_out) {
			if (val & PIA_CR_CX2) {
				// output
				if (val & PIA_CR_CX2OUT) {
					// follow bit 3
					pia->set_cb2_out( (val & PIA_CR_CX2SD) ? PIA_CX2_HIGH : PIA_CX2_LOW);
				}
			} else {
				// input
				pia->set_cb2_out(PIA_CX2_INPUT);
			}
		}
		pia->ctrl_b = val;
		break;
	}
}

uchar pia_rd(PIA *pia, uchar reg) {

	register uchar rv = 0xff;	// default if get function not set
	
	switch(reg & 0x03) {
	case 0:
		if (pia->ctrl_a & PIA_CR_DATA) {
			// port data register selected in CR
			if (pia->get_port_a_in) {
				rv = pia->get_port_a_in(pia->data_a);
			}
			rv = (pia->ddir_a & pia->data_a) | ((pia->ddir_a ^ 0xff) & rv);
		} else {
			// data direction register selected in CR
			rv = pia->ddir_b;
		}
		break;
	case 1:
		rv = pia->ctrl_a;	
		break;
	case 2:
		if (pia->ctrl_b & PIA_CR_DATA) {
			// port data register selected in CR
			if (pia->get_port_b_in) {
				rv = pia->get_port_b_in(pia->data_b);
			}
			rv = (pia->ddir_b & pia->data_b) | ((pia->ddir_b ^ 0xff) & rv);
		} else {
			// data direction register selected in CR
			rv = pia->ddir_b;
		}
		break;
	case 3:
		rv = pia->ctrl_b;	
		break;
	}
	return rv;
}

// extern trigger to set CA1 / CA2 input state
void pia_ca1(PIA *pia, uchar flag) {
	logout(0, "set pia1 CA1 to %d", flag);
}

void pia_ca2(PIA *pia, uchar flag);

void pia_cb1(PIA *pia, uchar flag);

void pia_cb2(PIA *pia, uchar flag);


void pia_init(PIA *p) {

	memset(p, 0, sizeof(PIA));

}

