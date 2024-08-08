#include "scene.h"
#include "pd_api.h"

int Scene_addGameObject(PlaydateAPI* pd, struct Scene* scene, struct GameObject* obj)
{
	/*scene->numGameObjects++;
	
	if (scene->numGameObjects == 1) {
		scene->gameObjects = pd->system->realloc(NULL, 8);
		*scene->gameObjects = pd->system->realloc(NULL, 8);
	}
	else {
		pd->system->realloc(scene->gameObjects, sizeof(struct GameObject*) * scene->numGameObjects);
	}

	scene->gameObjects[scene->numGameObjects - 1] = &obj;*/
	for (int idx = 0; idx < 256; idx++)
	{
		if (scene->gameObjects[idx] == NULL)
		{
			scene->numGameObjects++;
			scene->gameObjects[idx] = obj;

			obj->scene = scene;
			obj->scene_index = idx;

			return idx;
		}
	}

	return -1;
}

bool Scene_removeGameObject(PlaydateAPI* pd, struct Scene* scene, struct GameObject* obj)
{
	if (obj->scene != NULL)
	{
		scene->gameObjects[obj->scene_index] = NULL;

		obj->scene = NULL;
		obj->scene_index = 0;
	}
}

void Scene_update(struct PlaydateAPI* pd, struct Scene* scene)
{
	// First update all our game objects
	for (int i = 0; i < scene->numGameObjects; i++)
	{
		scene->gameObjects[i]->update(pd);

		GameObject_updateTransform(scene->gameObjects[i]);
	}

	/*for (int i = 0; i < scene->numCameras; i++)
	{
		scene->cameras[i]->update();
	}*/

	// Now call the custom update logic, if defined
	if (scene->update!= NULL)
	{
		scene->update(pd);
	}
}