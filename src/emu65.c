
#include 	<stdio.h>
#include	<stdlib.h>
#include	<getopt.h>

#include	"emu65.h"
#include  	"log.h"
#include	"ccurses.h"
#include	"iec.h"
#include	"mem.h"
#include	"c64io.h"
#include	"video.h"

extern uchar prb[];

int fancy=1;		/* 0= raw, 1=fancy */
int wide=0;		/* 0= 40 cols, 1= 80 cols */
int c64=1;		/* 0= 3032, 1= c64 */
int exrom=0;		/* exrom (note: the exp. port has /exrom) */
int game=0;		/* game */
int xwin=0;		/* 0= curses, 1=Xwindows */
int color=0;		/* 0= black/white, 1=color use for ncurses */

char *files[] = {
	"/var/lib/xcbm", "c64kernl.rom", "c64basic.rom", "c64chars.rom",
		   "c64romh.rom", "c64roml.rom",
		"cbm3032.rom"
}; 


void usage(void) {
	printf(
 "xcbm is a c64/cbm3032 emulator for curses or XWindows based Un*x systems\n"
 " xcbm [-bpvrfxcEG][-d rom_drirectory][-8 {0|1}=path_for_unit_8][-9 {0|1}=path]\n"
 "      [-K c64_kernel_file][-B c64_basic_file][-C character_rom_file]\n"
 "      [-L c64_exp_rom_on_lomem][-H c64_exp_rom_on_himem]\n"
 "      [-R cbm3032_rom_file]\n"
 "   -p =cbm3032 emulation   -v =c64 emulation (default)\n"
 "   -r =raw mode            -f =fancy mode (default)\n"
 "   -x =XWindows            -c =curses (default)\n"
 "   -E =hold EXROM low      -G =hold GAMES low\n"
 "   -b =force to black/white in curses mode\n"
 "   -d =set common ROM directory (default=/var/lib/xcbm)\n"
 "   -8 / -9 =set directories to be used as unit 8/9 'diskettes'\n"
 "   -K =set different Kernel file for c64 (c64kernl.rom)\n"
 "   -B =set differnet Basic file for c64 (c64basic.rom)\n"
 "   -C =set different Charrom file for c64 (c64chars.rom)\n"
 "   -L =set file for ROM on c64 expansion port line ROML\n"
 "   -H =set file for ROM on c64 expansion port line ROMH\n"
  );
  exit(1);
}

int main(int argc, char *argv[])
{
	int er=0;
	int o;
	
	loginit("c64.log");

	logout(4,"6502-Emulation \n(c) 1993/94 A.Fachat");

	while((o=getopt(argc,argv,"bpd:rfx8:9:K:B:C:L:H:EGR:?"))>=0) {
	    switch(o) {	
	    case 'b':
		color=-1;
		break;
	    case '?':
		usage();
		break;
	    case 'p':
		c64=0;
		break;
	    case 'd':
	 	files[0]=optarg;
		break;
	    case 'r':
		fancy=0;
		break;
	    case 'f':
		fancy=1;
		break;
	    case 'x':
		xwin=1;
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
	    case 'C':
		files[3]=optarg;
		break;
	    case 'H':
		files[4]=optarg;
		break;
	    case 'L':
		files[5]=optarg;
		break;
	    case 'R':
		files[6]=optarg;
		break;
	    case 'E':
		exrom=1;
		break;
	    case 'G':
		game=1;
		break;
	    default:
		logout(1,"unknown option!");
		break;
	    }
	}

	if(xwin || !c64 || wide) usage();
	
	mem_init(files,0,0);
	cur_init();
	video_init();
	io_init();	

	iec_init();
	iec_setdrive(8,0,".");
settrap(MP_KERNEL1,0xfce4,NULL,"test");
	cpu_run();
	
	return(er);	
}

/*
void trap1(scnt trapadr, CPU *cpu) {
        dismode=1;
}
*/


