#ifndef OUTPUT_H_
#define OUTPUT_H_

#include <string>
#include "SolarisType.h"

class Output {
public:
	Output();

	std::string GetPath(const std::string fileName);
	std::string GetFilenameWithoutExt(const std::string& path);
	std::string CombinePath(std::string dir, std::string filename);

	static std::string directory;
	static char directorySeparator;

	output_type_t outputType;

	std::string phases;
	std::string integrals;
	std::string constantProperties;
	std::string variableProperties;
	std::string compositionProperties;
	std::string twoBodyAffair;
	std::string log;
};

#endif