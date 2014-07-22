#include <iostream>
#include <fstream>
#include <algorithm>

#include "Common.h"
#include "DormandPrince.h"
#include "Error.h"
#include "EventCondition.h"
#include "FargoParameters.h"
#include "RungeKutta4.h"
#include "RungeKutta56.h"
#include "RungeKuttaFehlberg78.h"
#include "Settings.h"
#include "Simulation.h"
#include "TimeLine.h"
#include "Tokenizer.h"
#include "Tools.h"

int ReadFile(char* path, std::string *str)
{
	std::ifstream file(path);
	if (file) {
		std::string temp;
		while (std::getline(file, temp))
		{
			if (temp.length() == 0)
				continue;
			if (temp[0] == '#')
				continue;
			*str += temp;
			str->push_back('\n');

		} 	
	}
	else {
		std::cerr << "The file '" << path << "' could not opened!\r\nExiting to system!" << std::endl;
		exit(1);
	}
	file.close();

	return 0;
}
/*
int SetSettings(std::string& key, std::string& value, Settings& settings, const bool verbose)
{
	TimeLine timeLine;
	EventCondition e;

	Tools::Trim(key);
	Tools::Trim(value);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (key == "enabledistinctstarttimes") {
		if (!Tools::StringToBool(value,&settings.enableDistinctStartTimes)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    } 
    else if (key == "barycentric") { //framecenter kellene névnek
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
		if (value == "rungekutta4" || value == "rk4") {
			settings.intgr_type = RUNGE_KUTTA4;
			settings.integrator = new RungeKutta4();
		}
		else if (value == "rungekutta56" || value == "rk56") {
			settings.intgr_type = RUNGE_KUTTA56;
//			settings.integrator = new RungeKutta56();
		}
		else if (value == "rungekuttafehlberg78" || value == "rk78" || value == "rungekutta78") {
			settings.intgr_type = RUNGE_KUTTA_FEHLBERG78;
			settings.integrator = new RungeKuttaFehlberg78();
		}
		else if (value == "dormandprince" || value == "rkn76") {
			settings.intgr_type = DORMAND_PRINCE;
			settings.integrator = new DormandPrince();
		}
		else {
			Error::_errMsg = "Unknown integrator type!";
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
		settings.integrator->epsilon  = pow(10, atof(value.c_str()));
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
    else if (key == "timeLine_unit") {
	;
//TODO
    }
    settings.timeLine = new TimeLine(timeLine);
    if (key == "ejection_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.ejection= atoi(value.c_str());
    }
    else if (key == "ejection_unit") {
;
//TODO
    }
    else if (key == "hitCentrum_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.hitCentrum = atof(value.c_str());
    }
    else if (key == "hitCentrum_unit") {
;
//TODO
    }
    else if (key == "collision_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.collision->factor = atoi(value.c_str());
    }
    else if (key == "collision_stop") {
		if (!Tools::StringToBool(value,&settings.collision->stop)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
	settings.collision = new EventCondition(e);
	e = EventCondition();
    if (key == "closeEncounter_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.closeEncounter->factor = atof(value.c_str());
    }
    else if (key == "closeEncounter_stop") {
		if (!Tools::StringToBool(value,&settings.closeEncounter->stop)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
	settings.closeEncounter = new EventCondition(e);
	e = EventCondition();
    if (key == "weakCapture_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.weakCapture->factor = atof(value.c_str());
    }
    else if (key == "weakCapture_stop") {
		if (!Tools::StringToBool(value,&settings.weakCapture->stop)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	settings.weakCapture = new EventCondition(e);
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

int	ParseSettings(Settings &settings, std::string& str, const bool verbose)
{
	// instantiate Tokenizer classes
	Tokenizer fileTokenizer;
	Tokenizer lineTokenizer;
	std::string line;

	fileTokenizer.set(str, "\n");
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
			if (SetSettings(key, value, settings, verbose) == 1)
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
*/