#ifndef DORMAND_PRINCE_H_
#define DORMAND_PRINCE_H_

#include <string>
#include "Integrator.h"

class Acceleration;
class BodyData;
class TimeLine;

class DormandPrince : public Integrator
{
public:
	DormandPrince();

	int			Driver(BodyData *bodyData, Acceleration *acceleration, TimeLine *timeLine);
	int 		Step(  BodyData *bodyData, Acceleration *acceleration);
	int 		Step2( BodyData *bodyData, Acceleration *acceleration);
	double		GetErrorMax(int n, const double *yerr);

private:
	int		maxIter;
	int		sizeHeightRKD;
	double	b[9];
	double	bh[9];
	double	bdh[9];
	double	c[9];
	double	a[9][8];
};

#endif
