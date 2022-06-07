
#include <stdlib.h>

/*
 * alarm handling
 * 
 * manages the callbacks for the various timed events,
 * so that the CPU does not have to check all timers every time.
 * 
 * slightly modeled after the VICE alarm handling.
 */

#define	MAX_ALARMS	64

typedef struct alarm_s {
	// name of the alarm
	char *name;

	// context, e.g. CPU. Also holds the clock of the context
	struct alarm_context_s *context;

	// callback
	void (*callback)(struct alarm_s *alarm, CLOCK current);
	
	// call data if needed (e.g. pointer relevant chip instance)
	void *data;

	// when the alarm should trigger. CLOCK_MAX does not trigger
	CLOCK clk;

} alarm_t;


typedef struct alarm_context_s {
	// name of the context
	const char *name;

	// clock value
	CLOCK clk;

	// the list of potential alarms 
	alarm_t	*alarms[MAX_ALARMS];

	// the number of alarms in the list
	int num_alarms;

	// next alarm in alarms[] list (NULL = none)
	alarm_t *next_alarm;

} alarm_context_t;

static inline void alarm_context_init(alarm_context_t *ctx, const char *name) {
	ctx->name = name;
	ctx->clk = 0;
	ctx->num_alarms = 0;
	ctx->next_alarm = NULL;
}

static inline void alarm_register(alarm_context_t *ctx, alarm_t *alarm) {

	if (ctx->num_alarms >= MAX_ALARMS) {
		logout(0, "not enough alarm slots");
		exit (1);
	}

	alarm->context = ctx;
	ctx->alarms[ctx->num_alarms] = alarm;

	ctx->num_alarms ++;
}

static inline void update_alarms(alarm_context_t *actx) {
	
	CLOCK c = CLOCK_MAX;
	alarm_t *alarm = NULL;

	for (int i = 0; i < actx->num_alarms; i++) {

		alarm_t *alrm = actx->alarms[i];

		if (alrm->clk < c) {
			c = alrm->clk;
			alarm = alrm;
		} 
	}
	actx->next_alarm = alarm;
}

static inline void set_alarm_clock(alarm_t *alarm, CLOCK newtime) {

	alarm->clk = newtime;

	alarm_t *next = alarm->context->next_alarm;

	// if we change the current alarm (because we have been triggered,
	// or our new time is earlier than the current next alarm time,
	// then update the next alarm info
	if (next == NULL
		|| next == alarm
		|| newtime < next->clk) {

		update_alarms(alarm->context);
	} 
}

static inline void set_alarm_clock_diff(alarm_t *alarm, int addtime) {
	
	set_alarm_clock(alarm, alarm->context->clk + addtime);
}

static inline void set_alarm_clock_plus(alarm_t *alarm, int addtime) {
	
	set_alarm_clock(alarm, alarm->clk + addtime);
}

static inline void clr_alarm_clock(alarm_t *alarm) {
	set_alarm_clock(alarm, CLOCK_MAX);
}

static inline void advance_clock(alarm_context_t *actx, int add) {

	actx->clk += add;

	alarm_t *next = actx->next_alarm;

	while (next && actx->clk >= next->clk) {
		// we ran into the next alarm

		// no holes in list, so no NULL check
		alarm_t *alrm = next;

		// this should set a new alarm clock, and thus update next_alarm
		alrm->callback(alrm, actx->clk);

		// update next_alarm pointer
		next = actx->next_alarm;
	}
}

