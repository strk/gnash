// check.as - Include file for SWFC testcases providing common testing facilities
//
//   Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_AS3_CHECK_AS
#define GNASH_AS3_CHECK_AS

#include "dejagnu.as"

#define DEJAGNU_OBJ import dejagnu.Dejagnu; \
		var DJ:Dejagnu;

#define INIT_DEJAGNU DJ = new Dejagnu(this);

#define _INFO_ ' ['+__FILE__+':'+__LINE__+']'

#define check_equals(a, b) DJ.check_equals(a, b, _INFO_, #a);

#define xcheck_equals(a, b) DJ.xcheck_equals(a, b, _INFO_, #a);

#define check(a) DJ.check(a, _INFO_);
#define xcheck(a) DJ.xcheck(a, _INFO_);

#define pass(text) DJ.pass(text + _INFO_)
#define xpass(text) DJ.xpass(text + _INFO_)
#define fail(text) DJ.fail(text + _INFO_)
#define xfail(text) DJ.xfail(text + _INFO_)
#define pass(text) DJ.pass(text + _INFO_)
#define untested(text) DJ.untested(text + _INFO_)
#define unresolved(text) DJ.unresolved(text + _INFO_)

#define note(text) DJ.note(text + _INFO_);

#define totals(x) DJ.totals(x, _INFO_)

#define xtotals(x) DJ.xtotals(x, _INFO_)

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)
#define MEDIA(x) QUOTEME(MEDIADIR/x)

#endif
