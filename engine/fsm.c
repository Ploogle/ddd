#include "fsm.h"

bool fsm_set_state(struct FSM* fsm, int state)
{
	int old_state = fsm->current_state;

	// If there's an edge function for the state transition,
	// call it first and only change if it doesn't return false.
	if (fsm->on_change != 0x0 &&
		fsm->on_change[old_state] != 0x0 &&
		fsm->on_change[old_state][state] != 0x0)
	{
		if (fsm->on_change[old_state][state]())
		{
			fsm->current_state = state;
			return true;
		}
		else
		{
			return false;
		}
	}

	// Otherwise just set the state and assume it's good.
	fsm->current_state = state;
	return true;
}