#include "../../engine/ddd.h"
#include "../../engine/gradient.h"
#include "../models/model_cube.h"
#include "../models/model_blahaj_tri.h"
#include "../models/model_terrain1.h"
#include "../../engine/symbols.h";

/*
Externs
*/

extern PlaydateAPI* pd;
extern float DELTA_TIME;
extern uint8_t* frame;
extern PDButtons heldButtons, pressedButtons, releasedButtons;
extern LCDFont* FONT_MONTSERRAT_BLACK_ITALIC_24;
extern LCDFont* FONT_MONTSERRAT_BLACK_24;
extern LCDFont* FONT_MONTSERRAT_BOLD_14;

/*
	Definitions
*/
struct Scene FishingScene;
struct View UnderwaterView;
struct View LakeView;

void fish_update();
void lure_update();
void fish_vertShader(struct Actor* act, int time, struct Vector3* v_out);

/*
	Game Objects
*/
 struct Actor object_lure = {
 	.name = "Lure",
 	.mesh = &mesh_cube,
 	.position = { 0, .5f, 0 },
 	.rotation = { 0, 0, 0 },
 	.scale = { .05f, .05f, .05f },
	.update = &lure_update,
	.scene = &FishingScene,
 };

 struct Actor object_fish = {
	 .name = "Fish",
	 .mesh = &blahaj_tri,
	 .position = { 3, 2, -3 },
	 .rotation = { 0, 0, 0 },
	 .scale = { 1, 1, 1 },
	 .look_target = {
		 .tween_speed = 1.f,
	 },
	 .update = &fish_update,
	 .vertShader = &fish_vertShader,
	 .scene = &FishingScene,
};

struct Actor object_terrain = {
	.name = "Terrain",
	.mesh = &terrain1,
	.position = { 0, -1.5f, 0 },
	.rotation = { 0, 0, 0 },
	.scale = { 1, 1, 1 },
	.scene = &FishingScene,
};

/*
	Cameras
*/

struct Actor camera_default_object = {
	.name = "Default Camera",
	.position = { 0, 2.5f, 5.0f },
	.rotation = { 0, 0, 0},
	.scale = { 1, 1, 1 },
	.look_target = {
		.tween_speed = 1.f,
	}
};

struct Actor camera_secondary_object = {
	.name = "Secondary Camera",
	.position = { -4.0f, 4.f, 2.0f },
	.rotation = { 0, 3.14f, 0},
	.scale = { 1, 1, 1 },
	.look_target = {
		.tween_speed = 1.f,
	}
};

struct Actor camera_lake_object = {
	.name = "Lake Camera",
	.position = { 0.f, 2.f, 5.0f },
	.rotation = { 0, 0, 0},
	.scale = { 1, 1, 1 },
	.look_target = {
		.tween_speed = 1.f,
	}
};

struct Camera camera_default = {
	.actor = &camera_default_object,
	.near = 0.0f,
	.far = 10.0f,
	.fov = 60.0f,
	.far_fog = 10.0f,
	.near_fog = 1,
	//.look_target = &GLOBAL_ORIGIN,
	//.position = { 0, 2.5, 5.0f },
	//.rotation = {0,0,0},
	.render_mode = RENDER_WIREFRAME,
	.light_dir = { .5f,.5f,.5f },
};


struct Camera camera_secondary = {
	.actor = &camera_secondary_object,
	.near = 0.0f,
	.far = 10.0f,
	.fov = 60.0f,
	.far_fog = 10.0f,
	.near_fog = 1,
	//.look_target = &GLOBAL_ORIGIN,
	//.position = { 0, 2.5, 5.0f },
	//.rotation = {0,0,0},
	.render_mode = RENDER_WIREFRAME,
	.light_dir = { .5f,.5f,.5f },
};


struct Camera camera_lake = {
	.actor = &camera_lake_object,
	.near = 0.0f,
	.far = 10.0f,
	.fov = 60.0f,
	.far_fog = 10.0f,
	.near_fog = 1,
	.light_dir = { .5f,.5f,.5f },
};

