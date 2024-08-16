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
// #include "cube.h"
//#include "cube2.h"
//#include "toa_head.h"
//#include "fish.h"
#include "../ddd/ddd.h"
#include "scenes\TestScene.h"
#include "../ddd/gradient.h"


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
PlaydateAPI* pd = NULL;
LCDFont* font = NULL;
uint8_t* frame;

const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16

struct Scene* ActiveScene;
struct Scene* DefaultScene = &TestScene;

//int frame_count = 0;
//int off = 0;
//int dir = 1;
struct Vector3 camera_velocity = { 0,0,0 };

bool isUp = false;
bool isDown = false;
bool isForward = false;
bool isBackward = false;
bool isLeft = false;
bool isRight = false;
bool isRotateLeft = false;
bool isRotateRight = false;

void loadScene(struct Scene* new_scene)
{
	// TODO: scene->unload() or whatever

	ActiveScene = new_scene;

	if (ActiveScene->init != NULL) {
		ActiveScene->init(pd);
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

		// HOW TO LOAD A BITMAP
		/*bluenoise = pd->graphics->loadBitmap("Assets/bluenoise-pd.png.bin", NULL);
		pd->graphics->getBitmapData(bluenoise, &blue_w, &blue_h, &blue_rb, &blue_m, &bluenoise_data);
		if (bluenoise == NULL)
			pd->system->logToConsole("Didn't load blue noise...");
		else
			pd->system->logToConsole("Loaded blue %i,%i %i %i", blue_w, blue_h, blue_rb, blue_m);*/

		//srand(time(NULL));

		pd->system->setUpdateCallback(update, pd);
		pd->display->setRefreshRate(0);
		pd->system->setAutoLockDisabled(true);

		loadScene(DefaultScene);

		Gradient_init();
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

void handleButtons(PlaydateAPI* pd)
{
	PDButtons pushed;
	pd->system->getButtonState(&pushed, NULL, NULL);
	float moveSpeed = 0.1f;
	float rotSpeed = 0.1f;

	if (pushed & kButtonUp) {
		//camera_default.position.z -= moveSpeed;
		camera_velocity.z -= moveSpeed;
	}
	else if (pushed & kButtonDown) {
		//camera_default.position.z += moveSpeed;
		camera_velocity.z += moveSpeed;
	}
	
	if (pushed & kButtonLeft) {
		//camera_default.position.x += moveSpeed;
		camera_velocity.x += moveSpeed;
	}
	else if (pushed & kButtonRight) {
		//camera_default.position.x -= moveSpeed;
		camera_velocity.x -= moveSpeed;
	}

	float crankDelta = pd->system->getCrankChange();
	camera_default.rotation.y += 0.01f * crankDelta;
}

void camera_update()
{
	float accelSpeed = 0.05f;
	float maxSpeed = .5f;
	/*struct Vector3 forward = Vector3_getForward(&camera_default.rotation);
	forward = Vector3_multiplyScalar(&forward, accelSpeed);*/
	if (isForward)
	{
		//camera_velocity = Vector3_add(&camera_velocity, &forward);
		camera_velocity.z -= accelSpeed;
		if (camera_velocity.z > maxSpeed) camera_velocity.z = maxSpeed;
	}
	else if (isBackward)
	{
		//camera_velocity = Vector3_subtract(&camera_velocity, & forward);

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
		camera_default.rotation.y += 0.025f;
	}
	else if (isRotateRight)
	{
		camera_default.rotation.y -= 0.025f;
	}

	// Update camera velocity / decel

	struct Vector3 c_forward = Vector3_getForward(&camera_default.rotation);
	struct Vector3 c_left = Vector3_getLeft(&camera_default.rotation);
	struct Vector3 forward = Vector3_multiplyScalar(&c_forward, camera_velocity.z);
	struct Vector3 left = Vector3_multiplyScalar(&c_left, -camera_velocity.x);
	struct Vector3 dir = Vector3_add(&forward, &left);
	//struct Vector3 adj_dir = Vector3_multiplyScalar(&dir, camera_velocity.z);

	camera_default.position = Vector3_add(&camera_default.position, &dir);
	//camera_default.position = Vector3_add(&camera_default.position, &camera_velocity);
	camera_velocity = Vector3_multiplyScalar(&camera_velocity, 0.5f);

	Camera_setRotation(&camera_default, camera_default.rotation.x, camera_default.rotation.y, camera_default.rotation.z);
}

static int update(void* userdata)
{
	Gradient_tick(5);

	// TODO: Pass deltaTime into update functions
	float deltaTime = pd->system->getElapsedTime();
	
	pd->graphics->clear(kColorBlack);
	pd->graphics->setFont(font);

	frame = pd->graphics->getFrame();

	handleButtons(pd);

	Scene_update(pd, ActiveScene);

	camera_update();

	//Grid_render(pd, frame, &camera_default);
	YPlane_render(pd, frame, &camera_default, 0);

	for (int i = 0; i < ActiveScene->numGameObjects; i++)
	{
		//GameObject_render(pd, frame, ActiveScene->gameObjects[i], &camera_default);
		GameObject_drawMesh(pd, frame, ActiveScene->gameObjects[i], &camera_default);
	}


	pd->system->drawFPS(LCD_COLUMNS - 20, LCD_ROWS - 15);
	pd->system->resetElapsedTime();

	return 1;
}

