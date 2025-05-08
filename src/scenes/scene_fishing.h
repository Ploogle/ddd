#include "../../engine/ddd.h";
#include "../../engine/gradient.h";
#include "../models/model_cube.h";
#include "../models/model_blahaj_tri.h";
#include "../models/model_terrain1.h";
#include "../models/model_plane.h";
#include "../models/model_grid_512_long.h";
#include "../models/model_fractal_terrain.h";
#include "../models/fractal_terrain_lod.h";
#include "../models/lake_terrain1.h";
#include "../models/cast_selector.h";
#include "../models/lure_test.h";
#include "../../engine/symbols.h";
#include "../../engine/fsm.h";
//#include "../../engine/pd_stub.h";
#include "../data/lures.h";
#include "../../engine/wave_heightmap.h";

/*
	Externs
*/

extern PlaydateAPI* pd;
extern double DELTA_TIME;
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

struct Vector3 object_lure_target_position = { 0 };

SamplePlayer* reel_click_player;
SamplePlayer* reel_long_player;

SamplePlayer* soundtrack_intro;
SamplePlayer* soundtrack_lake_loop;
SamplePlayer* soundtrack_underwater_loop;
SamplePlayer* soundtrack_fish_interest_loop;
SamplePlayer* soundtrack_fish_on_loop;

void fish_update();
void lure_update();
void fish_vertShader(struct Actor* act, int time, struct Vector3* v_out);
void fsm_main_init();
void fsm_main_update();
bool fsm_main_update_cast_to_reeling();
bool fsm_main_update_reeling_to_cast();
bool fsm_main_reeling_to_fishon();
void fsm_fish_init();
void fsm_fish_update();
bool fsm_fish_idle_to_swim();
bool fsm_fish_swim_to_idle();
bool fsm_fish_approach_to_bite();
bool fsm_fish_to_approach();
bool fsm_fish_bite_to_hook();
void underwater_camera_update();


/*
	Assets
*/

AudioSample* audio_reel_click;
AudioSample* audio_reel_long;

AudioSample* audio_intro;
AudioSample* audio_lake_loop;
AudioSample* audio_underwater_loop;
AudioSample* audio_fish_interest_loop;
AudioSample* audio_fish_on_loop;

/*
	State Machines
*/

#define FSM_MAIN_LOADING 0
#define FSM_MAIN_CASTING 1
#define FSM_MAIN_REELING 2
#define FSM_MAIN_FISHON 3
#define FSM_MAIN_RESULTS 4
struct FSM fsm_main = {
	.init = &fsm_main_init,
	.update = &fsm_main_update
};

#define FSM_FISH_IDLE 0 // Sitting, doing nothing
#define FSM_FISH_SWIM 1 // Swim to random target
#define FSM_FISH_APPROACH 2 // Noticed, moving towards lure
#define FSM_FISH_BITE 3 // Bite lure, deciding whether to take bait
#define FSM_FISH_HOOKED 4 // Fish On!
#define FSM_FISH_ESCAPE 5 // Line broke, fish got away.
#define FSM_FISH_CAUGHT 6 // Successful catch
struct FSM fsm_fish = {
	.init = &fsm_fish_init,
	.update = &fsm_fish_update
};

/*
	Actors
*/

 struct Actor object_lure = {
 	.name = "Lure",
	.visible = true,
 	.mesh = &lure_test_mesh,
 	.position = { 0, 0, 0 },
	.rotation = {0, 0, 0},
 	.scale = { .2f, .2f, .2f },
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
		.tween_speed = .5f// 1.f,
	},
	.update = &fish_update,
	.vertShader = &fish_vertShader,
	.scene = &FishingScene,
	.use_fog = true,
};

struct Actor object_terrain = {
	.name = "Terrain",
	.visible = true,
	.mesh = &fractal_terrain_lod,
	.position = { 0, 0, 0 },
	.rotation = { 0, 0, 0 },
	.scale = {15, 1, 10},
	.scene = &FishingScene,
	.use_fog = true,
	.skip_black_triangles = true,
};

struct Actor object_cast_selector = {
	.name = "Cast Selector",
	.visible = true,
	.mesh = &cast_selector_mesh,
	.position = { 0, -.25, 0 },
	.rotation = { 0, 0, 0 },
	.scale = { .75f, .5f, .75f },
	.scene = &FishingScene,
};

