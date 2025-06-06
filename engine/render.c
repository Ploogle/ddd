#include <stdint.h>

#include <time.h>
#include <stdlib.h>

#include "actor.h"
#include "3dmath.h"
#include "render.h"
#include "pd_api.h"
#include "camera.h"
#include "bluenoise.h"
#include "wave_heightmap.h"

#define FOG_RADIUS 12.f

//typedef uint8_t Pattern[8];

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

LCDPattern grey50 = {
	// Bitmap
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,

	// Mask
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
};

int compare_zdepth(struct Triangle* a, struct Triangle* b)
{
	return a->center.z - b->center.z;
}


struct Vector3 projected_vertices[1024];
//struct Vector3 screen_vertices[1024];
struct Triangle tris[2048];
// TODO: https://webglfundamentals.org/webgl/lessons/webgl-3d-orthographic.html
//       ^^ Boil all transforms down to a single Matrix4x4
void Actor_drawMesh(uint8_t* bitmap, struct Actor* act, struct Camera* camera)
{
	if (camera == NULL) return; // Camera required

	struct Mesh* m = act->mesh;

	/*if (projected_vertices == NULL) {
		projected_vertices = pd->system->realloc(projected_vertices, sizeof(struct Vector3) * act->mesh->numVertices);
	}
	if (screen_vertices == NULL) {
		screen_vertices = pd->system->realloc(screen_vertices, sizeof(struct Vector3) * act->mesh->numVertices);
	}*/

	/*if (tris == NULL)
	{
		tris = pd->system->realloc(tris, sizeof(struct Triangle) * act->mesh->numIndices / 3);
	}*/

	int time_ms = pd->system->getCurrentTimeMilliseconds();
	struct Matrix4x4 act_transform = act->transform;
	struct Mesh act_mesh = *act->mesh;
	struct Vector3 m_origin = m->origin;
	struct Vector3 camera_actor_position = camera->actor->position;

	int x_range = camera_actor_position.x - (FOG_RADIUS);
	int z_range = camera_actor_position.z - (FOG_RADIUS);

	for (int i = 0; i < act->mesh->numVertices; i++)
	{
		projected_vertices[i] = act_mesh.vertices[i];

		if (act->vertShader != 0x0) {
			act->vertShader(act, time_ms, &projected_vertices[i]);
		}

		// Origin Translate
		projected_vertices[i] = Vector3_subtract(&projected_vertices[i], &m_origin);

		// Transform (translate -> scale -> rotate)
		PTR_Matrix4_apply(&act_transform, &projected_vertices[i]);
		//projected_vertices[i] = Matrix4_apply(&act_transform, &projected_vertices[i]);

		// Z limiting
		// TODO: Actual clipping?
		if (m->z_limit && projected_vertices[i].z > (camera_actor_position.z - camera->near))
		{
			projected_vertices[i].z = camera_actor_position.z - camera->near;
			projected_vertices[i].y = 0.75f;
		}

		/*projected_vertices[i] = Vector3_subtract(&projected_vertices[i], &camera->actor->position);
		projected_vertices[i] = Matrix3_apply(&camera->rotate_transform, &projected_vertices[i]);*/

		/*screen_vertices[i] = projected_vertices[i];
		PTR_Camera_worldToScreenPos(camera, &screen_vertices[i]);*/
	}



	struct Vector3 camera_light_dir = camera->light_dir;
	struct Vector3 point[3] = { {0,0,0}, {0,0,0}, {0,0,0} };
	int i = 0, p = 0;
	int t_idx = 0;
	struct Vector3 normal;
	struct Vector3 center;
	struct Vector3 line_to_camera;
	for (i = 0; i < m->numIndices; i += 3, t_idx++)
	{
		int triangle_idx = (float)i / 3.f;
		bool break_early = false;
		for (int ii = 0; ii < 3; ii++)
		{
			point[ii] = projected_vertices[m->indices[i + ii]];

			// If we're using fog, skip any geo that's definitely
			// outside the fog radius on the x and z axes
			if (act->use_fog)
			{
				if ((point[ii].x <= x_range || point[ii].x >= x_range + (FOG_RADIUS * 2)) ||
					(point[ii].z <= z_range || point[ii].z >= z_range + (FOG_RADIUS * 2)))
				{
					tris[t_idx] = (struct Triangle){ .visible = false };
					break_early = true;
				}
			}

		}
		if (break_early) continue;

		normal = pnormal(&point[0], &point[1], &point[2]);
		center = (struct Vector3){
			.x = (point[0].x + point[1].x + point[2].x) / 3.0f,
			.y = (point[0].y + point[1].y + point[2].y) / 3.0f,
			.z = (point[0].z + point[1].z + point[2].z) / 3.0f,
		};
		float worldZ0 = point[0].z;

		/*for (int ii = 0; ii < 3; ii++)
		{
		//screen_vertices[i] = projected_vertices[i];
			point[ii + i] = Vector3_subtract(&point[ii + i], &camera->actor->position);
			point[ii + i] = Matrix3_apply(&camera->rotate_transform, &point[ii + i]);
			PTR_Camera_worldToScreenPos(camera, &point[ii + i]);
		}*/

		 line_to_camera = Vector3_subtract(&camera_actor_position, &center);
		float c_dist = Vector3_length(&line_to_camera);
		float backface = Vector3_dot(normal, line_to_camera);

		float NdotL = Vector3_dot(normal, camera_light_dir);
		float fog = act->use_fog ? 1 - (c_dist / FOG_RADIUS) : 1; // 1;
		float light = NdotL * fog;
		float triangle_color = m->triangle_colors[triangle_idx];
		//light *= triangle_color;
		int pattern_idx = MAX(MIN(light * 32, 32), 0); // clamp to 0-32

		// Load cached screenspace-projected points
		//for (int ii = 0; ii < 3; ii++) point[ii] = screen_vertices[m->indices[i + ii]];

		tris[t_idx] = (struct Triangle){
			.indices = {
				m->indices[i],
				m->indices[i + 1],
				m->indices[i + 2]
			},
			.center = center, // screenspace center
			.shade = pattern_idx,
			.visible = backface > 0 && act->visible &&
				(act->skip_black_triangles ? pattern_idx > 0 : true)
			/*(point[0].x >= 0 || point[1].x >= 0 || point[2].x >= 0) &&
			(point[0].x <= 400 || point[1].x <= 400 || point[2].x <= 400) &&
			(point[0].y >= 0 || point[1].y >= 0 || point[2].y >= 0) &&
			(point[0].y <= 240 || point[1].y <= 240 || point[2].y <= 240) &&*/
			//point[0].z < (camera->position.z - camera->near) && // near clip
			 //point[0].z > 0 && point[1].z > 0 && point[2].z > 0
			//&& worldZ0 > (camera->position.z - camera->far), // far clip
		};
	}

	// Sort triangles by screen depth (z)
	qsort(tris, m->numIndices / 3, sizeof(struct Triangle), compare_zdepth);

	// Screen project the points
	for (int i = 0; i < act->mesh->numVertices; i++)
	{
		if (act->use_fog &&
			((projected_vertices[i].x <= x_range || projected_vertices[i].x >= x_range + (FOG_RADIUS * 2)) ||
				(projected_vertices[i].z <= z_range || projected_vertices[i].z >= z_range + (FOG_RADIUS * 2)))
			)
			continue;

		projected_vertices[i] = Vector3_subtract(&projected_vertices[i], &camera_actor_position);
		projected_vertices[i] = Matrix3_apply(&camera->rotate_transform, &projected_vertices[i]);
		PTR_Camera_worldToScreenPos(camera, &projected_vertices[i]);
	}

	for (int i = 0; i < m->numIndices / 3; i++)
	{
		struct Triangle* tri = &tris[i];
		if (!tri->visible) continue;

		bool render = true;
		for (int ii = 0; ii < 3; ii++)
		{
			point[ii] = projected_vertices[tri->indices[ii]];
			if (point[ii].z < 0) render = false;
		}
		if (!render) continue;

		api_fillTriangle(bitmap, LCD_ROWSIZE, &point[0], &point[1], &point[2], &patterns[tri->shade], tri->shade);
	}

	// Free the struct Vector3 projection memory
	/*pd->system->realloc(projected_vertices, 0);
	projected_vertices = NULL;

	pd->system->realloc(screen_vertices, 0);
	screen_vertices = NULL;

	pd->system->realloc(tris, 0);
	tris = NULL;*/
}

