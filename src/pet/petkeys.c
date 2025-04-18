
#include <stdio.h>
#include <ctype.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "cpu.h"
#include "mem.h"
#include "ccurses.h"
#include "stop.h"

#include "keys.h"
#include "config.h"


int timer;

#define	NUM_ROWS	16

uchar prb[NUM_ROWS];

// single key press definition - when what row is selected,
// which cols should be set
typedef struct {
	uchar 	row;
	uchar	col;
} keys;

// definition of the keys to use for the host key
// cnt = number of simultanous key presses
// keys[] is the list of keys to be pressed
typedef struct { 
	int 	cnt;
	keys	k[4];	
} keytab;

// extended key map
// contains the host key, plus the keytab entry
// to be searched for non-standard characters
typedef struct {
	int key;
	keytab 	k;
} xkeytab;

// number of extended key map entries
#define	ANZXKEYS	21	

// extended keymap entry
xkeytab xkeys[];
// key table - 128 entries, addressed by index
keytab ktab[];

extern keytab stop;

#define	KEYTIMER	10000	/* each (simulated) ms */

void key_exit(void);
void key_irq(scnt);


uchar key_read_cols(uchar row) {
	return prb[row];
}

void key_irq(scnt adr) {

	static int xflag;
	uchar rows;
	chtype key;
	keytab *k = NULL;
	scnt i;	

	//logout(0, "checking for key");

	// do we have a character in curses?
	if((key=esc_getch())!=ERR) {	 	/* read a char */

		logout(0, "get key %d (%c)", key, isprint(key)?key:'.');

		if(key<128) {
			// standard chars 0-127
			k=&ktab[key];
		} else 
		if(key>255) {
			// evaluate extended char table
			for(i=0;i<ANZXKEYS;i++) {
				if(xkeys[i].key==key) {
					k=&(xkeys[i].k);
					break;
				}
			}
		}
	}

	// Stop overrides key
	if (stop_ack_flag()) {
		k = &stop;
	}

	// do we have a translation?
	if(k) {	
		// yes. so handle it

		//logout(0, "translate into %d strokes", k->cnt);

		// 1. combine rows for all keys as mask
		// (just an optimization for the next step)
 		rows=0;
		for (i=0; i<k->cnt; i++) {
			rows|=k->k[i].row;
		}
		// iterate over all NUM_ROWS combinations of rows
		for(i=0;i<NUM_ROWS;i++) {
			uchar cols=0xff;
			// is any of the 8 rows (8 bits in 256 iterations) set?
			// note: with PET's 10 rows maybe not needed optimization
			if(1) { //(~i)&rows) {
				// yes, check which cols to set
				scnt j=0;
				while(j<k->cnt) {
					//if((~i)&k->k[j].row)
					if(i == k->k[j].row)
						cols &= k->k[j].col;
					j++;
				}
				prb[i]=cols;
			} else  {
				// no, set with $ff
				prb[i]=0xff;
			}
			//logout(0, "map %d -> %02x", i, prb[i]);
		}
		xflag =0;
	} else {				/* no char */
		if(!xflag) {
			for(i=0;i<NUM_ROWS;i++) {
				prb[i]=0xff;
			}
			xflag=1;
		}
	}	
}

void key_alarm_cb(struct alarm_s *alarm, CLOCK current) {

	key_irq(0);

	// every 100ms
	set_alarm_clock_plus(alarm, 100000);
}

static alarm_t key_alarm = {
	"keyboard",
	NULL,
	key_alarm_cb,
	NULL,
	CLOCK_MAX
};

void key_init(BUS *bus) {
	int i;
	
	for(i=0;i<NUM_ROWS;i++) { 
		prb[i]=0xff; 
	}

	alarm_register(&bus->actx, &key_alarm);
	set_alarm_clock(&key_alarm, 0);
}


#define	FREE	{ 0,0 }

#define	K_1	{ 6,	0xbf }
#define ARROWL	{ 0,	0xdf }
//#define	CTRL	{ 0x80,	0xfb }
#define	K_2	{ 7,	0xbf }
#define	SPC	{ 9,	0xfb }
//#define	CBM	{ 0x80, 0xdf }
#define	K_Q	{ 2,	0xfe }
#define	STP	{ 9, 	0xef }

