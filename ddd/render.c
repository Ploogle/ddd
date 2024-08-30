#include <stdint.h>

#include <time.h>
#include <stdlib.h>

#include "gameobject.h"
#include "3dmath.h"
#include "render.h"
#include "pd_api.h"
#include "camera.h"
#include "bluenoise.h"

#define sign(x) (-(int)((unsigned int)((int)(x)) >> (sizeof(int) * CHAR_BIT - 1)))

typedef uint8_t Pattern[8];

static Pattern patterns[] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x80, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 },
	{ 0x88, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00 },
	{ 0x88, 0x00, 0x20, 0x00, 0x88, 0x00, 0x02, 0x00 },
	{ 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 },
	{ 0xa8, 0x00, 0x22, 0x00, 0x8a, 0x00, 0x22, 0x00 },
	{ 0xaa, 0x00, 0x22, 0x00, 0xaa, 0x00, 0x22, 0x00 },
	{ 0xaa, 0x00, 0xa2, 0x00, 0xaa, 0x00, 0x2a, 0x00 },
	{ 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00, 0xaa, 0x00 },
	{ 0xaa, 0x40, 0xaa, 0x00, 0xaa, 0x04, 0xaa, 0x00 },
	{ 0xaa, 0x44, 0xaa, 0x00, 0xaa, 0x44, 0xaa, 0x00 },
	{ 0xaa, 0x44, 0xaa, 0x10, 0xaa, 0x44, 0xaa, 0x01 },
	{ 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 },
	{ 0xaa, 0x54, 0xaa, 0x11, 0xaa, 0x45, 0xaa, 0x11 },
	{ 0xaa, 0x55, 0xaa, 0x11, 0xaa, 0x55, 0xaa, 0x11 },
	{ 0xaa, 0x55, 0xaa, 0x51, 0xaa, 0x55, 0xaa, 0x15 },
	{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 },
	{ 0xba, 0x55, 0xaa, 0x55, 0xab, 0x55, 0xaa, 0x55 },
	{ 0xbb, 0x55, 0xaa, 0x55, 0xbb, 0x55, 0xaa, 0x55 },
	{ 0xbb, 0x55, 0xea, 0x55, 0xbb, 0x55, 0xae, 0x55 },
	{ 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee, 0x55 },
	{ 0xfb, 0x55, 0xee, 0x55, 0xbf, 0x55, 0xee, 0x55 },
	{ 0xff, 0x55, 0xee, 0x55, 0xff, 0x55, 0xee, 0x55 },
	{ 0xff, 0x55, 0xfe, 0x55, 0xff, 0x55, 0xef, 0x55 },
	{ 0xff, 0x55, 0xff, 0x55, 0xff, 0x55, 0xff, 0x55 },
	{ 0xff, 0x55, 0xff, 0xd5, 0xff, 0x55, 0xff, 0x5d },
	{ 0xff, 0x55, 0xff, 0xdd, 0xff, 0x55, 0xff, 0xdd },
	{ 0xff, 0x75, 0xff, 0xdd, 0xff, 0x57, 0xff, 0xdd },
	{ 0xff, 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd },
	{ 0xff, 0x77, 0xff, 0xfd, 0xff, 0x77, 0xff, 0xdf },
	{ 0xff, 0x77, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff },
	{ 0xff, 0xf7, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff },
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
};

int compare_zdepth(struct Triangle* a, struct Triangle* b)
{
	return a->center.z - b->center.z;
} 



// void Line_draw(uint8_t* bitmap, struct Vector3* p1, struct Vector3* p2, int thick)
// {
// 	// Sort the input points by x
// 	struct Vector3* x1;
// 	struct Vector3* y1;
// 	struct Vector3* x2;
// 	struct Vector3* y2;

// 	if (p1->x <= p2->x) { x1 = p1; y1 = p1; x2 = p2; y2 = p2; }
// 	else { x1 = p2; y1 = p2; x2 = p1; y2 = p1; }

