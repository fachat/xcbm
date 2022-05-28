
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"types.h"
#include	"emu6502.h"
#include	"emu64.h"
#include	"log.h"
#include 	"mem.h"
#include 	"mem64.h"

int	hiram;
int	loram;
int	charen;

meminfo memtab[MP_NUM];

#define	seekernel()	(hiram)
#define	seebasic()	(hiram&&loram)
#define	seeroml()	0
#define	seeromh()	0
#define	seechar()	((!charen)&&(hiram||loram))
#define	seeio()		((charen)&&(hiram||loram))
	

#define 	KERNEL	65536l
#define 	BASIC	(KERNEL+8192l)
#define		CHAROM	(BASIC+8192l)
#define		ROML	(CHAROM+4096l)
#define		ROMH	(ROML+8192l)
#define		MEMLEN	(ROMH+8192l)

/*******************************************************************/

void mem_setcpuport(scnt adr, scnt byt);
scnt mem_getcpuport(scnt adr);

int reg0,dir0;

/*******************************************************************/

void setmap(void) {
	int i,p;

	p=reg0|(~dir0);
	loram=p&0x01;
	hiram=p&0x02;
	charen=p&0x04;

logout(0,"set loram=%d, hiram=%d, charen=%d",loram,hiram,charen);
	
	for(i=0;i<16;i++) {
		if(i==13 && seeio()) 
			updatemw(i,MP_IO64);
		else 
			updatemw(i,i);
	}

	for(i=0;i<8;i++) {
		updatemr(i,i);
	}
	if(seeroml()) {
		updatemr(8,MP_ROML0);
		updatemr(9,MP_ROML1);
	} else {
		updatemr(8,8);
		updatemr(9,9);
	}
	if(seebasic()) {
		updatemr(10,MP_BASIC0);
		updatemr(11,MP_BASIC1);
	} else if(seeromh()) {
		updatemr(10,MP_ROMH0);
		updatemr(11,MP_ROMH1);
	} else {
		updatemr(10,10);
		updatemr(11,11);
	}
	updatemr(12,12);
	if(seeio()) {
		updatemr(13,MP_IO64);
	} else if(seechar()) {
		updatemr(13,MP_CHAROM);
	} else { 
		updatemr(13,13);
	}
	if(seekernel()) {
		updatemr(14,MP_KERNEL0);
		updatemr(15,MP_KERNEL1);
	} else {
		updatemr(14,14);
		updatemr(15,15);
	}	
}

void mem_setcpuport(scnt adr, scnt byt) {

	logout(0, "mem_setcpuport(a=%04x, b=%02x)", adr, byt);
	if(adr&1) {
		if(byt==reg0)
			return;
		reg0=byt;
	} else {
		if(byt==dir0)
			return;
		dir0=byt;
	}
	setmap();
}

scnt mem_getcpuport(scnt adr) {
	if(adr&1) {
		return reg0;
	}
	return dir0;
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
	/* RAM */
	for(i=MP_RAM0;i<MP_RAM0+16;i++) {
	 	memtab[i].mt_wr=mem+i*0x1000;
	 	memtab[i].mt_rd=mem+i*0x1000;
	}
	/* KERNEL */
	memtab[MP_KERNEL0].mt_rd = mem+KERNEL;
	memtab[MP_KERNEL1].mt_rd = mem+KERNEL+0x1000;
	/* BASIC */
	memtab[MP_BASIC0].mt_rd = mem+BASIC;
	memtab[MP_BASIC1].mt_rd = mem+BASIC+0x1000;
	/* CHAROM */
	memtab[MP_CHAROM].mt_rd = mem+CHAROM;
	/* ROML */
	memtab[MP_ROML0].mt_rd = mem+ROML;
	memtab[MP_ROML1].mt_rd = mem+ROML+0x1000;
	/* ROMH */
	memtab[MP_ROMH0].mt_rd = mem+ROMH;
	memtab[MP_ROMH1].mt_rd = mem+ROMH+0x1000;

	/* CPU virtual address space */
	for(i=0;i<16;i++) {
		/* video memory address */
		if((i&7)==1) {
			m[i].vr=mem+CHAROM;
		} else {
			m[i].vr=mem+i*0x1000;
		}
		/* current page */
		m[i].wr=m[i].rd=-1;
		/* mask/comp flag */
		if (i == 0) {
			/* CPU registers 0/1 */
			m[i].mask = 0xfffe;
			m[i].comp = 0x0000;
			m[i].m_wr = &mem_setcpuport;
			m[i].m_rd = &mem_getcpuport;
		} else {
			m[i].mask = 0;
		}
	}
	reg0=dir0=0;
	setmap();
}


int mem_init(char *names[], int games, int exrom){
	char fname[MAXLINE];
	size_t offset[]={ 0, KERNEL, BASIC, CHAROM, ROMH, ROML };
	size_t len[]=   { 0, 8192,   8192,  4096,   8192, 8192 };
	int i;
	loram=hiram=charen=1;
	mem=malloc(MEMLEN);
	if(mem){
	  atexit(mem_exit);
	  for(i=1;i<6;i++) {
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

