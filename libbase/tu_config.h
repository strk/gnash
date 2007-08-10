// tu_config.h	-- by Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Generic include file for configuring tu-testbed.


#ifndef TU_CONFIG_H
#define TU_CONFIG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dlmalloc.h"

// #define these in compatibility_include.h if you want something different.
#ifndef tu_malloc
#define tu_malloc(size) dlmalloc(size)
#endif
#ifndef tu_realloc
#define tu_realloc(old_ptr, new_size, old_size) dlrealloc(old_ptr, new_size)
#endif
#ifndef tu_free
#define tu_free(old_ptr, old_size) dlfree(old_ptr)
#endif

// tu_error_exit() is for fatal errors; it should not return!
// You can #define it to something else in compatibility_include.h; e.g. you could
// throw an exception, halt, whatever.
#ifndef tu_error_exit
#include <cstdlib>	// for exit()
#include <cstdio>
#define tu_error_exit(error_code, error_message) { fprintf(stderr, error_message); std::exit(error_code); }
#endif

// define TU_CONFIG_LINK_TO_LIBPNG to 0 to exclude libpng code from
// your build.  Be aware of what you're doing -- it may break
// features!
#ifndef TU_CONFIG_LINK_TO_LIBPNG
#define TU_CONFIG_LINK_TO_LIBPNG 1
#endif

// define TU_CONFIG_LINK_TO_LIBXML to 1 to include XML support in,
// depending on the GNOME libxml library.
#ifndef TU_CONFIG_LINK_TO_LIBXML
#define TU_CONFIG_LINK_TO_LIBXML 1
#endif

#ifdef _MSC_VER
	#ifdef BUILDING_DLL
		#define DSOEXPORT __declspec(dllexport)
	#else
		// Temporarily commented because of VC++ compiler problems 
		#define DSOEXPORT // __declspec(dllimport)
	#endif

	#define DSOLOCAL
#elif defined(__OS2__)
	#ifdef BUILDING_DLL
		#define DSOEXPORT __declspec(dllexport)
	#else
		// Temporarily commented because of VC++ compiler problems 
		#define DSOEXPORT // __declspec(dllimport)
	#endif

	#define DSOLOCAL

#else
	#ifdef HAVE_GNUC_VISIBILITY
		#define DSOEXPORT __attribute__ ((visibility("default")))
		#define DSOLOCAL __attribute__ ((visibility("hidden")))
	#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550) /* Sun Studio >= 8 */
		#define DSOEXPORT __global
		#define DSOLOCAL __hidden
	#else
		#define DSOEXPORT
		#define DSOLOCAL
	#endif
#endif

#endif // TU_CONFIG_H
