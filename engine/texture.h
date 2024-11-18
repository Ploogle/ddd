#ifndef __Texture__
#define __Texture__

#include <stdint.h>

struct Texture {
	uint16_t* data;
	int width;
	int height;
};

#endif