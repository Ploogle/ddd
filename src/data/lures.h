#ifndef __lures__
#define __lures__

#include <float.h>
#include "./lure.h";

extern void lure_move_ascend(struct Lure* lure, struct Actor* actor, float distance);

/*
	Timing Patterns
*/
struct TimingComparison double_burst[] = {
	{ // Short Quick Crank
		.dur_f = &lte,
		.duration = SHORT_BURST,
		.speed_f = &gte,
		.speed = CRANK_SPEED_FAST,
	},
	{ // Short Quick Crank
		.dur_f = &lte,
		.duration = SHORT_BURST,
		.speed_f = &gte,
		.speed = CRANK_SPEED_FAST,
	},
	{ // Long Rest
		.dur_f = &lte,
		.duration = SHORT_BURST,
		.speed_f = &lte,
		.speed = 0,
	},
	END_OF_PATTERN
};

struct TimingComparison short_no_rest[] = {
	{ // Short Quick Crank
		.dur_f = &lte,
		.duration = SHORT_BURST,
		.speed_f = &gte,
		.speed = CRANK_SPEED_FAST,
	},
	END_OF_PATTERN,
};

struct TimingComparison long_short_burst_pattern[] = {
	{ // Crank
		.dur_f = &lte,
		.duration = LONG_BURST,
	},
	{ // Rest
		.dur_f = &gte,
		.duration = SHORT_BURST,
	},
	{ // Crank
		.dur_f = &lte,
		.duration = SHORT_BURST,
	},
	{ // Rest
		.dur_f = &gte,
		.duration = SHORT_BURST,
	},
	END_OF_PATTERN
};

/*
	Lures
*/
struct Lure DefaultLure = {
	.name = "Default",
	.mesh = &mesh_cube,
	.sink_depth = FLT_MIN,
	.drag = .1f,
	.density = 1.05f,
	.timing_pattern = &double_burst,
	.move = &lure_move_ascend,
	.update = &lure_update_descend,
	.fit_score_pattern = &pattern_fit_default,
};

#endif