#ifndef __symbols__
#define __symbols__

#include <stdint.h>
#include <stdbool.h>

// TODO: Most of these should be moved to their own files, or 3dmath.h

struct Vector3
{
	float x, y, z;
};

struct Vector3 Vector3_make(float x, float y, float z);

struct Vector2
{
	union {
		float x, y;
		float u, v;
	};
};

struct Matrix3x3
{
	int isIdentity : 1;
	int inverting : 1;
	float m[3][3];
	float x;
	float y;
	float z;
};

struct Matrix4x4
{
	float m[4][4];
};

struct Mesh
{
	char name[32];
	// TODO: rename these to under_score syntax
	uint32_t numVertices;
	uint32_t numIndices;
	uint32_t numNormals;
	struct Vector3* boundingBox;
	struct Vector3* vertices;
	struct Vector3* normals;
	struct Vector2* tex_coords; // UV coords, per vert
	uint16_t* indices; // TODO: Move to faces system if there becomes a need.
	uint8_t* colors;
	struct Vector3 origin;
};


/*
	I envision Scenes as large related groups of related UI and game objects.
	Multiple scenes can be activated at once through a parent-child hierarchy and visibility.
*/
struct Scene
{
	char name[32];
	bool visible; // Flag to render
	bool active; // Flag to process (paused if inactive)

	int num_scenes;
	struct Scene** sub_scenes;

	/*
		TODO:
		- 
	*/
};

#endif