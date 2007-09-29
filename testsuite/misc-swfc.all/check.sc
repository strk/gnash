// check.as - Include file for SWFC testcases providing common testing facilities
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

#define check_equals(a, b) Dejagnu.check_equals(a, b, _INFO_, #a);
//#define check_equals(a, b, msg) Dejagnu.check(a, b, msg + _INFO_);

#define xcheck_equals(a, b) Dejagnu.xcheck_equals(a, b, _INFO_, #a);
//#define xcheck_equals(a, b, msg) Dejagnu.check(a, b, msg + _INFO_);

#define check(a) Dejagnu.check(a, _INFO_);
#define xcheck(a) Dejagnu.xcheck(a, _INFO_);

#define pass(text) Dejagnu.pass(text + _INFO_)
#define xpass(text) Dejagnu.xpass(text + _INFO_)
#define fail(text) Dejagnu.fail(text + _INFO_)
#define xfail(text) Dejagnu.xfail(text + _INFO_)
#define pass(text) Dejagnu.pass(text + _INFO_)
#define untested(text) Dejagnu.untested(text + _INFO_)
#define unresolved(text) Dejagnu.unresolved(text + _INFO_)

#define note(text) Dejagnu.note(text + _INFO_);

#define totals(x) Dejagnu.totals(x, _INFO_)

#define xtotals(x) Dejagnu.xtotals(x, _INFO_)

#define MEDIA(x) MEDIADIR/x

#endif
