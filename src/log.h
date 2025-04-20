
#define MAXLOG 250

int loginit(char *name);

char *logout_x(const char *srcfile, int srcline, int c,char *format, ...);

#define	logout(c, format, ...)	\
	logout_x(__FILE__, __LINE__, c, format, ##__VA_ARGS__)

