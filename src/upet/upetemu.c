
#include 	<stdio.h>
#include	<stdlib.h>
#include	<getopt.h>

//#include	<time.h>
#include	<math.h>

#include  	"log.h"

#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"emu65816.h"
#include	"ccurses.h"
#include	"devices.h"
#include	"vdrive.h"
#include	"mem.h"
#include	"mon.h"
#include	"labels.h"
#include	"config.h"
#include	"spi.h"

#include	"io.h"
#include	"video.h"
#include	"speed.h"
#include	"keys.h"
#include	"vdrive.h"


extern uchar prb[];


char *files[] = {
	"/var/lib/xupet", "petkernel4.rom", "petbasic4.rom", "petedit4.rom"
}; 


void usage(void) {
	printf(
 		"xupet is a Micro-PET emulator for curses based Un*x systems\n"
		" xupet [options]\n"
 		"Options:\n"
	);
 	config_print();
  	exit(1);
}

int main(int argc, char *argv[])
{
	int er=0;
	
	loginit("upet.log");

	config_init();

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

	video_init(cpu);
	key_init(cpu);	

	io_init(cpu->bus);	

	vdrive_init();
	vdrive_setdrive(8,0,".");

	mon_init();
	mon_register_cpu(cpu);

	// 200% speed for now
	speed_set_percent(1000);

	cpu_run();
	
	return(er);	
}



