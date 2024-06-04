#include "gameobject.h"
#include "3dmath.h"

void GameObject_setRotation(struct GameObject* go, float xTheta, float yTheta, float zTheta)
{
	go->rotationX = Matrix3_getRotationX(xTheta);
	go->rotationY = Matrix3_getRotationY(yTheta);
	go->rotationZ = Matrix3_getRotationZ(zTheta);
}