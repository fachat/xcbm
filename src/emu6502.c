
#include 	<stdio.h>

#include  	"log.h"
#include  	"types.h"
#include  	"alarm.h"
#include  	"bus.h"
#include	"emu6502.h"
#include	"timer.h"
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
int	is_ill = 0;

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
	int ll; 

	ll = snprintf(l, MAXLINE, "% 8ld", cpu->bus->actx.clk);

	ll += logcpu(cpu, l+ll, MAXLINE - ll);

	dis6502(&cpubank, cpu->pc, l+ll-1, MAXLINE-ll);

	logout(0, l);
}

int logcpu(CPU *cpu, char *line, int maxlen){
	
	return snprintf(line, maxlen, "  %04x A:%02x X:%02x Y:%02x P:%02x  S:%c%c%c%c%c%c%c%c   ",
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
}

void cpu_reset(CPU *cpu){
	cpu->pc=getadr(0xfffc);
	cpu->sr=IRQ+STRUE;
	err=0;
}

// TODO: add time offset to getbyt/getadr, so fetches are done at correct cycle
 
#define azp(a)		getbyt(a+1)
#define azpx(a)		((getbyt(a+1)+cpu.x)&0xff)
#define azpy(a)		((getbyt(a+1)+cpu.y)&0xff)
#define aabs(a)		getadr(a+1)
#define aabsx(a)	(getadr(a+1)+cpu.x)
#define aabsy(a)	(getadr(a+1)+cpu.y)
#define aindx(a)	getadr(getbyt(a+1)+cpu.x)
#define aindy(a)	(getadr(getbyt(a+1))+cpu.y)
#define aabsi(a)	getadr(getadr(a+1))

#define zp(a)		getbyt(azp(a))
#define zpx(a)		getbyt(azpx(a))
#define zpy(a)		getbyt(azpy(a))
#define abs(a)		getbyt(aabs(a))
#define absx(a)		getbyt(aabsx(a))
#define absy(a)		getbyt(aabsy(a))
#define indx(a)		getbyt(aindx(a))
#define indy(a)		getbyt(aindy(a))
#define imm(a)		getbyt(a+1)

#define setnz(a)	neg=a&0x80;zero=!a		/* set neg&zero */
#define setnza(a)	neg=a&0x80;zero=!(a&0xff)	/* set neg&zero */
#define setnv(a)	neg=(a)&0x80;overfl=(a)&0x40;	/* set neg & overfl */
#define setc(a)		carry=a>255			/* set c if a >255 */
#define setc1(a)	carry=a&0x01			/* set c if a&0x01 */
#define setc2(a)	carry=a<256			/* set c if a <256 */
#define setsignv(a)	overfl=0	/*	(((a)&0x80)^(((a)&0x100)>>1))*/
							/* set v on sign overfl */

#define carry1()   	(carry?1:0)			/* 1 if carry set 0 else */
#define carry()		carry
#define zero()		zero
#define overfl()	overfl
#define neg()		neg
#define isbrk()		cbrk
#define irq()		irq
#define dez()		dez

#define asec()		carry=1
#define aclc()		carry=0
#define asei()		irq=1
#define acli()		irq=0
#define ased()		dez=1
#define acld()		dez=0
#define aclv()		overfl=0
#define aseb()		cbrk=1
#define aclb()		cbrk=0

#define phbyt(a)	setbyt(256+cpu.sp,a);cpu.sp=(cpu.sp-1)&0xff
#define plbyt()		getbyt(256+(cpu.sp=(cpu.sp+1)&0xff))
#define phpc()		phbyt(cpu.pc/256);phbyt(cpu.pc&0xff)
#define plpc()		cpu.pc=plbyt();cpu.pc+=256*plbyt()

#define	next(a)		advance_clock(&(cpu.bus->actx), (a))

void ill(){
	is_ill=1;
	logout(4,"Illegal Intruction %02x %02x %02x at adress %04x\n",
		getbyt(cpu.pc),getbyt(cpu.pc+1),getbyt(cpu.pc+2),cpu.pc);
}

void nop(){
	cpu.pc+=1;
}

void brk(){
logout(0,"brk @ $%04x",cpu.pc);
	aseb();
	asei();
	cpu.pc+=2;
	cpu2struct(&cpu);
	phpc();
	phbyt(cpu.sr);
	cpu.pc=getadr(0xfffe);
	next(7);	// clock cycles;
}

void php(){
	cpu2struct(&cpu);
	phbyt(cpu.sr);
	cpu.pc++;
	next(3);	// clock cycles;
}

void jsr_abs(){
	register scnt a=getadr(cpu.pc+1); 
	cpu.pc+=2;
	phpc();
	cpu.pc=a;
	next(6);	// clock cycles;
}

void rti(){
	cpu.sr=plbyt();
	plpc();
logout(0, "Pulling %04x from stack as new PC", cpu.pc);
	struct2cpu(&cpu);
	next(6);	// clock cycles;
}

void pha(){
	phbyt(cpu.a);
	cpu.pc++;
	next(6);	// clock cycles;
}

void phx(){
	phbyt(cpu.x);
	cpu.pc++;
	next(6);	// clock cycles;
}

void phy(){
	phbyt(cpu.y);
	cpu.pc++;
	next(6);	// clock cycles;
}

void rts(){
	plpc();
	cpu.pc++;
	next(6);
}

void pla(){
	register scnt a=cpu.a=plbyt();
	setnz(a);
	cpu.pc++;
	next(4);
}

void plx(){
	register scnt a=cpu.x=plbyt();
	setnz(a);
	cpu.pc++;
	next(4);
}

void ply(){
	register scnt a=cpu.y=plbyt();
	setnz(a);
	cpu.pc++;
	next(4);
}

void plp(){
	cpu.sr=plbyt();
	struct2cpu(&cpu);
	cpu.pc+=1;
	next(4);
}

void clc(){
	aclc();
	cpu.pc+=1;
	next(2);
}
void cli(){
	acli();
	cpu.pc++;
	next(2);
}
void sei(){
	asei();
	cpu.pc+=1;
	next(2);
}
void clv(){
	aclv();
	cpu.pc+=1;
	next(2);
}
void cld(){
	acld();
	cpu.pc+=1;
	next(2);
}
void sed(){
	ased();
	cpu.pc+=1;
	next(2);
}
void sec(){
	asec();
	cpu.pc+=1;
	next(2);
}

void ora_indx(){
	register scnt a=cpu.a|=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(6);
}
void ora_zp(){
	register scnt a=cpu.a|=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(3);
}
void ora_imm(){
	register scnt a=cpu.a|=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(2);
}
void ora_abs(){
	register scnt a=cpu.a|=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);
}
void ora_indy(){
	register scnt a=cpu.a|=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(5); 	// TODO: page crossing
}
void ora_zpx(){
	register scnt a=cpu.a|=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(4);
}
void ora_absy(){
	register scnt a=cpu.a|=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO: page crossing
}
void ora_absx(){
	register scnt a=cpu.a|=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO: page crossing
}

