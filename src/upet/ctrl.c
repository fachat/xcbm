
#include "types.h"

#include "ctrl.h"
#include "upetmem.h"

// TODO: all the following due to meminfo_t in upetvideo.h. Needs optimization
#include <stdio.h>
#include "log.h"
#include "alarm.h"
#include "bus.h"
#include "emu65816.h"
#include "mem.h"
#include "upetvideo.h"

/* read-back values */

static byte r0_vid;
static byte r1_memmap;
static byte r2_bank;
static byte r3_speed;
static byte r4_vidblk;

/* write control ports e800-e807 */
void ctrl_wr(scnt adr, scnt val) {

	switch (adr & 0x0f) {
	case 0:
		r0_vid = val & 0xdf;
		mem_set_vctrl(r0_vid);
		vset_width((r0_vid & 0x02) ? 80 : 40);
		break;
	case 1:
		r1_memmap = val & 0xfb;
		mem_set_map(r1_memmap);
		break;
	case 2:
		r2_bank = val & 0x0f;
		mem_set_bank(r2_bank);
		break;
	case 3:
		r3_speed = val & 0x03;
		// TODO: set max speed
		break;
	case 4: 
		r4_vidblk = val & 0x07;
		mem_set_vidblk(r4_vidblk);
		break;
	default:
		break;
	}
}


/* read control ports e800-e807 */
scnt ctr_rd(scnt adr) {

	switch (adr & 0x0f) {
	case 0:
		return r0_vid;
	case 1:
		return r1_memmap;
	case 2:
		return r2_bank;
	case 3:
		return r3_speed;
	case 4:
		return r4_vidblk;
	default:
		return 0;
	}
}


