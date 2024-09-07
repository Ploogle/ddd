#include "3dmath.h"
#include <math.h>
#include <string.h>

float fast_atan2(float y, float x)
{
	float a = MIN(fabsf(x), fabsf(y)) / MAX(fabsf(x), fabsf(y));
	float s = a * a;
	float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;

	if (fabsf(y) > fabsf(x)) r = 1.57079637f - r;
	if (x < 0) r = 3.14159274 - r;
	if (y < 0) r = -r;

	return r;
}

// -- Vector --

static inline int Vector3_equals(struct Vector3 a, struct Vector3 b)
{
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}

 struct Vector3 Vector3_cross(struct Vector3 a, struct Vector3 b)
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
	struct Vector3 v = Vector3_cross(
		(struct Vector3) {
				p2->x - p1->x, p2->y - p1->y, p2->z - p1->z
		},
	(struct Vector3){
		p3->x - p1->x, p3->y - p1->y, p3->z - p1->z
	});

	return Vector3_normalize(v);
}

struct Vector3 Vector3_getForward(struct Vector3* rotation)
{
	return (struct Vector3) {
		.x = cosf(rotation->x) * sinf(-rotation->y),
		.y = -sinf(rotation->x),
		.z = cosf(rotation->x) * cosf(-rotation->y)
	};
}

struct Vector3 Vector3_getUp(struct Vector3* rotation)
{
	return (struct Vector3) {
		.x = sinf(rotation->x) * sinf(-rotation->y),
		.y = -cosf(rotation->x),
		.z = sinf(rotation->x) * cosf(-rotation->y)
	};
}

struct Vector3 Vector3_getLeft(struct Vector3* rotation)
{
	return (struct Vector3) {
		.x = -cosf(rotation->y),
		.y = 0,
		.z = -sinf(rotation->y)
	};
}

struct Vector3 Vector3_lerp(struct Vector3* a, struct Vector3* b, float w)
{
	if (b == NULL) return *a;
	if (a == NULL) return *b;

	// (b - a) * t + a
	struct Vector3 out = Vector3_subtract(b, a);
	out = Vector3_multiplyScalar(&out, w);
	out = Vector3_add(&out, a);

	return out;
}


// -- Matrix --

struct Matrix4x4 Matrix4_from3(struct Matrix3x3* m)
{
	struct Matrix4x4 out = { 0,0,0,0 };
	
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			out.m[x][y] = m->m[x][y];
		}
	}

	out.m[3][3] = 1;
	
	return out;
}

struct Matrix4x4 Matrix4_multiply(struct Matrix4x4* l, struct Matrix4x4* r)
{
	struct Matrix4x4 out = { 0,0,0,0 };
	int row, col, k;
	for (row = 0; row < 4; row++) {
		for (col = 0; col < 4; col++) {
			out.m[row][col] = 0;
			for (k = 0; k < 4; k++) {
				out.m[row][col] += l->m[row][k] * r->m[k][col];
			}
		}
	}

	return out;
}


struct Matrix3x3 Matrix3_multiply(struct Matrix3x3 l, struct Matrix3x3 r)
{
	struct Matrix3x3 m = { .isIdentity = 0, .inverting = l.inverting ^ r.inverting };
	
	/*if ( l.isIdentity )
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
		{*/
			m.m[0][0] = l.m[0][0] * r.m[0][0] + l.m[1][0] * r.m[0][1] + l.m[2][0] * r.m[0][2];
			m.m[1][0] = l.m[0][0] * r.m[1][0] + l.m[1][0] * r.m[1][1] + l.m[2][0] * r.m[1][2];
			m.m[2][0] = l.m[0][0] * r.m[2][0] + l.m[1][0] * r.m[2][1] + l.m[2][0] * r.m[2][2];

			m.m[0][1] = l.m[0][1] * r.m[0][0] + l.m[1][1] * r.m[0][1] + l.m[2][1] * r.m[0][2];
			m.m[1][1] = l.m[0][1] * r.m[1][0] + l.m[1][1] * r.m[1][1] + l.m[2][1] * r.m[1][2];
			m.m[2][1] = l.m[0][1] * r.m[2][0] + l.m[1][1] * r.m[2][1] + l.m[2][1] * r.m[2][2];