// THANKS @2DARRAY!!!
// p1 and p2 are positions in view/camera space.
// nearClip is the lowest legal z-value for visible geometry.
// returns 1 if we should draw this line, returns 0 if we should skip this line.
// this function might modify p1 or p2
int ApplyNearClipToLine(struct Vector3* p1, struct Vector3* p2, float nearClip) {
	float p1z = -p1->z, p2z = -p2->z;
	if (p1z > nearClip && p2z > nearClip) {
		// entire line is ahead of the near-clip plane - draw it all like normal
		return 1;
	}
	else if (p1z < nearClip && p2z < nearClip) {
		// entire line is behind the near-clip plane - skip it entirely
		return 0;
	}

	// if we made it here, both checks above failed - so our line crosses the near-clip plane!
	// find how far along the line it happens
	float t = (nearClip - p1z) / (p2z - p1z);

	// use our t value to create a Vector3 representing the point where the line crosses the near-plane
	struct Vector3 intersectPoint = {
	   -lerp(p1->x, p2->x, t),
	   -lerp(p1->y, p2->y, t),
	   -nearClip,
	};

	// replace whichever point was behind the near-clip plane with our new intersection-point
	if (p1z < nearClip) {
		*p1 = intersectPoint;
	}
	else {
		*p2 = intersectPoint;
	}

	// we've trimmed part of the line, but we DO want to draw the rest of it
	return 1;
}

