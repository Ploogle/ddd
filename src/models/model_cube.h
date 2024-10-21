#include "../engine/symbols.h"
#define CUBE_SIZE 10

struct Mesh mesh_cube = {
	.name = "Cube Mesh",
	.numVertices = 8,
	.numIndices = 12 * 3, // 12 faces
	.origin = { CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2 },
	.vertices = (struct Vector3[]){
		{.x = 0, .y = 0, .z = 0 },
		{.x = 0, .y = CUBE_SIZE, .z = 0 },
		{.x = CUBE_SIZE, .y = CUBE_SIZE, .z = 0 },
		{.x = CUBE_SIZE, .y = 0, .z = 0 },
		{.x = CUBE_SIZE, .y = 0, .z = CUBE_SIZE },
		{.x = CUBE_SIZE, .y = CUBE_SIZE,.z = CUBE_SIZE },
		{.x = 0, .y = CUBE_SIZE, .z = CUBE_SIZE },
		{.x = 0, .y = 0, .z = CUBE_SIZE },
	},
	.min_bounds = {0,0,0},
	.max_bounds = {CUBE_SIZE,CUBE_SIZE,CUBE_SIZE},
	.indices = (uint16_t[]){
		0, 1, 2, // south
		3, 2, 5, // east
		4, 5, 6, // north
		7, 6, 1, // west
		1, 6, 5, // top
		7, 0, 4, // bottom
		
		2, 3, 0, // south2
		4, 3, 5, // east2
		7, 4, 6, // north2
		1, 0, 7,  // west2
		5, 2, 1, // top2
		0, 3, 4, // bottom2
	}
};