#ifndef GLOBALS
#define GLOBALS

#include "symbols.h"

const struct Vector3 GLOBAL_UP = { 0,1,0 };
const struct Vector3 GLOBAL_ORIGIN = { 0,0,0 };
const struct Vector3 GLOBAL_UNIT = { 0,0,-1 };

PlaydateAPI* pd = NULL;
float DELTA_TIME = 0;
float TIME_SCALE = 1.f;

#endif // !GLOBALS
