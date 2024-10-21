#include "symbols.h"

#ifndef __3dmath__
#define __3dmath__

#define MIN(a,b) ((((a)<(b))?(a):(b)))
#define MAX(a,b) (((a)>(b))?(a):(b))

float fast_atan2(float y, float x);


// -- Vector -- 

static inline int Vector3_equals(struct Vector3 a, struct Vector3 b);

struct Vector3 Vector3_cross(struct Vector3 a, struct Vector3 b);

float Vector3_dot(struct Vector3 a, struct Vector3 b);

struct Vector3 pnormal(struct Vector3* p1, struct Vector3* p2, struct Vector3* p3);

float Vector3_lengthSquared(struct Vector3* v);

float Vector3_length(struct Vector3* v);

struct Vector3 Vector3_add(struct Vector3* a, struct Vector3* b);
struct Vector3 Vector3_subtract(struct Vector3* a, struct Vector3* b);
struct Vector3 Vector3_multiply(struct Vector3* a, struct Vector3* b);
struct Vector3 Vector3_multiplyScalar(struct Vector3* a, float scalar);

struct Vector3 Vector3_normalize(struct Vector3 v);

struct Vector3 Vector3_getForward(struct Vector3* rotation);
struct Vector3 Vector3_getUp(struct Vector3* rotation);
struct Vector3 Vector3_getLeft(struct Vector3* rotation);

struct Vector3 Vector3_lerp(struct Vector3* a, struct Vector3* b, float w);

// -- Matrix 3x3 --
static const struct Matrix3x3 MATRIX_ZERO = { 0,0,0 };

struct Matrix3x3 Matrix3_multiply(struct Matrix3x3 l, struct Matrix3x3 r);
struct Matrix3x3 Matrix3_multiplyScalar(struct Matrix3x3 m, float s);
struct Matrix3x3 Matrix3_add(struct Matrix3x3 l, struct Matrix3x3 r);
struct Matrix3x3 Matrix3_lerp(struct Matrix3x3 a, struct Matrix3x3 b, float w);

struct Vector3 Matrix3_apply(struct Matrix3x3* m, struct Vector3* p);
void PTR_Matrix3_apply(struct Matrix3x3* m, struct Vector3* p);
struct Matrix3x3 Matrix3_getRotationX(float theta);
struct Matrix3x3 Matrix3_getRotationY(float theta);
struct Matrix3x3 Matrix3_getRotationZ(float theta);

struct Matrix3x3 Matrix3_lookAt(struct Vector3 from, struct Vector3 to);
struct Matrix3x3 Matrix3_fromQuaternion(struct Quaternion q);

// -- Matrix 4x4 --
struct Vector3 Matrix4_apply(struct Matrix4x4* m, struct Vector3* v);
void PTR_Matrix4_apply(struct Matrix4x4* m, struct Vector3* v);
struct Matrix4x4 Matrix4_multiply(struct Matrix4x4* l, struct Matrix4x4* r);
struct Matrix4x4 Matrix4_from3(struct Matrix3x3* m);

struct Matrix4x4 Matrix4_getTransform(
	float xTheta, float yTheta, float zTheta,
	float xPos, float yPos, float zPos,
	float xScale, float yScale, float zScale);
struct Matrix4x4 Matrix4_getQuaternionTransform(
	struct Quaternion q,
	float xPos, float yPos, float zPos,
	float xScale, float yScale, float zScale);
struct Matrix4x4 Matrix4_getMatrixTransform(
	struct Matrix3x3 mat,
	float xPos, float yPos, float zPos,
	float xScale, float yScale, float zScale);

// -- Quaternion --
struct Quaternion Quaternion_LookAt(struct Vector3 sourcePoint, struct Vector3 destPoint);
struct Vector3 Vector3_fromQuaternion(struct Quaternion* q);
struct Quaternion Quaternion_fromAngleAxis(struct Vector3* axis, float angle);
struct Quaternion Quaternion_normalize(struct Quaternion q);
struct Vector3 Vector3_applyQuaternion(struct Vector3* v, struct Quaternion* q);
struct Quaternion Quaternion_fromVector3(struct Vector3* v);

#endif