struct Vector3 grid_points[] = {
	{0, 0, 0},
	{0, 1.5f, 0}, // y is flipped
	{ -1, 0, 0},
	{ 1, 0, 0},
	{ 0, 0, -1},
	{ 0, 0, 1},
};

void YPlane_render(uint8_t* bitmap, struct Camera* camera, float y_plane, bool show_axis)
{
	if (show_axis)
	{
		for (int i = 0; i < 6; i += 2)
		{
			Line_worldDraw(grid_points[i], grid_points[i + 1], 1, camera);
		}
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
			dot.x = x; dot.y = y_plane; dot.z = z;

			dot = Vector3_subtract(&dot, &camera->actor->position);

			// Rotation
			PTR_Matrix3_apply(&camera->rotate_transform, &dot);

			dot = Camera_worldToScreenPos(camera, &dot);
			if (dot.z < 0) continue;

			pd->graphics->setPixel(dot.x, dot.y, kColorWhite);
		}
	}
}

void YPlane_renderGrid(uint8_t* bitmap, struct Camera* camera, float y_plane)
{
	float cellSize = 1.5f;
	float gridSizeX = cellSize * 5;
	float gridSizeZ = cellSize * 5;
	struct Vector3 lineStart = { 0,0,0 };
	struct Vector3 lineEnd = { 0,0,0 };

	// Vertical lines
	//for (float x = -gridSizeX; x < gridSizeX; x += cellSize)
	//{
	//	lineStart.x = x; lineStart.y = y_plane; lineStart.z = -gridSizeZ * 2;
	//	lineEnd.x = x; lineEnd.y = y_plane; lineEnd.z = camera->actor->position.z;

	//	/** Cameraspace Calculations */
	//	lineStart = Vector3_subtract(&lineStart, &camera->actor->position);
	//	lineEnd = Vector3_subtract(&lineEnd, &camera->actor->position);

	//	PTR_Matrix3_apply(&camera->rotate_transform, &lineStart);
	//	PTR_Matrix3_apply(&camera->rotate_transform, &lineEnd);
	//	/** End Cameraspace*/

	//	ApplyNearClipToLine(&lineStart, &lineEnd, camera->near);

	//	PTR_Camera_worldToScreenPos(camera, &lineStart);
	//	PTR_Camera_worldToScreenPos(camera, &lineEnd);

	//	pd->graphics->drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 1, grey50);
	//}

	// Horizontal lines
	for (float z = camera->actor->position.z - .5f; z > -gridSizeZ * 2; z -= cellSize)
	{
		lineStart.x = -gridSizeX; lineStart.y = y_plane; lineStart.z = z;
		lineEnd.x = gridSizeX; lineEnd.y = y_plane; lineEnd.z = z;

		/** Cameraspace Calculations */
		lineStart = Vector3_subtract(&lineStart, &camera->actor->position);
		lineEnd = Vector3_subtract(&lineEnd, &camera->actor->position);

		PTR_Matrix3_apply(&camera->rotate_transform, &lineStart);
		PTR_Matrix3_apply(&camera->rotate_transform, &lineEnd);
		/** End Cameraspace*/

		ApplyNearClipToLine(&lineStart, &lineEnd, camera->near);

		lineStart = Camera_worldToScreenPos(camera, &lineStart);
		lineEnd = Camera_worldToScreenPos(camera, &lineEnd);

		pd->graphics->drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 1, grey50);
	}
}

