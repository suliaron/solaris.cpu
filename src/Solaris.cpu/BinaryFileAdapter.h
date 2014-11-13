#ifndef BINARYFILEADAPTER_H_
#define BINARYFILEADAPTER_H_

#include <list>
#include <string>
#include "SolarisType.h"

class Body;
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

	void	SavePhases(double time, int n, double *y, int *id, output_type_t type, bool& phasenew, int removed);
	void	SavePhase(std::ofstream& writer, double *y, int *id, output_type_t type, int removed);

	void	SaveIntegrals(double time, int n, double *integrals, output_type_t type, bool& integralnew);

	void	SaveTwoBodyAffairs(std::list<TwoBodyAffair>& list, output_type_t type, bool& twobodyaffnew);
	void	SaveTwoBodyAffair(std::ofstream& writer, TwoBodyAffair& affair, output_type_t type);

	void	SaveBodyProperties(double time, std::list<Body *>& bodyList, output_type_t type, bool& constpropnew, bool& varpropnew);

	void	SaveConstantProperty(std::ofstream& writer, Body* body, output_type_t type);
	void	SaveConstantProperty(Body* body, output_type_t type, bool& constpropnew);

	void	SaveVariableProperty(Body* body, double time, output_type_t type, bool& varpropnew);
	void	SaveVariableProperty(std::ofstream& writer, Body* body, double time, output_type_t type, bool& comppropnew);

	void	SaveCompositionProperty(std::ofstream& writer, Body* body, int propertyId, output_type_t type);

	bool phasenew;
	bool integralnew;
	bool twobodyaffnew;
	bool constpropnew;
	bool varpropnew;

private:
	std::string _errMsg;
	Output		*output;
	static int	_propertyId;
	static int	_compositionId;
};

#endif