			m.m[0][2] = l.m[0][2] * r.m[0][0] + l.m[1][2] * r.m[0][1] + l.m[2][2] * r.m[0][2];
			m.m[1][2] = l.m[0][2] * r.m[1][0] + l.m[1][2] * r.m[1][1] + l.m[2][2] * r.m[1][2];
			m.m[2][2] = l.m[0][2] * r.m[2][0] + l.m[1][2] * r.m[2][1] + l.m[2][2] * r.m[2][2];
		//}
		//else
		//	memcpy(&m.m, &l.m, sizeof(l.m));

		m.x = l.x * r.m[0][0] + l.y * r.m[0][1] + l.z * r.m[0][2] + r.x;
		m.y = l.x * r.m[1][0] + l.y * r.m[1][1] + l.z * r.m[1][2] + r.y;
		m.z = l.x * r.m[2][0] + l.y * r.m[2][1] + l.z * r.m[2][2] + r.z;
	//}
	
	return m;
}

struct Matrix3x3 Matrix3_multiplyScalar(struct Matrix3x3 m, float s)
{
	struct Matrix3x3 out = { 0,0,0 };
	
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			out.m[x][y] = m.m[x][y] * s;
		}
	}

	return out;
}

struct Matrix3x3 Matrix3_add(struct Matrix3x3 l, struct Matrix3x3 r)
{
	struct Matrix3x3 out = { 0,0,0 };

	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			out.m[x][y] = l.m[x][y] + r.m[x][y];
		}
	}

	return out;
}

struct Matrix3x3 Matrix3_lerp(struct Matrix3x3 a, struct Matrix3x3 b, float w)
{
	struct Matrix3x3 a_w = Matrix3_multiplyScalar(a, 1.f - w);
	struct Matrix3x3 b_w = Matrix3_multiplyScalar(b, w);

	return Matrix3_add(a_w, b_w);
}


struct Vector3 Matrix3_apply(struct Matrix3x3* m, struct Vector3* p)
{
	/*if ( m->isIdentity )
		return (struct Vector3) { p->x + m->x, p->y + m->y, p->z + m->z };*/

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

struct Matrix3x3 Matrix3_rotationFromAngles(struct Vector3 v, float angle)
{
	struct Matrix3x3 out = { 0 };

	float c = cosf(angle);
	float s = sinf(angle);
	float t = 1.0f - c;
	//  if axis is not already normalised then uncomment this
	// double magnitude = Math.sqrt(a1.x*a1.x + a1.y*a1.y + a1.z*a1.z);
	// if (magnitude==0) throw error;
	// a1.x /= magnitude;
	// a1.y /= magnitude;
	// a1.z /= magnitude;

	out.m[0][0] = c + v.x * v.x * t;
	out.m[1][1] = c + v.y * v.y * t;
	out.m[2][2] = c + v.z * v.z * t;


	float tmp1 = v.x * v.y * t;
	float tmp2 = v.z * s;
	out.m[1][0] = tmp1 + tmp2;
	out.m[0][1] = tmp1 - tmp2;
	tmp1 = v.x * v.z * t;
	tmp2 = v.y * s;
	out.m[2][0] = tmp1 - tmp2;
	out.m[0][2] = tmp1 + tmp2;    tmp1 = v.y * v.z * t;
	tmp2 = v.x * s;
	out.m[2][1] = tmp1 + tmp2;
	out.m[1][2] = tmp1 - tmp2;

	return out;
}

struct Matrix3x3 Matrix3_lookAt(struct Vector3 from, struct Vector3 to)
{
	struct Vector3 global_up = { 0,1,0 };

	struct Vector3 forward = Vector3_subtract(&from, &to);
	forward = Vector3_normalize(forward);

	struct Vector3 right = Vector3_cross(global_up, forward);
	right = Vector3_normalize(right);

	struct Vector3 up = Vector3_cross(forward, right);

	struct Matrix3x3 out = { 0,0,0,0 };
	out.m[0][0] = right.x;
	out.m[0][1] = right.y;
	out.m[0][2] = right.z;
	out.m[1][0] = up.x;
	out.m[1][1] = up.y;
	out.m[1][2] = up.z;
	out.m[2][0] = forward.x;
	out.m[2][1] = forward.y;
	out.m[2][2] = forward.z;
	return out;
}

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
			{ xP,				yP,								zP,								1}
		}
	};
}