/*
	Methods
*/

float ticker_x = 420;
const float TICKER_SPEED = 90;
char* ticker_queue[32];
int ticker_queue_length;
float next_clear_width = 0;
char* ticker_buffer = "            ";
int ticker_filler_index = 0;
LCDFont* TICKER_FONT;
char* ticker_filler_table[] = {
	////"Trying to sneak into the fish heaven? You'll need to complete all the lake rituals!",
	//"Here's my fish impression: glub glub glub glub glub glub glub glub glub glub",
	//"If you hold your breath for too long, you'll die.",
	///"Some fish have evolved the ability to have feelings about sports.",
	////"Having trouble attracting a fish? Try changing the lure! There's someone out there for everybody, according to my ex. Monica, I still love you.",
	//"If you're a fish cop, you have to tell me, or it's entrapment.",
	//"The Fish Vatican is located somewhere in Italy, but I've never been there.",
	//"Today's winning lottery numbers are: 2 94 84 2 65 8 19 297 4 3 27 3 7273 42 72 27",
	////"Da early fish gets da worm.",
	//"Don't throw empty beer cans into the lake. The fish are trying to cut back.",
	"Religious oppression has been reported in Florida. World Government has been dispatched to assist.",
	"Have you read the New New New International Version translation of scripture?",
	"The rivers have turned to blood throughout Europe. Fishing is on hold under further notice.",
	"Crowds chanting 'there is no God' have destroyed priceless monuments. Sadly, this is common.",
	NULL
};
char ticker_text[1024];

void ticker_add(char* item)
{
	if (ticker_queue_length > 31) return;
	ticker_queue[ticker_queue_length++] = item;

	strcat(ticker_text, ticker_queue[ticker_queue_length - 1]);
	strcat(ticker_text, ticker_buffer);

	if (next_clear_width == 0)
	{
		next_clear_width = pd->graphics->getTextWidth(
			TICKER_FONT,
			ticker_queue[0],
			strlen(ticker_queue[0]),
			kASCIIEncoding,
			0
		) +
			pd->graphics->getTextWidth(
				TICKER_FONT,
				ticker_buffer,
				strlen(ticker_buffer),
				kASCIIEncoding,
				0
			);
	}
}

void ticker_add_from(char** table)
{
	int length = 0;
	for(;;length++) if (table[length + 1] == NULL) break;

	// TODO: Ensure that we don't get duplicates until we've shown them all.
	//int idx = ((float)rand() / (float)RAND_MAX) * (length - 1);
	int idx = ticker_filler_index++;
	if (idx >= 4) idx = 0;
	ticker_add(table[idx]);
}

void ticker_clear_first()
{
	char* first = ticker_queue[0];
	
	ticker_x += next_clear_width - 4.67*2; // 1/3 of font size for both text, buffer

	for (int i = 0; i < ticker_queue_length - 1; i++)
	{
		ticker_queue[i] = ticker_queue[i + 1];
	}

	next_clear_width = pd->graphics->getTextWidth(
		TICKER_FONT,
		ticker_queue[0],
		strlen(ticker_queue[0]),
		kASCIIEncoding,
		0
	) +
		pd->graphics->getTextWidth(
			TICKER_FONT,
			ticker_buffer,
			strlen(ticker_buffer),
			kASCIIEncoding,
			0
		);

	ticker_queue_length--;

	//ticker_text = pd->system->realloc(ticker_text, sizeof(char) * 1024);
	strcpy(ticker_text, ticker_buffer);
	for (int i = 0; i < ticker_queue_length; i++)
	{
		strcat(ticker_text, ticker_queue[ticker_queue_length - 1]);
		strcat(ticker_text, ticker_buffer);
	}

	if (ticker_queue_length == 0)
	{
		ticker_x = 0;
		ticker_add_from(ticker_filler_table);
	}
}

