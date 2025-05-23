#include "../ddd/symbols.h"

struct Mesh cube2 = {
	.name="cube",
	.numVertices = 8,
	.numIndices = 36,
	.vertices = (struct Vector3[]) {
		{.x = 1.000000, .y = 1.000000, .z = -1.000000 },
		{.x = 1.000000, .y = -1.000000, .z = -1.000000 },
		{.x = 1.000000, .y = 1.000000, .z = 1.000000 },
		{.x = 1.000000, .y = -1.000000, .z = 1.000000 },
		{.x = -1.000000, .y = 1.000000, .z = -1.000000 },
		{.x = -1.000000, .y = -1.000000, .z = -1.000000 },
		{.x = -1.000000, .y = 1.000000, .z = 1.000000 },
		{.x = -1.000000, .y = -1.000000, .z = 1.000000 }
	},
	.indices = (uint16_t[]) {
		4,2,0,
		2,7,3,
		6,5,7,
		1,7,5,
		0,3,1,
		4,1,5,
		4,6,2,
		2,6,7,
		6,4,5,
		1,3,7,
		0,2,3,
		4,0,1
	}
};
