
#include 	<stdio.h>

#include  	"log.h"
#include  	"types.h"
#include  	"alarm.h"
#include  	"bus.h"
#include	"cpu.h"
#include	"timer.h"
#include 	"mem.h"
#include 	"speed.h"
#include 	"mon.h"
#include 	"labels.h"

#define	MAXLINE	200


char *kt[] ={ 
     "adc","and","asl","bbr","bbs","bcc","bcs","beq",
     "bit","bmi",
     "bne","bpl","bra","brk","bvc","bvs","clc","cld",
     "cli",
     "clv","cmp","cpx","cpy","dec","dex","dey","eor",
     "inc","inx","iny","jmp","jsr","lda","ldx","ldy",
     "lsr","nop","ora","pha","php","phx","phy","pla",
     "plp","plx","ply","rmb","rol",
     "ror","rti","rts","sbc","sec","sed","sei","smb",
     "sta",
     "stx","sty","stz","tax","tay","trb","tsb","tsx",
     "txa","txs","tya",
};

int cmd[256]=
   { 13, 37, -1, -1, -1, 37,  2, -1, 39, 37,  2, -1, -1, 37,  2, -1,
     11, 37, -1, -1, -1, 37,  2, -1, 16, 37, -1, -1, -1, 37,  2, -1,
     31,  1, -1, -1,  8,  1, 47, -1, 43,  1, 47, -1,  8,  1, 47, -1,
      9,  1, -1, -1, -1,  1, 47, -1, 52,  1, -1, -1, -1,  1, 47, -1,
     49, 26, -1, -1, -1, 26, 35, -1, 38, 26, 35, -1, 30, 26, 35, -1,
     14, 26, -1, -1, -1, 26, 35, -1, 18, 26, -1, -1, -1, 26, 35, -1,
     50,  0, -1, -1, -1,  0, 48, -1, 42,  0, 48, -1, 30,  0, 48, -1,
     15,  0, -1, -1, -1,  0, 48, -1, 54,  0, -1, -1, -1,  0, 48, -1,

     -1, 56, -1, -1, 58, 56, 57, -1, 25, -1, 65, -1, 58, 56, 57, -1,
      5, 56, -1, -1, 58, 56, 57, -1, 67, 56, 66, -1, -1, 56, -1, -1,
     34, 32, 33, -1, 34, 32, 33, -1, 61, 32, 60, -1, 34, 32, 33, -1,
      6, 32, -1, -1, 34, 32, 33, -1, 19, 32, 64, -1, 34, 32, 33, -1,
     22, 20, -1, -1, 22, 20, 23, -1, 29, 20, 24, -1, 22, 20, 23, -1,
     10, 20, -1, -1, -1, 20, 23, -1, 17, 20, -1, -1, -1, 20, 23, -1, 
     21, 51, -1, -1, 21, 51, 27, -1, 28, 51, 36, -1, 21, 51, 27, -1,
      7, 51, -1, -1, -1, 51, 27, -1, 53, 51, -1, -1, -1, 51, 27, -1 
};
      
int adm[256]=
   {  0, 12, -1, -1, -1,  3,  3, -1,  0,  1,  0, -1, -1,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1,
      2, 12, -1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1,
      0, 12, -1, -1, -1,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1,
      0, 12, -1, -1, -1,  3,  3, -1,  0,  1,  0, -1, 14,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  1, -1, -1, -1,  6,  6, -1,

     -1, 12, -1, -1,  3,  3,  3, -1,  0, -1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1,  4,  4,  5, -1,  0,  7,  0, -1, -1,  6, -1, -1,
      1, 12,  1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1,  4,  4,  5, -1,  0,  7,  0, -1,  6,  6,  7, -1,
      1, 12, -1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1, 
      1, 12, -1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1 
};

int adl[16]= { 1,2,3,2,2,2,3,3,3,1,2,3,2,2,3,3 };
char *ad1[16]={ "", "#", "", "", "", "", "", "", "(", "", "", "", "(", "(", "(", "(" };
char *ad2[16]={ "", "", "", "", ",x", ",y", ",x", ",y", ",x)", "", "", "", ",x)", "),y", ")", ")" };

// note: pc is unsigned int, as a bank can have > 64k
int dis6502(bank_t *bank, unsigned int pc, char *l, int maxlen){
	int c=bank->peek(bank, pc); //getbyt(pc);
	int o;
	int a = 0;
	int d;
	int f = 0;
	int olen = 1;
	scnt ad = 0;
	const char *ln = NULL;
	char addrbuf[10];

	const char *al = label_lookup(pc);
	if (al == NULL) {
		al = "";
	}

	if(cmd[c]<0){
	  sprintf(l," %02x illegal opcode!    ",c);
	} else {
	  a=adm[c];
	  o=cmd[c];
	  olen = adl[a];
	  switch(olen){
	  case 1:
	    sprintf(l," %02x        %-12s %s                        ", c, al, kt[o]);
	    break;
	  case 2:
	    ad = d = bank->peek(bank, pc+1); //getbyt(pc+1);
	    if (a == 10) {
		ad = pc + 2 + ad - 256*(ad>127);
	    }
	    ln = label_lookup(ad);
	    if (ln == NULL || a == 1) {
	    	snprintf(addrbuf, 10, (a==10) ? "$%04x" : "$%02x", ad);
		ln = addrbuf;
	    }
	    sprintf(l," %02x %02x     %-12s %s %s%s%s          ",
		c, d, al, kt[o],ad1[a],ln,ad2[a]);
	    if(a!=1) f=1;
	    break;
	  case 3:
	    ad = (bank->peek(bank, pc+1) & 0xff ) | (( bank->peek(bank, pc+2) << 8) & 0xff00); //getadr(pc+1);
	    ln = label_lookup(ad);
	    if (ln == NULL) {
	    	snprintf(addrbuf, 10, "$%04x", ad);
		ln = addrbuf;
	    }
	    sprintf(l," %02x %02x %02x  %-12s %s %s%s%s           ",
	      c, ad&255, (ad>>8)&255, al, kt[o],ad1[a],ln,ad2[a]);
	    if(c!=0x4c && c!=0x20) f=1;
	    break;
	  default:
	    break;
	  }
	}
	if (a == 13) {
		// indirect, y-indexed
		sprintf(l+40, "; (%02x) -> %04x", ad, (bank->peek(bank, ad) & 0xff) | ((bank->peek(bank, ad+1) << 8) & 0xff00));
	}
	if(f) {
#if 0 	/* clears interrupt flags? */
	  logout(0,"mem@ %04x = %02x %02x %02x %02x %02x %02x %02x %02x",
	    ad,getbyt(ad),getbyt(ad+1),getbyt(ad+2),getbyt(ad+3),getbyt(ad+4),
	    getbyt(ad+5),getbyt(ad+6),getbyt(ad+7));
#endif
	}
	// return length of opcode
	return olen;
}


