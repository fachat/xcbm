
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"log.h"
#include	"config.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"emu6502.h"
#include	"emu64.h"
#include 	"mem.h"
#include 	"mem64.h"

int	hiram;
int	loram;
int	charen;

static char *ram[0x10000];	// 64k RAM
static char *rom[0x9000];	// 36 ROM (8k Kernal, 8k BASIC, 4k charrom, 8k loram, 8k hiram)

static meminfo_t ram_info[16];

static bank_t rambank = {
	"ram",
	add_mem_trap,
	rm_mem_trap,
	ram_info
};

static meminfo_t rom_info[16];

static bank_t rombank = {
	"rom",
	add_mem_trap,
	rm_mem_trap,
	rom_info
};

static meminfo_t io_info[16];

static bank_t iobank = {
	"io",
	add_mem_trap,
	rm_mem_trap,
	io_info
};

static meminfo_t cart_info[16];

static bank_t cartbank = {
	"cart",
	add_mem_trap,
	rm_mem_trap,
	cart_info
};


#define	seekernel()	(hiram)
#define	seebasic()	(hiram&&loram)
#define	seeroml()	0
#define	seeromh()	0
#define	seechar()	((!charen)&&(hiram||loram))
#define	seeio()		((charen)&&(hiram||loram))
	

#define 	KERNEL	(MP_KERNEL0 * 4096)
#define 	BASIC	(MP_BASIC0 * 4096)
#define		CHAROM	(MP_CHAROM * 4096)
#define		ROML	(MP_ROML0 * 4096)
#define		ROMH	(MP_ROMH0 * 4096)

/*******************************************************************/

void mem_setcpuport(scnt adr, scnt byt);
scnt mem_getcpuport(scnt adr);

int reg0,dir0;

/*******************************************************************/

void setmap(void) {
	int i,p;

	p=reg0|(~dir0);
	loram=p&0x01;
	hiram=p&0x02;
	charen=p&0x04;

logout(0,"set loram=%d, hiram=%d, charen=%d",loram,hiram,charen);
	
	for(i=0;i<16;i++) {
		if(i==13 && seeio()) 
			updatemw(i,MP_IO64);
		else 
			updatemw(i,i);
	}

	for(i=0;i<8;i++) {
		updatemr(i,i);
	}
	if(seeroml()) {
		updatemr(8,MP_ROML0);
		updatemr(9,MP_ROML1);
	} else {
		updatemr(8,8);
		updatemr(9,9);
	}
	if(seebasic()) {
		updatemr(10,MP_BASIC0);
		updatemr(11,MP_BASIC1);
	} else if(seeromh()) {
		updatemr(10,MP_ROMH0);
		updatemr(11,MP_ROMH1);
	} else {
		updatemr(10,10);
		updatemr(11,11);
	}
	updatemr(12,12);
	if(seeio()) {
		updatemr(13,MP_IO64);
	} else if(seechar()) {
		updatemr(13,MP_CHAROM);
	} else { 
		updatemr(13,13);
	}
	if(seekernel()) {
		updatemr(14,MP_KERNEL0);
		updatemr(15,MP_KERNEL1);
	} else {
		updatemr(14,14);
		updatemr(15,15);
	}	
}

void mem_setcpuport(scnt adr, scnt byt) {

	//logout(0, "mem_setcpuport(a=%04x, b=%02x)", adr, byt);
	if(adr&1) {
		if(byt==reg0)
			return;
		reg0=byt;
	} else {
		if(byt==dir0)
			return;
		dir0=byt;
	}
	setmap();
}

scnt mem_getcpuport(scnt adr) {
	if(adr&1) {
		return reg0;
	}
	return dir0;
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
	/* 64k RAM */
	for(i=MP_RAM0;i<MP_RAM0+16;i++) {
	 	memtab[i].mt_wr=mem+i*0x1000;
	 	memtab[i].mt_rd=mem+i*0x1000;
	}
	/* KERNEL */
	memtab[MP_KERNEL0].mt_rd = mem+KERNEL;
	memtab[MP_KERNEL1].mt_rd = mem+KERNEL+0x1000;
	/* BASIC */
	memtab[MP_BASIC0].mt_rd = mem+BASIC;
	memtab[MP_BASIC1].mt_rd = mem+BASIC+0x1000;
	/* CHAROM */
	memtab[MP_CHAROM].mt_rd = mem+CHAROM;
	/* ROML */
	memtab[MP_ROML0].mt_rd = mem+ROML;
	memtab[MP_ROML1].mt_rd = mem+ROML+0x1000;
	/* ROMH */
	memtab[MP_ROMH0].mt_rd = mem+ROMH;
	memtab[MP_ROMH1].mt_rd = mem+ROMH+0x1000;

	/* CPU virtual address space */
	for(i=0;i<16;i++) {
		/* video memory address */
		if((i&7)==1) {
			m[i].vr=mem+CHAROM;
		} else {
			m[i].vr=mem+i*0x1000;
		}
		/* current page */
		m[i].wr=m[i].rd=-1;
		/* mask/comp flag */
		if (i == 0) {
			/* CPU registers 0/1 */
			m[i].mask = 0xfffe;
			m[i].comp = 0x0000;
			m[i].m_wr = &mem_setcpuport;
			m[i].m_rd = &mem_getcpuport;
		} else {
			m[i].mask = 0;
		}
	}
	reg0=dir0=0;
	setmap();
}