// 	float x = x1->x,
// 		y = y1->y,
// 		dx = x2->x - x1->x,
// 		dy = y2->y - y1->y,
// 		segs = (float)dy / (float)dx; // Divide the y axis into segments

// 	if (dx < abs(dy)) {
// 		segs = (float)dx / (float) dy;
// 		if (p1->y < p2->y) { y2 = p2; y1 = p1; }
// 		for (y = y1->y; y <= y2->y; y++)
// 		{
// 			if (x > 0 && x < LCD_COLUMNS && y > 0 && y < LCD_ROWS) {
// 				setpixel(bitmap, (uint8_t)x, (uint8_t)y, LCD_ROWSIZE);
// 			}
// 			x += segs;
// 		}
// 	}
// 	else {
// 		for (; x <= x2->x; x++)
// 		{
// 			if (x > 0 && x < LCD_COLUMNS && y > 0 && y < LCD_ROWS) {
// 				setpixel(bitmap, (uint8_t)x, (uint8_t)y, LCD_ROWSIZE);
// 			}
// 			y += segs;
// 		}
// 	}
// }

void Triangle_draw(uint8_t* bitmap, struct Vector3* a, struct Vector3* b, struct Vector3* c, RENDER_MODE mode, LCDSolidColor color, int line_width, float face_color)
{
	pd->graphics->fillTriangle(a->x, a->y, b->x, b->y, c->x, c->y, kColorWhite);

	/*pd->graphics->drawLine(a->x, a->y, b->x, b->y, line_width, kColorWhite);
	pd->graphics->drawLine(b->x, b->y, c->x, c->y, line_width, kColorWhite);
	pd->graphics->drawLine(c->x, c->y, a->x, a->y, line_width, kColorWhite);*/
}

float _edge(struct Vector3 a, struct Vector3 b, float px, float py)
{
	return (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x);
}

void GameObject_render(uint8_t* bitmap, struct GameObject* go, struct Camera* camera)
{
	/* 
		This will technically work for wireframe, but will need to be replaced
		with a proper z-sorted list of game objects once we're rasterizing.
	*/

	// Draw us
	GameObject_drawMesh(bitmap, go, camera);

	// Draw our children
	for (int i = 0; i < go->num_children; i++)
	{
		//GameObject_render(pd, bitmap, &go->children[i], camera);
	}
}

