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

float Vector3_lengthSquared(struct Vector3* v)
{
	return v->x * v->x + v->y * v->y + v->z * v->z;
}

float Vector3_length(struct Vector3* v)
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
struct Vector3 Vector3_multiplyScalar(struct Vector3* a, float scalar)
{
	return (struct Vector3) { a->x * scalar, a->y * scalar, a->z * scalar };
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

struct Vector3 Vector3_getForward(struct Vector3* rotation)
{
	//forward.x = cos(pitch) * sin(yaw);
	//forward.y = -sin(pitch);
	//forward.z = cos(pitch) * cos(yaw);

	return (struct Vector3) {
		.x = cosf(rotation->x) * sinf(rotation->y),
			.y = -sin(rotation->x),
			.z = cosf(rotation->x) * cosf(rotation->y)
	};
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

void PTR_Matrix3_apply(struct Matrix3x3* m, struct Vector3* p)
{
	//if (m->isIdentity)
	//	return (struct Vector3) { p->x + m->x, p->y + m->y, p->z + m->z };

	//struct Vector3 out;

	float x = p->x * m->m[0][0] + p->y * m->m[0][1] + p->z * m->m[0][2] + m->x;
	float y = p->x * m->m[1][0] + p->y * m->m[1][1] + p->z * m->m[1][2] + m->y;
	float z = p->x * m->m[2][0] + p->y * m->m[2][1] + p->z * m->m[2][2] + m->z;

	p->x = x; p->y = y; p->z = z;
	//return Vector3_make(x, y, z);
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
			{ 0,			cosf(theta),	-sinf(theta)},
			{ 0,			sinf(theta),	cosf(theta)},
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
			{ cosf(theta),	-sinf(theta),	0},
			{ sinf(theta), cosf(theta),		0},
			{ 0,			0,				1},
		}
	};
}


struct Matrix4x4 Matrix4_getRotationX(float theta)
{
	return (struct Matrix4x4) {
		.m = {
			{ 1,			0,				0,				0},
			{ 0,			cosf(theta),	-sinf(theta),	0},
			{ 0,			sinf(theta),	cosf(theta),	0},
			{ 0,			0,				0,				1}
		}
	};
}

struct Matrix4x4 Matrix4_getRotationY(float theta)
{
	return (struct Matrix4x4) {
		.m = {
			{ cosf(theta),	0,	sinf(theta), 0},
			{ 0,			1,	0,			 0},
			{ -sinf(theta),	0,	cosf(theta), 0},
			{ 0,			0,	0,			 1},
		}
	};
}

struct Matrix4x4 Matrix4_getRotationZ(float theta)
{
	return (struct Matrix4x4) {
		.m = {
			{ cosf(theta),	-sinf(theta),	0,	0},
			{ sinf(theta), cosf(theta),		0,	0},
			{ 0,			0,				1,	0},
			{ 0,			0,				0,	1}		
		}
	};
}

//#define cz cosf(zR)
//#define sz sinf(zR)
//#define cx cosf(xR)
//#define sx sinf(xR)
//#define cy cosf(yR)
//#define sy sinf(yR)
// Based on the math described here:
// https://en.wikipedia.org/wiki/Rotation_matrix#General_3D_rotations
struct Matrix4x4 Matrix4_getTransform(
	float xR, float yR, float zR, // rotation
	float xP, float yP, float zP, // position
	float xS, float yS, float zS) // scale
{
	float cz = cosf(zR);
	float sz = sinf(zR);
	float cx = cosf(xR);
	float sx = sinf(xR);
	float cy = cosf(yR);
	float sy = sinf(yR);
	return (struct Matrix4x4) {
		.m = {
			{ xS * (cy * cz),	xS * (sx * sy * cz - cx * sz),	xS * (cx * sy * cz + sx * sz),	0},
			{ yS * (cy * sz),	yS * (sx * sy * sz + cx * cz),	yS * (cx * sy * sz - sx * cz),	0},
			{ zS * -sy,			zS * (sx * cy),					zS * (cx * cy),					0},
			{ xP,				yP,							zP,								1}
		} // -yP
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
void PTR_Matrix4_apply(struct Matrix4x4* m, struct Vector3* v)
{
	/*struct Vector3 out;*/

	float x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0];
	float y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1];
	float z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2];
	float w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + m->m[3][3];

	if (w != 0.0f)
	{
		x /= w;
		y /= w;
		z /= w;
	}

	v->x = x;
	v->y = y;
	v->z = z;

	//return out;
}

struct Matrix4x4 Matrix4_fromQuaternion(struct Quaternion* q)
{
	//float xx = q->x;
	/*float xx = q->x * q->x;
	float yy = q->y * q->y;
	float zz = q->z * q->z;

	float xy = q->x * q->y;
	float wz = q->z * q->w;
	float xz = q->z * q->x;
	float wy = q->y * q->w;
	float yz = q->y * q->z;
	float wx = q->x * q->w;

	struct Matrix4x4 out;

	out.m[0][0] = 1.0f - 2.0f * (yy + zz);
	out.m[0][1] = 2.0f * (xy + wz);
	out.m[0][2] = 2.0f * (xz - wy);
	out.m[0][3] = 0.0f;
	out.m[1][0] = 2.0f * (xy - wz);
	out.m[1][1] = 1.0f - 2.0f * (zz + xx);
	out.m[1][2] = 2.0f * (yz + wx);
	out.m[1][3] = 0.0f;
	out.m[2][0] = 2.0f * (xz + wy);
	out.m[2][1] = 2.0f * (yz - wx);
	out.m[2][2] = 1.0f - 2.0f * (yy + xx);
	out.m[2][3] = 0.0f;
	out.m[3][0] = 0.0f;
	out.m[3][1] = 0.0f;
	out.m[3][2] = 0.0f;
	out.m[3][3] = 1.0f;*/

	//return out;

	/*result.M11 = 1.0f - 2.0f * (yy + zz);
	result.M12 = 2.0f * (xy + wz);
	result.M13 = 2.0f * (xz - wy);
	result.M14 = 0.0f;
	result.M21 = 2.0f * (xy - wz);
	result.M22 = 1.0f - 2.0f * (zz + xx);
	result.M23 = 2.0f * (yz + wx);
	result.M24 = 0.0f;
	result.M31 = 2.0f * (xz + wy);
	result.M32 = 2.0f * (yz - wx);
	result.M33 = 1.0f - 2.0f * (yy + xx);
	result.M34 = 0.0f;
	result.M41 = 0.0f;
	result.M42 = 0.0f;
	result.M43 = 0.0f;
	result.M44 = 1.0f;

	return result;
}*/
}