
void mem_init();
void mem_start();

void update_mem(int);

/*****************************************************************************/
/* some needed structs */

#define	PAGESIZE	4096
#define	PAGES		16


typedef struct trap_s trap_t;
typedef struct bank_s bank_t;
typedef struct meminfo_s meminfo_t;

struct trap_s {
		scnt		addr;
		void		(*exec)(CPU *cpu, scnt trapaddr);
		const char	*name;
};


struct bank_s {
		const char	*name;
		trap_t* 	(*addtrap)(bank_t *bank, scnt trapaddr, void (*execaddr)(CPU *cpu, scnt addr));
		trap_t* 	(*rmtrap)(bank_t *bank, scnt trapaddr);
		void		*map;	// either ptr to memmap_t array (cpu bank), or meminfo_t array (mem bank), 
					// or anything else the corresponding addtrap/rmtrap understands
};

/* information on a memory page (4k) */
struct meminfo_s {
                uchar   	*mt_wr;
		uchar		*mt_rd;
                void    	(*mf_wr)(scnt,scnt);
                scnt    	(*mf_rd)(scnt);
                trap_t 		**traplist;		// array of 4k trap_t* pointers if at least one is set
};

/* entry in the CPU's virtual address space, 16x 4k */
typedef struct {
		// escape e.g. CPU registers 0/1
		scnt		mask;			/* mask for special bits */
		scnt		comp;			/* compare after mask */
                void    	(*m_wr)(scnt,scnt);	/* if mask/comp match, use this to write */
                scnt    	(*m_rd)(scnt);		/* is mask/comp match, use this to read */
		// CPU bank based traps
                trap_t 		**traplist;		// array of 4k trap_t* pointers if at least one is set
		// actual memory mapping
                meminfo_t 	*inf;
} memmap_t;

/*****************************************************************************/
/* CPU memory map 
 *
 * Note: needs to be refined if we add drive CPUs
 */

extern memmap_t cpumap[PAGES];


/*****************************************************************************/
/* Funktions- und Speicheradressen fr 6502-Speicherzugriffe */

// default add/rm trap functions for CPU bank and memory banks respectively
trap_t *add_cpu_trap(bank_t *bank, scnt trapaddr, void (*exec)(CPU *cpu, scnt addr));
trap_t *rm_cpu_trap(bank_t *bank, scnt trapaddr);

trap_t *add_mem_trap(bank_t *bank, scnt trapaddr, void (*exec)(CPU *cpu, scnt addr));
trap_t *rm_mem_trap(bank_t *bank, scnt trapaddr);

static inline scnt getbyt(scnt a) {
	register scnt bank = a >> 12;
	register scnt offset = a & 0xfff;
	memmap_t *cpupage = &cpumap[bank];

	if (cpupage->mask && ((offset & cpupage->mask) == cpupage->comp)) {
		return cpupage->m_rd(offset);
	}

	meminfo_t *inf = cpupage->inf;

        if(inf->mf_rd != NULL) {
                return(inf->mf_rd(offset));
        }
        if(inf->mt_rd != NULL) {
             	return(inf->mt_rd[offset]);
        }
        return(a>>8);

}

static inline scnt getadr(scnt a) {

	return (getbyt(a) & 0xff) | ((getbyt(a+1) & 0xff) << 8);
}


static inline void setbyt(scnt a, scnt b) {
	register scnt bank =  a >> 12;
	register scnt offset = a & 0xfff;
	memmap_t *cpupage = &cpumap[bank];

	if ((cpupage->mask != 0) && ((offset & cpupage->mask) == cpupage->comp)) {
		cpupage->m_wr(offset,b);
	}

	meminfo_t *inf = cpupage->inf;

        if(inf->mt_wr != NULL) {
             	inf->mt_wr[offset] = b;
        }
        if(inf->mf_wr != NULL) {
                inf->mf_wr(a,b);
        }
}

static inline void (*trap6502(scnt a))(CPU*,scnt) {
	register scnt bank =  a >> 12;
	register scnt offset = a & 0xfff;
	memmap_t *cpupage = &cpumap[bank];

	if (cpupage->traplist
		&& cpupage->traplist[offset]) {
		return cpupage->traplist[offset]->exec;
	}

	meminfo_t *inf = cpupage->inf;

	if (inf->traplist
		&& inf->traplist[offset]) {
		return inf->traplist[offset]->exec;
	}
	return NULL;
}

/*
#define getvbyt(a)       (m[(a)>>12].vr[(a)&0xfff])

#define getadr(a)       (getbyt(a)+256*getbyt(a+1))
*/

/*****************************************************************************/
/* "Trap" setzen fr den Einsprung in C-Code (System-Eulation)) */

/*
int settrap(int mempage, scnt trapadr, void execadr(scnt trapadr, CPU *cpureg),
		 const char *name);
int rmtrap(int mempage, scnt trapadr);
void clrtrap(void);

void (*trap6502(scnt))(scnt,CPU*);
*/

/*****************************************************************************/
/* read/write functions fuer memory page setzen */

#if 0
void setrd(int mempage, scnt (*func)(scnt));
void setwr(int mempage, void (*func)(scnt,scnt));
#endif

/*****************************************************************************/
/* internals */

#if 0
void update_mem(int mempage);
void mem_exit(void);
void updatemr(int page, int newpage);
void updatemw(int page, int newpage);

#endif

int loadrom(char *fname, uchar *mem, size_t len);

