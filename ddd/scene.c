#include "scene.h"
#include "pd_api.h"

int Scene_addActor(struct Scene* scene, struct Actor* obj)
{
	/*scene->numActors++;
	
	if (scene->numActors == 1) {
		scene->gameObjects = pd->system->realloc(NULL, 8);
		*scene->gameObjects = pd->system->realloc(NULL, 8);
	}
	else {
		pd->system->realloc(scene->gameObjects, sizeof(struct Actor*) * scene->numActors);
	}

	scene->gameObjects[scene->numActors - 1] = &obj;*/
	for (int idx = 0; idx < 256; idx++)
	{
		if (scene->actors[idx] == NULL)
		{
			scene->numActors++;
			scene->actors[idx] = obj;

			obj->scene = scene;
			obj->scene_index = idx;

			return idx;
		}
	}

	return -1;
}

bool Scene_removeActor(struct Scene* scene, struct Actor* obj)
{
	if (obj->scene != NULL)
	{
		scene->actors[obj->scene_index] = NULL;

		obj->scene = NULL;
		obj->scene_index = 0;
	}
}

void Scene_update(struct Scene* scene)
{
	// First update all our game objects
	for (int i = 0; i < scene->numActors; i++)
	{
		// TODO: project actors right before update
		scene->actors[i]->update();

		LookTarget_tick(&scene->actors[i]->look_target);

		Actor_updateTransform(scene->actors[i]);
	}

	/*for (int i = 0; i < scene->numCameras; i++)
	{
		scene->cameras[i]->update();
	}*/

	// Now call the custom update logic, if defined
	if (scene->update!= NULL)
	{
		scene->update();
	}
}