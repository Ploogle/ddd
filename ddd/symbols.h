#ifndef __symbols__
#define __symbols__

#include <stdint.h>

struct Vector3
{
	float x, y, z;
};

struct Vector3 Vector3_make(float x, float y, float z);

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
	struct Vector3* boundingBox;
	struct Vector3* vertices;
	uint16_t* indices;
	uint8_t* colors;

	// TODO: Move these to a GameObject / Sprite3D or whatever,
	// with proper translation + rotations etc.
	struct Matrix4x4 tRotationX; // temporary rotation, for an example.
	struct Matrix4x4 tRotationY; // temporary rotation, for an example.
	struct Matrix4x4 tRotationZ; // temporary rotation, for an example.
	struct Vector3 position;
	struct Vector3 origin;
};

#endif