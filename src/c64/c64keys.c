
#include <stdio.h>

#include "log.h"
#include "config.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "cpu.h"
#include "mem.h"
#include "mem64.h"
#include "ccurses.h"

#include "c64io.h"
#include "keys.h"


int timer;

uchar pra[256];
uchar prb[256];

typedef struct {
	uchar 	row;
	uchar	col;
} keys;

typedef struct { 
	int 	cnt;
	keys	k[4];	
} keytab;

typedef struct {
	int key;
	keytab 	k;
} xkeytab;

#define	ANZXKEYS	21	

xkeytab xkeys[];
keytab ktab[];

#define	KEYTIMER	10000	/* each (simulated) ms */

void key_exit(void);
void key_irq(CPU*, scnt);

void key_init(CPU *cpu) {
	int i;
	
/*
	timer=time_register(key_irq);

	time_setval(timer,KEYTIMER);
	time_seton(timer);
*/
	rom_settrap(0xea31,key_irq,"keytrap" );
	for(i=0;i<256;i++) { prb[i]=0xff; pra[i]=0xff; }
}

void key_irq(CPU *cpu, scnt adr) {
	static int xflag;
	uchar rows;
	chtype key;
	keytab *k = NULL;
	scnt i;	
/*	time_setval(timer, KEYTIMER);*/

	if((key=esc_getch())!=ERR) {	 	/* read a char */
		if(key<128) {
			k=&ktab[key];
		} else 
		if(key>255) {
			for(i=0;i<ANZXKEYS;i++) {
				if(xkeys[i].key==key) {
					k=&(xkeys[i].k);
					break;
				}
			}
		}
		if(k) {	
	 		rows=0;
			for (i=0; i<4; i++) {
				rows|=k->k[i].row;
			}
			for(i=0;i<256;i++) {
				uchar cols=0xff;
				if((~i)&rows) {
					scnt j=0;
					while(j<4) {
						if((~i)&k->k[j].row)
							cols&=k->k[j].col;
						j++;
					}
					prb[i]=cols;
				} else  {
					prb[i]=0xff;
				}
			}
			xflag =0;
		}
	} else {				/* no char */
		if(!xflag) {
			for(i=0;i<256;i++) prb[i]=0xff;
			xflag=1;
		}
	}	
}

#define	FREE	{ 0,0 }

#define	K_1	{ 0x80,	0xfe }
#define ARROWL	{ 0x80,	0xfd }
#define	CTRL	{ 0x80,	0xfb }
#define	K_2	{ 0x80,	0xf7 }
#define	SPC	{ 0x80,	0xef }
#define	CBM	{ 0x80, 0xdf }
#define	K_Q	{ 0x80,	0xbf }
#define	STP	{ 0x80, 0x7f }

#define	POUND	{ 0x40,	0xfe }
#define	ASTER	{ 0x40, 0xfd }
#define	SEMIC	{ 0x40, 0xfb }
#define	CLR	{ 0x40,	0xf7 }
#define	SHIFTR	{ 0x40, 0xef }
#define	EQUAL	{ 0x40, 0xdf }
#define POWER	{ 0x40, 0xbf }
#define	DIVIDE	{ 0x40, 0x7f }

#define	ADD	{ 0x20, 0xfe }
#define	K_P	{ 0x20,	0xfd }
#define	K_L	{ 0x20,	0xfb }
#define	DASH	{ 0x20,	0xf7 }
#define	DOT	{ 0x20, 0xef }
#define	COLON	{ 0x20, 0xdf }
#define	AT	{ 0x20, 0xbf }
#define	COMMA	{ 0x20, 0x7f }

#define	K_9	{ 0x10,	0xfe }
#define	K_I	{ 0x10,	0xfd }
#define	K_J	{ 0x10,	0xfb }
#define	K_0	{ 0x10,	0xf7 }
#define	K_M	{ 0x10,	0xef }
#define	K_K	{ 0x10,	0xdf }
#define	K_O	{ 0x10,	0xbf }
#define	K_N	{ 0x10,	0x7f }

#define	K_7	{ 0x08,	0xfe }
#define	K_Y	{ 0x08,	0xfd }
#define	K_G	{ 0x08,	0xfb }
#define	K_8	{ 0x08,	0xf7 }
#define	K_B	{ 0x08,	0xef }
#define	K_H	{ 0x08,	0xdf }
#define	K_U	{ 0x08,	0xbf }
#define	K_V	{ 0x08,	0x7f }

#define	K_5	{ 0x04,	0xfe }
#define	K_R	{ 0x04,	0xfd }
#define	K_D	{ 0x04,	0xfb }
#define	K_6	{ 0x04,	0xf7 }
#define	K_C	{ 0x04,	0xef }
#define	K_F	{ 0x04,	0xdf }
#define	K_T	{ 0x04,	0xbf }
#define	K_X	{ 0x04,	0x7f }

#define	K_3	{ 0x02,	0xfe }
#define	K_W	{ 0x02,	0xfd }
#define	K_A	{ 0x02, 0xfb }
#define	K_4	{ 0x02,	0xf7 }
#define	K_Z	{ 0x02,	0xef }
#define	K_S	{ 0x02,	0xdf }
#define	K_E	{ 0x02,	0xbf }
#define	SHIFTL	{ 0x02,	0x7f }

