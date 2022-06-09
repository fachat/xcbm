

#define	PIA_PA	0
#define	PIA_CRA	1
#define	PIA_PB	2
#define	PIA_CRB	3

/* binary constances are only an extension to GCC, so hex here */
#define	PIA_CR_IRQ1	0x80
#define	PIA_CR_IRQ2	0x40
#define	PIA_CR_CX2	0x20
#define	PIA_CR_CX2OUT	0x10	/* output bit 3 when set, strobe otherwise */
#define	PIA_CR_CX2SD	0x08	/* strobe or data */
#define	PIA_CR_DATA	0x04
#define	PIA_CR_IRQ1POS	0x02
#define	PIA_CR_IRQ1EN	0x01

#define	PIA_CX2_LOW	0
#define	PIA_CX2_HIGH	1
#define	PIA_CX2_INPUT	2

#define	PIA_CX1_LOW	0
#define	PIA_CX1_HIGH	1

typedef struct PIA {
	// instance-specific functions that need to be set per instance
	//
	// read functions only change the bits and return unconnected bits as of origdata
	// write function will get output bits, plus input bits as high (from assumed internal pull-ups)
	// Those are being called when changes happen by CPU writes or other events
	//
	uchar (*get_port_a_in)(uchar origdata);
	void (*set_port_a_out)(uchar data, uchar dir);
	void (*set_ca2_out)(uchar flag);

	uchar (*get_port_b_in)(uchar origdata);
	void (*set_port_b_out)(uchar data, uchar dir);
	void (*set_cb2_out)(uchar flag);

	// internal state
	//
	// port A
	uchar ddir_a;	// data direction register A
	uchar data_a;	// data output register A
	uchar ctrl_a;	// control register A
	uchar last_ca1;	// to detect transitions on input
	uchar last_ca2;	// to detect transitions on input

	// port B
	uchar ddir_b;
	uchar data_b;
	uchar ctrl_b;
	uchar last_cb1;
	uchar last_cb2;

	// management

	const char * name;

	void (*set_interrupt)(scnt int_num, uchar flag);
	scnt int_num;
} PIA;

// initialize a PIA
// sets all(!) fields. read/write_port_* must be set afterwards
void pia_init(PIA *, const char *name);

// read/write to PIA registers by the CPU
void pia_wr(PIA *pia, uchar reg, uchar val);
uchar pia_rd(PIA *pia, uchar reg);

// extern trigger to set CA1 / CA2 input state
void pia_ca1(PIA *pia, uchar flag);
void pia_ca2(PIA *pia, uchar flag);

void pia_cb1(PIA *pia, uchar flag);
void pia_cb2(PIA *pia, uchar flag);

