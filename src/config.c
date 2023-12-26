
#include <curses.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "config.h"
#include "log.h"
#include "alarm.h"
#include "bus.h"
#include "cpu.h"
#include "video.h"
#include "ccurses.h"
#include "mem.h"
#include "mon.h"


static char cf_esc_char = 'p';
static float cf_speed_ratio = 0.;
static float cf_target_speed = 0.;
static int cf_shiftmap = 0;
static int cf_cfg_mode = 0;
static int cf_trace_enabled = 0;

#define	MAXLINE 80

static char line[MAXLINE];

/* ------------------------------------------------------------------- */

static config_t **config_list = NULL;
static int config_list_size = 0;

void config_register(config_t pars[]) {

	config_list = reallocarray(config_list, sizeof(config_t*), ++config_list_size);

	config_list[config_list_size - 1] = pars;
}

int config_parse(int argc, char *argv[]) {

	int argp = 1;
	int p, q, l;
	config_t *cp;
	char *par;
	config_t *found;
	char *par_found;

	while (argp < argc) {

		logout(0, "par: %s", argv[argp]);
		par = argv[argp];
		found = NULL;
		par_found = NULL;

		if (par[0] != '-') {
			return argp;
		}
	
		for (p = 0; p < config_list_size; p++) {
			cp = config_list[p];
			
			for (q = 0; cp[q].lname != NULL; q++) {
 
				if (par[1] == cp[q].sname) {
					found = &cp[q];
					if (found->param) {
						if (par[2] != 0) {
							par_found = par+2;
						} else {
							if (argp+1 < argc) {
								par_found = argv[++argp];
							}
						}
					}
				} else
				if (par[1] == '-') {
					l = strlen(cp[q].lname);
					if (strncmp(par+2, cp[q].lname, l) == 0) {
						if (cp[q].param) {
							if (par[2+l] == 0) {
								printf("Missing parameter '%s' for option '%s'\n",
									cp[q].param, cp[q].lname);
								return -1;
							}
							if (par[2+l] == '=') {
								found = &cp[q];
								par_found = par+3+l;
							}
						} else {
							if (par[2+l] == 0) {
								found = &cp[q];
							}
						}
					}
				}
				if (found) break;
			}
			if (found) break;
		}

		if (!found) {
			printf("unknown option '%s'!\n", argv[argp]);
			return -1;
		}

		if (found->param && !par_found) {
			printf("Missing parameter for option '%s'\n", found->lname);
			return -1;
		}

		if (found->set_param(par_found)) {
			printf("error evaluating option '%s'\n", found->lname);
			return -1;
		}

		argp++;
	}
	return argp;
}


void config_print() {

	int p = 0;
	int q;
	config_t *cp;

	while (p < config_list_size) {
		cp = config_list[p];
		p++;

		for (q = 0; cp[q].lname != NULL; q++) {

			if (cp[q].param == NULL) {
				printf("--%s\n",
					cp[q].lname);
			} else {
				printf("--%s=<%s>\n",
					cp[q].lname, 
					cp[q].param);
			}
			if (cp[q].sname != 0) {
				if (cp[q].param == NULL) {
					printf("-%c\n",
						cp[q].sname);
				} else {
					printf("-%c <%s>\n",
						cp[q].sname,
						cp[q].param);
				}
			}
			printf("  %s\n", cp[q].desc); 
		} 
	}
}

/* ------------------------------------------------------------------- */

static void config_update() {

        snprintf(line, MAXLINE, "Speed: % 3.0lf%%, limit=% 3.0lf%%  Esc: C-%c  Shift:%c%c %s %s", 
		cf_speed_ratio, cf_target_speed,
		cf_esc_char,
		cf_shiftmap & SLINE_SHIFT_L ? 'L' : '-',
		cf_shiftmap & SLINE_SHIFT_R ? 'R' : '-',
		cf_trace_enabled ? "TRACE" : " ",
		cf_cfg_mode ? "h=help" : " "
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
	cpu_set_trace(cf_trace_enabled);
}

/*************************************************************************/

typedef struct {
	const char	*opts;		// one or more characters that trigger the cmd
	int		(*cmd)(void);	// function to execute
	const char	*desc;		// description of the command (for help)
} cfgmode_t;

int cmd_trace() {
	config_toggle_trace();
	return ERR;
}

int cmd_mon() {
	mon_line(NULL);
	return ERR;
}

int cmd_exit() {
	exit(0);
	return ERR;
}

int cmd_help();

/* table of config options; table ends with NULL option */
static cfgmode_t cmds[] = {
	{ "m",		cmd_mon, 	"Enter the monitor" },
	{ "t",		cmd_trace, 	"Toggle trace mode (output in log file)" },
	{ "x",		cmd_exit, 	"Exit the emulator" },
	{ "?h",		cmd_help,	"Show this help" },
	{ NULL }
};

int cmd_help() {
	cfgmode_t *p = cmds;

	logout(0, "showing help for C-%c", config_get_esc_char());
	cur_exit();

	while (p->opts) {
		// print name
		const char *cp = p->opts;
		while (*cp) {
			if (cp != p->opts) {
				printf(", ");
			}
			printf("'%c'", *cp);
			cp++;
		}
		printf("\n");
		printf("\t%s\n", p->desc);

		p++;
	}

	printf("Press command or other key to continue\n");

	// read any character immediately, but without going back to curses
	// window mode, which getch() would do - as that would overwrite
	// our help text we just displayed
	raw();
	int c = fgetc(stdin);

	cur_setup();

	return c;
}


/*************************************************************************/

/* get an escaped character from the keyboard */
int esc_getch() {

	int c = getch();

	//if (c != ERR) logout(1, "received char %02x ('%c'), esc=%02x, '%c'", c, c, config_get_esc_char() & 0x1f, sline_get_esc_char() & 0x1f);

	if (c != (config_get_esc_char() & 0x1f)) {
		return c;
	}

	logout(1, "escape: Entering wait loop");
	
	cf_cfg_mode = 1;
	config_update();

	c = cur_getch();

	// if Ctrl-P again, exit pause/config mode
	if (c == (config_get_esc_char() & 0x1f)) {
		c = ERR;
	}

	int newc = c;

	do {
		c = newc;
		newc = ERR;

		cfgmode_t *p = cmds;
		while (p->opts) {
			if (index(p->opts, c)) {
				// found
				newc = p->cmd();
				break;
			}
			p++;
		}
	} while (newc != ERR);

	cf_cfg_mode = 0;
	config_update();

	return ERR;
}

/* ------------------------------------------------------------------- */

static int config_set_esc_char_param(const char *par) {
	config_set_esc_char(par[0]);
	return 0;
}

static int config_set_trace_mode_param(const char *par) {
	config_toggle_trace();
	return 0;
}

static int show_usage(const char *par) {
	usage();
	return 0;
}

static config_t config_pars[] = {
	{ "trace-mode", 't', NULL, config_set_trace_mode_param, "Toggle trace mode (defaults to off)" },
	{ "escape-char", 'e', "char", config_set_esc_char_param, "set Ctrl-char to enter pause/config mode (defaults to 'p')" },
	{ "help", '?', NULL, show_usage, "Show this help" },
	{ NULL }
};

void config_init() {

	config_register(config_pars);

	cpu_set_trace(0);
}

