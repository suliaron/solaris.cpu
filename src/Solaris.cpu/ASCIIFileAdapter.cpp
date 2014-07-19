#include <iostream>
#include <fstream>
#include <algorithm>

#include "Common.h"
#include "Error.h"
#include "FargoParameters.h"
#include "Settings.h"
#include "Simulation.h"
#include "TimeLine.h"
#include "Tokenizer.h"
#include "Tools.h"

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

int SetParameter(std::string& key, std::string& value, Settings& settings, const bool verbose)
{
	Trim(key);
	Trim(value);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (key == "enabledistinctstarttimes") {
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
    else if (key == "output_phases") {
		settings.output.phases = value;
    }
    else if (key == "output_constantproperties") {
		settings.output.constantProperties = value;
    }
    else if (key == "output_variableproperties") {
		settings.output.variableProperties = value;
    }
    else if (key == "output_compositionproperties") {
		settings.output.compositionProperties = value;
    }
    else if (key == "output_twobodyaffair") {
		settings.output.twoBodyAffair = value;
    }
    else if (key == "output_integrals") {
		settings.output.integrals = value;
    }
    else if (key == "output_log") {
		settings.output.log = value;
    }
    else if (key == "integrator_name") {
		if (value == "runge_kutta4") {
			settings.intgr_type = RUNGE_KUTTA4;
		}
		else if (value == "runge_kutta56") {
			settings.intgr_type = RUNGE_KUTTA56;
		}
		else if (value == "runge_kutta_fehlberg78") {
			settings.intgr_type = RUNGE_KUTTA_FEHLBERG78;
		}
		else if (value == "dormand_prince") {
			settings.intgr_type = DORMAND_PRINCE;
		}
		else {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "integrator_accuracy_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.integrator->accuracy = atof(value.c_str());
    }
    else if (key == "timeline_start") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.timeLine->start = atof(value.c_str());
    }
    else if (key == "timeline_length") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.timeLine->length = atof(value.c_str());
    }
    else if (key == "timeline_output") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.timeLine->output = atof(value.c_str());
    }
    else if (key == "TimeLine_unit") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.timeLine->start = atof(value.c_str());
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