//#define	POUND	{ 0x40,	0xfe }
#define	BACKSLASH { 1,	0xf7 }
#define	ASTER	{ 5, 	0x7f }
#define	SEMIC	{ 6, 	0xef }
#define	HOME	{ 0,	0xbf }
#define	SHIFTR	{ 8,	0xdf }
#define	EQUAL	{ 9, 	0x7f }
//#define POWER	{ 5, 	0x7f }
#define	DIVIDE	{ 3,	0x7f }

#define	ADD	{ 7, 	0x7f }
#define	K_P	{ 3,	0xef }
#define	K_L	{ 4,	0xef }
#define	DASH	{ 3,	0x7f }
#define	DOT	{ 9, 	0xbf }
#define	COLON	{ 5, 	0xef }
#define	AT	{ 8, 	0xfd }
#define	COMMA	{ 7, 	0xf7 }

#define	HASH	{ 0, 	0xfd }
#define	EXCL	{ 0, 	0xfe }
#define	QUOTE	{ 1, 	0xfe }
#define	DOLLAR	{ 1, 	0xfd }
#define	PERCENT	{ 0, 	0xfb }
#define	AMP	{ 0, 	0xf7 }
#define	SQUOTE	{ 1, 	0xfb }
#define	BROPEN	{ 0, 	0xef }
#define	BRCLOSE	{ 1, 	0xef }
#define	UPARR	{ 2, 	0xdf }
#define	LESS	{ 9, 	0xf7 }
#define	MORE	{ 8, 	0xef }
#define	QUEST	{ 7, 	0xef }

#define	K_9	{ 2,	0x7f }
#define	K_I	{ 3,	0xf7 }
#define	K_J	{ 4,	0xf7 }
#define	K_0	{ 8,	0xbf }
#define	K_M	{ 6,	0xf7 }
#define	K_K	{ 5,	0xf7 }
#define	K_O	{ 2,	0xef }
#define	K_N	{ 7,	0xfb }

#define	K_7	{ 2,	0xbf }
#define	K_Y	{ 3,	0xfb }
#define	K_G	{ 4,	0xfb }
#define	K_8	{ 3,	0xbf }
#define	K_B	{ 6,	0xfb }
#define	K_H	{ 5,	0xfb }
#define	K_U	{ 2,	0xf7 }
#define	K_V	{ 7,	0xfd }

#define	K_5	{ 5,	0xbf }
#define	K_R	{ 3,	0xfd }
#define	K_D	{ 4,	0xfd }
#define	K_6	{ 4,	0x7f }
#define	K_C	{ 6,	0xfd }
#define	K_F	{ 5,	0xfd }
#define	K_T	{ 2,	0xfb }
#define	K_X	{ 7,	0xfe }

#define	K_3	{ 6,	0x7f }
#define	K_W	{ 3,	0xfe }
#define	K_A	{ 4, 	0xfe }
#define	K_4	{ 4,	0xbf }
#define	K_Z	{ 6,	0xfe }
#define	K_S	{ 5,	0xfe }
#define	K_E	{ 2,	0xfd }
#define	SHIFTL	{ 8,	0xfe }

#define	DEL	{ 1,	0x7f }
#define	CR	{ 6, 	0xdf }
#define	CRSRR	{ 0,	0x7f }
//#define	K_F7	{ 0x01,	0xf7 }
//#define	K_F1	{ 0x01,	0xef }
//#define	K_F3	{ 0x01,	0xdf }
//#define	K_F5	{ 0x01,	0xbf }
#define	CRSRD	{ 1,	0xbf }

keytab stop = { 1, { STP } };

