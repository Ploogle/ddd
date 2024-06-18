#include "symbols.h"

#ifndef render_h
#define render_h

//#include "ddd.h"
#include "pd_api.h"
#include "camera.h"

#define LCD_ROWS 240//-30 // for UX blocking
#define LCD_COLUMNS 400//-30

#define inbounds(x,y,w,h) (x>0&&x<w&&y>0&&y<h)

// Determine pixel at x, y is black or white.
#define samplepixel(data, x, y, rowbytes) (((data[(y)*rowbytes+(x)/8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0) ? kColorWhite : kColorBlack)

#define getpixel(data, x, y, rowbytes) (data[(y)*rowbytes+(x)/8] & (1 << (uint8_t)(7 - ((x) % 8))))

// Set the pixel at x, y to black.
#define setpixel(data, x, y, rowbytes) (data[(y)*rowbytes+(x)/8] &= ~(1 << (uint8_t)(7 - ((x) % 8))))

// Set the pixel at x, y to white.
#define clearpixel(data, x, y, rowbytes) (data[(y)*rowbytes+(x)/8] |= (1 << (uint8_t)(7 - ((x) % 8))))

// Set the pixel at x, y to the specified color.
#define drawpixel(data, x, y, rowbytes, color) (((color) == kColorBlack) ? setpixel((data), (x), (y), (rowbytes)) : clearpixel((data), (x), (y), (rowbytes)))


//LCDRowRange drawLine(uint8_t* bitmap, int rowstride, Vector3* p1, Vector3* p2, int thick);
void Line_draw(uint8_t* bitmap, struct Vector3* p1, struct Vector3* p2, int thick);
void Line_worldDraw(PlaydateAPI* pd, struct Vector3 p1, struct Vector3 p2, float size, struct Camera* camera);

void Triangle_draw(PlaydateAPI* pd, uint8_t* bitmap, struct Vector3* a, struct Vector3* b, struct Vector3* c, RENDER_MODE mode, LCDSolidColor color, int line_width, float face_color);

float _edge(struct Vector3 a, struct Vector3 b, float px, float py);

void GameObject_drawMesh(PlaydateAPI* pd, uint8_t* bitmap, struct GameObject* go, struct Camera* camera);


void GameObject_render(PlaydateAPI* pd, uint8_t* bitmap, struct GameObject* go, struct Camera* camera);

void Grid_render(PlaydateAPI* pd, uint8_t* bitmap, struct Camera* camera);
//
//void Ray_renderTest(PlaydateAPI* pd, uint8_t* bitmap, struct Camera* camera, float lightStrength);
//
//float sdSphere(struct Vector3 p, float s);

#endif 
