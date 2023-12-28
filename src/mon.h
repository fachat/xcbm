


void mon_init();

enum mon_rc {
	CONT,
	EXIT,
	TRACE
};

enum mon_rc mon_run();


extern int monflag;

void mon_register_cpu(CPU *cpu);

void mon_register_bank(bank_t *bank);

/* switch monitor to the given CPU if not NULL */
void mon_line(CPU *cpu);

