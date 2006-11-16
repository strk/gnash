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
# define USE_RTRACE "http://localhost/testreport.php"
//# define USE_XTRACE
#endif

// Define USE_XTRACE to use "visual" trace
#ifdef USE_XTRACE
// This is obsoleted, use USE_DEJAGNU_MODULE insted
//# include "xtrace.as"
//# define trace xtrace
#define USE_DEJAGNU_MODULE 1
#endif

// Use facilities provided by dejagnu shared library module
//
// NOTE: if you define USE_DEJAGNU_MODULE you
//       will also need put dejagnu_so_init.as in first frame
//       and put dejagnu_so_fini.as in last frame.
#ifdef USE_DEJAGNU_MODULE
// When using ming-0.4.0beta or prior, the __shared_assets
// movieclip will NOT be usable, thus we fallback to using
// bare 'trace' function
# define trace(x) if ( xtrace ) xtrace(x); else trace(x)
#endif

// Define USE_RTRACE to use "report" trace
#ifdef USE_RTRACE
# include "rtrace.as"
# define trace rtrace
#endif

// ONLINE mode only prints failures
#ifdef ONLINE
# undef pass_check
#else
# define pass_check(text) trace("PASSED: "+text)
# define xpass_check(text) trace("XPASSED: "+text)
#endif

#define fail_check(text) trace("FAILED: "+text)
#define xfail_check(text) trace("XFAILED: "+text)

//
// Use check(<expression>)
//
#define check(expr)  \
	if ( expr ) pass_check(#expr + \
		" [" + __LINE__ + "]" ); \
	else fail_check(#expr + \
		" [" + __LINE__ + "]" ); \

#define xcheck(expr)  \
        if ( expr ) xpass_check(#expr + \
                " [" + __LINE__ + "]" ); \
        else xfail_check(#expr + \
                " [" + __LINE__ + "]" ); \

//
// Use check_equals(<obtained>, <expected>)
//
#define check_equals(obt, exp)  \
	if ( obt == exp ) pass_check( \
		#obt + " == " + #exp + \
		" [" + __LINE__ + "]" ); \
	else fail_check("expected: " + #exp + \
		" obtained: " + obt + \
		" [" + __LINE__ + "]" ); \

#define xcheck_equals(obt, exp)  \
        if ( obt == exp ) xpass_check( \
                #obt + " == " + #exp + \
                " [" + __LINE__ + "]" ); \
        else xfail_check("expected: " + #exp + \
                " obtained: " + obt + \
                " [" + __LINE__ + "]" ); \

trace(rcsid);
trace("SWF" + OUTPUT_VERSION + " - " + System.capabilities.version);
trace("");

#endif // _CHECK_AS_
