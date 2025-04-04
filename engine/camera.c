#include "camera.h"
#include "symbols.h"
#include "3dmath.h"
#include <math.h>

#define SCREEN_WIDTH_F 400.0f
#define SCREEN_HEIGHT_F 240.0f

extern double DELTA_TIME;

struct Matrix4x4 Camera_getProjectionMatrix(struct Camera* camera)  {
	struct Matrix4x4 projection = { 0 };

	float fAspectRatio = SCREEN_HEIGHT_F / SCREEN_WIDTH_F;
	float fFovRad = 1.0f / tanf(camera->fov * 0.5f / 180.0f * 3.14159f);

	projection.m[0][0] = fAspectRatio * fFovRad;
	projection.m[1][1] = fFovRad;
	projection.m[2][2] = CAMERA_FAR / (CAMERA_FAR - CAMERA_NEAR);
	projection.m[3][2] = (-CAMERA_FAR * CAMERA_NEAR) / (CAMERA_FAR - CAMERA_NEAR);
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
	p.z -= 1.0f;

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
	worldPos->z -= 1.0f;

	// TODO: Properly scale worldPos->z to unit size
}

struct Matrix3x3 Camera_getRotationMatrix(struct Camera* camera, float xTheta, float yTheta, float zTheta)
{
	struct Matrix3x3 out = Matrix3_multiply(
		Matrix3_getRotationX(xTheta),
		Matrix3_getRotationY(yTheta));

	out = Matrix3_multiply(out, Matrix3_getRotationZ(zTheta));
	return out;
	//camera->rotate_transform = out;

	/*struct Quaternion q = Quaternion_fromVector3(&camera->rotation);
	struct Matrix3x3 rot = Matrix3_fromQuaternion(q);

	camera->rotate_transform = rot;*/
}


void LookTarget_setTarget(struct LookTarget* lt, struct Vector3* target)
{
	if (lt->current == NULL)
	{
		lt->current = target;
	}
	else
	{
		lt->next = target;
	}

	lt->blend = 0;
	lt->is_tweening = true;

	lt->has_target = lt->current == NULL;
}

void LookTarget_tick(struct LookTarget* lt)
{
	// Don't bother unless we have at least one target and we're tweening
	if (!lt->has_target && lt->current != NULL) {
		lt->value = *lt->current;
		lt->has_target = true;
	}
	if (!lt->has_target) return;
	if (!lt->is_tweening) {
		lt->value = *lt->current;
		return;
	}

	// Currently tweening, lerp between current and next target
	lt->blend += DELTA_TIME * lt->tween_speed;

	if (lt->blend >= 1.f)
	{
		// Done tweening
		lt->blend = 1.f;
		lt->is_tweening = false;
			
		if (lt->next != NULL)
		{
			lt->current = lt->next;
			lt->next = NULL;
		}

		lt->value = *lt->current;
	}
	else
	{
		lt->value = Vector3_lerp(lt->current, lt->next, lt->blend);
	}
}