// TODO: https://webglfundamentals.org/webgl/lessons/webgl-3d-orthographic.html
//       ^^ Boil all transforms down to a single Matrix4x4
void GameObject_drawMesh(uint8_t* bitmap, struct GameObject* go, struct Camera* camera)
{
	if (camera == NULL) return; // Camera required

	// TODO: Add rasterization mode, cull faces, etc.
	struct Mesh* m = go->mesh;

	// Aggregate the translation of all our parents.
	/* TODO: Break all positional offsets off into a function that applies one
		GameObject's transforms on another GameObject. */
	//struct Vector3 agg_pos = { 0, 0, 0 };

	
	struct Vector3* projected_vertices = NULL;
	struct Vector3* screen_vertices = NULL;
	if (projected_vertices == NULL) {
		projected_vertices = pd->system->realloc(projected_vertices, sizeof(struct Vector3) * go->mesh->numVertices);
	}
	if (screen_vertices == NULL) {
		screen_vertices = pd->system->realloc(screen_vertices, sizeof(struct Vector3) * go->mesh->numVertices);
	}
	struct Triangle* tris = NULL;
	if (tris == NULL)
	{
		tris = pd->system->realloc(tris, sizeof(struct Triangle) * go->mesh->numIndices / 3);
	}

	int time_ms = pd->system->getCurrentTimeMilliseconds();

	for (int i = 0; i < go->mesh->numVertices; i++)
	{
		projected_vertices[i] = go->mesh->vertices[i];

		if (go->vertShader != 0x0) {
			go->vertShader(go, time_ms, &projected_vertices[i]);
		}

		// Origin Translate
		projected_vertices[i] = Vector3_subtract(&projected_vertices[i], &m->origin);

		// translate -> scale -> rotate
		projected_vertices[i] = Matrix4_apply(&go->transform, &projected_vertices[i]);
		
		// Z limiting
		// TODO: Add flag to object to turn this on; should only be used on terrain.
		/*if (projected_vertices[i].z > (camera->position.z - camera->near))
		{
			projected_vertices[i].z = camera->position.z - camera->near;
			projected_vertices[i].y = 0.75f;
		}*/

		// Parent Aggregate Translation
		// TODO: This needs to be expanded
		//projected_vertices[i] = Vector3_add(&projected_vertices[i], &agg_pos);

		screen_vertices[i] = projected_vertices[i];
		screen_vertices[i] = Vector3_subtract(&screen_vertices[i], &camera->position);
		screen_vertices[i] = Matrix3_apply(&camera->rotate_transform, &screen_vertices[i]);
		/*screen_vertices[i] = Matrix3_apply(&camera->rotationX, &screen_vertices[i]);
		screen_vertices[i] = Matrix3_apply(&camera->rotationY, &screen_vertices[i]);
		screen_vertices[i] = Matrix3_apply(&camera->rotationZ, &screen_vertices[i]);*/
		PTR_Camera_worldToScreenPos(camera, &screen_vertices[i]);
	}

	//struct GameObject* parent = go->parent;
	//while (parent != NULL)
	//{
	//	agg_pos = Vector3_add(&agg_pos, &parent->position);

	//	parent = parent->parent;
	//}

	struct Vector3 point[3] = { {0,0,0}, {0,0,0}, {0,0,0} };
	int i = 0, p = 0;
	int t_idx = 0;
	for (i = 0; i < m->numIndices; i += 3, t_idx++)
	{
		for (int ii = 0; ii < 3; ii++) point[ii] = projected_vertices[m->indices[i + ii]];

		struct Vector3 normal = pnormal(&point[0], &point[1], &point[2]);
		struct Vector3 center = {
			.x = (point[0].x + point[1].x + point[2].x) / 3.0f,
			.y = (point[0].y + point[1].y + point[2].y) / 3.0f,
			.z = (point[0].z + point[1].z + point[2].z) / 3.0f,
		};

		struct Vector3 line_to_camera = Vector3_subtract(&camera->position, &center);
		//struct Vector3 line_to_camera_n = Vector3_normalize(line_to_camera);

		//struct Vector3 forward = Vector3_getForward(&camera->rotation);
		

		// DRAW TRIANGLE (DEBUG)
		float backface = Vector3_dot(normal, line_to_camera);
		//if (backface < 0) continue; // SKIP RENDER IF BACK FACING AWAY
		//if (point[0].z > camera->position.z) continue;
		
		float worldZ0 = point[0].z;

		//Triangle_draw(pd, bitmap, &point[0], &point[1], &point[2], RENDER_WIREFRAME, kColorBlack, 1, 128);
		
		float NdotL = Vector3_dot(normal, camera->light_dir);
		float fog = 1;
		//float far_thresh = camera->position.z - camera->far_fog;
		//if (center.z < camera->position.z && center.z > far_thresh) {
		//	fog = 1 - (far_thresh - camera->position.z) / (far_thresh - center.z);
		//}
		//else if (center.z < 1) {
		//	//fog = 0.5f;
		//}
		
		/*if (point[0].z < camera->position.z + camera->far_fog)
		{
			fog = (camera->position.z + camera->far_fog) / (camera->position.z - camera->far);
		}*/
		float light = NdotL * fog;// MIN(NdotL, fog);
		int pattern_idx = MAX(MIN(light * 32, 32), 0); // clamp to 0-32

		//PTR_Camera_worldToScreenPos(camera, &center);

		//for (int ii = 0; ii < 3; ii++) {
		//	point[ii].z = 0;
		//	//if (point[ii].z < camera->position.z)// && point[ii].z < (camera->position.z - camera->near))
		//	//{
		//	//	point[ii].z = camera->near;
		//	//}
		//}

		// == Load cached screenspace-projected points ==
		for (int ii = 0; ii < 3; ii++) point[ii] = screen_vertices[m->indices[i + ii]];

		tris[t_idx] = (struct Triangle){
			.indices = {
				m->indices[i],
				m->indices[i + 1],
				m->indices[i + 2]
			},
			.center = center, // screenspace center
			.shade = pattern_idx,
			.visible = backface > 0 &&
				//point[0].z < (camera->position.z - camera->near) &&
				point[0].z > 0 && point[1].z > 0 && point[2].z > 0
				//&& worldZ0 > (camera->position.z - camera->far),
		};

		 //api_fillTriangle(bitmap, LCD_ROWSIZE, &point[0], &point[1], &point[2], &patterns[pattern_idx]);

		//pd->graphics->fillTriangle(point[0].x, point[0].y, point[1].x, point[1].y, point[2].x, point[2].y, kColorWhite);

		/*pd->graphics->drawLine(point[0].x, point[0].y, point[1].x, point[1].y, 1, kColorWhite);
		pd->graphics->drawLine(point[1].x, point[1].y, point[2].x, point[2].y, 1, kColorWhite);
		pd->graphics->drawLine(point[2].x, point[2].y, point[0].x, point[0].y, 1, kColorWhite);*/
		//}
	}

	qsort(tris, m->numIndices / 3, sizeof(struct Triangle), compare_zdepth);

	for (int i = 0; i < m->numIndices / 3; i++)
	{
		struct Triangle* tri = &tris[i];
		if (!tri->visible) continue;

		for (int ii = 0; ii < 3; ii++)
		{
			point[ii] = screen_vertices[tri->indices[ii]];
		}

		api_fillTriangle(bitmap, LCD_ROWSIZE, &point[0], &point[1], &point[2], &patterns[tri->shade]);
	}

	// Free the vertex projection memory
	pd->system->realloc(projected_vertices, 0);
	projected_vertices = NULL;

	pd->system->realloc(screen_vertices, 0);
	screen_vertices = NULL;

	pd->system->realloc(tris, 0);
	tris = NULL;
}