// TODO: MOVE LakeWaves_render methods into scene_fishing.h
#define LAKE_SIZE_X 6.f
#define LAKE_SIZE_Z 8.f

#define LAKE_BOUNDS_X 4.f
#define LAKE_BOUNDS_Z 8.f
struct Line3 lake_bounds_lines[] = {
	{
		.a = {LAKE_BOUNDS_X, 0, -LAKE_BOUNDS_Z - 1.5f},
		.b = {LAKE_BOUNDS_X, 0, LAKE_BOUNDS_Z - 1.5f}
	},
	{
		.a = {-LAKE_BOUNDS_X, 0, -LAKE_BOUNDS_Z - 1.5f},
		.b = {-LAKE_BOUNDS_X, 0, LAKE_BOUNDS_Z - 1.5f}
	}
};
void LakeWaves_render(uint8_t* bitmap, struct Camera* camera)
{
	struct Vector3 lineStart = { 0,0,0 };
	struct Vector3 lineEnd = { 0,0,0 };

	for (int i = 0; i < 2; i++)
	{
		lineStart = lake_bounds_lines[i].a;
		lineEnd = lake_bounds_lines[i].b;

		/** Cameraspace Calculations */
		lineStart = Vector3_subtract(&lineStart, &camera->actor->position);
		lineEnd = Vector3_subtract(&lineEnd, &camera->actor->position);

		PTR_Matrix3_apply(&camera->rotate_transform, &lineStart);
		PTR_Matrix3_apply(&camera->rotate_transform, &lineEnd);
		/** End Cameraspace*/

		if (ApplyNearClipToLine(&lineStart, &lineEnd, camera->near))
		{
			PTR_Camera_worldToScreenPos(camera, &lineStart);
			PTR_Camera_worldToScreenPos(camera, &lineEnd);

			//pd->graphics->drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 5, patterns[16]);
		}
	}
}

#define WATER_X_SIZE 20.f // heightmap dimensions in worldspace
#define WATER_Z_SIZE 20.f

#define WAVE_SPACING 1.5 // distance the waves are apart on z axis
#define MAX_LINE_HEIGHT 4 // how thick the waves can get
#define MAX_SHADE 32 // out of 32
#define MAX_HEIGHT .2f // how tall the waves can be on y axis

#define P_INC .1f // how far apart the line segments are in a wave

