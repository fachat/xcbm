

typedef struct {
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
	uchar last_ca1;	// to detect transitions on input
	uchar last_ca2;	// to detect transitions on input

	// port B
	uchar ddir_b;
	uchar data_b;
	uchar last_cb1;
	uchar last_cb2;

	// shift register
	uchar sr;

	// timer 1
	uchar t1cl;	// counter low
	uchar t1ch;	// counter hi
	uchar t1ll;	// latch low
	uchar t1lh;	// latch high

	// timer 2
	uchar t2cl;	// counter low
	uchar t2ch;	// counter high
	uchar t2ll;	// latch low

	// control
	uchar acr;
	uchar pcr;
	
	// interrupt
	uchar ifr;
	uchar ier;

        void (*set_interrupt)(scnt int_num, uchar flag);
        scnt int_num;
} VIA;

// initialize a VIA
// sets all(!) fields. read/write_port_* must be set afterwards
void via_init(VIA *);

// read/write to VIA registers by the CPU
void via_wr(VIA *via, uchar reg, uchar val);
uchar via_rd(VIA *via, uchar reg);

// extern trigger to set CA1 / CA2 input state
void via_ca1(VIA *via, uchar flag);
void via_ca2(VIA *via, uchar flag);

void via_cb1(VIA *via, uchar flag);
void via_cb2(VIA *via, uchar flag);

