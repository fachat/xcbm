
#include <stdio.h>

#include "types.h"

#include "log.h"
#include "alarm.h"

#include "graphlib.h"

// alarm context
static alarm_t v2_alarm;

// rasterline counter
static int rline;

// graphics buffer (at this time graphlib_pixel_t is a uint8 in RGB(332) encoding
static graphlib_pixel_t *pixbuf;


static void viccy2_rline_cb(alarm_t *alarm, CLOCK current) {

	logout(3, "raster display r=%d at c=%ld", rline, current);

	// update alarm for the next rasterline
	set_alarm_clock_plus(&v2_alarm, 20000);

	for (int i=0; i < 100; i++) pixbuf[i] ++;

	graphlib_poll_events();

	graphlib_update();

}

static void fill_rect(graphlib_pixel_t *fb,         
                      int x, int y, int w, int h,   
                      graphlib_pixel_t colour)      
{                                                   
    for (int row = y; row < y + h; row++) {         
        for (int col = x; col < x + w; col++) {     
            fb[row * GRAPHLIB_WIDTH + col] = colour;
        }                                           
    }                                               
}                                                   


/* called from mem module to tell the CRTC emu about the video memory */
void viccy2_init(alarm_context_t *actx) {

	logout(1, "Initializing the VICCY SDL display");

	// pixel buffer is organized in GRAPHLIB_HEIGHT x GRAPHLIB_WIDTH pixels 
	pixbuf = graphlib_get_framebuffer();

	fill_rect(pixbuf, 0, 0, GRAPHLIB_WIDTH, GRAPHLIB_HEIGHT, 64+8+1);
	graphlib_update();

	// setup alarm so that every rasterline is drawn in an alarm
	// and then the screen is updated at the end
	alarm_init(&v2_alarm, "viccy2", actx, &viccy2_rline_cb, NULL);

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


