#ifndef BODY_H_
#define BODY_H_

#include <string>
#include "Constants.h"
#include "Phase.h"
#include "OrbitalElement.h"
#include "Characteristics.h"
#include "SolarisType.h"

class Phase;
class OrbitalElement;
class Characteristics;

class Body {
public:

	Body();
	Body(int id);
	Body(body_type_t type);
	Body(body_type_t type, std::string name);

	inline int GetId() { return _id; }

	double GetGm();
	double CalculateOrbitalPeriod(double mu);

	void Print();

	mpcorbit_type_t		mPCOrbitType;
	std::string			mPCOrbitTypestr;
	std::string			name;
	std::string			designation;
	std::string			provisionalDesignation;
	std::string			reference;
	std::string			opposition;
	std::string			guid;
	body_type_t			type;
	ln_t				ln;
	migration_type_t	migrationType;
	double				migrationStopAt;

	Phase				*phase;
	OrbitalElement		*orbitalElement;
	Characteristics		*characteristics;

//private:
	// id of the body instance
	int			_id;

	// id of the class Body, its value will be assigned to the next instance of Body
	// guaranteeing the uniqueness of the _id field of each Body object.
	static int _bodyId;
};

#endif
