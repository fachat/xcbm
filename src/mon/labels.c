
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"
#include "config.h"
#include "types.h"
#include "labels.h"


const char **labels;

const char *label_lookup(int addr) {
	return labels[addr & 0xffff];
}

int label_byname(const char *name, int *value) {

	// totally slow
	for (int i = 0; i< 0xffff; i++) {

		if (labels[i] && strcmp(labels[i],name)==0) {
			*value = i;
			return 0;
		}
	}
	return -1;
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


int label_load(const char *filename) {
	size_t len = 2000;
	char *linep = malloc(len);
	int s;
	char *lname = malloc(len);;
	unsigned int laddr;

	FILE *fp = fopen(filename, "r");

	if (fp == NULL) {
		logout(1, "Error opening label file '%s' -> '%s'", filename, strerror(errno));
		return -1;
	}

	while((s = getline(&linep, &len, fp)) >= 0) {

		sscanf(linep, "%s %x", lname, &laddr);

		label_register(laddr, lname);
	}

	fclose(fp);

	logout(0, "Successfully loaded label file '%s'", filename);

	return 0;
}


static config_t label_pars[] = {
	{ "labels", 'l', "label_file", label_load, "Load xa65 label file for use in the monitor" },
	{ NULL }
};

void label_init() {
	
	labels = malloc(sizeof(const char*) * 65536);

	config_register(label_pars);
}


