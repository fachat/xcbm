
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
#include 	"petmem.h"
#include 	"petio.h"
#include 	"petvideo.h"
#include 	"mon.h"

#define	MAXLINE	200
	
#define		VRAM	0x8000
#define 	KERNEL	(MP_KERNEL*PAGESIZE)
#define 	BASIC	(MP_BASIC*PAGESIZE)
#define		EDITOR	(MP_EDIT*PAGESIZE)
#define		ROM9	(MP_ROM9*PAGESIZE)
#define		ROMA	(MP_ROMA*PAGESIZE)

#define		MEMLEN	0x10000

/*******************************************************************/


static uchar ram[0x09000];	// 32k main RAM + 4k video RAM
static uchar rom[0x07000];	// 4k ROM9, 4k ROMA, 12k BASIC, 4k EDIT, 4k KERNEL

uchar *colram = NULL;

/* in prep for extra 8296 memory banks */
/* includes 32k RAM, 4k Video, and all ROM + I/O */

static meminfo_t pet_info[16];

static bank_t petbank = {
	"pet",
	add_mem_trap,
	rm_mem_trap,
	pet_info
};

/* 
 * initialize the CPU memory map, depending on the configuration chosen
 */
void setmap(void) {
	int i;

	for(i=0;i<16;i++) {
		cpumap[i].inf = &pet_info[i];

		// mask/comp flags for I/O area
		if (i == 14) {
			// I/O memory space 
			// note: limited to 4k page
			cpumap[i].mask = 0x0f00;
			cpumap[i].comp = 0x0800;
			cpumap[i].m_wr = &io_wr;
			cpumap[i].m_rd = &io_rd;
		}
	}
}


/*******************************************************************/
/*
 * initialize the bank information that stays constant
 * and is then used by setmap() to set the actual CPU mapping
 */
void inimemvec(void){
	int i;
	for(i=0;i<16;i++) {
		pet_info[i].mt_wr=NULL;
		pet_info[i].mt_rd=NULL;
		pet_info[i].traplist=malloc(PAGESIZE * sizeof(trap_t*));
		pet_info[i].mf_wr=NULL;
		pet_info[i].mf_rd=NULL;
		memset(pet_info[i].traplist, 0, PAGESIZE * sizeof(trap_t*));
	}

	/* RAM (including VRAM) */
	for(i=MP_RAM0;i<=MP_VRAM;i++) {
	 	pet_info[i].mt_wr=ram+i*0x1000;
	 	pet_info[i].mt_rd=ram+i*0x1000;
	}
	/* video + color RAM at $8800 */
	pet_info[MP_VRAM].mf_wr = vmem_wr;

	/* KERNEL */
	pet_info[MP_ROM_OFFSET+MP_KERNEL].mt_rd = rom+KERNEL;
	/* BASIC */
	pet_info[MP_ROM_OFFSET+MP_BASIC].mt_rd = rom+BASIC;
	pet_info[MP_ROM_OFFSET+MP_BASIC+1].mt_rd = rom+BASIC+0x1000;
	pet_info[MP_ROM_OFFSET+MP_BASIC+2].mt_rd = rom+BASIC+0x2000;
	/* EDITOR */
	pet_info[MP_ROM_OFFSET+MP_EDIT].mt_rd = rom+EDITOR;

	/* the CPU map parts that may need to survive a setmap() */
	for(i=0;i<16;i++) {
		cpumap[i].mask = 0;
		cpumap[i].comp = 0;

		cpumap[i].traplist=malloc(PAGESIZE * sizeof(trap_t*));
		memset(pet_info[i].traplist, 0, PAGESIZE * sizeof(trap_t*));
	}
	setmap();
}

/* ---------------------------------------------------------------*/

static const char *names[] = { 
		"/var/lib/cbm/pet",
		"petkernel4.rom",
		"petbasic4.rom",
		"petedit4.rom"
};

static int mem_set_rom_dir(const char *param) {
	names[0] = param;
	return 0;
}

static int mem_set_kernal(const char *param) {
	names[1] = param;
	return 0;
}

static int mem_set_basic(const char *param) {
	names[2] = param;
	return 0;
}

static int mem_set_edit(const char *param) {
	names[3] = param;
	return 0;
}

static config_t mem_pars[] = {
	{ "rom-dir", 'd', "rom_directory", mem_set_rom_dir, "set common ROM directory (default = /var/lib/cbm/pet)" },
	{ "kernal-rom", 'K', "kernal_rom_filename", mem_set_kernal, "set kernal ROM file name (in ROM directory; default 'petkernal4.rom')" },
	{ "basic-rom", 'B', "basic_rom_filename", mem_set_basic, "set basic ROM file name (in ROM directory; default 'petbasic4.rom')" },
	{ "edit-rom", 'E', "edit_rom_filename", mem_set_edit, "set edit ROM file name (in ROM directory; default 'petedit4.rom')" },
	{ NULL }
};


void mem_init() {
	config_register(mem_pars);

	mon_register_bank(&petbank);
}

void mem_start() {
	char fname[MAXLINE];
	size_t offset[]={ 0, KERNEL, BASIC, EDITOR };
	size_t len[]=   { 0, 4096,   3*4096,  2048 };
	int i;

	  for(i=1;i<4;i++) {
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

