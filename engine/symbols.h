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
	struct Vector3* vertices;
	uint16_t* indices;
	uint32_t numVertices;
	uint32_t numIndices;
	struct Vector3 origin;
	struct Vector3 min_bounds;
	struct Vector3 max_bounds;
	char name[32];
	bool z_limit;
};

struct Triangle
{
	int indices[3];
	struct Vector3 center;
	bool visible;
	int shade;
};

struct Quaternion
{
	float x;
	float y;
	float z;
	float w;
};

#endif