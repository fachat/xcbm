
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"
#include "ccurses.h"
#include "mem.h"
#include "petmem.h"
#include "petvideo.h"
#include "petio.h"

/***********************************************************************/
/* PET video manipulation routines                                     */
/* Note. we're already implementing the Colour-PET variant             */
/***********************************************************************/

#include "video.h"

int screenadr=0x8000;
int crtc_reg[16];
int videopage=0;

static const int width=80;
static const int scrlen=width*25;

void wrvid(scnt a, scnt b){
	static chtype c;
	static int line, col;
	if(a>=screenadr && a<screenadr+scrlen) {
		a-=screenadr;
		line=a/width;
		col =a%width;
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
	int i;
	setwr(MP_VRAM,NULL);
	//val=vic_reg[24];
	//screenadr=videopage+1024*(val>>4);
	setwr(MP_VRAM,vmem_wr);
	update=0;
	for(i=screenadr;i<screenadr+scrlen;i++) {
	  wrvid(i,getvbyt(i));
	}
	update=1;
/*	touchwin(scr);*/
	refresh();
}

void set_vdrive_cb(struct alarm_s *alarm, CLOCK current);
void clr_vdrive_cb(struct alarm_s *alarm, CLOCK current);

alarm_t set_vdrive = {
	"Set VDRIVE",
	NULL,
	set_vdrive_cb,
	NULL,
	CLOCK_MAX
};

alarm_t clr_vdrive = {
	"Clear VDRIVE",
	NULL,
	clr_vdrive_cb,
	NULL,
	CLOCK_MAX
};

void set_vdrive_cb(struct alarm_s *alarm, CLOCK current) {
	io_set_vdrive(1);
	set_alarm_clock_plus(&set_vdrive, ((BUS*)set_vdrive.data)->cyclesperframe);
}

void clr_vdrive_cb(struct alarm_s *alarm, CLOCK current) {
	io_set_vdrive(0);
	set_alarm_clock_plus(&clr_vdrive, ((BUS*)clr_vdrive.data)->cyclesperframe);
}


	
int video_init(CPU *cpu){
	int i;
	update=0;
	for(i=0;i<16;i++) crtc_reg[i]=0;
	screenadr=0x8000;
//	crtc_wr(33,0);

	setwr(MP_VRAM, vmem_wr);

	// 128 cycles VDRIVE pulse
	set_vdrive.data = cpu->bus;
	alarm_register(&cpu->bus->actx, &set_vdrive);
	set_alarm_clock(&set_vdrive, 0);
	clr_vdrive.data = cpu->bus;
	alarm_register(&cpu->bus->actx, &clr_vdrive);
	set_alarm_clock(&clr_vdrive, 128);

	updatevideo();
	return(0);
}

static inline void colram_wr(scnt adr, scnt val) {
	if(color && adr<1000) {
	  wrvid(adr+screenadr,getvbyt(adr+screenadr));
	}
}

void vmem_wr(scnt addr,scnt val ) {
	register scnt a = addr & 0x0fff;

	if (a & 0x800) {
		// write col RAM
		colram_wr(addr, val);
	} else {
		wrvid(addr, val);
	}
}


void crtc_wr(scnt xreg,scnt val ) {
#if 0
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
#endif
}

scnt crtc_rd(scnt xreg) {
#if 0
	return(xreg==18?0:vic_reg[xreg]);
#endif
	return 0;
}

