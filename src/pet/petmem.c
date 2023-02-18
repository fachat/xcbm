
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

#define	MAXLINE	200
	
#define		VRAM	0x8000
#define 	KERNEL	(MP_KERNEL*4096l)
#define 	BASIC	(MP_BASIC*4096l)
#define		EDITOR	(MP_EDIT*4096l)

#define		MEMLEN	0x10000

/*******************************************************************/

meminfo memtab[MP_NUM];

uchar *colram = NULL;

void setmap(void) {
	int i;

	for(i=0;i<16;i++) {
		updatemw(i,i);
		updatemr(i,i);
	}
}


/*******************************************************************/

void inimemvec(void){
	int i;
	for(i=0;i<MP_NUM;i++) {
		memtab[i].mt_wr=NULL;
		memtab[i].mt_rd=NULL;
		memtab[i].ntraps=0;
		memtab[i].mf_wr=NULL;
		memtab[i].mf_rd=NULL;
	}
	/* RAM (including VRAM) */
	for(i=MP_RAM0;i<MP_RAM0+9;i++) {
	 	memtab[i].mt_wr=mem+i*0x1000;
	 	memtab[i].mt_rd=mem+i*0x1000;
	}
	/* video + color RAM at $8800 */
	colram = memtab[MP_VRAM].mt_wr + 0x0800;

	/* KERNEL */
	memtab[MP_KERNEL].mt_rd = mem+KERNEL;
	/* BASIC */
	memtab[MP_BASIC].mt_rd = mem+BASIC;
	memtab[MP_BASIC+1].mt_rd = mem+BASIC+0x1000;
	memtab[MP_BASIC+2].mt_rd = mem+BASIC+0x2000;
	/* EDITOR */
	memtab[MP_EDIT].mt_rd = mem+EDITOR;

	for(i=0;i<16;i++) {
		// video memory address */
		if(i==8) {
			m[i].vr=mem+VRAM;
		} else {
			m[i].vr=NULL;
		}
		// current page
		m[i].wr=m[i].rd=-1;
		// mask/comp flags
		if (i == 14) {
			// I/O memory space
			m[i].mask = 0xff00;
			m[i].comp = 0xe800;
			m[i].m_wr = &io_wr;
			m[i].m_rd = &io_rd;
		} else {
			m[i].mask = 0;
		}
	}
	setmap();
}

/* ---------------------------------------------------------------*/

static const char *names[] = { 
		"/var/lib/cbm",
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
	{ "rom-dir", 'd', "rom_directory", mem_set_rom_dir, "set common ROM directory (default = /var/lib/cbm)" },
	{ "kernal-rom", 'K', "kernal_rom_filename", mem_set_kernal, "set kernal ROM file name (in ROM directory; default 'petkernal4.rom')" },
	{ "basic-rom", 'B', "basic_rom_filename", mem_set_basic, "set basic ROM file name (in ROM directory; default 'petbasic4.rom')" },
	{ "edit-rom", 'E', "edit_rom_filename", mem_set_edit, "set edit ROM file name (in ROM directory; default 'petedit4.rom')" },
	{ NULL }
};


void mem_init() {
	config_register(mem_pars);
}

void mem_start() {
	char fname[MAXLINE];
	size_t offset[]={ 0, KERNEL, BASIC, EDITOR };
	size_t len[]=   { 0, 4096,   3*4096,  2048 };
	int i;
	mem=malloc(MEMLEN);
	if(mem){
	  atexit(mem_exit);
	  for(i=1;i<4;i++) {
	    if(names[i][0]=='/') {
	      strcpy(fname,names[i]);
	    } else {
	      strcpy(fname,names[0]);
	      if(fname[strlen(fname)-1]!='/') strcat(fname,"/");
	      strcat(fname,names[i]);
	    }
	    loadrom(fname, offset[i], len[i]);
	  }	
	  inimemvec();
	  return;
	}
	exit(1);
}

