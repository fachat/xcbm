
#include 	<stdio.h>
#include	<stdlib.h>
#include	<getopt.h>

#include  	"log.h"
#include  	"config.h"
#include  	"labels.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"cpu.h"
#include	"emu6502.h"
#include	"ccurses.h"
#include	"mem.h"
#include	"mem64.h"
#include	"mon.h"

#include	"io.h"
#include	"video.h"
#include	"speed.h"
#include	"keys.h"
#include	"devices.h"
#include	"vdrive.h"

#define	MAXLINE		200

extern uchar prb[];


void usage(void) {
	printf(
 "xcbm is a c64/cbm3032 emulator for curses or XWindows based Un*x systems\n"
 " xcbm [Options]\n"
	);
	config_print();

 //"   -b =force to black/white in curses mode\n"
  	exit(1);
}

int main(int argc, char *argv[])
{
	int er=0;
	
	loginit("c64.log");

	config_init();

	label_init();

	logout(4,"6502-Emulation \n(c) 1993/94 A.Fachat");

	// init vdrive table
	devices_init();

	mem_init();

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

	// PAL
	CPU *cpu = cpu_init("main", 985248, 20, 0);

	mon_init();
	mon_register_cpu(cpu);

	video_init(cpu);
	key_init(cpu);
	io_init(cpu->bus);	

	vdrive_init();
	vdrive_setdrive(8,0,".");
//settrap(MP_KERNEL0+1,0xfce4,NULL,"test");

	speed_set_percent(100);

	cpu_run();
	
	return(er);	
}


