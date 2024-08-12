#include "../../ddd/ddd.h"
#include "../../ddd/gradient.h"
#include "../cube.h"
#include "../cube2.h"
#include "../toa_head.h"
// #include "../fish.h"
 #include "../fish2.h"
#include "../salmon.h"
#include "../blahaj_tri.h"
#include "../terrain1.h"


struct Scene TestScene;

/*
	Game Objects
*/
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

struct GameObject object_test = {
	.name = "Test",
	.mesh = &blahaj_tri,
	.position = { 0, 1, 0 },
	.rotation = { 0, -1, 0 },
	.scale = { 1, 1, 1 }
};

struct GameObject object_terrain = {
	.name = "Terrain",
	.mesh = &terrain1,
	.position = { 0, 0, 0 },
	.rotation = { 0, 0, 0 },
	.scale = { 1, 1, 1 }
};


/*
	Cameras
*/

struct Camera camera_default = {
	.near = 0.1f,
	.far = 1000.0f,
	.fov = 60.0f,
	.look_target = { 0, 0, 0 },
	.position = { 0, .25f, 5.0f },
	//.rotationX = { 0, 0, 0 },
	.rotation = {-.1f,0,0},
	.render_mode = RENDER_WIREFRAME,
	.light_dir = { .5f,.5f,.5f },
};

/*
	Methods
*/

void cube_update(PlaydateAPI* pd)
{
	// Rotate cube
	object_cube.rotation.x += 0.01f;
	object_cube.rotation.y += 0.05f;
	object_cube.rotation.z += 0.025f;

}

void cube2_update(PlaydateAPI* pd)
{

}

void test_update(PlaydateAPI* pd)
{
	//	object_test.rotation.x += 0.01f;
	object_test.rotation.y += 0.03f;
	//object_test.rotation.z += 0.025f;

}

float quicksin(float t)
{
	return (t * (180 - t)) / 8100.0f;
}

void test_vertShader (PlaydateAPI* pd, struct GameObject* go, int time, struct Vector3* v_out)
{
	//v_out->x += sinf(v_out->z - time / 100.0f) / 2.5f;
	float z_extent = go->mesh->max_bounds.z - go->mesh->min_bounds.z;
	float p = (v_out->z - go->mesh->min_bounds.z) / z_extent;
	v_out->x += Gradient_sample(1.0f - p) / 5;
}

void terrain_update(PlaydateAPI* pd)
{

}


void test_scene_init(PlaydateAPI* pd)
{
	// TODO: This will need to be moved
	camera_default.projection = Camera_getProjectionMatrix(&camera_default);

	object_cube.update = &cube_update;
	object_cube2.update = &cube2_update;
	object_test.update = &test_update;
	object_terrain.update = &terrain_update;

	object_test.vertShader = &test_vertShader;

	/*Scene_addGameObject(pd, &TestScene, &object_cube);
	Scene_addGameObject(pd, &TestScene, &object_cube2);*/
	Scene_addGameObject(pd, &TestScene, &object_test);
	//Scene_addGameObject(pd, &TestScene, &object_terrain);
}

void test_scene_update(PlaydateAPI* pd)
{
	struct Vector3 object_forward = Vector3_getForward(&object_test.rotation);
	object_forward = Vector3_multiplyScalar(&object_forward, 5);
	struct Vector3 object_forward_pos = Vector3_subtract(&object_test.position, &object_forward);
	//Line_worldDraw(pd, object_test.position, object_forward_pos, 1, &camera_default);
}


/* 
	Scene Object
*/
struct Scene TestScene = {
	.name = "Test",
	/*.numGameObjects = 2,
	.gameObjects = (struct GameObject* []){ &object_cube, &object_cube2 },*/
	.numCameras = 1,
	.cameras = (struct Camera* []){ &camera_default },
	.init = &test_scene_init,
	.update = &test_scene_update,
};
