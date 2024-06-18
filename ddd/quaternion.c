//#include "quaternion.h"
//
///*
//    I got everything from here
//    https://github.com/microsoft/referencesource/blob/master/System.Numerics/System/Numerics/Quaternion.cs#L12
//*/
//
//bool Quaternion_isIdentity(struct Quaternion* q) {
//    return q->x == 0 && q->y == 0 && q->z == 0 && q->w == 1.0f;
//}
//
//float Quaternion_length(struct Quaternion* q)
//{
//    // Might need to be &q
//    return sqrtf(Quaternion_lengthSquared(q));
//}
//
//float Quaternion_lengthSquared(struct Quaternion* q)
//{
//    return q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w;
//}
//
//void Quaternion_normalize(struct Quaternion* q)
//{
//    float ls = q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w;
//
//    float invNorm = 1.0f / sqrtf(ls);
//
//    q->x = q->x * invNorm;
//    q->y = q->y * invNorm;
//    q->z = q->z * invNorm;
//    q->w = q->w * invNorm;
//}
//
//struct Quaternion Quaternion_fromAxisAngle(struct Vector3 axis, float angle)
//{
//    float halfAngle = angle * 0.5f;
//    return (struct Quaternion) {
//        .x = axis.x * sinf(halfAngle),
//        .y = axis.y * sinf(halfAngle),
//        .z = axis.z * sinf(halfAngle),
//        .w = cosf(halfAngle)
//    };
//}
//
//struct Quaternion Quaternion_fromYawPitchRoll(float yaw, float pitch, float roll)
//{
//    //  Roll first, about axis the object is facing, then
//    //  pitch upward, then yaw to face into the new heading
//    float sr, cr, sp, cp, sy, cy;
//
//    float halfRoll = roll * 0.5f;
//    sr = sinf(halfRoll);
//    cr = cosf(halfRoll);
//
//    float halfPitch = pitch * 0.5f;
//    sp = sinf(halfPitch);
//    cp = cosf(halfPitch);
//
//    float halfYaw = yaw * 0.5f;
//    sy = sinf(halfYaw);
//    cy = cosf(halfYaw);
//
//	return (struct Quaternion) {
//        .x = cy * sp * cr + sy * cp * sr,
//        .y = sy * cp * cr - cy * sp * sr,
//        .z = cy * cp * sr - sy * sp * cr,
//        .w = cy * cp * cr + sy * sp * sr,
//	};
//}
//
//    float Quaternion_dot(struct Quaternion* q1, struct Quaternion* q2)
//    {
//        return q1->x * q2->x +
//            q1->y * q2->y +
//            q1->z * q2->z +
//            q1->w * q2->w;
//    }
//
//    struct Quaternion Quaternion_slerp(struct Quaternion* q1, struct Quaternion* q2, float amount)
//    {
//        const float epsilon = 1e-6f;
//
//        float t = amount;
//
//        float cosOmega = q1->x * q2->x + q1->y * q2->y + q1->z * q2->z + q1->w * q2->w;
//
//        bool flip = false;
//
//        if (cosOmega < 0.0f)
//        {
//            flip = true;
//            cosOmega = -cosOmega;
//        }
//
//        float s1, s2;
//
//        if (cosOmega > (1.0f - epsilon))
//        {
//            // Too close, do straight linear interpolation.
//            s1 = 1.0f - t;
//            s2 = (flip) ? -t : t;
//        }
//        else
//        {
//            float omega = cosf(cosOmega);
//            float invSinOmega = (1.0f / sinf(omega));
//
//            s1 = sinf((1.0f - t) * omega) * invSinOmega;
//            s2 = (flip)
//                ? -sinf(t * omega) * invSinOmega
//                : sinf(t * omega) * invSinOmega;
//        }
//
//        return (struct Quaternion) {
//            .x = s1 * q1->x + s2 * q2->x,
//            .y = s1 * q1->y + s2 * q2->y,
//            .z = s1 * q1->z + s2 * q2->z,
//            .w = s1 * q1->w + s2 * q2->w,
//        };
//    }
//
//    struct Quaternion Quaternion_concat(struct Quaternion* v1, struct Quaternion* v2)
//    {
//        // Concatenate rotation is actually q2 * q1 instead of q1 * q2.
//        // So that's why value2 goes q1 and value1 goes q2.
//        float q1x = v2->x;
//        float q1y = v2->y;
//        float q1z = v2->z;
//        float q1w = v2->w;
//
//        float q2x = v1->x;
//        float q2y = v1->y;
//        float q2z = v1->z;
//        float q2w = v1->w;
//
//        // cross(av, bv)
//        float cx = q1y * q2z - q1z * q2y;
//        float cy = q1z * q2x - q1x * q2z;
//        float cz = q1x * q2y - q1y * q2x;
//
//        float dot = q1x * q2x + q1y * q2y + q1z * q2z;
//
//        return (struct Quaternion) {
//            .x = q1x * q2w + q2x * q1w + cx,
//                .y = q1y * q2w + q2y * q1w + cy,
//                .z = q1z * q2w + q2z * q1w + cz,
//                .w = q1w * q2w - dot
//        };
//    }
//
//    struct Quaternion Quaternion_multiply(struct Quaternion* v1, struct Quaternion* v2)
//    {
//        float q1x = v1->x;
//        float q1y = v1->y;
//        float q1z = v1->z;
//        float q1w = v1->w;
//
//        float q2x = v2->x;
//        float q2y = v2->y;
//        float q2z = v2->z;
//        float q2w = v2->w;
//
//        // cross(av, bv)
//        float cx = q1y * q2z - q1z * q2y;
//        float cy = q1z * q2x - q1x * q2z;
//        float cz = q1x * q2y - q1y * q2x;
//
//        float dot = q1x * q2x + q1y * q2y + q1z * q2z;
//
//        return (struct Quaternion) {
//            .x = q1x * q2w + q2x * q1w + cx,
//                .y = q1y * q2w + q2y * q1w + cy,
//                .z = q1z * q2w + q2z * q1w + cz,
//                .w = q1w * q2w - dot
//        };
//    }
//
//    struct Quaternion Quaternion_scaleBy(struct Quaternion* q, float factor)
//    {
//        return (struct Quaternion) {
//            .x = q->x * factor,
//                .y = q->y * factor,
//                .z = q->z * factor,
//                .w = q->w * factor
//        };
//    }