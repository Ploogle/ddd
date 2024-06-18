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
#include "../ddd/ddd.h"


PlaydateAPI* pd = NULL;

static int update(void* userdata);
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
const char* bluenoisepath = "Assets/bluenoise.png";
LCDFont* font = NULL;
LCDBitmap* bluenoise;
uint8_t* bluenoise_data = NULL;
int blue_w=0, blue_h=0, blue_rb=0, blue_m=0;

#ifdef _WINDLL
__declspec(dllexport)
#endif

#define CUBE_SIZE 10
struct Mesh mesh_cube = {
	.name = "Cube Mesh",
	.numVertices = 8,
	.numIndices = 12 * 3, // 12 faces
	.origin = { CUBE_SIZE / 2, CUBE_SIZE / 2, CUBE_SIZE / 2 },
	.vertices = (struct Vector3[]){
		{.x = 0, .y = 0, .z = 0 },
		{.x = 0, .y = CUBE_SIZE, .z = 0 },
		{.x = CUBE_SIZE, .y = CUBE_SIZE, .z = 0 },
		{.x = CUBE_SIZE, .y = 0, .z = 0 },
		{.x = CUBE_SIZE, .y = 0, .z = CUBE_SIZE },
		{.x = CUBE_SIZE, .y = CUBE_SIZE,.z = CUBE_SIZE },
		{.x = 0, .y = CUBE_SIZE, .z = CUBE_SIZE },
		{.x = 0, .y = 0, .z = CUBE_SIZE },
	},
	.indices = (uint16_t[]){
		0, 1, 2, // south
		3, 2, 5, // east
		4, 5, 6, // north
		7, 6, 1, // west
		1, 6, 5, // top
		7, 0, 4, // bottom
		
		2, 3, 0, // south2
		4, 3, 5, // east2
		7, 4, 6, // north2
		1, 0, 7,  // west2
		5, 2, 1, // top2
		0, 3, 4, // bottom2
	}
};

struct GameObject object_cube;

struct GameObject object_cube2 = {
	.name = "Cube 2",
	.mesh = &mesh_cube,
	.position = { 1, .5f, 0 },
	.rotation = { 0, 0, 0 },
	.scale = { .05f, .05f, .05f }
};

struct GameObject object_cube = {
	.name = "Cube",
	.mesh = &mesh_cube,
	.position = { 0, 1.5f, 0},
	.rotation = { 0, 0, 0 },
	.scale = { .1f, .1f, .1f }
};

struct Camera camera_default = {
	.near = 0.1f,
	.far = 1000.0f,
	.fov = 60.0f,
	.look_target = { 0, 0, 0 },
	.position = { 0, 1.0f, 2.5f },
	//.rotationX = { 0, 0, 0 },
	.rotation = {0,0,0},
	.render_mode = RENDER_WIREFRAME,
};

#define u_random ((float)rand() / (float)RAND_MAX) - ((float)rand() / (float)RAND_MAX) / 2

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16


uint8_t* frame;

int frame_count = 0;
int off = 0;
int dir = 1;
//
//struct Vector3 cube_rotation = { 0,0,0 };
//struct Vector3 cube_pos = { 0,0,0 };
//struct Vector3 camera_rotation = { 0,0,0 };
struct Vector3 camera_velocity = { 0,0,0 };
//struct Vector3 camera_rotationVelocity = { 0,0,0 };

//float xTheta = 0;
//float yTheta = 0;
//float zTheta = 0;
//float xPos = 0;
//float yPos = 0;
//float zPos = 0;
//float cam_xTheta = 0;
//float cam_yTheta = 0;
//float cam_zTheta = 0;
//struct Vector3 cube_position = { 0, 0, -10 };
bool isForward = false;
bool isBackward = false;
bool isLeft = false;
bool isRight = false;
bool isRotateLeft = false;
bool isRotateRight = false;

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

		/*bluenoise = pd->graphics->loadBitmap("Assets/bluenoise-pd.png.bin", NULL);
		pd->graphics->getBitmapData(bluenoise, &blue_w, &blue_h, &blue_rb, &blue_m, &bluenoise_data);
		if (bluenoise == NULL)
			pd->system->logToConsole("Didn't load blue noise...");
		else
			pd->system->logToConsole("Loaded blue %i,%i %i %i", blue_w, blue_h, blue_rb, blue_m);*/

		pd->system->setUpdateCallback(update, pd);
		pd->display->setRefreshRate(0);
		pd->display->setInverted(1); // quick cheat, make sure to properly set colors in the end

		camera_default.projection = Camera_getProjectionMatrix(&camera_default);
	}
	else if (event == kEventKeyPressed)
	{
		// Get ASCII value of keyboard press, simulator only
		// (Although... wonder if keyboards work via USB...)
		pd->system->logToConsole("key pressed %x", arg);


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
		camera_default.position.z -= moveSpeed;
	}
	else if (pushed & kButtonDown) {
		camera_default.position.z += moveSpeed;
	}
	
	if (pushed & kButtonLeft) {
		camera_default.position.x += moveSpeed;
	}
	else if (pushed & kButtonRight) {
		camera_default.position.x -= moveSpeed;
	}

	float crankDelta = pd->system->getCrankChange();
	//camera_default.position.y += crankDelta * moveSpeed;
	camera_default.rotation.y += 0.01f * crankDelta;
	//pos.z += crankDelta * moveSpeed;

	if (off < 0) dir = 1;
	if (off > 256) dir = -1;
	off += dir;
}

void cube_update()
{
	// Rotate cube
	object_cube.rotation.x += 0.01f;
	object_cube.rotation.y += 0.05f;
	object_cube.rotation.z += 0.025f;

	GameObject_updateTransform(&object_cube);
}

void camera_update()
{
	float accelSpeed = 0.05f;
	float maxSpeed = .5f;
	struct Vector3 forward = Vector3_getForward(&camera_default.rotation);
	forward = Vector3_multiplyScalar(&forward, accelSpeed);
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

	if (isRotateLeft)
	{
		camera_default.rotation.y += 0.025f;
	}
	else if (isRotateRight)
	{
		camera_default.rotation.y -= 0.025f;
	}

	// Update camera velocity / decel
	camera_default.position = Vector3_add(&camera_default.position, &camera_velocity);
	//if (!isForward && !isBackward && !isLeft && !isRight)
	camera_velocity = Vector3_multiplyScalar(&camera_velocity, 0.5f);

	Camera_setRotation(&camera_default, camera_default.rotation.x, camera_default.rotation.y, camera_default.rotation.z);
}

static int update(void* userdata)
{
	//char frame_text[64];
	float deltaTime = pd->system->getElapsedTime();
	
	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);

	frame = pd->graphics->getFrame();

	//handleButtons(pd);

	//cube_update();

	camera_update();

	// TODO: Need to fix orientation of all axes; most of them are flipped
	//GameObject_render(pd, frame, &object_cube, &camera_default);
	GameObject_render(pd, frame, &object_cube2, &camera_default);

	//Grid_render(pd, frame, &camera_default);

	//Ray_renderTest(pd, frame, &camera_default, off);

	pd->system->drawFPS(LCD_COLUMNS - 20, LCD_ROWS - 15);
	pd->system->resetElapsedTime();

	frame_count++;

	return 1;
}

