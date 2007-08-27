// tu_math.h	-- Willem Kokke

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// platform abstract math.h include file

#ifndef TU_MATH_H
#define TU_MATH_H

#include "tu_config.h"
#include <cmath>
#include <cfloat>

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
