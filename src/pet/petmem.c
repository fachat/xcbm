
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

	
unsigned char *mem;

#define 	KERNEL	(MP_KERNEL*4096l)
#define 	BASIC	(MP_BASIC*4096l)
#define		EDITOR	(MP_EDIT*4096l)

/*******************************************************************/

meminfo memtab[MP_NUM];
mt m[16];
int reg0,dir0;


scnt getbyt(scnt a) {
        register scnt c=a>>12;
	if(!(a&0xfffe)) {
		if(a) return(reg0);
		else return(dir0);
	}
        if(m[c].i.mf_rd) {
/*logout(0,"read address %04x gives function call at %p",(int)a,m[c].i.mf_rd); */
                return(m[c].i.mf_rd(a));
	}
        if(m[c].i.mt_rd) {
/*logout(0,"read address %04x gives %02x",(int)a,(int)m[c].i.mt_rd[a&0xfff]); */
             return(m[c].i.mt_rd[a&0xfff]);
	}
        return(a>>8);
}

/*******************************************************************/

void updatemr(int page, int newpage) {
	int i,n;
/*logout(0,"update read access page %d to %d",page,newpage);*/
	if(m[page].rd!=newpage) {
	    m[page].rd=newpage;
	    m[page].i.mt_rd=memtab[newpage].mt_rd;
	    m[page].i.mf_rd=memtab[newpage].mf_rd;
	    n=m[page].i.ntraps=memtab[newpage].ntraps;
	    for(i=0;i<n;i++) {
		m[page].i.traplist[i]=memtab[newpage].traplist[i];
/*logout(0,"new trap page %d, trap #%d is %x",page,i,m[page].i.traplist[i].adr);*/
	    }
	}
}

void updatemw(int page, int newpage) {
/*logout(0,"update write access page %d to %d",page,newpage);*/
	if(m[page].wr!=newpage) {
	    m[page].wr=newpage;
	    m[page].i.mt_wr=memtab[newpage].mt_wr;
	    m[page].i.mf_wr=memtab[newpage].mf_wr;
	}
}

/*******************************************************************/

void setmap(void) {
	int i;

	for(i=0;i<16;i++) {
		if(i==14) 
			updatemw(i,MP_IO64);
		else 
			updatemw(i,i);
			updatemr(i,i);
	}
}


/*******************************************************************/


void update_mem(int mempage) {
	int i,j,n;	
/*logout(0,"update_mem page %d",mempage);*/
	for(i=0;i<16;i++) {
		if(m[i].rd==mempage) {
			m[i].i.mt_rd=memtab[mempage].mt_rd;
			m[i].i.mf_rd=memtab[mempage].mf_rd;
			n=m[i].i.ntraps=memtab[mempage].ntraps;
			for(j=0;j<n;j++) {
			      m[i].i.traplist[j]=memtab[mempage].traplist[j];
			}
		}
		if(m[i].wr==mempage) {
			m[i].i.mt_wr=memtab[mempage].mt_wr;
			m[i].i.mf_wr=memtab[mempage].mf_wr;
		}
	}
}

void setrd(int mempage, scnt (*func)(scnt)) {
	memtab[mempage].mf_rd=func;
	update_mem(mempage);
}

void setwr(int mempage, void (*func)(scnt,scnt)) {
	memtab[mempage].mf_wr=func;
	update_mem(mempage);
}

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
	for(i=MP_RAM0;i<MP_RAM0+7;i++) {
	 	memtab[i].mt_wr=mem+i*0x1000;
	 	memtab[i].mt_rd=mem+i*0x1000;
	}
	/* KERNEL */
	memtab[MP_KERNEL].mt_rd = mem+KERNEL;
	/* BASIC */
	memtab[MP_BASIC].mt_rd = mem+BASIC;
	memtab[MP_BASIC+1].mt_rd = mem+BASIC+0x1000;
	memtab[MP_BASIC+2].mt_rd = mem+BASIC+0x2000;
	/* EDITOR */
	memtab[MP_EDIT].mt_rd = mem+EDITOR;

	for(i=0;i<16;i++) {
		if((i&7)==1) m[i].vr=mem+CHAROM;
		else m[i].vr=mem+i*0x1000;
		m[i].wr=m[i].rd=-1;
	}
	setmap();
}

void mem_exit(void){
	free(mem);
}
	
int loadrom(char *fname, size_t offset, size_t len) {
	FILE *fp;
	size_t nread;

	fp=fopen(fname,"rb");
	if(fp) {
		nread=fread(mem+offset,1,len,fp);
		fclose(fp);		
		if(nread!=len) {
			logout(4,"Read Error reading %s > errno=%d, %s",
				fname, errno, strerror(errno));
		} else {
			logout(0,"Read of %s successful", fname);
		}
	} else {
		logout(4,"Error opening %s > errno=%d, %s",
                        fname, errno, strerror(errno));
	}
	return(errno);
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

