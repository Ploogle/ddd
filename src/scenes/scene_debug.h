
#include "../../engine/ddd.h";
#include "../../engine/gradient.h";
#include "../models/model_cube.h";
#include "../models/model_blahaj_tri.h";
#include "../../engine/symbols.h";

/*
	Externs
*/

extern PlaydateAPI* pd;
extern float DELTA_TIME;
extern uint8_t* frame;
extern PDButtons heldButtons, pressedButtons, releasedButtons;

/*
	Definitions
*/
struct Scene DebugScene;
struct View DefaultView;

void debug_camera_default_update();
void debug_scene_init();
void debug_scene_update();
void debug_object_cube_update();

/*
	Actors
*/

struct Actor debug_object_cube = {
   .name = "Cube",
   .visible = true,
   .mesh = &mesh_cube,
   .position = { 0, .5f, 0 },
   .rotation = { 0, 0, 0 },
   .scale = { .05f, .05f, .05f },
   .scene = &DebugScene,
   .update = &debug_object_cube_update,
};

/*
	Cameras
*/

struct Actor debug_camera_default_object = {
	.name = "Default Camera",
	.visible = true,
	.position = { 0, 2.5f, 5.0f },
	.rotation = { 0, 0, 0},
	.scale = { 1, 1, 1 },
	.look_target = {
		.tween_speed = 1.f,
	},
	.update = &debug_camera_default_update,
};

struct Actor debug_camera_secondary_object = {
	.name = "Secondary Camera",
	.visible = true,
	.position = { -4.0f, 4.f, 2.0f },
	.rotation = { 0, 3.14f, 0},
	.scale = { 1, 1, 1 },
	.look_target = {
		.tween_speed = 1.f,
	}
};

struct Camera debug_camera_default = {
	.actor = &debug_camera_default_object,
	.near = 0.0f,
	.far = 10.0f,
	.fov = 60.0f,
	.far_fog = 10.0f,
	.near_fog = 1,
	.render_mode = RENDER_WIREFRAME,
	.light_dir = { .5f,.5f,.5f },
};


struct Camera debug_camera_secondary = {
	.actor = &debug_camera_secondary_object,
	.near = 0.0f,
	.far = 10.0f,
	.fov = 60.0f,
	.far_fog = 10.0f,
	.near_fog = 1,
	.render_mode = RENDER_WIREFRAME,
	.light_dir = { .5f,.5f,.5f },
};

/*
	Methods
*/

void debug_object_cube_update()
{
	debug_object_cube.rotation.y += DELTA_TIME;
}

bool look_lerp = false;
int look_lerp_dir = 1;
int target_idx = 1;
struct Vector3 camera_velocity = { 0,0,0 };
void debug_camera_default_update()
{
	float moveSpeed = DELTA_TIME * 4;
	float rotSpeed = DELTA_TIME;

	if (heldButtons & kButtonUp) {
		camera_velocity.z -= moveSpeed;
	}
	else if (heldButtons & kButtonDown) {
		camera_velocity.z += moveSpeed;
	}

	if (heldButtons & kButtonLeft) {
		camera_velocity.x += moveSpeed;
	}
	else if (heldButtons & kButtonRight) {
		camera_velocity.x -= moveSpeed;
	}

	//if (pressed & kButtonA && !camera_default.actor->look_target.is_tweening) {
	//	if (target_idx == 0)
	//	{
	//		target_idx = 1;
	//		LookTarget_setTarget(&camera_default.actor->look_target, &object_fish.position);
	//		//LookTarget_setTarget(&object_blahaj.look_target, &activeCamera->actor->position);
	//	}
	//	else if(target_idx == 1)
	//	{
	//		target_idx = 0;
	//		LookTarget_setTarget(&camera_default.actor->look_target, &object_lure.position);
	//		//LookTarget_setTarget(&object_blahaj.look_target, &object_cube2.position);
	//	}
	//}

	float crankDelta = pd->system->getCrankChange();
	camera_velocity.y += DELTA_TIME * crankDelta;
	/*if (camera_default.look_blend == 1.f) {
		camera_default.actor->rotation.y -= 0.01f * crankDelta;
	}
	else {
		debug_camera_default.actor->position.y += 0.01f * crankDelta;
	}*/
	struct Vector3 raw_forward = Vector3_getForward(&debug_camera_default.actor->rotation);
	struct Vector3 raw_left = Vector3_getLeft(&debug_camera_default.actor->rotation);
	struct Vector3 raw_up = Vector3_getUp(&debug_camera_default.actor->rotation);
	struct Vector3 forward = Vector3_multiplyScalar(&raw_forward, camera_velocity.z);
	struct Vector3 left = Vector3_multiplyScalar(&raw_left, -camera_velocity.x);
	struct Vector3 up = Vector3_multiplyScalar(&raw_up, -camera_velocity.y);
	struct Vector3 dir = Vector3_add(&forward, &left);
	dir = Vector3_add(&dir, &up);

	debug_camera_default.actor->position = Vector3_add(&debug_camera_default.actor->position, &dir);
	camera_velocity = Vector3_multiplyScalar(&camera_velocity, 0.5f);
}

void debug_scene_init()
{

}

void debug_scene_update()
{

}


void debug_default_draw()
{
	YPlane_render(frame, &debug_camera_default, 0, true);
}


/*
	Scene Object
*/

struct View DefaultView = {
	.Enabled = true,
	.Visible = true,
	.numCameras = 1,
	.cameras = (struct Camera* []){
		&debug_camera_default,
		NULL
	},
	.actors = (struct Actor* []){
		&debug_object_cube,
		NULL
	},
	.draw = &debug_default_draw,
};

struct Scene DebugScene = {
	.init = &debug_scene_init,
	.update = &debug_scene_update,
	.views = (struct View* []) {
		&DefaultView,
		NULL
	},
};
