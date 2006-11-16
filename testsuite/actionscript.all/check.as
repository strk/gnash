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

if ( ! dejagnu_module_initialized )
{
	trace("No properly initialized dejagnu module found.\n"
		+ " Possible reasons are:\n"
		+ " 1) this testcase was compiled using a bogus\n"
		+ "    makeswf version (up to Ming-0.4.0beta2).\n"
		+ " 2) You are using a player with bogus IMPORT \n"
		+ "    tag handling (actions in the imported movie \n"
		+ "    have not been run yet and we should be in frame2\n"
		+ "    of the importer movie so far).\n"
		+ " 4) The Dejagnu.swf file is corrupted or was not found\n"
		+ "    where expected.\n"
		+ "In any case, we will fallback to trace mode\n\n" );
}
else
{
	info = function(msg) {
		xtrace(msg);
		trace(msg);
	};
	pass_check = function (msg) {
		pass(msg);
	};
	xpass_check = function (msg) {
		xpass(msg);
	};
	fail_check = function (msg) {
		fail(msg);
	};
	xfail_check = function (msg) {
		xfail(msg);
	};
}

# define trace(x) info(msg)

#else // ndef USE_DEJAGNU_MODULE

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

#define info(x) trace(x)


#endif

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

info(rcsid);
info("SWF" + OUTPUT_VERSION + " - " + System.capabilities.version);
info("");

#endif // _CHECK_AS_
