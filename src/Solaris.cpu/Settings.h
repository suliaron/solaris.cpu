#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Integrator.h"
#include "Output.h"
#include "SolarisType.h"

class EventCondition;
class TimeLine;

class Settings
{
public:
	Settings();

	bool				enableDistinctStartTimes;
	frame_center_t		frame_center;
	Integrator			*integrator;
	integrator_type_t	intgr_type;

	Output			output;
	TimeLine		*timeLine;

	double			ejection;
	double			hitCentrum;
	EventCondition	*closeEncounter;
	EventCondition	*collision;
	EventCondition	*weakCapture;
};

#endif