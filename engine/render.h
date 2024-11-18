
#ifndef render_h
#define render_h

//#include "ddd.h"
#include "symbols.h"
#include "pd_api.h"
#include "camera.h"

#define LCD_ROWS 240
#define LCD_COLUMNS 400

#define RENDER_X_START 0
#define RENDER_X_END 400
#define RENDER_Y_START 0
#define RENDER_Y_END 240

extern PlaydateAPI* pd;
extern float DELTA_TIME;

int compare_zdepth(struct Triangle* a, struct Triangle* b);
void sortTri(struct Vector3** p1, struct Vector3** p2, struct Vector3** p3);

void Actor_drawMesh(uint8_t* bitmap, struct Actor* act, struct Camera* camera);
void Line_worldDraw(struct Vector3 p1, struct Vector3 p2, float size, struct Camera* camera);
void YPlane_render(uint8_t* bitmap, struct Camera* camera, float y_plane, bool show_axis);

// == "Borrowed" from SDK 3D Project ==
void sortTri(struct Vector3** p1, struct Vector3** p2, struct Vector3** p3);
void api_fillTriangle(uint8_t* bitmap, int rowstride, struct Vector3* p1, struct Vector3* p2, struct Vector3* p3, uint8_t pattern[8], char shade);
void api_fillRange(uint8_t* bitmap, int rowstride, int y, int endy, int32_t* x1p, int32_t dx1, int32_t* x2p, int32_t dx2, uint8_t pattern[8], char shade);
void api_drawMaskPattern(uint32_t* p, uint32_t mask, uint32_t color);
int32_t api_slope(float x1, float y1, float x2, float y2);
void api_drawFragment(uint32_t* row, int x1, int x2, int y, uint32_t color);
uint32_t api_swap(uint32_t n);

int compare_zdepth(struct Triangle* a, struct Triangle* b);

#endif 
