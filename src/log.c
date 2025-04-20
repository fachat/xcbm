
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include "log.h"

FILE *flog=NULL;

void logend(void){
	if(flog)
		fclose(flog);
}

int loginit(char *name){
	if(flog) {
		fclose(flog);
	}
	if(name){
		flog=fopen(name,"w");
	}
	atexit(logend);
	return(0);
}		

char *logout_x(const char *file, int lineno, int c,char *format, ...)
{
	char ch[]={" #:+!*"};
	static char line[MAXLOG+1];
	struct tm *ts;
	time_t ti;
	int sizeleft = MAXLOG;
	int sizeprtd;
	int pos = 0;

	va_list argl;
	va_start(argl,format);

	time(&ti);
	ts=localtime(&ti);

	line[pos++]=ch[c&0x07];
	sizeleft-=1;

	strftime(line+pos,22," %d-%b-%y %H:%M:%S  ",ts);
	sizeleft-=21;
	pos+=21;

	sizeprtd = snprintf(line+pos,sizeleft, "[%-12s] ", file);
	if (sizeprtd > sizeleft) {
		// truncated
		sizeprtd=sizeleft;
	}
	sizeleft -= sizeprtd;
	pos +=sizeprtd;

	vsnprintf(line+pos,sizeleft,format,argl);
/*	printf("%s\n",line);*/

	if(flog){
		fprintf(flog,"%s\n",line);
		fflush(flog);
	}
	return(line);
}
