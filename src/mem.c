
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"log.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"emu6502.h"
#include 	"mem.h"

	
unsigned char *mem;

/*******************************************************************/

mt m[16];


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

	logout(0, "setrd(page=%d, f=%p)", mempage, func);

	memtab[mempage].mf_rd=func;
	update_mem(mempage);
}

void setwr(int mempage, void (*func)(scnt,scnt)) {

	logout(0, "setwr(page=%d, f=%p)", mempage, func);

	memtab[mempage].mf_wr=func;
	update_mem(mempage);
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
	

