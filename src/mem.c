
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

//mt m[16];

//static memmap_t cpumap[16];

/*******************************************************************/

static inline trap_t* add_trap(trap_t ***traplistp, scnt offset, void (*exec)(CPU *cpu, scnt addr), const char *name) {
	trap_t *old = NULL;

	trap_t **traplist = *traplistp;

	if (traplist) {
		old = traplist[offset];
	} else {
		if (exec == NULL) {
			// no traplist and delete - just return
			return NULL;
		}
		traplist = malloc(PAGESIZE * sizeof(trap_t*));
		memset(traplist, 0, PAGESIZE * sizeof(trap_t*));
		*traplistp = traplist;
	}

	if (exec == NULL) {
		// remove
		traplist[offset] = NULL;
	} else {
		traplist[offset] = malloc(sizeof(trap_t));
		traplist[offset]->exec = exec;
		traplist[offset]->name = name;
	}
	return old;
}

trap_t *add_cpu_trap(bank_t *bankp, scnt trapaddr, void (*exec)(CPU *cpu, scnt addr), const char *name) {
        register scnt bank = trapaddr >> 12;
        register scnt offset = trapaddr & 0xfff;
        memmap_t *map = &((memmap_t*)(bankp->map))[bank];
	
	return add_trap(&(map->traplist), offset, exec, name);
}

trap_t *rm_cpu_trap(bank_t *bankp, scnt trapaddr) {
        register scnt bank = trapaddr >> 12;
        register scnt offset = trapaddr & 0xfff;
        memmap_t *cpumap = &((memmap_t*)(bankp->map))[bank];
	
	return add_trap(&(cpumap->traplist), offset, NULL, NULL);
}

trap_t *add_mem_trap(bank_t *bankp, scnt trapaddr, void (*exec)(CPU *cpu, scnt addr), const char *name) {
        register scnt bank = trapaddr >> 12;
        register scnt offset = trapaddr & 0xfff;
        meminfo_t *map = &((meminfo_t*)(bankp->map))[bank];
	
	return add_trap(&(map->traplist), offset, exec, name);
}

trap_t *rm_mem_trap(bank_t *bankp, scnt trapaddr) {
        register scnt bank = trapaddr >> 12;
        register scnt offset = trapaddr & 0xfff;
        meminfo_t *map = &((meminfo_t*)(bankp->map))[bank];
	
	return add_trap(&(map->traplist), offset, NULL, NULL);
}


/*******************************************************************/
/* TODO bank_[mem|cpu]_[peek|poke]() */

void bank_mem_poke(bank_t *bank, scnt addr, scnt val) {
}

scnt bank_mem_peek(bank_t *bankp, scnt addr) {
        register scnt bank = addr >> 12;
        register scnt offset = addr & 0xfff;
        meminfo_t *inf = &((meminfo_t*)(bankp->map))[bank];
        //memmap_t *cpumap = &((memmap_t*)(bankp->map))[bank];

	// TODO: peek instead of read
	if (inf->mf_rd != NULL) {
		return (inf->mf_rd(offset));
	}
	if (inf->mt_rd != NULL) {
		return (inf->mt_rd[offset]);
	}
	return addr >> 8;
}

void bank_cpu_poke(bank_t *bank, scnt addr, scnt val) {
}

scnt bank_cpu_peek(bank_t *bankp, scnt addr) {
        register scnt bank = addr >> 12;
        register scnt offset = addr & 0xfff;
        memmap_t *cpumap = &((memmap_t*)(bankp->map))[bank];

	if (cpumap->mask && ((offset & cpumap->mask) == cpumap->comp)) {
		return cpumap->m_rd(offset);
	}

	meminfo_t *inf = cpumap->inf;
	// TODO: peek instead of read
	if (inf->mf_rd != NULL) {
		return (inf->mf_rd(offset));
	}
	if (inf->mt_rd != NULL) {
		return (inf->mt_rd[offset]);
	}
	return addr >> 8;
	return 0;
}


/*******************************************************************/


int loadrom(char *fname, uchar *mem, size_t len) {
	FILE *fp;
	size_t nread;

	fp=fopen(fname,"rb");
	if(fp) {
		nread=fread(mem,1,len,fp);
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
	

