// check.as - Include file for MTASC testcases providing common testing facilities
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
//
// Original author: David Rorex - drorex@gmail.com
//

#ifndef _CHECK_AS_
#define _CHECK_AS_

#define _INFO_ ' ['+__FILE__+':'+__LINE__+']'

// First argument is the expression we test, second is the result we expect
#define check_equals(obt, exp) Dejagnu.check_equals(obt, exp, _INFO_);

// First argument is the expression we test, second is the result we expect
#define xcheck_equals(obt, exp) Dejagnu.xcheck_equals(obt, exp, _INFO_);

#define check(a) Dejagnu.check(a, #a+' '+_INFO_);
#define xcheck(a) Dejagnu.xcheck(a, #a+' '+_INFO_);

#define pass(text) Dejagnu.pass(text + _INFO_)
#define xpass(text) Dejagnu.xpass(text + _INFO_)
#define fail(text) Dejagnu.fail(text + _INFO_)
#define xfail(text) Dejagnu.xfail(text + _INFO_)
#define pass(text) Dejagnu.pass(text + _INFO_)
#define untested(text) Dejagnu.untested(text + _INFO_)
#define unresolved(text) Dejagnu.unresolved(text + _INFO_)

#define check_totals(n) Dejagnu.totals(n,  _INFO_)
#define xcheck_totals(n) Dejagnu.xtotals(n,  _INFO_)

#define note(text) Dejagnu.note(text + _INFO_);


#endif
