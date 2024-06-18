#ifndef __quaternion__
#define __quaternion__

#include <stdbool.h>
#include <math.h>
#include "symbols.h"

struct Quaternion
{
    float x;
    float y;
    float z;
    float w;
} Quaternion_identity = { .w = 1.0f };

bool Quaternion_isIdentity(struct Quaternion* q);
float Quaternion_length(struct Quaternion* q);
float Quaternion_lengthSquared(struct Quaternion* q);
void Quaternion_normalize(struct Quaternion* q);
struct Quaternion Quaternion_fromAxisAngle(struct Vector3 axis, float angle);
struct Quaternion Quaternion_fromYawPitchRoll(float yaw, float pitch, float roll);
float Quaternion_dot(struct Quaternion* q1, struct Quaternion* q2);
struct Quaternion Quaternion_slerp(struct Quaternion* q1, struct Quaternion* q2, float amount);
struct Quaternion Quaternion_concat(struct Quaternion* v1, struct Quaternion* v2);
struct Quaternion Quaternion_multiply(struct Quaternion* v1, struct Quaternion* v2);
struct Quaternion Quaternion_scaleBy(struct Quaternion* q, float factor);

#endif