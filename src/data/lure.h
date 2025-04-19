#ifndef __Lure__
#define __Lure__

#include <math.h>

#include "../../engine/actor.h"
#include "../../engine/symbols.h"

#define TIMING_BUFFER_LENGTH 20

#define SHORT_BURST .25
#define MEDIUM_BURST .5
#define LONG_BURST 1

//#define DIRECTION_IDLE 0
//#define DIRECTION_TOWARDS 1
//#define DIRECTION_AWAY 2

// Full rotations (360 degrees) / second
#define CRANK_SPEED_SLOW .5
#define CRANK_SPEED_REGULAR 1 
#define CRANK_SPEED_FAST 2

// TODO: Currently hard cut pass / fail, could weight by how close
float lte(float a, float b) { return a <= b ? 1 : 0; }
float gte(float a, float b) { return a >= b ? 1 : 0; }
float eq(float a, float b) { return a == b ? 1 : 0; }

struct TimingComparison {
	float (*dur_f)(float a, float b);
	float (*speed_f)(float a, float b);
	float duration;
	//char direction; // DIRECTION_IDLE, DIRECTION_TOWARDS, DIRECTION_AWAY
	char speed; // CRANK_SPEED_SLOW, CRANK_SPEED_REGULAR, CRANK_SPEED_FAST
	bool isEmpty;
};

#define END_OF_PATTERN { .isEmpty = true }

struct CrankPeriod
{
	float duration;
	float distance;
	//char direction; // DIRECTION_IDLE, DIRECTION_TOWARDS, DIRECTION_AWAY
};

typedef struct Lure
{
	char name[64];
	struct Mesh* mesh;
	float sink_depth; // how deep from the surface it can sink
	
	float drag; // how much drag the lure has in the water, from 0-1
	float density; // how fast the lure sinks / floats

	struct TimingComparison* timing_pattern;

	void (*move)(struct Lure* lure, struct Actor* actor, float distance);
	float (*fit_score_pattern)(float times[TIMING_BUFFER_LENGTH], struct TimingComparison comps[]);
	void (*update)(struct Lure* lure, struct Actor* actor);
};

void lure_update_descend(struct Lure* lure, struct Actor* actor)
{
	actor->position.y -= DELTA_TIME * lure->density;
	if (actor->position.y < 0) actor->position.y = 0;
}

void lure_move_ascend(struct Lure* lure, struct Actor* actor, float distance)
{
	actor->position.z += DELTA_TIME * distance * lure->drag;
	actor->position.y += DELTA_TIME * distance * lure->drag / lure->density;
}

/*
	Given a buffer of 5 timing pairs (crank, rest),
	fit the timings against a pattern.
*/
float pattern_fit_default(struct CrankPeriod cranks[TIMING_BUFFER_LENGTH], struct TimingComparison comps[])
{
	int num_comps = 0;
	for (; !comps[num_comps].isEmpty; num_comps++);

	float fit = 0, largest_fit = 0;
	int lf_start = 0, f_start = -1, f_end = -1;

	for (int j = 0; j < TIMING_BUFFER_LENGTH; j++)
	{
		fit = 0;
		f_start = -1;
		for (int i = j, ii = 0, c = 0; ii < TIMING_BUFFER_LENGTH; ii++, i++, c++)
		{
			if (cranks[i].duration <= 0 && cranks[i].distance <= 0)
			{
				if (f_end < 0) f_end = ii;
				continue;
			}
			if (i >= TIMING_BUFFER_LENGTH) i = 0;
			if (c >= num_comps) c = 0;

			float deg_per_second = fabsf((cranks[i].duration > 0 ?
				cranks[i].distance / cranks[i].duration / 360.f : 0));

			float current_fit = 0,
				duration_fit = 0, duration_split = 1.f,
				speed_fit = 0, speed_split = 1.f;
			if (comps[c].dur_f != NULL) {
				duration_fit = comps[c].dur_f(cranks[i].duration, comps[c].duration);
			}
			if (comps[c].speed_f != NULL) {
				speed_fit = comps[c].speed_f(deg_per_second, comps[c].speed);
			}
			if (comps[c].dur_f != NULL && comps[c].speed_f != NULL) {
				duration_split = speed_split = .5f;
			}
			current_fit += (duration_fit * duration_split) + (speed_fit * speed_split);
			fit += current_fit;

			if (current_fit > 0 && f_start < 0) f_start = i;
		}

		if (fit > largest_fit) 
		{
			largest_fit = fit;
			lf_start = f_start;
		}
	}
	if (f_end < 0) f_end = TIMING_BUFFER_LENGTH;

	float final_fit = largest_fit / (f_end - lf_start);
	return MAX(0, MIN(final_fit, 1));
}

float pattern_fit_morse(float times[10])
{
	// Instead of fitting for a timing pattern, fit for the morse dictionary.
	
	// Should detect "dit-length" dynamically (1/3 of a dot)
	// rather than defined lengths.
}

//void lure_fit_example() {
//	struct TimingComparison short_burst_pattern[] = {
//		{
//			.dur_f = &lt,
//			.duration = SHORT_BURST,
//		},
//		{
//			.dur_f = &gt,
//			.duration = SHORT_BURST,
//		},
//		END_OF_PATTERN
//	};
//	float crank_time_buffer[] = {
//		1, .1, 1.1f,
//		.2, 1.2f,
//		.25, .3, 
//		.4, .4,
//		0, 0, 0, 0
//	};
//
//	float fit = pattern_fit_default(crank_time_buffer, short_burst_pattern);
//};


#endif