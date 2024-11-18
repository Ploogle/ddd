#ifndef PD_STUB
#define PD_STUB

#include <math.h>
#include "pd_api.h"

extern PlaydateAPI* pd;


// Returns the number of "ticks" — whose size is defined by the value of _ticksPerRevolution_ passed in to the function  — the crank has turned through since the last time this function was called.Ticks can be positive or negative, depending upon the direction of rotation.If the crank turns through more than one tick in - between update cycles, a value of 2 or more could be returned.
// An example : say you have a movie player, and you want your movie to advance 6 frames for every one revolution of the crank.Calling `playdate.getCrankTicks(6)` during each update will cause you to get a return value of 1 as the crank turns past each 60 degree increment(since we passed in a 6, each tick represents 360 / 6 = 60 degrees.) So `getCrankTicks(6)` will return a 1 as the crank turns past the 0 degree absolute position, the 60 degree absolute position, and so on for 120, 180, 240, and 300 degree positions.Otherwise, 0 will be returned.

float TICK_LASTCRANKREADING = 0;
void playdate_resetCrankTicks() {
	TICK_LASTCRANKREADING = 0;
}

int playdate_getCrankTicks(int ticksPerRotation)
{

	int totalSegments = ticksPerRotation;
	int degreesPerSegment = 360 / ticksPerRotation;

	float thisCrankReading = pd->system->getCrankAngle();
	if (TICK_LASTCRANKREADING == 0)
	{
		TICK_LASTCRANKREADING = thisCrankReading;
	}

	// if it seems we've gone more than halfway around the circle, that probably means we're seeing :
	// 1) a reversal in direction, not that the player is really cranking that fast. (a good assumption if fps is 20 or higher; maybe not as good if we're at 2 fps or similar.) 
	// 2) a crossing of the 359->0 border, which gives the appearance of a massive crank change, but is really very small.
	// both these cases can be treated identically.
	float difference = thisCrankReading - TICK_LASTCRANKREADING;
	if (difference > 180 || difference < -180)
	{
		if (TICK_LASTCRANKREADING >= 180)
		{
			// move tick_lastCrankReading back 360 degrees so it's < 0. It's the same location, just it is unequivocally lower than thisCrankReading
			TICK_LASTCRANKREADING -= 360;
		}
		else
		{
			//move tick_lastCrankReading ahead 360 degrees so it's > 0. It's the same location, just now it is unequivocally greater than thisCrankReading.
			TICK_LASTCRANKREADING += 360;
		}
	}

	// which segment is thisCrankReading in ?
	int thisSegment = ceil(thisCrankReading / degreesPerSegment);
	int lastSegment = ceil(TICK_LASTCRANKREADING / degreesPerSegment);

	int segmentBoundariesCrossed = thisSegment - lastSegment;

	// save off value
	TICK_LASTCRANKREADING = thisCrankReading;

	return segmentBoundariesCrossed;
}


#endif // !PD_STUB
