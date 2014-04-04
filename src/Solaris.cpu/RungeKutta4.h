#ifndef RUNGEKUTTA4_H_
#define RUNGEKUTTA4_H_

#include <string>
#include "Integrator.h"

class Acceleration;
class BodyData;
class TimeLine;

class RungeKutta4 : public Integrator {
public:
	RungeKutta4();

	int			Driver(BodyData *bodyData, Acceleration *acceleration, TimeLine *timeLine);
	int 		Step(  BodyData *bodyData, Acceleration *acceleration);

};

#endif