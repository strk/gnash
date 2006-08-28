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
#if defined(_WIN32) || defined(WIN32)
# ifndef _INC_WINDOWS

#  define WINAPI	__stdcall
#  define APIENTRY WINAPI
#  define CALLBACK __stdcall
#  define DECLSPEC_IMPORT __declspec(dllimport)

#  if !defined(_GDI32_)
#   define WINGDIAPI DECLSPEC_IMPORT
#  else
#   define WINGDIAPI
#  endif

# else
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
# endif

# ifndef _WCHAR_T_DEFINED
   typedef unsigned short wchar_t;
#  define _WCHAR_T_DEFINED
# endif // _WCHAR_T_DEFINED

# define PROC_NAME_PREFIX "wgl"

# include <GL/gl.h>
# include <GL/glu.h>
#endif // WIN32

#ifdef __MACH__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# define APIENTRY
# define PROC_NAME_PREFIX
#endif // __MACH__

#if !defined(WIN32) && !defined(__MACH__)
# include <GL/gl.h>
# include <GL/glx.h>
# include <GL/glu.h>
#ifndef APIENTRY
# define APIENTRY
#endif
# define PROC_NAME_PREFIX "glX"
#endif // no WIN32 or OSX

#endif // TU_OPENGL_INCLUDES_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
