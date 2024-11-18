#pragma once
#ifndef __MODEL_PLANE__
#define __MODEL_PLANE__

#include "../../engine/symbols.h"

struct Mesh flat_plane = {
	.name = "Plane",
	.numVertices = 4,
	.numIndices = 2 * 3,
	.origin = { .5f, .5f, .5f },
	.vertices = (struct Vector3[]){
		{.x = 0, .y = 0, .z = 0 },
		{.x = 1, .y = 0, .z = 0 },
		{.x = 1, .y = 0, .z = 1 },
		{.x = 0, .y = 0, .z = 1 },
	},
	.min_bounds = {0,0,0},
	.max_bounds = {1, 0, 1},
	.indices = (uint16_t[]){
		2, 1, 0,
		3, 2, 0,
	},
	.z_limit = true,
};

#endif