float wave_z_offset = 0;
float wave_x_offset = 4.f;
float SLANT = 1.;
void LakeWaves_renderWaterHeight(uint8_t* bitmap, struct Camera* camera)
{
	WAVE_RIPPLE_V += DELTA_TIME / 20;
	wave_z_offset += DELTA_TIME * .5;
	if (wave_z_offset > WAVE_SPACING) {
		wave_z_offset -= WAVE_SPACING;

		// Flip offsets when we loop
		wave_x_offset *= -1;
		SLANT *= -1;
	}

	struct Vector3 pointa = { .x = LAKE_SIZE_X };
	struct Vector3 pointb = { .x = -LAKE_SIZE_X };
	struct Vector3 point = { 0 };
	struct Vector3 previous_point = { .y = -100 };
	float u, v;
	float last_height = 0;
	const float HEIGHT_THRESH = MAX_HEIGHT * .2f;
	int i = 0;
	for (float z = -LAKE_SIZE_Z; z < LAKE_SIZE_Z; z += WAVE_SPACING, i++)
	{
		pointa.z = z + SLANT * ((int)i % 2 == 0 ? 1 : -1);
		pointb.z = z - SLANT * ((int)i % 2 == 0 ? 1 : -1);

		// TODO: properly replace this with camera position
		if (z >= 5)
			continue;

		if (i % 2 == 0)
		{
			pointa.x = LAKE_SIZE_X + wave_x_offset;
			pointb.x = -LAKE_SIZE_X + wave_x_offset;
		}
		else 
		{
			pointa.x = LAKE_SIZE_X - wave_x_offset;
			pointb.x = -LAKE_SIZE_X - wave_x_offset;
		}

		previous_point.y = -100;

		for (float p = 0; p <= 1; p += P_INC) {

			point = Vector3_lerp(&pointa, &pointb, p);
			point.z += wave_z_offset;

			u = (point.x + WATER_X_SIZE) / (WATER_X_SIZE * 2.5f);
			v = (point.z - camera->actor->position.z) / (camera->actor->position.z - WATER_Z_SIZE);

			float height_sample = (sample_wave_heightmap((int)((u + WAVE_RIPPLE_U) * WAVE_HEIGHTMAP_COLUMNS), (int)(((v + WAVE_RIPPLE_V) * WAVE_HEIGHTMAP_ROWS))) / 128.f) * MAX_HEIGHT;
			height_sample = MAX_HEIGHT - height_sample; // invert!
			point.y = height_sample;

			float delta_perc = fabsf(height_sample - last_height) / MAX_HEIGHT;

			point = Vector3_subtract(&point, &camera->actor->position);

			// Rotation
			PTR_Matrix3_apply(&camera->rotate_transform, &point);

			PTR_Camera_worldToScreenPos(camera, &point);
			//point = Camera_worldToScreenPos(camera, &point);
			if (point.z < 0) // behind-camera cull
			{
				last_height = height_sample;
				//previous_point = point;
				continue;
			}

			if (previous_point.y > -100 && point.y > -100) // bad null check but shrug
			{
				float height_a_perc = (last_height - HEIGHT_THRESH) / (MAX_HEIGHT - HEIGHT_THRESH);
				float height_b_perc = (height_sample - HEIGHT_THRESH) / (MAX_HEIGHT - HEIGHT_THRESH);

				// Forcing line edges to height 0 by checking left / right bounds (assuming 0% - 100% range)
				if (p <= P_INC) height_a_perc = 0;
				else if (p >= 1 - P_INC) height_b_perc = 0;

				fillLine(bitmap, &previous_point, height_a_perc * MAX_LINE_HEIGHT, height_a_perc * MAX_SHADE, &point, height_b_perc * MAX_LINE_HEIGHT, height_b_perc * MAX_SHADE);
			}

			last_height = height_sample;
			previous_point = point;
		}
	}
}

void fillLine(uint8_t* bitmap, struct Vector3* line_a, float width_a, char shade_a, struct Vector3* line_b, float width_b, char shade_b)
{
	struct Vector3 a1 = *line_a;
	struct Vector3 a2 = *line_a;

	a1.y -= width_a;
	if (a1.y < 0) a1.y = 0;
	if (a2.y < 0) a2.y = 0;

	struct Vector3 b1 = *line_b;
	struct Vector3 b2 = *line_b;

	b1.y -= width_b;
	if (b1.y < 0) b1.y = 0;
	if (b2.y < 0) b2.y = 0;

	api_fillTriangle(bitmap, LCD_ROWSIZE, &a1, &b1, &a2, shade_a, shade_a);
	api_fillTriangle(bitmap, LCD_ROWSIZE, &a2, &b1, &b2, shade_b, shade_b);
}


void Line_worldDraw(struct Vector3 p1, struct Vector3 p2, float size, struct Camera* camera)
{
	struct Vector3 a = p1;
	struct Vector3 b = p2;

	a = Vector3_subtract(&a, &camera->actor->position);
	b = Vector3_subtract(&b, &camera->actor->position);

	// Rotation
	a = Matrix3_apply(&camera->rotate_transform, &a);
	b = Matrix3_apply(&camera->rotate_transform, &b);

	PTR_Camera_worldToScreenPos(camera, &a);
	PTR_Camera_worldToScreenPos(camera, &b);

	if (a.z < 0 || b.z < 0) return;

	// Line
	pd->graphics->drawLine(a.x, a.y, b.x + size - 1, b.y + size - 1, size, kColorWhite);
}

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
void api_fillTriangle(uint8_t* bitmap, int rowstride, struct Vector3* p1, struct Vector3* p2, struct Vector3* p3, uint8_t pattern[8], char shade)
{
	// sort by y coord
	sortTri(&p1, &p2, &p3);

	int endy = MIN(LCD_ROWS, p3->y);

	if (p1->y > LCD_ROWS || endy < 0) return;

	int32_t x1 = p1->x * (1 << 16);
	int32_t x2 = x1;

	int32_t sb = api_slope(p1->x, p1->y, p2->x, p2->y);
	int32_t sc = api_slope(p1->x, p1->y, p3->x, p3->y);

	int32_t dx1 = MIN(sb, sc);
	int32_t dx2 = MAX(sb, sc);

	api_fillRange(bitmap, rowstride, p1->y, MIN(LCD_ROWS, p2->y), &x1, dx1, &x2, dx2, pattern, shade);

	int dx = api_slope(p2->x, p2->y, p3->x, p3->y);

	if (sb < sc)
	{
		x1 = p2->x * (1 << 16);
		api_fillRange(bitmap, rowstride, p2->y, endy, &x1, dx, &x2, dx2, pattern, shade);
	}
	else
	{
		x2 = p2->x * (1 << 16);
		api_fillRange(bitmap, rowstride, p2->y, endy, &x1, dx1, &x2, dx, pattern, shade);
	}
}

