
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


void usage() {

	printf("rompatch - patch a binary image with given other binary\n"
		"at a given address\n" 
		"Usage:\n" 
		"  romcheck <options> [inputfile]\n" 
		"patch inputfile ROM according to options.\n"
		"If inputfile is omitted, stdin is used.\n"
		"Options:\n"
		" -p <addr> <filename>\n"
		"     paste the contents of the given file into the\n"
		"     ROM image at the address given (relative to start\n"
		"     of ROM. E.g.\n"
		"      -p 0x7e00 inimmu\n"
		"     patches the contents of file 'inimmu' to address\n"
		"     $7e00 of the inputfile ROM image\n"
		" -o <outputfile>\n"
		"     define where to write the patched ROM image\n"
		"     If not used, file is written to stdout\n");

}

typedef struct {
	int addr;
	char *fname;
} patchinfo_t;

int main(int argc, char *argv[]) {

	char *fname_in = NULL;
	char *fname_out = NULL;

	FILE *fin = NULL;
	FILE *fout = stdout;

	unsigned char *buf = NULL;
	unsigned int size = 4096;	// initial buffer size
	unsigned int p = 0;
	int c = 0;

	patchinfo_t *patches = NULL;
	int npatches = 0;

	/* PARSE OPTIONS */

	p = 1;
	while (p < argc) {
		if (argv[p][0] != '-') {
			break;
		}

		switch(argv[p][1]) {
		case 'o':
			/* output file name */
			if (argv[p][2] != 0) {
				fname_out = &(argv[p][2]);
			} else
			if (p+1 < argc) {
				fname_out = &(argv[p+1][0]);
				p++;
			} else {
				fprintf(stderr, "Output file not given for '-o'\n");
				return(-1);
			}
			break;
		case 'p':
			/* this is the patch info */
			if (p+2 < argc) {
				patches = realloc(patches, (npatches + 1) * sizeof(patchinfo_t));
				if (patches == NULL) {
					fprintf(stderr, "Could not allocate memory\n");
					return (-1);
				}
				sscanf(argv[p+1], "%i", &patches[npatches].addr);
				patches[npatches].fname = argv[p+2];
				npatches++;
				p+=2;
			} else {
				fprintf(stderr, "Values missing for '-p'\n");
				return(-1);
			}
			break;
		case 'h':
		case '?':
			usage();
			return 0;
		default:
			fprintf(stderr, "Unknown option '%c'\n", argv[p][1]);
			return -1;
		}
		p++;
	}

	if (p < argc) {
		fname_in = &(argv[p][0]);
		p++;
	}

	if (p < argc) {
		fprintf(stderr, "Extra parameters '%s ...'\n", argv[p] );
		return -1;
	}

	/* EVALUATE OPTIONS */

	if (npatches == 0) {
		fprintf(stderr, "Warning: no patches given!\n");
	}

	/* OPEN INPUT/OUTPUT FILES */

	if (fname_in == NULL) {
		// we assume stdin
		fin = stdin;
	} else {
		fin = fopen(fname_in, "rb");
		if (fin == NULL) {
			fprintf(stderr, "Could not open file for reading '%s': %s\n", fname_in, strerror(errno));
			return(-1);
		}
	}

	buf = malloc(size);
	if (buf == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		return(-1);
	}

	p = 0;

	/* READ INPUT FILE */
	
	while ((c = fgetc(fin)) != EOF) {

		if (p >= size) {
			size *= 2;
			buf = realloc(buf, size);
			if (buf == NULL) {
				fprintf(stderr, "Could not re-allocate memory of size %d\n", size);
				return(-1);
			}
		}

		buf[p] = (unsigned char) c;
		p++;
	}

	size = p;

	/* CLEAN UP INPUT */

	fclose(fin);

	/* PROCESS PATCHES */

	for (int i = 0; i < npatches; i++) {

		patchinfo_t patch = patches[i];

		fprintf(stderr,"Processing patch %s\n", patch.fname);
	
		if (patch.addr > size) {
			fprintf(stderr, "Patch address %d larger than ROM size %d\n", patch.addr, size);
			return(-1);
		}

		p = patch.addr;

		fin = fopen(patch.fname, "rb");
		if (fin == NULL) {
			fprintf(stderr, "Could not open file for reading '%s': %s\n", patch.fname, strerror(errno));
			return(-1);
		}
	
		while ((c = fgetc(fin)) != EOF) {

			if (p >= size) {
				fprintf(stderr, "Patch '%s' too large!\n", patch.fname);
				return(-1);
			}

			buf[p] = c;

			p++;
		}
		fclose(fin);

	}


	/* OUTPUT RESULT */

	/* WRITE OUT FIXED ROM */

	if (fname_out != NULL) {
		fout = fopen(fname_out, "wb");
	} else {
		fout = stdout;
	}
	if (fout == NULL) {
		fprintf(stderr, "Could not open file for writing '%s': %s\n", fname_out, strerror(errno));
		return -1;
	}
	
	if (fwrite(buf, size, 1, fout) != 1) {
		fprintf(stderr, "Short write! Fix code!\n");
		fclose(fout);
		return -1;
	}

	fclose(fout);

	return(0);
}