#define asl_x(len) \
	register scnt o=getbyt(a)<<1;\
	setc(o);\
	o&=0xff;\
	setnz(o);\
	setbyt(a,o);\
	cpu.pc+=len

void asl_acc(){
	register scnt o=cpu.a<<1;
	setc(o);
	o&=0xff;
	setnz(o);
	cpu.a=o;
	cpu.pc+=1;
	next(2);
}
void asl_zp(){
	register scnt a=azp(cpu.pc);
	asl_x(2);
	next(5);
}
void asl_abs(){
	register scnt a=aabs(cpu.pc);
	asl_x(3);
	next(6);
}
void asl_zpx(){
	register scnt a=azpx(cpu.pc);
	asl_x(2);
	next(6);
}
void asl_absx(){
	register scnt a=aabsx(cpu.pc);
	asl_x(3);
	next(7);
}

void and_indx(){
	register scnt a=cpu.a&=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(6);
}
void and_zp(){
	register scnt a=cpu.a&=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(3);
}
void and_imm(){
	register scnt a=cpu.a&=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(2);
}
void and_abs(){
	register scnt a=cpu.a&=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);
}
void and_indy(){
	register scnt a=cpu.a&=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(5);	// TODO page crossing
}
void and_zpx(){
	register scnt a=cpu.a&=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(4);
}
void and_absy(){
	register scnt a=cpu.a&=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page crossing
}
void and_absx(){
	register scnt a=cpu.a&=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page crossing
}