struct Vector3 grid_points[] = {
	{0, 0, 0},
	{0, 1.5f, 0}, // y is flipped
	{ -1, 0, 0},
	{ 1, 0, 0},
	{ 0, 0, -1},
	{ 0, 0, 1},
};

/* TODO: This isn't working because the camera is at 0,0,0
	and the grid should be drawn at y=0. Need to wait until
	the camera can sit at y=5 or something for this grid to be visible. */
void Grid_render(uint8_t* bitmap, struct Camera* camera)
{
	for (int i = 0; i < 6; i += 2)
	{
		Line_worldDraw(grid_points[i], grid_points[i + 1], 1, camera);
	}

	//struct Matrix4x4 transform = Matrix4_getTransform(
	//	camera->rotation.x, camera->rotation.y, camera->rotation.z,
	//	//0, 0, 0,
	//	-camera->position.x, -camera->position.y, -camera->position.z,
	//	1.0f, 1.0f, 1.0f
	//);

	// Grid dots
	float gridSize = 5.0f;
	float cellSize = 0.5f;
	struct Vector3 dot = { 0,0,0 };
	for (float x = -gridSize; x < gridSize - 1; x += cellSize)
	{
		for (float z = -gridSize; z < gridSize - 1; z += cellSize)
		{
			// TODO: Add "behind camera" check.
			// TODO: Could add that to the Line_worldDraw function directly
			//dot.x = x;
			//dot.z = z;
			//Line_worldDraw(pd, dot, dot, 1, camera);
			//dot = (struct Vector3){ x, 0, z };
			dot.x = x; dot.y = 0; dot.z = z;
			//dot = Matrix4_apply(&transform, &dot);

			dot = Vector3_subtract(&dot, &camera->position);

			//// Rotation
			//// TODO: replace this with a combined transform
			/*PTR_Matrix3_apply(&camera->rotationX, &dot);
			PTR_Matrix3_apply(&camera->rotationY, &dot);
			PTR_Matrix3_apply(&camera->rotationZ, &dot);*/
			PTR_Matrix3_apply(&camera->rotate_transform, &dot);

			dot = Camera_worldToScreenPos(camera, &dot);
			//PTR_Camera_worldToScreenPos(camera, &dot);

			pd->graphics->setPixel(dot.x, dot.y, kColorWhite);
		}
	}
}