struct Matrix4x4 Matrix4_getQuaternionTransform(
	struct Quaternion q,
	float xP, float yP, float zP, // position
	float xS, float yS, float zS) // scale
	 {

	float q0 = q.x;
	float q1 = q.y;
	float q2 = q.z;
	float q3 = q.w;

	float r00 = 2 * (q0 * q0 + q1 * q1) - 1;
	float r01 = 2 * (q1 * q2 - q0 * q3);
	float r02 = 2 * (q1 * q3 + q0 * q2);

	float r10 = 2 * (q1 * q2 + q0 * q3);
	float r11 = 2 * (q0 * q0 + q2 * q2) - 1;
	float r12 = 2 * (q2 * q3 - q0 * q1);

	float r20 = 2 * (q1 * q3 - q0 * q2);
	float r21 = 2 * (q2 * q3 + q0 * q1);
	float r22 = 2 * (q0 * q0 + q3 * q3) - 1;

	//return (struct Matrix3x3) {
	//	.m = {
	//		{r00, r01, r02},
	//		{r10, r11, r12},
	//		{r20, r21, r22}
	//	}
	//};
	return (struct Matrix4x4) {
		.m = {
			{ r00 * xS, r01 * xS, r02 * xS, 0},
			{ r10 * yS, r11 * yS, r12 * yS, 0},
			{ r20 * zS, r21 * zS, r22 * zS, 0},
			{ xP, yP, zP, 1}
		}
	};
};

struct Matrix4x4 Matrix4_getMatrixTransform(
	struct Matrix3x3 mat,
	float xP, float yP, float zP, // position
	float xS, float yS, float zS) // scale
{
	return (struct Matrix4x4) {
		.m = {
			{ mat.m[0][0] * xS, mat.m[0][1] * xS, mat.m[0][2] * xS, 0},
			{ mat.m[1][0] * yS, mat.m[1][1] * yS, mat.m[1][2] * yS, 0},
			{ mat.m[2][0] * zS, mat.m[2][1] * zS, mat.m[2][2] * zS, 0},
			{ xP, yP, zP, 1}
		}
	};
};

struct Vector3 Matrix4_apply(struct Matrix4x4* m, struct Vector3* v)
{
	struct Vector3 out;

	out.x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0];
	out.y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1];
	out.z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2];
	float w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + m->m[3][3];

	/*if (w != 0.0f)
	{*/
		w += 0.001f;
		out.x /= w;
		out.y /= w;
		out.z /= w;
	//}

	return out;
}
void PTR_Matrix4_apply(struct Matrix4x4* m, struct Vector3* v)
{
	/*struct Vector3 out;*/

	float x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0];
	float y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1];
	float z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2];
	float w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + m->m[3][3];

	/*if (w != 0.0f)
	{*/
		w += 0.001f;
		x /= w;
		y /= w;
		z /= w;
	//}

	v->x = x;
	v->y = y;
	v->z = z;

	//return out;
}

// == Quaternion ==

struct Matrix3x3 Matrix3_fromQuaternion(struct Quaternion q)
{
	float q0 = q.x;
	float q1 = q.y;
	float q2 = q.z;
	float q3 = q.w;

	float r00 = 2 * (q0 * q0 + q1 * q1) - 1;
	float r01 = 2 * (q1 * q2 - q0 * q3);
	float r02 = 2 * (q1 * q3 + q0 * q2);

	float r10 = 2 * (q1 * q2 + q0 * q3);
	float r11 = 2 * (q0 * q0 + q2 * q2) - 1;
	float r12 = 2 * (q2 * q3 - q0 * q1);

	float r20 = 2 * (q1 * q3 - q0 * q2);
	float r21 = 2 * (q2 * q3 + q0 * q1);
	float r22 = 2 * (q0 * q0 + q3 * q3) - 1;

	return (struct Matrix3x3) {
		.m = {
			{r00, r01, r02},
			{r10, r11, r12},
			{r20, r21, r22}
		}
	};
}

struct Quaternion Quaternion_fromAngleAxis(struct Vector3* axis, float angle)
{
	struct Vector3 n_axis = Vector3_normalize(*axis);
	float sinA = sinf(angle * .5f);
	float cosA = sinf(angle * .5f);
	return (struct Quaternion) {
		axis->x* sinA,
			axis->y* sinA,
			axis->z* sinA,
			cosA
	};
}

