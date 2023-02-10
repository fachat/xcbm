
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"
#include "mon.h"
#include "labels.h"


static struct sigaction monaction;
static struct sigaction oldaction;

int monflag = 0;

//static void mon_sigaction(int sig, siginfo_t siginfo, void *p);
static void mon_sighandler(int sig) {

	monflag = 1;
}


void mon_init() {

	monflag = 0;

	monaction.sa_handler = mon_sighandler;
	monaction.sa_flags = 0;
	sigemptyset(&monaction.sa_mask);

	int er = sigaction(SIGINT, &monaction, &oldaction);

	if (er) {

		logout(0, "Could not establish signal handler: %s", 
			strerror(er));

	}
}


void mon_line(CPU *cpu) {

	logout(0, "Entering monitor!");

	exit(1);
}