void api_fillRange(uint8_t* bitmap, int rowstride, int y, int endy, int32_t* x1p, int32_t dx1, int32_t* x2p, int32_t dx2, uint8_t pattern[8], char shade)
{
	int32_t x1 = *x1p, x2 = *x2p;
	int starty = y;

	/*if ( endy < 0 )
	{
		int dy = endy - y;
		*x1p = x1 + dy * dx1;
		*x2p = x2 + dy * dx2;
		return;
	}*/

	if (y < 0)
	{
		x1 += -y * dx1;
		x2 += -y * dx2;
		y = 0;
	}

	float y_perc = 0;
	float y_inc = 1.f / (float)(endy - starty);
	if (endy >= RENDER_Y_END) endy = RENDER_Y_END - 1;
	while (y < endy)
	{
		uint8_t p = patterns[shade][y % 8];
		//uint8_t p = pattern[y%8];
		uint32_t color = (p << 24) | (p << 16) | (p << 8) | p;

		/*if (y_perc > .25f && y_perc < .75f)
			color = 0;*/

			//if (y > RENDER_Y_START && y < RENDER_Y_END)
			//{
		api_drawFragment((uint32_t*)&bitmap[y * rowstride], (x1 >> 16), (x2 >> 16) + 1, y, color);
		//}

		x1 += dx1;
		x2 += dx2;
		++y;
		y_perc += y_inc;
	}

	*x1p = x1;
	*x2p = x2;
}

void api_drawFragment(uint32_t* row, int x1, int x2, int y, uint32_t color)
{
	if (x1 < RENDER_X_START) x1 = RENDER_X_START;
	if (x2 > RENDER_X_END) x2 = RENDER_X_END;

	if (x2 < 0 || x1 >= LCD_COLUMNS)
		return;

	if (x1 < 0)
		x1 = 0;

	if (x2 > LCD_COLUMNS)
		x2 = LCD_COLUMNS;

	if (x1 > x2)
		return;

	// Operate on 32 bits at a time

	int startbit = x1 % 32;
	uint32_t startmask = api_swap((1 << (32 - startbit)) - 1);
	int endbit = x2 % 32;
	uint32_t endmask = api_swap(((1 << endbit) - 1) << (32 - endbit));

	int col = x1 / 32;
	uint32_t* p = row + col;

	float x_perc = 0;
	float x_inc = 1.f / ((float)(x2 - x1) / 34.f);
	/*uint8_t pat = patterns[0][y % 8];
	color = (pat << 24) | (pat << 16) | (pat << 8) | pat;*/

	if (col == x2 / 32)
	{
		uint32_t mask = 0;

		if (startbit > 0 && endbit > 0)
			mask = startmask & endmask;
		else if (startbit > 0)
			mask = startmask;
		else if (endbit > 0)
			mask = endmask;

		api_drawMaskPattern(p, mask, color);
	}
	else
	{
		int x = x1;

		if (startbit > 0)
		{
			api_drawMaskPattern(p++, startmask, color);
			x += (32 - startbit);
			x_perc += x_inc;
		}

		while (x + 32 <= x2)
		{
			api_drawMaskPattern(p++, 0xffffffff, color);
			x += 32;
			x_perc += x_inc;

			/*pat = patterns[(int)(x_perc * 32.f)][y % 8];
			color = (pat << 24) | (pat << 16) | (pat << 8) | pat;*/
		}

		if (endbit > 0)
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
	float dx = x2 - x1;
	float dy = y2 - y1;

	if (dy < 1)
		return dx * (1 << 16);
	else
		return dx / dy * (1 << 16);
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