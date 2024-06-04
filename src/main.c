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
	.position = { 0, 0, 0},
	.rotationX = { 0 },
	.rotationY = { 0 },
	.rotationZ = { 0 },
	.scale = { .5f, .5f, .5f }
};

struct GameObject object_cube = {
	.name = "Cube",
	.mesh = &mesh_cube,
	.position = { 0, 0, -30},
	.rotationX = { 0 },
	.rotationY = { 0 },
	.rotationZ = { 0 },
	.scale = { 1, 1, 1 }
};

struct Camera camera_default = {
	.near = 0.1f,
	.far = 1000.0f,
	.fov = 60.0f,
	.look_target = {0,0,0},
	.position = {0,0,0},
	.render_mode = RENDER_WIREFRAME,
};

#define u_random ((float)rand() / (float)RAND_MAX) - ((float)rand() / (float)RAND_MAX) / 2

#define TEXT_WIDTH 86
#define TEXT_HEIGHT 16


uint8_t* frame;

int frame_count = 0;
int off = 0;
int dir = 1;
float fTheta = 0;

float xTheta = 0;
float yTheta = 0;
float zTheta = 0;
//struct Vector3 cube_position = { 0, 0, -10 };

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
	(void)arg; // arg is currently only used for event = kEventKeyPressed

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
		xTheta += rotSpeed;
	}
	else if (pushed & kButtonDown) {
		xTheta -= rotSpeed;
	}
	
	if (pushed & kButtonLeft) {
		yTheta -= rotSpeed;
	}
	else if (pushed & kButtonRight) {
		yTheta += rotSpeed;
	}

	float crankDelta = pd->system->getCrankChange();
	object_cube.position.z += crankDelta * moveSpeed;

	if (off < 0) dir = 1;
	if (off > 256) dir = -1;
	off += dir;
}

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	char frame_text[64];
	float deltaTime = pd->system->getElapsedTime();
	
	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);

	frame = pd->graphics->getFrame();

	handleButtons(pd);

	xTheta += 0.01f;
	yTheta += 0.05f;
	zTheta += 0.025f;

	// TODO: Need to fix orientation of all axes; most of them are flipped
	//       Also need to implement camera transform
	//Grid_render(pd, frame, &camera_default);

	GameObject_setRotation(&object_cube, xTheta, yTheta, zTheta);
	GameObject_render(pd, frame, &object_cube, &camera_default);

	pd->system->drawFPS(LCD_COLUMNS - 20, LCD_ROWS - 15);
	pd->system->resetElapsedTime();

	frame_count++;

	return 1;
}

