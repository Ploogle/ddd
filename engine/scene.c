#include "scene.h"
#include "pd_api.h"

void Scene_update(struct Scene* scene)
{
	for (int v = 0; scene->views[v] != NULL; v++)
	{
		if (!scene->views[v]->Enabled) continue;

		if (scene->views[v]->update != 0x0)
			scene->views[v]->update();

		// First update all our actors
		for (int i = 0; i < scene->views[v]->actors[i] != NULL; i++)
		{
			// TODO: project actors right before update
			if (scene->views[v]->actors[i]->update != 0x0)
				scene->views[v]->actors[i]->update();

			LookTarget_tick(&scene->views[v]->actors[i]->look_target);

			Actor_updateTransform(scene->views[v]->actors[i]);
		}

		// Then update camera actors
		for (int i = 0; i < scene->views[v]->cameras[i] != NULL; i++)
		{
			if (scene->views[v]->cameras[i]->actor->update != 0x0)
				scene->views[v]->cameras[i]->actor->update();
		}
	}

	/*for (int i = 0; i < scene->numCameras; i++)
	{
		scene->cameras[i]->update();
	}*/

	// Now call the custom update logic, if defined
	if (scene->update!= NULL)
		scene->update();
}