#define	DEL	{ 0x01,	0xfe }
#define	CR	{ 0x01, 0xfd }
#define	CRSRR	{ 0x01,	0xfb }
#define	K_F7	{ 0x01,	0xf7 }
#define	K_F1	{ 0x01,	0xef }
#define	K_F3	{ 0x01,	0xdf }
#define	K_F5	{ 0x01,	0xbf }
#define	CRSRD	{ 0x01,	0x7f }

keytab ktab[128] = {
/* ^@	*/ { 2,	{ CTRL, AT }},
/* ^A	*/ { 2, { CTRL, K_A }},
/* ^B	*/ { 2, { CTRL, K_B }},
/* ^C	*/ { 2, { CTRL, K_C }},
/* ^D 	*/ { 2, { CTRL, K_D }},
/* ^E 	*/ { 2, { CTRL, K_E }},
/* ^F	*/ { 2, { CTRL, K_F }},
/* ^G	*/ { 2, { CTRL, K_G }},
/* ^H	*/ { 2, { CTRL, K_H }},
/* ^I	*/ { 2, { CTRL, K_I }},
/* ^J	*/ { 1, { CR }}, 	/*{ 2, { CTRL, K_J }},*/
/* ^K	*/ { 2, { CTRL, K_K }},
/* ^L	*/ { 2, { CTRL, K_L }},
/* ^M	*/ { 2, { CTRL, K_M }},
/* ^N	*/ { 2, { CTRL, K_N }},
/* ^O	*/ { 2, { CTRL, K_O }},
/* ^P	*/ { 2, { CTRL, K_P }},
/* ^Q	*/ { 2, { CTRL, K_Q }},
/* ^R	*/ { 2, { CTRL, K_R }},
/* ^S	*/ { 2, { CTRL, K_S }},
/* ^T	*/ { 2, { CTRL, K_T }},
/* ^U	*/ { 2, { CTRL, K_U }},
/* ^V	*/ { 2, { CTRL, K_V }},
/* ^W	*/ { 2, { CTRL, K_W }},
/* ^X	*/ { 2, { CTRL, K_X }},
/* ^Y	*/ { 2, { CTRL, K_Y }},
/* ^Z	*/ { 2, { CTRL, K_Z }},

/* 0x1b	*/ { 2, { CTRL, COLON }},
/* 0x1c	*/ { 2, { CTRL, K_3 }},
/* 0x1d	*/ { 1, { CRSRR }},
/* 0x1e	*/ { 2, { CTRL, K_6 }},
/* 0x1f	*/ { 2, { CTRL, K_7 }},

/* ' ' 	*/ { 1, { SPC }},
/* '!'	*/ { 2, { SHIFTL, K_1 }},
/* '"'	*/ { 2, { SHIFTL, K_2 }},
/* '#' 	*/ { 2, { SHIFTL, K_3 }},
/* '$' 	*/ { 2, { SHIFTL, K_4 }},
/* '%'	*/ { 2, { SHIFTL, K_5 }},
/* '&'	*/ { 2, { SHIFTL, K_6 }},
/* ''	*/ { 2, { SHIFTL, K_7 }},
/* '(' 	*/ { 2, { SHIFTL, K_8 }},
/* ')'	*/ { 2, { SHIFTL, K_9 }},
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
/* '<'	*/ { 2, { SHIFTL, COMMA }},
/* '=' 	*/ { 1, { EQUAL }},
/* '>'	*/ { 2, { SHIFTL, DOT }},
/* '?'	*/ { 2, { SHIFTL, DIVIDE }},

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
/* '\'	*/ { 1, { POUND }},
/* ']'	*/ { 2, { SHIFTR, SEMIC }},
/* '^'	*/ { 1, { POWER }},
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
/* '~'	*/ { 2, { SHIFTR, POWER }},
/* 0x7f */ { 0 }
};

xkeytab xkeys[ANZXKEYS] = {
	{ KEY_UP, 	{ 2, { SHIFTL, CRSRD }}},
	{ KEY_DOWN,	{ 1, { CRSRD }}},
	{ KEY_LEFT,	{ 2, { SHIFTL, CRSRR }}},
	{ KEY_RIGHT,	{ 1, { CRSRR }}},
	{ KEY_F(1),	{ 1, { K_F1 }}},
	{ KEY_F(2),	{ 2, { K_F1, SHIFTL }}},
	{ KEY_F(3),	{ 1, { K_F3 }}},
	{ KEY_F(4),	{ 2, { K_F3, SHIFTL }}},
	{ KEY_F(5),	{ 1, { K_F5 }}},
	{ KEY_F(6),	{ 2, { K_F5, SHIFTL }}},
	{ KEY_F(7),	{ 1, { K_F7 }}},
	{ KEY_F(8),	{ 2, { K_F7, SHIFTL }}},
	{ KEY_HOME,	{ 1, { CLR }}},
	{ KEY_SHOME,	{ 2, { SHIFTR, CLR }}},
	{ KEY_CLEAR,	{ 2, { SHIFTL, CLR }}},
	{ KEY_BACKSPACE,{ 1, { DEL }}},
	{ KEY_IC,	{ 2, { SHIFTL, DEL }}},
	{ KEY_DC,	{ 1, { DEL }}},
	{ KEY_ENTER,	{ 1, { CR }}},
	{ KEY_RESET,	{ 1, { STP }}},
	{ KEY_END,	{ 1, { STP }}},
};
	
