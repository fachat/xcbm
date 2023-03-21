
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"
#include "ccurses.h"
#include "video.h"

#define	MAXLINE		200

int color = 0;		/* 0 = black/white, 1= color use for ncurses */

WINDOW 	*scr;
int update;


rgb_t rgb[16]={ { 0,    0,    0,	0 },	/* black */
		{ 1000, 1000, 1000,	7 },	/* white */ 
		{ 1000, 0,    0,	1 },	/* red */
		{ 0,    0,    0,	5 },	/* Tuerkis */
		{ 500,  0,  500,	6 },	/* Violett */
		{ 0,    1000, 0,	2 },	/* green */
		{ 0,    0,    1000,	4 },	/* blue */
		{ 0,    0,    0,	3 },	/* yellow */
		{ 0,    0,    0,	0 },	/* orange */
		{ 0,    0,    0,	0 },	/* brown */
		{ 1000,  200,  200,	0 },	/* bright red */
		{ 250,   250,  250,	0 },	/* grey 1 */
		{ 500,   500,  500,	0 },	/* grey 2 */
		{ 200,  1000,  200,	0 },	/* bright green */
		{ 200,   200, 1000,	6 },	/* bright blue */
		{ 750,   750,  750,	0 } };	/* grey 3 */
 

int cur_getch() {
	struct timespec sleep;

        int c = ERR;
        while (c == ERR) {
                c = getch();

                // wait a bit to release CPU
                sleep.tv_sec = 0;
                sleep.tv_nsec = 20000000;
                nanosleep(&sleep, NULL);
        }
	return c;
}

void cur_setup(void) {
	scr =initscr();

	cbreak();	/* may be changed to raw() someday */
	noecho();
	leaveok(scr, TRUE);
	scrollok(scr, FALSE);
	nonl();
	keypad(scr,TRUE);
/*
	notimeout(scr,TRUE);
*/
	meta(scr,TRUE);
	nodelay(scr,TRUE);

	if(color>=0 && has_colors()) {
	  int i;
	  logout(1,"Terminal reports color capability...");
	  color=1;
	  if(can_change_color()) {
	    logout(1,"Terminal can also change colors...");
	    color=2;
	  }
	  start_color();
	  logout(2,"Terminal has %d possible colors and %d color pairs",
			COLORS,COLOR_PAIRS);
	  if(color==2 && COLORS>=16) {
	    for(i=0;i<16;i++) {
	      init_color(i,rgb[i].r,rgb[i].g,rgb[i].b); 
	    }
	    color=3;
	  }
	} else 
	if (color < 0 && has_colors()) {
	} else {
	  color=0;
	}
}

void cur_exit(void) {
	logout(1,"Exiting curses window mode");
	fflush(stdout);
	nocbreak();
	endwin();
}

ssize_t cur_getline(char **line, size_t *llen) {


        if (*llen == 0) {
                *line = malloc(MAXLINE);
                *llen = MAXLINE;
        }

	int r = getline(line, llen, stdin);


	return r;
}


int cur_init(void) {

	cur_setup();

	atexit(cur_exit);

	return(0);
}


void video_set_status_line(const char *line) {

        int i = 0;
        int l = strlen(line);

	update = 0;

        for (i = 0; i < l; i++) {
                mvaddch(25,i,line[i]);
        }
        for (;i < 80; i++) {
                mvaddch(25, i, 32);     // fill with space
        }

	update = 1;
	refresh();
}

