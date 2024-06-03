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
#include "../ddd/symbols.h"
#include "../ddd/gradient.h"
#include "../ddd/bluenoise.h"

static int update(void* userdata);
const char* fontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
const char* bluenoisepath = "Assets/bluenoise.png";
LCDFont* font = NULL;
LCDBitmap* bluenoise;
uint8_t* bluenoise_data = NULL;
int blue_w=0, blue_h=0, blue_rb=0, blue_m=0;

LCDBitmap* fish = NULL;
uint8_t* fish_data = NULL;
int fish_w, fish_h;


LCDBitmap* coop = NULL;
uint8_t* coop_data = NULL;
int coop_w, coop_h, coop_rb;

#ifdef _WINDLL
__declspec(dllexport)
#endif

struct Vector3 p1 = { .x = 50, .y = 50,.z = 0 };
struct Vector3 p2 = { .x = 100, .y = 50,.z = 100 };
struct Vector3 p3 = { .x = 100, .y = 100, .z = 50 };
struct Vector3 p4 = { .x = 50, .y = 100,.z = 100 };
#define CUBE_SIZE 10
struct Mesh mesh_cube = {
	.name = "Cube",
	.numVertices = 8,
	.numIndices = 12 * 3, // 12 faces
	.position = {0,0,15},
	.origin = { CUBE_SIZE / 2,CUBE_SIZE / 2,CUBE_SIZE / 2 },
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
		0, 2, 3, // south2
		3, 2, 5, // east
		3, 5, 4, // east2
		4, 5, 6, // north
		4, 6, 7, // north2
		7, 6, 1, // west
		7, 1, 0, // west2
		1, 6, 5, // top
		1, 5, 2, // top2
		4, 7, 0, // bottom
		4, 0, 3, // bottom2
	}
};

struct Camera camera_default = {
	.near = 0.1f,
	.far = 1000.0f,
	.fov = 60.0f,
	.look_target = {0,0,0},
	.position = {0,0,0},
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

		camera_default.projection = Camera_getProjectionMatrix(&camera_default);
		mesh_cube.tRotationX = (struct Matrix4x4){ 0 };
		mesh_cube.tRotationY = (struct Matrix4x4){ 0 };
		mesh_cube.tRotationZ = (struct Matrix4x4){ 0 };
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
	mesh_cube.position.z += crankDelta * moveSpeed;

	if (off < 0) dir = 1;
	if (off > 256) dir = -1;
	off += dir;
}

void jigglePoints()
{
	int rspeed = 2;
	p1.x += u_random * rspeed;
	p1.y += u_random * rspeed;
	p2.x += u_random * rspeed;
	p2.y += u_random * rspeed;
	p3.x += u_random * rspeed;
	p3.y += u_random * rspeed;
	p4.x += u_random * rspeed;
	p4.y += u_random * rspeed;
}

static int update(void* userdata)
{
	PlaydateAPI* pd = userdata;
	char frame_text[64];
	float deltaTime = pd->system->getElapsedTime();
	
	pd->graphics->clear(kColorWhite);
	pd->graphics->setFont(font);

	frame = pd->graphics->getFrame();

	// Rotation Z
	mesh_cube.tRotationZ.m[0][0] = cosf(zTheta);
	mesh_cube.tRotationZ.m[0][1] = sinf(zTheta);
	mesh_cube.tRotationZ.m[1][0] = -sinf(zTheta);
	mesh_cube.tRotationZ.m[1][1] = cosf(zTheta);
	mesh_cube.tRotationZ.m[2][2] = 1;

	// Rotation Y
	mesh_cube.tRotationY.m[0][0] = cosf(yTheta);
	mesh_cube.tRotationY.m[0][2] = sin(yTheta);
	mesh_cube.tRotationY.m[1][1] = 1;
	mesh_cube.tRotationY.m[2][0] = -sin(yTheta);
	mesh_cube.tRotationY.m[2][2] = cos(yTheta);


	// Rotation X
	mesh_cube.tRotationX.m[0][0] = 1;
	mesh_cube.tRotationX.m[1][1] = cosf(xTheta * 0.5f);
	mesh_cube.tRotationX.m[1][2] = sinf(xTheta * 0.5f);
	mesh_cube.tRotationX.m[2][1] = -sinf(xTheta * 0.5f);
	mesh_cube.tRotationX.m[2][2] = cosf(xTheta * 0.5f);

	handleButtons(pd);
	jigglePoints();

	Mesh_draw(pd, frame, &mesh_cube, NULL);
	Mesh_draw(pd, frame, &mesh_cube, &camera_default);

	pd->system->drawFPS(LCD_COLUMNS - 20, LCD_ROWS - 15);
	pd->system->resetElapsedTime();

	frame_count++;

	return 1;
}

