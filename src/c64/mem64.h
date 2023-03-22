
#define MP_RAM0		0	/* RAM1-15= MP_RAM0+{1-15} */


#define	MP_KERNEL0	0	
#define	MP_KERNEL1	1
#define	MP_BASIC0	2
#define	MP_BASIC1	3
#define	MP_CHAROM	4
#define	MP_ROML0	5
#define	MP_ROML1	6
#define	MP_ROMH0	7
#define	MP_ROMH1	8

void rom_settrap(scnt addr, void (*trapfunc)(CPU *cpu, scnt addr), const char *name);

/* set video character memory address callback */
void mem_set_vaddr(scnt addr, void (*wrvid)(meminfo_t *inf, scnt addr, scnt val));

/* peek into RAM, to re-draw video screen */
scnt mem_getvbyt(scnt addr);

