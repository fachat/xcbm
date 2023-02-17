
#include <curses.h>
#include <stdio.h>
#include <time.h>

#include "types.h"
#include "config.h"
#include "log.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"
#include "video.h"


static char cf_esc_char = 'p';
static float cf_speed_ratio = 0.;
static float cf_target_speed = 0.;
static int cf_shiftmap = 0;
int cf_trace_enabled = 0;

#define	MAXLINE 80

static char line[MAXLINE];

static void config_update() {

        snprintf(line, MAXLINE, "Speed: % 3.0lf%%, limit=% 3.0lf%%  Esc: C-%c  Shift:%c%c %s", 
		cf_speed_ratio, cf_target_speed,
		cf_esc_char,
		cf_shiftmap & SLINE_SHIFT_L ? 'L' : '-',
		cf_shiftmap & SLINE_SHIFT_R ? 'R' : '-',
		cf_trace_enabled ? "TRACE" : " "
	);
        line[MAXLINE-1]=0;
        video_set_status_line(line);
}

void config_set_esc_char(char c) {

	cf_esc_char = c;
}

char config_get_esc_char() {
	return cf_esc_char;
}

void config_set_speed(float hostperc, float limitperc) {

	cf_speed_ratio = hostperc;
	cf_target_speed = limitperc;

	config_update();
}

void config_set_shift(int shiftmap) {

	cf_shiftmap = shiftmap;
}

void config_toggle_trace() {
	cf_trace_enabled = !cf_trace_enabled;
}

/* get an escaped character from the keyboard */
int esc_getch() {

	struct timespec sleep;

	int c = getch();

	//if (c != ERR) logout(1, "received char %02x ('%c'), esc=%02x, '%c'", c, c, config_get_esc_char() & 0x1f, sline_get_esc_char() & 0x1f);

	if (c != (config_get_esc_char() & 0x1f)) {
		return c;
	}

	logout(1, "escape: Entering wait loop");

	// wait test
	c = ERR;
	while (c == ERR) {
		c = getch();
		
		// wait a bit to release CPU
		sleep.tv_sec = 0;
		sleep.tv_nsec = 20000000;
		nanosleep(&sleep, NULL);
	}

	// if Ctrl-P again, exit pause/config mode
	if (c == (config_get_esc_char() & 0x1f)) {
		c = ERR;
	}

	switch (c) {
	case 't':
		// toggle trace
		config_toggle_trace();
		c = ERR;
		break;
	default:
		break;
	}

	return c;
}

