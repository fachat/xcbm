
		
typedef struct CPU {
		const char 	*name;

		BUS		*bus;

		scnt		pc;
		
		scnt		sp;
		scnt		a;
		scnt		x;
		scnt		y;

		scnt		sr;

		//alarm_context_t	actx;
		alarm_t		speed;
} CPU;

#define	NEG		128
#define	OVL		64
#define	STRUE		32
#define	BRK		16
#define	DEC		8
#define	IRQ		4
#define	ZERO		2
#define	CARRY		1

CPU *cpu_init(const char *name, int cyclespersec, int msperframe, int cmos);	/* init trap etc */

int cpu_run(void);	/* start execution at RESET address */

void cpu_set_irq(scnt int_mask, uchar flag);

void cpu_set_nmi(scnt int_mask, uchar flag);

int cpu_is_irq();