keytab ktab[128] = {
/* ^@	*/ { 0 }, //2,	{ CTRL, AT }},
/* ^A	*/ { 0 }, //2, { CTRL, K_A }},
/* ^B	*/ { 0 }, //2, { CTRL, K_B }},
/* ^C	*/ { 0 }, //2, { CTRL, K_C }},
/* ^D 	*/ { 1, { STP } }, //2, { CTRL, K_D }}, // replacement for STOP
/* ^E 	*/ { 0 }, //2, { CTRL, K_E }},
/* ^F	*/ { 0 }, //2, { CTRL, K_F }},
/* ^G	*/ { 0 }, //2, { CTRL, K_G }},
/* ^H	*/ { 0 }, //2, { CTRL, K_H }},
/* ^I	*/ { 0 }, //2, { CTRL, K_I }},
/* ^J	*/ { 1, { CR }}, 	/*{ 2, { CTRL, K_J }},*/
/* ^K	*/ { 0 }, //2, { CTRL, K_K }},
/* ^L	*/ { 0 }, //2, { CTRL, K_L }},
/* ^M	*/ { 1, { CR }},	//{ 0 }, //2, { CTRL, K_M }},
/* ^N	*/ { 0 }, //2, { CTRL, K_N }},
/* ^O	*/ { 0 }, //2, { CTRL, K_O }},
/* ^P	*/ { 0 }, //2, { CTRL, K_P }},
/* ^Q	*/ { 0 }, //2, { CTRL, K_Q }},
/* ^R	*/ { 0 }, //2, { CTRL, K_R }},
/* ^S	*/ { 0 }, //2, { CTRL, K_S }},
/* ^T	*/ { 0 }, //2, { CTRL, K_T }},
/* ^U	*/ { 0 }, //2, { CTRL, K_U }},
/* ^V	*/ { 0 }, //2, { CTRL, K_V }},
/* ^W	*/ { 0 }, //2, { CTRL, K_W }},
/* ^X	*/ { 0 }, //2, { CTRL, K_X }},
/* ^Y	*/ { 0 }, //2, { CTRL, K_Y }},
/* ^Z	*/ { 0 }, //2, { CTRL, K_Z }},

/* 0x1b	*/ { 0 }, //2, { CTRL, COLON }},
/* 0x1c	*/ { 0 }, //2, { CTRL, K_3 }},
/* 0x1d	*/ { 1, { CRSRR }},
/* 0x1e	*/ { 0 }, //2, { CTRL, K_6 }},
/* 0x1f	*/ { 0 }, //2, { CTRL, K_7 }},

/* ' ' 	*/ { 1, { SPC }},
/* '!'	*/ { 1, { EXCL }},
/* '"'	*/ { 1, { QUOTE }},
/* '#' 	*/ { 1, { HASH }},
/* '$' 	*/ { 1, { DOLLAR }},
/* '%'	*/ { 1, { PERCENT }},
/* '&'	*/ { 1, { AMP }},
/* ''	*/ { 1, { SQUOTE }},
/* '(' 	*/ { 1, { BROPEN }},
/* ')'	*/ { 1, { BRCLOSE }},
/* '*'	*/ { 1, { ASTER }},
/* '+'	*/ { 1, { ADD }},
/* ','	*/ { 1, { COMMA }},
/* '-'	*/ { 1, { DASH }}, 
/* '.'	*/ { 1, { DOT }},
/* '/' 	*/ { 1, { DIVIDE }},
/* 0	*/ { 1, { K_0 }},
/* 1	*/ { 1, { K_1 }},
/* 2	*/ { 1, { K_2 }},
/* 3	*/ { 1, { K_3 }},
/* 4	*/ { 1, { K_4 }},
/* 5 	*/ { 1, { K_5 }},
/* 6 	*/ { 1, { K_6 }},
/* 7	*/ { 1, { K_7 }},
/* 8	*/ { 1, { K_8 }},
/* 9	*/ { 1, { K_9 }},
/* ':'	*/ { 1, { COLON }},
/* ';'	*/ { 1, { SEMIC }},
/* '<'	*/ { 1, { LESS }},
/* '=' 	*/ { 1, { EQUAL }},
/* '>'	*/ { 1, { MORE }},
/* '?'	*/ { 1, { QUEST }},

/* '@'	*/ { 1, { AT }},
/* A	*/ { 2, { SHIFTR , K_A }},
/* A	*/ { 2, { SHIFTR , K_B }},
/* A	*/ { 2, { SHIFTR , K_C }},
/* A	*/ { 2, { SHIFTR , K_D }},
/* A	*/ { 2, { SHIFTR , K_E }},
/* A	*/ { 2, { SHIFTR , K_F }},
/* A	*/ { 2, { SHIFTR , K_G }},
/* A	*/ { 2, { SHIFTR , K_H }},
/* A	*/ { 2, { SHIFTR , K_I }},
/* A	*/ { 2, { SHIFTR , K_J }},
/* A	*/ { 2, { SHIFTR , K_K }},
/* A	*/ { 2, { SHIFTR , K_L }},
/* A	*/ { 2, { SHIFTR , K_M }},
/* A	*/ { 2, { SHIFTR , K_N }},
/* A	*/ { 2, { SHIFTR , K_O }},
/* A	*/ { 2, { SHIFTR , K_P }},
/* A	*/ { 2, { SHIFTR , K_Q }},
/* A	*/ { 2, { SHIFTR , K_R }},
/* A	*/ { 2, { SHIFTR , K_S }},
/* A	*/ { 2, { SHIFTR , K_T }},
/* A	*/ { 2, { SHIFTR , K_U }},
/* A	*/ { 2, { SHIFTR , K_V }},
/* A	*/ { 2, { SHIFTR , K_W }},
/* A	*/ { 2, { SHIFTR , K_X }},
/* A	*/ { 2, { SHIFTR , K_Y }},
/* A	*/ { 2, { SHIFTR , K_Z }},

/* '['	*/ { 2, { SHIFTR, COLON }},
/* '\'	*/ { 1, { BACKSLASH }},
/* ']'	*/ { 2, { SHIFTR, SEMIC }},
/* '^'	*/ { 1, { UPARR }},
/* '_'	*/ { 1, { ARROWL }},

/* '§'	*/ { 0 }, 
/* A	*/ { 1, { K_A }},
/* A	*/ { 1, { K_B }},
/* A	*/ { 1, { K_C }},
/* A	*/ { 1, { K_D }},
/* A	*/ { 1, { K_E }},
/* A	*/ { 1, { K_F }},
/* A	*/ { 1, { K_G }},
/* A	*/ { 1, { K_H }},
/* A	*/ { 1, { K_I }},
/* A	*/ { 1, { K_J }},
/* A	*/ { 1, { K_K }},
/* A	*/ { 1, { K_L }},
/* A	*/ { 1, { K_M }},
/* A	*/ { 1, { K_N }},
/* A	*/ { 1, { K_O }},
/* A	*/ { 1, { K_P }},
/* A	*/ { 1, { K_Q }},
/* A	*/ { 1, { K_R }},
/* A	*/ { 1, { K_S }},
/* A	*/ { 1, { K_T }},
/* A	*/ { 1, { K_U }},
/* A	*/ { 1, { K_V }},
/* A	*/ { 1, { K_W }},
/* A	*/ { 1, { K_X }},
/* A	*/ { 1, { K_Y }},
/* A	*/ { 1, { K_Z }},

/* '{'	*/ { 0 },
/* '|'	*/ { 0 },
/* '}'	*/ { 0 },
/* '~'	*/ { 2, { SHIFTR, UPARR }},
/* 0x7f */ { 0 }
};


