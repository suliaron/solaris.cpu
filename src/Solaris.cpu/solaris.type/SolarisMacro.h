// NBody settings

//#define NDIM		4		// Number of dimensions, 4 to coalesce memory copies
//#define NTILE		256
//
//#define	NVAR		2		// Number of vector variables per body (coordinate, velocity)
//#define NPAR		2		// Number of parameters per body (mass, radius)
//
//#define K			(var_t)0.01720209895
//#define K2			(var_t)0.0002959122082855911025
//
//#define	PI			(var_t)3.1415926535897932384626
//#define	TWOPI		(var_t)6.2831853071795864769253
//#define	TORAD		(var_t)0.0174532925199432957692
//#define TODEG		(var_t)57.295779513082320876798
//
//#define FOUR_PI_OVER_THREE	4.1887902047863909846168578443727

#define HANDLE_NULL(a) \
	if ((a) == NULL) { \
		Error::_errMsg = "host memory allocation"; \
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__); \
        return 1; \
	}

#define HANDLE_RESULT(result) \
	if (result == 1) { \
		Error::PushLocation(__FILE__, __FUNCTION__, __LINE__); \
		return 1; \
	}

// These macro functions must be enclosed in parentheses in order to give
// correct results in the case of a division i.e. 1/SQR(x) -> 1/((x)*(x))
#define	SQR(x)		((x)*(x))
#define	CUBE(x)		((x)*(x)*(x))
#define FORTH(x)	((x)*(x)*(x)*(x))
#define FIFTH(x)	((x)*(x)*(x)*(x)*(x))

#define MASS_STAR		1.0			// M_sol
#define MASS_JUPITER	1.0e-3		// M_sol
#define RAD_STAR		0.005		// AU

#define MASS_SUN		1.9891E+30	// kg
#define MASS_FACTOR		5e-7		// M_sol
#define MASS_MU			log(4.0)
#define MASS_S			0.3
#define MASS_MIN		1.0e-20		// M_sol
#define MASS_MAX		1.0e-19		// M_sol

#define DIST_MIN		4.5			// AU
#define DIST_MAX		15			// AU

//#define DENSITY			3000.0		// kg m-3
//#define AU				149.6e9		// m
