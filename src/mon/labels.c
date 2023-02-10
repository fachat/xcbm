
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "types.h"
#include "labels.h"


const char **labels;

void label_init() {
	
	labels = malloc(sizeof(const char*) * 65536);
}

const char *label_lookup(int addr) {
	return labels[addr & 0xffff];
}

static void label_register(int addr, const char *lname) {


	int l = strlen(lname);
	if (l > 0 && lname[l-1] == ',') {
		l--;
	}
	
	logout(0, "Register label '%s' at %04x", lname, addr);

	addr &= 0xffff;

	const char *p = labels[addr];
	if (p != NULL) {
		logout(2, "Multiple definitions for label at %04x: %s, %s", addr, p, lname);
		return;
	}

	char *p2 = malloc(l + 1);
	strncpy(p2, lname, l);
	p2[l] = 0;

	labels[addr] = p2;
}


void label_load(const char *filename) {
	size_t len = 2000;
	char *linep = malloc(len);
	int s;
	char *lname = malloc(len);;
	unsigned int laddr;

	FILE *fp = fopen(filename, "r");

	if (fp == NULL) {
		logout(1, "Error opening label file '%s' -> '%s'", filename, strerror(errno));
		return;
	}

	while((s = getline(&linep, &len, fp)) >= 0) {

		sscanf(linep, "%s %x", lname, &laddr);

		label_register(laddr, lname);
	}

	fclose(fp);

	logout(0, "Successfully loaded label file '%s'", filename);
}

