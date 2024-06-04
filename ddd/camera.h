#ifndef __camera__
#define __camera__

#include "symbols.h"

typedef enum {
	RENDER_WIREFRAME,
	RENDER_FILL, 
	RENDER_WPF, // Wireframe + fill (thicker wireframe
	RENDER_SDKFILL,
} RENDER_MODE;

struct Camera
{
	struct Vector3 position; // Position in space the camera is looking from
	struct Vector3 look_target; // Position in space the camera is looking at
	struct Matrix4x4 projection; // Should be updated when fov, near, or far changes
	float fov; // Field of View
	float near; // Near clipping plane
	float far; // Far clipping plane
	RENDER_MODE render_mode;
};

struct Matrix4x4 Camera_getProjectionMatrix(struct Camera* camera);
struct Vector3 Camera_worldToScreenPos(struct Camera* camera, struct Vector3* worldPos);

#endif