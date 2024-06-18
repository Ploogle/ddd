#include <stdint.h>

#include "gameobject.h"
#include "3dmath.h"
#include "render.h"
#include "pd_api.h"
#include "camera.h"
#include "bluenoise.h"

#define sign(x) (-(int)((unsigned int)((int)(x)) >> (sizeof(int) * CHAR_BIT - 1)))

void Line_draw(uint8_t* bitmap, struct Vector3* p1, struct Vector3* p2, int thick)
{
	// Sort the input points by x
	struct Vector3* x1;
	struct Vector3* y1;
	struct Vector3* x2;
	struct Vector3* y2;

	if (p1->x <= p2->x) { x1 = p1; y1 = p1; x2 = p2; y2 = p2; }
	else { x1 = p2; y1 = p2; x2 = p1; y2 = p1; }

	float x = x1->x,
		y = y1->y,
		dx = x2->x - x1->x,
		dy = y2->y - y1->y,
		segs = (float)dy / (float)dx; // Divide the y axis into segments

	if (dx < abs(dy)) {
		segs = (float)dx / (float) dy;
		if (p1->y < p2->y) { y2 = p2; y1 = p1; }
		for (y = y1->y; y <= y2->y; y++)
		{
			if (x > 0 && x < LCD_COLUMNS && y > 0 && y < LCD_ROWS) {
				setpixel(bitmap, (uint8_t)x, (uint8_t)y, LCD_ROWSIZE);
			}
			x += segs;
		}
	}
	else {
		for (; x <= x2->x; x++)
		{
			if (x > 0 && x < LCD_COLUMNS && y > 0 && y < LCD_ROWS) {
				setpixel(bitmap, (uint8_t)x, (uint8_t)y, LCD_ROWSIZE);
			}
			y += segs;
		}
	}
}

void Triangle_draw(PlaydateAPI* pd, uint8_t* bitmap, struct Vector3* a, struct Vector3* b, struct Vector3* c, RENDER_MODE mode, LCDSolidColor color, int line_width, float face_color)
{
	switch (mode)
	{
	case RENDER_WIREFRAME:
		pd->graphics->drawLine(a->x, a->y, b->x, b->y, line_width > 0 ? line_width : 1, color);
		pd->graphics->drawLine(b->x, b->y, c->x, c->y, line_width > 0 ? line_width : 1, color);
		pd->graphics->drawLine(c->x, c->y, a->x, a->y, line_width > 0 ? line_width : 1, color);
		break;

	case RENDER_SDKFILL:
		pd->graphics->fillTriangle(a->x, a->y, b->x, b->y, c->x, c->y, kColorBlack);

	case RENDER_FILL:
		struct Vector3* top = a;
		struct Vector3* bottom = b;
		//struct Vector3* middle = c; // vertical middle
		struct Vector3* left;
		struct Vector3* right;

		if (a->y >= b->y && a->y >= c->y) bottom = a;
		else if (b->y > a->y && b->y > c->y) bottom = b;
		else bottom = c;

		if (a->y <= b->y && a->y <= c->y) top = a;
		else if (b->y < a->y && b->y < c->y) top = b;
		else top = c;

		if (a->x >= b->x && a->x >= c->x) right = a;
		else if (b->x > a->x && b->x > c->x) right = b;
		else right = c;

		if (a->x <= b->x && a->x <= c->x) left = a;
		else if (b->x < a->x && b->x < c->x) left = b;
		else left = c;

		// Edge Function Method
		for (float x = MAX(left->x, 0); x < abs(right->x) && x < LCD_COLUMNS; x++) 
		{
			for (float y = MAX(top->y, 0); y < abs(bottom->y) && y < LCD_ROWS; y++) {
				//for (float y = 0; y < LCD_ROWS; y++) {

				float PAB = _edge(*a, *b, x, y);
				float PBC = _edge(*b, *c, x, y);
				float PCA = _edge(*c, *a, x, y);
				if (PAB < 0) break; // back face cull
		
				if (PAB >= 0 && PBC >= 0 && PCA >= 0)
				{
					//uint8_t blue = samplebluenoise((uint8_t)x, (uint8_t)y);
					//if (blue > face_color)
						setpixel(bitmap, (int)x, (int)y, LCD_ROWSIZE);
				}
			}
		}

		// Debug visualization, keep around for now
		/*struct Vector3 center = {
			.x = (a->x + b->x + c->x) / 3.0f,
			.y = (a->y + b->y + c->y) / 3.0f,
		};*/

		/*pd->graphics->drawLine(center.x, center.y, a->x, a->y, 3, kColorBlack);
		pd->graphics->drawLine(center.x, center.y, b->x, b->y, 2, kColorBlack);
		pd->graphics->drawLine(center.x, center.y, c->x, c->y, 1, kColorBlack);*/
		/*pd->graphics->drawLine(center.x, center.y, center.x, center.y - 100, 2, kColorBlack);
		pd->graphics->drawLine(center.x, center.y, center.x + 100, center.y, 2, kColorBlack);*/

		/*pd->graphics->drawLine(center.x, center.y, top->x, top->y, 3, kColorBlack);
		pd->graphics->drawLine(center.x, center.y, bottom->x, bottom->y, 1, kColorBlack);*/


		break;

	case RENDER_WPF: // Wireframe + Fill
		Triangle_draw(pd, bitmap, a, b, c, RENDER_SDKFILL, kColorBlack, 1, face_color);
		Triangle_draw(pd, bitmap, a, b, c, RENDER_WIREFRAME, kColorWhite, 1, face_color);
		break;
	}
}

