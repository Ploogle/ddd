#ifndef __GameObject__
#define __GameObject__

#include "symbols.h"

struct GameObject
{
	char name[32];

	// One mesh per object, if we want more we can add them as children.
	struct Mesh* mesh;

	struct Vector3 rotation; // euler
	struct Vector3 position;
	struct Vector3 scale;

	// TODO: This only needs to be calculated during render,
	// can replace with GameObject_getTransform that returns
	// the calculated value.
	struct Matrix4x4 transform;

	int num_children;
	struct GameObject** children;
	struct GameObject* parent;

	/* TODO:
	 - Probably needs some lifecycle function pointers incl. update
	 */
};

/** Set rotation with theta for x */
//void GameObject_setRotation(struct GameObject* go, float xTheta, float yTheta, float zTheta);

void GameObject_updateTransform(struct GameObject* go);

#endif