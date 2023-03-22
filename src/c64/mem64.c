
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
#include 	"c64io.h"
#include 	"mon.h"

int	hiram;
int	loram;
int	charen;

static uchar ram[0x10000];	// 64k RAM
static uchar rom[0x9000];	// 36 ROM (8k Kernal, 8k BASIC, 4k charrom, 8k loram, 8k hiram)

static meminfo_t ram_info[16];

static bank_t rambank = {
	"ram",
	add_mem_trap,
	rm_mem_trap,
	bank_mem_peek,
	bank_mem_poke,
	ram_info,
	PAGESMASK
};

static meminfo_t rom_info[16];

static bank_t rombank = {
	"rom",
	add_mem_trap,
	rm_mem_trap,
	bank_mem_peek,
	bank_mem_poke,
	rom_info,
	PAGESMASK
};

static meminfo_t io_info[16];

static bank_t iobank = {
	"io",
	add_mem_trap,
	rm_mem_trap,
	bank_mem_peek,
	bank_mem_poke,
	io_info,
	PAGESMASK
};

static meminfo_t cart_info[16];

static bank_t cartbank = {
	"cart",
	add_mem_trap,
	rm_mem_trap,
	bank_mem_peek,
	bank_mem_poke,
	cart_info,
	PAGESMASK
};


#define	seekernel()	(hiram)
#define	seebasic()	(hiram&&loram)
#define	seeroml()	0
#define	seeromh()	0
#define	seechar()	((!charen)&&(hiram||loram))
#define	seeio()		((charen)&&(hiram||loram))
	
/* offset in rom[] */
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
// see also https://codebase64.org/doku.php?id=base:memory_management

