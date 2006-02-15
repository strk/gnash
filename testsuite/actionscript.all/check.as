// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef _CHECK_AS_
#define _CHECK_AS_

// ONLINE mode uses XTRACE
#ifdef ONLINE
# define USE_XTRACE
#endif

// Define USE_XTRACE to use "visual" trace
#ifdef USE_XTRACE
# include "xtrace.as"
# define trace xtrace
#endif

// ONLINE mode only prints failures
#ifdef ONLINE
# undef pass_check
#else
# define pass_check(text) trace("PASSED: "+text)
#endif

#define fail_check(text) trace("FAILED: "+text+" - SWF"+OUTPUT_VERSION+" - "+System.capabilities.version)

//
// Use check(<expression>)
//
#define check(expr)  \
	if ( expr ) pass_check(#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
	else fail_check(#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

#endif // _CHECK_AS_
