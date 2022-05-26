
#include 	<stdio.h>

#include  	"log.h"
#include  	"types.h"
#include	"emu6502.h"
#include	"timer.h"
#include	"emucmd.h"
#include 	"mem.h"

#define	MAXLINE	200

CPU		cpu;

void struct2cpu(CPU*);
void cpu2struct(CPU*);

int	hnmi=0;
int	hirq=0;
int 	dismode	=0;
int 	traplines =0;

int	xxmode =0;

/*******************************************************************/

char *kt[] ={ 
     "adc","and","asl","bbr","bbs","bcc","bcs","beq",
     "bit","bmi",
     "bne","bpl","bra","brk","bvc","bvs","clc","cld",
     "cli",
     "clv","cmp","cpx","cpy","dec","dex","dey","eor",
     "inc","inx","iny","jmp","jsr","lda","ldx","ldy",
     "lsr","nop","ora","pha","php","phx","phy","pla",
     "plp","plx","ply","rmb","rol",
     "ror","rti","rts","sbc","sec","sed","sei","smb",
     "sta",
     "stx","sty","stz","tax","tay","trb","tsb","tsx",
     "txa","txs","tya",
};

int cmd[256]=
   { 13, 37, -1, -1, -1, 37,  2, -1, 39, 37,  2, -1, -1, 37,  2, -1,
     11, 37, -1, -1, -1, 37,  2, -1, 16, 37, -1, -1, -1, 37,  2, -1,
     31,  1, -1, -1,  8,  1, 47, -1, 43,  1, 47, -1,  8,  1, 47, -1,
      9,  1, -1, -1, -1,  1, 47, -1, 52,  1, -1, -1, -1,  1, 47, -1,
     49, 26, -1, -1, -1, 26, 35, -1, 38, 26, 35, -1, 30, 26, 35, -1,
     14, 26, -1, -1, -1, 26, 35, -1, 18, 26, -1, -1, -1, 26, 35, -1,
     50,  0, -1, -1, -1,  0, 48, -1, 42,  0, 48, -1, 30,  0, 48, -1,
     15,  0, -1, -1, -1,  0, 48, -1, 54,  0, -1, -1, -1,  0, 48, -1,

     -1, 56, -1, -1, 58, 56, 57, -1, 25, -1, 65, -1, 58, 56, 57, -1,
      5, 56, -1, -1, 58, 56, 57, -1, 67, 56, 66, -1, -1, 56, -1, -1,
     34, 32, 33, -1, 34, 32, 33, -1, 61, 32, 60, -1, 34, 32, 33, -1,
      6, 32, -1, -1, 34, 32, 33, -1, 19, 32, 64, -1, 34, 32, 33, -1,
     22, 20, -1, -1, 22, 20, 23, -1, 29, 20, 24, -1, 22, 20, 23, -1,
     10, 20, -1, -1, -1, 20, 23, -1, 17, 20, -1, -1, -1, 20, 23, -1, 
     21, 51, -1, -1, 21, 51, 27, -1, 28, 51, 36, -1, 21, 51, 27, -1,
      7, 51, -1, -1, -1, 51, 27, -1, 53, 51, -1, -1, -1, 51, 27, -1 
};
      
int adm[256]=
   {  0, 12, -1, -1, -1,  3,  3, -1,  0,  1,  0, -1, -1,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1,
      2, 12, -1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1,
      0, 12, -1, -1, -1,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1,
      0, 12, -1, -1, -1,  3,  3, -1,  0,  1,  0, -1, 14,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  1, -1, -1, -1,  6,  6, -1,

     -1, 12, -1, -1,  3,  3,  3, -1,  0, -1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1,  4,  4,  5, -1,  0,  7,  0, -1, -1,  6, -1, -1,
      1, 12,  1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1,  4,  4,  5, -1,  0,  7,  0, -1,  6,  6,  7, -1,
      1, 12, -1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1, 
      1, 12, -1, -1,  3,  3,  3, -1,  0,  1,  0, -1,  2,  2,  2, -1,
     10, 13, -1, -1, -1,  4,  4, -1,  0,  7, -1, -1, -1,  6,  6, -1 
};

