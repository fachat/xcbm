
#include <stdlib.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "cpu.h"
#include "ccurses.h"
#include "mem.h"
#include "mem64.h"

/***********************************************************************/
/* c64 video manipulation routines                                     */
/***********************************************************************/

#include "video.h"

uchar colram[1024];
int screenadr=0;
int vic_reg[48];
int videopage=0;
int hiresadr=0;
int charadr=0;

void wrvid(meminfo_t *inf, scnt a, scnt b){
	static chtype c;
	static int line, col;

	// may be called with page offset, or with absolute address
	a &= 0xfff;

	if(a>=(screenadr & 0xfff) && a<(screenadr & 0xfff)+1000) {
		a-=(screenadr & 0xfff);
		line=a/40;
		col =a%40;
		c=b&0x7f;
		if(c<32)
			c+=96;
		if(b&0x80)
			c|=A_REVERSE;
		if(color) {
		 	c|=COLOR_PAIR(colram[a]);
		}

		mvaddch(line,col,c);
		if(update)
			refresh();
/*
		move(line,col);
		echochar(c);
*/
	}
}

void updatevideo(void) {
	int i, val=vic_reg[24];

	// clear current
	mem_set_vaddr((screenadr>>12),NULL);

	screenadr=videopage+1024*(val>>4);
	charadr=videopage+1024*(val&0x0e);
	hiresadr=videopage+1024*(val&0x08);
logout(0,"set videoaddresses: screen=%x, char=%x, hires=%x",
			screenadr, charadr, hiresadr);

	// set new video address in ram bank
	mem_set_vaddr((screenadr>>12),wrvid);

	update=0;
	for(i=screenadr;i<screenadr+1000;i++) {
	  wrvid(NULL, i,mem_getvbyt(i));
	}
	update=1;
/*	touchwin(scr);*/
	refresh();
}
	
void setvideopage(scnt reg) {
	videopage=(reg&0x3)<<14;
logout(0,"videopage=%x",videopage);
	updatevideo();
}


int video_init(BUS *bus, int cyclesperscreen){
	int i;
	update=0;
	for(i=0;i<48;i++) vic_reg[i]=0;
	screenadr=0;
	setvideopage(3);
	video_wr(33,0);
	return(0);
}

void colram_wr(scnt adr, scnt val) {
	colram[adr]=val&0x0f;

	if(color && adr<1000) {
	  wrvid(NULL, adr+screenadr,mem_getvbyt(adr+screenadr));
	}
}

void video_wr(scnt xreg,scnt val ) {
	int i;
	vic_reg[xreg]=val;
	switch(xreg) {
	case 24:
		updatevideo();
		break;
	case 33:	/* background color 0 */
		if(color) {
		    for(i=0;i<16;i++) {
			init_pair(i,rgb[i].fg,rgb[val&0x0f].fg);
	logout(0,"set color: %d -> fg=%d, bg=%d",i,rgb[i].fg,rgb[val&0x0f].fg);
		    }
		    updatevideo();
		}
		break;
	default:
		break;
	}
}

scnt video_rd(scnt xreg) {
	return(xreg==18?0:vic_reg[xreg]);
}
	