struct Actor lake_terrain = {
	.name = "Distant Terrain",
	.visible = true,
	.mesh = &lake_terrain_mesh,
	.position = {0, 0, -20},
	.rotation = {0, 1.57f, 0}, // 90deg on y axis
	.scale = {1.f,2.5f,5.f},
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
		.tween_speed = .1f,//1.f,
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


float ticker_x = 420;
const float TICKER_SPEED = 60.0f;
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
	"If you teach a man to fish, he'll eat for a day. If you teach a fish to man, he will attain salvation, amen.",
	"Hint: Shake a fish. They love it!",
	"You'll catch more flies with honey than vinegar. But we're catching fish, baby!",
	"The average human can run faster than most fish.",
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
	if (idx >= length) {
		idx = 0; ticker_filler_index = 0;
	}
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
#define STANDARD_DEPTH_WIDTH 34
float depth_width = STANDARD_DEPTH_WIDTH;
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
//float line_tension = 0.f;
float line_fish_tension = 0.f;
float line_reel_tension = 0.f;
#define LINE_TENSION MIN(line_fish_tension+line_reel_tension, 1)
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

	if (LINE_TENSION >= a)
	{
		h = aY;

		if (LINE_TENSION >= b)
		{
			h = bY;

			// Add remaining % > b
			h += ((LINE_TENSION - b) / (1 - b)) * (1 - bY);
		}
		else { // > a, < b
			h += ((LINE_TENSION - a) / (b - a)) * (bY - aY);
		}
	}
	else { // < a
		h = (LINE_TENSION / a) * aY;
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

	float adjMeterWidth = MAX(line_integrity * TENSION_METER_WIDTH, 0);
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
	/*struct Vector3 move_direction = Vector3_getForward(&object_fish.rotation);
	object_fish.forward = move_direction;
	float b = 0;*/

	//object_blahaj.rotation.y += DELTA_TIME;
	//struct Vector3 move_direction = object_fish.forward;// Vector3_getForward(&object_blahaj.rotation);
	//move_direction = Vector3_normalize(move_direction);
	//move_direction = Vector3_multiplyScalar(&move_direction, 150.0f * DELTA_TIME);

	//struct Vector3 move_forward = Vector3_multiplyScalar(&object_fish.forward, -DELTA_TIME * 1.f);
	////object_blahaj.position = Vector3_add(&object_blahaj.position, &move_forward);

	//struct Vector3 trace_target = Vector3_subtract(&object_fish.position, &move_direction);
}


//float quadBezierPoint(float p0, float p1, float p2, float t) {
//	return (1 - t) * (1 - t) * p0 + 2 * (1 - t) * t * p1 + t * t * p2;
//}

void lure_line_render(struct Vector3* target)
{
	struct Vector3 line_target = *target;//object_lure.position;
	struct Vector3 line_origin = camera_default_object.position;
	line_origin.x -= 3;
	line_origin.z -= 5;

	struct Vector3 last_point;
	struct Vector3 point;

	float h_diff = fabsf(line_target.y - line_origin.y);

	for (float p = 0; p <= 1; p += .15)
	{
		point = Vector3_lerp(&line_target, &line_origin, p);

		// Approximated quadratic ease curve
		point.y += (1 - cosf(p)) * 10;

		// Camera project
		point = Vector3_subtract(&point, &camera_default_object.position);
		PTR_Matrix3_apply(&camera_default.rotate_transform, &point);
		point = Camera_worldToScreenPos(&camera_default, &point);

		if (p > 0 && point.z > 0 && last_point.z > 0)
		{
			int shade = 24 * (1 - p);
			fillLine(frame, &last_point, 3, shade, &point, 3, shade);
		}

		last_point = point;
	}
}


float lure_wiggle_timer = 0;
void lure_update()
{
	//object_lure.rotation.y += DELTA_TIME;

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

	if (fsm_main.current_state == FSM_MAIN_REELING ||
		fsm_main.current_state == FSM_MAIN_FISHON)
	{
		if (ActiveLure->update != 0x0) {
			ActiveLure->update(ActiveLure, &object_lure_target_position);
		}

		//float change = pd->system->getCrankChange();
		if (UnderwaterView.Visible && crank_change != 0) // We don't care about direction currently
		{
			//object_lure.position.z -= change * .002f;
			//object_lure.position.y -= change * .001f;

			if (ActiveLure->move != 0x0) {
				ActiveLure->move(ActiveLure, &object_lure_target_position, fabsf(crank_change));

				lure_wiggle_timer += crank_change;
				if (lure_wiggle_timer > 1) lure_wiggle_timer -= 1;

				object_lure.rotation.y = sinf(lure_wiggle_timer * .01) * .5;
				//object_lure.rotation.z = Gradient_sample(lure_wiggle_timer) * .2;
				//object_lure.rotation.x = crank_change;
			}
		}

		if (yards <= 5) // TODO: Define "return to shore" threshold
		{
			yards = 1000;
			if (fsm_main.current_state == FSM_MAIN_REELING)
			{
				// Return to shore without fish
				fsm_set_state(&fsm_main, FSM_MAIN_CASTING);
			}
			else if (fsm_main.current_state == FSM_MAIN_FISHON)
			{
				// Caught a fish! Show results.
				fsm_set_state(&fsm_main, FSM_MAIN_RESULTS);
			}
		}
	}

	object_lure.position = Vector3_lerp(&object_lure.position, &object_lure_target_position, .25f);
}

struct Vector3 target_underwater_camera_position = { 0 };
void underwater_camera_update()
{
	target_underwater_camera_position.y = MAX(object_lure.position.y + 3, 0);
	target_underwater_camera_position.x = object_lure.position.x;

	camera_default_object.position = Vector3_lerp(&camera_default_object.position, &target_underwater_camera_position, .01);
	camera_default_object.position.z = object_lure.position.z + 3; // Don't lerp the Z axis


	// TODO: Add camera controls (but not too extreme so we can use our clipping hack)
}

float fish_wiggle_speed = 0.f;
void fish_vertShader (struct Actor* act, int time, struct Vector3* v_out)
{
	float z_extent = act->mesh->max_bounds.z - act->mesh->min_bounds.z;
	float p = (v_out->z - act->mesh->min_bounds.z) / z_extent;
	v_out->x += Gradient_sample(1.0f - p) / (20.f - fish_wiggle_speed);
}

void audio_lake_loop_callback(SoundSource* c, void* userdata)
{
	//pd->sound->sampleplayer->play(soundtrack_lake_loop, 1, 1.f);
}

float music_timer = -1;
void audio_intro_finish_callback(SoundSource* c, void* userdata)
{
	pd->sound->sampleplayer->play(soundtrack_lake_loop, 1, 1.f);
	//audio_lake_loop_callback(c, userdata);
	music_timer = 0; // start timer
}

struct Vector3 fish_swim_target = {0}; // The spot the fish is swimming towards
struct Vector3 fish_remain_position = { 0 };
float lake_loop_length;
void fishing_scene_init()
{
	//LookTarget_setTarget(&object_fish.look_target, &object_lure.position);
	LookTarget_setTarget(&object_fish.look_target, &fish_remain_position);
	LookTarget_setTarget(&camera_default.actor->look_target, &object_lure.position);
	LookTarget_setTarget(&camera_secondary.actor->look_target, &object_lure.position);
	LookTarget_setTarget(&camera_lake.actor->look_target, &object_cast_selector.position);

	object_lure_target_position = object_lure.position;

	ticker_add_from(ticker_filler_table);
	TICKER_FONT = FONT_MONTSERRAT_BOLD_14;

	fsm_main.init();
	fsm_fish.init();
	fsm_set_state(&fsm_main, FSM_MAIN_LOADING);
	fsm_set_state(&fsm_fish, FSM_FISH_IDLE);

	reel_click_player = pd->sound->sampleplayer->newPlayer();
	reel_long_player = pd->sound->sampleplayer->newPlayer();

	soundtrack_intro = pd->sound->sampleplayer->newPlayer();
	soundtrack_lake_loop = pd->sound->sampleplayer->newPlayer();
	soundtrack_underwater_loop = pd->sound->sampleplayer->newPlayer();
	soundtrack_fish_interest_loop = pd->sound->sampleplayer->newPlayer();
	soundtrack_fish_on_loop = pd->sound->sampleplayer->newPlayer();

	pd->sound->source->setFinishCallback(soundtrack_intro, &audio_intro_finish_callback, NULL);
	//pd->sound->source->setFinishCallback(soundtrack_lake_loop, &audio_lake_loop_callback, NULL);

	// Load Assets
	// TODO: Loading screen
	audio_reel_click = pd->sound->sample->load("Assets/Sounds/reel_click_single.wav");
	audio_reel_long = pd->sound->sample->load("Assets/Sounds/reel_click_long.wav");

	audio_intro = pd->sound->sample->load("Assets/Sounds/fh-intro.wav");
	audio_lake_loop = pd->sound->sample->load("Assets/Sounds/fh-lake.wav");
	audio_underwater_loop = pd->sound->sample->load("Assets/Sounds/fh-underwater.wav");
	audio_fish_interest_loop = pd->sound->sample->load("Assets/Sounds/fh-fishnotice.wav");
	audio_fish_on_loop = pd->sound->sample->load("Assets/Sounds/fh-fishon.wav");

	//lake_loop_length = pd->sound->sample->getLength(audio_lake_loop);
	
	pd->sound->sampleplayer->setSample(reel_click_player, audio_reel_click);
	pd->sound->sampleplayer->setSample(reel_long_player, audio_reel_long);

	pd->sound->sampleplayer->setSample(soundtrack_intro, audio_intro);
	pd->sound->sampleplayer->setSample(soundtrack_lake_loop, audio_lake_loop);
	pd->sound->sampleplayer->setSample(soundtrack_underwater_loop, audio_underwater_loop);
	pd->sound->sampleplayer->setSample(soundtrack_fish_interest_loop, audio_fish_interest_loop);
	pd->sound->sampleplayer->setSample(soundtrack_fish_on_loop, audio_fish_on_loop);

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

	pd->sound->sampleplayer->play(soundtrack_intro, 1, 1.f);
}


float lure_fitness = 0;
struct CrankPeriod crank_buffer[TIMING_BUFFER_LENGTH];
void shift_crank_buffer(float duration, float distance)
{
	float deg_per_second = (duration > 0 ? distance / duration / 360 : 0);
	//pd->system->logToConsole("duration: %f, distance: %f, speed: %f", duration, distance, deg_per_second);

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
		//pd->system->logToConsole("fitness: %f", lure_fitness);
	}
}

