#include <stdlib.h>

#include "symbols.h"
#include "3dmath.h"

struct Vector3 Vector3_make(float x, float y, float z)
{
	return (struct Vector3) { .x = x, .y = y, .z = z };
}

//void Tri_make(struct Vector3* p1, struct Vector3* p2, struct Vector3* p3, struct Tri* out)
//{
//	*out = (struct Tri){ .p = {p1, p2, p3} };
//}

//void Cube_make(float size, struct Mesh* out)
//{
//	float cube_size = size == 0 ? 1 : size;
//
//	
//
//	/*struct Tri south, south2,
//		east, east2,
//		north, north2,
//		west, west2,
//		top, top2,
//		bottom, bottom2;
//
//	Tri_make(&p1, &p2, &p3, &south);
//	Tri_make(&p1, &p3, &p4, &south2);
//	Tri_make(&p4, &p3, &p6, &east);
//	Tri_make(&p4, &p6, &p5, &east2);
//	Tri_make(&p5, &p6, &p7, &north);
//	Tri_make(&p5, &p7, &p8, &north2);
//	Tri_make(&p8, &p7, &p2, &west);
//	Tri_make(&p8, &p2, &p1, &west2);
//	Tri_make(&p2, &p7, &p6, &top);
//	Tri_make(&p2, &p6, &p3, &top2);
//	Tri_make(&p5, &p8, &p1, &bottom);
//	Tri_make(&p5, &p1, &p4, &bottom2);
//
//	struct Tri* faces[12] = {&south, &south2, &east, &east2, &north, &north2, &west, &west2, &top, &top2, &bottom, &bottom2};
//	*out = (struct Mesh){.f = faces, .length = 12};*/
//};

//void Mesh_make(struct Mesh* out) {
//	return;
//}