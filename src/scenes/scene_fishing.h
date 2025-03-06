#include "../../engine/ddd.h";
#include "../../engine/gradient.h";
#include "../models/model_cube.h";
#include "../models/model_blahaj_tri.h";
#include "../models/model_terrain1.h";
#include "../models/model_plane.h";
#include "../models/model_grid_512_long.h";
#include "../../engine/symbols.h";
#include "../../engine/fsm.h";
//#include "../../engine/pd_stub.h";
#include "../data/lures.h";

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

struct Lure* ActiveLure = &DefaultLure;

SamplePlayer* reel_click_player;
SamplePlayer* reel_long_player;

void fish_update();
void lure_update();
void fish_vertShader(struct Actor* act, int time, struct Vector3* v_out);
void fsm_fishing_init();
void fsm_fishing_update();
void underwater_camera_update();


/*
	Assets
*/

AudioSample* audio_reel_click;
AudioSample* audio_reel_long;

/*
	State Machines
*/
struct FSM fsm_fishing = {
	.init = &fsm_fishing_init,
	.update = &fsm_fishing_update
};

#define FSM_LOADING 0
#define FSM_CASTING 1
#define FSM_REELING 2
#define FSM_FISH_ON 3
#define FSM_RESULTS 4

/*
	Actors
*/

 struct Actor object_lure = {
 	.name = "Lure",
	.visible = true,
 	.mesh = &mesh_cube,
 	.position = { 0, .5f, 0 },
 	.rotation = { 0, 0, 0 },
 	.scale = { .05f, .05f, .05f },
	.update = &lure_update,
	.scene = &FishingScene,
	.use_fog = true,
 };

 struct Actor object_fish = {
	.name = "Fish",
	.visible = true,
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
	.use_fog = true,
};

struct Actor object_terrain = {
	.name = "Terrain",
	.visible = true,
	.mesh = &Grid512Long, //&terrain1, //  &flat_plane,
	.position = { 0, 0, 0 },
	.rotation = { 0, 0, 0 },
	.scale = {15,1,15},// { 30, 1, 50 },
	.scene = &FishingScene,
	.use_fog = true,
	.skip_black_triangles = true,
};

struct Actor object_cast_selector = {
	.name = "Cast Selector",
	.visible = true,
	.mesh = &mesh_cube,
	.position = { 0, .5f, 0 },
	.rotation = { 0, 0, 0 },
	.scale = { .1f, .01f, .1f },
	//.update = &lure_update,
	.scene = &FishingScene,
};

/*
	Cameras
*/

struct Actor camera_default_object = {
	.name = "Default Camera",
	.visible = true,
	.position = { 0, 2.5f, 5.0f },
	.rotation = { 0, 0, 0},
	.scale = { 1, 1, 1 },
	.look_target = {
		.tween_speed = 1.f,
	},
	.update = &underwater_camera_update,
};

struct Actor camera_secondary_object = {
	.name = "Secondary Camera",
	.visible = true,
	.position = { -4.0f, 4.f, 2.0f },
	.rotation = { 0, 3.14f, 0},
	.scale = { 1, 1, 1 },
	.look_target = {
		.tween_speed = 1.f,
	}
};

