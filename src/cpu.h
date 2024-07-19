
typedef struct CPU CPU;
typedef struct bank_s bank_t;

const char *cpu_name(CPU *cpu);

saddr cpu_pc(CPU *cpu);

/* flag bits */
#define	CPUFLG_TRACE	0x01

void cpu_set_trace(int flag);

/* status register flags */
#define	NEG		128
#define	OVL		64
#define	STRUE		32
#define	BRK		16
#define	DEC		8
#define	IRQ		4
#define	ZERO		2
#define	CARRY		1

int cpu_run(void);	/* start execution at RESET address */

void cpu_set_irq(scnt int_mask, uchar flag);

void cpu_set_nmi(scnt int_mask, uchar flag);

// returns the interrupt mask with all bits set that currently set an interrupt.
int cpu_is_irq();

// log CPU registers 
int cpu_log(CPU *cpu, char *line, int maxlen);

// print disassembly to line buffer
int cpu_dis(bank_t *bank, int addr, char *line, int maxlen);

void cpu_res();

