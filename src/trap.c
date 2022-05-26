
#include <stdio.h>

#include "types.h"
#include "emu6502.h"
#include "log.h"
#include "mem.h"



int settrap(int mp, scnt trapadr, void (*execadr)(scnt trapadr, CPU *cpu),
		const char *name){
	int i,n;
	logout(2,"set trap: mempage=%d, name=%s, trapadr=%04x -> %p",
		mp,name,trapadr,execadr);

	if((n=memtab[mp].ntraps)<MAXTRAPS) {
		for(i=0;i<n;i++) {
/*logout(0,"test: i=%d, n=%d, trapadr=%x, traplist[i].adr0%x",
	i,n,trapadr,memtab[mp].traplist[i].adr);*/
			if(memtab[mp].traplist[i].adr==trapadr) {
				logout(2,"overwrite old trap %s",memtab[mp].traplist[i].name);
				break;
			}
		}

		memtab[mp].traplist[i].name=name;
		memtab[mp].traplist[i].exec=execadr;
		memtab[mp].traplist[i].adr=trapadr;
		if(i==n)
			memtab[mp].ntraps++;
		update_mem(mp);
	} else {
		logout(2,"set trap failed!");
	}
        return(1);
}

int rmtrap(int mp, scnt trapadr) {
        int i,j,n=memtab[mp].ntraps, er = -1;
        for( i=0 ; i<n ; i++ ) {
          if( memtab[mp].traplist[i].adr == trapadr ) {
	    n=--memtab[mp].ntraps;
	    for(j=i;j<n;j++)
	      memtab[mp].traplist[j]=memtab[mp].traplist[j+1];
            er = 0;
	    update_mem(mp);
            break;
          }
        }
        logout(1,"remove trap %s @ %04x",(int)trapadr);
        return(er);
}

void (*trap6502(scnt pc))(scnt,CPU*) {
	static unsigned int i,n,mp;
	mp=pc>>12;
	if(n=m[mp].i.ntraps) {
	  for(i=0;i<n;i++) {
	    if(pc==m[mp].i.traplist[i].adr) {
/*	      logout(0,"found trap %s @ %04x, jump to %p",
		m[mp].i.traplist[i].name,m[mp].i.traplist[i].adr,
		m[mp].i.traplist[i].exec);*/
	      return(m[mp].i.traplist[i].exec);
	    }
	  }
	}
	return(NULL);
}

