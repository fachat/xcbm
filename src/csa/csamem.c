
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"log.h"
#include 	"config.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"cpu.h"
#include 	"mem.h"
#include 	"csamem.h"
#include 	"petio.h"
#include 	"petvideo.h"
#include 	"mon.h"

#define	MAXLINE	200
	
/*******************************************************************
 * Simulated memory map:
 *
 * 00000-07fff 	32k SRAM (BIOS board)
 * 08000-0ffff	32k ROM (pre-filled with PET BASIC 4)
 *		I/O window at 0e800-0efff
 * 10000-1ffff	64k RAM (VDC board)
 * 80000-fffff	512k RAM (lowest 32k overlapp with 00000-07fff)
 *
 * Of the video RAM, after reset, the lowest 16k are available
 * to the CRTC, but can be changed with bits 0/1 in the VDC
 * control port at $e888
 *
 * The CPU address after reset is mapped 1:1 to the lowest
 * 64k of memory. After the first write to the MMU at I/O effx,
 * the mapping is determined by the MMU registers, one
 * for every 4k block. 
 *
 * At this time the control register at I/O efd0 (wprot/notmapped/noexec)
 * and $efd8/efd9 (read CPU address from external bus master) 
 * are ignored 
 */

#define		VRAM	0x8000
#define 	KERNEL	(MP_KERNEL*4096l)
#define 	BASIC	(MP_BASIC*4096l)
#define		EDITOR	(MP_EDIT*4096l)

#define		ROMLEN	0x08000
#define		VRAMLEN	0x10000
#define		RAMLEN	0x80000


/*******************************************************************/

static uchar rom[ROMLEN];
static uchar ram[RAMLEN];
static uchar vram[RAMLEN];

static int mmu_init = 0;	// on RESET the MMU goes 1:1 - after first write we actually map
static uchar mmu[16];		// MMU mapping (TODO: MMU ctrl bits go elsewhere)

static meminfo_t bus_info[CSAPAGES];

static bank_t physbank = {
	"bus",
	add_mem_trap,
	rm_mem_trap,
	bank_mem_peek,
	bank_mem_poke,
	bus_info,
	CSAPAGESMASK
};


/*******************************************************************/

void setmap(void) {
	int i;

	if (mmu_init == 0) {
		// not initialized yet
		for(i=0;i<16;i++) {
			cpumap[i].inf = &bus_info[i];

		}	
	} else {
		for(i=0;i<16;i++) {
			cpumap[i].inf = &bus_info[mmu[i]];

		}	
	}

	// I/O memory space 
	// note: limited to 4k page
	cpumap[14].mask = 0x0800;
	cpumap[14].comp = 0x0800;
	cpumap[14].m_wr = &io_wr;
	cpumap[14].m_rd = &io_rd;
	cpumap[14].m_peek = &io_peek;
}

/*******************************************************************/

void inimemvec(void){
	int i;
	for(i=0;i<CSAPAGES;i++) {
		bus_info[i].page=i;
		bus_info[i].mt_wr=NULL;
		bus_info[i].mt_rd=NULL;
		bus_info[i].traplist=NULL;
		bus_info[i].mf_wr=NULL;
		bus_info[i].mf_rd=NULL;
		bus_info[i].mf_peek=NULL;
	}
	/* low32k RAM */
	for(i=0;i<8;i++) {
	 	bus_info[i].mt_wr=ram+i*0x1000;
	 	bus_info[i].mt_rd=ram+i*0x1000;
	}

	/* next 32k (ROM) */
	for(i=8;i<16;i++) {
	 	bus_info[i].mt_rd=rom+(i-8)*0x1000;
	}

	/* video RAM */
	for(i=16;i<32;i++) {
	 	bus_info[i].mt_wr=vram+(i-16)*0x1000;
	 	bus_info[i].mt_rd=vram+(i-16)*0x1000;
		/* TODO: optimize so only the actual page is trapped */
	 	bus_info[i].mf_wr=vmem_wr;
	}

	/* upper 512k RAM */
	for(i=128;i<256;i++) {
	 	bus_info[i].mt_wr=ram+(i-128)*0x1000;
	 	bus_info[i].mt_rd=ram+(i-128)*0x1000;
	}

	/* the CPU map parts that may need to survive a setmap() */
	for(i=0;i<16;i++) {
                cpumap[i].mask = 0;
                cpumap[i].comp = 0;

                cpumap[i].traplist=NULL;
	}

	/* MMU */
	mmu_init = 0;
	for(i=0; i<16; i++) {
		mmu[i] = 0;
	}
	
	setmap();
}

/* ---------------------------------------------------------------*/

void mmu_wr(scnt addr, scnt val) {

	mmu_init = 1;
	
	mmu[addr & 0x0f] = val;

	setmap();
}

scnt mmu_rd(scnt addr) {
	return mmu[addr & 0x0f];
}

/* ---------------------------------------------------------------*/

static const char *names[] = { 
		"/var/lib/cbm/csa",
		"csa.rom"
};

static int mem_set_rom_dir(const char *param) {
	names[0] = param;
	return 0;
}

static int mem_set_rom(const char *param) {
	names[1] = param;
	return 0;
}

/**
 * TODO: load single CSA ROM instead
 */
static config_t mem_pars[] = {
	{ "rom-dir", 'd', "rom_directory", mem_set_rom_dir, "set common ROM directory (default = /var/lib/cbm/pet)" },
	{ "rom", 'R', "rom_filename", mem_set_rom, "set 32k ROM file name (in ROM directory; default 'pet.rom')" },
	{ NULL }
};


void mem_init() {
	config_register(mem_pars);

	mon_register_bank(&physbank);

	vmem_set(vram, 0xffff);
}

void mem_start() {
	char fname[MAXLINE];
	size_t offset[]={ 0, 0 };
	size_t len[]=   { 0, 32768 };
	int i;

	  for(i=1;i<2;i++) {
	    if(names[i][0]=='/') {
	      strcpy(fname,names[i]);
	    } else {
	      strcpy(fname,names[0]);
	      if(fname[strlen(fname)-1]!='/') strcat(fname,"/");
	      strcat(fname,names[i]);
	    }
	    loadrom(fname, rom+offset[i], len[i]);
	  }	
	  inimemvec();
	  return;
}

