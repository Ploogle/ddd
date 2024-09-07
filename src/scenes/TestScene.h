#include "../../ddd/ddd.h"
#include "../../ddd/gradient.h"
 #include "../cube.h"
// #include "../cube2.h"
// #include "../toa_head.h"
// #include "../fish.h"
//  #include "../fish2.h"
// #include "../salmon.h"
#include "../blahaj_tri.h"
#include "../terrain1.h"
#include "../../ddd/symbols.h";

struct Scene TestScene;

extern PlaydateAPI* pd;
extern float DELTA_TIME;

/*
	Game Objects
*/
 struct GameObject object_cube2 = {
 	.name = "Cube 2",
 	.mesh = &mesh_cube,
 	.position = { 3, .5f, 0 },
 	.rotation = { 0, 0, 0 },
 	.scale = { .05f, .05f, .05f }
 };

// struct GameObject object_cube = {
// 	.name = "Cube",
// 	.mesh = &mesh_cube,
// 	.position = { 0, 1.5f, 0},
// 	.rotation = { 0, 0, 0 },
// 	.scale = { .1f, .1f, .1f }
// };

struct GameObject object_blahaj = {
	.name = "Blahaj",
	.mesh = &blahaj_tri,
	.position = { -2, 2, 0 },
	.rotation = { 0, 0, 0 },
	.scale = { 1, 1, 1 }
};

struct GameObject object_terrain = {
	.name = "Terrain",
	.mesh = &terrain1,
	.position = { 0, .5, 0 },
	.rotation = { 0, 0, 0 },
	.scale = { 1, 1, 1 }
};


/*
	Cameras
*/

struct Camera camera_default = {
	.near = 0.0f,
	.far = 10.0f,
	.fov = 60.0f,
	.far_fog = 10.0f,
	.near_fog = 1,
	//.look_target = &GLOBAL_ORIGIN,
	.position = { 0, 2.5, 5.0f },
	.rotation = {0,0,0},
	.render_mode = RENDER_WIREFRAME,
	.light_dir = { .5f,.5f,.5f },
};

/*
	Methods
*/

void blahaj_update()
{
	//object_blahaj.rotation.y += DELTA_TIME;
	struct Vector3 forward = object_blahaj.forward;// Vector3_getForward(&object_blahaj.rotation);
	forward = Vector3_normalize(forward);
	forward = Vector3_multiplyScalar(&forward, 150.0f * DELTA_TIME);

	struct Vector3 move_forward = Vector3_multiplyScalar(&object_blahaj.forward, -DELTA_TIME * 1.f);
	object_blahaj.position = Vector3_add(&object_blahaj.position, &move_forward);

	struct Vector3 trace_target = Vector3_subtract(&object_blahaj.position, &forward);
	Line_worldDraw(
		trace_target,
		object_blahaj.position,
		1.0f,
		&camera_default);
}

void cube2_update()
{
	object_cube2.rotation.y += DELTA_TIME;
}

void test_vertShader (struct GameObject* go, int time, struct Vector3* v_out)
{
	float z_extent = go->mesh->max_bounds.z - go->mesh->min_bounds.z;
	float p = (v_out->z - go->mesh->min_bounds.z) / z_extent;
	v_out->x += Gradient_sample(1.0f - p) / 5.0f;
}

void terrain_update()
{

}

void test_scene_init()
{
	// TODO: This will need to be moved
	camera_default.projection = Camera_getProjectionMatrix(&camera_default);

	// object_cube.update = &cube_update;
	 object_cube2.update = &cube2_update;
	object_blahaj.update = &blahaj_update;
	object_terrain.update = &terrain_update;

	object_blahaj.vertShader = &test_vertShader;
	//object_blahaj.look_target = &camera_default.position;

	//Scene_addGameObject(&TestScene, &object_cube);
	Scene_addGameObject(&TestScene, &object_cube2);
	//Scene_addGameObject(&TestScene, &object_terrain);
	Scene_addGameObject(&TestScene, &object_blahaj);
	//Scene_addGameObject(&TestScene, &object_test2);
	//Scene_addGameObject(&TestScene, &object_test3);
}

void test_scene_update()
{
	//struct Vector3 forward = Vector3_getForward(&object_test.rotation);
	/*struct Vector3 left = Vector3_getLeft(&object_test.rotation);
	struct Vector3 up = Vector3_getUp(&object_test.rotation);*/

	//forward = Vector3_multiplyScalar(&forward, 3);
	/*left = Vector3_multiplyScalar(&left, 5);
	up = Vector3_multiplyScalar(&up, 5);*/

	/*struct Vector3 origin = Vector3_add(&object_test.position, &object_test.mesh->origin);
	struct Vector3 object_forward_pos = Vector3_subtract(&origin, &forward);*/
	/*struct Vector3 object_right_pos = Vector3_subtract(&origin, &left);
	struct Vector3 object_up_pos = Vector3_subtract(&origin, &up);*/

	//Line_worldDraw(pd, origin, object_forward_pos, 1, &camera_default);
}


/* 
	Scene Object
*/
struct Scene TestScene = {
	.name = "Test",
	.numCameras = 1,
	.cameras = (struct Camera* []){ &camera_default },
	.init = &test_scene_init,
	.update = &test_scene_update,
};
