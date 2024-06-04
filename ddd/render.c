#include <stdint.h>

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
		GameObject_render(pd, bitmap, &go->children[i], camera);
	}
}

// TODO: https://webglfundamentals.org/webgl/lessons/webgl-3d-orthographic.html
//       ^^ Boil all transforms down to a single Matrix4x4
void GameObject_drawMesh(PlaydateAPI* pd, uint8_t* bitmap, struct GameObject* go, struct Camera* camera)
{
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

					// Note: Do any world-space offsets here before projecting with a camera.
					//point->z += 3;
			for (int p = 0; p < 3; p++)
			{
				// Origin Offset
				point[p] = Vector3_subtract(&point[p], &m->origin);

				// Scale
				point[p] = Vector3_multiply(&point[p], &go->scale);

				// Rotation
				// TODO: Combine these rotations
				// BUG: It's not rotating around parent offset.
				point[p] = Matrix3_apply(&go->rotationX, &point[p]);
				point[p] = Matrix3_apply(&go->rotationY, &point[p]);
				point[p] = Matrix3_apply(&go->rotationZ, &point[p]);

				// Parent Aggregate Translation
				point[p] = Vector3_add(&point[p], &agg_pos);

				// Translation
				point[p] = Vector3_add(&point[p], &go->position);

			}

			struct Vector3 normal = pnormal(&point[0], &point[1], &point[2]);
			float dot = Vector3_dot(normal, (struct Vector3) { -0.5f, -1.0f, 0.0f });
			float light = abs((1 - (dot + 1.0f) / 2.0f) * 256.0f);
			struct Vector3 center = {
				.x = (point[0].x + point[1].x + point[2].x) / 3.0f,
				.y = (point[0].y + point[1].y + point[2].y) / 3.0f,
				.z = (point[0].z + point[1].z + point[2].z) / 3.0f,
			};
			struct Vector3 line_to_camera = Vector3_subtract(&center, &camera->position);

			if (camera != NULL) { // projected
				for (int p = 0; p < 3; p++)
				{
					point[p] = Camera_worldToScreenPos(camera, &point[p]);
				}
			} // else screenspace

			// Calculate normals, lighting, culling, etc here.

			// Draw the triangle
			
			float backface = Vector3_dot(normal, line_to_camera);
			if (camera != NULL && backface < 0)
			{
				Triangle_draw(pd, bitmap, &point[0], &point[1], &point[2], camera->render_mode, kColorBlack, 1, light);
			}
		}
}

/* TODO: This isn't working because the camera is at 0,0,0
	and the grid should be drawn at y=0. Need to wait until
	the camera can sit at y=5 or something for this grid to be visible. */
void Grid_render(PlaydateAPI* pd, uint8_t* bitmap, struct Camera* camera)
{
	struct Vector3 point[] = {
		{0, 0, 0},
		{5, 0, 0},
		{0, 0, 0},
		{0, 5, 5}
		/*{ 0, -1, 0},
		{ 0, -1, 100},
		{ 2, -1, 0},
		{ 2, -1, 100},
		{ 4, -1, 0},
		{ 4, -1, 100},
		{ 6, -1, 0},
		{ 6, -1, 100},
		{ 8, -1, 0},
		{ 8, -1, 100},*/
	};

	for (int i = 0; i < 4; i += 2)
	{
		point[i] = Camera_worldToScreenPos(camera, &point[i]);
		point[i + 1] = Camera_worldToScreenPos(camera, &point[i + 1]);

		Line_draw(bitmap, &point[i], &point[i + 1], 1);
	}
}