void setmap(void) {
	int p;

	p=reg0|(~dir0);
	loram=p&0x01;
	hiram=p&0x02;
	charen=p&0x04;

logout(0,"set loram=%d, hiram=%d, charen=%d",loram,hiram,charen);

	// $8000-$9fff
	if (seeroml()) {
		cpumap[8].inf = &cart_info[8];
		cpumap[9].inf = &cart_info[9];
	} else {
		cpumap[8].inf = &ram_info[8];
		cpumap[9].inf = &ram_info[9];
	}

	// $a000-$bfff
	if(seebasic()) {
		cpumap[10].inf = &rom_info[10];
		cpumap[11].inf = &rom_info[11];
	} else if(seeromh()) {
		cpumap[10].inf = &cart_info[10];
		cpumap[11].inf = &cart_info[11];
	} else {
		cpumap[10].inf = &ram_info[10];
		cpumap[11].inf = &ram_info[11];
	}

	/* $dxxx */
	if(seeio()) {
		cpumap[13].inf = &io_info[13];
	} else if(seechar()) {
		cpumap[13].inf = &rom_info[13];
	} else { 
		cpumap[13].inf = &ram_info[13];
	}

	if(seekernel()) {
		cpumap[14].inf = &rom_info[14];
		cpumap[15].inf = &rom_info[15];
	} else {
		cpumap[14].inf = &ram_info[14];
		cpumap[15].inf = &ram_info[15];
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

void rom_settrap(scnt addr, void (*trapfunc)(CPU *cpu, scnt addr), const char *name) {

	add_mem_trap(&rombank, addr, trapfunc, name);
}

/* set video character memory address callback */
void mem_set_vaddr(scnt addr, void (*wrvid)(scnt addr, scnt val)) {

	int page = addr >> 12;

	ram_info[page].mf_wr = wrvid;
}

scnt mem_getvbyt(scnt addr) {

	int page = addr >> 12;
	int offset = addr & 0xfff;

	return ram_info[page].mt_rd[offset];
}


/*******************************************************************/
/*
 * configure the banks, as well as the initial parts of the CPU memory map,
 * that needs to be kept (if changed) across setmap() calls
 */

void inimemvec(void){
	int i;
	for(i=0;i<PAGES;i++) {
		ram_info[i].page=i;
		ram_info[i].mt_wr=NULL;
		ram_info[i].mt_rd=NULL;
		ram_info[i].mf_wr=NULL;
		ram_info[i].mf_rd=NULL;
		ram_info[i].mf_peek=NULL;
		ram_info[i].traplist=NULL;

		rom_info[i].page=i;
		rom_info[i].mt_wr=NULL;
		rom_info[i].mt_rd=NULL;
		rom_info[i].mf_wr=NULL;
		rom_info[i].mf_rd=NULL;
		rom_info[i].mf_peek=NULL;
		rom_info[i].traplist=NULL;

		io_info[i].page=i;
		io_info[i].mt_wr=NULL;
		io_info[i].mt_rd=NULL;
		io_info[i].mf_wr=NULL;
		io_info[i].mf_rd=NULL;
		io_info[i].mf_peek=NULL;
		io_info[i].traplist=NULL;

		cart_info[i].page=i;
		cart_info[i].mt_wr=NULL;
		cart_info[i].mt_rd=NULL;
		cart_info[i].mf_wr=NULL;
		cart_info[i].mf_rd=NULL;
		cart_info[i].mf_peek=NULL;
		cart_info[i].traplist=NULL;
	}
	/* 64k RAM */
	for(i=MP_RAM0;i<MP_RAM0+16;i++) {
	 	ram_info[i].mt_wr=ram+i*0x1000;
	 	ram_info[i].mt_rd=ram+i*0x1000;
	}

	/* should ROM be modifyable in the monitor? */
	/* anyway, CPU writes go to RAM below ROM */

	/* KERNEL */
	rom_info[14].mt_rd = rom+KERNEL;
	rom_info[14].mt_wr = ram+KERNEL;
	rom_info[15].mt_rd = rom+KERNEL+0x1000;
	rom_info[15].mt_wr = ram+KERNEL+0x1000;
	/* BASIC */
	rom_info[10].mt_rd = rom+BASIC;
	rom_info[10].mt_wr = ram+BASIC;
	rom_info[11].mt_rd = rom+BASIC+0x1000;
	rom_info[11].mt_wr = ram+BASIC+0x1000;
	/* CHAROM */
	rom_info[13].mt_rd = rom+CHAROM;
	rom_info[13].mt_wr = ram+CHAROM;
	/* ROML */
	cart_info[8].mt_rd = rom+ROML;
	cart_info[8].mt_wr = ram+ROML;
	cart_info[9].mt_rd = rom+ROML+0x1000;
	cart_info[9].mt_wr = ram+ROML+0x1000;
	/* ROMH - depending on config this is mapped in two locations - our bank just maps both */
	cart_info[10].mt_rd = rom+ROMH;
	cart_info[10].mt_wr = ram+ROMH;
	cart_info[11].mt_rd = rom+ROMH+0x1000;
	cart_info[11].mt_wr = ram+ROMH+0x1000;
	cart_info[14].mt_rd = rom+ROMH;
	cart_info[14].mt_wr = ram+ROMH;
	cart_info[15].mt_rd = rom+ROMH+0x1000;
	cart_info[15].mt_wr = ram+ROMH+0x1000;

	/* IO */
	io_info[13].mf_wr = io_wr;
	io_info[13].mf_rd = io_rd;
	io_info[13].mf_peek = io_peek;

	/* CPU virtual address space */
	for(i=0;i<16;i++) {
		cpumap[i].mask = 0;
		cpumap[i].comp = 0;

		/* mask/comp flag */
		if (i == 0) {
			/* CPU registers 0/1 */
			cpumap[i].mask = 0xfffe;
			cpumap[i].comp = 0x0000;
			cpumap[i].m_wr = &mem_setcpuport;
			cpumap[i].m_rd = &mem_getcpuport;
		}

		// lower 32k and $cxxx are always RAM
		if (i < 8 || i == 12) {
			cpumap[i].inf = &ram_info[i];
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

	memset(rom_info, 0, sizeof(rom_info));
	memset(ram_info, 0, sizeof(ram_info));
	memset(io_info, 0, sizeof(io_info));
	memset(cart_info, 0, sizeof(cart_info));

	mon_register_bank(&rambank);
	mon_register_bank(&rombank);
	mon_register_bank(&iobank);
	mon_register_bank(&cartbank);
}


void mem_start(){
	char fname[MAXLINE];
	size_t offset[]={ 0, KERNEL, BASIC, CHAROM, ROMH, ROML };
	size_t len[]=   { 0, 8192,   8192,  4096,   8192, 8192 };
	int i;
	loram=hiram=charen=1;

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

