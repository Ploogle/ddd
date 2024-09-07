#include "gameobject.h"
#include "3dmath.h"

extern struct Vector3 GLOBAL_ORIGIN;
extern struct Vector3 GLOBAL_UNIT;

//void GameObject_setRotation(struct GameObject* go, float xTheta, float yTheta, float zTheta)
//{
//	go->rotationX4 = Matrix4_getRotationX(xTheta);
//	go->rotationY4 = Matrix4_getRotationY(yTheta);
//	go->rotationZ4 = Matrix4_getRotationZ(zTheta);
//}

void GameObject_updateTransform(struct GameObject* go)
{
	if (go->look_target != NULL)
	{
		/*struct Quaternion rot = Quaternion_LookAt(go->position, *go->look_target);
		struct Vector3 qv = Vector3_fromQuaternion(&rot);*/
		struct Matrix3x3 look = Matrix3_lookAt(go->position, *go->look_target);
		go->transform = Matrix4_getMatrixTransform(
			look,
			go->position.x, go->position.y, go->position.z,
			go->scale.x, go->scale.y, go->scale.z
		);

		go->forward = Vector3_subtract(&go->position, go->look_target);
		go->forward = Vector3_normalize(go->forward);


		//go->forward = Matrix3_apply(&look, &GLOBAL_UNIT);
	}
	else {

		struct Quaternion rot = Quaternion_fromVector3(&go->rotation);
		/*go->look_target != NULL ?
			Quaternion_LookAt(go->position, *go->look_target) :*/

		go->transform = Matrix4_getQuaternionTransform(
			rot,
			go->position.x, go->position.y, go->position.z,
			go->scale.x, go->scale.y, go->scale.z
		);
	}
}