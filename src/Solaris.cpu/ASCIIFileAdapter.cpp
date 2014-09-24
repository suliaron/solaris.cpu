#include <iostream>
#include <fstream>
#include <algorithm>

#include "BodyGroupList.h"
#include "Component.h"
#include "Constants.h"
#include "DormandPrince.h"
#include "Ephemeris.h"
#include "Error.h"
#include "EventCondition.h"
#include "FargoParameters.h"
#include "Nebula.h"
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

static int SetSettings(std::string& key, std::string& value, Settings& settings, const bool verbose)
{
	Tools::Trim(key);
	Tools::Trim(value);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (key == "enabledistinctstarttimes") {
		if (Tools::StringToBool(value,&settings.enableDistinctStartTimes)) {
			Error::_errMsg = "Invalid value: '" + value + "'!";
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
			Error::_errMsg = "Invalid value: '" + value + "'!";
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
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!settings.timeLine) {
			settings.timeLine = new TimeLine();
		}
		settings.timeLine->start = atof(value.c_str());
		settings.timeLine->startTimeDefined = true;
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
		else if (!settings.timeLine) {
			settings.timeLine = new TimeLine();
		}
		settings.timeLine->length = atof(value.c_str());
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
		else if (!settings.timeLine) {
			settings.timeLine = new TimeLine();
		}
		settings.timeLine->output = atof(value.c_str());
    }
    else if (key == "timeline_unit") {
		double dummy = 0.0;
		if (UnitTool::TimeToDay(value, dummy) == 1) {
			Error::_errMsg = "Unrecognized dimension!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!settings.timeLine) {
			Error::_errMsg = "Invalid timeline";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		UnitTool::TimeToDay(value, settings.timeLine->start);
		UnitTool::TimeToDay(value, settings.timeLine->length);
		UnitTool::TimeToDay(value, settings.timeLine->output);
	}
    else if (key == "ejection_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
				Error::_errMsg = "Value out of range!";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
		}
		settings.ejection = atof(value.c_str());
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
		if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
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
		else if (!settings.closeEncounter) {
			settings.closeEncounter = new EventCondition();
		}
		settings.closeEncounter->factor = atof(value.c_str());
    }
    else if (key == "closeencounter_stop") {
		if (!settings.closeEncounter) {
			settings.closeEncounter = new EventCondition();
		}
		if (Tools::StringToBool(value,&settings.closeEncounter->stop)) {
			Error::_errMsg = "Invalid value: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
	else if (key == "collision_factor") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!Validator::ElementOfAndContainsLower(0.0,1.0,atof(value.c_str()))) {
			Error::_errMsg = "Invalid value for 'factor'";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!settings.collision) {
			settings.collision = new EventCondition();
		}
		settings.collision->factor = atof(value.c_str());
    }
    else if (key == "collision_stop") {
		if (!settings.collision) {
			settings.collision = new EventCondition();
		}
		if (Tools::StringToBool(value,&settings.collision->stop)) {
			Error::_errMsg = "Invalid value: '" + value + "'!";
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
		else if (!settings.weakCapture) {
			settings.weakCapture= new EventCondition();
		}
		settings.weakCapture->factor = atof(value.c_str());
    }
    else if (key == "weakcapture_stop") {
		if (!settings.weakCapture) {
			settings.weakCapture = new EventCondition();
		}
		if (Tools::StringToBool(value,&settings.weakCapture->stop)) {
			Error::_errMsg = "Invalid value: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
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

static int SetBody(std::string& bodystring, Body& body)
{
	//id, name, designation, provisionaldesignation, type, mpcorbittype, migration type, mitration stopat, reference...
	//opposition, ln, pos x, pos y, pos z, vel x, vel y, vel z, absvismag, cd, mass, radius, density, component number, component name, component ratio

	Tokenizer bodyTokenizer;
	Component component;
	double ratiosum = 100;
	int compnum;

	bodyTokenizer.set(bodystring, "|");
	std::string value;
	
	value = bodyTokenizer.next();
	if (!Tools::IsNumber(value)) {
		Error::_errMsg = "Invalid number: '" + value + "'!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	body._id = atoi(value.c_str());

	value = bodyTokenizer.next();
	if (value != " ") {
		body.name = value;
	}
	
	value = bodyTokenizer.next();
	if (value != " ") {
		body.designation = value;
	}

	value = bodyTokenizer.next();
	if (value != " ") {
		body.provisionalDesignation = value;
	}

	value = bodyTokenizer.next();
	if (value != " ") {
		if (value == "centralbody")				{ body.type = CentralBody;		 }
		else if (value == "giantplanet")		{ body.type = GiantPlanet;		 }
		else if (value == "rockyplanet")		{ body.type = RockyPlanet;		 }
		else if (value == "protoplanet")		{ body.type = ProtoPlanet;		 }
		else if (value == "superplanetesimal")	{ body.type = SuperPlanetesimal; }
		else if (value == "planetesimal")		{ body.type = Planetesimal;		 }
		else if (value == "testparticle")		{ body.type = TestParticle;		 }
		else {
			Error::_errMsg = "Unknown body type!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}

	value = bodyTokenizer.next();
	if (value != " ") {
		if (     value == "aten")											{ body.mPCOrbitType = Aten;											}
		else if (value == "apollo")											{ body.mPCOrbitType = Apollo;										}
		else if (value == "amor")											{ body.mPCOrbitType = Amor;											}
		else if (value == "objectwithqlt1_665")								{ body.mPCOrbitType = ObjectWithqLt1_665;							}
		else if (value == "hungaria")										{ body.mPCOrbitType = Hungaria;										}
		else if (value == "phocaea")										{ body.mPCOrbitType = Phocaea;										}
		else if (value == "hilda")											{ body.mPCOrbitType = Hilda;										}
		else if (value == "jupitertrojan")									{ body.mPCOrbitType = JupiterTrojan;								}
		else if (value == "centaur")										{ body.mPCOrbitType = Centaur;										}
		else if (value == "plutino")										{ body.mPCOrbitType = Plutino;										}
		else if (value == "otherresonanttno")								{ body.mPCOrbitType = OtherResonantTNO;								}
		else if (value == "cubewano")										{ body.mPCOrbitType = Cubewano;										}
		else if (value == "scattereddisk")									{ body.mPCOrbitType = ScatteredDisk;								}
		else if (value == "objectisneo")									{ body.mPCOrbitType = ObjectIsNEO;									}
		else if (value == "objectis1kmorlargerneo")							{ body.mPCOrbitType = ObjectIs1kmOrLargerNEO;						}
		else if (value == "oneoppositionobjectseenatearlieropposition")		{ body.mPCOrbitType = OneOppositionObjectSeenAtEarlierOpposition;	}
		else if (value == "criticallistnumberedobject")						{ body.mPCOrbitType = CriticalListNumberedObject;					}
		else if (value == "objectispha")									{ body.mPCOrbitType = ObjectIsPHA;									}
		else {
			Error::_errMsg = "Unknown body MPCOrbitType!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}

	value = bodyTokenizer.next();
	if (value != " ") {
		if (     value == "i")  { body.migrationType = TypeI;  }
		else if (value == "ii") { body.migrationType = TypeII; }
		else {
			Error::_errMsg = "Unknown migration type";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}

	value = bodyTokenizer.next();
	if (value != " ") {
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
		body.migrationStopAt = atof(value.c_str());
	}
	
	value = bodyTokenizer.next();
	if (value != " ") {
		body.reference = value;
	}

	value = bodyTokenizer.next();
	if (value != " ") {
		body.opposition = value;
	}

	value = bodyTokenizer.next();
	if (value != " ") {
		if (     value == "l1") { body.ln = L1; }
		else if (value == "l2") { body.ln = L2; }
		else if (value == "l3") { body.ln = L3; }
		else if (value == "l4") { body.ln = L4; }
		else if (value == "l5") { body.ln = L5; }
		else {
			Error::_errMsg = "Unknown ln!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}

	value = bodyTokenizer.next();
	if (!Tools::IsNumber(value)) {
		Error::_errMsg = "Invalid number: '" + value + "'!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	body.phase = new Phase(body.GetId());
	body.phase->position.x =  atof(value.c_str());

	value = bodyTokenizer.next();
	if (!Tools::IsNumber(value)) {
		Error::_errMsg = "Invalid number: '" + value + "'!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	body.phase->position.y =  atof(value.c_str());

	value = bodyTokenizer.next();
	if (!Tools::IsNumber(value)) {
		Error::_errMsg = "Invalid number: '" + value + "'!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	body.phase->position.z =  atof(value.c_str());

	value = bodyTokenizer.next();
	if (!Tools::IsNumber(value)) {
		Error::_errMsg = "Invalid number: '" + value + "'!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	body.phase->velocity.x =  atof(value.c_str());

	value = bodyTokenizer.next();
	if (!Tools::IsNumber(value)) {
		Error::_errMsg = "Invalid number: '" + value + "'!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	body.phase->velocity.y =  atof(value.c_str());

	value = bodyTokenizer.next();
	if (!Tools::IsNumber(value)) {
		Error::_errMsg = "Invalid number: '" + value + "'!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	body.phase->velocity.z =  atof(value.c_str());
		
	value = bodyTokenizer.next();
	if (value != " "){
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!body.characteristics) {
			body.characteristics = new Characteristics();
		}
		body.characteristics->absVisMag = atof(value.c_str());
	}

	value = bodyTokenizer.next();
	if (value != " ") {
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
		else if (!body.characteristics) {
			body.characteristics = new Characteristics();
		}
		body.characteristics->stokes = atof(value.c_str());
	}

	value = bodyTokenizer.next();
	if (value != " ") {
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
		else if (!body.characteristics) {
			body.characteristics = new Characteristics();
		}
		body.characteristics->mass = atof(value.c_str());
	}

	value = bodyTokenizer.next();
	if (value != " ") {
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
		else if (!body.characteristics) {
			body.characteristics = new Characteristics();
		}
		body.characteristics->radius = atof(value.c_str());
	}

	value = bodyTokenizer.next();
	if (value != " ") {
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
		else if (body.type != SuperPlanetesimal && atof(value.c_str()) != 0 && body.characteristics->radius > 0) {
			Error::_errMsg = "The radius and density of a body cannot be defined simultaneously!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (!body.characteristics) {
			body.characteristics = new Characteristics();
		}
		body.characteristics->density = atof(value.c_str());
	}

	value = bodyTokenizer.next();
	if (value != "0") {
		compnum = atoi(value.c_str());
		for (2*compnum ; compnum > 0 ; compnum--) {
			value = bodyTokenizer.next();
			body.characteristics->componentList.push_back(component);
			body.characteristics->componentList.back().name = value;

			value = bodyTokenizer.next();
			if (!Tools::IsNumber(value)) {
				Error::_errMsg = "Invalid number: '" + value + "'!";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
			}
			else if (!Validator::ElementOfAndContainsEndPoints(0.0, 100.0, atof(value.c_str()))) {
				Error::_errMsg = "Value out of range";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
			}
			body.characteristics->componentList.back().ratio = atof(value.c_str());
			ratiosum -= atof(value.c_str());
		}
		if (bodyTokenizer.next() != "") {
			Error::_errMsg = "Invalid number of components";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		else if (fabs(ratiosum) > 1.0e-4 ) {
		Error::_errMsg = "The sum of the ComponentList tag is not 100%!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
		}
	}
	else if (bodyTokenizer.next() != "") {
		Error::_errMsg = "Invalid number of components";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}

	return 0;
}

static int SetBodyGroupList(std::string& key, std::string& value, BodyGroupList& bodyGroupList, const bool verbose)
{
	//TODO: check if all position and velocity tags are set!
	//TODO: check orbital element angle unit!

	BodyGroup bodyGroup;
	Body body;
	
	Tools::Trim(key);
	Tools::Trim(value);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (key == "description") {
		bodyGroupList.items.push_back(bodyGroup);
		bodyGroupList.items.back().description = value;
    } 
    else if (key == "epoch") {
		if (Ephemeris::GetFormat(value) == UndefinedFormat) {
			Error::_errMsg = "Undefined epoch format!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		bodyGroupList.items.back().epoch = value;

    }
    else if (key == "offset") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		bodyGroupList.items.back().offset = atof(value.c_str());
    }
    else if (key == "referenceframe") {
		bodyGroupList.items.back().referenceFrame = value;
    }
    else if (key == "guid") {
		bodyGroupList.items.back().guid = value;
    }
	else if (key == "body") {
		if (SetBody(value, body) == 1) {
				return 1;
		}
		bodyGroupList.items.back().items.push_back(body);
	}
 //   else if (key == "bodygroup_body_type") {
	//	bodyGroupList.items.back().items.push_back(body);
	//	if (value == "centralbody")				{ bodyGroupList.items.back().items.back().type = CentralBody;		 }
	//	else if (value == "giantplanet")		{ bodyGroupList.items.back().items.back().type = GiantPlanet;		 }
	//	else if (value == "rockyplanet")		{ bodyGroupList.items.back().items.back().type = RockyPlanet;		 }
	//	else if (value == "protoplanet")		{ bodyGroupList.items.back().items.back().type = ProtoPlanet;		 }
	//	else if (value == "superplanetesimal")	{ bodyGroupList.items.back().items.back().type = SuperPlanetesimal;  }
	//	else if (value == "planetesimal")		{ bodyGroupList.items.back().items.back().type = Planetesimal;		 }
	//	else if (value == "testparticle")		{ bodyGroupList.items.back().items.back().type = TestParticle;		 }
	//	else {
	//		Error::_errMsg = "Unknown body type!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
 //   }
	//else if (key == "bodygroup_body_name") {
	//	bodyGroupList.items.back().items.back().name = value;
	//}
	//else if (key == "bodygroup_body_mpcorbittype") {
	//	if (     value == "aten")											{ bodyGroupList.items.back().items.back().mPCOrbitType = Aten;											}
	//	else if (value == "apollo")											{ bodyGroupList.items.back().items.back().mPCOrbitType = Apollo;										}
	//	else if (value == "amor")											{ bodyGroupList.items.back().items.back().mPCOrbitType = Amor;											}
	//	else if (value == "objectwithqlt1_665")								{ bodyGroupList.items.back().items.back().mPCOrbitType = ObjectWithqLt1_665;							}
	//	else if (value == "hungaria")										{ bodyGroupList.items.back().items.back().mPCOrbitType = Hungaria;										}
	//	else if (value == "phocaea")										{ bodyGroupList.items.back().items.back().mPCOrbitType = Phocaea;										}
	//	else if (value == "hilda")											{ bodyGroupList.items.back().items.back().mPCOrbitType = Hilda;											}
	//	else if (value == "jupitertrojan")									{ bodyGroupList.items.back().items.back().mPCOrbitType = JupiterTrojan;									}
	//	else if (value == "centaur")										{ bodyGroupList.items.back().items.back().mPCOrbitType = Centaur;										}
	//	else if (value == "plutino")										{ bodyGroupList.items.back().items.back().mPCOrbitType = Plutino;										}
	//	else if (value == "otherresonanttno")								{ bodyGroupList.items.back().items.back().mPCOrbitType = OtherResonantTNO;								}
	//	else if (value == "cubewano")										{ bodyGroupList.items.back().items.back().mPCOrbitType = Cubewano;										}
	//	else if (value == "scattereddisk")									{ bodyGroupList.items.back().items.back().mPCOrbitType = ScatteredDisk;									}
	//	else if (value == "objectisneo")									{ bodyGroupList.items.back().items.back().mPCOrbitType = ObjectIsNEO;									}
	//	else if (value == "objectis1kmorlargerneo")							{ bodyGroupList.items.back().items.back().mPCOrbitType = ObjectIs1kmOrLargerNEO;						}
	//	else if (value == "oneoppositionobjectseenatearlieropposition")		{ bodyGroupList.items.back().items.back().mPCOrbitType = OneOppositionObjectSeenAtEarlierOpposition;	}
	//	else if (value == "criticallistnumberedobject")						{ bodyGroupList.items.back().items.back().mPCOrbitType = CriticalListNumberedObject;					}
	//	else if (value == "objectispha")									{ bodyGroupList.items.back().items.back().mPCOrbitType = ObjectIsPHA;									}
	//	else {
	//		Error::_errMsg = "Unknown body MPCOrbitType!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
 //   }
	//else if (key == "bodygroup_body_designation" || key == "bodygroup_body_des") {
	//	bodyGroupList.items.back().items.back().designation = value;
	//}
	//else if (key == "bodygroup_body_provisionaldesignation" || key == "bodygroup_body_provdes") {
	//	bodyGroupList.items.back().items.back().provisionalDesignation = value;
	//}
	//else if (key == "bodygroup_body_reference" || key == "bodygroup_body_ref") {
	//	bodyGroupList.items.back().items.back().reference = value;
	//}
	//else if (key == "bodygroup_body_opposition" || key == "bodygroup_body_opp") {
	//	bodyGroupList.items.back().items.back().opposition = value;
	//}
 //   else if (key == "bodygroup_body_ln") {
	//	if (     value == "l1") { bodyGroupList.items.back().items.back().ln = L1; }
	//	else if (value == "l2") { bodyGroupList.items.back().items.back().ln = L2; }
	//	else if (value == "l3") { bodyGroupList.items.back().items.back().ln = L3; }
	//	else if (value == "l4") { bodyGroupList.items.back().items.back().ln = L4; }
	//	else if (value == "l5") { bodyGroupList.items.back().items.back().ln = L5; }
	//	else {
	//		Error::_errMsg = "Unknown ln!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
 //   }
	//else if (key == "bodygroup_body_guid") {
	//	bodyGroupList.items.back().items.back().guid = value;
	//}
 //   else if (key == "bodygroup_body_phase_position_x") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		bodyGroupList.items.back().items.back().phase = new Phase(bodyGroupList.items.back().items.back().GetId());
	//	}
	//	bodyGroupList.items.back().items.back().phase->position.x =  atof(value.c_str());
	//}
 //   else if (key == "bodygroup_body_phase_position_y") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		bodyGroupList.items.back().items.back().phase = new Phase(bodyGroupList.items.back().items.back().GetId());
	//	}
	//	bodyGroupList.items.back().items.back().phase->position.y =  atof(value.c_str());
	//}
 //   else if (key == "bodygroup_body_phase_position_z") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		bodyGroupList.items.back().items.back().phase = new Phase(bodyGroupList.items.back().items.back().GetId());
	//	}
	//	bodyGroupList.items.back().items.back().phase->position.z =  atof(value.c_str());
	//}
 //   else if (key == "bodygroup_body_phase_position_unit") {
	//	double dummy = 0.0;
	//	if (UnitTool::DistanceToAu(value, dummy) == 1) {
	//			Error::_errMsg = "Unrecognized dimension!";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		Error::_errMsg = "Invalid phase";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	UnitTool::DistanceToAu(value, bodyGroupList.items.back().items.back().phase->position.x);
	//	UnitTool::DistanceToAu(value, bodyGroupList.items.back().items.back().phase->position.y);
	//	UnitTool::DistanceToAu(value, bodyGroupList.items.back().items.back().phase->position.z);
 //   }
	//else if (key == "bodygroup_body_phase_velocity_x") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		bodyGroupList.items.back().items.back().phase = new Phase(bodyGroupList.items.back().items.back().GetId());
	//	}
	//	bodyGroupList.items.back().items.back().phase->velocity.x =  atof(value.c_str());
	//}
 //   else if (key == "bodygroup_body_phase_velocity_y") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		bodyGroupList.items.back().items.back().phase = new Phase(bodyGroupList.items.back().items.back().GetId());
	//	}
	//	bodyGroupList.items.back().items.back().phase->velocity.y =  atof(value.c_str());
	//}
 //   else if (key == "bodygroup_body_phase_velocity_z") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		bodyGroupList.items.back().items.back().phase = new Phase(bodyGroupList.items.back().items.back().GetId());
	//	}
	//	bodyGroupList.items.back().items.back().phase->velocity.z =  atof(value.c_str());
	//}
 //   else if (key == "bodygroup_body_phase_velocity_unit") {
	//	double dummy = 0.0;
	//	if (UnitTool::VelocityToCM(value, dummy) == 1) {
	//			Error::_errMsg = "Unrecognized dimension!";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().phase) {
	//		Error::_errMsg = "Invalid phase";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	UnitTool::VelocityToCM(value, bodyGroupList.items.back().items.back().phase->velocity.x);
	//	UnitTool::VelocityToCM(value, bodyGroupList.items.back().items.back().phase->velocity.y);
	//	UnitTool::VelocityToCM(value, bodyGroupList.items.back().items.back().phase->velocity.z);
 //   }
 //   else if (key == "bodygroup_body_orbitalelement_semimajoraxis" || key == "bodygroup_body_orbitalelement_a") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	if (!Validator::GreaterThan(0,  atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		bodyGroupList.items.back().items.back().orbitalElement = new OrbitalElement();
	//	}
	//	bodyGroupList.items.back().items.back().orbitalElement->semiMajorAxis = atof(value.c_str());
 //   }
	//else if (key == "bodygroup_body_orbitalelement_eccentricity" || key == "bodygroup_body_orbitalelement_e") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	if (!Validator::ElementOfAndContainsLower(0.0, 1.0, atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		bodyGroupList.items.back().items.back().orbitalElement = new OrbitalElement();
	//	}
	//	bodyGroupList.items.back().items.back().orbitalElement->eccentricity = atof(value.c_str());
 //   }
 //   else if (key == "bodygroup_body_orbitalelement_inclination" || key == "bodygroup_body_orbitalelement_incl") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		bodyGroupList.items.back().items.back().orbitalElement = new OrbitalElement();
	//	}
	//	bodyGroupList.items.back().items.back().orbitalElement->inclination = atof(value.c_str());
 //   }
 //   else if (key == "bodygroup_body_orbitalelement_argumentofpericenter" || key == "bodygroup_body_orbitalelement_peri") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		bodyGroupList.items.back().items.back().orbitalElement = new OrbitalElement();
	//	}
	//	bodyGroupList.items.back().items.back().orbitalElement->argumentOfPericenter = atof(value.c_str());
 //   }
 //   else if (key == "bodygroup_body_orbitalelement_longitudeofnode" || key == "bodygroup_body_orbitalelement_node") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		bodyGroupList.items.back().items.back().orbitalElement = new OrbitalElement();
	//	}
	//	bodyGroupList.items.back().items.back().orbitalElement->longitudeOfNode = atof(value.c_str());
 //   }
 //   else if (key == "bodygroup_body_orbitalelement_meananomaly" || key == "bodygroup_body_orbitalelement_m") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		bodyGroupList.items.back().items.back().orbitalElement = new OrbitalElement();
	//	}
	//	bodyGroupList.items.back().items.back().orbitalElement->meanAnomaly = atof(value.c_str());
 //   }
 //   else if (key == "bodygroup_body_orbitalelement_distanceunit") {
	//	double dummy = 0.0;
	//	if (UnitTool::DistanceToAu(value, dummy) == 1) {
	//			Error::_errMsg = "Unrecognized dimension!";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		Error::_errMsg = "Invalid orbital element";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	UnitTool::DistanceToAu(value, bodyGroupList.items.back().items.back().orbitalElement->semiMajorAxis);
	//}
	//else if (key == "bodygroup_body_orbitalelement_angleunit") {
	//	double dummy = 0.0;
	//	if (UnitTool::AngleToRadian(value, dummy) == 1) {
	//			Error::_errMsg = "Unrecognized dimension!";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().orbitalElement) {
	//		Error::_errMsg = "Invalid orbital element";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	UnitTool::AngleToRadian(value, bodyGroupList.items.back().items.back().orbitalElement->inclination);
	//	if (!Validator::ElementOfAndContainsEndPoints(0, Constants::Pi, bodyGroupList.items.back().items.back().orbitalElement->inclination)) {
	//			Error::_errMsg = "Value out of range";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	UnitTool::AngleToRadian(value, bodyGroupList.items.back().items.back().orbitalElement->argumentOfPericenter);
	//	Ephemeris::ShiftIntoRange(0.0, 2.0*Constants::Pi, bodyGroupList.items.back().items.back().orbitalElement->argumentOfPericenter);
	//	UnitTool::AngleToRadian(value, bodyGroupList.items.back().items.back().orbitalElement->longitudeOfNode);
	//	Ephemeris::ShiftIntoRange(0.0, 2.0*Constants::Pi, bodyGroupList.items.back().items.back().orbitalElement->longitudeOfNode);
	//	UnitTool::AngleToRadian(value, bodyGroupList.items.back().items.back().orbitalElement->meanAnomaly);
	//	Ephemeris::ShiftIntoRange(0.0, 2.0*Constants::Pi, bodyGroupList.items.back().items.back().orbitalElement->meanAnomaly);
	//}
	//else if (key == "bodygroup_body_charasteristics_absvismag") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		bodyGroupList.items.back().items.back().characteristics = new Characteristics();
	//	}
	//	bodyGroupList.items.back().items.back().characteristics->absVisMag = atof(value.c_str());
 //   }
	//else if (key == "bodygroup_body_charasteristics_cd") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		bodyGroupList.items.back().items.back().characteristics = new Characteristics();
	//	}
	//	bodyGroupList.items.back().items.back().characteristics->stokes = atof(value.c_str());
 //   }
	//else if (key == "bodygroup_body_characteristics_mass_value") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		bodyGroupList.items.back().items.back().characteristics = new Characteristics();
	//	}
	//	bodyGroupList.items.back().items.back().characteristics->mass = atof(value.c_str());
 //   }
	//else if (key == "bodygroup_body_characteristics_mass_unit") {
	//	double dummy = 0.0;
	//	if (UnitTool::MassToSolar(value, dummy) == 1) {
	//			Error::_errMsg = "Unrecognized dimension!";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		Error::_errMsg = "Invalid orbital element";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	UnitTool::MassToSolar(value, bodyGroupList.items.back().items.back().characteristics->mass);
 //   }
	//else if (key == "bodygroup_body_characteristics_radius_value") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		bodyGroupList.items.back().items.back().characteristics = new Characteristics();
	//	}
	//	bodyGroupList.items.back().items.back().characteristics->radius = atof(value.c_str());
 //   }
	//else if (key == "bodygroup_body_characteristics_radius_unit") {
	//	double dummy = 0.0;
	//	if (UnitTool::DistanceToAu(value, dummy) == 1) {
	//			Error::_errMsg = "Unrecognized dimension!";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		Error::_errMsg = "Invalid orbital element";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	UnitTool::DistanceToAu(value, bodyGroupList.items.back().items.back().characteristics->radius);
 //   }
	//else if (key == "bodygroup_body_characteristics_density_value") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (body.type != SuperPlanetesimal && atof(value.c_str()) != 0 && bodyGroupList.items.back().items.back().characteristics->radius > 0) {
	//		Error::_errMsg = "The radius and density of a body cannot be defined simultaneously!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		bodyGroupList.items.back().items.back().characteristics = new Characteristics();
	//	}
	//	bodyGroupList.items.back().items.back().characteristics->density = atof(value.c_str());
 //   }
	//else if (key == "bodygroup_body_characteristics_density_unit") {
	//	double dummy = 0.0;
	//	if (UnitTool::VolumeDensityToCM(value, dummy) == 1) {
	//			Error::_errMsg = "Unrecognized dimension!";
	//			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//			return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		Error::_errMsg = "Invalid orbital element";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	UnitTool::VolumeDensityToCM(value, bodyGroupList.items.back().items.back().characteristics->density);
	//}
	//else if (key == "bodygroup_body_characteristics_componentlist_component_name") {
	//	if (!bodyGroupList.items.back().items.back().characteristics) {
	//		bodyGroupList.items.back().items.back().characteristics = new Characteristics();
	//	}
	//	bodyGroupList.items.back().items.back().characteristics->componentList.push_back(component);
	//	bodyGroupList.items.back().items.back().characteristics->componentList.back().name = value;
 //   }
	//else if (key == "bodygroup_body_characteristics_componentlist_component_ratio") {
	//	if (!Tools::IsNumber(value)) {
	//		Error::_errMsg = "Invalid number: '" + value + "'!";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!Validator::ElementOfAndContainsEndPoints(0.0, 100.0, atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	else if (!bodyGroupList.items.back().items.back().characteristics) {
	//		bodyGroupList.items.back().items.back().characteristics = new Characteristics();
	//	}
	//	bodyGroupList.items.back().items.back().characteristics->componentList.back().ratio = atof(value.c_str());
	//	ratiosum -= atof(value.c_str());
 //   }
	//else if (key == "bodygroup_body_migration_type") {
	//	if (     value == "i")  { bodyGroupList.items.back().items.back().migrationType = TypeI;  }
	//	else if (value == "ii") { bodyGroupList.items.back().items.back().migrationType = TypeII; }
	//	else {
	//		Error::_errMsg = "Unknown migration type";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
 //   }
	//else if (key == "bodygroup_body_migration_stopat") {
	//	if (!Validator::GreaterThan(0.0, atof(value.c_str()))) {
	//		Error::_errMsg = "Value out of range";
	//		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
	//		return 1;
	//	}
	//	bodyGroupList.items.back().items.back().migrationStopAt = atof(value.c_str());
 //   }
    else {
        std::cerr << "Unrecoginzed key: '" << key << "'!" << std::endl;
		return 1;
    }

	if (verbose) {
		std::cout << key << " was assigned to " << value << std::endl;
	}

	
	return 0;
}

static int SetNebula(std::string& key, std::string& value, Nebula* nebula, const bool verbose)
{

	Tools::Trim(key);
	Tools::Trim(value);
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	if (key == "name") {
		nebula->name = value;
    } 
    else if (key == "description") {
		nebula->description = value;
    }
    else if (key == "fargopath") {
		nebula->fargoPath = value;
    }
    else if (key == "gascomponent_alpha") {
		if (!Validator::GreaterThan(0.0, atof(value.c_str()))) {
			Error::_errMsg = "Value out of range";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.alpha = atof(value.c_str());
    }
    else if (key == "gascomponent_type") {
		if (value == "constant")
			nebula->gasComponent.type = CONSTANT;
		else if (value == "linear")
			nebula->gasComponent.type = LINEAR;
		else if (value == "exponential")
			nebula->gasComponent.type = EXPONENTIAL;
		else {
			Error::_errMsg = "Unknown gascomponent type!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
    }
    else if (key == "gascomponent_timescale") {
		if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
			Error::_errMsg = "Value out of range";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.timeScale = atof(value.c_str());
    }
    else if (key == "gascomponent_t0") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.t0 = atof(value.c_str());
    }
    else if (key == "gascomponent_t1") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.t1 = atof(value.c_str());
    }
    else if (key == "gascomponent_unit") {
		double dummy = 0.0;
		if (UnitTool::TimeToDay(value, dummy) == 1) {
			Error::_errMsg = "Unrecognized dimension!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		UnitTool::TimeToDay(value, nebula->gasComponent.timeScale);
		UnitTool::TimeToDay(value, nebula->gasComponent.t0);
		UnitTool::TimeToDay(value, nebula->gasComponent.t1);
    }
    else if (key == "gascomponent_eta_c") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.eta.c = atof(value.c_str());
    }
	else if (key == "gascomponent_eta_index") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.eta.index = atof(value.c_str());
    }
	else if (key == "gascomponent_tau_c") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.tau.c = atof(value.c_str());
    }
	else if (key == "gascomponent_tau_index") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.tau.index = atof(value.c_str());
    }
    else if (key == "gascomponent_scaleheight_c") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.scaleHeight.c = atof(value.c_str());
    }
	else if (key == "gascomponent_scaleheight_index") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.scaleHeight.index = atof(value.c_str());
    }
    else if (key == "gascomponent_densityfunction_index") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.density.index = atof(value.c_str());
    }
    else if (key == "gascomponent_densityfunction_density_value") {
		if (!Tools::IsNumber(value)) {
			Error::_errMsg = "Invalid number: '" + value + "'!";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		if (!Validator::GreaterThanOrEqualTo(0.0, atof(value.c_str()))) {
			Error::_errMsg = "Value out of range";
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
		nebula->gasComponent.density.c = atof(value.c_str());
    }
    else if (key == "gascomponent_densityfunction_density_unit") {
		double dummy = 0.0;
		if (UnitTool::VolumeDensityToCM(value, dummy) == 1) {
				Error::_errMsg = "Unrecognized dimension!";
				Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
				return 1;
		}
		UnitTool::VolumeDensityToCM(value, nebula->gasComponent.density.c);
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

static int	ParseBodyGroupList(BodyGroupList &bodyGroupList, std::string& str, const bool verbose)
{
	// instantiate Tokenizer classes
	Tokenizer fileTokenizer;
	Tokenizer lineTokenizer;
	std::string line;

	Phase phase;
	OrbitalElement orbitalElement;
	Characteristics characteristics;

	fileTokenizer.set(str, "\n");
	while ((line = fileTokenizer.next()) != "") {
		lineTokenizer.set(line, "=");
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
			if (SetBodyGroupList(key, value, bodyGroupList, verbose) == 1)
				return 1;
		}
		else {
			Error::_errMsg = "Invalid key/value pair: " + line;
			Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
			return 1;
		}
	}

	if (bodyGroupList.items.back().items.back().phase!= 0 && bodyGroupList.items.back().items.back().orbitalElement!= 0 ) {
		Error::_errMsg = "You cannot define both Phase and OrbitalElement tags for the same Body!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	if (bodyGroupList.items.back().items.back().phase == 0 && bodyGroupList.items.back().items.back().orbitalElement == 0) {
		Error::_errMsg = "Either Phase or OrbitalElement tag must be defined for a Body!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	else if (bodyGroupList.items.back().items.back().type == UndefinedBodyType) {
		Error::_errMsg = "The 'type' attribute is obligatory!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	else if (bodyGroupList.items.back().items.back().type == CentralBody && bodyGroupList.items.back().items.back().phase == 0 && bodyGroupList.items.back().items.back().orbitalElement == 0) {
		bodyGroupList.items.back().items.back().phase= new Phase(bodyGroupList.items.back().items.back().GetId());
	}
	else if (bodyGroupList.items.back().items.back().type == TestParticle && bodyGroupList.items.back().items.back().characteristics != 0 && bodyGroupList.items.back().items.back().migrationType != No) {
		Error::_errMsg = "You cannot define Characteristics and MigrationType tags for TestParticle type body!";
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__);
		return 1;
	}
	
	return 0;
}


static int	ParseNebula(Nebula* nebula, std::string& str, const bool verbose)
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
			if (SetNebula(key, value, nebula, verbose) == 1)
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