#include <ctime>
#include <string>
#include <iostream>

#include "ASCIIFileAdapter.cpp"
#include "BinaryFileAdapter.h"
#include "Error.h"
#include "FargoParameters.h"
#include "Nebula.h"
#include "Output.h"
#include "Simulation.h"
#include "Simulator.h"
#include "Settings.h"
#include "TimeLine.h"
#include "Tools.h"

/**
 * It will iterate over argv[] to get the parameters.
 *
 * @param argc contains the number of arguments passed to the program
 * @param argv is a one-dimensional array of strings, each string is one of the arguments that was passed to the program
 * @param directory the output directory where the output files will be stored. If the input file was given without any
 * directory, than the current directory is used
 * @param fileName the file name of the input file
 * @return 0 on success 1 on error
 */
int ProcessArgv(int argc, char* argv[], std::string &directory, std::string &fileNameSettings, std::string &fileNameBodyGroupList, std::string &fileNameNebula, std::string &runType)
{
	if (argc < 2) { // Check the value of argc.
        Error::_errMsg = Constants::Usage;
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        if (     strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			std::cout << Constants::CodeName << ":" << std::endl << Constants::Usage;
			exit(0);
		}
		else if (strcmp(argv[i], "-iDir") == 0) {
			i++;
			directory = argv[i];
		}
        else if (strcmp(argv[i], "-is") == 0) {
			i++;
			fileNameSettings  = argv[i];
        }
		else if (strcmp(argv[i], "-ib") == 0) {
			i++;
			fileNameBodyGroupList  = argv[i];
        }
		else if (strcmp(argv[i], "-in") == 0) {
			i++;
			fileNameNebula  = argv[i];
        }
        else if (strcmp(argv[i], "-c") == 0) {
            runType = "Continue";
			i++;
			directory = argv[i];
			i++;
			fileNameSettings  = argv[i];
			i++;
			fileNameBodyGroupList  = argv[i];
			i++;
			if (argv[i]) {
				fileNameNebula  = argv[i];	
			}
        }
		else {
			Error::_errMsg = "Invalid argument.\n" + Constants::Usage;
            return 1;
		}
	}

	// If the file name is empty
	if (fileNameSettings.length() == 0) {
		Error::_errMsg = "Missing settings file name.";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
        return 1;
	}
	else if (fileNameBodyGroupList.length() == 0) {
		Error::_errMsg = "Missing bodygrouplist file name.";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
        return 1;
	}
	
	// If the directory is empty, then use the current/working directory
	if (directory.length() == 0) {
		 int result = Tools::GetWorkingDirectory(directory);
		 if (result == 1){
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
            return 1;
		}
	}

	return 0;
}

/**
 * Loads the input data and stores it into the simulation object.
 *
 * @param inputPath the input path of the data file
 * @param simulation the object where the input data will be stored
 * @return 0 on success 1 on error
 */
/*
int LoadInput(char* inputPath, Simulation &simulation)
{
	XmlFileAdapter xml(inputPath);
	if (XmlFileAdapter::Load(inputPath, xml.doc) == 1) {
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}

	if (XmlFileAdapter::DeserializeSimulation(xml.doc, simulation) == 1) {
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}

	if (simulation.nebula != 0 && simulation.nebula->fargoPath.length() > 0) {
		simulation.fargoParameters = new FargoParameters();
		simulation.fargoParameters->ReadConfigFile(simulation.nebula->fargoPath);
		if (simulation.fargoParameters->ParseConfig(true) == 1) {
    		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
        }
	}

	return 0;
}
*/
int LoadInput(char* inputPathSettings, char* inputPathBodyGroupList, char* inputPathNebula, Simulation &simulation)
{
	std::string settings;
	if (ReadFile(inputPathSettings, settings) == 1) {
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	if (ParseSettings(simulation.settings, settings, true) == 1) {
    		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
    }
	std::string bodyGroupList;
	if (ReadFile(inputPathBodyGroupList, bodyGroupList) == 1) {
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	if (ParseBodyGroupList(simulation.bodyGroupList, bodyGroupList, true) == 1) {
    		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
    }
	/*std::string nebula;
	if (ReadFile(inputPathNebula, nebula) == 1) {
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	simulation.nebula = new Nebula();
	if (ParseNebula(simulation.nebula, nebula, true) == 1) {
    		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
    }*/

/*	if (XmlFileAdapter::DeserializeSimulation(xml.doc, simulation) == 1) {
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}

	if (simulation.nebula != 0 && simulation.nebula->fargoPath.length() > 0) {
		simulation.fargoParameters = new FargoParameters();
		simulation.fargoParameters->ReadConfigFile(simulation.nebula->fargoPath);
		if (simulation.fargoParameters->ParseConfig(true) == 1) {
    		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
        }
	}*/

	return 0;
}

/*
 * 2013.04.04: -i H:\Work\VSSolutions\Solaris\TestCases\JupiterSaturnWithJupiterTrojans\L4_T1e7\JupiterL4Trojans.xml
 * Goal: Test the Simulator.Synchronization() function
 */
int main(int argc, char* argv[])
{
	if (Tools::GetDirectorySeparator(&Output::directorySeparator) == 1) {
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}

	std::string		fileNameSettings;
    std::string		fileNameBodyGroupList;
	std::string		fileNameNebula;
	std::string     runType("New");
    if (ProcessArgv(argc, argv, Output::directory, fileNameSettings, fileNameBodyGroupList, fileNameNebula, runType) == 1) {
		Error::PrintStackTrace();
		exit(1);
	}

	char*	inputPathSettings = 0;
	Tools::CreatePath(Output::directory, fileNameSettings, Output::directorySeparator, &inputPathSettings);
	char*	inputPathBodyGroupList = 0;
	Tools::CreatePath(Output::directory, fileNameBodyGroupList, Output::directorySeparator, &inputPathBodyGroupList);
	char*	inputPathNebula = 0;
	if (0 < fileNameNebula.length())
	{
		Tools::CreatePath(Output::directory, fileNameNebula, Output::directorySeparator, &inputPathNebula);
	}

    Simulation  simulation(runType);
	if (LoadInput(inputPathSettings, inputPathBodyGroupList, inputPathNebula, simulation) == 1) {
		Error::PrintStackTrace();
		exit(1);
	}

    simulation.binary = new BinaryFileAdapter(&simulation.settings.output);
	simulation.binary->LogStartParameters(argc, argv);
	simulation.binary->Log("Simulation data was successfully loaded and deserialized.", true);

	if (simulation.CheckBodyGroupList() == 1) {
		Error::PrintStackTrace();
		exit(1);
	}
	simulation.binary->Log("BodyGroupList check succeeded.", true);

	if (simulation.Initialize() == 1) {
		Error::PrintStackTrace();
		exit(1);
	}
	simulation.binary->Log("Simulation was successfully initialized.", true);

	Simulator		simulator(&simulation);
    if (simulation.runType == "Continue" ) {
        if (simulator.Continue() == 1) {
		    Error::PrintStackTrace();
		    exit(1);
	    }
    }

	//if (simulator.Run() == 1) {
	//	Error::PrintStackTrace();
	//	exit(1);
	//}
	
	return 0;
}
