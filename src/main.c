//
//  main.c
//  Extension
//
//  Created by Dave Hayden on 7/30/14.
//  Copyright (c) 2014 Panic, Inc. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "pd_api.h"
#include "../engine/ddd.h"
#include "scenes\scene_fishing.h"
#include "scenes\scene_debug.h"
#include "../engine/gradient.h"


/* 
	Definitions
*/
static int update(void* userdata);

#ifdef _WINDLL
__declspec(dllexport)
#endif

/*
	Values
*/
LCDFont* font = NULL;
LCDFont* FONT_MONTSERRAT_BLACK_24 = NULL;
LCDFont* FONT_MONTSERRAT_BLACK_ITALIC_24 = NULL;
LCDFont* FONT_MONTSERRAT_BOLD_14 = NULL;
uint8_t* frame;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
const char* fontpath2 = "/Assets/Fonts/Montserrat-Black.pft";
const char* fontpath3 = "/Assets/Fonts/Montserrat-BlackItalic.pft";
const char* fontpath4 = "/Assets/Fonts/Montserrat-Bold-14.pft";

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

struct Scene* ActiveScene;
struct Scene* DefaultScene = &FishingScene;
struct Camera* ActiveCamera;

PDButtons heldButtons, pressedButtons, releasedButtons;

void camera_init();

void loadScene(struct Scene* new_scene)
{
	// TODO: scene->unload() or whatever

	ActiveScene = new_scene;

	if (ActiveScene->init != NULL) {
		ActiveScene->init();
	}
}

int eventHandler(PlaydateAPI* _pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

	pd = _pd;

	if ( event == kEventInit )
	{
		const char* err;

		font = pd->graphics->loadFont(fontpath, &err);
		if ( font == NULL )
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		FONT_MONTSERRAT_BLACK_24 = pd->graphics->loadFont(fontpath2, &err);
		if (FONT_MONTSERRAT_BLACK_24 == NULL)
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		FONT_MONTSERRAT_BLACK_ITALIC_24 = pd->graphics->loadFont(fontpath3, &err);
		if (FONT_MONTSERRAT_BLACK_ITALIC_24 == NULL)
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		FONT_MONTSERRAT_BOLD_14 = pd->graphics->loadFont(fontpath4, &err);
		if (FONT_MONTSERRAT_BOLD_14 == NULL)
			pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontpath, err);

		pd->system->setUpdateCallback(update, pd);
		pd->display->setRefreshRate(50);
		pd->system->setAutoLockDisabled(true);

		loadScene(DefaultScene);

		Gradient_init();

		camera_init();
	}
	
	return 0;
}

// TODO: Make this a more generic Camera_init(Camera* c)
void camera_init()
{
	struct Camera* cam;
	for (int v = 0; ActiveScene->views[v] != NULL; v++) {

		for (int i = 0; i < ActiveScene->views[v]->numCameras; i++)
		{
			cam = ActiveScene->views[v]->cameras[i];

			cam->projection = Camera_getProjectionMatrix(cam);
			cam->rotate_transform = Camera_getRotationMatrix(
				cam,
				cam->actor->rotation.x,
				cam->actor->rotation.y,
				cam->actor->rotation.z
			);
			cam->look_blend = 1.f;
			//cam->actor->look_target = (struct LookTarget){
			//	.blend = 0,
			//	.is_tweening = false,
			//	.tween_speed = 2.f,
			//	//.current = &object_blahaj.position,
			//	.next = NULL,
			//};
		}
	}
}

void camera_update()
{
	struct Camera* ActiveCamera;
	for (int v = 0; v < ActiveScene->views[v] != NULL; v++)
	{
		if (!ActiveScene->views[v]->Enabled) continue;

		ActiveCamera = ActiveScene->views[v]->cameras[
			ActiveScene->views[v]->activeCameraIndex];

		// Calculate our look_target->value
		LookTarget_tick(&ActiveCamera->actor->look_target);

		if (ActiveCamera->look_blend == 1.f)
		{
			// Manually set yaw for left/right relative camera movement around looktarget.
			struct Vector3 target_forward = Vector3_subtract(&ActiveCamera->actor->position, &ActiveCamera->actor->look_target.value);
			ActiveCamera->actor->rotation.y = atan2f(-target_forward.x, target_forward.z);

			// Compute lookat matrix
			ActiveCamera->rotate_transform = Matrix3_lookAt(ActiveCamera->actor->position, ActiveCamera->actor->look_target.value);
		}
		else {
			// Compute rotation matrix for free-camera rotation thetas
			ActiveCamera->rotate_transform = Camera_getRotationMatrix(
				ActiveCamera,
				ActiveCamera->actor->rotation.x,
				ActiveCamera->actor->rotation.y,
				ActiveCamera->actor->rotation.z
			);
		}
	}
}

static int update(void* userdata)
{
	if (pd == NULL) return 1;

	Gradient_tick(5);

	playdate_resetCrankTicks();

	DELTA_TIME = pd->system->getElapsedTime();
	pd->system->resetElapsedTime();
	DELTA_TIME *= TIME_SCALE;
	
	pd->graphics->clear(kColorBlack);
	pd->graphics->setFont(font);
	frame = pd->graphics->getFrame();

	pd->system->getButtonState(&heldButtons, &pressedButtons, &releasedButtons);

	Scene_update(ActiveScene);

	camera_update();

	for (int v = 0; v < ActiveScene->views[v] != NULL; v++)
	{
		if (!ActiveScene->views[v]->Visible) continue;

		ActiveCamera = ActiveScene->views[v]->cameras[
			ActiveScene->views[v]->activeCameraIndex];
	
		if (ActiveScene->views[v]->predraw != 0x0)
			ActiveScene->views[v]->predraw();

		if (ActiveScene->views[v]->draw != 0x0)
			ActiveScene->views[v]->draw();

		for (int i = 0; ActiveScene->views[v]->actors[i] != NULL; i++)
		{
			Actor_drawMesh(frame, ActiveScene->views[v]->actors[i], ActiveCamera);
		}

		if (ActiveScene->views[v]->postdraw != 0x0)
			ActiveScene->views[v]->postdraw();
	}

	//pd->system->drawFPS(LCD_COLUMNS - 20, LCD_ROWS - 15);

	return 1;
}

