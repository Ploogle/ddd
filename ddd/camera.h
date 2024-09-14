#ifndef __camera__
#define __camera__

#include "symbols.h"

#define CAMERA_FAR 100.0f
#define CAMERA_NEAR 0.1f
#define CAMERA_SCALE 40.0f

typedef enum {
	RENDER_WIREFRAME,
	RENDER_FILL, 
	RENDER_WPF, // Wireframe + fill (thicker wireframe
	RENDER_SDKFILL,
	RENDER_SPLAT,
} RENDER_MODE;


struct LookTarget
{
	struct Vector3 value;
	struct Vector3* current;
	struct Vector3* next;

	float blend;
	float is_tweening;
	float tween_speed;

	bool has_target;
};

void LookTarget_setTarget(struct LookTarget* lt, struct Vector3* target);
void LookTarget_tick(struct LookTarget* lt);


struct Camera
{
	struct Actor* actor;

	// TODO: Replace a lot of these values with a single Actor reference 
	// == Transforms ==
	//struct Vector3 position; // Position in space the camera is looking from
	//struct Vector3 rotation;
	//struct LookTarget look_target;

	// == Calculated / Cached Transforms ==
	struct Matrix3x3 rotate_transform;
	struct Matrix4x4 projection; // Should be updated when fov, near, or far changes
	struct Matrix3x3 previous_rotate_transform; // for look target blending

	// == Planes ==
	float fov; // Field of View
	float near; // Near clipping plane
	float far; // Far clipping plane
	float far_fog; // If greater than -1, fades out geo from far_fog up to far clip.
	float near_fog; // If greater than -1, fades out geo from 0 to near_fog.

	// == Render Properties ==
	RENDER_MODE render_mode;
	float look_blend; // Blend between normal projection and look_target calculated projection
	// TODO: light dir should be for the scene
	struct Vector3 light_dir; // "Global" (to camera) light direction
};

struct Matrix4x4 Camera_getProjectionMatrix(struct Camera* camera);
struct Vector3 Camera_worldToScreenPos(struct Camera* camera, struct Vector3* worldPos);
void PTR_Camera_worldToScreenPos(struct Camera* camera, struct Vector3* worldPos);
struct Matrix3x3 Camera_getRotationMatrix(struct Camera* camera, float xTheta, float yTheta, float zTheta);
void Camera_lookAt(struct Camera* camera, struct Vector3* point);

#endif