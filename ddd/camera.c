#include "camera.h"
#include "symbols.h"
#include "3dmath.h"
#include <math.h>

#define SCREEN_WIDTH_F 400.0f
#define SCREEN_HEIGHT_F 240.0f

struct Matrix4x4 Camera_getProjectionMatrix(struct Camera* camera)  {
	struct Matrix4x4 projection = { 0 };

	float fAspectRatio = SCREEN_HEIGHT_F / SCREEN_WIDTH_F;
	float fFovRad = 1.0f / tanf(camera->fov * 0.5f / 180.0f * 3.14159f);

	projection.m[0][0] = fAspectRatio * fFovRad;
	projection.m[1][1] = fFovRad;
	projection.m[2][2] = camera->far / (camera->far - camera->near);
	projection.m[3][2] = (-camera->far * camera->near) / (camera->far - camera->near);
	projection.m[2][3] = 1.0f;
	projection.m[3][3] = 0.0f;

	return projection;
}

/// <summary>
/// This returns a new Vector3, transformed from a world position to screen position for the given
/// Camera object.
/// 
/// MAKE SURE TO APPLY ANY RELEVANT WORLDSPACE OPERATIONS FIRST. This does not return a reference.
/// </summary>
struct Vector3 Camera_worldToScreenPos(struct Camera* camera, struct Vector3* worldPos)
{
	// TODO: Support perspective _and_ orthographic projection?
	// (Maybe also the cool blend tween that Unity's editor does.)
	struct Vector3 p = Matrix4_apply(&camera->projection, worldPos);
	
	p.x += 1.0f;
	p.y += 1.0f;
	p.x *= 0.5f * SCREEN_WIDTH_F;
	p.y *= 0.5f * SCREEN_HEIGHT_F;

	return p;
}
void PTR_Camera_worldToScreenPos(struct Camera* camera, struct Vector3* worldPos)
{
	// TODO: Support perspective _and_ orthographic projection?
	// (Maybe also the cool blend tween that Unity's editor does.)
	PTR_Matrix4_apply(&camera->projection, worldPos);

	worldPos->x += 1.0f;
	worldPos->y += 1.0f;
	worldPos->x *= 0.5f * SCREEN_WIDTH_F;
	worldPos->y *= 0.5f * SCREEN_HEIGHT_F;
}

void Camera_setRotation(struct Camera* camera, float xTheta, float yTheta, float zTheta)
{
	camera->rotationX = Matrix3_getRotationX(xTheta);
	camera->rotationY = Matrix3_getRotationY(yTheta);
	camera->rotationZ = Matrix3_getRotationZ(zTheta);
}

// TODO: Deprecate
struct Vector3 Camera_worldPosition(struct Camera* camera)
{
	return (struct Vector3) {
		.x = camera->position.x,
			.y = camera->position.y,
			.z = camera->position.z
	};
}
