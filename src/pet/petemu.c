
#include 	<stdio.h>
#include	<stdlib.h>
#include	<getopt.h>

#include	<time.h>
#include	<math.h>

#include  	"log.h"

#include	"types.h"
#include	"alarm.h"
#include	"emu6502.h"
#include	"ccurses.h"
#include	"iec.h"
#include	"mem.h"

#include	"petemu.h"
#include	"io.h"
#include	"video.h"
#include	"speed.h"


extern uchar prb[];


char *files[] = {
	"/var/lib/xcbm", "petkernel4.rom", "petbasic4.rom", "petedit4.rom"
}; 


void usage(void) {
	printf(
 "xcbm4032 is a cbm4032 emulator for curses based Un*x systems\n"
 " xcbm4032 [options]\n"
 "Options:\n"
 "-b\n"
 "   force to black/white in curses mode\n"
 "-d rom_directory\n"
 "   set common ROM directory (default=/var/lib/xcbm)\n"
 "-8 {0|1}=path_for_unit_8\n"
 "-9 {0|1}=path_for_unit_9\n"
 "   set directories to be used as unit 8/9 'diskettes'\n"
 "-K pet_kernel_file\n"
 "-B pet_basic_file\n"
 "-E pet_edit_file\n"
 "   set different ROM files\n"
  );
  exit(1);
}

int main(int argc, char *argv[])
{
	int er=0;
	int o;
	
	loginit("c64.log");

	logout(4,"6502-Emulation \n(c) 1993/94 A.Fachat");

	while((o=getopt(argc,argv,"bd:8:9:K:B:E:?"))>=0) {
	    switch(o) {	
	    case 'b':
		color=-1;
		break;
	    case '?':
		usage();
		break;
	    case 'd':
	 	files[0]=optarg;
		break;
	    case '8':
		if(optarg[0]=='0' && optarg[1]=='=') 
		  iec_setdrive(8,0,optarg+2); 
		else if(optarg[0]=='1' && optarg[1]=='=') 
		  iec_setdrive(8,0,optarg+2); 
		else logout(4,"wrong filename for disk setup");
		break;
	    case '9':
		if(optarg[0]=='0' && optarg[1]=='=') 
		  iec_setdrive(9,0,optarg+2); 
		else if(optarg[0]=='1' && optarg[1]=='=') 
		  iec_setdrive(9,0,optarg+2); 
		else logout(4,"wrong filename for disk setup");
		break;
	    case 'K':
		files[1]=optarg;
		break;
	    case 'B':
		files[2]=optarg;
		break;
	    case 'E':
		files[3]=optarg;
		break;
	    default:
		logout(1,"unknown option!");
		break;
	    }
	}

	mem_init(files,0,0);
	cur_init();

	CPU *cpu = cpu_init(1000000, 16);

	video_init(cpu);
	io_init();	

	iec_init();
	iec_setdrive(8,0,".");
//settrap(MP_KERNEL1,0xfce4,NULL,"test");


	// 200% speed for now
	speed_set_percent(200);

	cpu_run();
	
	return(er);	
}


/*
void trap1(scnt trapadr, CPU *cpu) {
        dismode=1;
}
*/