void ui_news_ticker()
{
	static const TICKER_HEIGHT = 18;
	pd->graphics->fillRect(0, 240 - TICKER_HEIGHT, 400, TICKER_HEIGHT, kColorBlack);

	if (ticker_x <= -next_clear_width)
	{
		ticker_clear_first();
	}

	if (ticker_x < 400 - next_clear_width && ticker_queue_length == 1)
	{
		ticker_add_from(ticker_filler_table);
	}

	pd->graphics->setFont(TICKER_FONT);
	pd->graphics->setDrawMode(kDrawModeFillWhite);
	pd->graphics->drawText(ticker_text, strlen(ticker_text), kASCIIEncoding, ticker_x, 240 - TICKER_HEIGHT + 2);
}

void ui_fish_name()
{
	char* name = "HATEFUL SALMON";
	int s_width = pd->graphics->getTextWidth(FONT_MONTSERRAT_BOLD_14, name, strlen(name), kASCIIEncoding, 0);
	static int padding = 4;

	pd->graphics->setFont(FONT_MONTSERRAT_BOLD_14);
	pd->graphics->fillRect(400 - s_width - padding*2, 240 - 50, s_width + padding * 2, 16, kColorBlack);
	pd->graphics->setDrawMode(kDrawModeFillWhite);
	pd->graphics->drawText(name, strlen(name), kASCIIEncoding, 400 - s_width - padding, 240 - 50);
}

float depth = 150;
#define STANDARD_DEPTH_WIDTH 25
float depth_width = 34;//STANDARD_DEPTH_WIDTH;
void ui_depth()
{
	float zoom = 15; //max(fabsf(depth), 2.f);
	float DEPTH_TICK = (240.f) / zoom;
	float y = 0;
	float offset = depth * DEPTH_TICK;
	int mod_by = 1;
	if (zoom < 30) mod_by = 5;
	if (zoom < 4) mod_by = 10;
	if (zoom < 2) mod_by = 25;

	pd->graphics->setFont(FONT_MONTSERRAT_BOLD_14);
	pd->graphics->setDrawMode(kDrawModeFillWhite);
	pd->graphics->fillRect(0, 0, depth_width, 240, kColorBlack);

	float minValue = depth - DEPTH_TICK / 2;
	float maxValue = DEPTH_TICK / 2 + zoom;

	float d2 = 32 + (minValue < 0 || maxValue < 0 ? 6 : 0);
	float d3 = 34 + (minValue < 0 || maxValue < 0 ? 6 : 0);

	if (abs(minValue) > 9 || abs(maxValue) > 9) {
		if (abs(minValue) > 99 || abs(maxValue) > 99) {
			if (depth_width < d3) depth_width += DELTA_TIME * 4;
			if (depth_width > d3) depth_width -= DELTA_TIME * 4;
		}
		else {
			if (depth_width < d2) depth_width += DELTA_TIME * 4;
			if (depth_width > d2) depth_width -= DELTA_TIME * 4;
		}
	}
	else
	{
		if (depth_width > STANDARD_DEPTH_WIDTH) depth_width -= DELTA_TIME;
		if (depth_width > STANDARD_DEPTH_WIDTH) depth_width = STANDARD_DEPTH_WIDTH;
	}



	char s_text[8];
	int line_width = 10;
	for (int i = depth - DEPTH_TICK / 2; i < DEPTH_TICK / 2 + depth; i++)
	{
		y = (240 - (240 / 2)) - (i * zoom) + (depth * zoom);

		//if (y < 0 || y > 240) continue;
		
		line_width = 10;
		if (abs(i) > 9) line_width = 8;
		if (abs(i) > 99) line_width = 4;

		pd->graphics->drawLine(0, y, line_width, y, 1, kColorWhite);

		if (i % mod_by == 0)
		{
			itoa(i, s_text, 10);
			pd->graphics->drawText(s_text, strlen(s_text), kASCIIEncoding, line_width + 2, y - 7);
		}
	}
	pd->graphics->drawLine(0, 240 / 2, 15, 240 / 2, 3, kColorWhite);

	//itoa(depth, s_text, 10);
	//pd->graphics->drawText(s_text, strlen(s_text), kASCIIEncoding, 28, 240 / 2 - 7);
}

