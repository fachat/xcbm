
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"log.h"
#include 	"config.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"emu6502.h"
#include 	"mem.h"
#include 	"upetmem.h"
#include 	"petio.h"
#include 	"petvideo.h"
#include 	"mon.h"
#include 	"spi.h"

/*******************************************************************/


static uchar fram[0x80000];	// 512k Fast RAM
static uchar vram[0x80000];	// 512k Video RAM

static int swap;		// if set, swap FRAM and VRAM
static int bank;		// where in fram is the low 32k mapped from?
static int vidblk;		// location of the video window in vram
static int wprot;		// write protect bits from $e801
static int vctrl;		// video control

static meminfo_t pet_info[UPETPAGES];

static bank_t rambank = {
	"ram",
	add_mem_trap,
	rm_mem_trap,
	bank_mem_peek,
	bank_mem_poke,
	pet_info,
	UPETPAGESMASK
};

/* 
 * initialize the CPU memory map, depending on the configuration chosen
 */
void setmap(void) {
	int i;

	int j=UPETPAGES/2;
	int k;

	logout(0, "set map (bank=%d, swap=%d)\n", bank, swap);

	for(i=0;i<UPETPAGES;i++) {

		if (i < j) {
			if (swap) {
				k = i + j;
			} else {
				if (i < 8) {
					k = (bank * 8) + i;
				} else {
					k = i;
				}
			}
		} else {
			if (swap) {
				if ((i-j) < 8) {
					k = (bank * 8) + i - j;
				} else {
					k = i - j;
				}
			} else {
				k = i;
			}
		}

		logout(0, "map page %02x to ram bank at %02x", i, k);

		cpumap[i].inf = &pet_info[k];

		// video window
		if (i == 8 && !(vctrl & 0x04)) {
			cpumap[i].mask = 0x0800;
			cpumap[i].comp = 0x0000;
			cpumap[i].m_wr = wrvid;
		}
		// mask/comp flags for I/O area
		if (i == 14) {
			// I/O memory space 
			// note: limited to 4k page
			cpumap[i].mask = 0x0f00;
			cpumap[i].comp = 0x0800;
			cpumap[i].m_wr = &io_wr;
			cpumap[i].m_rd = &io_rd;
			cpumap[i].m_peek = &io_peek;
		}
	}
}

void mem_set_vctrl(byte b) {
	vctrl = b;
	setmap();
}

void mem_set_bank(byte newbank) {
	bank = newbank & 0x0f;
	setmap();
}

void mem_set_map(byte newmap) {
	wprot = newmap & 0xf0;
	swap = newmap & 0x02;
	setmap();
}

void mem_set_vidblk(byte newblk) {
	vidblk = newblk;
	setmap();
}


/*******************************************************************/
/*
 * initialize the bank information that stays constant
 * and is then used by setmap() to set the actual CPU mapping
 */
void inimemvec(void){
	int i;

	int j = UPETPAGES/2;

	for(i=0; i<j; i++) {
		pet_info[i].page=i;
		pet_info[i].mt_wr=fram+i*4096;
		pet_info[i].mt_rd=fram+i*4096;
		pet_info[i].traplist=NULL;
		pet_info[i].mf_wr=NULL;
		pet_info[i].mf_rd=NULL;
		pet_info[i].mf_peek=NULL;
	}
	for(i=j; i<UPETPAGES; i++) {
		pet_info[i].page=i;
		pet_info[i].mt_wr=vram+(i-j)*4096;
		pet_info[i].mt_rd=vram+(i-j)*4096;
		pet_info[i].traplist=NULL;
		pet_info[i].mf_wr=NULL;
		pet_info[i].mf_rd=NULL;
		pet_info[i].mf_peek=NULL;
	}

	/* the CPU map parts that may need to survive a setmap() */
	for(i=0;i<UPETPAGES;i++) {
		cpumap[i].mask = 0;
		cpumap[i].comp = 0;

		cpumap[i].traplist=NULL;
	}
	setmap();
}

void mem_init() {
	//config_register(mem_pars);

	mon_register_bank(&rambank);

	bank = 0;

	vmem_set(vram + 0x9000, 0x0fff);
}

void mem_start() {
	inimemvec();

	mem_set_map(0x02);	// reset state

	spi_ipl(vram+0xff00);

	return;
}

