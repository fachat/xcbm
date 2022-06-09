
#include <string.h>

#include "log.h"
#include "types.h"
#include "pia.h"



static void update_int(PIA *pia) {

	if ((pia->ctrl_a & (PIA_CR_IRQ1 | PIA_CR_IRQ2))
		|| (pia->ctrl_b & (PIA_CR_IRQ1 | PIA_CR_IRQ2))) {
		// set interrupt
		pia->set_interrupt(pia->int_num, 1);
	} else {
		pia->set_interrupt(pia->int_num, 0);
	}
}


// read/write to PIA registers by the CPU
void pia_wr(PIA *pia, uchar reg, uchar val) {

	reg = reg & 0x03;

	//logout(0, "%s write %02x to reg %02x", pia->name, val, reg);

	switch(reg) {
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

	reg = reg & 0x03;
	
	//logout(0, "%s read from reg %02x", pia->name, reg);

	switch(reg) {
	case 0:
		if (pia->ctrl_a & PIA_CR_DATA) {
			// port data register selected in CR
			if (pia->get_port_a_in) {
				rv = pia->get_port_a_in(pia->data_a);
			}
			rv = (pia->ddir_a & pia->data_a) | ((pia->ddir_a ^ 0xff) & rv);
			// reading port A clears CA1 interrupt flag
			pia->ctrl_a &= ~PIA_CR_IRQ1;
			update_int(pia);
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
			// reading port B clears CA1 interrupt flag
			pia->ctrl_b &= ~PIA_CR_IRQ1;
			update_int(pia);
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

	if (pia->last_ca1 && !flag) {
		// falling edge
		if ((pia->ctrl_a & (PIA_CR_IRQ1EN + PIA_CR_IRQ1POS))
			 == (PIA_CR_IRQ1EN)) {
			// set IRQ flag
			pia->ctrl_a |= PIA_CR_IRQ1;
			update_int(pia);
		}
	} else
	if (!pia->last_ca1 && flag) {
		// rising edge
		if ((pia->ctrl_a & (PIA_CR_IRQ1EN + PIA_CR_IRQ1POS))
			 == (PIA_CR_IRQ1EN + PIA_CR_IRQ1POS)) {
			// set IRQ flag
			pia->ctrl_a |= PIA_CR_IRQ1;
			update_int(pia);
		}
	}
	pia->last_ca1 = flag;
}

void pia_ca2(PIA *pia, uchar flag);

void pia_cb1(PIA *pia, uchar flag) {
	logout(0, "set pia1 CB1 to %d", flag);

	if (pia->last_cb1 && !flag) {
		// falling edge
		if ((pia->ctrl_b & (PIA_CR_IRQ1EN + PIA_CR_IRQ1POS))
			 == (PIA_CR_IRQ1EN)) {
			// set IRQ flag
			pia->ctrl_b |= PIA_CR_IRQ1;
			update_int(pia);
		}
	} else
	if (!pia->last_cb1 && flag) {
		// rising edge
		if ((pia->ctrl_b & (PIA_CR_IRQ1EN + PIA_CR_IRQ1POS))
			 == (PIA_CR_IRQ1EN + PIA_CR_IRQ1POS)) {
			// set IRQ flag
			pia->ctrl_b |= PIA_CR_IRQ1;
			update_int(pia);
		}
	}
	pia->last_cb1 = flag;
}

void pia_cb2(PIA *pia, uchar flag);


void pia_init(PIA *p, const char *name) {

	memset(p, 0, sizeof(PIA));

	p->name = name;

	// DDR as input
	p->ddir_a = 0xff;
	p->ddir_b = 0xff;

	// all Cxy are input
	p->last_ca1 = 1;
	p->last_ca2 = 1;
	p->last_cb1 = 1;
	p->last_cb2 = 1;
}

