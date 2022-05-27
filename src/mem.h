
int mem_init(char *name[],int,int);

void update_mem(int);
void mem_setcpuport(scnt adr,scnt val);

/*****************************************************************************/
/* some needed structs */

#define	MAXTRAPS	16

typedef struct {
		scnt		adr;
		void		(*exec)(scnt trapadr, CPU *cpu);
		const char	*name;
} trap;

typedef struct {
                uchar   *mt_wr;
		uchar	*mt_rd;
                int     ntraps;
                trap    traplist[MAXTRAPS];
                void    (*mf_wr)(scnt,scnt);
                scnt    (*mf_rd)(scnt);
} meminfo;

typedef struct {
                int     	rd;
                int     	wr;
		uchar		*vr;
                meminfo 	i;
} mt;

#define MP_RAM0		0	/* RAM1-15= MP_RAM0+{1-15} */
#define	MP_KERNEL0	16	
#define	MP_KERNEL1	17
#define	MP_BASIC0	18
#define	MP_BASIC1	19
#define	MP_CHAROM	20
#define	MP_ROML0	21
#define	MP_ROML1	22
#define	MP_ROMH0	23
#define	MP_ROMH1	24
#define	MP_IO64		25
#define	MP_NUM		26


/*****************************************************************************/
/* Funktions- und Speicheradressen fr 6502-Speicherzugriffe */

extern unsigned char *mem;
extern mt m[16];
extern meminfo memtab[MP_NUM];

#define setbyt(a,b)     \
{       register scnt c=(a)>>12;\
/*logout(0,"adr=%04x, byte=%02x, mt=%p, mf=%p\n",a,b,m[c].i.mt_wr,m[c].i.mf_wr);*/\
	if(!(a&0xfffe)) mem_setcpuport(a,b); else\
        if(m[c].i.mt_wr) m[c].i.mt_wr[(a)&0xfff]=(char)(b); else\
        if(m[c].i.mf_wr) m[c].i.mf_wr(a,b);\
}

scnt getbyt(scnt);

#define getvbyt(a)       (m[(a)>>12].vr[(a)&0xfff])

/*
#define getbyt(a)       (mf_rd[(a)>>12]?mf_rd[(a)>>12](a):(mt_rd[(a)>>12]?\
        mt_rd[(a)>>12][(a)&0xfff]:(a)>>8))
*/
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

