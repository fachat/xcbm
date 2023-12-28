
#include <signal.h>

#include "log.h"
#include "stop.h"

static struct sigaction stopaction;
static struct sigaction oldaction;

static int stopflag = 0;

//static void stop_sigaction(int sig, siginfo_t siginfo, void *p);
static void stop_sighandler(int sig) {

        stopflag = 1;
}


int stop_get_flag() {
	return stopflag;
}

int stop_ack_flag() {
	int rv = stopflag;
	stopflag = 0;
	return rv;
}

void stop_init() {

        stopflag = 0;

        stopaction.sa_handler = stop_sighandler;
        stopaction.sa_flags = 0;
        sigemptyset(&stopaction.sa_mask);

        int er = sigaction(SIGINT, &stopaction, &oldaction);

        if (er) {

                logout(0, "Could not establish signal handler: %s",
                        strerror(er));

        }
}