int adl[16]= { 1,2,3,2,2,2,3,3,3,1,2,3,2,2,3,3 };
char *ad1[16]={ "", "#$", "$", "$", "$", "$", "$", "$", "($", "", "$", "", "($", "($", "($", "($" };
char *ad2[16]={ "", "", "", "", ",x", ",y", ",x", ",y", ",x)", "", "", "", ",x)", "),y", ")", ")" };

uchar zero,carry,overfl,neg,irq,dez,cbrk;
int err;

void logass(CPU *cpu){
	char l[MAXLINE];
	int c=getbyt(cpu->pc);
	int o,a,f=0,ad=0;
	
/*	sprintf(l,"\033[15;1H%04x %02x %02x %02x %02x %02x \n  ",*/
	sprintf(l,"%04x %02x %02x %02x %02x %02x            \n  ",
		cpu->pc,cpu->a,cpu->x,cpu->y,cpu->sp,cpu->sr);
	if(cmd[c]<0){
	  sprintf(l+28," %02x illegal opcode!    ",c);
	} else {
	  a=adm[c];
	  o=cmd[c];
	  switch(adl[a]){
	  case 1:
	    sprintf(l+28," %s                        ", kt[o]);
	    break;
	  case 2:
	    if(a!=10) {
	      sprintf(l+28," %s %s%02x%s          ",
	        kt[o],ad1[a],getbyt(cpu->pc+1),ad2[a]);
	      if(a!=1) f=1;
	      ad=getbyt(cpu->pc+1);
	    } else {
	      c=getbyt(cpu->pc+1);
	      sprintf(l+28," %s %s%04x          ",
	        kt[o],ad1[a],cpu->pc+2+c-256*(c>127));
	    }
	    break;
	  case 3:
	    sprintf(l+28," %s %s%04x%s           ",
	      kt[o],ad1[a],getadr(cpu->pc+1),ad2[a]);
	    if(c!=0x4c && c!=0x20) f=1;
	    ad=getadr(cpu->pc+1);
	    break;
	  default:
	    break;
	  }
	}
	if(f) {
	  logout(0,"mem@ %04x = %02x %02x %02x %02x %02x %02x %02x %02x",
	    ad,getbyt(ad),getbyt(ad+1),getbyt(ad+2),getbyt(ad+3),getbyt(ad+4),
	    getbyt(ad+5),getbyt(ad+6),getbyt(ad+7));
	}
	logout(0,l);
}

void cpu_reset(CPU *cpu){
	cpu->pc=getadr(0xfffc);
	cpu->sr=IRQ+STRUE;
	err=0;
}

 
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
#define setnv(a)	neg=a&0x80;overfl=a&0x40	/* set neg & overfl */
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

void ill(){
	err=1;
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
}

void php(){
	cpu2struct(&cpu);
	phbyt(cpu.sr);
	cpu.pc++;
}

void jsr_abs(){
	register scnt a=getadr(cpu.pc+1); 
	cpu.pc+=2;
	phpc();
	cpu.pc=a;
}

void rti(){
	cpu.sr=plbyt();
	plpc();
	cpu.pc++;
	struct2cpu(&cpu);
}

void pha(){
	phbyt(cpu.a);
	cpu.pc++;
}

void rts(){
	plpc();
	cpu.pc++;
}

void pla(){
	register scnt a=cpu.a=plbyt();
	setnz(a);
	cpu.pc++;
}

void plp(){
	cpu.sr=plbyt();
	struct2cpu(&cpu);
	cpu.pc+=1;
}

void clc(){
	aclc();
	cpu.pc+=1;
}
void cli(){
	acli();
	cpu.pc++;
}
void sei(){
	asei();
	cpu.pc+=1;
}
void clv(){
	aclv();
	cpu.pc+=1;
}
void cld(){
	acld();
	cpu.pc+=1;
}
void sed(){
	ased();
	cpu.pc+=1;
}
void sec(){
	asec();
	cpu.pc+=1;
}

