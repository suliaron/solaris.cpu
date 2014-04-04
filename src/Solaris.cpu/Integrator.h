#ifndef INTEGRATOR_H_
#define INTEGRATOR_H_

#include <string>

class BodyData;
class Acceleration;
class TimeLine;

class Integrator
{
public:
	Integrator();

	std::string	name;
	std::string	reference;
	double		accuracy;
	double		epsilon;

	virtual int Driver(BodyData *bodyData, Acceleration *acceleration, TimeLine *timeLine) = 0;
};

#endif
