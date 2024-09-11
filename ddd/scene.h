#ifndef _SCENE_
#define _SCENE_

#include "actor.h"
#include "pd_api.h"

extern PlaydateAPI* pd;

struct Scene {
	char name[32];
	int numActors;
	struct Actor* actors[256];
	int numCameras;
	struct Camera** cameras;

	/** Lifecycle methods */
	void (*init)();
	void (*update)();
	/*void (*addActor)(struct Actor);
	void (*removeActor)(struct Actor);*/
};

/*
	I envision Scenes as large related groups of related UI and game objects.
	Multiple scenes can be activated at once through a parent-child hierarchy and visibility.
*/
//struct Scene
//{
//	char name[32];
//	bool visible; // Flag to render
//	bool active; // Flag to process (paused if inactive)
//
//	int num_scenes;
//	struct Scene** sub_scenes;
//
//	/*
//		TODO:
//		- 
//	*/
//};



int Scene_addActor(struct Scene* scene, struct Actor* obj);

bool Scene_removeActor(struct Scene* scene, struct Actor* obj);

void Scene_update(struct Scene* scene);

#endif // !_SCENE_