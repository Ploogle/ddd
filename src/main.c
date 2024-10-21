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

struct Vector3 camera_velocity = { 0,0,0 };

bool isUp = false;
bool isDown = false;
bool isForward = false;
bool isBackward = false;
bool isLeft = false;
bool isRight = false;
bool isRotateLeft = false;
bool isRotateRight = false;

void camera_init();

void loadScene(struct Scene* new_scene)
{
	// TODO: scene->unload() or whatever

	ActiveScene = new_scene;
	
	/*for (int v = 0; ActiveScene->views[v] != NULL; v++)
	{
		if (ActiveScene->views[v]->Visible)
			ActiveCamera = ActiveScene->views[v]->cameras[
				ActiveScene->views[v]->activeCameraIndex
			];
	}*/


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


		// HOW TO LOAD A BITMAP
		/*bluenoise = pd->graphics->loadBitmap("Assets/bluenoise-pd.png.bin", NULL);
		pd->graphics->getBitmapData(bluenoise, &blue_w, &blue_h, &blue_rb, &blue_m, &bluenoise_data);
		if (bluenoise == NULL)
			pd->system->logToConsole("Didn't load blue noise...");
		else
			pd->system->logToConsole("Loaded blue %i,%i %i %i", blue_w, blue_h, blue_rb, blue_m);*/

		//srand(time(NULL));

		pd->system->setUpdateCallback(update, pd);
		pd->display->setRefreshRate(50);
		pd->system->setAutoLockDisabled(true);

		loadScene(DefaultScene);

		Gradient_init();

		camera_init();
	}
	else if (event == kEventKeyPressed)
	{
		/* DEBUG CONTROLS FOR SIMULATOR */
		switch (arg)
		{
		case 0x69: // I
			isForward = true;
			break;
		case 0x6b: // K
			isBackward = true;
			break;
		case 0x6a: // J
			isLeft = true;
			break;
		case 0x6c: // L
			isRight = true;
			break;
		case 0x75: // U
			isRotateLeft = true;
			break;
		case 0x6f: // O
			isRotateRight = true;
			break;
		case 0x79: // Y
			isUp = true;
			break;
		case 0x68: // H
			isDown = true;
			break;
		}
	}
	else if (event == kEventKeyReleased)
	{
		switch (arg)
		{
		case 0x69: // I
			isForward = false;
			break;
		case 0x6b: // K
			isBackward = false;
			break;
		case 0x6a: // J
			isLeft = false;
			break;
		case 0x6c: // L
			isRight = false;
			break;
		case 0x75: // U
			isRotateLeft = false;
			break;
		case 0x6f: // O
			isRotateRight = false;
			break;
		case 0x79: // Y
			isUp = false;
			break;
		case 0x68: // H
			isDown = false;
			break;
		}
	}
	
	return 0;
}


bool look_lerp = false;
int look_lerp_dir = 1;
int target_idx = 1;
void handleButtons()
{
	PDButtons current, pressed;
	pd->system->getButtonState(&current, &pressed, NULL);
	float moveSpeed = 4.0f * DELTA_TIME;
	float rotSpeed = DELTA_TIME;

	if (current & kButtonUp) {
		camera_velocity.z -= moveSpeed;
	}
	else if (current & kButtonDown) {
		camera_velocity.z += moveSpeed;
	}
	
	if (current & kButtonLeft) {
		camera_velocity.x += moveSpeed;
	}
	else if (current & kButtonRight) {
		camera_velocity.x -= moveSpeed;
	}

	if (pressed & kButtonA && !camera_default.actor->look_target.is_tweening) {
		if (target_idx == 0)
		{
			target_idx = 1;
			LookTarget_setTarget(&camera_default.actor->look_target, &object_fish.position);
			//LookTarget_setTarget(&object_blahaj.look_target, &activeCamera->actor->position);
		}
		else if(target_idx == 1)
		{
			target_idx = 0;
			LookTarget_setTarget(&camera_default.actor->look_target, &object_lure.position);
			//LookTarget_setTarget(&object_blahaj.look_target, &object_cube2.position);
		}
	}

	/*if (pressed & kButtonB)
	{
		UnderwaterView.activeCameraIndex++;
		if (UnderwaterView.activeCameraIndex >= UnderwaterView.numCameras)
			UnderwaterView.activeCameraIndex = 0;
	}*/

	float crankDelta = pd->system->getCrankChange();
	if (camera_default.look_blend == 1.f) {
		camera_default.actor->rotation.y -= 0.01f * crankDelta;
	}
	else {
		camera_default.actor->position.y += 0.01f * crankDelta;
	}
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

		// Debug camera controls
		// TODO: hide this behind a flag or move to separate function
		float accelSpeed = 4.0f * DELTA_TIME;
		float maxSpeed = 5.0f;
		if (isForward)
		{
			camera_velocity.z -= accelSpeed;
			if (camera_velocity.z > maxSpeed) camera_velocity.z = maxSpeed;
		}
		else if (isBackward)
		{
			camera_velocity.z += accelSpeed;
			if (camera_velocity.z < -maxSpeed) camera_velocity.z = -maxSpeed;
		}

		if (isLeft)
		{
			camera_velocity.x += accelSpeed;
			if (camera_velocity.x > maxSpeed) camera_velocity.x = maxSpeed;
		}
		if (isRight)
		{
			camera_velocity.x -= accelSpeed;
			if (camera_velocity.x < -maxSpeed) camera_velocity.x = -maxSpeed;
		}

		if (isUp)
		{
			camera_velocity.y += accelSpeed;
			if (camera_velocity.y > maxSpeed) camera_velocity.y = maxSpeed;
		}
		if (isDown)
		{
			camera_velocity.y -= accelSpeed;
			if (camera_velocity.y < -maxSpeed) camera_velocity.y = -maxSpeed;
		}

		if (isRotateLeft)
		{
			camera_default.actor->rotation.y += DELTA_TIME;
		}
		else if (isRotateRight)
		{
			camera_default.actor->rotation.y -= DELTA_TIME;
		}

		struct Vector3 raw_forward = Vector3_getForward(&camera_default.actor->rotation);
		struct Vector3 raw_left = Vector3_getLeft(&camera_default.actor->rotation);
		struct Vector3 raw_up = Vector3_getUp(&camera_default.actor->rotation);
		struct Vector3 forward = Vector3_multiplyScalar(&raw_forward, camera_velocity.z);
		struct Vector3 left = Vector3_multiplyScalar(&raw_left, -camera_velocity.x);
		struct Vector3 up = Vector3_multiplyScalar(&raw_up, -camera_velocity.y);
		struct Vector3 dir = Vector3_add(&forward, &left);
		dir = Vector3_add(&dir, &up);

		camera_default.actor->position = Vector3_add(&camera_default.actor->position, &dir);
		camera_velocity = Vector3_multiplyScalar(&camera_velocity, 0.5f);

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

	DELTA_TIME = pd->system->getElapsedTime();
	pd->system->resetElapsedTime();
	DELTA_TIME *= TIME_SCALE;
	
	pd->graphics->clear(kColorBlack);
	pd->graphics->setFont(font);
	frame = pd->graphics->getFrame();

	handleButtons();

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

		for (int i = 0; i < ActiveScene->views[v]->actors[i] != NULL; i++)
		{
			Actor_drawMesh(frame, ActiveScene->views[v]->actors[i], ActiveCamera);
		}

		if (ActiveScene->views[v]->postdraw != 0x0)
			ActiveScene->views[v]->postdraw();
	}

	//pd->system->drawFPS(LCD_COLUMNS - 20, LCD_ROWS - 15);

	return 1;
}