#define rol_x(len) \
	register scnt o=(getbyt(a)<<1)+carry1();\
	setc(o);\
	o&=0xff;\
	setnz(o);\
	setbyt(a,o);\
	cpu.pc+=len

void rol_zp(){
	register scnt a=azp(cpu.pc);
	rol_x(2);
	next(5);
}
void rol_abs(){
	register scnt a=aabs(cpu.pc);
	rol_x(3);
	next(6);
}
void rol_zpx(){
	register scnt a=azpx(cpu.pc);
	rol_x(2);
	next(6);
}
void rol_absx(){
	register scnt a=aabsx(cpu.pc);
	rol_x(3);
	next(7);
}
void rol_acc(){
	register scnt o=(cpu.a<<1)+carry1();
	setc(o);
	o&=0xff;
	setnz(o);
	cpu.a=o;
	cpu.pc+=1;
	next(2);
}

void bit_zp(){
	register scnt o=zp(cpu.pc);
	zero=!(cpu.a&o);
	setnv(o);
	cpu.pc+=2;
	next(3);
}
void bit_abs(){
	register scnt o=abs(cpu.pc);
	zero=!(cpu.a&o);
	setnv(o);
	cpu.pc+=3;
	next(4);
}

void bpl(){
	register scnt a=imm(cpu.pc);
	if(!neg()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void bmi(){
	register scnt a=imm(cpu.pc);
	if(neg()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void bvc(){
	register scnt a=imm(cpu.pc);
	if(!overfl()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void bvs(){
	register scnt a=imm(cpu.pc);
	if(overfl()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void bcc(){
	register scnt a=imm(cpu.pc);
	if(!carry()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void bcs(){
	register scnt a=imm(cpu.pc);
	if(carry()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void bne(){
	register scnt a=imm(cpu.pc);
	if(!zero()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void beq(){
	register scnt a=imm(cpu.pc);
	if(zero()) {
		cpu.pc=cpu.pc+a+2-256*(a>127);
		next(3);	// TODO page xing
	} else {
		cpu.pc+=2;
		next(2);
	}
}

void jmp_abs(){
	cpu.pc=aabs(cpu.pc);
	next(3);
}
void jmp_absi(){
	cpu.pc=aabsi(cpu.pc);
	next(5);
}

void eor_indx(){
	register scnt a=cpu.a^=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(6);
}
void eor_zp(){
	register scnt a=cpu.a^=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(3);
}
void eor_imm(){
	register scnt a=cpu.a^=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(2);
}
void eor_abs(){
	register scnt a=cpu.a^=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);
}
void eor_indy(){
	register scnt a=cpu.a^=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(5);	// TODO page xing
}
void eor_zpx(){
	register scnt a=cpu.a^=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(4);
}
void eor_absy(){
	register scnt a=cpu.a^=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}
void eor_absx(){
	register scnt a=cpu.a^=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}

#define lsr_x(len) \
	register scnt o=getbyt(a);\
	setc1(o);\
	o>>=1;\
	setnz(o);\
	setbyt(a,o);\
	cpu.pc+=len

void lsr_zp(){
	register scnt a=azp(cpu.pc);
	lsr_x(2);
	next(5);
}
void lsr_zpx(){
	register scnt a=azpx(cpu.pc);
	lsr_x(2);
	next(6);
}
void lsr_abs(){
	register scnt a=aabs(cpu.pc);
	lsr_x(3);
	next(6);
}
void lsr_absx(){
	register scnt a=aabsx(cpu.pc);
	lsr_x(3);
	next(7);
}
void lsr_acc(){
	register scnt o=cpu.a;
	setc1(o);
	o>>=1;
	setnz(o);
	cpu.a=o;
	cpu.pc++;
	next(2);
}

#define ror_x(len) \
	register scnt o=getbyt(a)+(carry1()?256:0);\
	setc1(o);\
	o>>=1;\
	setnz(o);\
	setbyt(a,o);\
	cpu.pc+=len
	
void ror_zp(){
	register scnt a=azp(cpu.pc);
	ror_x(2);
	next(5);
}
void ror_zpx(){
	register scnt a=azpx(cpu.pc);
	ror_x(2);
	next(6);
}
void ror_abs(){
	register scnt a=aabs(cpu.pc);
	ror_x(3);
	next(6);
}
void ror_absx(){
	register scnt a=aabsx(cpu.pc);
	ror_x(3);
	next(7);
}
void ror_acc(){
	register scnt o=cpu.a+(carry1()?256:0);
	setc1(o);
	o>>=1;
	setnz(o);
	cpu.a=o;
	cpu.pc+=1;
	next(2);
}

#define adc_x(len) 	\
		register uchar sa,sx=a;\
		if(!dez) {\
		  sa=cpu.a;\
		  a+=cpu.a+carry1();\
		  overfl=((a^sa)&0x80)&&!((sa^sx)&0x80);\
		  setc(a);\
		  a&=0xff;\
		  setnz(a);\
		} else {\
		  register scnt al,ah;\
		  al=(cpu.a&0x0f)+(a&0x0f)+carry1();\
		  ah=(cpu.a>>4)+(a>>4)+al>15;\
		  if(al>9) al+=6;\
		  zero=!((cpu.a+a+carry1())&0xff);\
		  neg=ah&8;\
		  overfl=(((ah<<4)^cpu.a) & 128) && !((cpu.a^a)&128);\
		  if(ah>9) ah+=6;\
		  carry=ah>15;\
		  a=((ah<<4)|(al&0x0f))&0xff; \
		}\
		cpu.a=a;\
		cpu.pc+=len

void adc_indx(){
	register scnt a=indx(cpu.pc);
	adc_x(2);
	next(6);
}
void adc_zp(){
	register scnt a=zp(cpu.pc);
	adc_x(2);
	next(3);
}
void adc_imm(){
	register scnt a=imm(cpu.pc);
	adc_x(2);
	next(2);
}
void adc_abs(){
	register scnt a=abs(cpu.pc);
	adc_x(3);
	next(4);
}
void adc_indy(){
	register scnt a=indy(cpu.pc);
	adc_x(2);
	next(5);	// TODO page xing
}
void adc_zpx(){
	register scnt a=zpx(cpu.pc);
	adc_x(2);
	next(4);
}
void adc_absy(){
	register scnt a=absy(cpu.pc);
	adc_x(3);
	next(4);	// TODO page xing
}
void adc_absx(){
	register scnt a=absx(cpu.pc);
	adc_x(3);
	next(4);	// TODO page xing
}

void stz_zp(){
	setbyt(azp(cpu.pc),0);
	cpu.pc+=2;
	next(3);
}
void stz_abs(){
	setbyt(aabs(cpu.pc),0);
	cpu.pc+=3;
	next(4);
}
void stz_zpx(){
	setbyt(azpx(cpu.pc),0);
	cpu.pc+=2;
	next(4);
}
void stz_absx(){
	setbyt(aabsx(cpu.pc),0);
	cpu.pc+=3;
	next(5);
}

void sta_indx(){
	setbyt(aindx(cpu.pc),cpu.a);
	cpu.pc+=2;
	next(6);
}
void sty_zp(){
	setbyt(azp(cpu.pc),cpu.y);
	cpu.pc+=2;
	next(3);
}
void sta_zp(){
	setbyt(azp(cpu.pc),cpu.a);
	cpu.pc+=2;
	next(3);
}
void stx_zp(){
	setbyt(azp(cpu.pc),cpu.x);
	cpu.pc+=2;
	next(3);
}
void sty_abs(){
	setbyt(aabs(cpu.pc),cpu.y);
	cpu.pc+=3;
	next(4);
}
void sta_abs(){
	setbyt(aabs(cpu.pc),cpu.a);
	cpu.pc+=3;
	next(4);
}
void stx_abs(){
	setbyt(aabs(cpu.pc),cpu.x);
	cpu.pc+=3;
	next(4);
}
void sta_indy(){
	setbyt(aindy(cpu.pc),cpu.a);
	cpu.pc+=2;
	next(6);
}
void sty_zpx(){
	setbyt(azpx(cpu.pc),cpu.y);
	cpu.pc+=2;
	next(4);
}
void sta_zpx(){
	setbyt(azpx(cpu.pc),cpu.a);
	cpu.pc+=2;
	next(4);
}
void stx_zpy(){
	setbyt(azpy(cpu.pc),cpu.x);
	cpu.pc+=2;
	next(4);
}
void sta_absy(){
	setbyt(aabsy(cpu.pc),cpu.a);
	cpu.pc+=3;
	next(5);
}
void sta_absx(){
	setbyt(aabsx(cpu.pc),cpu.a);
	cpu.pc+=3;
	next(5);
}

void tya(){
	register scnt a=cpu.a=cpu.y;
	setnz(a);
	cpu.pc++;
	next(2);
}

void txa(){
	register scnt a=cpu.a=cpu.x;
	setnz(a);
	cpu.pc++;
	next(2);
}

void txs(){
	cpu.sp=cpu.x;
	cpu.pc++;
	next(2);
}

void tay(){
	register scnt a=cpu.y=cpu.a;
	setnz(a);
	cpu.pc++;
	next(2);
}

void tax(){
	register scnt a=cpu.x=cpu.a;
	setnz(a);
	cpu.pc++;
	next(2);
}

void tsx(){
	register scnt a=cpu.x=cpu.sp;
	setnz(a);
	cpu.pc++;
	next(2);
}

void ldy_imm(){
	register scnt a=cpu.y=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(2);
}
void lda_indx(){
	register scnt a=cpu.a=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(6);
}
void ldx_imm(){
	register scnt a=cpu.x=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(2);
}
void ldy_zp(){
	register scnt a=cpu.y=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(3);
}
void lda_zp(){
	register scnt a=cpu.a=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(3);
}
void ldx_zp(){
	register scnt a=cpu.x=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(3);
}
void lda_imm(){
	register scnt a=cpu.a=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(2);
}
void ldy_abs(){
	register scnt a=cpu.y=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);
}
void lda_abs(){
	register scnt a=cpu.a=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);
}
void ldx_abs(){
	register scnt a=cpu.x=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);
}
void lda_indy(){
	register scnt a=cpu.a=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(5);	// TODO page xing
}
void ldy_zpx(){
	register scnt a=cpu.y=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(4);
}
void lda_zpx(){
	register scnt a=cpu.a=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(4);
}
void ldx_zpy(){
	register scnt a=cpu.x=zpy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
	next(4);
}
void lda_absy(){
	register scnt a=cpu.a=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}
void ldy_absx(){
	register scnt a=cpu.y=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}
void lda_absx(){
	register scnt a=cpu.a=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}
void ldx_absy(){
	register scnt a=cpu.x=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}

void iny(){
	register scnt a=cpu.y=(cpu.y+1)&0xff;
	setnz(a);
	cpu.pc++;
	next(2);
}

void dex(){
	register scnt a=cpu.x=(cpu.x-1)&0xff;
	setnz(a);
	cpu.pc++;
	next(2);
}

void inx(){
	register scnt a=cpu.x=(cpu.x+1)&0xff;
	setnz(a);
	cpu.pc++;
	next(2);
}

void dey(){
	register scnt a=cpu.y=(cpu.y-1)&0xff;
	setnz(a);
	cpu.pc++;
	next(2);
}

#define madd(offset,len) \
	register scnt o=((getbyt(a))+(offset))&0xff;\
	setnz(o);\
	setbyt(a,o);\
	cpu.pc+=len

void dec_zp(){
	register scnt a=azp(cpu.pc);
	madd(-1,2);
	next(5);
}
void dec_zpx(){
	register scnt a=azpx(cpu.pc);
	madd(-1,2);
	next(6);
}
void dec_abs(){
	register scnt a=aabs(cpu.pc);
	madd(-1,3);
	next(6);
}
void dec_absx(){
	register scnt a=aabsx(cpu.pc);
	madd(-1,3);
	next(7);
}

void dec(){
	register scnt a=(cpu.a - 1) & 0xff;
	setnz(a);
	cpu.a = a;
	cpu.pc += 1;
	next(2);
}

void inc(){
	register scnt a=(cpu.a + 1) & 0xff;
	setnz(a);
	cpu.a = a;
	cpu.pc += 1;
	next(2);
}
void inc_zp(){
	register scnt a=azp(cpu.pc);
	madd(1,2);
	next(5);
}
void inc_zpx(){
	register scnt a=azpx(cpu.pc);
	madd(1,2);
	next(6);
}
void inc_abs(){
	register scnt a=aabs(cpu.pc);
	madd(1,3);
	next(6);
}
void inc_absx(){
	register scnt a=aabsx(cpu.pc);
	madd(1,3);
	next(7);
}

void cpy_imm(){
	register scnt a=cpu.y-imm(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(2);
}
void cmp_indx(){
	register scnt a=cpu.a-indx(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(6);
}
void cpy_zp(){
	register scnt a=cpu.y-zp(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(3);
}
void cmp_zp(){
	register scnt a=cpu.a-zp(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(3);
}
void cmp_imm(){
	register scnt a=cpu.a-imm(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(2);
}
void cpy_abs(){
	register scnt a=cpu.y-abs(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
	next(4);
}
void cmp_abs(){
	register scnt a=cpu.a-abs(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
	next(4);
}
void cmp_indy(){
	register scnt a=cpu.a-indy(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(5);	// TODO page xing
}
void cmp_zpx(){
	register scnt a=cpu.a-zpx(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(4);
}
void cmp_absy(){
	register scnt a=cpu.a-absy(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}
void cmp_absx(){
	register scnt a=cpu.a-absx(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
	next(4);	// TODO page xing
}
void cpx_imm(){
	register scnt a=cpu.x-imm(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(2);
}
void cpx_zp(){
	register scnt a=cpu.x-zp(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
	next(3);
}
void cpx_abs(){
	register scnt a=cpu.x-abs(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
	next(4);
}

#define sbc_x(len) \
	register scnt sa,sx=a;\
	sa=cpu.a;\
	a=cpu.a-a-1+carry1();\
	overfl=((a^sa)&0x80)&&!((sa^sx)&0x80);\
	setc2(a);\
	setnza(a);\
	if(!dez) {\
	  cpu.a=a&0xff; \
	} else {\
	  register scnt al,ah;\
	  al=(sa&0x0f)-(sx&0x0f)-1+carry();\
	  if(al&16) al-=6;\
	  ah=(sa>>4)-(sx>>4)-(al&16);\
	  if(ah&16) ah-=6;\
	  cpu.a=((ah&0x0f)<<4)|(al&0x0f);\
	}\
	cpu.pc+=len

void sbc_indx(){
	register scnt a=indx(cpu.pc);
	sbc_x(2);
	next(6);
}
void sbc_zp(){
	register scnt a=zp(cpu.pc);
	sbc_x(2);
	next(3);
}
void sbc_imm(){
	register scnt a=imm(cpu.pc);
	sbc_x(2);
	next(2);
}
void sbc_abs(){
	register scnt a=abs(cpu.pc);
	sbc_x(3);
	next(4);
}
void sbc_indy(){
	register scnt a=indy(cpu.pc);
	sbc_x(2);
	next(5);	// TODO page xing
}
void sbc_zpx(){
	register scnt a=zpx(cpu.pc);
	sbc_x(2);
	next(4);
}
void sbc_absy(){
	register scnt a=absy(cpu.pc);
	sbc_x(3);
	next(4);	// TODO page xing
}
void sbc_absx(){
	register scnt a=absx(cpu.pc);
	sbc_x(3);
	next(4);	// TODO page xing
}

//void (*sim[256])(void)=
sim_f sim[256] =
   {    brk,		ora_indx,	ill,		ill,	ill,		ora_zp,		asl_zp,		ill,
        php,		ora_imm,	asl_acc,	ill,	ill,		ora_abs,	asl_abs,	ill,
	bpl,		ora_indy,	ill,		ill,	ill,		ora_zpx,	asl_zpx,	ill,
   	clc,		ora_absy,	ill,		ill,	ill,		ora_absx,	asl_absx,	ill,

   	jsr_abs,	and_indx,	ill,		ill,	bit_zp,		and_zp,		rol_zp,		ill,
   	plp,		and_imm,	rol_acc,	ill,	bit_abs,	and_abs,	rol_abs,	ill,
   	bmi,		and_indy,	ill,		ill,	ill,		and_zpx,	rol_zpx,	ill,
   	sec,		and_absy,	ill,		ill,	ill,		and_absx,	rol_absx,	ill,

   	rti,		eor_indx,	ill,		ill,	ill,		eor_zp,		lsr_zp,		ill,
   	pha,		eor_imm,	lsr_acc,	ill,	jmp_abs,	eor_abs,	lsr_abs,	ill,
   	bvc,		eor_indy,	ill,		ill,	ill,		eor_zpx,	lsr_zpx,	ill,
   	cli,		eor_absy,	ill,		ill,	ill,		eor_absx,	lsr_absx,	ill,

   	rts,		adc_indx,	ill,		ill,	ill,		adc_zp,		ror_zp,		ill,
   	pla,		adc_imm,	ror_acc,	ill,	jmp_absi,	adc_abs,	ror_abs,	ill,
   	bvs,		adc_indy,	ill,		ill,	ill,		adc_zpx,	ror_zpx,	ill,
   	sei,		adc_absy,	ill,		ill,	ill,		adc_absx,	ror_absx,	ill,
   	
   	ill,		sta_indx,	ill,		ill,	sty_zp,		sta_zp,		stx_zp,		ill,
   	dey,		ill,		txa,		ill,	sty_abs,	sta_abs,	stx_abs,	ill,
   	bcc,		sta_indy,	ill,		ill,	sty_zpx,	sta_zpx,	stx_zpy,	ill,
   	tya,		sta_absy,	txs,		ill,	ill,		sta_absx,	ill,		ill,
   	
   	ldy_imm,	lda_indx,	ldx_imm,	ill,	ldy_zp,		lda_zp,		ldx_zp,		ill,
   	tay,		lda_imm,	tax,		ill,	ldy_abs,	lda_abs,	ldx_abs,	ill,
   	bcs,		lda_indy,	ill,		ill,	ldy_zpx,	lda_zpx,	ldx_zpy,	ill,
   	clv,		lda_absy,	tsx,		ill,	ldy_absx,	lda_absx,	ldx_absy,	ill,
   	
   	cpy_imm,	cmp_indx,	ill,		ill,	cpy_zp,		cmp_zp,		dec_zp,		ill,
   	iny,		cmp_imm,	dex,		ill,	cpy_abs,	cmp_abs,	dec_abs,	ill,
   	bne,		cmp_indy,	ill,		ill,	ill,		cmp_zpx,	dec_zpx,	ill,	
   	cld,		cmp_absy,	ill,		ill,	ill,		cmp_absx,	dec_absx,	ill,
   	
   	cpx_imm,	sbc_indx,	ill,		ill,	cpx_zp,		sbc_zp,		inc_zp,		ill,
   	inx,		sbc_imm,	nop,		ill,	cpx_abs,	sbc_abs,	inc_abs,	ill,
   	beq,		sbc_indy,	ill,		ill,	ill,		sbc_zpx,	inc_zpx,	ill,
   	sed,		sbc_absy,	ill,		ill,	ill,		sbc_absx,	inc_absx,	ill
};

//void (*simcmos[256])(void)=
sim_f simcmos[256] =
   {    brk,		ora_indx,	ill,		ill,	ill,		ora_zp,		asl_zp,		ill,
        php,		ora_imm,	asl_acc,	ill,	ill,		ora_abs,	asl_abs,	ill,
	bpl,		ora_indy,	ill,		ill,	ill,		ora_zpx,	asl_zpx,	ill,
   	clc,		ora_absy,	inc,		ill,	ill,		ora_absx,	asl_absx,	ill,

   	jsr_abs,	and_indx,	ill,		ill,	bit_zp,		and_zp,		rol_zp,		ill,
   	plp,		and_imm,	rol_acc,	ill,	bit_abs,	and_abs,	rol_abs,	ill,
   	bmi,		and_indy,	ill,		ill,	ill,		and_zpx,	rol_zpx,	ill,
   	sec,		and_absy,	dec,		ill,	ill,		and_absx,	rol_absx,	ill,

   	rti,		eor_indx,	ill,		ill,	ill,		eor_zp,		lsr_zp,		ill,
   	pha,		eor_imm,	lsr_acc,	ill,	jmp_abs,	eor_abs,	lsr_abs,	ill,
   	bvc,		eor_indy,	ill,		ill,	ill,		eor_zpx,	lsr_zpx,	ill,
   	cli,		eor_absy,	phy,		ill,	ill,		eor_absx,	lsr_absx,	ill,

   	rts,		adc_indx,	ill,		ill,	stz_zp,		adc_zp,		ror_zp,		ill,
   	pla,		adc_imm,	ror_acc,	ill,	jmp_absi,	adc_abs,	ror_abs,	ill,
   	bvs,		adc_indy,	ill,		ill,	stz_zpx,	adc_zpx,	ror_zpx,	ill,
   	sei,		adc_absy,	ply,		ill,	ill,		adc_absx,	ror_absx,	ill,
   	
   	ill,		sta_indx,	ill,		ill,	sty_zp,		sta_zp,		stx_zp,		ill,
   	dey,		ill,		txa,		ill,	sty_abs,	sta_abs,	stx_abs,	ill,
   	bcc,		sta_indy,	ill,		ill,	sty_zpx,	sta_zpx,	stx_zpy,	ill,
   	tya,		sta_absy,	txs,		ill,	stz_abs,	sta_absx,	stz_absx,	ill,
   	
   	ldy_imm,	lda_indx,	ldx_imm,	ill,	ldy_zp,		lda_zp,		ldx_zp,		ill,
   	tay,		lda_imm,	tax,		ill,	ldy_abs,	lda_abs,	ldx_abs,	ill,
   	bcs,		lda_indy,	ill,		ill,	ldy_zpx,	lda_zpx,	ldx_zpy,	ill,
   	clv,		lda_absy,	tsx,		ill,	ldy_absx,	lda_absx,	ldx_absy,	ill,
   	
   	cpy_imm,	cmp_indx,	ill,		ill,	cpy_zp,		cmp_zp,		dec_zp,		ill,
   	iny,		cmp_imm,	dex,		ill,	cpy_abs,	cmp_abs,	dec_abs,	ill,
   	bne,		cmp_indy,	ill,		ill,	ill,		cmp_zpx,	dec_zpx,	ill,	
   	cld,		cmp_absy,	phx,		ill,	ill,		cmp_absx,	dec_absx,	ill,
   	
   	cpx_imm,	sbc_indx,	ill,		ill,	cpx_zp,		sbc_zp,		inc_zp,		ill,
   	inx,		sbc_imm,	nop,		ill,	cpx_abs,	sbc_abs,	inc_abs,	ill,
   	beq,		sbc_indy,	ill,		ill,	ill,		sbc_zpx,	inc_zpx,	ill,
   	sed,		sbc_absy,	plx,		ill,	ill,		sbc_absx,	inc_absx,	ill
};


/*******************************************************************/

void cpu2struct(CPU *cpu){
	cpu->sr=STRUE;
	if(carry()) 	cpu->sr+=CARRY;
	if(zero())  	cpu->sr+=ZERO;
	if(overfl())	cpu->sr+=OVL;
	if(neg())	cpu->sr+=NEG;
	if(isbrk())	cpu->sr+=BRK;
	if(dez())	cpu->sr+=DEC;
	if(irq())	cpu->sr+=IRQ;
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
	
int cpu_run(void){
	scnt c;
	void (*v)(CPU*, scnt);
	cpu_reset(&cpu);
	struct2cpu(&cpu);
	
	do{
/*if(dismode || hirq) printf("\n\nhirq=%d, irq=%d, hnmi=%d\n",hirq,irq,hnmi);*/

                if(v=trap6502(cpu.pc)) {
			cpu2struct(&cpu);
			v(&cpu, cpu.pc);
			struct2cpu(&cpu);
		}
		if (is_ill || is_mon()) {
			is_ill = 0;
			cpu2struct(&cpu);
			mon_line(&cpu);
			struct2cpu(&cpu);
		}

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

 		if(traplines) {
			if(!(--traplines))
				dismode =2;
		}
		if(config_is_trace_enabled()) {
			cpu2struct(&cpu);
			logass(&cpu);
		}
		c=getbyt(cpu.pc);
		(*simp[c])();
		inc_time(10);

	} while(!err);
	
	return(0);
}


CPU *cpu_init(const char *n, int cyclespersec, int msperframe, int cmos) {

	if (cmos) {
		simp = simcmos;
	} else {
		simp = sim;
	}

	alarm_context_init(&bus.actx, "main cpu");

	cpu.name = n;
	cpu.bus = &bus;
	bus.cpu = &cpu;

	speed_init(&cpu, cyclespersec, msperframe);

	cpu_reset(&cpu);

	mon_register_bank(&cpubank);

	return &cpu;
}


