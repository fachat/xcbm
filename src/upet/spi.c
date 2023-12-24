
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

static int selected;		// selected device

/* ---------------------------------------------------------------*/

static uchar spiimg[SPI_LEN];	

static int flash_cmd = 0;	// are we reading or writing or ...
static int flash_state = 0;	// where are we in the command?
static int flash_addr = 0;	// flash addr

// write a byte, and at the same time read one from SPI, and return it... emulated...
static scnt flash_wr(scnt val, int flag) {
	return 0;
}

static void flash_deselect() {
	flash_cmd = 0;
	flash_addr = 0;
	flash_state = 0;
}

void spi_ipl(uchar *iplblk) {

	memcpy(iplblk, spiimg, 256);
}

/* ---------------------------------------------------------------*/

#define	SPI_FLASH	1	/* flash image is this selected device */

void spi_wr(scnt addr, scnt val) {

	switch (addr & 0x03) {
	case 0:		// control register
		switch (val & 0x03) {
		case 1:
			selected = 1;
			break;
		default:
			flash_deselect();
			selected = -1;
			break;
		}
		break;
	case 1:		// read/write with auto-shift
		switch (selected) {
		case 1:	// flash
			flash_wr(val, 1);
			break;
		}
		break;
	case 2:		// peek data from last transfer without auto-triggering
		switch (selected) {
		case 1:	// flash
			flash_wr(val, 0);
			break;
		}
		break;
	default:
		break;
	}
}

scnt spi_rd(scnt addr) {

	switch (addr & 0x03) {
	case 0:		// control register (ignore state, we're always ready)
		return 0;
		break;
	case 1:		// read/write with auto-shift
		switch (selected) {
		case 1:	// flash
			return flash_wr(0, 1);
		}
		break;
	case 2:		// peek data from last transfer without auto-triggering
		switch (selected) {
		case 1:	// flash
			return flash_wr(0, 0);
		}
		break;
	default:
		break;
	}
	return 0;
}


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

	// reset flash state
	selected = -1;
	flash_deselect();

	  return;
}


