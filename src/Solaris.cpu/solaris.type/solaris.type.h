typedef double		ttt_t;
typedef double		var_t;

struct double2
{
    double x, y;
} double2;

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

typedef bool		bool_t;

typedef int			int_t;

typedef struct int2
	{
		int x;
		int y;
	} int2_t;

typedef enum body_type
{
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
SMA,
ECC,
INC,
PERI,
NODE,
MEAN
} orbelem_name_t;

typedef enum phys_prop_name
{
MASS,
RADIUS,
DENSITY,
DRAG_COEFF
} phys_prop_name_t;



	typedef struct orbelem
{
//! Semimajor-axis of the body
var_t sma;
//! Eccentricity of the body
var_t ecc;
//! Inclination of the body
var_t inc;
//! Argument of the pericenter
var_t peri;
//! Longitude of the ascending node
var_t node;
//! Mean anomaly
var_t mean;
} orbelem_t;

typedef enum migration_type
		{
			MIGRATION_TYPE_NO,
			MIGRATION_TYPE_TYPE_I,
			MIGRATION_TYPE_TYPE_II
		} migration_type_t;

typedef enum mpcorbit_type {
    MPCORBIT_TYPE_UNDEFINED						= 0,
    MPCORBIT_TYPE_ATEN							= 2,
    APOLLO										= 3,
    AMOR										= 4,
    OBJECTWITHQLT1_665							= 5,
    HUNGARIA									= 6,
    PHOCAEA										= 7,
    HILDA										= 8,
    JUPITERTROJAN								= 9,
    CENTAUR										= 10,
    PLUTINO										= 14,
    OTHERRESONANTTNO							= 15,
    CUBEWANO									= 16,
    SCATTEREDDISK								= 17,
    OBJECTISNEO									= 2048,
    OBJECTIS1KMORLARGERNEO						= 4096,
    ONEOPPOSITIONOBJECTSEENATEARLIEROPPOSITION	= 8192,
    CRITICALLISTNUMBEREDOBJECT					= 16384,
    OBJECTISPHA									= 32768
} mpcorbit_type_t;

typedef enum ln {
	UndefinedLn,
	L1,
	L2,
	L3,
	L4,
	L5
} ln_t;

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
			//! Unique number to identify the object
			int_t id;
			//! Type of the body
			body_type_t body_type;
			//! Indicates whether the body is participating in the simulation or not (i.e. escaped)
			bool active;
			//! The initial conditions are valid for this epoch
			var_t epoch;
			//! Mass of body in M_sol
			var_t mass;
			//! Radius of body in AU
			var_t radius;
			//! Density of body in M_sol AU-3
			var_t density;
			//! Drag coefficient for the Stokes-type drag
			var_t cd;
			//! Used for the drag force
			var_t gamma_stokes;
			//! Used for the drag force
			var_t gamma_epstein;
			//! Type of the migration
			migration_type_t mig_type;
			//! The migration stop at this distance measured from the star
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

