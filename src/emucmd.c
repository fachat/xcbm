
#include <stdio.h>
/*
#include <termios.h>
#include <unistd.h>
*/
#include "emu65.h"
#include "mem.h"

extern struct termios saveterm;
/*struct termios term;*/

int getline(char *n) {
	int i=0,c;
/*
	tcgetattr(STDIN_FILENO, &term);
	tcsetattr(STDIN_FILENO, TCSANOW, &saveterm);
*/
	while((c=getchar())!=10) {
		n[i++]=c;
	}
	n[i]='\0';
/*
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
*/
	return(c);
}

void onetrap( scnt trapadr, CPU *cpu) {
	rmtrap(trapadr);
	dismode =2;	
}

int command(void) {
	scnt i,adr;
	int er=3;
	unsigned int ad;
	static char l[100];

	do {
		getline(l);
	
		switch(l[0]) {
		case '1': 
			dismode=1;
			er =0;
			break;
		case '2':
			dismode=1;
			sscanf(l+1,"%d",&traplines);
			er =0;
			break;
		case 't':
			sscanf(l+1,"%x",&ad); adr=ad;
			settrap( adr, onetrap, "emucmd - onetrap");
			er=1;
			break;
		case 'x':
			er =2;
			break;
		case 'r':
			dismode=0;
			er =0;
			break;
		case 'm':
			sscanf(l+1,"%x",&ad); adr=ad;
			printf("\n%04x ",adr);
			for(i=0;i<8;i++) { printf("%02x ",getbyt(adr+i));}
			printf("\n");
			break;
		default:
			er =0;
			break;
		}
	} while(er==3);		
	return(er);
}