struct Vector3 Vector3_fromQuaternion(struct Quaternion* q) {
	// https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	struct Vector3 angles;


	// roll (x-axis rotation)
	double sinr_cosp = 2 * (q->w * q->z + q->x * q->y);
	double cosr_cosp = 1 - 2 * (q->z * q->z + q->x * q->x);
	angles.z = atan2f(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = sqrtf(1 + 2 * (q->w * q->x - q->z * q->y));
	double cosp = sqrtf(1 - 2 * (q->w * q->x - q->z * q->y));
	angles.x = 2 * atan2f(sinp, cosp) - 3.1415f / 2.0f;

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (q->w * q->y + q->z * q->x);
	double cosy_cosp = 1 - 2 * (q->x * q->x + q->y * q->y);
	angles.y = atan2f(siny_cosp, cosy_cosp);

	return angles;
}

struct Quaternion Quaternion_LookAt(struct Vector3 sourcePoint, struct Vector3 destPoint)
{
	struct Vector3 forward = Vector3_subtract(&destPoint, &sourcePoint);
	struct Vector3 normalized_forward = Vector3_normalize(forward);

	struct Vector3 stdForward = { 0, 0, 1 };

	float dot = Vector3_dot(stdForward, normalized_forward);

	/*if (abs(dot - (-1.0f)) < 0.000001f)
	{
		return (struct Quaternion) { 0, 1, 0, 3.1415926535897932f };
	}
	if (Math.Abs(dot - (1.0f)) < 0.000001f)
	{
		return (struct Quaternion) { 0, 0, 0, 1 };
	}*/

	float rotAngle = acosf(dot);
	struct Vector3 rotAxis = Vector3_cross(stdForward, normalized_forward);
	rotAxis = Vector3_normalize(rotAxis);
	return Quaternion_fromAngleAxis(&rotAxis, rotAngle);
}

struct Quaternion Quaternion_normalize(struct Quaternion q) {
	float magnitude = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);

	if (magnitude > 0.0f) {
		q.x /= magnitude;
		q.y /= magnitude;
		q.z /= magnitude;
		q.w /= magnitude;
	}

	return q;
}

struct Vector3 Vector3_applyQuaternion(struct Vector3* v, struct Quaternion* q)
{
	struct Vector3 u = { q->x, q->y, q->z };
	float s = q->w;

	float dot_uv = Vector3_dot(u, *v);
	float dot_uu = Vector3_dot(u, u);
	struct Vector3 cross_uv = Vector3_cross(u, *v);

	return (struct Vector3) {
		(2.0f * dot_uv * u.x) + ((s * s - dot_uu) * v->x) + (2.0f * s * cross_uv.x),
			(2.0f * dot_uv * u.y) + ((s * s - dot_uu) * v->y) + (2.0f * s * cross_uv.y),
			(2.0f * dot_uv * u.z) + ((s * s - dot_uu) * v->z) + (2.0f * s * cross_uv.z),
	};
}

struct Quaternion Quaternion_fromVector3(struct Vector3* v)
{
	struct Vector3 n_v = *v;// Vector3_normalize(*v);

	float yaw = n_v.x,
		pitch = -n_v.y,
		roll = 3.14f - n_v.z;

	float qx = sinf(roll / 2.0f) * cosf(pitch / 2.0f) * cosf(yaw / 2.0f) - cosf(roll / 2.0f) * sinf(pitch / 2.0f) * sinf(yaw / 2.0f);
	float qy = cosf(roll / 2.0f) * sinf(pitch / 2.0f) * cosf(yaw / 2.0f) + sinf(roll / 2.0f) * cosf(pitch / 2.0f) * sinf(yaw / 2.0f);
	float qz = cosf(roll / 2.0f) * cosf(pitch / 2.0f) * sinf(yaw / 2.0f) - sinf(roll / 2.0f) * sinf(pitch / 2.0f) * cosf(yaw / 2.0f);
	float qw = cosf(roll / 2.0f) * cosf(pitch / 2.0f) * cosf(yaw / 2.0f) + sinf(roll / 2.0f) * sinf(pitch / 2.0f) * sinf(yaw / 2.0f);
	
	struct Quaternion out = {
		.x = qx, .y = qy, .z = qz, .w = qw
	};

	return Quaternion_normalize(out);
}