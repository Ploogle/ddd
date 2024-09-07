#ifndef __GameObject__
#define __GameObject__

#include "symbols.h"
#include <pd_api.h>

extern PlaydateAPI* pd;

struct GameObject
{
	char name[32];

	// One mesh per object, if we want more we can add them as children.
	struct Mesh* mesh;
	//struct Vector3* offset_vertices;

	struct Vector3 rotation; // euler
	struct Vector3 position;
	struct Vector3 scale;

	struct Vector3 forward;
	
	struct Vector3* look_target;

	// TODO: This only needs to be calculated during render,
	// can replace with GameObject_getTransform that returns
	// the calculated value.
	struct Matrix4x4 transform;

	int num_children;
	struct GameObject** children;
	struct GameObject* parent;

	struct Scene* scene;
	int scene_index;

	/// <summary>
	/// A function that's called every frame.
	/// </summary>
	void (*update)();

	/// <summary>
	/// Transforms a list of vertices (v_out) in gameplay space
	/// </summary>
	void (*vertShader)(struct GameObject* go, int time, struct Vector3* v_out);
};

/** Set rotation with theta for x */
//void GameObject_setRotation(struct GameObject* go, float xTheta, float yTheta, float zTheta);

void GameObject_updateTransform(struct GameObject* go);

#endif