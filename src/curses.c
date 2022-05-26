
#include <stdlib.h>

#include "log.h"
#include "ccurses.h"
#include "emu65.h"
#include "mem.h"

WINDOW 	*scr;
int update;

typedef struct rgb_t {
		int	r;
		int	g;
		int 	b;
		int	fg;	/* if only the 8 std colors are available */
} rgb_t;

rgb_t rgb[16]={ { 0,    0,    0,	0 },	/* black */
		{ 1000, 1000, 1000,	7 },	/* white */ 
		{ 1000, 0,    0,	1 },	/* red */
		{ 0,    0,    0,	5 },	/* Tuerkis */
		{ 500,  0,  500,	6 },	/* Violett */
		{ 0,    1000, 0,	2 },	/* green */
		{ 0,    0,    1000,	4 },	/* blue */
		{ 0,    0,    0,	3 },	/* yellow */
		{ 0,    0,    0,	0 },	/* orange */
		{ 0,    0,    0,	0 },	/* brown */
		{ 1000,  200,  200,	0 },	/* bright red */
		{ 250,   250,  250,	0 },	/* grey 1 */
		{ 500,   500,  500,	0 },	/* grey 2 */
		{ 200,  1000,  200,	0 },	/* bright green */
		{ 200,   200, 1000,	6 },	/* bright blue */
		{ 750,   750,  750,	0 } };	/* grey 3 */
 
void cur_exit(void);

int cur_init(void) {

	scr =initscr();

	cbreak();	/* may be changed to raw() someday */
	noecho();
	leaveok(scr, TRUE);
	scrollok(scr, FALSE);
	nonl();
	keypad(scr,TRUE);
/*
	notimeout(scr,TRUE);
*/
	meta(scr,TRUE);
	nodelay(scr,TRUE);

	if(color>=0 && has_colors()) {
	  int i;
	  logout(1,"Terminal reports color capability...");
	  color=1;
	  if(can_change_color()) {
	    logout(1,"Terminal can also change colors...");
	    color=2;
	  }
	  start_color();
	  logout(2,"Terminal has %d possible colors and %d color pairs",
			COLORS,COLOR_PAIRS);
	  if(color==2 && COLORS>=16) {
	    for(i=0;i<16;i++) {
	      init_color(i,rgb[i].r,rgb[i].g,rgb[i].b); 
	    }
	    color=3;
	  }
	} else 
	if (color < 0 && has_colors()) {
	} else {
	  color=0;
	}
	atexit(cur_exit);
	return(0);
}

void cur_exit(void) {
	nocbreak();
	endwin();
}

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

void wrvid(scnt a, scnt b){
	static chtype c;
	static int line, col;
	if(a>=screenadr && a<screenadr+1000) {
		a-=screenadr;
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
	setwr(MP_RAM0+(screenadr>>12),NULL);
	screenadr=videopage+1024*(val>>4);
	charadr=videopage+1024*(val&0x0e);
	hiresadr=videopage+1024*(val&0x08);
logout(0,"set videoaddresses: screen=%x, char=%x, hires=%x",
			screenadr, charadr, hiresadr);
	setwr(MP_RAM0+(screenadr>>12),wrvid);
	update=0;
	for(i=screenadr;i<screenadr+1000;i++) {
	  wrvid(i,getvbyt(i));
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


int video_init(){
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
	  wrvid(adr+screenadr,getvbyt(adr+screenadr));
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
	