float _edge(struct Vector3 a, struct Vector3 b, float px, float py)
{
	return (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x);
}

void GameObject_render(PlaydateAPI* pd, uint8_t* bitmap, struct GameObject* go, struct Camera* camera)
{
	/* 
		This will technically work for wireframe, but will need to be replaced
		with a proper z-sorted list of game objects once we're rasterizing.
	*/

	// Draw us
	GameObject_drawMesh(pd, bitmap, go, camera);

	// Draw our children
	for (int i = 0; i < go->num_children; i++)
	{
		//GameObject_render(pd, bitmap, &go->children[i], camera);
	}
}

// TODO: https://webglfundamentals.org/webgl/lessons/webgl-3d-orthographic.html
//       ^^ Boil all transforms down to a single Matrix4x4
void GameObject_drawMesh(PlaydateAPI* pd, uint8_t* bitmap, struct GameObject* go, struct Camera* camera)
{
	if (camera == NULL) return; // Camera required

	// TODO: Add rasterization mode, cull faces, etc.
	struct Mesh* m = go->mesh;

	// Aggregate the translation of all our parents.
	/* TODO: Break all positional offsets off into a function that applies one
		GameObject's transforms on another GameObject. */
	struct Vector3 agg_pos = { 0, 0, 0 };

	struct GameObject* parent = go->parent;
	while (parent != NULL)
	{
		agg_pos = Vector3_add(&agg_pos, &parent->position);

		parent = parent->parent;
	}

	for (int i = 0; i < m->numIndices; i += 3)
	{
		uint16_t index[] = { m->indices[i], m->indices[i + 1], m->indices[i + 2] };
		struct Vector3 point[] = {
			m->vertices[index[0]],
			m->vertices[index[1]],
			m->vertices[index[2]]
		}; /* Converting to array of non-pointers so we can mutate the values without
				modifying the originals*/

		//if (point[0].z < camera->near &&
		//	point[1].z < camera->near &&
		//	point[2].z < camera->near)
		//	continue;

		// Note: Do any world-space offsets here before projecting with a camera.
		//point->z += 3;
		struct Matrix4x4 transform = Matrix4_getTransform(
			go->rotation.x, go->rotation.y, go->rotation.z,
			go->position.x, go->position.y, go->position.z,
			go->scale.x, go->scale.y, go->scale.z
		);
		for (int p = 0; p < 3; p++)
		{

			/* OBJECT TRANSFORMS
			   ------ ----------
			   */

			// Origin Translate
			point[p] = Vector3_subtract(&point[p], &m->origin);

			// BUG: It's not rotating around parent offset.
			point[p] = Matrix4_apply(&transform, &point[p]); // translate -> scale -> rotate

			// Parent Aggregate Translation
			// TODO: This needs to be expanded
			point[p] = Vector3_add(&point[p], &agg_pos);

			//point[p].y *= -1;
		}



		struct Matrix4x4 camera_rotate_transform = Matrix4_getTransform(
			camera->rotation.x, camera->rotation.y, camera->rotation.z,
			0,0,0,
			//camera->position.x, -camera->position.y, camera->position.z,
			1.0f, 1.0f, 1.0f
		);
		struct Vector3 center_raw = {
			.x = (point[0].x + point[1].x + point[2].x) / 3.0f,
			.y = (point[0].y + point[1].y + point[2].y) / 3.0f,
			.z = (point[0].z + point[1].z + point[2].z) / 3.0f,
		};

		struct Vector3 cpos = camera->position;
		cpos.y *= -1;
		//for (int p = 0; p < 3; p++)
		//{
		//	/* CAMERA TRANSFORMS
		//	   ------ ---------- */
		//	   //point[p] = Vector3_add(&point[p], &camera->position);
		//	point[p] = Vector3_subtract(&point[p], &cpos);
		//	point[p] = Matrix4_apply(&camera_rotate_transform, &point[p]);

		//	//   // Translation

		//	//// Rotation
		//	//point[p] = Matrix3_apply(&camera->rotationX, &point[p]);
		//	//point[p] = Matrix3_apply(&camera->rotationY, &point[p]);
		//	//point[p] = Matrix3_apply(&camera->rotationZ, &point[p]);
		//}

		struct Vector3 normal = pnormal(&point[0], &point[1], &point[2]);
		struct Vector3 center = {
			.x = (point[0].x + point[1].x + point[2].x) / 3.0f,
			.y = (point[0].y + point[1].y + point[2].y) / 3.0f,
			.z = (point[0].z + point[1].z + point[2].z) / 3.0f,
		};
		struct Vector3 zero = { 0,0,0 };
		struct Vector3 fake_camera = { 0, 1.0f, 2.5f };
		struct Vector3 corrected_camera = Camera_worldPosition(camera);
		//corrected_camera.y *= -1;
		struct Vector3 scaled_normal = Vector3_multiplyScalar(&normal, 0.5f); 
		struct Vector3 normal_end = Vector3_add(&center, &scaled_normal);
		//struct Vector3 line_to_camera = Vector3_subtract(&fake_camera, &center);
		struct Vector3 line_to_camera = Vector3_subtract(&camera->position, &center);
		struct Vector3 line_to_camera_n = Vector3_normalize(line_to_camera);
		struct Vector3 line_to_camera_scaled = Vector3_multiplyScalar(&line_to_camera_n, 0.5f);
		struct Vector3 line_to_fake_camera = Vector3_subtract(&zero, &fake_camera);
		struct Vector3 camera_line_end = Vector3_add(&center, &line_to_camera);

		struct Vector3 camera_ground = Vector3_multiplyScalar(&camera->position, 0.5f);
		camera_ground.y = 0;
		struct Vector3 forward = Vector3_getForward(&camera->rotation);
		
		// line to camera
		//Line_worldDraw(pd, center, camera_line_end, 1, camera);

		// normals
		//Line_worldDraw(pd, center, normal_end, 2, camera);

		// center dot
		//Line_worldDraw(pd, center_raw, center_raw, 5, camera);


		// DRAW TRIANGLE (DEBUG)
		float backface = Vector3_dot(normal, line_to_camera_n);
		if (backface > 0) {
			Line_worldDraw(pd, point[0], point[1], 1, camera);
			Line_worldDraw(pd, point[1], point[2], 1, camera);
			Line_worldDraw(pd, point[2], point[0], 1, camera);
		}
	}
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
void Grid_render(PlaydateAPI* pd, uint8_t* bitmap, struct Camera* camera)
{
	for (int i = 0; i < 6; i += 2)
	{
		Line_worldDraw(pd, grid_points[i], grid_points[i + 1], 1, camera);
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
			/*dot = Matrix3_apply(&camera->rotationX, &dot);
			dot = Matrix3_apply(&camera->rotationY, &dot);
			dot = Matrix3_apply(&camera->rotationZ, &dot);*/
			PTR_Matrix3_apply(&camera->rotationX, &dot);
			PTR_Matrix3_apply(&camera->rotationY, &dot);
			PTR_Matrix3_apply(&camera->rotationZ, &dot);

			dot = Camera_worldToScreenPos(camera, &dot);
			//PTR_Camera_worldToScreenPos(camera, &dot);


			//pd->graphics->drawLine(dot.x, dot.y, dot.x, dot.y, 1.0f, kColorBlack);
			pd->graphics->setPixel(dot.x, dot.y, kColorBlack);
		}
	}
}

