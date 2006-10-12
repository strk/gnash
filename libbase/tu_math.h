// tu_math.h	-- Willem Kokke

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// platform abstract math.h include file

#ifndef TU_MATH_H
#define TU_MATH_H

#include "tu_config.h"
#include <cmath>
#include <cfloat>

// OSX doesn't have single precision math functions defined in math.h
#ifdef __MACH__
    #define sinf sin
    #define cosf cos
    #define tanf tan
    #define asinf asin
    #define acosf acos
    #define atanf atan
    #define atan2f atan2
    #define sqrtf sqrt
    #define logf log
    #define expf exp
    #define fabsf fabs
    #define powf pow
#elif !defined(__GNUC__) //Non gnuism...
    #define sinf std::sinf
    #define cosf std::cosf
    #define tanf std::tanf
    #define asinf std::asinf
    #define acosf std::acosf
    #define atanf std::atanf
    #define atan2f std::atan2f
    #define sqrtf std::sqrtf
    #define logf std::logf
    #define expf std::expf
    #define fabsf std::fabsf
    #define powf std::powf
#endif

// isfinite() comes with C99; fake version here in case compiler lacks it.
#ifndef isfinite
#define isfinite(x) (sizeof(x) == sizeof(float) ? isfinitef(x) : isfinited(x))
#define isfinitef(x) ((x) >= -FLT_MAX && (x) <= FLT_MAX)	// NAN should fail this, yes?
#define isfinited(x) ((x) >= -DBL_MAX && (x) <= DBL_MAX)
#endif // not isfinite

#ifdef WIN32
#	define isnan _isnan
#	define isinf !_finite
#endif


#endif // TU_MATH_H
