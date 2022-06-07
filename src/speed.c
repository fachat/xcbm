
#include <time.h>
#include <math.h>


#include "log.h"

#include "types.h"
#include "alarm.h"
#include "emu6502.h"



void speed_alarm_cb(alarm_t *alarm, CLOCK current) {

        long ms; // Milliseconds
        time_t s;  // Seconds
        struct timespec spec;

        clock_gettime(CLOCK_MONOTONIC, &spec);

        s  = spec.tv_sec;
        ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
        if (ms > 999) {
                s++;
                ms = 0;
        }

	set_alarm_clock(alarm, alarm->clk + ((CPU*)alarm->data)->cyclesperframe);

        logout(0, "speed ctrl: clock=%lu, ms=%ld.%03d", current, s, ms);
}


void speed_init(CPU *cpu, int cyclespersec, int msperframe) {

	cpu->msperframe = msperframe;
	cpu->cyclesperframe = (cyclespersec / 1000) * msperframe;

	cpu->speed.name = "CPU speed control";
	cpu->speed.callback = speed_alarm_cb;
	cpu->speed.data = cpu;
	cpu->speed.clk = CLOCK_MAX;

	alarm_register(&cpu->actx, &cpu->speed);

	set_alarm_clock_diff(&cpu->speed, cpu->cyclesperframe);
}

