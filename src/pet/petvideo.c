
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

int crtc_reg[16];
int videopage=0;

static const int width=80;
static const int scrlen=width*25;

/* video RAM (mirrors 0x8xxx of CPU pet bank 
 * ... or the 64k video dRAM on the CS/A CRTC board 
 * The values are set from the memory module
 */
static uchar *vram;
static scnt vrmask = 0;

/* screen (character memory) offset in VRAM 
 * This is set via CRTC or CS/A video control
 * register in crtc_wr()
 */
static scnt vrbase = 0;

/* current CRTC register (for register read/write) */
static scnt reg = 0;

void vmem_set(uchar *vramp, scnt mask) {
	vram = vramp;
	vrmask = mask;
}

static void wrvid(scnt a, scnt b){
	static chtype c;
	static int line, col;
	if(a<scrlen) {
		line=a/width;
		col =a%width;
//logout(0, "wrvid(%d,%d)-> %d, update=%d", line, col, b, update);
		c=b&0x7f;
		if(c<32)
			c+=96;
		if(b&0x80)
			c|=A_REVERSE;
		if(color) {
// NOTE: color makes stuff invisible on xcsa running GeckOS 1.3
//		 	c|=COLOR_PAIR(vram[(vrbase+a+0x800)&vrmask]);
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
	//val=vic_reg[24];
	//screenadr=videopage+1024*(val>>4);
	update=0;
	for(i=0;i<scrlen;i++) {
	  wrvid(i,vram[(vrbase+i)&vrmask]);
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
	//screenadr=0x8000;
//	crtc_wr(33,0);

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
	  wrvid(adr,vram[(vrbase+adr)&vrmask]);
	}
}

/* 
 * note that vmem_wr is called with a mapped (CPU) address.
 * Therefore we include meminfo_t to understand where in
 * vram physical space we actually are
 */
void vmem_wr(meminfo_t *inf, scnt addr,scnt val ) {
	scnt offset = addr & 0x0fff;

	// calculate physical address
	scnt phys = vrmask & (offset + ((inf->page & 0x0f) << 12));

	logout(2, "vmem_wr(addr=%04x, page=%d, -> offset=%04x, phys=%04x, vrbase=%04x", 
			addr, inf->page, offset, phys, vrbase);

	if (phys < vrbase || phys > vrbase + 2000) {
		return;
	}

	// this should have been done by setbyt() already
	//vram[(phys + offset) & vrmask] = val;

	//if (offset & 0x800) {
		// write col RAM
	//	colram_wr(offset, val);
	//} else {
		wrvid(phys - vrbase, val);
	//}
}


void crtc_wr(scnt xreg,scnt val ) {

	xreg &= 0x0f;
	if (xreg < 8) {
		// CRTC
		xreg &= 1;
		if (xreg == 0) {
			reg = val & 0x1f;
			return;
		}

		logout(0, "CRTC reg %d <- %02x", reg, val);

		// write to actual register
		switch(reg) {
		case 12:
			// display start address high (6 bit)
			vrbase = vrbase & 0xff | ((val & 0x3f) << 8);
			updatevideo();
			break;
		case 13:
			// display start address low (8 bit)
			vrbase = vrbase & 0xff00 | (val & 0xff);
			updatevideo();
			break;
		default:
			break;
		}
	} else {
		// CRTC board control register (TODO: CS/A only)
		// bits 0/1 are vrbase 14/15
		vrbase = vrbase & 0x3fff | ((val & 0x03) << 14);
	}
}

scnt crtc_rd(scnt xreg) {
#if 0
	return(xreg==18?0:vic_reg[xreg]);
#endif
	return 0;
}

