
#include <stdio.h>

#include "types.h"

#include "log.h"
#include "alarm.h"

// alarm context
static alarm_t v2_alarm;

// rasterline counter
static int rline;


static void viccy2_rline_cb(alarm_t *alarm, CLOCK current) {

	// update alarm for the next rasterline
	set_alarm_clock_plus(&v2_alarm, 64);
}



/* called from mem module to tell the CRTC emu about the video memory */
void viccy2_init(alarm_context_t *actx) {

	// setup alarm so that every rasterline is drawn in an alarm
	// and then the screen is updated at the end
	v2_alarm.name = "viccy2";
	v2_alarm.callback = &viccy2_rline_cb;
	v2_alarm.clk = CLOCK_MAX;

	alarm_register(actx, &v2_alarm);

	// give the system some time to init before we start display
	set_alarm_clock_diff(&v2_alarm, 20000);

	// init rasterline counter
	rline = 0;
}


/* CRTC emulation */
void viccy2_wr(scnt addr, scnt val) {

}

scnt viccy2_rd(scnt addr) {

	return 0xff;
}


