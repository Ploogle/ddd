#ifndef __Actor__
#define __Actor__

#include "symbols.h"
#include "camera.h"
#include "scene.h"
#include <pd_api.h>

extern PlaydateAPI* pd;

typedef struct Actor
{
	// One mesh per object, if we want more we can add them as children.
	struct Mesh* mesh;
	struct Vector3 position;
	struct Vector3 rotation; // euler
	struct Vector3 scale;

	struct Vector3 forward;
	struct LookTarget look_target;

	// TODO: This only needs to be calculated during render,
	// can replace with Actor_getTransform that returns
	// the calculated value.
	struct Matrix4x4 transform;

	int num_children;
	struct Actor** children;
	struct Actor* parent;

	int render_layer;

	struct Scene* scene;
	int index_in_scene;

	/// <summary>
	/// A function that's called every frame.
	/// </summary>
	void (*update)();

	/// <summary>
	/// Transforms a list of vertices (v_out) in gameplay space
	/// </summary>
	void (*vertShader)(struct Actor* act, int time, struct Vector3* v_out);

	char name[32];
} Actor;

/** Set rotation with theta for x */
//void Actor_setRotation(struct Actor* go, float xTheta, float yTheta, float zTheta);

void Actor_updateTransform(struct Actor* act);

#endif