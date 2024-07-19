


typedef struct CPU {
                const char      *name;

                BUS             *bus;

                saddr           pc;
                saddr           mask;
                
                scnt            sp;
                scnt            a;
                scnt            x;
                scnt            y;

                scnt            sr;

                scnt            flags;

                //alarm_context_t       actx;
                alarm_t         speed;
} CPU;


CPU *cpu_init(const char *name, int cyclespersec, int msperframe, int cmos);	/* init trap etc */

int cpu_run(void);	/* start execution at RESET address */

void CPU_reset();