xkeytab xkeys[ANZXKEYS] = {
	{ KEY_UP, 	{ 2, { SHIFTL, CRSRD }}},
	{ KEY_DOWN,	{ 1, { CRSRD }}},
	{ KEY_LEFT,	{ 2, { SHIFTL, CRSRR }}},
	{ KEY_RIGHT,	{ 1, { CRSRR }}},
//	{ KEY_F(1),	{ 1, { K_F1 }}},
//	{ KEY_F(2),	{ 2, { K_F1, SHIFTL }}},
//	{ KEY_F(3),	{ 1, { K_F3 }}},
//	{ KEY_F(4),	{ 2, { K_F3, SHIFTL }}},
//	{ KEY_F(5),	{ 1, { K_F5 }}},
//	{ KEY_F(6),	{ 2, { K_F5, SHIFTL }}},
//	{ KEY_F(7),	{ 1, { K_F7 }}},
//	{ KEY_F(8),	{ 2, { K_F7, SHIFTL }}},
	{ KEY_HOME,	{ 1, { HOME }}},
	{ KEY_SHOME,	{ 2, { SHIFTR, HOME }}},
	{ KEY_CLEAR,	{ 2, { SHIFTL, HOME }}},
	{ KEY_BACKSPACE,{ 1, { DEL }}},
	{ KEY_IC,	{ 2, { SHIFTL, DEL }}},
	{ KEY_DC,	{ 1, { DEL }}},
	{ KEY_ENTER,	{ 1, { CR }}},
	{ KEY_RESET,	{ 1, { STP }}},
	{ KEY_END,	{ 1, { STP }}},
};
	