// TODO: Turn this into a new system and put it in the main scene probably
void music_update()
{
	// Timer off by default, ticks up when music_timer is set to at least 0
	if (music_timer >= 0)
		music_timer += DELTA_TIME;

	float b = 180; // bpm
	b = 60. / b; // beat duration in seconds
	b = b * 4; // measure duration (4 beats)
	b = b * 4; // sample is 4 measures

	if (music_timer >= b)
	{
		pd->sound->sampleplayer->play(soundtrack_lake_loop, 1, 1.f);
		music_timer -= b;
	}
}

void on_line_break()
{
	// Change fish state
	// Change gameplay state
	// Play animation
	// Play sound effect

	pd->system->logToConsole("Line broke!");
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

	music_update();

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
			//pd->system->logToConsole("Change direction %f -> %f", change, crank_distance);
			// Add crank_duration to buffer
			shift_crank_buffer(crank_duration, crank_distance);

			idle_duration = 0;
			crank_duration = 0;
		}

		idle_duration = 0;
		is_cranking = true;
		crank_duration += DELTA_TIME;
		crank_distance += change;

		line_reel_tension = .7f * MIN(fabsf(change / 40.f), 1); // cap at 100% tension

		pd->system->logToConsole("crank change %f, tension %f", change, line_reel_tension);
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

	if (fsm_main.current_state == FSM_MAIN_REELING || fsm_main.current_state == FSM_MAIN_FISHON)
	{
		depth = object_lure.position.y - WATER_SURFACE_Y;
		yards = MAP_RANGE(-30, 30, 100, 0, object_lure.position.z);
	}


	// If total line tension exceeds a threshold,
	if (LINE_TENSION > .98f)
	{
		// reduce the line integrity.
		line_integrity -= .025f;

		// TODO: Play sound effect

		// If the line integrity falls below a threshold,
		if (line_integrity < .1f)
		{
			// the line breaks.
			on_line_break();
		}
	}

	fsm_main.update();
	fsm_fish.update();
}


