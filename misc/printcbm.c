
#include <stdio.h>

char *cmd[]={ "end","for","next","data","input#","input","dim","read",
		    "let","goto","run","if","restore","gosub","return",
		    "rem","stop","on","wait","load","save","verify","def",
		    "poke","print#","print","cont","list","clr","cmd","sys",
		    "open","close","get","new","tab(","to","fn","spc(",
		    "then","not","step","+","-","*","/","^","and","or",
		    ">","=","<","sgn","int","abs","usr","fre","pos","sqr",
		    "rnd","log","exp","cos","sin","tan","atn","peek","len",
		    "str$","val","asc","chr$","left$","right$","mid$","go"};
		    
int main(int argc, char *argv[]){
	FILE *fp;
	int a,b,c,d;
	if(argc>1){
		fp=fopen(argv[1],"rb");
		if(fp){
			b=fgetc(fp);
			b=fgetc(fp);
			while(b!=EOF){
				a=fgetc(fp);
				a=a+256*fgetc(fp);
				if(a){
					a=fgetc(fp);
					a=a+256*fgetc(fp);
					printf("%d ",a);
					while(c=fgetc(fp)){
						if(c==EOF)
							break;
						if(c>=0x80 && c<0xcc)
							printf("%s",cmd[c-0x80]);
						else
							printf("%c",c);
					}
					printf("\n");
				} else
					break;
			}
			fclose(fp);
		} else 
			printf("File %s not found!\n",argv[1]);
	} else 
		printf("usage: listcbm file\n");
	return(0);
}
