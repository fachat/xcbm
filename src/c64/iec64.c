
#include <stdio.h>

#include "types.h"
#include "emu6502.h"
#include "log.h"
#include "iec.h"
#include "mem.h"

#define atnislo()	(getbyt(0xdd00)&0x08)
#define	iseof()		(getbyt(0x00a3)&0x80)
#define	seteof()	setbyt(0x0090,getbyt(0x0090)|0x40)


extern device	*dev;

extern device	*devs[16];

void bytein(scnt adr, CPU *cpu) {
	if(!dev) {		
		cpu->pc=0xee42;
	} else {
		if(dev->mode!=MODE_TALK) {
			cpu->pc=0xee42;	
		} else {
			cpu->a=dev->get((void*)dev);
			/*cpu->sr &= ~(IRQ|CARRY);*/
			if(dev->timeout) {
logout(0,"set timeout pc->0xee42");
				cpu->pc=0xee42;
			} else {
				setbyt(0x00a4,cpu->a);
				cpu->pc=0xee82;
			}
		}
	}

}

void byteout(scnt adr, CPU *cpu) {
	scnt by=getbyt(0x95);
	scnt a= by & 0x0f;
	scnt b= by & 0xf0;
/*printf("byteout(adr=%04x, by=%02x, a=%02x, b=%02x, dev=%p)\n",adr,by,a,b,dev);*/

	cpu->sr &= ~(IRQ|CARRY);
	cpu->pc =0xedac;

	if(atnislo()) {
		if(b==0x20) {
			if(dev=devs[a]) {
				dev->mode = MODE_LISTEN;
				dev->out(by,dev);
			}
		} else 
		if(b==0x40) {
			if(dev=devs[a]) {
				dev->mode = MODE_TALK;	
				dev->out(by,dev);
			}
		} else
		if(dev) {
			if(by==0x3f) {
				dev->out(by, dev);
				dev->mode=MODE_FREE;
				dev=NULL;
			} else
			if(by==0x5f) {
				dev->out(by, dev);
				dev->mode=MODE_FREE;
				dev=NULL;
			} else
				dev->out(by, dev);
	    	} else {
			cpu->pc=0xedad;	/* device not present */
	        }
	} else {
		if(dev) {
			dev->out(by,dev);
		} else {
			cpu->pc=0xedad;
		}
	}
}

void xrts(scnt adr, CPU *cpu) {
	cpu->a=cpu->x;
	cpu->pc=0xeeba;
}

int iec_init(void) { /* is called _after_ iec_setdrive! */
	dev=NULL;
	settrap(MP_KERNEL0,0xed40,byteout,NULL /*"byteout"*/ );
	settrap(MP_KERNEL0,0xee13,bytein,NULL /*"bytein"*/ );
	settrap(MP_KERNEL0,0xeeb3,xrts,NULL /*"xrts"*/);	/* delay loop */
	return(0);
}


