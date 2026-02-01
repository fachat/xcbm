
#include 	<stdio.h>

/* from lib65816 */
#include  	"lib65816/cpu.h"
#include  	"lib65816/cpuevent.h"

#include  	"log.h"
#include  	"types.h"
#include  	"alarm.h"
#include  	"bus.h"
#include	"cpu.h"
#include	"emu65816.h"
#include	"timer.h"
#include 	"mem.h"
#include 	"speed.h"
#include 	"mon.h"
#include 	"stop.h"
#include 	"config.h"

#define	PAGES	256
#define	MAXLINE	200

BUS		bus;
CPU		cpu;

memmap_t cpumap[PAGES];

void struct2cpu(CPU*);
void cpu2struct(CPU*);

int	hnmi=0;
int	hirq=0;

typedef void (*sim_f)();

int	xxmode =0;

uchar zero,carry,overfl,neg,irq,dez,cbrk;
int err;

static bank_t cpubank = {
	"cpu",
	add_cpu_trap,
	rm_cpu_trap,
	bank_cpu_peek,
	bank_cpu_poke,
	cpumap,
	PAGESMASK
};

#define next(a)         advance_clock(&(cpu.bus->actx), (a))

const char *cpu_name(CPU *cpu) {
        return cpu->name;
}

saddr cpu_pc(CPU *cpu) {
        return cpu->pc;
}

unsigned char cpu_st(CPU *cpu) {
        return cpu->sr;
}

/*******************************************************************/

void cpu_set_trace(int flag) {
        if (flag) {
                cpu.flags |= CPUFLG_TRACE;
		CPU_setTrace(1);
        } else {
                cpu.flags &= ~CPUFLG_TRACE;
		CPU_setTrace(0);
        }
}

void cpu_set_irq(scnt int_mask, uchar flag) {
        if (((hirq & int_mask) && !flag)
                || (!(hirq & int_mask) && flag)) {
                logout(0, "set IRQ %02x to %d", int_mask, flag);
        }

        if (flag) {
                CPU_addIRQ(int_mask);
                hirq |= int_mask;
        } else {
                CPU_clearIRQ(int_mask);
                hirq &= ~int_mask;
        }
}

void cpu_set_nmi(scnt int_mask, uchar flag) {
        if (((hnmi & int_mask) && !flag)
                || (!(hnmi & int_mask) && flag)) {
                logout(0, "set NMI %02x to %d", int_mask, flag);
        }

        if (flag) {
                if (!hnmi) {
                        CPU_nmi();
                }
                hnmi |= int_mask;
        } else {
                hnmi &= ~int_mask;
        }
}

void cpu2struct(CPU *cpu){
	cpu->a = A.W;
	cpu->x = X.W;
	cpu->y = Y.W;
	cpu->sp = S.W;
	cpu->sr = P;
	// TODO: E, D, DB, high bytes
}

void struct2cpu(CPU *cpu){
	A.W = cpu->a;
	X.W = cpu->x;
	Y.W = cpu->y;
	S.W = cpu->sp;
	P = cpu->sr;
}

struct CPUEvent timer_ev;

#define	TIMER_INTERVAL	2

void timer_handler( word32 timestamp ) {

	void (*v)(CPU*, scnt);

//	printf("timer_handler\n\r", timestamp);

                // this may be changed to signal ctrl-c to the actual emulated machine
/*
                if (stop_ack_flag()) {
                        cpu2struct(&cpu);
                        mon_line(&cpu);
                        struct2cpu(&cpu);
                }
*/
                if(v=trap6502(cpu.pc)) {
                        cpu2struct(&cpu);
                        v(&cpu, cpu.pc);
                        struct2cpu(&cpu);
                }

	// handle alarms. This is needed to handle the alarms that create timer
	// interrupts
	next(TIMER_INTERVAL);

	// currently we do impedance mismatch by calling during
	// basically every instruction...
	CPUEvent_schedule( &timer_ev, TIMER_INTERVAL, timer_handler);
}

extern FILE *flog;
	
int cpu_run(void){

	struct2cpu(&cpu);

	CPUEvent_initialize();

	CPUEvent_schedule( &timer_ev, 10000, timer_handler);

	CPU_reset();

	CPU_setDbgOutfile(flog);

	//CPU_setTrace(1);

	CPU_run();

	return(0);
}

void cpu_res() {
	CPU_reset();
}

byte MEM_readMem(word32 address, word32 timestamp, word32 emulFlags) {
	byte v = getbyt(address & cpu.mask);
	//logout(0, "readMem (%05x) -> %02x", address, v);
	return v;
}

byte MEM_peekMem(word32 address, word32 timestamp, word32 emulFlags) {
	byte v = peekbyt(address & cpu.mask);
	//logout(0, "peekMem (%05x) -> %02x", address, v);
	return v;
}

void MEM_writeMem(word32 address, byte b, word32 timestamp) {
	//logout(0, "writeMem (%06x, masked=%06x) -> %02x", address, address & cpu.mask, b);
	setbyt(address & cpu.mask, b);
}

void EMUL_handleWDM(byte opcode, word32 timestamp) {
}


CPU *cpu_init(const char *n, int cyclespersec, int msperframe, int cmos, int addrmask) {

#if 0
	if (cmos) {
		simp = simcmos;
	} else {
		simp = sim;
	}
#endif

	alarm_context_init(&bus.actx, "main cpu");

	cpu.name = n;
	cpu.mask = addrmask;
	cpu.bus = &bus;
	bus.cpu = &cpu;

	speed_init(&cpu.bus->actx, cyclespersec, msperframe);

	CPU_reset();

	//mycpu_reset(&cpu);

	mon_register_bank(&cpubank);

	return &cpu;
}

static bank_t *peekbank;
static unsigned char peek(int addr) {
	return peekbank->peek(peekbank, addr);
}

int cpu_log(CPU *cpu, char *line, int maxlen) {

	return CPU_log(line, maxlen);
}

// needed in lib65816 for unknown reasons (referenced as extern)
int no_io = 0;
int trace = 1;

int cpu_dis(bank_t *bank, int addr, unsigned char *stat, char *line, int maxlen) {

	peekbank = bank;

	return CPU_dis(line, maxlen, addr, stat, &peek);
}

