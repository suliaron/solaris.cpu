#include <cstdio>

#include "Settings.h"
#include "Output.h"

Settings::Settings() :
	frame_center(FRAME_CENTER_ASTRO),
	enableDistinctStartTimes(false),
	integrator(0),
	intgr_type(INTEGRATOR_TYPE_UNDEFINED),
	timeLine(0),
	collision(0),
	closeEncounter(0),
	weakCapture(0),
	ejection(0.0),
	hitCentrum(0.0)
{
}
