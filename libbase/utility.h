// utility.h --	Various little utility functions, macros & typedefs.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_UTILITY_H
#define GNASH_UTILITY_H

// HAVE_FINITE, HAVE_PTHREADS, WIN32, NDEBUG etc.
#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <string>
#include <typeinfo>

#if defined(__GNUC__) && __GNUC__ > 2
#  include <cxxabi.h>
#endif

#if defined(_WIN32) || defined(WIN32)
#ifndef NDEBUG

// On windows, replace ANSI assert with our own, for a less annoying
// debugging experience.
#ifndef __MINGW32__
#undef assert
#define assert(x)	if (!(x)) { __asm { int 3 } }
#endif
#endif // not NDEBUG
#endif // _WIN32

#ifdef __amigaos4__
#include <stdio.h> //for FILE * in tu_file.h
#include <fcntl.h> //for fcntl in Socket.cpp
#include <netdb.h> //for hostent in Socket.cpp
#include <netinet/tcp.h> //for TCP_NODELAY in Socket.cpp
#undef UNUSED //to avoid "already defined" messages
#define SHUT_RDWR 0

namespace std
{
	typedef std::basic_string<wchar_t> wstring;
};
#endif

#if defined(__HAIKU__)
namespace std {
	class wstring : public std::basic_string<char>
	{
	public:
		wstring(const char *t)
			: std::basic_string<char>(t)
		{
		}
		wstring()
		{
		}
		wstring(const wstring &that)
			: std::basic_string<char>(that.c_str())
		{
		}
		wstring(const std::basic_string<char> &that)
			: std::basic_string<char>(that)
		{
		}
	};
};
#endif

namespace gnash {

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
		std::free(typeNameUnmangled);
	}
#endif // __GNUC__ > 2
	return typeName;
}

} // namespace gnash

// Handy macro to quiet compiler warnings about unused parameters/variables.
#define UNUSED(x) static_cast<void>((x))

#endif 


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
