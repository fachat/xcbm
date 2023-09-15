
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"
#include "labels.h"
#include "mem.h"
#include "mon.h"
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

static struct sigaction monaction;
static struct sigaction oldaction;

static CPU *cpu;

static void cmd_err(const char *msg) {
	printf("%s", msg);
}

typedef struct {
	const char *name;
	int (*func)(char *pars, CPU *cpu, unsigned int *default_addr);
	const char *desc;
} cmd_t;

static int cmd_quit(char *pars, CPU *tocpu, unsigned int *default_addr) {
	return R_QUIT;
}

static int cmd_bank(char *pars, CPU *tocpu, unsigned int *default_addr) {

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

static int cmd_dis(char *pars, CPU *tocpu, unsigned int *default_addr) {
	unsigned int from, to;
	int l, r;
	char line[MAXLEN];
	int llen = MAXLEN;

	to = *default_addr;
	from = to;

	if (r = getpars(pars, &from, &to)) {
		return r;
	}
	if (to == from) {
		to = from + 32;
	}

	do {
		l = dis6502(mon_bank, from, line, llen);

		printf(": %05x %s", from, line);

		printf("\n");

		from += l;
	} while (from < to);

	*default_addr = from;

	return R_CONT;
}

static int cmd_mem(char *pars, CPU *tocpu, unsigned int *default_addr) {
	unsigned int from, to;
	char buf[16];
	int l, r;

	to = *default_addr;
	from = to;

	if (r = getpars(pars, &from, &to)) {
		return r;
	}
	if (to == from) {
		to = from + 64;
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

	*default_addr = from;

	return R_CONT;
}

static int cmd_reg(char *pars, CPU *tocpu, unsigned int *default_addr) {
	printf("   PC  AC XR YR SP NV1BDIZC\n");
	printf(": %04x %02x %02x %02x %02x %c%c%c%c%c%c%c%c\n", tocpu->pc, tocpu->a, tocpu->x, tocpu->y, tocpu->sp, 
	        cpu->sr & 0x80 ? 'N' : '-',
                cpu->sr & 0x40 ? 'V' : '-',
                cpu->sr & 0x20 ? '1' : '-',
                cpu->sr & 0x10 ? 'B' : '-',
                cpu->sr & 0x08 ? 'D' : '-',
                cpu->sr & 0x04 ? 'I' : '-',
                cpu->sr & 0x02 ? 'Z' : '-',
                cpu->sr & 0x01 ? 'C' : '-');

	return R_CONT;
}



static int cmd_help(char *pars, CPU *tocpu, unsigned int *default_addr);

static cmd_t cmds[] = {
	{ "mem", cmd_mem, "Show a memory dump in hex: m <from_in_hex> [<to_in_hex>]" },
	{ "dis", cmd_dis, "Disassemble a memory area: d <from_in_hex> [<to_in_hex>]" },
	{ "reg", cmd_reg, "show current set of CPU registers" },
	{ "bank", cmd_bank, "show current bank or set new one" },
	{ "help", cmd_help, "Show this help" },
	{ "x", cmd_quit, "Leave the monitor" },
	{ NULL }
};

static int cmd_help(char *pars, CPU *tocpu, unsigned int *default_addr) {
	
	cmd_t *p = cmds;

	while (p->name) {
		printf("%-10s: %s\n", p->name, p->desc);
		p++;
	}
	return R_CONT;
}

static cmd_t* mon_parse(char *line, char **pp) {
	
	cmd_t *c = cmds;
	*pp = NULL;

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
			*pp = line+p;
			return c;
		}
		c++;
	}
	return NULL;
}

static void mon_prompt() {
	printf("%s/%s: ", cpu->name, mon_bank->name);
}

void mon_line(CPU *tocpu) {

	char *line = NULL;
	ssize_t len = 0;
	size_t buflen = 0;
	char *p, *pp;
	int r;
	cmd_t *c = NULL;

	if (tocpu != NULL) {
		cpu = tocpu;
	}

	unsigned int default_addr = cpu->pc;

	logout(0, "Entering monitor!");

	cur_exit();

	mon_prompt();

	monflag = 0;

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
			// we can parse, so get back cmd_t and params (in pp)
			c = mon_parse(p, &pp);
		} else {
			// use previous command, but clear params (then default_addr is used where applicable)
			pp = "";
		}

		if (c != NULL) {
			r = c->func(pp, cpu, &default_addr);
		} else {
			r = R_ERR;
		}
		if (r == R_ERR) {
			printf(" ?\r\n");
		}
		if (r == R_QUIT) {
			break;
		}
		mon_prompt();
	}
	
	if (len < 0) {
		if (errno == EINTR && monflag) {
			// interrupted
			exit(1);
		}
	}

	if (len < 0) {
		logout(1, "Error in monitor getline: %d -> %d: %s", len, errno, strerror(errno));
	}
	
	free(line);

	cur_setup();

	monflag = 0;
}

int monflag = 0;

//static void mon_sigaction(int sig, siginfo_t siginfo, void *p);
static void mon_sighandler(int sig) {

	monflag = 1;
}


/* currently only single CPU supported */
void mon_register_cpu(CPU *cpu_p) {
	cpu = cpu_p;
}

void mon_init() {

	monflag = 0;

	monaction.sa_handler = mon_sighandler;
	monaction.sa_flags = 0;
	sigemptyset(&monaction.sa_mask);

	int er = sigaction(SIGINT, &monaction, &oldaction);

	if (er) {

		logout(0, "Could not establish signal handler: %s", 
			strerror(er));

	}

	// set CPU bank as initial bank
	cmd_bank("cpu", NULL, 0);
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


