#include "3dmath.h"
#include <math.h>
#include <string.h>

// -- Vector --

static inline int Vector3_equals(struct Vector3 a, struct Vector3 b)
{
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}

static inline struct Vector3 Vector3_cross(struct Vector3 a, struct Vector3 b)
{
	return (struct Vector3){ .x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x };
}

float Vector3_dot(struct Vector3 a, struct Vector3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float Vector3_lengthSquared(struct Vector3* v)
{
	return v->x * v->x + v->y * v->y + v->z * v->z;
}

static inline float Vector3_length(struct Vector3* v)
{
	return sqrtf(Vector3_lengthSquared(v));
}

/// <summary>
/// a + b
/// </summary>
struct Vector3 Vector3_add(struct Vector3* a, struct Vector3* b)
{
	return (struct Vector3) { a->x + b->x, a->y + b->y, a->z + b->z };
}

/// <summary>
/// a - b
/// </summary>
struct Vector3 Vector3_subtract(struct Vector3* a, struct Vector3* b)
{
	return (struct Vector3) { a->x - b->x, a->y - b->y, a->z - b->z };
}

/// <summary>
/// a * b
/// </summary>
struct Vector3 Vector3_multiply(struct Vector3* a, struct Vector3* b)
{
	return (struct Vector3) { a->x * b->x, a->y * b->y, a->z * b->z };
}

struct Vector3 Vector3_normalize(struct Vector3 v)
{
	float d = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	return Vector3_make(v.x / d, v.y / d, v.z / d);
}

struct Vector3 pnormal(struct Vector3* p1, struct Vector3* p2, struct Vector3* p3)
{
	struct Vector3 v = Vector3_cross(Vector3_make(p2->x - p1->x, p2->y - p1->y, p2->z - p1->z),
		Vector3_make(p3->x - p1->x, p3->y - p1->y, p3->z - p1->z));

	return Vector3_normalize(v);
}

// -- Matrix --

struct Matrix3x3 Matrix3_make(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33, int inverting)
{
	return (struct Matrix3x3){ .isIdentity = 0, .inverting = inverting, .m = {{m11, m12, m13}, {m21, m22, m23}, {m31, m32, m33}}, .x = 0, .y = 0, .z = 0 };
}

struct Matrix3x3 Matrix3_makeTranslate(float x, float y, float z)
{
	return (struct Matrix3x3){ .isIdentity = 1, .inverting = 0, .m = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, .x = x, .y = y, .z = z };
}

struct Matrix3x3 Matrix3_multiply(struct Matrix3x3 l, struct Matrix3x3 r)
{
	struct Matrix3x3 m = { .isIdentity = 0, .inverting = l.inverting ^ r.inverting };
	
	if ( l.isIdentity )
	{
		if ( r.isIdentity )
			m = identityMatrix;
		else
			memcpy(&m.m, &r.m, sizeof(r.m));
		
		m.x = l.x + r.x;
		m.y = l.y + r.y;
		m.z = l.z + r.z;
	}
	else
	{
		if ( !r.isIdentity )
		{
			m.m[0][0] = l.m[0][0] * r.m[0][0] + l.m[1][0] * r.m[0][1] + l.m[2][0] * r.m[0][2];
			m.m[1][0] = l.m[0][0] * r.m[1][0] + l.m[1][0] * r.m[1][1] + l.m[2][0] * r.m[1][2];
			m.m[2][0] = l.m[0][0] * r.m[2][0] + l.m[1][0] * r.m[2][1] + l.m[2][0] * r.m[2][2];

			m.m[0][1] = l.m[0][1] * r.m[0][0] + l.m[1][1] * r.m[0][1] + l.m[2][1] * r.m[0][2];
			m.m[1][1] = l.m[0][1] * r.m[1][0] + l.m[1][1] * r.m[1][1] + l.m[2][1] * r.m[1][2];
			m.m[2][1] = l.m[0][1] * r.m[2][0] + l.m[1][1] * r.m[2][1] + l.m[2][1] * r.m[2][2];

			m.m[0][2] = l.m[0][2] * r.m[0][0] + l.m[1][2] * r.m[0][1] + l.m[2][2] * r.m[0][2];
			m.m[1][2] = l.m[0][2] * r.m[1][0] + l.m[1][2] * r.m[1][1] + l.m[2][2] * r.m[1][2];
			m.m[2][2] = l.m[0][2] * r.m[2][0] + l.m[1][2] * r.m[2][1] + l.m[2][2] * r.m[2][2];
		}
		else
			memcpy(&m.m, &l.m, sizeof(l.m));

		m.x = l.x * r.m[0][0] + l.y * r.m[0][1] + l.z * r.m[0][2] + r.x;
		m.y = l.x * r.m[1][0] + l.y * r.m[1][1] + l.z * r.m[1][2] + r.y;
		m.z = l.x * r.m[2][0] + l.y * r.m[2][1] + l.z * r.m[2][2] + r.z;
	}
	
	return m;
}

struct Vector3 Matrix3_apply(struct Matrix3x3* m, struct Vector3* p)
{
	if ( m->isIdentity )
		return (struct Vector3) { p->x + m->x, p->y + m->y, p->z + m->z };

	struct Vector3 out;

	float x = p->x * m->m[0][0] + p->y * m->m[0][1] + p->z * m->m[0][2] + m->x;
	float y = p->x * m->m[1][0] + p->y * m->m[1][1] + p->z * m->m[1][2] + m->y;
	float z = p->x * m->m[2][0] + p->y * m->m[2][1] + p->z * m->m[2][2] + m->z;
	
	return Vector3_make(x, y, z);
}

float Matrix3_getDeterminant(struct Matrix3x3* m)
{
	return m->m[0][0] * m->m[1][1] * m->m[2][2]
		 + m->m[0][1] * m->m[1][2] * m->m[2][0]
		 + m->m[0][2] * m->m[1][0] * m->m[2][1]
		 - m->m[2][0] * m->m[1][1] * m->m[0][2]
		 - m->m[1][0] * m->m[0][1] * m->m[2][2]
		 - m->m[0][0] * m->m[2][1] * m->m[1][2];
}

struct Matrix3x3 Matrix3_getRotationX(float theta)
{
	return (struct Matrix3x3) {
		.m = {
			{ 1,			0,				0},
			{ 0, cosf(theta),	sinf(theta)},
			{ 0,			-sinf(theta),	cosf(theta)},
		}
	};
}

struct Matrix3x3 Matrix3_getRotationY(float theta)
{
	return (struct Matrix3x3) {
		.m = {
			{ cosf(theta),	0,	sinf(theta)},
			{ 0,			1,	0},
			{ -sinf(theta),	0,	cosf(theta)},
		}
	};
}


struct Matrix3x3 Matrix3_getRotationZ(float theta)
{
	return (struct Matrix3x3) {
		.m = {
			{ cosf(theta),	sinf(theta),	0},
			{ -sinf(theta), cosf(theta),	0},
			{ 0,			0,				1},
		}
	};
}

struct Vector3 Matrix4_apply(struct Matrix4x4* m, struct Vector3* v)
{
	struct Vector3 out;

	out.x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0];
	out.y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1];
	out.z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2];
	float w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + m->m[3][3];

	if (w != 0.0f)
	{
		out.x /= w;
		out.y /= w;
		out.z /= w;
	}

	return out;
}