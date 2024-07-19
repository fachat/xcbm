
#include 	<stdio.h>
#include	<stdlib.h>
#include	<getopt.h>

//#include	<time.h>
#include	<math.h>

#include  	"log.h"

#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"cpu.h"
#include	"emu65816.h"
#include	"ccurses.h"
#include	"devices.h"
#include	"vdrive.h"
#include	"mem.h"
#include	"mon.h"
#include	"stop.h"
#include	"labels.h"
#include	"config.h"
#include	"spi.h"
#include	"sdcard.h"

#include	"io.h"
#include	"video.h"
#include	"speed.h"
#include	"keys.h"
#include	"vdrive.h"



void usage(void) {
	printf(
 		"xupet is a Micro-PET emulator for curses based Un*x systems\n"
		" xupet [options]\n"
 		"Options:\n"
	);
 	config_print();
  	exit(1);
}

static int sdcard_set_img(const char *name) {

	sdcard_set_path(name);
	sdcard_attach();

	return 0;
}

static config_t sdcard_pars[] = {
	{ "sdcard", 'S', "imgpath", sdcard_set_img, "Set the path for the emulated SD card" },
	{ NULL }
};

int main(int argc, char *argv[])
{
	int er=0;
	
	loginit("upet.log");

	setbinprefix("upet", argv[0]);

	config_init();

	config_register(sdcard_pars);

	// init virtual device table
	devices_init();

	label_init();

	spi_init();

	mem_init();

	logout(4,"65816-Emulation \n(c) 1993/2023 A.Fachat");

	int e = config_parse(argc, argv);
	if (e < 0) {
		exit(1);
	}
	if (e < argc) {
		printf("Unknown extra parameter(s) %s ...\n", argv[e]);
		exit(1);
	}

	spi_start();
	mem_start();
	cur_init();

	CPU *cpu = cpu_init("main", 1000000, 16, 0, 0xfffff);

	video_init(cpu->bus, 1000000/60);
	key_init(cpu->bus);	

	io_init(cpu->bus);	

	vdrive_init();
	vdrive_setdrive(8,0,".");

	stop_init();

	mon_init();
	mon_register_cpu(cpu);

	// 200% speed for now
	speed_set_percent(1000);

	cpu_run();
	
	return(er);	
}



