
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

char *logout(int c,char *format, ...)
{
	char ch[]={" #:+!*"};
	static char line[MAXLOG+1];
	static struct tm *ts;
	static time_t ti;

	va_list argl;
	va_start(argl,format);

	time(&ti);
	ts=localtime(&ti);

	line[0]=ch[c&0x07];

	strftime(line+1,22," %d-%b-%y %H:%M:%S  ",ts);
	
	vsnprintf(line+22,MAXLOG-22,format,argl);
/*	printf("%s\n",line);*/

	if(flog){
		fprintf(flog,"%s\n",line);
		fflush(flog);
	}
	return(line);
}
