
#include	<stdio.h>
#include 	<string.h>

#include	"log.h"
#include 	"config.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"emu6502.h"
#include 	"mem.h"

#define	MAXLINE		200

// up to 512k Flash ROM
#define	SPI_LEN		0x20000

static uchar spiimg[SPI_LEN];	


/* ---------------------------------------------------------------*/

static const char *names[] = { 
		"/var/lib/cbm/upet",
		"spi.rom"
};

static int mem_set_rom_dir(const char *param) {
	names[0] = param;
	return 0;
}

static int mem_set_spiimg(const char *param) {
	names[1] = param;
	return 0;
}

static config_t mem_pars[] = {
	{ "rom-dir", 'd', "rom_directory", mem_set_rom_dir, "set common ROM directory (default = /var/lib/cbm/pet)" },
	{ "spi-rom", 'S', "spi_rom", mem_set_spiimg, "set SPI Flash ROM file name (in ROM directory; default 'spi.rom')" },
	{ NULL }
};


void spi_init() {
	config_register(mem_pars);
}

void spi_start() {
	char fname[MAXLINE];
	int i;

	    i = 1;
	    if(names[i][0]=='/') {
	      strcpy(fname,names[i]);
	    } else {
	      strcpy(fname,names[0]);
	      if(fname[strlen(fname)-1]!='/') strcat(fname,"/");
	      strcat(fname,names[i]);
	    }
	    loadrom(fname, spiimg, SPI_LEN);

	  return;
}

void spi_ipl(uchar *iplblk) {

	memcpy(iplblk, spiimg, 256);
}


