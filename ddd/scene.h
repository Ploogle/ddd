#ifndef _SCENE_
#define _SCENE_

#include "actor.h"
#include "pd_api.h"

extern PlaydateAPI* pd;

struct View {
	bool Enabled;
	bool Visible;

	struct Actor** actors;
	struct Camera** cameras;
	int numCameras;
	int activeCameraIndex;

	/** Lifecycle methods */
	void (*init)();
	void (*predraw)(); // Before draw
	void (*draw)(); // Before models
	void (*postdraw)(); // After models
	void (*onEnable)();
	void (*onDisable)();
};

struct Scene {
	struct View** views;

	/** Lifecycle methods */
	void (*init)();
	void (*update)();
};



void Scene_update(struct Scene* scene);

#endif // !_SCENE_