
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"log.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"cpu.h"
#include 	"mem.h"


#define	MAXLINE	200
	
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

void bank_mem_poke(bank_t *bank, saddr addr, scnt val) {
}

scnt bank_mem_peek(bank_t *bankp, saddr addr) {
	addr &= bankp->mapmask;
        register scnt page = addr >> 12;
        register scnt offset = addr & 0xfff;

        meminfo_t *inf = &((meminfo_t*)(bankp->map))[page];

	if (inf->mf_peek != NULL) {
		return (inf->mf_peek(inf, offset));
	}
	if (inf->mt_rd != NULL) {
		return (inf->mt_rd[offset]);
	}
	return addr >> 8;
}

void bank_cpu_poke(bank_t *bank, saddr addr, scnt val) {
}

scnt bank_cpu_peek(bank_t *bankp, saddr addr) {
	addr &= bankp->mapmask;
        register scnt page = addr >> 12;
        register scnt offset = addr & 0xfff;

        memmap_t *cpumap = &((memmap_t*)(bankp->map))[page];

	if (cpumap->mask && ((offset & cpumap->mask) == cpumap->comp)) {
		return cpumap->m_peek(offset);
	}

	meminfo_t *inf = cpumap->inf;

	if (inf->mf_peek != NULL) {
		return (inf->mf_peek(inf, offset));
	}
	if (inf->mt_rd != NULL) {
		return (inf->mt_rd[offset]);
	}
	return addr >> 8;
}


/*******************************************************************/


int loadrom_int(const char *prefix, const char *filename, uchar *mem, size_t len) {
	FILE *fp;
	size_t nread;
	char fname[MAXLINE];
	int l;

	fname[MAXLINE-1] = 0;
        if(filename[0]=='/') {
              strncpy(fname,filename, MAXLINE-1);
        } else {
              strncpy(fname,prefix, MAXLINE-1);
	      l = strlen(fname);
              if(l < MAXLINE-1 && fname[l-1]!='/') {
			strcat(fname,"/");
			l++;
	      }
              strncat(fname,filename, MAXLINE-1-l);
        }

	logout(0, "Trying to load '%s'", fname);

	errno = 0;
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

static char binprefix[MAXLINE+4];

void setbinprefix(char *emu, char *argv0) {

	strncpy(binprefix, argv0, MAXLINE-1);
	binprefix[MAXLINE-1]=0;

	char *c = rindex(binprefix, '/');
	if (c) {
		*c = 0;
	}
	c = rindex(binprefix, '/');
	if (c) {
		*c = 0;
	}
	strncat(c, "/roms/", MAXLINE-1-strlen(binprefix));
	strncat(c, emu, MAXLINE-1-strlen(binprefix));
}

int loadrom(const char *prefix, const char *fname, uchar *mem, size_t len) {
	int r;

	r = loadrom_int(prefix, fname, mem, len);
	if (r == 0) {
		return 0;
	}

	r = loadrom_int(binprefix, fname, mem, len);
	if (r == 0) {
		return 0;
	}

	return r;
}

