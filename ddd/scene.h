#ifndef _SCENE_
#define _SCENE_

#include "gameobject.h"
#include "pd_api.h"

struct Scene {
	char name[32];
	int numGameObjects;
	struct GameObject* gameObjects[256];
	int numCameras;
	struct Camera** cameras;

	/** Lifecycle methods */
	void (*init)(PlaydateAPI*);
	void (*update)(PlaydateAPI*);
	/*void (*addGameObject)(struct GameObject);
	void (*removeGameObject)(struct GameObject);*/
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



int Scene_addGameObject(PlaydateAPI* pd, struct Scene* scene, struct GameObject* obj);

bool Scene_removeGameObject(PlaydateAPI* pd, struct Scene* scene, struct GameObject* obj);

void Scene_update(PlaydateAPI* pd, struct Scene* scene);

#endif // !_SCENE_