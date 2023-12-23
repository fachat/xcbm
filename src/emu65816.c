
#include 	<stdio.h>

/* from lib65816 */
#include  	"cpu.h"
#include  	"cpuevent.h"

#include  	"log.h"
#include  	"types.h"
#include  	"alarm.h"
#include  	"bus.h"
#include	"emu6502.h"
#include	"timer.h"
#include	"emucmd.h"
#include 	"mem.h"
#include 	"speed.h"
#include 	"mon.h"
#include 	"config.h"
#include	"asm6502.h"

#define	MAXLINE	200

BUS		bus;
CPU		cpu;

memmap_t cpumap[PAGES];

void struct2cpu(CPU*);
void cpu2struct(CPU*);

int	hnmi=0;
int	hirq=0;
int 	dismode	=0;
int 	traplines =0;

typedef void (*sim_f)();

static sim_f *simp;

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

void logass(CPU *cpu){
	char l[MAXLINE];
	
/*	sprintf(l,"\033[15;1H%04x %02x %02x %02x %02x %02x \n  ",*/
	sprintf(l,"% 8ld %04x A:%02x X:%02x Y:%02x P:%02x  S:%c%c%c%c%c%c%c%c            \n  ",
		cpu->bus->actx.clk,
		cpu->pc,cpu->a,cpu->x,cpu->y,cpu->sp,
		cpu->sr & 0x80 ? 'N' : '-',
		cpu->sr & 0x40 ? 'V' : '-',
		cpu->sr & 0x20 ? '1' : '-',
		cpu->sr & 0x10 ? 'B' : '-',
		cpu->sr & 0x08 ? 'D' : '-',
		cpu->sr & 0x04 ? 'I' : '-',
		cpu->sr & 0x02 ? 'Z' : '-',
		cpu->sr & 0x01 ? 'C' : '-'
		);

	dis6502(&cpubank, cpu->pc, l+47, MAXLINE-47);

	logout(0, l);
}


/*******************************************************************/

void cpu2struct(CPU *cpu){
	cpu->sr=STRUE;
#if 0
	if(carry()) 	cpu->sr+=CARRY;
	if(zero())  	cpu->sr+=ZERO;
	if(overfl())	cpu->sr+=OVL;
	if(neg())	cpu->sr+=NEG;
	if(isbrk())	cpu->sr+=BRK;
	if(dez())	cpu->sr+=DEC;
	if(irq())	cpu->sr+=IRQ;
#endif
}

void struct2cpu(CPU *cpu){
	carry		=cpu->sr&CARRY;
	zero		=cpu->sr&ZERO;
	neg		=cpu->sr&NEG;
	cbrk		=cpu->sr&BRK;
	dez		=cpu->sr&DEC;
	overfl		=cpu->sr&OVL;
	irq		=cpu->sr&IRQ;
}

struct CPUEvent timer_ev;

void timer_handler( word32 timestamp ) {
//	printf("timer_handler\n\r", timestamp);

	CPUEvent_schedule( &timer_ev, 10000, timer_handler);
}

//struct CPUEvent timer_ev = {
//	NULL, NULL, 10000, timer_handler
//};


	
int cpu_run(void){
	scnt c;
	void (*v)(CPU*, scnt);
	//mycpu_reset(&cpu);
	struct2cpu(&cpu);

	CPUEvent_initialize();

	CPUEvent_schedule( &timer_ev, 10000, timer_handler);

	CPU_reset();

	CPU_setTrace(1);

	CPU_run();
	
	do{
/*if(dismode || hirq) printf("\n\nhirq=%d, irq=%d, hnmi=%d\n",hirq,irq,hnmi);*/

		if (is_mon()) {
			cpu2struct(&cpu);
			mon_line(&cpu);
			struct2cpu(&cpu);
		}
#if 0
		if(hirq && !(irq)) {
			aclb();
			cpu2struct(&cpu);
logout(0,"irq: push %04x as rti address - set pc to IRQ address %04x", cpu.pc, getadr(0xfffe));
			phpc();
			phbyt(cpu.sr);
			cpu.pc=getadr(0xfffe);
			asei();
		}
                if(hnmi) {
			hnmi=0;
                        cpu2struct(&cpu);
                        phpc();
                        phbyt(cpu.sr);
                        cpu.pc=getadr(0xfffa);
                        cpu.sr |= IRQ;
			struct2cpu(&cpu);
                }
#endif
                if(v=trap6502(cpu.pc)) {
			cpu2struct(&cpu);
			v(&cpu, cpu.pc);
			struct2cpu(&cpu);
		}

 		if(traplines) {
			if(!(--traplines))
				dismode =2;
		}
		if(config_is_trace_enabled()) {
			cpu2struct(&cpu);
			logass(&cpu);
		}
		//c=getbyt(cpu.pc);
		//(*simp[c])();
		//inc_time(10);
/*
		if(dismode>1) {
			int er;
			while( (er=command()) == 1 ) ;
			if( er==2 )
				break;
		}
*/
	} while(!err);
	
	return(0);
}

byte MEM_readMem(word32 address, word32 timestamp, word32 emulFlags) {
	return getbyt(address);
}

void MEM_writeMem(word32 address, byte b, word32 timestamp) {
	setbyt(address, b);
}

void EMUL_handleWDM(byte opcode, word32 timestamp) {
}


CPU *cpu_init(const char *n, int cyclespersec, int msperframe, int cmos) {

#if 0
	if (cmos) {
		simp = simcmos;
	} else {
		simp = sim;
	}
#endif

	alarm_context_init(&bus.actx, "main cpu");

	cpu.name = n;
	cpu.bus = &bus;
	bus.cpu = &cpu;

	speed_init(&cpu, cyclespersec, msperframe);

	CPU_reset();

	//mycpu_reset(&cpu);

	mon_register_bank(&cpubank);

	return &cpu;
}


