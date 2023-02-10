
/* MOS 6522 registers */
#define VIA_PRB         0  /* Port B */
#define VIA_PRA         1  /* Port A */
#define VIA_DDRB        2  /* Data direction register for port B */
#define VIA_DDRA        3  /* Data direction register for port A */

#define VIA_T1CL        4  /* Timer 1 count low */
#define VIA_T1CH        5  /* Timer 1 count high */
#define VIA_T1LL        6  /* Timer 1 latch low */
#define VIA_T1LH        7  /* Timer 1 latch high */
#define VIA_T2CL        8  /* Timer 2 count low - read only */
#define VIA_T2LL        8  /* Timer 2 latch low - write only */
#define VIA_T2CH        9  /* Timer 2 count high - read only */
#define VIA_T2LH        9  /* Timer 2 latch high - write only */

#define VIA_SR          10 /* Serial port shift register */
#define VIA_ACR         11 /* Auxiliary control register */
#define VIA_PCR         12 /* Peripheral control register */

#define VIA_IFR         13 /* Interrupt flag register */
#define VIA_IER         14 /* Interrupt control register */
#define VIA_PRA_NHS     15 /* Port A with no handshake */

/* Interrupt Masks  */
/* MOS 6522 */
#define VIA_IM_IRQ      128     /* Control Bit */
#define VIA_IM_T1       64      /* Timer 1 underflow */
#define VIA_IM_T2       32      /* Timer 2 underflow */
#define VIA_IM_CB1      16      /* Handshake */
#define VIA_IM_CB2      8       /* Handshake */
#define VIA_IM_SR       4       /* Shift Register completion */
#define VIA_IM_CA1      2       /* Handshake */
#define VIA_IM_CA2      1       /* Handshake */




typedef struct {
	BUS *bus;

	const char *name;

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

	alarm_t sralarm;

	// timer 1
	uchar t1cl;	// counter low
	uchar t1ch;	// counter hi
	uchar t1ll;	// latch low
	uchar t1lh;	// latch high

	CLOCK tau;
	CLOCK tal;
	CLOCK tai;

	uchar t1pb7;

	alarm_t t1alarm;

	// timer 2
	uchar t2cl;	// counter low
	uchar t2ch;	// counter high
	uchar t2ll;	// latch low
	uchar t2lh;	// latch high

	CLOCK tbu;
	CLOCK tbi;

	alarm_t t2alarm;

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
void via_init(VIA *, BUS *, const char *name);

// read/write to VIA registers by the CPU
void via_wr(VIA *via, scnt addr, uchar val);
uchar via_rd(VIA *via, scnt addr);

// extern trigger to set CA1 / CA2 input state
void via_ca1(VIA *via, uchar flag);
void via_ca2(VIA *via, uchar flag);

void via_cb1(VIA *via, uchar flag);
void via_cb2(VIA *via, uchar flag);

