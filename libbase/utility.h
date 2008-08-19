// utility.h --	Various little utility functions, macros & typedefs.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA



#ifndef UTILITY_H
#define UTILITY_H

// HAVE_FINITE, HAVE_PTHREADS, WIN32, NDEBUG etc.
#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cassert>
#include <cctype>
#include <string>
#include <typeinfo>
#include <cmath>
#include <boost/cstdint.hpp>
#include <algorithm> // std::min, std::max
#include <limits>

#if defined(__GNUC__) && __GNUC__ > 2
#  include <cxxabi.h>
#endif

#if defined(_WIN32) || defined(WIN32)
#ifndef NDEBUG

// On windows, replace ANSI assert with our own, for a less annoying
// debugging experience.
//int	tu_testbed_assert_break(const char* filename, int linenum, const char* expression);
#ifndef __MINGW32__
#undef assert
#define assert(x)	if (!(x)) { __asm { int 3 } }	// tu_testbed_assert_break(__FILE__, __LINE__, #x))
#endif
#endif // not NDEBUG
#endif // _WIN32


// Compile-time assert.  Thanks to Jon Jagger
// (http://www.jaggersoft.com) for this trick.
#define compiler_assert(x)	switch(0){case 0: case x:;}

// Define this to enable fast float&double to uint32 conversion.
// If the behaviour is undefined when overflow occurs with your 
// compiler, disable this macro.
#ifndef __APPLE__
	#define TRUST_FLOAT_TO_UINT32_CONVERSION  1 
#endif

namespace gnash {

// Using a possible built-in pi constant M_PI, which is not in
// the C++ standard, has no conceivable advantage, so we will use this
// one. Make it as accurate as you like.
static const double PI = 3.14159265358979323846;

// Commonly-used inlined mathematical functions are defined in
// namespace gnash::utility so that it's clear where they
// come from.
namespace utility {

inline bool isFinite(double d)
{
#if defined(HAVE_FINITE) && !defined(HAVE_ISFINITE)
    return (finite(d));
#else
    // Put using namespace std; here if you have to
    // put it anywhere.
    using namespace std;
    return (isfinite(d));
#endif
}

// TODO: deprecate this.
inline float infinite_to_fzero(float x)
{
    return utility::isFinite(x) ? x : 0.0f;
}

inline double infinite_to_zero(double x)
{
    return utility::isFinite(x) ? x : 0.0;
}

inline int iabs(int i)
{
    return (i < 0) ? -i : i;
}

template <typename T> inline T clamp(T i, T min, T max)
{
	assert( min <= max );
	return std::max<T>(min, std::min<T>(i, max));
}

inline float flerp(float a, float b, float f)
{
    return static_cast<float>((b - a) * f + a);
}

inline int frnd(float f) 
{
    return static_cast<int>(f + 0.5f);
}

} // end of namespace utility

/// Some of these functions could also be in the gnash::utility namespace,
/// although some are used so often that it would get annoying.

inline double TWIPS_TO_PIXELS(int i) 
{ 
    return static_cast<double>(i / 20.0); 
}

// truncate when overflow occurs.
inline boost::int32_t PIXELS_TO_TWIPS(double a) 
{ 
#ifdef TRUST_FLOAT_TO_UINT32_CONVERSION
    // truncate when overflow occurs.
    return static_cast<boost::int32_t>(static_cast<boost::uint32_t>(a * 20)); 
#else

    // This truncates large values without relying on undefined behaviour.
    // For very large values of 'a' it is noticeably slower than the UB
    // version (due to fmod), but should always be legal behaviour. For
    // ordinary values (within ±1.07374e+08 pixels) it is comparable to
    // the UB version for speed. Because values outside the limit are
    // extremely rare, using this safe version has no implications for
    // performance under normal circumstances.
    static const double upperUnsignedLimit =
                std::numeric_limits<boost::uint32_t>::max() + 1.0;
    static const double upperSignedLimit =
                std::numeric_limits<boost::int32_t>::max() / 20.0;
    static const double lowerSignedLimit =
                std::numeric_limits<boost::int32_t>::min() / 20.0;

    if (a >= lowerSignedLimit && a <= upperSignedLimit)
    {
        return static_cast<boost::int32_t>(a * 20.0);
    }

    // This slow truncation happens only in very unlikely cases.
    return a >= 0 ? static_cast<boost::uint32_t>(std::fmod(a * 20.0, upperUnsignedLimit))
                : - static_cast<boost::uint32_t>(std::fmod( - a * 20.0, upperUnsignedLimit));
#endif
}

inline boost::int32_t Fixed16Mul(boost::int32_t a, boost::int32_t b)
{
    // truncate when overflow occurs.
    return static_cast<boost::int32_t>((static_cast<boost::int64_t>(a) * static_cast<boost::int64_t>(b) + 0x8000) >> 16);
}

/// \brief
/// Return the smallest multiple of given base greater or equal
/// given limit
inline unsigned int
smallestMultipleContaining(unsigned int base, unsigned int x)
{
        int f=x/base;
        if ( x%base ) f++;
        return base*f;
}


/// Return (unmangled) name of this instance type. Used for
/// logging in various places.
template <class T>
std::string typeName(const T& inst)
{
	std::string typeName = typeid(inst).name();
#if defined(__GNUC__) && __GNUC__ > 2
	int status;
	char* typeNameUnmangled = 
		abi::__cxa_demangle (typeName.c_str(), NULL, NULL,
				     &status);
	if (status == 0)
	{
		typeName = typeNameUnmangled;
		free(typeNameUnmangled);
	}
#endif // __GNUC__ > 2
	return typeName;
}

/// Used in logging.
#ifdef HAVE_PTHREADS
#include <pthread.h>
#else
# ifdef _WIN32
} // end namespace gnash
extern "C" unsigned long int /* DWORD WINAPI */ GetCurrentThreadId(void);
namespace gnash {
# else
/* getpid() */
#include <sys/types.h>
#include <unistd.h>
# endif
#endif

inline unsigned long int /* pthread_t */ get_thread_id(void)
{
#ifdef HAVE_PTHREADS
# ifdef __APPLE_CC__
    return reinterpret_cast<unsigned long int>(pthread_self());
# else
    // This isn't a proper style C++ cast, but FreeBSD has a problem with
    // static_cast for this as pthread_self() returns a pointer. We can
    // use that too, this ID is only used for the log file to keep output
    // from seperare threads clear.
    return (unsigned long int)pthread_self();
# endif 
#else
# ifdef _WIN32
    return GetCurrentThreadId();
# else
    return static_cast<unsigned long int>(getpid());
# endif
#endif
}

} // namespace gnash

// Handy macro to quiet compiler warnings about unused parameters/variables.
#define UNUSED(x) (x) = (x)

// Compile-time constant size of array.
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


inline size_t bernstein_hash(const void* data_in, int size, unsigned int seed = 5381)
// Computes a hash of the given data buffer.
// Hash function suggested by http://www.cs.yorku.ca/~oz/hash.html
// Due to Dan Bernstein.  Allegedly very good on strings.
//
// One problem with this hash function is that e.g. if you take a
// bunch of 32-bit ints and hash them, their hash values will be
// concentrated toward zero, instead of randomly distributed in
// [0,2^32-1], because of shifting up only 5 bits per byte.
{
	const unsigned char* data = static_cast<const unsigned char*>(data_in);
	unsigned int h = seed;
	while (size > 0) {
		size--;
		h = ((h << 5) + h) ^ static_cast<unsigned>(data[size]);
	}

	return h;
}

#endif // UTILITY_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
