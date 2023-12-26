
void mem_init();
void mem_start();

void update_mem(int);

/*****************************************************************************/
/* some needed structs */

#define	PAGESIZE	4096
//#define	PAGES		256
#define	PAGESMASK	0xffff


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
		trap_t* 	(*addtrap)(bank_t *bank, scnt trapaddr, void (*execaddr)(CPU *cpu, scnt addr), const char *name);
		trap_t* 	(*rmtrap)(bank_t *bank, scnt trapaddr);
		scnt		(*peek)(bank_t *bank, saddr addr);
		void		(*poke)(bank_t *bank, saddr addr, scnt val);
		void		*map;	// either ptr to memmap_t array (cpu bank), or meminfo_t array (mem bank), 
					// or anything else the corresponding addtrap/rmtrap understands
		int		mapmask; // mask of valid address bits for map index (typically 0xffff)
};

/* information on a memory page (4k) */
struct meminfo_s {
		int		page;			// page number in bank
                uchar   	*mt_wr;
		uchar		*mt_rd;
                void    	(*mf_wr)(meminfo_t* inf, scnt,scnt);
                scnt    	(*mf_rd)(meminfo_t* inf, scnt);
                scnt    	(*mf_peek)(meminfo_t* inf, scnt);
                trap_t 		**traplist;		// array of 4k trap_t* pointers if at least one is set
};

/* entry in the CPU's virtual address space, 16x 4k */
typedef struct {
		// escape e.g. CPU registers 0/1
		scnt		mask;			/* mask for special bits */
		scnt		comp;			/* compare after mask */
                void    	(*m_wr)(scnt,scnt);	/* if mask/comp match, use this to write */
                scnt    	(*m_rd)(scnt);		/* is mask/comp match, use this to read */
                scnt    	(*m_peek)(scnt);	/* is mask/comp match, use this to peek */
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

//extern memmap_t cpumap[PAGES];
extern memmap_t cpumap[];


/*****************************************************************************/
/* Funktions- und Speicheradressen fr 6502-Speicherzugriffe */

// default add/rm trap functions for CPU bank and memory banks respectively
trap_t *add_cpu_trap(bank_t *bank, scnt trapaddr, void (*exec)(CPU *cpu, scnt addr), const char *name);
trap_t *rm_cpu_trap(bank_t *bank, scnt trapaddr);

trap_t *add_mem_trap(bank_t *bank, scnt trapaddr, void (*exec)(CPU *cpu, scnt addr), const char *name);
trap_t *rm_mem_trap(bank_t *bank, scnt trapaddr);

scnt bank_mem_peek(bank_t *bank, saddr addr);
void bank_mem_poke(bank_t *bank, saddr addr, scnt val);

scnt bank_cpu_peek(bank_t *bank, saddr addr);
void bank_cpu_poke(bank_t *bank, saddr addr, scnt val);

static inline scnt getbyt(saddr a) {
	register scnt bank = a >> 12;
	register scnt offset = a & 0xfff;
	memmap_t *cpupage = &cpumap[bank];

	if (cpupage->mask && ((offset & cpupage->mask) == cpupage->comp) && cpupage->m_rd != NULL) {
		return cpupage->m_rd(offset);
	}

	meminfo_t *inf = cpupage->inf;

        if(inf->mf_rd != NULL) {
                return(inf->mf_rd(inf, a));
        }
        if(inf->mt_rd != NULL) {
             	return(inf->mt_rd[offset]);
        }
        return(a>>8);

}

static inline scnt peekbyt(saddr a) {
	register scnt bank = a >> 12;
	register scnt offset = a & 0xfff;
	memmap_t *cpupage = &cpumap[bank];

	if (cpupage->mask && ((offset & cpupage->mask) == cpupage->comp)) {
		if (cpupage->m_peek) {
			return cpupage->m_peek(offset);
		}
		return 0;	// no side effects
	}

	meminfo_t *inf = cpupage->inf;

        if(inf->mf_rd != NULL) {
		if (inf->mf_peek) {
                	return(inf->mf_peek(inf, a));
		}
                return 0;	// no side effects
        }
        if(inf->mt_rd != NULL) {
             	return(inf->mt_rd[offset]);
        }
        return(a>>8);
}

static inline scnt getaddr(saddr a) {

	return (getbyt(a) & 0xff) | ((getbyt(a+1) & 0xff) << 8);
}


static inline void setbyt(saddr a, scnt b) {
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
                inf->mf_wr(inf, a,b);
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


int loadrom(char *fname, uchar *mem, size_t len);