void fsm_main_init()
{
	fsm_main.on_change[FSM_MAIN_CASTING][FSM_MAIN_REELING] = &fsm_main_update_cast_to_reeling;
	fsm_main.on_change[FSM_MAIN_REELING][FSM_MAIN_CASTING] = &fsm_main_update_reeling_to_cast;
	fsm_main.on_change[FSM_MAIN_REELING][FSM_MAIN_FISHON] = &fsm_main_reeling_to_fishon;
}

void fsm_main_update()
{
	switch (fsm_main.current_state)
	{
	case FSM_MAIN_LOADING:
		// Do initial scene stuff

		fsm_set_state(&fsm_main, FSM_MAIN_CASTING);
		break;

	case FSM_MAIN_CASTING:
		//pd->system->logToConsole("FSM_CASTING");
		break;
	}
}


//#define FISH_INTEREST_IGNORE 0
//#define FISH_INTEREST_NOTICE 1
//#define FISH_INTEREST_THINK 5
//#define FISH_INTEREST_REJECT 10
//#define FISH_INTEREST_BITE 11
//int get_fish_interest(struct Actor* actor, struct Fish* fish, struct Lure* lure)
//{
//
//}

struct Vector3 fish_station; // The spot the fish should swim around
bool fsm_main_update_cast_to_reeling() {
	pd->system->logToConsole("CAST -> REELING");

	// Randomly place fish
	fish_station = (struct Vector3){3, 2, -3},

	object_lure.position.x = MAP_RANGE(-5, 5, -10, 10, object_cast_selector.position.x);
	object_lure.position.z = MAP_RANGE(-5, 5, -20, 20, object_cast_selector.position.z);
	object_lure.position.y = WATER_SURFACE_Y;

	LakeView.Visible = false;
	UnderwaterView.Visible = true;

	camera_default_object.position.z = object_lure.position.z + 5;
	camera_default_object.position.y = object_lure.position.y - 3;
	camera_default_object.position.x = object_lure.position.x;

	object_lure_target_position = object_lure.position;

	return true;
}

