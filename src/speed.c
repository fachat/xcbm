
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>


#include "log.h"

#include "types.h"
#include "alarm.h"
#include "emu6502.h"

static unsigned int target_speed_percent = 100;

static struct timespec last;

// set target speed in percent of the original machine. 0 = warp
void speed_set_percent(unsigned int per) {

	target_speed_percent = per;
}

void speed_alarm_cb(alarm_t *alarm, CLOCK current) {

        long ms; // Milliseconds
        time_t s;  // Seconds
        struct timespec spec;
        struct timespec diff;
        struct timespec rem;

	// get current time
        clock_gettime(CLOCK_MONOTONIC, &spec);

	CPU *cpu = (CPU*) alarm->data;

        s  = spec.tv_sec;
        ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
        if (ms > 999) {
                s++;
                ms = 0;
        }

	set_alarm_clock(alarm, alarm->clk + cpu->cyclesperframe);

        logout(0, "speed ctrl: clock=%lu, time=%ld.%03ds, last=%ld.%03ds", current, s, ms, last.tv_sec, last.tv_nsec /1000000);

	long expectedns = (cpu->msperframe * 1000000ul) * 100 / target_speed_percent;


	diff.tv_sec  = spec.tv_sec  - last.tv_sec;
	diff.tv_nsec = spec.tv_nsec - last.tv_nsec;
	if (diff.tv_nsec < 0) {
		--diff.tv_sec;
		diff.tv_nsec += 1000000000L;
	}
	logout(0, "diff=%ld.%03ds", diff.tv_sec, diff.tv_nsec / 1000000);


	if (diff.tv_sec > 0 || diff.tv_nsec > expectedns) {
		logout(0, "too slow? diff=%ld.%03ds", diff.tv_sec, diff.tv_nsec / 1000000);
		// we are too slow
	} else {
		// we are too fast - wait rest of interval
		long wait = expectedns - diff.tv_nsec;
		diff.tv_nsec = wait;
		diff.tv_sec = 0;

		int r = nanosleep(&diff, &rem);
		if (r) {
			logout(0, "interrupted: errno=%d (%s)", errno, strerror(errno));
		}
		last = spec;
		last.tv_nsec += expectedns;
		if (last.tv_nsec > 1000000000l) {
			last.tv_sec++;
			last.tv_nsec -= 1000000000l;
		}
	}


}


void speed_init(CPU *cpu, int cyclespersec, int msperframe) {

	cpu->msperframe = msperframe;
	cpu->cyclesperframe = (cyclespersec / 1000) * msperframe;

	clock_gettime(CLOCK_MONOTONIC, &last);

	target_speed_percent = 100;

	cpu->speed.name = "CPU speed control";
	cpu->speed.callback = speed_alarm_cb;
	cpu->speed.data = cpu;
	cpu->speed.clk = CLOCK_MAX;

	alarm_register(&cpu->actx, &cpu->speed);

	set_alarm_clock_diff(&cpu->speed, cpu->cyclesperframe);
}