void YPlane_render(uint8_t* bitmap, struct Camera* camera, float y_plane)
{
	for (int i = 0; i < 6; i += 2)
	{
		Line_worldDraw(grid_points[i], grid_points[i + 1], 1, camera);
	}

	// Plane lines
	float gridSizeX = 5.0f;
	float gridSizeZ = 5.0f;
	float cellSize = 1.0f;
	struct Vector3 dot = { 0,0,0 };
	for (float x = -gridSizeX; x < gridSizeX; x += cellSize)
	{
		for (float z = -gridSizeZ; z < gridSizeZ; z += cellSize)
		{
			// TODO: Add "behind camera" check.
			// TODO: Could add that to the Line_worldDraw function directly
			//dot.x = x;
			//dot.z = z;
			//Line_worldDraw(pd, dot, dot, 1, camera);
			//dot = (struct Vector3){ x, 0, z };
			dot.x = x; dot.y = y_plane; dot.z = z;
			//dot = Matrix4_apply(&transform, &dot);

			dot = Vector3_subtract(&dot, &camera->position);

			//// Rotation
			//// TODO: replace this with a combined transform
			/*PTR_Matrix3_apply(&camera->rotationX, &dot);
			PTR_Matrix3_apply(&camera->rotationY, &dot);
			PTR_Matrix3_apply(&camera->rotationZ, &dot);*/
			PTR_Matrix3_apply(&camera->rotate_transform, &dot);

			dot = Camera_worldToScreenPos(camera, &dot);
			//PTR_Camera_worldToScreenPos(camera, &dot);
			if (dot.z < 0) continue;

			pd->graphics->setPixel(dot.x, dot.y, kColorWhite);
		}
	}
}

void Line_worldDraw(struct Vector3 p1, struct Vector3 p2, float size, struct Camera* camera)
{
	struct Vector3 a = p1;
	struct Vector3 b = p2;

	// TODO: Precalculate this
	//struct Matrix4x4 transform = Matrix4_getTransform(
	//	camera->rotation.x, camera->rotation.y, camera->rotation.z,
	//	0,0,0,
	//	//camera->position.x, -camera->position.y, camera->position.z,
	//	1.0f, 1.0f, 1.0f
	//);

	a = Vector3_subtract(&a, &camera->position);
	b = Vector3_subtract(&b, &camera->position);

	//a = Matrix4_apply(&transform, &a);
	//b = Matrix4_apply(&transform, &b);

	////// Rotation
	////// TODO: replace this with a combined transform
	/*a = Matrix3_apply(&camera->rotationX, &a);
	a = Matrix3_apply(&camera->rotationY, &a);
	a = Matrix3_apply(&camera->rotationZ, &a);
	b = Matrix3_apply(&camera->rotationX, &b);
	b = Matrix3_apply(&camera->rotationY, &b);
	b = Matrix3_apply(&camera->rotationZ, &b);*/
	a = Matrix3_apply(&camera->rotate_transform, &a);
	b = Matrix3_apply(&camera->rotate_transform, &b);
	
	//a = Camera_worldToScreenPos(camera, &a);
	//b = Camera_worldToScreenPos(camera, &b);
	PTR_Camera_worldToScreenPos(camera, &a);
	PTR_Camera_worldToScreenPos(camera, &b);

	// Dots
	/*pd->graphics->setPixel(a.x, a.y, kColorBlack);
	pd->graphics->setPixel(b.x, b.y, kColorBlack);*/

	// Line
	pd->graphics->drawLine(a.x, a.y, b.x + size - 1, b.y + size - 1, size, kColorWhite);
}

