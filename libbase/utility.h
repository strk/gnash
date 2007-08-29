// utility.h --	Various little utility functions, macros & typedefs.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#include "tu_config.h"
#include "tu_math.h"
#include "tu_types.h"
#include "tu_swap.h"

#include <cassert>
#include <cctype>
#include <string>
#include <typeinfo>

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


//
// new/delete wackiness -- if USE_DL_MALLOC is defined, we're going to
// try to use Doug Lea's malloc as much as possible by overriding the
// default operator new/delete.
//
#ifdef USE_DL_MALLOC

void*	operator new(size_t size);
void	operator delete(void* ptr);
void*	operator new[](size_t size);
void	operator delete[](void* ptr);

#else	// not USE_DL_MALLOC

// If we're not using DL_MALLOC, then *really* don't use it: #define
// away dlmalloc(), dlfree(), etc, back to the platform defaults.
#define dlmalloc	malloc
#define dlfree	free
#define dlrealloc	realloc
#define dlcalloc	calloc
#define dlmemalign	memalign
#define dlvalloc	valloc
#define dlpvalloc	pvalloc
#define dlmalloc_trim	malloc_trim
#define dlmalloc_stats	malloc_stats

#endif	// not USE_DL_MALLOC


#ifndef M_PI
#define M_PI 3.141592654
#endif // M_PI

//
// some misc handy math functions
//

inline float infinite_to_fzero(float x) {	return isfinite(x) ? x : 0.0f; }
inline int	imax(int a, int b) { if (a < b) return b; else return a; }
inline int	imin(int a, int b) { if (a < b) return a; else return b; }

inline int	iclamp(int i, int min, int max) {
	assert( min <= max );
	return imax(min, imin(i, max));
}

inline float	fclamp(float f, float xmin, float xmax) {
	assert( xmin <= xmax );
	return fmax(xmin, fmin(f, xmax));
}

inline float flerp(float a, float b, float f) { return (b - a) * f + a; }

//This is from C99.
const float LN_2 = 0.693147180559945f;
// the overridden log(f) will use logf IFF f is a float
#ifndef HAVE_LOG2
inline double	log2(double f) { return std::log(f) / LN_2; }
#endif
//exp2 might be missing on Net-/OpenBSD.
#ifndef HAVE_EXP2
inline double	exp2(double x) { return std::pow((double)2, double(x)); }
#endif
inline int	frnd(float f) { return (int)(f + 0.5f); }	// replace with inline asm if desired
#ifndef HAVE_TRUNC
inline double trunc(double x) ( x < 0 ?  -(std::floor(-x)) : std::floor(x) )
#endif

// Handy macro to quiet compiler warnings about unused parameters/variables.
#define UNUSED(x) (x) = (x)


// Compile-time constant size of array.
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


inline size_t	bernstein_hash(const void* data_in, int size, unsigned int seed = 5381)
// Computes a hash of the given data buffer.
// Hash function suggested by http://www.cs.yorku.ca/~oz/hash.html
// Due to Dan Bernstein.  Allegedly very good on strings.
//
// One problem with this hash function is that e.g. if you take a
// bunch of 32-bit ints and hash them, their hash values will be
// concentrated toward zero, instead of randomly distributed in
// [0,2^32-1], because of shifting up only 5 bits per byte.
{
	const unsigned char*	data = (const unsigned char*) data_in;
	unsigned int	h = seed;
	while (size > 0) {
		size--;
		h = ((h << 5) + h) ^ (unsigned) data[size];
	}

	return h;
}


inline size_t	sdbm_hash(const void* data_in, int size, unsigned int seed = 5381)
// Alternative: "sdbm" hash function, suggested at same web page
// above, http::/www.cs.yorku.ca/~oz/hash.html
//
// This is somewhat slower, but it works way better than the above
// hash function for hashing large numbers of 32-bit ints.
{
	const unsigned char*	data = (const unsigned char*) data_in;
	unsigned int	h = seed;
	while (size > 0) {
		size--;
		h = (h << 16) + (h << 6) - h + (unsigned) data[size];
	}

	return h;
}

inline size_t	bernstein_hash_case_insensitive(const void* data_in, int size, unsigned int seed = 5381)
// Computes a hash of the given data buffer; does tolower() on each
// byte.  Hash function suggested by
// http://www.cs.yorku.ca/~oz/hash.html Due to Dan Bernstein.
// Allegedly very good on strings.
{
	const unsigned char*	data = (const unsigned char*) data_in;
	unsigned int	h = seed;
	while (size > 0) {
		size--;
		h = ((h << 5) + h) ^ (unsigned) std::tolower(data[size]);		
	}

	// Alternative: "sdbm" hash function, suggested at same web page above.
	// h = 0;
	// for bytes { h = (h << 16) + (h << 6) - hash + *p; }

	return h;
}

/// Dump the internal statistics from malloc() so we can track memory leaks
void dump_memory_stats(const char *from, int line, const char *label);

/// Return (unmangled) name of this instance type
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

namespace gnash {

/// VERY crude sample-rate and steroe conversion. Converts input data to 
/// output format.
//
/// @param adjusted_data
/// Where the converted data is placed.
///
/// @param adjusted_size
/// The size of the converted data.
///
/// @param data
/// Data that needs to be converted.
///
/// @param sample_count
/// The datas current sample count.
/// 
/// @param sample_size
/// The datas current sample size.
///
/// @param sample_rate
/// The datas current sample rate.
///
/// @param stereo
/// Whether the current data is in stereo
///
/// @param m_sample_rate
/// The samplerate we which to convert to.
///
/// @param m_stereo
/// Do we want the output data to be in stereo?
///
void	convert_raw_data(int16_t** adjusted_data,
		  int* adjusted_size, void* data, int sample_count,
		  int sample_size, int sample_rate, bool stereo,
		  int m_sample_rate, bool m_stereo);
} // namespace gnash 

#endif // UTILITY_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
