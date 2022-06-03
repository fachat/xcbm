
#include <string.h>

#include "types.h"
#include "emu6502.h"

#include "timer.h"
#include "video.h"
#include "keys.h"
#include "io.h"

#include "pia.h"

PIA pia1;
PIA pia2;

#if 0

void	io_wr(scnt,scnt);
scnt	io_rd(scnt);
CIA	cia1, cia2;

void cia1_tia(int); 
void cia1_tib(int);
void cia2_tia(int);
void cia2_tib(int);

void cia1_wr(scnt,scnt);
void cia2_wr(scnt,scnt);
scnt cia1_rd(scnt);
scnt cia2_rd(scnt);

#endif

// PIA1
//
// port A:
// 	0-3: keyboard row select out
// 	4: cassette #1 switch in
// 	5: cassette #2 switch in
// 	6: EOI in
// 	7: DIAG in
//
// CA1:	Cassette #1 read in
// CA2: EOI out
//
// port B:
// 	0-7: keyboard col input
//
// CB1: vertical drive in (interrupt generator)
// CB2: cassette #1 motor out
//

static uchar pia1_get_porta(uchar origdata) {
}

static void pia1_set_porta(uchar data, uchar dir) {
}

static void pia1_set_ca2(uchar flag) {
}

static uchar pia1_get_portb(uchar origdata) {
}


int io_init(void) {

	pia_init(&pia1);
	pia_init(&pia2);

	pia1.get_port_a_in = pia1_get_porta;
	pia1.set_port_a_out = pia1_set_porta;
	pia1.set_ca2_out = pia1_set_ca2;

	pia1.get_port_b_in = pia1_get_portb;

#if 0
	memset(&cia1,0,sizeof(CIA));
	memset(&cia2,0,sizeof(CIA));
	cia1.timera=time_register(cia1_tia,"cia1 timera");
	cia1.timerb=time_register(cia1_tib,"cia1 timerb");
	cia2.timera=time_register(cia2_tia,"cia2 timera");
	cia2.timerb=time_register(cia2_tib,"cia2 timerb");

	key_init(0);
#endif
	return(0);
}

#if 0
void io_wr(scnt adr, scnt val) {
	register scnt a=(adr&0x0c00);
	switch(a) {
	case 0:
		video_wr(adr&0x3f,val);
		break;
	case 0x400:
		sid_wr(adr&0x1f,val);
		break;
	case 0x800:
		colram_wr(adr&0x3ff,val);
		break;
	case 0xc00:	
		a=adr&0x300;
		switch(a) {
		case 0:
			cia1_wr(adr&0x0f,val);
			break;
		case 0x100:
			cia2_wr(adr&0x0f,val);
			break;
		}
		break;
	}
}

scnt io_rd(scnt adr) {
        register scnt a=(adr&0x0c00);
        switch(a) {
        case 0:
                return(video_rd(adr&0x3f));
                break;
        case 0x400:
                return(sid_rd(adr&0x1f));
                break;
        case 0x800:
                return(colram_rd(adr&0x3ff));
                break;
        case 0xc00:
                a=adr&0x300;
                switch(a) {
                case 0:
                        return(cia1_rd(adr&0x0f));
                        break;
                case 0x100:
                        return(cia2_rd(adr&0x0f));
                        break;
                default:
                        return((adr&0xff00)>>8);
                }
 	}
	return(0);
}

/****************************************************************************/


#define	cia1_setirq()	\
 if(!(cia1.icr&0x80)) {if(cia1.icr&cia1.imr&0x7f){cia1.icr|=0x80;hirq++;}}
#define	cia2_setirq()	\
 if(!(cia2.icr&0x80)) {if(cia2.icr&cia2.imr&0x7f){cia2.icr|=0x80;hnmi++;}}

void cia1_tia(int endval) {
	cia1.talo = cia1.talo_w;
	cia1.tahi = cia1.tahi_w;
	if(cia1.cra&8) { /* single shot */
		cia1.cra &= 0xfe;	/* timer a stop */
		time_reset(cia1.timera);
	} else {	/* continous mode */
		time_setval(cia1.timera, cia1.talo_w+256*cia1.tahi_w);
	}
	cia1.icr |= 1;
	cia1_setirq();
/*printf("\ncia1_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia1.icr, cia1.imr,hirq);*/
}

void cia1_tib(int endval) {
        cia1.tblo = cia1.tblo_w;
        cia1.tbhi = cia1.tbhi_w;
        if(cia1.crb&8) { /* single shot */
                cia1.crb &= 0xfe;       /* timer a stop */
                time_reset(cia1.timerb);
        } else {        /* continous mode */
                time_setval(cia1.timerb, cia1.tblo_w+256*cia1.tbhi_w+endval);
        }
        cia1.icr |= 2;
        cia1_setirq();
/*printf("\ncia1_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia1.icr, cia1.imr,hirq);*/
}

void cia2_tia(int endval) {
        cia2.talo = cia2.talo_w;
        cia2.tahi = cia2.tahi_w;
        if(cia2.cra&8) { /* single shot */
                cia2.cra &= 0xfe;       /* timer a stop */
                time_reset(cia2.timera);
        } else {        /* continous mode */
                time_setval(cia2.timera, cia2.talo_w+256*cia2.tahi_w+endval);
        }
        cia2.icr |= 1;
        cia2_setirq();
/*printf("\ncia2_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia2.icr, cia2.imr,hnmi);*/
}

