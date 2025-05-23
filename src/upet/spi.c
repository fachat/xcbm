
#include	<stdio.h>
#include 	<string.h>

#include	"log.h"
#include 	"config.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"cpu.h"
#include 	"mem.h"
#include 	"sdcard.h"
#include 	"rtc.h"

#define	SPI_FLASH_VERBOSE

// up to 512k Flash ROM
#define	SPI_LEN		0x20000

static int selected;		// selected device

/* ---------------------------------------------------------------*/

static uchar spiimg[SPI_LEN];	

static int flash_cmd = 0;	// are we reading or writing or ...
static int flash_state = 0;	// where are we in the command?
static int flash_addr = 0;	// flash addr

// write a byte, and at the same time read one from SPI, and return it... emulated...
static scnt flash_wr(scnt val) {

	scnt rv = 0;

#ifdef SPI_FLASH_VERBOSE
	int old_state = flash_state;
#endif

	switch (flash_state) {
	case 0:		// initial
		if (val = 3) {	
			// READ
			flash_cmd = val;
			flash_state = 1;
		} else {
			// error
			flash_state = -1;	
		}
		break;
	case 1:		// address byte 2
		flash_addr |= (val & 0xff) << 16;
		flash_state ++;
		break;
	case 2:		// address byte 1
		flash_addr |= (val & 0xff) << 8;
		flash_state ++;
		break;
	case 3:		// address byte 0
		flash_addr |= (val & 0xff);
		flash_state ++;
		break;
	case 4:	
		switch(flash_cmd) {
		case 3:
			rv = spiimg[(flash_addr++) & 0x1ffff];
		}
	}
#ifdef SPI_FLASH_VERBOSE
	logout(0, "flash_wr(%02x) with state=%d, cmd=%d, addr=%06x -> new state %d, rv=%02x", 
		val, old_state, flash_cmd, flash_addr, rv);
#endif
	return rv;

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
#define	SPI_SDCARD	3	/* SD card */
#define	SPI_RTC		5	/* RTC chip */

static scnt spi_last = 0xff;

void spi_wr(scnt addr, scnt val) {

	switch (addr & 0x03) {
	case 0:		// control register
		int old_selected = selected;
		switch (val & 0x07) {
		case SPI_FLASH:
			rtc_select(0);
			sdcard_select(0);
			selected = SPI_FLASH;
			break;
		case SPI_SDCARD:
			rtc_select(0);
			sdcard_select(1);
			flash_deselect();
			selected = SPI_SDCARD;
			break;
		case SPI_RTC:
			rtc_select(1);
			flash_deselect();
			selected = SPI_SDCARD;
			break;
		default:
			flash_deselect();
			sdcard_select(0);
			rtc_select(0);
			selected = -1;
			break;
		}
		if (old_selected != selected) {
			logout(0, "SPI selected device %d", selected);
		}
		break;
	case 1:		// read/write with auto-shift
		switch (selected) {
		case SPI_FLASH:	// flash
			spi_last = flash_wr(val);
			break;
		case SPI_SDCARD:
			spi_last = sdcard_handle(val);
			break;
		case SPI_RTC:
			spi_last = rtc_handle(val);
			break;
		}
		break;
	default:
		break;
	}
}

scnt spi_rd(scnt addr) {

	scnt tmp;

	switch (addr & 0x03) {
	case 0:		// control register (ignore state, we're always ready)
		return selected & 0x07;
		break;
	case 1:		// read/write with auto-shift
		tmp = spi_last;
		switch (selected) {
		case SPI_FLASH:	// flash
			spi_last = flash_wr(0);
			break;
		case SPI_SDCARD:
			spi_last = sdcard_handle(0xff);
			break;
		case SPI_RTC:
			spi_last = rtc_handle(0xff);
			break;
		}
		return tmp;	
		break;
	case 2:		// peek data from last transfer without auto-triggering
		return spi_last;
	default:
		break;
	}
	return 0;
}


/* ---------------------------------------------------------------*/

static const char *names[] = { 
		"/var/lib/cbm/upet",
		"spi.rom",
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
	{ "spi-rom", 'R', "spi_rom", mem_set_spiimg, "set SPI Flash ROM file name (in ROM directory; default 'spi.rom')" },
	{ NULL }
};


void spi_init() {
	config_register(mem_pars);
}

void spi_start() {
	int i;

	    i = 1;
	    loadrom(names[0],names[i], spiimg, SPI_LEN);

	// reset flash state
	selected = -1;
	flash_deselect();

	  return;
}


