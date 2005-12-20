// ogl.h	-- by Thatcher Ulrich <tu@tulrich.com>
//              -- Willem Kokke <willem@mindparity.com>

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// platform independent opengl includes

#ifndef TU_OPENGL_INCLUDES_H
#define TU_OPENGL_INCLUDES_H

#include "tu_config.h"

// WIN32 includes.  We don't want to have to include windows.h because
// it's such a pig, so #define a couple things that are required to
// make the gl.h stuff work.
#ifdef WIN32
#	ifndef _INC_WINDOWS

#		define WINAPI	__stdcall
#		define APIENTRY WINAPI
#		define CALLBACK __stdcall
#		define DECLSPEC_IMPORT __declspec(dllimport)

#		if !defined(_GDI32_)
#			define WINGDIAPI DECLSPEC_IMPORT
#		else
#			define WINGDIAPI
#		endif

#	else
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif

#	ifndef _WCHAR_T_DEFINED
		typedef unsigned short wchar_t;
#		define _WCHAR_T_DEFINED
#	endif // _WCHAR_T_DEFINED

#	define PROC_NAME_PREFIX "wgl"

#	include <GL/gl.h>
#	include <GL/glu.h>
#endif // WIN32

#ifdef __MACH__
#	include <OpenGL/gl.h>
#	include <OpenGL/glu.h>
#	define APIENTRY
#	define PROC_NAME_PREFIX
#endif // __MACH__

#if !defined(WIN32) && !defined(__MACH__)
#	include <GL/gl.h>
#	include <GL/glu.h>
#ifndef APIENTRY
#	define APIENTRY
#endif
#	define PROC_NAME_PREFIX "glX"
#endif // no WIN32 or OSX

// GL extension constants...
// WK: is there no glext.h equivalent for this?

#	define GL_VERTEX_ARRAY_RANGE_NV          0x851D
#	define GL_VERTEX_ARRAY_RANGE_LENGTH_NV   0x851E
#	define GL_VERTEX_ARRAY_RANGE_VALID_NV    0x851F
#	define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV 0x8520
#	define GL_VERTEX_ARRAY_RANGE_POINTER_NV  0x8521

#	define GL_VERTEX_ARRAY_RANGE_WITHOUT_FLUSH_NV	0x8533

#	define GL_TEXTURE0_ARB                   0x84C0
#	define GL_TEXTURE1_ARB                   0x84C1
#	define GL_MAX_TEXTURE_UNITS_ARB          0x84E2

#	define GL_CLAMP_TO_EDGE                  0x812F

#	define GL_ALL_COMPLETED_NV               0x84F2

#endif // TU_OPENGL_INCLUDES_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