void Line_worldDraw(PlaydateAPI* pd, struct Vector3 p1, struct Vector3 p2, float size, struct Camera* camera)
{
	struct Vector3 a = p1;
	struct Vector3 b = p2;


	struct Matrix4x4 transform = Matrix4_getTransform(
		camera->rotation.x, camera->rotation.y, camera->rotation.z,
		0,0,0,
		//camera->position.x, -camera->position.y, camera->position.z,
		1.0f, 1.0f, 1.0f
	);

	a = Vector3_subtract(&a, &camera->position);
	b = Vector3_subtract(&b, &camera->position);

	//a = Matrix4_apply(&transform, &a);
	//b = Matrix4_apply(&transform, &b);

	////// Rotation
	////// TODO: replace this with a combined transform
	a = Matrix3_apply(&camera->rotationX, &a);
	a = Matrix3_apply(&camera->rotationY, &a);
	a = Matrix3_apply(&camera->rotationZ, &a);
	b = Matrix3_apply(&camera->rotationX, &b);
	b = Matrix3_apply(&camera->rotationY, &b);
	b = Matrix3_apply(&camera->rotationZ, &b);
	
	//a = Camera_worldToScreenPos(camera, &a);
	//b = Camera_worldToScreenPos(camera, &b);
	PTR_Camera_worldToScreenPos(camera, &a);
	PTR_Camera_worldToScreenPos(camera, &b);


	pd->graphics->drawLine(a.x, a.y, b.x + size, b.y + size, size, kColorBlack);
}

//
//float sdSphere(struct Vector3 p, float s)
//{
//	float f = Vector3_length(&p) - s;
//	return f;
//}
//
//// https://iquilezles.org/articles/distfunctions/
//void Ray_renderTest(PlaydateAPI* pd, uint8_t* bitmap, struct Camera* camera, float lightStrength)
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