struct Actor camera_lake_object = {
	.name = "Lake Camera",
	.visible = true,
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

#define WATER_SURFACE_Y 10
bool fsm_fishing_cast_to_reeling() {
	pd->system->logToConsole("CAST -> REELING");
	
	object_lure.position.x = MAP_RANGE(-5, 5, -10, 10, object_cast_selector.position.x);
	object_lure.position.z = MAP_RANGE(-5, 5, -20, 20, object_cast_selector.position.z);
	object_lure.position.y = WATER_SURFACE_Y;


	LakeView.Visible = false;
	UnderwaterView.Visible = true;

	camera_default_object.position.z = object_lure.position.z + 5;
	camera_default_object.position.y = object_lure.position.y - 3;
	camera_default_object.position.x = object_lure.position.x;

	return true;
}

bool fsm_fishing_reeling_to_cast() {
	pd->system->logToConsole("CAUGHT NOTHING");

	object_cast_selector.position.x = 0;
	object_cast_selector.position.y = 0;
	object_cast_selector.position.z = 0;

	object_cast_selector.scale.x = .1f;
	object_cast_selector.scale.z = .1f;

	LakeView.Visible = true;
	UnderwaterView.Visible = false;

	return true;
}

void fsm_fishing_init()
{
	fsm_fishing.on_change[FSM_CASTING][FSM_REELING] = &fsm_fishing_cast_to_reeling;
	fsm_fishing.on_change[FSM_REELING][FSM_CASTING] = &fsm_fishing_reeling_to_cast;
}

float ticker_x = 420;
const float TICKER_SPEED = 90;
char* ticker_queue[32];
int ticker_queue_length;
float next_clear_width = 0;
char* ticker_buffer = "            ";
int ticker_filler_index = 0;
LCDFont* TICKER_FONT;
char* ticker_filler_table[] = {
	//"Trying to sneak into the fish heaven? You'll need to complete all the lake rituals!",
	"Here's my fish impression: glub glub glub glub glub glub glub glub glub glub",
	"If you hold your breath for too long, you'll die.",
	"Some fish have evolved the ability to have feelings about sports.",
	//"Having trouble attracting a fish? Try changing the lure! There's someone out there for everybody, according to my ex. Monica, I still love you.",
	"If you're a fish cop, you have to tell me, or it's entrapment.",
	"The Fish Vatican is located somewhere in Italy, but I've never been there.",
	//"Today's winning lottery numbers are: 2 94 84 2 65 8 19 297 4 3 27 3 7273 42 72 27",
	//"Da early fish gets da worm.",
	"Don't throw empty beer cans into the lake. The fish are trying to cut back.",
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
	if (idx >= 6) idx = 0;
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

// BUG: Repeats same "random" ticker item
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
			itoa(abs(i), s_text, 10);
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
	char s_yards[5] = "000";
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

#define TENSION_METER_HEIGHT 170
#define TENSION_METER_WIDTH 24
#define TENSION_OFFSET_BUFFER 8
float tension_offset_x = TENSION_METER_WIDTH + TENSION_OFFSET_BUFFER;
bool tension_meter_visible = false;

float line_integrity = 1.f;
float line_tension = 0.f;
void ui_tension()
{
	// Slide in / out as visibility changes
	if (tension_meter_visible)
	{
		// TODO: Use curve ramp
		if (tension_offset_x > 0)
			tension_offset_x -= DELTA_TIME * 60;
		else tension_offset_x = 0;
	}
	else
	{
		if (tension_offset_x < TENSION_METER_WIDTH + TENSION_OFFSET_BUFFER)
			tension_offset_x += DELTA_TIME * 60;
		else tension_offset_x = TENSION_METER_WIDTH + TENSION_OFFSET_BUFFER;
	}

	float h = 0;
	float a = .3; // First threshold % value
	float b = .9; // Second threshold % value
	float aY = .5f; // First threshold % location
	float bY = .8f; // Second threshold % location
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
		400 - TENSION_METER_WIDTH - buffer - border * 2 + tension_offset_x,
		buffer + border,
		TENSION_METER_WIDTH + border * 2,
		TENSION_METER_HEIGHT + border * 2,
		kColorBlack
	);
	pd->graphics->drawRect(
		400 - TENSION_METER_WIDTH - buffer - border + tension_offset_x,
		buffer + border * 2,
		TENSION_METER_WIDTH,
		TENSION_METER_HEIGHT,
		kColorWhite
	);

	pd->graphics->drawLine(
		400 - TENSION_METER_WIDTH - buffer - border + tension_offset_x,
		(TENSION_METER_HEIGHT + buffer + border) - TENSION_METER_HEIGHT * aY,
		400 - buffer - border - 1 + tension_offset_x,
		(TENSION_METER_HEIGHT + buffer + border) - TENSION_METER_HEIGHT * aY,
		1,
		kColorWhite
	);
	pd->graphics->drawLine(
		400 - TENSION_METER_WIDTH - buffer - border + tension_offset_x,
		(TENSION_METER_HEIGHT + buffer + border) - TENSION_METER_HEIGHT * bY,
		400 - buffer - border - 1 + tension_offset_x,
		(TENSION_METER_HEIGHT + buffer + border) - TENSION_METER_HEIGHT * bY,
		1,
		kColorWhite
	);

	float adjMeterWidth = line_integrity * TENSION_METER_WIDTH;
	if ((int)adjMeterWidth % 2 != 0) adjMeterWidth = (int)adjMeterWidth + 1;

	pd->graphics->fillRect(
		(400.f - TENSION_METER_WIDTH - buffer - border) + ((float)TENSION_METER_WIDTH - adjMeterWidth + 1) / 2.f + tension_offset_x,
		(buffer + border + TENSION_METER_HEIGHT + 2) - TENSION_METER_HEIGHT * h,
		adjMeterWidth,
		h * TENSION_METER_HEIGHT,
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

	//object_lure.position.y -= DELTA_TIME;
	//if (object_lure.position.y < 0) object_lure.position.y = 0;


	// TODO: use to play crank clicks
	int crank_ticks = playdate_getCrankTicks(10);
	float crank_change = pd->system->getCrankChange();
	if (crank_ticks != 0) // We don't care about direction currently
	{
		pd->system->logToConsole("change %d", crank_change);
		pd->system->logToConsole("ticks %d", crank_ticks);
		//pd->sound->sampleplayer->setVolume(reel_click_player);
		//if (crank_ticks <- 1)
		//{
		//	if (!pd->sound->sampleplayer->isPlaying(reel_long_player))
		//		pd->sound->sampleplayer->play(reel_long_player, 1, 1.f);
		//}
		//else {
			//pd->sound->sampleplayer->stop(reel_long_player);

			float volume = (fabsf(crank_ticks) / 10) * .75f + .25f;
			pd->sound->sampleplayer->setVolume(reel_click_player, volume, volume);
			pd->sound->sampleplayer->play(reel_click_player, 1, 1.f);
		//}
	}



	if (fsm_fishing.current_state == FSM_REELING ||
		fsm_fishing.current_state == FSM_FISH_ON)
	{
		if (ActiveLure->update != 0x0) {
			ActiveLure->update(ActiveLure, &object_lure);
		}

		float change = pd->system->getCrankChange();
		if (UnderwaterView.Visible && change != 0) // We don't care about direction currently
		{
			//object_lure.position.z -= change * .002f;
			//object_lure.position.y -= change * .001f;

			if (ActiveLure->move != 0x0) {
				ActiveLure->move(ActiveLure, &object_lure, fabsf(crank_change));
			}
		}

		if (yards <= 5) // TODO: Define "return to shore" threshold
		{
			yards = 1000;
			if (fsm_fishing.current_state == FSM_REELING)
			{
				// Return to shore without fish
				fsm_set_state(&fsm_fishing, FSM_CASTING);
			}
			else if (fsm_fishing.current_state == FSM_FISH_ON)
			{
				// Caught a fish! Show results.
				fsm_set_state(&fsm_fishing, FSM_RESULTS);
			}
		}
	}

}

struct Vector3 target_underwater_camera_position = { 0 };
void underwater_camera_update()
{
	target_underwater_camera_position.z = object_lure.position.z + 3;
	target_underwater_camera_position.y = MAX(object_lure.position.y + 3, 0);
	target_underwater_camera_position.x = object_lure.position.x;

	/*if (camera_default_object.position.z > object_lure.position.z + 1)
		camera_default_object.position.z = object_lure.position.z + 1;*/

	struct Vector3 diff = Vector3_subtract(&camera_default_object.position, &target_underwater_camera_position);
	if (Vector3_lengthSquared(&diff) > .1f)
	{
		diff = Vector3_multiplyScalar(&diff, DELTA_TIME *2);
	
		camera_default_object.position = Vector3_subtract(&camera_default_object.position, &diff);
	}

	// TODO: Add camera controls (but not too extreme so we can use our clipping hack)
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
	LookTarget_setTarget(&camera_lake.actor->look_target, &object_cast_selector.position);

	ticker_add_from(ticker_filler_table);
	TICKER_FONT = FONT_MONTSERRAT_BOLD_14;

	fsm_fishing.init();
	fsm_set_state(&fsm_fishing, FSM_LOADING);

	reel_click_player = pd->sound->sampleplayer->newPlayer();
	reel_long_player = pd->sound->sampleplayer->newPlayer();

	// Load Assets
	// TODO: Loading screen
	audio_reel_click = pd->sound->sample->load("Assets/Sounds/reel_click_single.wav");
	audio_reel_long = pd->sound->sample->load("Assets/Sounds/reel_click_long.wav");
	//float length = pd->sound->sample->getLength(audio_reel_click);
	pd->sound->sampleplayer->setSample(reel_click_player, audio_reel_click);
	pd->sound->sampleplayer->setSample(reel_long_player, audio_reel_long);

	float r = 0;
	for (int i = 0; i < Grid512Long.numVertices; i++)
	{
		r = 0;
		if (fabsf(Grid512Long.vertices[i].x) >= .25f)
		{
			r += ((float)rand() / (float)RAND_MAX) * fabsf(Grid512Long.vertices[i].x) * 1.f;
			r += (fabsf(Grid512Long.vertices[i].x * Grid512Long.vertices[i].x) / 1.f) * 2.f;
			//r += ((float)rand() / (float)RAND_MAX) * 1.5f;
		}
		else
		{
			r += ((float)rand() / (float)RAND_MAX) * .05f;
		}
		Grid512Long.vertices[i].y = r;
	}
}


float lure_fitness = 0;
struct CrankPeriod crank_buffer[TIMING_BUFFER_LENGTH];
void shift_crank_buffer(float duration, float distance)
{
	float deg_per_second = (duration > 0 ? distance / duration / 360 : 0);
	pd->system->logToConsole("duration: %f, distance: %f, speed: %f", duration, distance, deg_per_second);

	// Find last empty space if the buffer isn't full
	int last_value_index = -1;
	for (int i = 0; i < TIMING_BUFFER_LENGTH - 1; i++)
	{
		if (crank_buffer[i].duration > 0)
		{
			last_value_index = i;
		}
	}

	// Shift everything to the left
	if (last_value_index == TIMING_BUFFER_LENGTH - 2)
	{
		for (int i = 0; i < TIMING_BUFFER_LENGTH - 1; i++)
		{
			crank_buffer[i] = crank_buffer[i + 1];
		}
	}

	// Insert new value in first empty spot
	crank_buffer[last_value_index + 1].duration = duration;
	crank_buffer[last_value_index + 1].distance = distance;
	
	if (ActiveLure->timing_pattern != NULL && ActiveLure->fit_score_pattern != NULL)
	{
		lure_fitness = ActiveLure->fit_score_pattern(crank_buffer, ActiveLure->timing_pattern);
		pd->system->logToConsole("fitness: %f", lure_fitness);
	}
}

float pullMod = 1;
float test_timer = 0;
int lake_length = 100;

float crank_duration = 0;
float idle_duration = 0;
float crank_distance = 0;
#define IDLE_DEADZONE .2f
bool is_cranking = false;

void fishing_scene_update()
{
	ticker_x -= DELTA_TIME * TICKER_SPEED;

	float change = pd->system->getCrankChange();

	if (change != 0) {
		if (!is_cranking) // First frame cranking, previously idle
		{
			// Add idle_duration to buffer
			shift_crank_buffer(idle_duration, 0);

			crank_duration = 0;
			crank_distance = 0;
		}
		// Direction changed without idle period?
		else if ( (change < 0) != (crank_distance < 0) ) { // sign diff check
			pd->system->logToConsole("Change direction %f -> %f", change, crank_distance);
			// Add crank_duration to buffer
			shift_crank_buffer(crank_duration, crank_distance);

			idle_duration = 0;
			crank_duration = 0;
		}

		idle_duration = 0;
		is_cranking = true;
		crank_duration += DELTA_TIME;
		crank_distance += change;
	}
	else 
	{
		if (idle_duration > IDLE_DEADZONE) {
			if (is_cranking) // First frame idle
			{
				// Add crank_duration to buffer
				shift_crank_buffer(crank_duration, crank_distance);

				idle_duration = 0;
				crank_duration = 0;
			}

			is_cranking = false;
		}

		idle_duration += DELTA_TIME;
	}

	if (fsm_fishing.current_state == FSM_REELING || fsm_fishing.current_state == FSM_FISH_ON)
	{
		depth = object_lure.position.y - WATER_SURFACE_Y;
		yards = MAP_RANGE(-30, 30, 100, 0, object_lure.position.z);
	}

	if (line_tension > .98f)
	{
		line_integrity -= .025f;

		if (line_integrity < .1f)
		{
			line_integrity = .1f;
		}
	}

	fsm_fishing.update();
}


void fsm_fishing_update()
{
	switch (fsm_fishing.current_state)
	{
	case FSM_LOADING:
		// Do initial scene stuff

		fsm_set_state(&fsm_fishing, FSM_CASTING);
		break;

	case FSM_CASTING:
		//pd->system->logToConsole("FSM_CASTING");
		break;
	}
}

#define CAST_SELECTOR_SPEED 5
#define CAST_ANIMATION_TIME 1
float cast_animation_timer = 0;
void lake_update()
{
	// TODO: Add bounds
	if (heldButtons & kButtonLeft) {
		object_cast_selector.position.x += DELTA_TIME * CAST_SELECTOR_SPEED;
	}
	else if (heldButtons & kButtonRight) {
		object_cast_selector.position.x -= DELTA_TIME * CAST_SELECTOR_SPEED;
	}

	if (heldButtons & kButtonUp) {
		object_cast_selector.position.z -= DELTA_TIME * CAST_SELECTOR_SPEED;
	}
	else if (heldButtons & kButtonDown) {
		object_cast_selector.position.z += DELTA_TIME * CAST_SELECTOR_SPEED;
	}

	if (object_cast_selector.position.x > 5) object_cast_selector.position.x = 5;
	if (object_cast_selector.position.x < -5) object_cast_selector.position.x = -5;

	if (object_cast_selector.position.z > 5) object_cast_selector.position.z = 5;
	if (object_cast_selector.position.z < -5) object_cast_selector.position.z = -5;

	if (fsm_fishing.current_state == FSM_CASTING)
	{
		// Cast selection
		if (pressedButtons & kButtonA && cast_animation_timer == 0)
		{
			// Trigger cast animation playing out
			cast_animation_timer = CAST_ANIMATION_TIME;
		}

		// When that's done, go to FSM_REELING
		if (cast_animation_timer > 0) 
		{
			cast_animation_timer -= DELTA_TIME;

			object_cast_selector.scale.x =
				object_cast_selector.scale.z -= DELTA_TIME * .1f / CAST_ANIMATION_TIME;

			// If we _were_ above 0 and now we're not
			if (cast_animation_timer <= 0)
			{
				cast_animation_timer = 0;

				fsm_set_state(&fsm_fishing, FSM_REELING);
			}
		}
	}
}

void lake_draw()
{
	YPlane_render(frame, &camera_lake, 0, false);
}

void lake_postdraw()
{
	// UI Rendering
	if (fsm_fishing.current_state == FSM_FISH_ON)
	{
		ui_yards();
		ui_tension();
	}
	ui_news_ticker();
}


void underwater_update()
{
}

void underwater_draw()
{
}

void underwater_postdraw()
{
	YPlane_render(frame, &camera_default, WATER_SURFACE_Y, false);

	// UI Rendering
	ui_depth(); // render before yards / ticker
	ui_yards();
	ui_news_ticker();
	ui_fish_name();
	//ui_radar();
	ui_tension();
}

/* 
	Scene Object
*/

struct View UnderwaterView = {
	.Enabled = true,
	.Visible = false,
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
	.update = &underwater_update,
	.draw = &underwater_draw,
	.postdraw = &underwater_postdraw,
};

struct View LakeView = {
	.Enabled = true,
	.Visible = true,
	.numCameras = 1,
	.cameras = (struct Camera* []){
		&camera_lake,
		NULL
	},
	.actors = (struct Actor* []){
		&object_cast_selector,
		NULL
	},
	.update = &lake_update,
	.draw = &lake_draw,
	.postdraw = &lake_postdraw,
};

struct Scene FishingScene = {
	.init = &fishing_scene_init,
	.update = &fishing_scene_update,
	.views = (struct View* []) {
		&LakeView,
		&UnderwaterView,
		NULL
	},
};