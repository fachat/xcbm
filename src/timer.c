
#include "timer.h"
#include "log.h"

int tval[MAXTIMER];
int tset[MAXTIMER];
const char *tname[MAXTIMER];
void (*tfunc[MAXTIMER])();
int tnumber;
int tindex;

void time_init(void) {
	tnumber=0;
}

/* registers timer to timing module, gives function to be called on underflow
   returns timer-number */
int time_register(void (*func)(int), const char *name) {
	if(tnumber<MAXTIMER) {
		tset[tnumber]=0;
		tfunc[tnumber]=func;
		tname[tnumber]=name;
		logout(1,"time_register returns %d for timer %s",
			tnumber,tname[tnumber]);
		return(tnumber++);	
	} else {
		return(-1);
	}
}

/* set timer[timerindex] to value timeval */
void time_set(int timerindex, int timeval) {
	tval[timerindex]=timeval;
	tset[timerindex]=1;
}


