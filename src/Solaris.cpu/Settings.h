#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "FrameCenter.h"
#include "Integrator.h"
#include "IntegratorType.h"
#include "Output.h"

class EventCondition;
class TimeLine;

class Settings
{
public:
	Settings();

	bool			enableDistinctStartTimes;
	FrameCenter		frame_center;
	Integrator		*integrator;
	IntegratorType	intgr_type;

	Output			output;
	TimeLine		*timeLine;

	double			ejection;
	double			hitCentrum;
	EventCondition	*closeEncounter;
	EventCondition	*collision;
	EventCondition	*weakCapture;
};

#endif