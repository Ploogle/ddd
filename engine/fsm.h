#ifndef __FSM__
#define __FSM__

#include <stdbool.h>

struct FSM
{
	int current_state;

	bool (*on_change[32][32])();
	bool (*set_state)(int);
	void (*init)();
	void (*update)();

	char* name[64];
};

bool fsm_set_state(struct FSM* fsm, int state);

#endif