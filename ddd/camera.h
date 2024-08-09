#ifndef __camera__
#define __camera__

#include "symbols.h"

typedef enum {
	RENDER_WIREFRAME,
	RENDER_FILL, 
	RENDER_WPF, // Wireframe + fill (thicker wireframe
	RENDER_SDKFILL,
	RENDER_SPLAT,
} RENDER_MODE;

struct Camera
{
	struct Vector3 position; // Position in space the camera is looking from
	struct Vector3 rotation;
	// TODO: Replace this with a properly-combined matrix or Quaternion or something
	struct Matrix3x3 rotate_transform;
	struct Matrix3x3 rotationX; // temporary rotation, for an example.
	struct Matrix3x3 rotationY; // temporary rotation, for an example.
	struct Matrix3x3 rotationZ; // temporary rotation, for an example.
	struct Vector3 look_target; // Position in space the camera is looking at
	struct Matrix4x4 projection; // Should be updated when fov, near, or far changes
	float fov; // Field of View
	float near; // Near clipping plane
	float far; // Far clipping plane
	RENDER_MODE render_mode;
};

struct Matrix4x4 Camera_getProjectionMatrix(struct Camera* camera);
struct Vector3 Camera_worldToScreenPos(struct Camera* camera, struct Vector3* worldPos);
void PTR_Camera_worldToScreenPos(struct Camera* camera, struct Vector3* worldPos);
void Camera_setRotation(struct Camera* camera, float xTheta, float yTheta, float zTheta);
struct Vector3 Camera_worldPosition(struct Camera* camera);


#endif