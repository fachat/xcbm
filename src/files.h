

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "log.h"

/* emulate the x16 files interface required by sdcard.c */

#define	XSEEK_SET	SEEK_SET
#define	XSEEK_END	SEEK_END
#define	XSEEK_CUR	SEEK_CUR

typedef	int64_t Sint64;

typedef struct x16file {
	FILE *fp;
} x16file;

static inline struct x16file *x16open(const char *path, const char *attribs) {
	FILE *file = fopen(path, attribs);
	if (file == NULL) {
		return NULL;
	}	
	struct x16file *f = malloc(sizeof(struct x16file));
	f->fp = file;
	return f;
}

static inline void x16close(struct x16file *f) {
	int rv = fclose(f->fp);
	free(f);
	if (rv < 0) {
		logout(1, "Error closing file: %s", strerror(errno));
	}
}

static inline int64_t x16size(struct x16file *f) {
	struct stat info;

	int rv = fstat(fileno(f->fp), &info);

	if (rv < 0) {
		logout(1, "Error stat'ing file: %s", strerror(errno));
		return 0;
	}
	return info.st_size;
}

static inline int x16seek(struct x16file *f, int64_t pos, int origin) {

	return fseek(f->fp, pos, origin);
}


static inline uint64_t x16write(struct x16file *f, const uint8_t *data, uint64_t data_size, uint64_t data_count) {
	
	return fwrite((void*)data, data_size, data_count, f->fp);
}


static inline uint64_t x16read(struct x16file *f, uint8_t *data, uint64_t data_size, uint64_t data_count) {

	return fread((void*)data, data_size, data_count, f->fp);
}

