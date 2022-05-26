
extern int c64;
extern int fancy;
extern int exrom;
extern int game;
extern int xwin;
extern int wide;
extern int color;
extern int loram;
extern int hiram;
extern int charen;

#define	MAXLINE		200

typedef		unsigned short int	scnt;
typedef 	unsigned char  		uchar;
typedef 	unsigned long		cnt;
typedef		unsigned char		byte;

typedef struct CPU {
		scnt		pc;
		
		scnt		sp;
		scnt		a;
		scnt		x;
		scnt		y;

		scnt		sr;
} CPU;

extern 	int	hirq;
extern	int	hnmi;
extern 	int	dismode;
extern 	int 	traplines;

#define	NEG		128
#define	OVL		64
#define	STRUE		32
#define	BRK		16
#define	DEC		8
#define	IRQ		4
#define	ZERO		2
#define	CARRY		1

int cpu_init(void);	/* init trap etc */
int cpu_run(void);	/* start execution at RESET address */

/* Counter-Verwaltung fr im CPU-Takt taktende Counter */
/* Die Counter werden abw„rts gez„hlt, bei erreichen von NULL wird */
/* die angegebene Routine aufgerufen und der Z„hler mit dem */
/* Rckgabewert gesetzt */

int cnt_init(cnt initval, int exec(int cntnr));
int cntset(int cntnr, cnt val);
int cntget(int cntnr, cnt *val);


