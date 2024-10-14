#include "../../ddd/ddd.h"
#include "../../ddd/gradient.h"
 #include "../cube.h"
#include "../blahaj_tri.h"
#include "../terrain1.h"
#include "../../ddd/symbols.h";

/*
Externs
*/

extern PlaydateAPI* pd;
extern float DELTA_TIME;
extern uint8_t* frame;
extern PDButtons heldButtons, pressedButtons, releasedButtons;\

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
 	.position = { 3, .5f, 0 },
 	.rotation = { 0, 0, 0 },
 	.scale = { .05f, .05f, .05f },
	.update = &lure_update,
	.scene = &FishingScene,
 };

 struct Actor object_fish = {
	 .name = "Fish",
	 .mesh = &blahaj_tri,
	 .position = { -2, 2, 0 },
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
	LookTarget_setTarget(&camera_default.actor->look_target, &object_fish.position);
	LookTarget_setTarget(&camera_secondary.actor->look_target, &object_lure.position);
}

void fishing_scene_update()
{
	if (pressedButtons & kButtonB)
	{
		LakeView.Visible = !LakeView.Visible;
		UnderwaterView.Visible = !LakeView.Visible;
	}
}

void lake_update()
{

}

void lake_predraw()
{
	pd->graphics->fillRect(20, 20, 400 - 20*2, 240 - 20*2, kColorBlack);
}

void lake_draw()
{
	YPlane_render(frame, &camera_lake, 0, false);
}

void lake_postdraw()
{
	pd->graphics->fillRect(400 / 2, 240 / 2, 20, 20, kColorBlack);
}

void underwater_predraw()
{

}

void underwater_draw()
{

}

void underwater_postdraw()
{

}

void underwater_update()
{

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
	.predraw = &underwater_predraw,
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
		NULL
	},
	.predraw = &lake_predraw,
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