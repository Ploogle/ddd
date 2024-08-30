#ifndef __symbols__
#define __symbols__

#include <stdint.h>
#include <stdbool.h>

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
	uint32_t numVertices;
	uint32_t numIndices;
	//uint32_t numNormals;
	struct Vector3 min_bounds;
	struct Vector3 max_bounds;
	struct Vector3* vertices;
	//struct Vector3* normals;
	//struct Vector2* tex_coords; // UV coords, per vert
	uint16_t* indices;
	//uint8_t* colors;
	struct Vector3 origin;
};

struct Triangle
{
	int indices[3];
	struct Vector3 center;
	int shade;
	bool visible;
};

struct Quaternion
{
	float w;
	float x;
	float y;
	float z;
};

#endif