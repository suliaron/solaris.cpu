#ifndef BINARYFILEADAPTER_H_
#define BINARYFILEADAPTER_H_

#include <cstdint>
#include <list>
#include <string>
#include "Counter.h"
#include "SolarisType.h"
#include "StopWatch.h"

class Body;
class BodyData;
class Output;
class TwoBodyAffair;

class BinaryFileAdapter
{
public:
	BinaryFileAdapter(Output *output);

    bool    FileExists(const std::string& name);

	void	Log(std::string msg, bool printToScreen);
	void	LogStartParameters(int argc, char* argv[]);
	void	LogTimeSpan(std::string msg, time_t startTime);

	void	SavePhases(double time, int n, double *y, int *id, output_type_t type,  int removed);
	void	SavePhase(std::ofstream& writer, double *y, int *id, output_type_t type, int removed);

	void	SaveIntegrals(double time, int n, double *integrals, output_type_t type);

	void	SaveTwoBodyAffairs(std::list<TwoBodyAffair>& list, output_type_t type);
	void	SaveTwoBodyAffair(std::ofstream& writer, TwoBodyAffair& affair, output_type_t type);

	void	SaveBodyProperties(double time, std::list<Body *>& bodyList, output_type_t type);

	void	SaveConstantProperty(std::ofstream& writer, Body* body, output_type_t type);
	void	SaveConstantProperty(Body* body, output_type_t type);

	void	SaveVariableProperty(Body* body, double time, output_type_t type);
	void	SaveVariableProperty(std::ofstream& writer, Body* body, double time, output_type_t type);

	void	SaveCompositionProperty(std::ofstream& writer, Body* body, int propertyId, output_type_t type);

	void	SaveCollisionProperty(BodyData* bodyData, output_type_t type, int i, int j);

	void	SaveElapsedTimes(double time, Counter counter, StopWatch timer1, StopWatch timer2, output_type_t type);
	void	SaveElapsedTimes(double time, StopWatch timer, output_type_t type);
	void	SaveElapsedTimes(double time, StopWatch *timer, output_type_t type);

private:
	std::string _errMsg;
	Output		*output;
	static int	_propertyId;
	static int	_compositionId;
};

#endif
