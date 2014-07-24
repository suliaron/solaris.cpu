#include <iostream>
#include <fstream>
#include <algorithm>

#include "Common.h"
#include "Constants.h"
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
#include "Validator.h"
#include "Units.h"


static int ReadFile(char* path, std::string& str)
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
			str += temp;
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

static int SetTimeLine (std::string& key, std::string& value, TimeLine *timeLine)
{
	if (key == "timeline_start") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		timeLine->start = atof(value.c_str());
		timeLine->startTimeDefined = true;
    }
    else if (key == "timeline_length") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (atof(value.c_str()) == 0.0) {
			Error::_errMsg = "Invalid value";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		} 
		timeLine->length = atof(value.c_str());
    }
    else if (key == "timeline_output") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!Validator::GreaterThan(0.0, atof(value.c_str()))) {
			Error::_errMsg = "Value out of range";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		timeLine->output = atof(value.c_str());
    }
    else if (key == "timeline_unit") {
		double dummy = 0.0;
		if (UnitTool::TimeToDay(value, dummy) == 1) {
			Error::_errMsg = "Unrecognized dimension!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		UnitTool::TimeToDay(value, timeLine->start);
		UnitTool::TimeToDay(value, timeLine->length);
		UnitTool::TimeToDay(value, timeLine->output);
	}
	return 0;
}

static int SetEventCondition(std::string& key, std::string& value, EventCondition *e)
{
	if (key == "collision_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!Validator::ElementOfAndContainsLower(0.0,1.0,atoi(value.c_str()))) {
			Error::_errMsg = "Invalid value for 'factor'";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		e->factor = atoi(value.c_str());
    }
    else if (key == "collision_stop") {
		if (Tools::StringToBool(value,&e->stop)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "closeencounter_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
			Error::_errMsg = "Value out of range";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		e->factor = atof(value.c_str());
    }
    else if (key == "closeencounter_stop") {
		if (Tools::StringToBool(value,&e->stop)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "weakcapture_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
			Error::_errMsg = "Value out of range";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		e->factor = atof(value.c_str());
    }
    else if (key == "weakcapture_stop") {
		if (Tools::StringToBool(value,&e->stop)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}
	return 0;
}

static int SetSettings(std::string& key, std::string& value, Settings& settings, TimeLine& timeLine, EventCondition& e, const bool verbose)
{
	Tools::Trim(key);
	Tools::Trim(value);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (key == "enabledistinctstarttimes") {
		if (Tools::StringToBool(value,&settings.enableDistinctStartTimes)) {
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
		if (!Validator::ElementOfAndContainsEndPoints(-16.0, 0.0, atof(value.c_str()))) {
			Error::_errMsg = "Value out of range!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.integrator->accuracy = atof(value.c_str());
		settings.integrator->epsilon  = pow(10, atof(value.c_str()));
    }
    else if (key == "timeline_start") {
		if (SetTimeLine(key,value,&timeLine)) {
			Error::_errMsg = "Invalid timeline!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}
	else if (key == "timeline_length") {
		if (SetTimeLine(key,value,&timeLine)) {
			Error::_errMsg = "Invalid timeline!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}
	else if (key == "timeline_output") {
		if (SetTimeLine(key,value,&timeLine)) {
			Error::_errMsg = "Invalid timeline!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}
	else if (key == "timeline_unit") {
		if (SetTimeLine(key,value,&timeLine)) {
			Error::_errMsg = "Invalid timeline!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.timeLine = new TimeLine(timeLine);
	}
    else if (key == "ejection_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		if (!Validator::GreaterThanOrEqualTo(0.0, atoi(value.c_str()))) {
				Error::_errMsg = "Value out of range!";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
		}
		settings.ejection = atoi(value.c_str());
    }
    else if (key == "ejection_unit") {
		double dummy = 0.0;
		if (UnitTool::DistanceToAu(value, dummy) == 1) {
				Error::_errMsg = "Unrecognized dimension!";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
		}
		UnitTool::DistanceToAu(value, settings.ejection);
    }
    else if (key == "hitcentrum_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		if (!Validator::GreaterThanOrEqualTo(0.0, atoi(value.c_str()))) {
				Error::_errMsg = "Value out of range!";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
		}
		settings.hitCentrum = atof(value.c_str());
    }
    else if (key == "hitcentrum_unit") {
		double dummy = 0.0;
		if (UnitTool::DistanceToAu(value, dummy) == 1) {
				Error::_errMsg = "Unrecognized dimension!";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
		}
		UnitTool::DistanceToAu(value, settings.hitCentrum);
    }
	else if (key == "closeencounter_factor") {
		if (SetEventCondition(key,value,&e)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "closeencounter_stop") {
		if (SetEventCondition(key,value,&e)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.closeEncounter = new EventCondition(e);
		e = EventCondition();
    }
    else if (key == "collision_factor") {
		if (SetEventCondition(key,value,&e)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "collision_stop") {
		if (SetEventCondition(key,value,&e)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		settings.collision = new EventCondition(e);
		e = EventCondition();
    }
    else if (key == "weakcapture_factor") {
		if (SetEventCondition(key,value,&e)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "weakcapture_stop") {
		if (SetEventCondition(key,value,&e)) {
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

static int	ParseSettings(Settings &settings, std::string& str, const bool verbose)
{
	// instantiate Tokenizer classes
	Tokenizer fileTokenizer;
	Tokenizer lineTokenizer;
	std::string line;

	TimeLine timeLine;
	EventCondition e;

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
			if (SetSettings(key, value, settings, timeLine, e, verbose) == 1)
				return 1;
		}
		else {
			Error::_errMsg = "Invalid key/value pair: " + line;
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}

	if (!settings.integrator) {
		Error::_errMsg = "Missing integrator!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	if (!settings.timeLine) {
		Error::_errMsg = "Missing timeline!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}

	return 0;
}