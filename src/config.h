

typedef struct config_s {
	// long name of the config (used with --)
	// if NULL, ends a list of config items
	const char *lname;
	// short name of the config (used with - and in pause mode).
	// if 0 then not applicable
	const char sname;
	// if not NULL, config takes a parameter of that name that is set with the next function
	const char *param;
	// called when the parameter is evaluated; if no config param name defined, function param should be ignored
	// returns 0 if ok; if nok, error code is returned
	int (*set_param)(const char *);
	// description
	const char *desc;
} config_t;

/* configure a list of config options. Last entry in pars array has lname == NULL */
void config_register(config_t *pars);

void config_print();

int config_parse(int argc, char *argv[]);

void config_init();

extern void usage();

/* -----------------------------------------------------------------------*/

void config_set_esc_char(char c);

void config_set_speed(float hostperc, float limitperc);

/* may be extended at some point */
#define	SLINE_SHIFT_R	1
#define	SLINE_SHIFT_L	2

void config_set_shift(int shiftmap);


/* get an escaped character from the keyboard */
int esc_getch();