float yards = 40;
void ui_yards()
{
	char s_yards[4] = "000";
	itoa(yards, s_yards, 10);

	float s_width = pd->graphics->getTextWidth(FONT_MONTSERRAT_BLACK_24, s_yards, strlen(s_yards), kASCIIEncoding, 0);
	
	pd->graphics->setDrawMode(kDrawModeFillWhite);

	pd->graphics->fillRect(0, 0, 80, 30, kColorBlack);
	pd->graphics->drawLine(0, 0, 80, 0, 1, kColorWhite);
	pd->graphics->drawLine(80, 0, 80, 30, 1, kColorWhite);
	pd->graphics->drawLine(80, 30, 0, 30, 1, kColorWhite);
	pd->graphics->drawLine(0, 30, 0, 0, 1, kColorWhite);
	pd->graphics->setFont(FONT_MONTSERRAT_BLACK_24);
	pd->graphics->drawText(s_yards, strlen(s_yards), kASCIIEncoding, (80 - s_width)/2, 4);

	pd->graphics->setFont(FONT_MONTSERRAT_BOLD_14);
	pd->graphics->drawText("YDS", 3, kASCIIEncoding, 80-30, 32);
}

void ui_radar()
{
	static int radius = 60;
	pd->graphics->fillEllipse(400 - radius, 0, radius, radius, 0, 360, kColorBlack);
	pd->graphics->drawEllipse(400 - radius, 0, radius, radius, 1, 0, 360, kColorWhite);

	// TODO: Draw to radar buffer, update.
}


float line_integrity = 1.f;
float line_tension = 0.f;
void ui_tension()
{
	float h = 0;
	float a = .3; // First threshold % value
	float b = .9; // Second threshold % value
	float aY = .5f; // First threshold % location
	float bY = .8f; // Second threshold % location
	int meterHeight = 170;
	int meterWidth = 24;
	int buffer = 4;
	int border = 2;

	if (line_tension >= a) 
	{
		h = aY;

		if (line_tension >= b)
		{
			h = bY;

			// Add remaining % > b
			h += ((line_tension - b) / (1 - b)) * (1 - bY);
		}
		else { // > a, < b
			h += ((line_tension - a) / (b - a)) * (bY - aY);
		}
	}
	else { // < a
		h = (line_tension / a) * aY;
	}

	pd->graphics->fillRect(
		400 - meterWidth - buffer - border * 2,
		buffer + border,
		meterWidth + border * 2,
		meterHeight + border * 2,
		kColorBlack
	);
	pd->graphics->drawRect(
		400 - meterWidth - buffer - border,
		buffer + border * 2,
		meterWidth,
		meterHeight,
		kColorWhite
	);

	pd->graphics->drawLine(
		400 - meterWidth - buffer - border,
		(meterHeight + buffer + border) - meterHeight * aY,
		400 - buffer - border - 1,
		(meterHeight + buffer + border) - meterHeight * aY,
		1,
		kColorWhite
	);
	pd->graphics->drawLine(
		400 - meterWidth - buffer - border,
		(meterHeight + buffer + border) - meterHeight * bY,
		400 - buffer - border - 1,
		(meterHeight + buffer + border) - meterHeight * bY,
		1,
		kColorWhite
	);

	float adjMeterWidth = line_integrity * meterWidth;
	if ((int)adjMeterWidth % 2 != 0) adjMeterWidth = (int)adjMeterWidth + 1;

	pd->graphics->fillRect(
		(400.f - meterWidth - buffer - border) + ((float)meterWidth - adjMeterWidth + 1) / 2.f,
		(buffer + border + meterHeight + 2) - meterHeight * h,
		adjMeterWidth,
		h * meterHeight,
		kColorWhite
	);
}

