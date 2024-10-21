#ifndef __Gradient__
#define __Gradient__

#include <stdlib.h>
#include <math.h>

#define SINE_GRADIENT_LENGTH 128

float* SINE_GRADIENT[SINE_GRADIENT_LENGTH];

int SINE_GRADIENT_OFFSET = 0;

void Gradient_init() {
	for (int i = 0; i < SINE_GRADIENT_LENGTH; i++)
	{
		SINE_GRADIENT[i] = (float*)malloc(sizeof(float));
		*SINE_GRADIENT[i] = sinf((6.28f / (float)SINE_GRADIENT_LENGTH) * (float)i);
	}
}

void Gradient_tick(int t)
{
	SINE_GRADIENT_OFFSET += t;
	if (SINE_GRADIENT_OFFSET >= SINE_GRADIENT_LENGTH)
	{
		SINE_GRADIENT_OFFSET = 0;
	}
}

float Gradient_sample(float p)
{
	int idx = (int)(p * (float)SINE_GRADIENT_LENGTH) + SINE_GRADIENT_OFFSET;
	if (idx >= SINE_GRADIENT_LENGTH) {
		idx -= SINE_GRADIENT_LENGTH;
	}
	return *SINE_GRADIENT[idx];
}

#endif
