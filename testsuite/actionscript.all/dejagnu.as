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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//

#ifndef _DEJAGNU_AS_
#define _DEJAGNU_AS_

// Define USE_XTRACE to use "visual" trace
#ifdef USE_XTRACE
# include "xtrace.as"
# define trace xtrace
#endif

// Include the other testing file so user can mix the two if they
// want. Both start with "PASSED", so they work with DejaGnu. They
// only differ in the types of data displayed. Differnt data for
// different folks works for me as long as the standard keywords are
// used to keep DejaGnu happy.
rcsid="$Id: dejagnu.as,v 1.4 2006/10/15 02:30:55 rsavoye Exp $";

#include "check.as"

// Track our state

var passed = 0;
var failed = 0;
var xpassed = 0;
var xfailed = 0;
var untest = 0; 
var unresolv = 0;

//
// Use dejagnu(<expression>)
// Make sure your expression doesn't contain single quotes
//

#define dejagnu(expr, text)  \
 	if ( expr ) pass(text);\

// 	if ( expr ) pass(text);
// 	else fail(text);

// These are the four primary test states as required by the POSIX
// testing methodologies standard.
#define pass(text) passed++; trace("PASSED: " + text + " [" + __LINE__ + "]")
#define fail(text) failed++; trace("FAILED: " + text + " [" + __LINE__ + "]")
#define xpass(text) xpassed++; trace("XPASSED: " + text + " [" + __LINE__ + "]")
#define xfail(text) xfailed++; trace("XFAILED: " + text + " [" + __LINE__ + "]")
#define untested(text) untest++; trace("UNTESTED: " + text + " [" + __LINE__ + "]")
#define unresolved(text) unresolv++; trace("UNRESOLVED: " + text + " [" + __LINE__ + "]")

// 
#define note(text) trace(text)

#define totals() \
        trace("Totals:"); \
        trace("    passed: " + passed ); \
        trace("    failed: " + failed ); \
        if (xfailed) trace("    expected failures: " + xfailed); \
        if (xpassed) trace("    unexpected passes: " + xpassed); \
        if (untest) trace("    untested: " + untest); \
        if (unresolv) trace("    unresolved: " + unresolv); \


#endif // _DEJAGNU_AS_
