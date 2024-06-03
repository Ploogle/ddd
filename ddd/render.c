#include <stdint.h>

#include "3dmath.h"
#include "render.h"
#include "pd_api.h"

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

void Mesh_draw(PlaydateAPI* pd, uint8_t* bitmap, struct Mesh* m, struct Camera* camera)
{
	// TODO: Add rasterization mode, cull faces, etc.

#pragma region Wireframe
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
			// Origin offset before rotation
			// TODO: Replace this with Vector3_add
			point[p].x -= m->origin.x;
			point[p].y -= m->origin.y;
			point[p].z -= m->origin.z;

			// Rotation
			// TODO: Combine these rotations
			point[p] = Matrix4_multiply(&point[p], &m->tRotationX);
			point[p] = Matrix4_multiply(&point[p], &m->tRotationY);
			point[p] = Matrix4_multiply(&point[p], &m->tRotationZ);

			// Translation
			point[p].x += m->position.x;
			point[p].y += m->position.y;
			point[p].z += m->position.z;
		}

		if (camera != NULL) { // projected
			for (int p = 0; p < 3; p++)
			{
				point[p] = Camera_worldToScreenPos(camera, &point[p]);
			}
		} // else screenspace

		// Calculate normals, lighting, culling, etc here.

		pd->graphics->drawLine(point[0].x, point[0].y, point[1].x, point[1].y, 1, kColorBlack);
		pd->graphics->drawLine(point[1].x, point[1].y, point[2].x, point[2].y, 1, kColorBlack);
		pd->graphics->drawLine(point[2].x, point[2].y, point[0].x, point[0].y, 1, kColorBlack);
	}
#pragma endregion
}