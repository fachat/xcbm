
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "cpu.h"
#include "labels.h"
#include "mem.h"
#include "mon.h"
#include "stop.h"
#include "asm6502.h"
#include "ccurses.h"
#include "labels.h"

#define	R_CONT	0
#define	R_QUIT	1
#define	R_ERR	2
#define	R_NOPAR	3

#define	min(a,b)	((a)<(b)?(a):(b))

#define	MAXLEN		200

/* this should be more dynamic, but heck.... */
#define	MAXBANKS	16

static bank_t *banks[MAXBANKS];
static int numbanks = 0;
static bank_t *mon_bank = NULL;

static CPU *cpu;

static void cmd_err(const char *msg) {
	printf("%s", msg);
}

typedef struct {
	const char *name;
	int (*func)(char *pars);
	const char *desc;
} cmd_t;

static int cmd_quit(char *pars) {
	return R_QUIT;
}

static int cmd_bank(char *pars) {

	if (*pars) {
		// we have a bank parameter
		for (int i = 0; i < numbanks; i++) {
			if (!strcmp(pars, banks[i]->name)) {
				// found it
				mon_bank = banks[i];
				break;
			}
		}
	} else {
		// no bank parameter - print list of available banks
		/* preliminary */
		int i = 0;
		while(i < numbanks) {
			printf("%s\n", banks[i]->name);
			i++;
		}
	}
	return R_CONT;
}


static int getpars(const char *pars, unsigned int *from, unsigned int *to) {

	int r = sscanf(pars, "%x %x", from, to);

	if (r == 0) {
		cmd_err("Missing parameters");
		return R_ERR;
	}
	if (r == 1) {
		*to = *from;
	}

	if (to < from) {
		return R_NOPAR;
	}
	return R_CONT;
}

static int cmd_dis(char *pars) {
	unsigned int from, to;
	int l, r;
	char line[MAXLEN];
	int llen = MAXLEN;

	if (r = getpars(pars, &from, &to)) {
		return r;
	}
	if (to == from) {
		to = from + 16;
	}

	do {
		l = dis6502(mon_bank, from, line, llen);

		printf(": %05x %s", from, line);

		printf("\n");

		from += l;
	} while (from < to);

	return R_CONT;
}

static int cmd_mem(char *pars) {
	unsigned int from, to;
	char buf[16];
	int l, r;

	if (r = getpars(pars, &from, &to)) {
		return r;
	}
	if (to == from) {
		to = from + 16;
	}

	do {
		const char *thislabel = label_lookup(from);
		int i;

		l = min(16, to-from+1);

		printf("; %-10s %05x ", thislabel == NULL ? "" : thislabel, from);

		for (i = 0; i < l; i++) {
			if (i > 0 && label_lookup(from + i)) {
				l = i;
				break;
			}
			buf[i] = mon_bank->peek(mon_bank, from + i); //getbyt(from + i);
		}
		
		for (i = 0; i < l; i++) {
			if (i == 8) printf(" ");
			printf(" %02x", buf[i] & 0xff);
		}
		for (; i < 16; i++) {
			if (i == 8) printf(" ");
			printf("   ");
		}

		printf("  ");
		for (i = 0; i < l; i++) {
			if (i == 8) printf(" ");
			char c = buf[i];
			if (c < 32) c |= 64;
			printf("%c", isprint(c)?c:'.');
		}
		printf("\n");

		from += l;
	} while (from < to);

	return R_CONT;
}


static int cmd_help(char *pars);


static cmd_t cmds[] = {
	{ "mem", cmd_mem, "Show a memory dump in hex: m <from_in_hex> [<to_in_hex>]" },
	{ "dis", cmd_dis, "Disassemble a memory area: d <from_in_hex> [<to_in_hex>]" },
	{ "bank", cmd_bank, "show current bank or set new one" },
	{ "help", cmd_help, "Show this help" },
	{ "x", cmd_quit, "Leave the monitor" },
	{ NULL }
};

static int cmd_help(char *pars) {
	
	cmd_t *p = cmds;

	while (p->name) {
		printf("%-10s: %s\n", p->name, p->desc);
		p++;
	}
	return R_CONT;
}

static int mon_parse(char *line) {
	
	cmd_t *c = cmds;

	while (c-> name != NULL) {

		int p = 0;
		// compare command name
		while (c->name[p] == line[p]) {
			p++;
		}

		// end condition
		if (line[p] == 0 || isspace(line[p])) {
			// skip whitespace before parameter
			while (isspace(line[p])) {
				p++;
			}
			// found
			return c->func(line+p);
		}
		c++;
	}
	return R_ERR;
}

static void mon_prompt() {
	printf("%s/%s: ", cpu_name(cpu), mon_bank->name);
}

void mon_line(CPU *tocpu) {

	char *line = NULL;
	ssize_t len = 0;
	size_t buflen = 0;
	char *p;
	int r;

	if (tocpu != NULL) {
		cpu = tocpu;
	}

	logout(0, "Entering monitor!");

	cur_exit();

	mon_prompt();

	stop_ack_flag();

	// monitor loop
	while ((len = cur_getline(&line, &buflen)) >= 0) {

		// remove trailing newline
		if (len > 0 
			&& line[len-1] == '\n') {
			len--;
			line[len] = 0;
		}

		p = line;
	
		// ignore leading whitespace	
		while (*p && isspace(*p)) { 
			p++; 	
		}

		if (*p) {
			if (r = mon_parse(p)) {
				if (r == R_ERR) {
					printf(" ?\r\n");
				}
				if (r == R_QUIT) {
					break;
				}
			}
		}
		mon_prompt();
	}
	
	if (len < 0) {
		if (errno == EINTR && stop_get_flag()) {
			// interrupted
			exit(1);
		}
	}

	if (len < 0) {
		logout(1, "Error in monitor getline: %d -> %d: %s", len, errno, strerror(errno));
	}
	
	free(line);

	cur_setup();

	stop_ack_flag();
}


/* currently only single CPU supported */
void mon_register_cpu(CPU *cpu_p) {
	cpu = cpu_p;
}

void mon_init() {

	// set CPU bank as initial bank
	cmd_bank("cpu");
}


void mon_register_bank(bank_t *bank) {

	int i = numbanks;

	if (i < MAXBANKS) {
		banks[i] = bank;
		numbanks++;
		return;
	}

	logout(4, "Too many banks - should not happen");
	exit(1);
}


