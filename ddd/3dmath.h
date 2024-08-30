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


// -- Matrix --

static struct Matrix3x3 identityMatrix = { .isIdentity = 1, .inverting = 0, .m = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}, .x = 0, .y = 0, .z = 0 };
static const struct Matrix3x3 MATRIX_ZERO = { 0,0,0 };

struct Matrix3x3 Matrix3_make(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33, int inverting);

struct Matrix3x3 Matrix3_makeTranslate(float dx, float dy, float dz);

struct Matrix3x3 Matrix3_multiply(struct Matrix3x3 l, struct Matrix3x3 r);

struct Vector3 Matrix3_apply(struct Matrix3x3* m, struct Vector3* p);
void PTR_Matrix3_apply(struct Matrix3x3* m, struct Vector3* p);

float Matrix3_getDeterminant(struct Matrix3x3* m);

struct Vector3 Matrix4_apply(struct Matrix4x4* m, struct Vector3* v);
void PTR_Matrix4_apply(struct Matrix4x4* m, struct Vector3* v);

struct Matrix3x3 Matrix3_getRotationX(float theta);
struct Matrix3x3 Matrix3_getRotationY(float theta);
struct Matrix3x3 Matrix3_getRotationZ(float theta);

struct Matrix4x4 Matrix4_getRotationX(float theta);
struct Matrix4x4 Matrix4_getRotationY(float theta);
struct Matrix4x4 Matrix4_getRotationZ(float theta);

struct Matrix3x3 Matrix3_fromAngleAxis(struct Vector3 v, float angle);
struct Matrix3x3 Matrix3_lookAt(struct Vector3 from, struct Vector3 to);

struct Matrix4x4 Matrix4_getTransform(
	float xTheta, float yTheta, float zTheta,
	float xPos, float yPos, float zPos,
	float xScale, float yScale, float zScale);


struct Quaternion Quaternion_LookAt(struct Vector3 sourcePoint, struct Vector3 destPoint);
struct Vector3 Vector3_fromQuaternion(struct Quaternion* q);
struct Quaternion Quaternion_fromAngleAxis(struct Vector3* axis, float angle);
struct Quaternion Quaternion_normalize(struct Quaternion q);
struct Vector3 Vector3_applyQuaternion(struct Vector3* v, struct Quaternion* q);
struct Quaternion Quaternion_fromVector3(struct Vector3* v);




#endif
