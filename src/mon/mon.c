
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
#define	R_TRACE	4

#define	min(a,b)	((a)<(b)?(a):(b))

#define	MAXLEN		200

/* this should be more dynamic, but heck.... */
#define	MAXBANKS	16

static bank_t *banks[MAXBANKS];
static int numbanks = 0;
static bank_t *mon_bank = NULL;

// monitor flag
int monflag = 0;
// number of ops to trace (if monflag == 2)
static int trace_n;

static CPU *cpu;

/**************************************************************************/

static void cmd_err(const char *msg) {
	printf("%s", msg);
}

/**************************************************************************/
// parameter scanner

static char *scan_name(char *p, char *buf, int len) {
	
	int i = 0;

	if (len == 0) {
		cmd_err("Short buffer in scan_name");		
		return NULL;
	}
	
	len--;

	while (*p && isspace(*p)) {
		p++;
	}

	while (p[i] && isalnum(p[i])) {
		if (i >= len) {
			cmd_err("Length overflow in scan_name");		
			return NULL;
		}
		buf[i] = p[i];
		i++;
	}
	buf[i] = 0;

	return p+i;		
}

// decimal
static char *scan_dec(char *p, unsigned int *res) {

	unsigned int r = 0;

	while (*p && isspace(*p)) {
		p++;
	}

	if (!isdigit(*p)) {
		return NULL;
	}

	while(*p && isdigit(*p)) {
		r = r*10 + (*p & 0x0f);
		p++;
	}

	*res = r;
	return p;
}


// default hex
static char *scan_addr(char *p, unsigned int *res) {

	unsigned int r = 0;
	int i;
	char tmp;

	while (*p && isspace(*p)) {
		p++;
	}

	if (isalpha(*p)) {
		i=1;
		while (isalnum(p[i])) {
			i++;
		}
		tmp = p[i];
		p[i]= 0;

		if (label_byname(p, (int*) &r) == 0) {
			p[i]=tmp;
			*res = r;
			return p+i;
		}
		p[i]=tmp;
	}

	if (*p == '.') {
		p++;
		while(*p && isdigit(*p)) {
			r = r*10 + (*p & 0x0f);
			p++;
		}
	} else {
		if (*p == '$') {
			p++;
		}

		if (!isxdigit(*p)) {
			// error
			return NULL;
		}

		while (*p && isxdigit(*p)) {
			if (isdigit(*p)) {
				r = r*16 + (*p & 0x0f);
			} else {
				r = r*16 + (*p & 0x0f) + 9;
			}
			p++;
		}
	}
	*res = r;
	return p;
}




/**************************************************************************/

typedef struct {
	const char *name;
	int (*func)(char *pars, CPU *cpu, unsigned int *default_addr);
	const char *desc;
} cmd_t;

static int cmd_quit(char *pars, CPU *tocpu, unsigned int *default_addr) {
	return R_QUIT;
}

static int cmd_bank(char *pars, CPU *tocpu, unsigned int *default_addr) {

	if (*pars && pars[0]) {
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

static void trap(CPU *tcpu, scnt addr) {
	monflag = 2;
}

static int cmd_step(char *pars, CPU *tocpu, unsigned int *default_addr) {
	unsigned int n = 0;
	char *p = scan_dec(pars, &n);
	if (p != NULL) {
		trace_n = n;
	}
	return R_TRACE;
}

static int cmd_break(char *pars, CPU *tocpu, unsigned int *default_addr) {

	static int break_no = 0;
	
	char *p;
	unsigned int addr;
	char name[100];

	if (*pars && pars[0]) {
		// we have a break address?
		p = scan_addr(pars, &addr);
		if (p == NULL) {
			cmd_err("Missing parameters");
			return R_ERR;
		}
		p = scan_name(p, name, 99);
		if (p == NULL) {
			snprintf(name, 99, "%d", break_no++);
			name[99]=0;
		}
		mon_bank->addtrap(mon_bank, addr, trap, name);
	} else {
		// no break parameter - print list of defined breaks
		// TODO
	}
	return R_CONT;
}


static int getpars(char *pars, unsigned int *from, unsigned int *to) {

	char *p;

	p = scan_addr(pars, from);

	if (p == NULL) {
		return R_CONT;
	}

	p = scan_addr(p, to);

	if (p == NULL) {
		*to = *from;
	}

	if (to < from) {
		return R_NOPAR;
	}
	return R_CONT;
}

static int trace_dis(CPU *tocpu, unsigned int addr) {
	char line[MAXLEN];
	int llen = MAXLEN;
	int l = 0;

	l = cpu_log(tocpu, line, llen);
	
	cpu_dis(mon_bank, addr, line + l - 1, llen - l);

	printf(": %05x %s\n", addr, line);

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
		l = cpu_dis(mon_bank, from, line, llen);

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
	char buf[100];
	printf("   PC  AC   XR   YR   SP      NV1BDIZC\n");

	cpu_log(tocpu, buf, 100);

	printf("%s\n", buf);

	return R_CONT;
}



static int cmd_help(char *pars, CPU *tocpu, unsigned int *default_addr);

static cmd_t cmds[] = {
	{ "mem", cmd_mem, "Show a memory dump in hex: m <from_in_hex> [<to_in_hex>]" },
	{ "dis", cmd_dis, "Disassemble a memory area: d <from_in_hex> [<to_in_hex>]" },
	{ "reg", cmd_reg, "show current set of CPU registers" },
	{ "bank", cmd_bank, "show current bank or set new one" },
	{ "break", cmd_break, "show current break points in current bank or set new one" },
	{ "step", cmd_step, "step CPU one or more operations: s [num of ops]" },
	{ "help", cmd_help, "Show this help" },
	{ "x", cmd_quit, "Leave the monitor (eXit)" },
	{ "c", cmd_quit, "Leave the monitor (continue)" },
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
	printf("%s/%s: ", cpu_name(cpu), mon_bank->name);
}

void mon_line(CPU *tocpu) {

	char *line = NULL;
	ssize_t len = 0;
	size_t buflen = 0;
	char *p, *pp;
	int r = R_CONT;
	static cmd_t *c = NULL;

	unsigned int default_addr = cpu_pc(cpu);

	logout(0, "Entering monitor!");
	cur_exit();

	if (monflag == 2) {
		trace_dis(tocpu, default_addr);
		if (trace_n > 0) {
			trace_n --;
			if (trace_n > 0) {
				r = R_TRACE;
			}
		} 
	}

	monflag = 0;

	if (tocpu != NULL) {
		cpu = tocpu;
	}


	stop_ack_flag();

	// monitor loop (skipped on continuous trace)
	while ( r == R_CONT ) {

		mon_prompt();

		len = cur_getline(&line, &buflen);
		if (len <= 0) {
			break;
		}

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
			r = R_CONT;
		}
	}

	if (r == R_TRACE) {
		monflag = 2;
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