/* -------------------------------------------------------------*/

static const char *names[] = {
                "/var/lib/cbm/c64",
                "c64kernl.rom",
                "c64basic.rom",
                "c64chars.rom",
                "c64romh.rom",
                "c64roml.rom"
};

static int exrom = 0;
static int games = 0;

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

static int mem_set_charom(const char *param) {
        names[3] = param;
        return 0;
}

static int mem_set_romh(const char *param) {
        names[4] = param;
        return 0;
}

static int mem_set_roml(const char *param) {
        names[5] = param;
        return 0;
}

static int mem_toggle_games(const char *p) {
	games = !games;
	return 0;
}

static int mem_toggle_exrom(const char *p) {
	exrom = !exrom;
	return 0;
}

static config_t mem_pars[] = {
	{ "exrom", 'E', NULL, mem_toggle_exrom, "Toggle EXROM line (default off)" },
	{ "games", 'G', NULL, mem_toggle_games, "Toggle GAMES line (default off)" },
	{ "rom-dir", 'd', "rom_directory", mem_set_rom_dir, "set common ROM directory (default = /var/lib/cbm/c64)" },
        { "kernal-rom", 'K', "kernal_rom_filename", mem_set_kernal, "set kernal ROM file name (in ROM directory; default 'c64kernl.rom')" },
        { "basic-rom", 'B', "basic_rom_filename", mem_set_basic, "set basic ROM file name (in ROM directory; default 'c64basic.rom')" },
        { "charom", 'C', "character_rom_filename", mem_set_charom, "set charactor ROM file name (in ROM directory; default 'c64chars.rom')" },
        { "roml", 'L', "roml_filename", mem_set_roml, "set ROM file for lomem (in ROM directory; default 'c64roml.rom')" },
        { "romh", 'H', "romh_filename", mem_set_romh, "set ROM file for himem (in ROM directory; default 'c64romh.rom')" },
	{ NULL }
};

void mem_init() {
	config_register(mem_pars);

	memset(rominfo, 0, sizeof(rominfo));
	memset(raminfo, 0, sizeof(rominfo));
	memset(ioinfo, 0, sizeof(rominfo));
	memset(cartinfo, 0, sizeof(rominfo));

	// see also https://codebase64.org/doku.php?id=base:memory_management

	// set RAM bank
	for (int i = 0; i < 16; i++) {
		raminfo[i].mt_rd = &rom[i * 0x1000];
		raminfo[i].mt_wr = &rom[i * 0x1000];
	}

	// set ROM bank
	// kernal
	rominfo[14].mt_rd = &rom[KERNAL];
	rominfo[15].mt_rd = &rom[KERNAL + 0x1000];
	// BASIC
	rominfo[10].mt_rd = &rom[BASIC];
	rominfo[11].mt_rd = &rom[BASIC + 0x1000];
	// charrom
	rominfo[13].mr_rd = &rom[CHAROM];

	// set Cartridge bank
	// ROML
	cartinfo[8].mt_rd = &rom[ROML];
	cartinfo[9].mt_rd = &rom[ROML + 0x1000];
	// ROMH
	cartinfo[10].mt_rd = &rom[ROMH];
	cartinfo[11].mt_rd = &rom[ROMH + 0x1000];
}


void mem_start(){
	char fname[MAXLINE];
	size_t offset[]={ 0, KERNEL, BASIC, CHAROM, ROMH, ROML };
	size_t len[]=   { 0, 8192,   8192,  4096,   8192, 8192 };
	int i;
	loram=hiram=charen=1;
	if(mem){
	  atexit(mem_exit);
	  for(i=1;i<6;i++) {
	    if(names[i][0]=='/') {
	      strcpy(fname,names[i]);
	    } else {
	      strcpy(fname,names[0]);
	      if(fname[strlen(fname)-1]!='/') strcat(fname,"/");
	      strcat(fname,names[i]);
	    }
	    loadrom(fname, rom + offset[i], len[i]);
	  }	
	  inimemvec();
	  return;
	}
	exit(1);
}

