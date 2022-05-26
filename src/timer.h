
#define	MAXTIMER	20

void time_init(void);

/* registers timer to timing module, gives function to be called on underflow
   returns timer-number */ 
int time_register(void (*func)(int), const char *name);

extern int tval[];
extern int tset[];
extern void (*tfunc[])(int);
extern int tindex;
extern int tnumber;

#define	inc_time(a) for(tindex=0;tindex<tnumber;tindex++) { \
	if(tset[tindex] && (tval[tindex]-=(a))<0) tfunc[tindex](tval[tindex]);}
		
#define	time_reset(a)		tset[(a)]=0
#define	time_setval(a,b)	tval[(a)]=(b)
#define	time_seton(a)		tset[(a)]=1
#define	time_get(a)		tval[(a)]

 
