
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"types.h"
#include	"emu6502.h"
#include	"log.h"
#include 	"mem.h"
#include 	"petmem.h"

#define	MAXLINE		200
	
#define		VRAM	0x8000
#define 	KERNEL	(MP_KERNEL*4096l)
#define 	BASIC	(MP_BASIC*4096l)
#define		EDITOR	(MP_EDIT*4096l)

#define		MEMLEN	0x10000

/*******************************************************************/

meminfo memtab[MP_NUM];

uchar *colram = NULL;

void setmap(void) {
	int i;

	for(i=0;i<16;i++) {
		updatemw(i,i);
		updatemr(i,i);
	}
}


/*******************************************************************/

void inimemvec(void){
	int i;
	for(i=0;i<MP_NUM;i++) {
		memtab[i].mt_wr=NULL;
		memtab[i].mt_rd=NULL;
		memtab[i].ntraps=0;
		memtab[i].mf_wr=NULL;
		memtab[i].mf_rd=NULL;
	}
	/* RAM (including VRAM) */
	for(i=MP_RAM0;i<MP_RAM0+9;i++) {
	 	memtab[i].mt_wr=mem+i*0x1000;
	 	memtab[i].mt_rd=mem+i*0x1000;
	}
	/* video + color RAM at $8800 */
	colram = memtab[MP_VRAM].mt_wr + 0x0800;

	/* KERNEL */
	memtab[MP_KERNEL].mt_rd = mem+KERNEL;
	/* BASIC */
	memtab[MP_BASIC].mt_rd = mem+BASIC;
	memtab[MP_BASIC+1].mt_rd = mem+BASIC+0x1000;
	memtab[MP_BASIC+2].mt_rd = mem+BASIC+0x2000;
	/* EDITOR */
	memtab[MP_EDIT].mt_rd = mem+EDITOR;

	for(i=0;i<16;i++) {
		if(i==8) m[i].vr=mem+VRAM;
		else m[i].vr=NULL;
		m[i].wr=m[i].rd=-1;
	}
	setmap();
}


int mem_init(char *names[], int games, int exrom){
	char fname[MAXLINE];
	size_t offset[]={ 0, KERNEL, BASIC, EDITOR };
	size_t len[]=   { 0, 4096,   3*4096,  2048 };
	int i;
	mem=malloc(MEMLEN);
	if(mem){
	  atexit(mem_exit);
	  for(i=1;i<4;i++) {
	    if(names[i][0]=='/') {
	      strcpy(fname,names[i]);
	    } else {
	      strcpy(fname,names[0]);
	      if(fname[strlen(fname)-1]!='/') strcat(fname,"/");
	      strcat(fname,names[i]);
	    }
	    loadrom(fname, offset[i], len[i]);
	  }	
	  inimemvec();
	  return(0);
	}
	exit(1);
}

