#include "../../ddd/ddd.h"
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
	.rotation = { 0, 0, 0 },
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
	.position = { 0, 2.0f, 5.0f },
	//.rotationX = { 0, 0, 0 },
	.rotation = {0,0,0},
	.render_mode = RENDER_WIREFRAME,
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
	object_test.rotation.y += 0.05f;
	//object_test.rotation.z += 0.025f;

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

	/*Scene_addGameObject(pd, &TestScene, &object_cube);
	Scene_addGameObject(pd, &TestScene, &object_cube2);*/
	Scene_addGameObject(pd, &TestScene, &object_test);
	Scene_addGameObject(pd, &TestScene, &object_terrain);
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