void ora_indx(){
	register scnt a=cpu.a|=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ora_zp(){
	register scnt a=cpu.a|=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ora_imm(){
	register scnt a=cpu.a|=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ora_abs(){
	register scnt a=cpu.a|=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void ora_indy(){
	register scnt a=cpu.a|=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ora_zpx(){
	register scnt a=cpu.a|=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ora_absy(){
	register scnt a=cpu.a|=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void ora_absx(){
	register scnt a=cpu.a|=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
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
}
void asl_zp(){
	register scnt a=azp(cpu.pc);
	asl_x(2);
}
void asl_abs(){
	register scnt a=aabs(cpu.pc);
	asl_x(3);
}
void asl_zpx(){
	register scnt a=azpx(cpu.pc);
	asl_x(2);
}
void asl_absx(){
	register scnt a=aabsx(cpu.pc);
	asl_x(3);
}

void and_indx(){
	register scnt a=cpu.a&=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void and_zp(){
	register scnt a=cpu.a&=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void and_imm(){
	register scnt a=cpu.a&=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void and_abs(){
	register scnt a=cpu.a&=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void and_indy(){
	register scnt a=cpu.a&=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void and_zpx(){
	register scnt a=cpu.a&=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void and_absy(){
	register scnt a=cpu.a&=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void and_absx(){
	register scnt a=cpu.a&=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
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
}
void rol_abs(){
	register scnt a=aabs(cpu.pc);
	rol_x(3);
}
void rol_zpx(){
	register scnt a=azpx(cpu.pc);
	rol_x(2);
}
void rol_absx(){
	register scnt a=aabsx(cpu.pc);
	rol_x(3);
}
void rol_acc(){
	register scnt o=(cpu.a<<1)+carry1();
	setc(o);
	o&=0xff;
	setnz(o);
	cpu.a=o;
	cpu.pc+=1;
}

void bit_zp(){
	register scnt o=zp(cpu.pc);
	zero=!(cpu.a&o);
	setnv(o);
	cpu.pc+=2;
}
void bit_abs(){
	register scnt o=abs(cpu.pc);
	zero=!(cpu.a&o);
	setnv(o);
	cpu.pc+=3;
}

void bpl(){
	register scnt a=imm(cpu.pc);
	if(!neg()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void bmi(){
	register scnt a=imm(cpu.pc);
	if(neg()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void bvc(){
	register scnt a=imm(cpu.pc);
	if(!overfl()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void bvs(){
	register scnt a=imm(cpu.pc);
	if(overfl()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void bcc(){
	register scnt a=imm(cpu.pc);
	if(!carry()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void bcs(){
	register scnt a=imm(cpu.pc);
	if(carry()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void bne(){
	register scnt a=imm(cpu.pc);
	if(!zero()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void beq(){
	register scnt a=imm(cpu.pc);
	if(zero()) 
		cpu.pc=cpu.pc+a+2-256*(a>127);
	else
		cpu.pc+=2;
}

void jmp_abs(){
	cpu.pc=aabs(cpu.pc);
}
void jmp_absi(){
	cpu.pc=aabsi(cpu.pc);
}

void eor_indx(){
	register scnt a=cpu.a^=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void eor_zp(){
	register scnt a=cpu.a^=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void eor_imm(){
	register scnt a=cpu.a^=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void eor_abs(){
	register scnt a=cpu.a^=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void eor_indy(){
	register scnt a=cpu.a^=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void eor_zpx(){
	register scnt a=cpu.a^=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void eor_absy(){
	register scnt a=cpu.a^=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void eor_absx(){
	register scnt a=cpu.a^=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
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
}
void lsr_zpx(){
	register scnt a=azpx(cpu.pc);
	lsr_x(2);
}
void lsr_abs(){
	register scnt a=aabs(cpu.pc);
	lsr_x(3);
}
void lsr_absx(){
	register scnt a=aabsx(cpu.pc);
	lsr_x(3);
}
void lsr_acc(){
	register scnt o=cpu.a;
	setc1(o);
	o>>=1;
	setnz(o);
	cpu.a=o;
	cpu.pc++;
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
}
void ror_zpx(){
	register scnt a=azpx(cpu.pc);
	ror_x(2);
}
void ror_abs(){
	register scnt a=aabs(cpu.pc);
	ror_x(3);
}
void ror_absx(){
	register scnt a=aabsx(cpu.pc);
	ror_x(3);
}
void ror_acc(){
	register scnt o=cpu.a+(carry1()?256:0);
	setc1(o);
	o>>=1;
	setnz(o);
	cpu.a=o;
	cpu.pc+=1;
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
}
void adc_zp(){
	register scnt a=zp(cpu.pc);
	adc_x(2);
}
void adc_imm(){
	register scnt a=imm(cpu.pc);
	adc_x(2);
}
void adc_abs(){
	register scnt a=abs(cpu.pc);
	adc_x(3);
}
void adc_indy(){
	register scnt a=indy(cpu.pc);
	adc_x(2);
}
void adc_zpx(){
	register scnt a=zpx(cpu.pc);
	adc_x(2);
}
void adc_absy(){
	register scnt a=absy(cpu.pc);
	adc_x(3);
}
void adc_absx(){
	register scnt a=absx(cpu.pc);
	adc_x(3);
}

void sta_indx(){
	setbyt(aindx(cpu.pc),cpu.a);
	cpu.pc+=2;
}
void sty_zp(){
	setbyt(azp(cpu.pc),cpu.y);
	cpu.pc+=2;
}
void sta_zp(){
	setbyt(azp(cpu.pc),cpu.a);
	cpu.pc+=2;
}
void stx_zp(){
	setbyt(azp(cpu.pc),cpu.x);
	cpu.pc+=2;
}
void sty_abs(){
	setbyt(aabs(cpu.pc),cpu.y);
	cpu.pc+=3;
}
void sta_abs(){
	setbyt(aabs(cpu.pc),cpu.a);
	cpu.pc+=3;
}
void stx_abs(){
	setbyt(aabs(cpu.pc),cpu.x);
	cpu.pc+=3;
}
void sta_indy(){
	setbyt(aindy(cpu.pc),cpu.a);
	cpu.pc+=2;
}
void sty_zpx(){
	setbyt(azpx(cpu.pc),cpu.y);
	cpu.pc+=2;
}
void sta_zpx(){
	setbyt(azpx(cpu.pc),cpu.a);
	cpu.pc+=2;
}
void stx_zpy(){
	setbyt(azpy(cpu.pc),cpu.x);
	cpu.pc+=2;
}
void sta_absy(){
	setbyt(aabsy(cpu.pc),cpu.a);
	cpu.pc+=3;
}
void sta_absx(){
	setbyt(aabsx(cpu.pc),cpu.a);
	cpu.pc+=3;
}

void tya(){
	register scnt a=cpu.a=cpu.y;
	setnz(a);
	cpu.pc++;
}

void txa(){
	register scnt a=cpu.a=cpu.x;
	setnz(a);
	cpu.pc++;
}

void txs(){
	cpu.sp=cpu.x;
	cpu.pc++;
}

void tay(){
	register scnt a=cpu.y=cpu.a;
	setnz(a);
	cpu.pc++;
}

void tax(){
	register scnt a=cpu.x=cpu.a;
	setnz(a);
	cpu.pc++;
}

void tsx(){
	register scnt a=cpu.x=cpu.sp;
	setnz(a);
	cpu.pc++;
}

void ldy_imm(){
	register scnt a=cpu.y=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void lda_indx(){
	register scnt a=cpu.a=indx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ldx_imm(){
	register scnt a=cpu.x=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ldy_zp(){
	register scnt a=cpu.y=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void lda_zp(){
	register scnt a=cpu.a=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ldx_zp(){
	register scnt a=cpu.x=zp(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void lda_imm(){
	register scnt a=cpu.a=imm(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ldy_abs(){
	register scnt a=cpu.y=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void lda_abs(){
	register scnt a=cpu.a=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void ldx_abs(){
	register scnt a=cpu.x=abs(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void lda_indy(){
	register scnt a=cpu.a=indy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ldy_zpx(){
	register scnt a=cpu.y=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void lda_zpx(){
	register scnt a=cpu.a=zpx(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void ldx_zpy(){
	register scnt a=cpu.x=zpy(cpu.pc);
	setnz(a);
	cpu.pc+=2;
}
void lda_absy(){
	register scnt a=cpu.a=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void ldy_absx(){
	register scnt a=cpu.y=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void lda_absx(){
	register scnt a=cpu.a=absx(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}
void ldx_absy(){
	register scnt a=cpu.x=absy(cpu.pc);
	setnz(a);
	cpu.pc+=3;
}

void iny(){
	register scnt a=cpu.y=(cpu.y+1)&0xff;
	setnz(a);
	cpu.pc++;
}

void dex(){
	register scnt a=cpu.x=(cpu.x-1)&0xff;
	setnz(a);
	cpu.pc++;
}

void inx(){
	register scnt a=cpu.x=(cpu.x+1)&0xff;
	setnz(a);
	cpu.pc++;
}

void dey(){
	register scnt a=cpu.y=(cpu.y-1)&0xff;
	setnz(a);
	cpu.pc++;
}

#define madd(offset,len) \
	register scnt o=((getbyt(a))+(offset))&0xff;\
	setnz(o);\
	setbyt(a,o);\
	cpu.pc+=len

void dec_zp(){
	register scnt a=azp(cpu.pc);
	madd(-1,2);
}
void dec_zpx(){
	register scnt a=azpx(cpu.pc);
	madd(-1,2);
}
void dec_abs(){
	register scnt a=aabs(cpu.pc);
	madd(-1,3);
}
void dec_absx(){
	register scnt a=aabsx(cpu.pc);
	madd(-1,3);
}

void inc_zp(){
	register scnt a=azp(cpu.pc);
	madd(1,2);
}
void inc_zpx(){
	register scnt a=azpx(cpu.pc);
	madd(1,2);
}
void inc_abs(){
	register scnt a=aabs(cpu.pc);
	madd(1,3);
}
void inc_absx(){
	register scnt a=aabsx(cpu.pc);
	madd(1,3);
}

void cpy_imm(){
	register scnt a=cpu.y-imm(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cmp_indx(){
	register scnt a=cpu.a-indx(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cpy_zp(){
	register scnt a=cpu.y-zp(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cmp_zp(){
	register scnt a=cpu.a-zp(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cmp_imm(){
	register scnt a=cpu.a-imm(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cpy_abs(){
	register scnt a=cpu.y-abs(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
}
void cmp_abs(){
	register scnt a=cpu.a-abs(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
}
void cmp_indy(){
	register scnt a=cpu.a-indy(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cmp_zpx(){
	register scnt a=cpu.a-zpx(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cmp_absy(){
	register scnt a=cpu.a-absy(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
}
void cmp_absx(){
	register scnt a=cpu.a-absx(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
}
void cpx_imm(){
	register scnt a=cpu.x-imm(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cpx_zp(){
	register scnt a=cpu.x-zp(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=2;
}
void cpx_abs(){
	register scnt a=cpu.x-abs(cpu.pc);
	setc2(a);
	setnza(a);
	cpu.pc+=3;
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
}
void sbc_zp(){
	register scnt a=zp(cpu.pc);
	sbc_x(2);
}
void sbc_imm(){
	register scnt a=imm(cpu.pc);
	sbc_x(2);
}
void sbc_abs(){
	register scnt a=abs(cpu.pc);
	sbc_x(3);
}
void sbc_indy(){
	register scnt a=indy(cpu.pc);
	sbc_x(2);
}
void sbc_zpx(){
	register scnt a=zpx(cpu.pc);
	sbc_x(2);
}
void sbc_absy(){
	register scnt a=absy(cpu.pc);
	sbc_x(3);
}
void sbc_absx(){
	register scnt a=absx(cpu.pc);
	sbc_x(3);
}

void (*sim[256])(void)=
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
	void (*v)(scnt,CPU*);
	cpu_reset(&cpu);
	struct2cpu(&cpu);
	
	do{
/*if(dismode || hirq) printf("\n\nhirq=%d, irq=%d, hnmi=%d\n",hirq,irq,hnmi);*/
		if(hirq && !(irq)) {
/*printf("\nirq: set pc to IRQ address\n");*/
			aclb();
			cpu2struct(&cpu);
			cpu.pc-=1;
			phpc();
			phbyt(cpu.sr);
			cpu.pc=getadr(0xfffe);
			asei();
		}
                if(hnmi) {
			hnmi=0;
                        cpu2struct(&cpu);
			cpu.pc--;
                        phpc();
                        phbyt(cpu.sr);
                        cpu.pc=getadr(0xfffa);
                        cpu.sr |= IRQ;
			struct2cpu(&cpu);
                }
                if(v=trap6502(cpu.pc)) {
			cpu2struct(&cpu);
			v(cpu.pc,&cpu);
			struct2cpu(&cpu);
		}

 		if(traplines) {
			if(!(--traplines))
				dismode =2;
		}
		if(dismode) {
			cpu2struct(&cpu);
			logass(&cpu);
		}
		c=getbyt(cpu.pc);
		(*sim[c])();
		inc_time(10);
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

