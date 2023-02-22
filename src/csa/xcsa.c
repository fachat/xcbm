
#include 	<stdio.h>
#include	<stdlib.h>
#include	<getopt.h>

#include	<time.h>
#include	<math.h>

#include  	"log.h"

#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"emu6502.h"
#include	"ccurses.h"
#include	"devices.h"
#include	"vdrive.h"
#include	"mem.h"
#include	"mon.h"
#include	"labels.h"
#include	"config.h"

#include	"io.h"
#include	"video.h"
#include	"speed.h"
#include	"keys.h"
#include	"vdrive.h"


extern uchar prb[];


char *files[] = {
	"/var/lib/xcbm", "petkernel4.rom", "petbasic4.rom", "petedit4.rom"
}; 


void usage(void) {
	printf(
 		"xcbm8032 is a cbm8032 emulator for curses based Un*x systems\n"
		" xcbm4032 [options]\n"
 		"Options:\n"
	);
 	config_print();
  	exit(1);
}

int main(int argc, char *argv[])
{
	int er=0;
	
	loginit("c64.log");

	config_init();

	// init virtual device table
	devices_init();

	label_init();

	mem_init();

	logout(4,"6502-Emulation \n(c) 1993/2023 A.Fachat");

	int e = config_parse(argc, argv);
	if (e < 0) {
		exit(1);
	}
	if (e < argc) {
		printf("Unknown extra parameter(s) %s ...\n", argv[e]);
		exit(1);
	}

	mem_start();
	cur_init();

	CPU *cpu = cpu_init("main", 1000000, 16);

	video_init(cpu);
	key_init(cpu);	

	io_init(cpu->bus);	

	vdrive_init();
	vdrive_setdrive(8,0,".");
//settrap(MP_KERNEL1,0xfce4,NULL,"test");

	mon_init();
	mon_register_cpu(cpu);

	// 200% speed for now
	speed_set_percent(1000);

	// TODO: move that into CPU struct
	//dismode=1;

	cpu_run();
	
	return(er);	
}


/*
void trap1(scnt trapadr, CPU *cpu) {
        dismode=1;
}
*/


