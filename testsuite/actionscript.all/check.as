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
//

#ifndef _CHECK_AS_
#define _CHECK_AS_

#if MING_VERSION_CODE >= 00040003
# define MING_SUPPORTS_ASM
# if MING_VERSION_CODE >= 00040004
#  define MING_SUPPORTS_ASM_EXTENDS
#  define MING_SUPPORTS_ASM_GETPROPERTY
#  define MING_SUPPORTS_ASM_SETPROPERTY
#  define MING_SUPPORTS_ASM_TONUMBER
#  define MING_SUPPORTS_ASM_TOSTRING
#  if MING_VERSION_CODE >= 00040005
#   define MING_SUPPORTS_ASM_TARGETPATH
#   if MING_VERSION_CODE < 00040006
#     define MING_LOGICAL_ANDOR_BROKEN
#   endif
#   if MING_VERSION_CODE >= 00040006
#     define MING_SUPPORTS_ASM_IMPLEMENTS
#   endif
#  endif
# endif
#endif


// ONLINE mode uses DEJAGNU module
#ifdef ONLINE
# define USE_DEJAGNU_MODULE
#endif

// Use facilities provided by dejagnu shared library module
//
// NOTE: if you define USE_DEJAGNU_MODULE you
//       will also need put dejagnu_so_init.as in first frame
//       and put dejagnu_so_fini.as in last frame.
#ifdef USE_DEJAGNU_MODULE

# define trace info

#else // ndef USE_DEJAGNU_MODULE

#define pass_check(text) trace("PASSED: "+text)
#define xpass_check(text) trace("XPASSED: "+text)
#define fail_check(text) trace("FAILED: "+text)
#define xfail_check(text) trace("XFAILED: "+text)
#define info(x) trace(x)


#endif

//
// Use to get stuff in the testsuite/media.
// Example:
//
//   getUrl( MEDIA(green.swf) );
//
#define MEDIA(x) MEDIADIR + "/" + #x

#define INFO " [" + __FILE__ + ":" + __LINE__ + "]" 

#define check_totals(x) totals(x, INFO)
#define xcheck_totals(x) xtotals(x, INFO)

//
// Use check(<expression>)
//
#define check(expr)  \
	if ( expr ) pass_check(#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
	else fail_check(#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

#define xcheck(expr)  \
        if ( expr ) xpass_check(#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
        else xfail_check(#expr + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

//
// Use check_equals(<obtained>, <expected>)
//
#define check_equals(obt, exp)  \
	if ( obt == exp ) pass_check( \
		#obt + " == " + #exp + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
	else fail_check("expected: " + #exp + \
		" obtained: " + obt + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

#define xcheck_equals(obt, exp)  \
        if ( obt == exp ) xpass_check( \
                #obt + " == " + #exp + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \
        else xfail_check("expected: " + #exp + \
                " obtained: " + obt + \
		" [" + __FILE__ + ":" + __LINE__ + "]" ); \

#ifndef SUPPRESS_RCSID_DUMP
info('['+rcsid+']');
#endif

// If using the DEJAGNU_MODULE this info will be printed
// by code in dejagnu_so_init.as
#ifndef USE_DEJAGNU_MODULE
info("SWF" + OUTPUT_VERSION + " - " + System.capabilities.version + "\n");
#endif

#endif // _CHECK_AS_
