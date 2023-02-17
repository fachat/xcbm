

void config_set_esc_char(char c);

void config_set_speed(float hostperc, float limitperc);

/* may be extended at some point */
#define	SLINE_SHIFT_R	1
#define	SLINE_SHIFT_L	2

void config_set_shift(int shiftmap);


/* get an escaped character from the keyboard */
int esc_getch();

/* inlined for optimization */
extern int cf_trace_enabled;

static inline int config_is_trace_enabled() {
	return cf_trace_enabled;
}