bool fsm_main_update_reeling_to_cast() {
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

bool fsm_main_reeling_to_fishon() {
	tension_meter_visible = true;
	// TODO: Set to false on transitioning back to main

	return true;
}

void fsm_fish_init()
{
	fsm_fish.on_change[FSM_FISH_IDLE][FSM_FISH_SWIM] = &fsm_fish_idle_to_swim;
	fsm_fish.on_change[FSM_FISH_SWIM][FSM_FISH_IDLE] = &fsm_fish_swim_to_idle;
	fsm_fish.on_change[FSM_FISH_IDLE][FSM_FISH_APPROACH] = &fsm_fish_to_approach;
	fsm_fish.on_change[FSM_FISH_SWIM][FSM_FISH_APPROACH] = &fsm_fish_to_approach;
	fsm_fish.on_change[FSM_FISH_APPROACH][FSM_FISH_BITE] = &fsm_fish_approach_to_bite;
	fsm_fish.on_change[FSM_FISH_BITE][FSM_FISH_SWIM] = &fsm_fish_idle_to_swim;
	fsm_fish.on_change[FSM_FISH_BITE][FSM_FISH_HOOKED] = &fsm_fish_bite_to_hook;
}

#define IDLE_TIMER_MAX 2
#define FISH_NOTICE_COOLDOWN_MAX 5
#define FISH_BITE_MAX 1
float fish_speed = 3; // TODO: Move this to fish data?
float fish_roam_radius = 4; // TODO: Move this to fish data?
float idle_timer = 0;
float fish_notice_cooldown = FISH_NOTICE_COOLDOWN_MAX;
float fish_bite_timer = 0;

#define FISH_ON_FIGHTING 1
#define FISH_ON_RELAX 0
#define FISH_ON_TIMER_MIN 1
#define FISH_ON_TIMER_MAX 4
int fish_on_state = FISH_ON_FIGHTING;
float fish_on_state_timer = FISH_ON_TIMER_MAX;
float look_at_fish_blend = 0.f;
float fish_side_lure_offset = 0.f; // x axis offset from lure
void fsm_fish_update()
{
	// Main behaviors
	switch (fsm_fish.current_state)
	{
	case FSM_FISH_IDLE:
		if (fish_notice_cooldown > 0) fish_notice_cooldown -= DELTA_TIME;
		else fish_notice_cooldown = 0;
		if (UnderwaterView.Visible)
		{
			// Decide if we should move randomly
			idle_timer += DELTA_TIME;

			if (idle_timer > IDLE_TIMER_MAX)
			{
				fsm_set_state(&fsm_fish, FSM_FISH_SWIM);
			}
		}
		break;

	case FSM_FISH_SWIM:
		if (fish_notice_cooldown > 0) fish_notice_cooldown -= DELTA_TIME;
		else fish_notice_cooldown = 0;
		if (UnderwaterView.Visible)
		{
			// Go back to idle if we're done swimming
			float distance = Vector3_getDistanceSquared(&object_fish.position, &fish_swim_target);
#define STOP_SWIMMING_THRESHOLD .5
			if (distance < STOP_SWIMMING_THRESHOLD) {
				fsm_set_state(&fsm_fish, FSM_FISH_IDLE);
				break;
			}

			// Swim
			// TODO: Ease speed in / out, tie speed to wave
			//struct Vector3 move_direction = object_fish.forward;// Vector3_getForward(&object_fish.rotation);
			//move_direction = Vector3_normalize(move_direction);
			//move_direction = Vector3_multiplyScalar(&move_direction, DELTA_TIME);
			//object_fish.position = Vector3_subtract(&object_fish.position, &move_direction);

			struct Vector3 move_forward = Vector3_multiplyScalar(&object_fish.forward, DELTA_TIME * .5);
			object_fish.position = Vector3_subtract(&object_fish.position, &move_forward);
			//float speed = Vector3_length(&move_forward);
			// TODO: Set this dynamically based on speed, once we're easing
			//fish_wiggle_speed = (speed / .1f) * 20.f; 
			//pd->system->logToConsole("swim speed %f", speed);
			/*if (speed > STOP_SWIMMING_THRESHOLD)
			{
				float b = 2;
			}*/
			// TODO: Currently being forced to look at the lure, need to remove that and instead
			// turn to look at the swim target

			//struct Vector3 trace_target = Vector3_subtract(&object_fish.position, &move_direction);
		}
		break;

	case FSM_FISH_APPROACH:
	{
		struct Vector3 move_forward = Vector3_multiplyScalar(&object_fish.forward, DELTA_TIME * .5);
		object_fish.position = Vector3_subtract(&object_fish.position, &move_forward);
	}
	break;

	case FSM_FISH_HOOKED:
	{
		// Fish has committed to taking the bait, it's on the hook until the line breaks or we reel it in.
		// Alternate between resisting (pulling away) and not.
		float z_delta = 0.f;
		switch (fish_on_state)
		{
		case FISH_ON_FIGHTING:
			//pd->system->logToConsole("fish fighting");
			// Reeling adds tension to the line.
			// Increment line tension
			// Increment yards
#define fish_speed 2.f;
			// TODO: Replace Fish Speed with an attribute from the fish
			//yards += DELTA_TIME * fish_speed;
			z_delta -= DELTA_TIME * fish_speed;

#define FISH_MAX_TENSION .5f
			if (line_fish_tension < FISH_MAX_TENSION)
			{
				line_fish_tension += DELTA_TIME;
			}
			if (line_fish_tension > FISH_MAX_TENSION) line_fish_tension = FISH_MAX_TENSION;

			break;

		case FISH_ON_RELAX:
			//pd->system->logToConsole("fish relaxing");
			z_delta += DELTA_TIME * .1f; // figure out natural towards-player drift
			if (line_fish_tension > 0)
			{
				line_fish_tension -= DELTA_TIME;
			}
			if (line_fish_tension < 0) line_fish_tension = 0;
			break;
		}

		// Move the lure forward or back
		// TODO: Also juke left/right (x)
		object_lure_target_position.z += z_delta;
		struct Vector3 fish_target = object_lure_target_position;
		fish_target = Vector3_subtract(&fish_target, &object_fish.position);
		fish_target = Vector3_normalize(fish_target);
		fish_target = Vector3_subtract(&object_lure_target_position, &fish_target);
		/*fish_target.x += fish_side_lure_offset;
		fish_target.z += fish_on_state == FISH_ON_FIGHTING ? 1 : -1;*/
		object_fish.position = Vector3_lerp(&object_fish.position, &fish_target, .5f);
		//object_fish.position.x += fish_side_lure_offset;

		// Wait out the remaining time we have in this state
		if (fish_on_state_timer > 0)
		{
			fish_on_state_timer -= DELTA_TIME;
		}
		else
		{
			// Flip the state
			fish_on_state = fish_on_state == FISH_ON_FIGHTING ? FISH_ON_RELAX : FISH_ON_FIGHTING;

			// Calculate the new time duration
			float r = ((float)rand() / (float)RAND_MAX); // 0 - 1
			fish_on_state_timer = FISH_ON_TIMER_MIN + r * (FISH_ON_TIMER_MAX - FISH_ON_TIMER_MIN);
		}
	}
	break;
	}

	// == FISH INTEREST STATE CHANGES ==
	switch (fsm_fish.current_state)
	{
	case FSM_FISH_IDLE:
	case FSM_FISH_SWIM:
	{
		// If we've cooled down from the last approach
		if (fish_notice_cooldown > 0) break;

		// Calculate the approach interest
		float distance_to_fish = Vector3_getDistanceSquared(&object_fish.position, &object_lure.position);
		// TODO: Calculate this threshold based on fish and/or lure
		float distance_notice_threshold = 3.5; // Threshold outside of which the lure isn't noticed


		// Make sure we clear the "close enough" threshold before checking anything else
		if (distance_to_fish >
			distance_notice_threshold * distance_notice_threshold)
		{
			break;
		}

		/*if (look_at_fish_blend < .5)
		{*/
			if (camera_default_object.look_target.next == NULL)
			{
				LookTarget_setTarget(&camera_default_object.look_target, &object_fish.position);
				camera_default_object.look_target.max_blend = .25f; // look partway
			}
			//camera_default_object.look_target. = &object_fish;

			//look_at_fish_blend += DELTA_TIME;
			//camera_default.look_blend = look_at_fish_blend;
		//}

		// Calculate dot product of the fish facing the lure
		struct Vector3 fish_to_lure = Vector3_subtract(&object_lure.position, &object_fish.position);
		fish_to_lure = Vector3_normalize(fish_to_lure);
		float dot_fish_facing = Vector3_dot(object_fish.forward, fish_to_lure);

		// If the fish is looking at the lure
#define FISH_NOTICE_DOT_THRESHOLD .5
		//pd->system->logToConsole("fish facing dot %f > %f?", dot_fish_facing, -FISH_NOTICE_DOT_THRESHOLD);
		if (dot_fish_facing < -FISH_NOTICE_DOT_THRESHOLD
			// or the lure_fitness is too tempting
			// || (lure_fitness >= .75
			&& is_cranking
			) {

			// Then we approach
			fsm_set_state(&fsm_fish, FSM_FISH_APPROACH);
			pd->system->logToConsole("fish noticed lure, approachingg");
		}
	}
	break;

	case FSM_FISH_APPROACH:
	{
		struct Vector3 fish_ref_point = Actor_getFrontPointZ(&object_fish);
		float distance_to_fish = Vector3_getDistanceSquared(&fish_ref_point, &object_lure.position);

		// Once we've reached the lure
		if (distance_to_fish < .5) {
			fsm_set_state(&fsm_fish, FSM_FISH_BITE);
			pd->system->logToConsole("fish reached lure");
		}
	}
	break;

	case FSM_FISH_BITE:
	{
		// Decide if we should take the bait

		// Wait a little
		if (fish_bite_timer <= FISH_BITE_MAX)
		{
			fish_bite_timer += DELTA_TIME;
		}
		else { // and decide
			// TODO: Base this on something besides random chance
			float r = ((float)rand() / (float)RAND_MAX);
			bool take_bait = r > 0; // > .5 is 50/50
			if (take_bait) {
				pd->system->logToConsole("FISH ON!");
				fsm_set_state(&fsm_fish, FSM_FISH_HOOKED);
				fsm_set_state(&fsm_main, FSM_MAIN_FISHON);
			}
			else {
				// TODO: Add "swim away" rejection state instead?
				pd->system->logToConsole("fish rejected bait");
				fish_notice_cooldown = FISH_NOTICE_COOLDOWN_MAX;
				fsm_set_state(&fsm_fish, FSM_FISH_SWIM);
			}
		}
	}
	break;
	}
}

bool fsm_fish_idle_to_swim()
{
	pd->system->logToConsole("FISH IDLE -> SWIM");
	// Set new swim target
	fish_swim_target = object_lure.position;//fish_station;
	fish_swim_target.y = object_fish.position.y;
	fish_swim_target.x += ((float)rand() / (float)RAND_MAX) * fish_roam_radius;
	fish_swim_target.z += ((float)rand() / (float)RAND_MAX) * fish_roam_radius;
	// TODO: Handle y differently (more limited radius) 
	//fish_swim_target.y += ((float)rand() / (float)RAND_MAX) * fish_roam_radius;
	//if (fish_swim_target.y < 0) fish_swim_target.y = 0;

	/*object_fish.look_target.blend = 0;
	object_fish.look_target.current = &object_fish.forward;
	object_fish.look_target.next = &fish_swim_target;*/
	//object_fish.look_target.
	LookTarget_setTarget(&object_fish.look_target, &fish_swim_target);

	fish_wiggle_speed = 5.f;

	return true;
}

bool fsm_fish_swim_to_idle()
{
	pd->system->logToConsole("FISH SWIM -> IDLE");
	idle_timer = 0;
	//object_fish.position = fish_station,
	////LookTarget_setTarget(&object_fish.look_target, NULL);
	fish_remain_position = object_fish.position;
	fish_remain_position = Vector3_subtract(&fish_remain_position, &object_fish.forward);
	LookTarget_setTarget(&object_fish.look_target, &fish_remain_position);

	fish_wiggle_speed = -10.f;

	return true;
}

bool fsm_fish_to_approach()
{
	pd->system->logToConsole("FISH APPROACH");

	LookTarget_setTarget(&object_fish.look_target, &object_lure.position);
	fish_wiggle_speed = 5.f;

	return true;
}

bool fsm_fish_approach_to_bite()
{
	fish_bite_timer = 0;
	return true;
}

bool fsm_fish_bite_to_hook()
{
	fish_side_lure_offset = object_fish.position.x - object_lure.position.x;

	return true;
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

	object_cast_selector.rotation.y += DELTA_TIME * .34f;

	// == Cast selector y bob
	float WATER_X_SIZE = 20;
	float WATER_Z_SIZE = 20;
	float MAX_HEIGHT = .2f;
	float u = (object_cast_selector.position.x + WATER_X_SIZE) / (WATER_X_SIZE * 2.5f);
	float v = (object_cast_selector.position.z - camera_lake.actor->position.z) / (camera_lake.actor->position.z - WATER_Z_SIZE);

	float height_sample = (sample_wave_heightmap((int)((u + WAVE_RIPPLE_U) * WAVE_HEIGHTMAP_COLUMNS), (int)(((v + WAVE_RIPPLE_V) * WAVE_HEIGHTMAP_ROWS))) / 128.f) * MAX_HEIGHT;
	height_sample = MAX_HEIGHT - height_sample; // invert!
	object_cast_selector.mesh->origin.y = height_sample / object_cast_selector.scale.y * .75f;
	// ==

	if (fsm_main.current_state == FSM_MAIN_CASTING)
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
				object_cast_selector.scale.z -= DELTA_TIME * .5f / CAST_ANIMATION_TIME;

			// If we _were_ above 0 and now we're not
			if (cast_animation_timer <= 0)
			{
				cast_animation_timer = 0;

				fsm_set_state(&fsm_main, FSM_MAIN_REELING);
			}
		}
	}
}

