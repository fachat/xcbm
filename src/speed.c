
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>


#include "log.h"

#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "cpu.h"
#include "config.h"

static unsigned int target_speed_percent = 100;
static double speed_ratio = 0.0;

static unsigned int msperframe;

static struct timespec last;

/* update status only every N frames */
static const int UPDATE_COUNT = 5;
static int update_counter;

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

	// get current time
        clock_gettime(CLOCK_MONOTONIC, &spec);

	int cyclesperframe = (int) alarm->data;

        s  = spec.tv_sec;
        ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
        if (ms > 999) {
                s++;
                ms = 0;
        }

	set_alarm_clock(alarm, alarm->clk + cyclesperframe);

        logout(0, "speed ctrl: clock=%lu, time=%ld.%03ds, last=%ld.%03ds", current, s, ms, last.tv_sec, last.tv_nsec /1000000);

	long expectedns = target_speed_percent == 0 ? 1 : (msperframe * 1000000ul) * 100 / target_speed_percent;

	logout(0, "spec nsec=%ld, last nsec=%ld", spec.tv_nsec, last.tv_nsec);

	diff.tv_sec  = spec.tv_sec  - last.tv_sec;
	diff.tv_nsec = spec.tv_nsec - last.tv_nsec;

	//logout(0, "diff sec=%ld, nsec=%ld", diff.tv_sec, diff.tv_nsec);

	if (diff.tv_sec < 0 || (diff.tv_sec == 0 && diff.tv_nsec < 0)) {
		logout(1, "went back in time... ");
		diff.tv_sec = 0;
		diff.tv_nsec = 1;
	}

	while (diff.tv_nsec < 0) {
		diff.tv_sec -= 1;
		diff.tv_nsec += 1000000000ul;
	}

	//logout(0, "diff=%ld.%03ds", diff.tv_sec, diff.tv_nsec / 1000000);

	double speedratio = 100.0;

	if (diff.tv_sec > 0 || diff.tv_nsec > expectedns) {
		logout(0, "too slow? diff=%ld.%03ds", diff.tv_sec, diff.tv_nsec / 1000000);
		// we are too slow
		last = spec;

		double upper = (msperframe * 1000000.);
		double lower = (diff.tv_sec * 1000000000. + diff.tv_nsec);

		speedratio = 100.0 * upper / lower;

		logout(0, "expected=%ld, upper=%lf / lower=%lf -> ratio=%lf%%", expectedns,upper,lower,speedratio); 
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

	/* smooth out fluctuations */
	speed_ratio = (2.0 * speed_ratio + speedratio) / 3.0;

	if (--update_counter) {
		return;
	}

	update_counter = UPDATE_COUNT;

	config_set_speed(speed_ratio, target_speed_percent);
}


void speed_init(alarm_context_t *actx, int pcyclespersec, int pmsperframe) {

	alarm_t *speed = malloc(sizeof(alarm_t));

	update_counter = UPDATE_COUNT;

	msperframe = pmsperframe;
	int cyclesperframe = (pcyclespersec / 1000) * pmsperframe;

	clock_gettime(CLOCK_MONOTONIC, &last);

	target_speed_percent = 100;

	speed->name = "CPU speed control";
	speed->data = (void*) cyclesperframe;
	speed->callback = speed_alarm_cb;
	speed->clk = CLOCK_MAX;

	alarm_register(actx, speed);

	set_alarm_clock_diff(speed, cyclesperframe);
}