void fish_update()
{
	//object_blahaj.rotation.y += DELTA_TIME;
	struct Vector3 move_direction = object_fish.forward;// Vector3_getForward(&object_blahaj.rotation);
	move_direction = Vector3_normalize(move_direction);
	move_direction = Vector3_multiplyScalar(&move_direction, 150.0f * DELTA_TIME);

	struct Vector3 move_forward = Vector3_multiplyScalar(&object_fish.forward, -DELTA_TIME * 1.f);
	//object_blahaj.position = Vector3_add(&object_blahaj.position, &move_forward);

	struct Vector3 trace_target = Vector3_subtract(&object_fish.position, &move_direction);
}

void lure_update()
{
	object_lure.rotation.y += DELTA_TIME;
}

void fish_vertShader (struct Actor* act, int time, struct Vector3* v_out)
{
	float z_extent = act->mesh->max_bounds.z - act->mesh->min_bounds.z;
	float p = (v_out->z - act->mesh->min_bounds.z) / z_extent;
	v_out->x += Gradient_sample(1.0f - p) / 5.0f;
}

void fishing_scene_init()
{
	LookTarget_setTarget(&object_fish.look_target, &object_lure.position);
	LookTarget_setTarget(&camera_default.actor->look_target, &object_lure.position);
	LookTarget_setTarget(&camera_secondary.actor->look_target, &object_lure.position);

	ticker_add_from(ticker_filler_table);
	TICKER_FONT = FONT_MONTSERRAT_BOLD_14;
}

float pullMod = 1;
void fishing_scene_update()
{
	ticker_x -= DELTA_TIME * TICKER_SPEED;
	yards -= DELTA_TIME * 2;
	if (yards < 0) yards = 40;

	depth -= DELTA_TIME * 4.75f;

	line_tension += pullMod * DELTA_TIME * .25f;
	if (line_tension > 1) {
		line_tension = 1;
		pullMod = -1;
	}
	else if (line_tension < 0)
	{
		line_tension = 0;
		pullMod = 1;
	}

	if (line_tension > .98f)
	{
		line_integrity -= .025f;

		if (line_integrity < .1f)
		{
			line_integrity = .1f;
		}
	}


	
	//pull = fabsf(sinf(pd->system->getCurrentTimeMilliseconds() / 500.f));
	//tension = fabsf(sinf(pd->system->getCurrentTimeMilliseconds() / 1234.f));

	if (pressedButtons & kButtonB)
	{
		LakeView.Visible = !LakeView.Visible;
		UnderwaterView.Visible = !LakeView.Visible;
	}
}

void lake_update()
{

}

void lake_draw()
{
	YPlane_render(frame, &camera_lake, 0, false);
}

void lake_postdraw()
{
	// TODO: Add scene postdraw stage for UI that gets shared across views
	// UI Rendering
	//ui_depth(); // render before yards / ticker
	ui_yards();
	ui_news_ticker();
	ui_tension();
}

void underwater_draw()
{

}

void underwater_postdraw()
{
	// TODO: Add scene postdraw stage for UI that gets shared across views
	// UI Rendering
	ui_depth(); // render before yards / ticker
	ui_yards();
	ui_news_ticker();
	ui_fish_name();
	//ui_radar();
	ui_tension();
}

void underwater_update()
{

}


/* 
	Scene Object
*/

struct View UnderwaterView = {
	.Enabled = true,
	.Visible = true,
	.numCameras = 2,
	.cameras = (struct Camera* []){
		&camera_default,
		&camera_secondary,
		NULL
	},
	.actors = (struct Actor* []) {
		&object_terrain,
		&object_fish,
		&object_lure,
		NULL
	},
	.draw = &underwater_draw,
	.postdraw = &underwater_postdraw,
};

struct View LakeView = {
	.Enabled = true,
	.Visible = false,
	.numCameras = 1,
	.cameras = (struct Camera* []){
		&camera_lake,
		NULL
	},
	.actors = (struct Actor* []){
		NULL
	},
	.draw = &lake_draw,
	.postdraw = &lake_postdraw,
};

struct Scene FishingScene = {
	.init = &fishing_scene_init,
	.update = &fishing_scene_update,
	.views = (struct View* []) {
		&UnderwaterView,
		&LakeView,
		NULL
	},
};