//
//float sdSphere(struct Vector3 p, float s)
//{
//	float f = Vector3_length(&p) - s;
//	return f;
//}
//
//// https://iquilezles.org/articles/distfunctions/
//void Ray_renderTest(uint8_t* bitmap, struct Camera* camera, float lightStrength)
//{
//	struct Vector3 light_dir = Vector3_normalize((struct Vector3) { .9f, -.9f, 0.0f });
//	//pd->system->logToConsole("%f", light_dir.x);
//	for (int x = 0; x < 400; x++)
//	{
//		for (int y = 0; y < 240; y++)
//		{
//			struct Vector3 pos = { x - 200, y - 140, -1.0f };
//			float d = sdSphere(pos, lightStrength);
//			if (d < 0)
//			{
//				float sdfx = sdSphere((struct Vector3) { pos.x + 0.0001f, pos.y, 0 }, 20.0f);
//				float sdfy = sdSphere((struct Vector3) { pos.x, pos.y + 0.0001f, 0 }, 20.0f);
//				float sdfz = sdSphere((struct Vector3) { pos.x, pos.y, 0.0001f }, 20.0f);
//				struct Vector3 normal = Vector3_normalize((struct Vector3) { sdfx - d, sdfy - d, sdfz - d });
//				float NdotL = Vector3_dot(normal, light_dir);
//				//pd->system->logToConsole("%f", NdotL);
//
//				if (samplebluenoise(x, y) / 255.0f < abs(NdotL))
//				//if (NdotL > )
//					setpixel(bitmap, x, y, LCD_ROWSIZE);
//			}
//		}
//	}
//}

void sortTri(struct Vector3** p1, struct Vector3** p2, struct Vector3** p3)
{
	float y1 = (*p1)->y, y2 = (*p2)->y, y3 = (*p3)->y;

	if (y1 <= y2 && y1 < y3)
	{
		if (y3 < y2) // 1,3,2
		{
			struct Vector3* tmp = *p2;
			*p2 = *p3;
			*p3 = tmp;
		}
	}
	else if (y2 < y1 && y2 < y3)
	{
		struct Vector3* tmp = *p1;
		*p1 = *p2;

		if (y3 < y1) // 2,3,1
		{
			*p2 = *p3;
			*p3 = tmp;
		}
		else // 2,1,3
			*p2 = tmp;
	}
	else
	{
		struct Vector3* tmp = *p1;
		*p1 = *p3;

		if (y1 < y2) // 3,1,2
		{
			*p3 = *p2;
			*p2 = tmp;
		}
		else // 3,2,1
			*p3 = tmp;
	}

}

// == "Borrowed" from SDK 3D Project ==

void api_fillTriangle(uint8_t* bitmap, int rowstride, struct Vector3* p1, struct Vector3* p2, struct Vector3* p3, uint8_t pattern[8])
{
	// sort by y coord
	
	sortTri(&p1, &p2, &p3);
	
	int endy = MIN(LCD_ROWS, p3->y);
	
	if ( p1->y > LCD_ROWS || endy < 0 ) return;
	//return (LCDRowRange){ 0, 0 };

	int32_t x1 = p1->x * (1<<16);
	int32_t x2 = x1;
	
	int32_t sb = api_slope(p1->x, p1->y, p2->x, p2->y);
	int32_t sc = api_slope(p1->x, p1->y, p3->x, p3->y);

	int32_t dx1 = MIN(sb, sc);
	int32_t dx2 = MAX(sb, sc);
	
	api_fillRange(bitmap, rowstride, p1->y, MIN(LCD_ROWS, p2->y), &x1, dx1, &x2, dx2, pattern);
	
	int dx = api_slope(p2->x, p2->y, p3->x, p3->y);
	
	if ( sb < sc )
	{
		x1 = p2->x * (1<<16);
		api_fillRange(bitmap, rowstride, p2->y, endy, &x1, dx, &x2, dx2, pattern);
	}
	else
	{
		x2 = p2->x * (1<<16);
		api_fillRange(bitmap, rowstride, p2->y, endy, &x1, dx1, &x2, dx, pattern);
	}
	
	// return (LCDRowRange){ MAX(0, p1->y), endy };
}

