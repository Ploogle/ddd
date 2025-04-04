#include <stdint.h>
#ifndef __WAVE_HEIGHTMAP__
#define __WAVE_HEIGHTMAP__

#define WAVE_HEIGHTMAP_COLUMNS 400
#define WAVE_HEIGHTMAP_ROWS 240

#define sample_wave_heightmap(x, y) (wave_heightmap_table[(y%WAVE_HEIGHTMAP_ROWS)*WAVE_HEIGHTMAP_COLUMNS+(x%WAVE_HEIGHTMAP_COLUMNS)])

extern float WAVE_RIPPLE_U;
extern float WAVE_RIPPLE_V;
extern uint8_t wave_heightmap_table[96000];

#endif