void lake_draw()
{
	//YPlane_render(frame, &camera_lake, 0, false);
	//YPlane_renderGrid(frame, &camera_lake, 0);
	LakeWaves_render(frame, &camera_lake);
	LakeWaves_renderWaterHeight(frame, &camera_lake);
	//perlin_test();
}

void lake_postdraw()
{
	// UI Rendering
	if (fsm_main.current_state == FSM_MAIN_FISHON)
	{
		ui_yards();
		ui_tension();
	}
	ui_news_ticker();
}


void underwater_update()
{
	object_lure.visible =
		fsm_fish.current_state != FSM_FISH_BITE &&
		fsm_fish.current_state != FSM_FISH_HOOKED;
}

void underwater_draw()
{

}

void underwater_postdraw()
{
	// Draw things relative to the fish reference points
	struct Vector3 fish_ref_point = Actor_getFrontPointZ(&object_fish);// object_fish.position;
	struct Vector3 fish_up_offset = { .y = -1.1, .z = .5 };
	struct Vector3 fish_up = Vector3_subtract(&fish_ref_point, &fish_up_offset);

	// Circle the reference point 
	struct Vector3 fish_front = fish_ref_point;
	fish_front = Vector3_subtract(&fish_front, &camera_default_object.position);
	PTR_Matrix3_apply(&camera_default.rotate_transform, &fish_front);
	fish_front = Camera_worldToScreenPos(&camera_default, &fish_front);

	// Draw mouth reference point
//#define FISH_FRONT_RADIUS 10
//	pd->graphics->drawEllipse(
//		fish_front.x - (FISH_FRONT_RADIUS / 2),
//		fish_front.y - (FISH_FRONT_RADIUS / 2),
//		FISH_FRONT_RADIUS, FISH_FRONT_RADIUS,
//		1, 0, 360, kColorWhite);

	// Circle a point above the reference point
	// This is where we'll display the current state
	fish_up = Vector3_subtract(&fish_up, &camera_default_object.position);
	PTR_Matrix3_apply(&camera_default.rotate_transform, &fish_up);
	fish_up = Camera_worldToScreenPos(&camera_default, &fish_up);

	pd->graphics->setFont(FONT_MONTSERRAT_BLACK_24);
	float text_off = pd->graphics->getTextWidth(FONT_MONTSERRAT_BLACK_24, "X", 1, kASCIIEncoding, 0) / 2;
	switch (fsm_fish.current_state) {
	case FSM_FISH_IDLE:
		pd->graphics->drawText("...", 3, kASCIIEncoding, fish_up.x - text_off, fish_up.y);
		break;
	case FSM_FISH_APPROACH:
		pd->graphics->drawText("?", 3, kASCIIEncoding, fish_up.x - text_off, fish_up.y);
		break;
	case FSM_FISH_HOOKED:
		pd->graphics->drawText("!", 3, kASCIIEncoding, fish_up.x - text_off, fish_up.y);
		break;
	case FSM_FISH_SWIM:
		if (fish_notice_cooldown > 0)
			pd->graphics->drawText("X", 3, kASCIIEncoding, fish_up.x - text_off, fish_up.y);
		break;
	}
	/*pd->graphics->drawEllipse(
		fish_up.x - (FISH_FRONT_RADIUS / 2),
		fish_up.y - (FISH_FRONT_RADIUS / 2),
		FISH_FRONT_RADIUS / 2, FISH_FRONT_RADIUS / 2,
		1, 0, 360, kColorWhite);*/


	// Render lure line
	if (fsm_fish.current_state == FSM_FISH_BITE ||
		fsm_fish.current_state == FSM_FISH_HOOKED)
		lure_line_render(&fish_ref_point);
	else
		lure_line_render(&object_lure.position);


	// Replace with waves?
	// YPlane_render(frame, &camera_default, WATER_SURFACE_Y, false); 

	// UI Rendering
	ui_depth(); // render before yards / ticker
	ui_yards();
	ui_news_ticker();
	//ui_fish_name();
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
		&lake_terrain,
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