void cia2_tib(int endval) {
        cia2.tblo = cia2.tblo_w;
        cia2.tbhi = cia2.tbhi_w;
        if(cia2.crb&8) { /* single shot */
                cia2.crb &= 0xfe;       /* timer a stop */
                time_reset(cia2.timerb);
        } else {        /* continous mode */
                time_setval(cia2.timerb, cia2.tblo_w+256*cia2.tbhi_w+endval);
        }
        cia2.icr |= 2;
        cia2_setirq();
/*printf("\ncia2_setirq, icr=%02x, imr=%02x, hirq=%d\n",cia2.icr, cia2.imr,hnmi);*/
}

void cia1_wr(scnt reg, scnt val) {
	switch(reg) {
	case 0:
		cia1.pra = val;
		break;
	case 1:
		cia1.prb = val;
		break;
	case 2:
		cia1.ddra = val;
		break;
	case 3:
		cia1.ddrb = val;
		break;
	case 4:
		cia1.talo_w = val;
		break;
	case 5:
		cia1.tahi_w = val;
		break;
	case 6:
		cia1.tblo_w = val;
		break;
	case 7:
		cia1.tbhi_w = val;
		break;
	case 13:
		if(val&0x80) { 	/* set mask bits */
			cia1.imr |= (val&0x7f);
		} else {
			cia1.imr &= (val^0x7f);
		}
		break;
	case 14:
		if(val&1) { 
			time_seton(cia1.timera);
			if(!(cia1.cra&1))
				time_setval(cia1.timera,
					cia1.talo_w+256*cia1.tahi_w);
		}
		cia1.cra = val;
		break;
	case 15:
                if(val&1) {
                        time_seton(cia1.timerb);
                        if(!(cia1.crb&1))
                                time_setval(cia1.timerb,
					cia1.tblo_w+256*cia1.tbhi_w);
                }
		cia1.crb = val;
		break;
	}
}

void cia2_wr(scnt reg, scnt val) {
        switch(reg) {
        case 0:
		if((val^cia2.pra)&0x3) 
			setvideopage((val&0x3)^3);
                cia2.pra = val;
                break;
        case 1:
                cia2.prb = val;
                break;
        case 2:
                cia2.ddra = val;
                break;
        case 3:
                cia2.ddrb = val;
                break;
        case 4:
                cia2.talo_w = val;
                break;
        case 5:
                cia2.tahi_w = val;
                break;
        case 6:
                cia2.tblo_w = val;
                break;
        case 7:
                cia2.tbhi_w = val;
                break;
        case 13:
                if(val&0x80) {  /* set mask bits */
                        cia2.imr |= (val&0x7f);
                } else {
                        cia2.imr &= (val^0x7f);
                }
                break;
        case 14:
                if(val&1) {
                        time_seton(cia2.timera);
                        if(!(cia2.cra&1))
                                time_setval(cia2.timera,
					cia2.talo_w+256*cia2.tahi_w);
                }
                cia2.cra = val;
                break;
        case 15:
                if(val&1) {
                        time_seton(cia2.timerb);
                        if(!(cia2.crb&1))
                                time_setval(cia2.timerb, 
					cia2.tblo_w+256*cia2.tbhi_w);
                }
                cia2.crb = val;
                break;
        }
} 

 
scnt cia1_rd(scnt reg) {
	scnt i; 
	switch(reg) {
	case 0:
		return(cia1.pra);
	case 1:
		return(prb[cia1.pra]);
	case 2:
		return(cia1.ddra);
	case 3:
		return(cia1.ddrb);
	case 4:
		return(time_get(cia1.timera)&0xff);
	case 5:
		return((time_get(cia1.timera)&0xff00)>>8);
	case 6:
		return(time_get(cia1.timerb)&0xff);
	case 7:
		return((time_get(cia1.timerb)&0xff00)>>8);
	case 13:
		if(cia1.icr&0x80) hirq--;
		i=cia1.icr;
		cia1.icr=0;
		return(i);
	case 14:
		return(cia1.cra);
	case 15:
		return(cia1.crb);
	}
	return(0);
}	

scnt cia2_rd(scnt reg) {
        scnt i;
        switch(reg) {
        case 0:
                return(cia2.pra);
        case 1:
                return(cia2.prb);
        case 2:
                return(cia2.ddra);
        case 3:
                return(cia2.ddrb);
        case 4:
                return(time_get(cia2.timera)&0xff);
        case 5:
                return((time_get(cia2.timera)&0xff00)>>8);
        case 6:
                return(time_get(cia2.timerb)&0xff);
        case 7:
                return((time_get(cia2.timerb)&0xff00)>>8);
        case 13:
                if(cia2.icr&0x80) hirq--;
                i=cia2.icr;
                cia2.icr=0;
                return(i);
        case 14:
                return(cia2.cra);
        case 15:
                return(cia2.crb);
        }
        return(0);
}
#endif

