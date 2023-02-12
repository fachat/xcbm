
int mem_init(char *name[],int,int);

void update_mem(int);

/*****************************************************************************/
/* some needed structs */

#define	MAXTRAPS	16

typedef struct {
		scnt		adr;
		void		(*exec)(scnt trapadr, CPU *cpu);
		const char	*name;
} trap;

/* information on a memory page (4k) */
typedef struct {
                uchar   *mt_wr;
		uchar	*mt_rd;
                int     ntraps;
                trap    traplist[MAXTRAPS];
                void    (*mf_wr)(scnt,scnt);
                scnt    (*mf_rd)(scnt);
} meminfo;

/* entry in the CPU's virtual address space, 16x 4k */
typedef struct {
		scnt		mask;			/* mask for special bits */
		scnt		comp;			/* compare after mask */
                void    	(*m_wr)(scnt,scnt);	/* if mask/comp match, use this to write */
                scnt    	(*m_rd)(scnt);		/* is mask/comp match, use this to read */

                int     	rd;	/* current entry in the memtab[] array */
                int     	wr;	/* current entry in the memtab[] array */
                meminfo 	i;	/* copy of memtab entry */

		uchar		*vr;	/* video memory address */
} mt;


/*****************************************************************************/
/* Funktions- und Speicheradressen fr 6502-Speicherzugriffe */

extern unsigned char *mem;
extern mt m[];
extern meminfo memtab[];

static inline scnt getbyt(scnt a) {
	register scnt bank = a >> 12;

	if (m[bank].mask && ((a & m[bank].mask) == m[bank].comp)) {
		return m[bank].m_rd(a);
	}

        if(m[bank].i.mf_rd != NULL) {
		//logout(0,"read address %04x gives function call at %p",(int)a,m[bank].i.mf_rd); 
                return(m[bank].i.mf_rd(a));
        }
        if(m[bank].i.mt_rd != NULL) {
		//logout(0,"read address %04x gives %02x",(int)a,(int)m[bank].i.mt_rd[a&0xfff]); 
             	return(m[bank].i.mt_rd[a&0xfff]);
        }
        return(a>>8);

}

static inline void setbyt(scnt a, scnt b) {
	register scnt bank =  a >> 12;

#if 0		/* TODO: implement watchpoints */
if (a >= 0x0404 && a < 0x0504) {
	logout(0, "writing to %04x <- %02x", a, b);
}
#endif

	if ((m[bank].mask != 0) && ((a & m[bank].mask) == m[bank].comp)) {
		//logout(0, "masked write (a=%04x, mask=%04x, comp=%04x, val=%02x)", a, m[bank].mask, m[bank].comp, b);
		m[bank].m_wr(a,b);
	}
        if(m[bank].i.mt_wr != NULL) {
		//logout(0,"write address %04x,%02x",(int)a,(int)b); 
             	m[bank].i.mt_wr[a&0xfff] = b;
        }
        if(m[bank].i.mf_wr != NULL) {
		//logout(0,"write address %04x,%02x gives function call at %p",(int)a,(int)b,m[bank].i.mf_wr); 
                m[bank].i.mf_wr(a,b);
        }
}

#define getvbyt(a)       (m[(a)>>12].vr[(a)&0xfff])

#define getadr(a)       (getbyt(a)+256*getbyt(a+1))

/*****************************************************************************/
/* "Trap" setzen fr den Einsprung in C-Code (System-Eulation)) */

int settrap(int mempage, scnt trapadr, void execadr(scnt trapadr, CPU *cpureg),
		 const char *name);
int rmtrap(int mempage, scnt trapadr);
void clrtrap(void);

void (*trap6502(scnt))(scnt,CPU*);

/*****************************************************************************/
/* read/write functions fuer memory page setzen */

void setrd(int mempage, scnt (*func)(scnt));
void setwr(int mempage, void (*func)(scnt,scnt));

/*****************************************************************************/
/* internals */

void update_mem(int mempage);
void mem_exit(void);
int loadrom(char *fname, size_t offset, size_t len);
void updatemr(int page, int newpage);
void updatemw(int page, int newpage);

