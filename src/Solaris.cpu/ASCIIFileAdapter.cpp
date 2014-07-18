#include <iostream>
#include <fstream>
#include <algorithm>

#include "Error.h"
#include "FargoParameters.h"
#include "Tokenizer.h"
#include "Tools.h"
#include "Common.h"
/*
FargoParameters::FargoParameters()
{
	aspectRatio			= 0.05;            // Thickness over Radius in the disc
	sigma0				= 1.45315e-5;      // Surface Density at r=1
	alphaViscosity		= 0.001;           // Uniform kinematic viscosity
	sigmaSlope          = 0.5;             // Slope of surface density profile.
	flaringIndex        = 0.0;
	excludeHill			= "YES";

// Planet parameters

	planetConfig        = "./planets.cfg";
	thicknessSmoothing  = 0.6;             // Smoothing parameters in disk thickness

// Numerical method parameters

	transport           = "FARGO";
	innerBoundary       = "STOCKHOLM";     // choose : OPEN or RIGID or NONREFLECTING
	disk                = "YES"; 
	omegaFrame          = 0;
	frame               = "COROTATING";
	indirectTerm        = "YES";

// Mesh parameters

	nRad                = 256;             // Radial number of zones
	nSec                = 512;             // Azimuthal number of zones (sectors)
	rMin                = 0.2;             // Inner boundary radius
	rMax                = 15.0;            // Outer boundary radius
	radialSpacing       = "Logarithmic";   // Zone interfaces evenly spaced

// Output control parameters

	nTot                = 800000;          // Total number of time steps
	nInterm             = 2000;            // Time steps between outputs
	dT                  = 0.314159;        // Time step length. 2PI = 1 orbit
	outputDir           = "./";

// Viscosity damping due to a dead zone

	viscModR1           = 0.0;             // Inner radius of dead zone
	viscModDeltaR1      = 0.0;             // Width of viscosity transition at inner radius
	viscModR2           = 0.0;             // Outer radius of dead zone
	viscModDeltaR2      = 0.0;             // Width of viscosity transition at outer radius
	viscMod             = 0.0;             // Viscosity damp
}*/
std::string config;

int ReadConfigFile(std::string& path)
{
	std::ifstream file(path);
	if (file) {
		std::string str;
		while (std::getline(file, str))
		{
			if (str.length() == 0)
				continue;
			if (str[0] == '#')
				continue;
			config += str;
			config.push_back('\n');
			str.push_back('\n');
		} 	
	}
	else {
		std::cerr << "The file '" << path << "' could not opened!\r\nExiting to system!" << std::endl;
		exit(1);
	}
	file.close();

	return 0;
}

int SetParameter(std::string& key, std::string& value, settings_t& settings, const bool verbose)
{
	Trim(key);
	Trim(value);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (key == "enableDistinctStartTimes") {
		if (!Tools::StringToBool(value,&settings.enableDistinctStartTimes)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    } 
    else if (key == "barycentric") {
		if (value == "astro") {
			settings.frame_center = FRAME_CENTER_ASTRO;
		}
		else if (value == "bary") {
			settings.frame_center = FRAME_CENTER_BARY;
		}
		else {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "Output_Phases") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Output_Phases = atof(value.c_str());
    }
    else if (key == "Output_ConstantProperties") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Output_ConstantProperties = atof(value.c_str());
    }
    else if (key == "Output_VariableProperties") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Output_VariableProperties = atof(value.c_str());
    }
    else if (key == "Output_CompositionProperties") {
		Output_CompositionProperties = value;
    }
    else if (key == "Output_TwoBodyAffair") {
		Output_TwoBodyAffair = value;
    }
    else if (key == "Output_Integrals") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Output_Integrals = atof(value.c_str());
    }
    else if (key == "Output_Log") {
		Output_Log = value;
    }
    else if (key == "Integrator_name") {
		Integrator_name = value;
    }
    else if (key == "Integrator_Accuracy_value") {
		Integrator_Accuracy_value = value;
    }
    else if (key == "TimeLine_start") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		TimeLine_start = atof(value.c_str());
    }
    else if (key == "TimeLine_length") {
		TimeLine_length = value;
    }
    else if (key == "TimeLine_output") {
		TimeLine_output = value;
    }
    else if (key == "TimeLine_unit") {
		TimeLine_unit = atoi(value.c_str());
    }
    else if (key == "Ejection_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Ejection_value = atoi(value.c_str());
    }
    else if (key == "Ejection_unit") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Ejection_unit = atof(value.c_str());
    }
    else if (key == "HitCentrum_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		HitCentrum_value = atof(value.c_str());
    }
    else if (key == "HitCentrum_unit") {
		HitCentrum_unit = value;
    }
    else if (key == "Collision_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Collision_factor = atoi(value.c_str());
    }
    else if (key == "Collision_stop") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		Collision_stop = atoi(value.c_str());
    }
    else if (key == "CloseEncounter_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		CloseEncounter_factor = atof(value.c_str());
    }
    else if (key == "CloseEncounter_stop") {
		CloseEncounter_stop = value;
    }
    else if (key == "WeakCapture_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		WeakCapture_factor = atof(value.c_str());
    }
    else if (key == "WeakCapture_stop") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		WeakCapture_stop = atof(value.c_str());
    }
    else {
        std::cerr << "Unrecoginzed key: '" << key << "'!" << std::endl;
		return 1;
    }

	if (verbose) {
		std::cout << key << " was assigned to " << value << std::endl;
	}

	return 0;
}

int	ParseConfig(const bool verbose)
{
	// instantiate Tokenizer classes
	Tokenizer fileTokenizer;
	Tokenizer lineTokenizer;
	std::string line;

	fileTokenizer.set(config, "\n");
	while ((line = fileTokenizer.next()) != "") {
		lineTokenizer.set(line, " = ");
		std::string token;
		int tokenCounter = 1;

		std::string key; 
		std::string value;
		while ((token = lineTokenizer.next()) != "" && tokenCounter <= 2) {

			if (tokenCounter == 1)
				key = token;
			else if (tokenCounter == 2)
				value = token;

			tokenCounter++;
		}
		if (tokenCounter > 2) {
			if (SetParameter(key, value, verbose) == 1)
				return 1;
		}
		else {
			Error::_errMsg = "Invalid key/value pair: " + line;
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}

	return 0;
}