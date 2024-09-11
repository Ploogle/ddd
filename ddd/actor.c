#include "actor.h"
#include "3dmath.h"

extern struct Vector3 GLOBAL_ORIGIN;
extern struct Vector3 GLOBAL_UNIT;

//void Actor_setRotation(struct Actor* go, float xTheta, float yTheta, float zTheta)
//{
//	go->rotationX4 = Matrix4_getRotationX(xTheta);
//	go->rotationY4 = Matrix4_getRotationY(yTheta);
//	go->rotationZ4 = Matrix4_getRotationZ(zTheta);
//}

void Actor_updateTransform(struct Actor* act)
{
	if (act->look_target.has_target)
	{
		/*struct Quaternion rot = Quaternion_LookAt(go->position, *go->look_target);
		struct Vector3 qv = Vector3_fromQuaternion(&rot);*/
		struct Matrix3x3 look = Matrix3_lookAt(act->position, act->look_target.value);
		act->transform = Matrix4_getMatrixTransform(
			look,
			act->position.x, act->position.y, act->position.z,
			act->scale.x, act->scale.y, act->scale.z
		);

		act->forward = Vector3_subtract(&act->position, &act->look_target.value);
		act->forward = Vector3_normalize(act->forward);

		//go->forward = Matrix3_apply(&look, &GLOBAL_UNIT);
	}
	else {
		struct Quaternion rot = Quaternion_fromVector3(&act->rotation);
		/*go->look_target != NULL ?
			Quaternion_LookAt(go->position, *go->look_target) :*/

		act->transform = Matrix4_getQuaternionTransform(
			rot,
			act->position.x, act->position.y, act->position.z,
			act->scale.x, act->scale.y, act->scale.z
		);
	}
}