void api_fillRange(uint8_t* bitmap, int rowstride, int y, int endy, int32_t* x1p, int32_t dx1, int32_t* x2p, int32_t dx2, uint8_t pattern[8])
{
	int32_t x1 = *x1p, x2 = *x2p;
	
	if ( endy < 0 )
	{
		int dy = endy - y;
		*x1p = x1 + dy * dx1;
		*x2p = x2 + dy * dx2;
		return;
	}
	
	if ( y < 0 )
	{
		x1 += -y * dx1;
		x2 += -y * dx2;
		y = 0;
	}
	
	while ( y < endy )
	{
		uint8_t p = pattern[y%8];
		uint32_t color = (p<<24) | (p<<16) | (p<<8) | p;
		
		api_drawFragment((uint32_t*)&bitmap[y*rowstride], (x1>>16), (x2>>16)+1, color);
		
		x1 += dx1;
		x2 += dx2;
		++y;
	}
	
	*x1p = x1;
	*x2p = x2;
}


void api_drawFragment(uint32_t* row, int x1, int x2, uint32_t color)
{
	if ( x2 < 0 || x1 >= LCD_COLUMNS )
		return;
	
	if ( x1 < 0 )
		x1 = 0;
	
	if ( x2 > LCD_COLUMNS )
		x2 = LCD_COLUMNS;
	
	if ( x1 > x2 )
		return;
	
	// Operate on 32 bits at a time
	
	int startbit = x1 % 32;
	uint32_t startmask = api_swap((1 << (32 - startbit)) - 1);
	int endbit = x2 % 32;
	uint32_t endmask = api_swap(((1 << endbit) - 1) << (32 - endbit));
	
	int col = x1 / 32;
	uint32_t* p = row + col;

	if ( col == x2 / 32 )
	{
		uint32_t mask = 0;
		
		if ( startbit > 0 && endbit > 0 )
			mask = startmask & endmask;
		else if ( startbit > 0 )
			mask = startmask;
		else if ( endbit > 0 )
			mask = endmask;
		
		api_drawMaskPattern(p, mask, color);
	}
	else
	{
		int x = x1;
		
		if ( startbit > 0 )
		{
			api_drawMaskPattern(p++, startmask, color);
			x += (32 - startbit);
		}
		
		while ( x + 32 <= x2 )
		{
			api_drawMaskPattern(p++, 0xffffffff, color);
			x += 32;
		}
		
		if ( endbit > 0 )
			api_drawMaskPattern(p, endmask, color);
	}
}

void api_drawMaskPattern(uint32_t* p, uint32_t mask, uint32_t color)
{
	if (mask == 0xffffffff)
		*p = color;
	else
		*p = (*p & ~mask) | (color & mask);
}


int32_t api_slope(float x1, float y1, float x2, float y2)
{
	float dx = x2-x1;
	float dy = y2-y1;
	
	if ( dy < 1 )
		return dx * (1<<16);
	else
		return dx / dy * (1<<16);
}

uint32_t api_swap(uint32_t n)
{
#if TARGET_PLAYDATE
	//return __REV(n);
	uint32_t result;

	__asm volatile ("rev %0, %1" : "=l" (result) : "l" (n));
	return(result);
#else
	return ((n & 0xff000000) >> 24) | ((n & 0xff0000) >> 8) | ((n & 0xff00) << 8) | (n << 24);
#endif
}
