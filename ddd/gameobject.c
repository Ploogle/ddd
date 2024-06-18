#include "gameobject.h"
#include "3dmath.h"

//void GameObject_setRotation(struct GameObject* go, float xTheta, float yTheta, float zTheta)
//{
//	go->rotationX4 = Matrix4_getRotationX(xTheta);
//	go->rotationY4 = Matrix4_getRotationY(yTheta);
//	go->rotationZ4 = Matrix4_getRotationZ(zTheta);
//}

void GameObject_updateTransform(struct GameObject* go)
{
	go->transform = Matrix4_getTransform(
		go->rotation.x, go->rotation.y, go->rotation.z,
		go->position.x, go->position.y, go->position.z,
		go->scale.x, go->scale.y, go->scale.z
	);
}