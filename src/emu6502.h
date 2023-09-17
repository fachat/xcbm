
		
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

CPU *cpu_init(const char *name, int cyclespersec, int msperframe, int cmos);	/* init trap etc */
int cpu_run(void);	/* start execution at RESET address */

void maincpu_reset();

int logcpu(CPU *cpu, char *line, int maxlen);

static inline void cpu_set_irq(scnt int_mask, uchar flag) {
        if (((hirq & int_mask) && !flag)
                || (!(hirq & int_mask) && flag)) {
                logout(0, "set IRQ %02x to %d", int_mask, flag);
        }

        if (flag) {
                hirq |= int_mask;
        } else {
                hirq &= ~int_mask;
        }
}

static inline void cpu_set_nmi(scnt int_mask, uchar flag) {
        if (((hnmi & int_mask) && !flag)
                || (!(hnmi & int_mask) && flag)) {
                logout(0, "set NMI %02x to %d", int_mask, flag);
        }

        if (flag) {
                hnmi |= int_mask;
        } else {
                hnmi &= ~int_mask;
        }
}


