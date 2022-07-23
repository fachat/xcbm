
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>


#include "log.h"

#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"
#include "video.h"

#define	MAXLINE	200

static unsigned int target_speed_percent = 100;
static double speed_ratio = 0.0;

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
	struct timespec waitts;
	char line[MAXLINE];

	// get current time
        clock_gettime(CLOCK_MONOTONIC, &spec);

	BUS *bus = (BUS*) alarm->data;

        s  = spec.tv_sec;
        ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
        if (ms > 999) {
                s++;
                ms = 0;
        }

	set_alarm_clock(alarm, alarm->clk + bus->cyclesperframe);

        logout(0, "speed ctrl: clock=%lu, time=%ld.%03ds, last=%ld.%03ds", current, s, ms, last.tv_sec, last.tv_nsec /1000000);

	long expectedns = target_speed_percent == 0 ? 1 : (bus->msperframe * 1000000ul) * 100 / target_speed_percent;

	logout(0, "spec nsec=%ld, last nsec=%ld", spec.tv_nsec, last.tv_nsec);

	diff.tv_sec  = spec.tv_sec  - last.tv_sec;
	diff.tv_nsec = spec.tv_nsec - last.tv_nsec;
	if (diff.tv_nsec < 0) {
		diff.tv_sec = 0;
		diff.tv_nsec = 1;
	}
	//logout(0, "diff=%ld.%03ds", diff.tv_sec, diff.tv_nsec / 1000000);

	double speedratio = 100.0;
	

	if (diff.tv_sec > 0 || diff.tv_nsec > expectedns) {
		logout(0, "too slow? diff=%ld.%03ds", diff.tv_sec, diff.tv_nsec / 1000000);
		// we are too slow
		last = spec;

		speedratio = (expectedns == 1 ? (bus->msperframe * 1000000) : expectedns) 
				/ (diff.tv_sec * 1000000000 + diff.tv_nsec) * 100.0;
	} else {
		// we are too fast - wait rest of interval
		long wait = expectedns - diff.tv_nsec;
		waitts.tv_nsec = wait;
		waitts.tv_sec = 0;

		int r = nanosleep(&waitts, &rem);
		if (r) {
			logout(0, "interrupted: errno=%d (%s)", errno, strerror(errno));
		}
		last = spec;
		last.tv_nsec += expectedns;
		if (last.tv_nsec > 1000000000l) {
			last.tv_sec++;
			last.tv_nsec -= 1000000000l;
		}

		speedratio = target_speed_percent;
	}

	speed_ratio = (2.0 * speed_ratio + speedratio) / 3.0;
	snprintf(line, MAXLINE, "diff=%9ld, Host: % 5.0lf%%, limited to=%d%%", diff.tv_nsec, speed_ratio, target_speed_percent);
	line[MAXLINE-1]=0;
	video_set_status_line(line);

}


void speed_init(CPU *cpu, int cyclespersec, int msperframe) {

	cpu->bus->msperframe = msperframe;
	cpu->bus->cyclesperframe = (cyclespersec / 1000) * msperframe;

	clock_gettime(CLOCK_MONOTONIC, &last);

	target_speed_percent = 100;

	cpu->speed.name = "CPU speed control";
	cpu->speed.callback = speed_alarm_cb;
	cpu->speed.data = cpu->bus;
	cpu->speed.clk = CLOCK_MAX;

	alarm_register(&cpu->bus->actx, &cpu->speed);

	set_alarm_clock_diff(&cpu->speed, cpu->bus->cyclesperframe);
}

