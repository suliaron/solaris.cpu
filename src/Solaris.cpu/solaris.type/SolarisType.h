#pragma once

#include <vector>
#include <string>

using namespace std;

typedef double		ttt_t;
typedef double		var_t;
typedef bool		bool_t;

typedef int			int_t;

typedef struct var2
	{
		double x;
		double y;
	} var2_t;

typedef struct vec
	{
		double x;
		double y;
		double z;
	} vec_t;

typedef struct int2
	{
		int x;
		int y;
	} int2_t;

typedef enum body_type
	{
		BODY_TYPE_UNDEFINED,
		BODY_TYPE_STAR,
		BODY_TYPE_GIANTPLANET,
		BODY_TYPE_ROCKYPLANET,
		BODY_TYPE_PROTOPLANET,
		BODY_TYPE_SUPERPLANETESIMAL,
		BODY_TYPE_PLANETESIMAL,
		BODY_TYPE_TESTPARTICLE,
		BODY_TYPE_N
	} body_type_t;

typedef enum orbelem_name
	{
		ORBELEM_NAME_SMA,
		ORBELEM_NAME_ECC,
		ORBELEM_NAME_INC,
		ORBELEM_NAME_PERI,
		ORBELEM_NAME_NODE,
		ORBELEM_NAME_MEAN,
		ORBELEM_NAME_N
	} orbelem_name_t;

typedef enum phys_prop_name
	{
		PHYS_PROP_NAME_MASS,
		PHYS_PROP_NAME_RADIUS,
		PHYS_PROP_NAME_DENSITY,
		PHYS_PROP_NAME_DRAG_COEFF,
		PHYS_PROP_NAME_N
	} phys_prop_name_t;

typedef enum mpcorbit_type
	{
		MPCORBIT_TYPE_UNDEFINED										= 0,
		MPCORBIT_TYPE_ATEN											= 2,
		MPCORBIT_TYPE_APOLLO										= 3,
		MPCORBIT_TYPE_AMOR											= 4,
		MPCORBIT_TYPE_OBJECTWITHQLT1_665							= 5,
		MPCORBIT_TYPE_HUNGARIA										= 6,
		MPCORBIT_TYPE_PHOCAEA										= 7,
		MPCORBIT_TYPE_HILDA											= 8,
		MPCORBIT_TYPE_JUPITERTROJAN									= 9,
		MPCORBIT_TYPE_CENTAUR										= 10,
		MPCORBIT_TYPE_PLUTINO										= 14,
		MPCORBIT_TYPE_OTHERRESONANTTNO								= 15,
		MPCORBIT_TYPE_CUBEWANO										= 16,
		MPCORBIT_TYPE_SCATTEREDDISK									= 17,
		MPCORBIT_TYPE_OBJECTISNEO									= 2048,
		MPCORBIT_TYPE_OBJECTIS1KMORLARGERNEO						= 4096,
		MPCORBIT_TYPE_ONEOPPOSITIONOBJECTSEENATEARLIEROPPOSITION	= 8192,
		MPCORBIT_TYPE_CRITICALLISTNUMBEREDOBJECT					= 16384,
		MPCORBIT_TYPE_OBJECTISPHA									= 32768
	} mpcorbit_type_t;

typedef enum migration_type
	{
		MIGRATION_TYPE_NO,
		MIGRATION_TYPE_TYPE_I,
		MIGRATION_TYPE_TYPE_II
	} migration_type_t;

typedef enum ln
	{
		LN_UNDEFINED,
		LN_L1,
		LN_L2,
		LN_L3,
		LN_L4,
		LN_L5
	} ln_t;

typedef enum frame_center
	{
		FRAME_CENTER_ASTRO,
		FRAME_CENTER_BARY,
		FRAME_CENTER_N
	} frame_center_t;

typedef enum gas_decrease_type 
	{
		GAS_DECREASE_TYPE_CONSTANT,
		GAS_DECREASE_TYPE_LINEAR,
		GAS_DECREASE_TYPE_EXPONENTIAL,
		GAS_DECREASE_TYPE_N
	} gas_decrease_type_t;

typedef enum integrator_type
	{
		INTEGRATOR_TYPE_UNDEFINED,
		INTEGRATOR_TYPE_DORMAND_PRINCE,
		INTEGRATOR_TYPE_RUNGE_KUTTA4,
		INTEGRATOR_TYPE_RUNGE_KUTTA56,
		INTEGRATOR_TYPE_RUNGE_KUTTA_FEHLBERG78
	} integrator_type_t;

typedef struct orbelem
	{
		var_t sma;
		var_t ecc;
		var_t inc;
		var_t peri;
		var_t node;
		var_t mean;
	} orbelem_t;

typedef struct distribution
	{
		var2_t	limits;
		var_t	(*pdf)(var_t);
	} distribution_t;

typedef struct oe_dist
	{
		distribution_t	item[6];
	} oe_dist_t;

typedef struct phys_prop_dist
	{
		distribution_t	item[4];
	} phys_prop_dist_t;

typedef struct body_disk
	{
		vector<string>		names;
		int_t				nBody[BODY_TYPE_N];
		oe_dist_t			oe_d[BODY_TYPE_N];
		phys_prop_dist_t	pp_d[BODY_TYPE_N];
		migration_type_t	*mig_type;
		var_t				*stop_at;
	} body_disk_t;

typedef struct param
	{
		int_t id;
		body_type_t body_type;
		bool active;
		var_t epoch;
		var_t mass;
		var_t radius;
		var_t density;
		var_t cd;
		var_t gamma_stokes;
		var_t gamma_epstein;
		migration_type_t mig_type;
		var_t mig_stop_at;
		string des;
		string provdes;
		mpcorbit_type_t mpcorbit_type;
		string ref;
		string opp;
		ln_t ln;
		var_t absvismag;
		int_t compnum;
		string* name;
		var_t* ratio;
	} param_t;