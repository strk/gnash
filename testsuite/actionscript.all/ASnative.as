// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

rcsid="$Id: ASnative.as,v 1.1 2008/04/15 11:41:49 bwy Exp $";
#include "check.as"

countVO = 0;
countTS = 0;

// ASNative Math (call valueOf)

func = {};
func.valueOf = function () {
    //trace ("valueOf()");
    countVO++;
    return 0.3;
};

func.toString = function () {
    //trace ("toString()");
    countTS++;
    return "gNaSh mUsT woRK! öÜäÄ€€";
};

a = ASnative(200, 0);
xcheck_equals(a(func), 0.3); // abs
xcheck_equals(a(0.3), 0.3); // abs

a = ASnative(200, 1);
xcheck_equals(a(func, func + 1), 0.3); // min
xcheck_equals(a(0.3, 1.3), 0.3); // min

a = ASnative(200, 2);
xcheck_equals(a(func, func + 1), 1.3); // max
xcheck_equals(a(0.3, 1.3), 1.3); // max

a = ASnative(200, 3);
xcheck_equals(a(func).toString(), "0.29552020666134"); // sin
xcheck_equals(a(0.3).toString(), "0.29552020666134"); // sin

a = ASnative(200, 4);
xcheck_equals(a(func).toString(), "0.955336489125606"); // cos
xcheck_equals(a(0.3).toString(), "0.955336489125606"); // cos

a = ASnative(200, 5);
xcheck_equals(a(func, func + 1).toString(), "0.226798848053886"); // atan2
xcheck_equals(a(0.3, 1.3).toString(), "0.226798848053886"); // atan2

a = ASnative(200, 6);
xcheck_equals(a(func).toString(), "0.309336249609623"); // tan
a = ASnative(200, 7);
xcheck_equals(a(func).toString(), "1.349858807576"); // exp
a = ASnative(200, 8);
xcheck_equals(a(func).toString(), "-1.20397280432594"); // log
a = ASnative(200, 9);
xcheck_equals(a(func).toString(), "0.547722557505166"); // sqrt
a = ASnative(200, 10);
xcheck_equals(a(func), 0); // round
a = ASnative(200, 11);
xcheck(a(func) >= 0 && a(func < 1)); // random
a = ASnative(200, 12);
xcheck_equals(a(func), 0); // floor
a = ASnative(200, 13);
xcheck_equals(a(func), 1); // ceil
a = ASnative(200, 14);
xcheck_equals(a(func).toString(), "0.291456794477867"); // atan
a = ASnative(200, 15);
xcheck_equals(a(func).toString(), "0.304692654015398"); // asin
a = ASnative(200, 16);
xcheck_equals(a(func).toString(), "1.2661036727795"); // acos
a = ASnative(200, 17);
xcheck_equals(a(func, func + 1).toString(), "0.209053590580785"); // pow
a = ASnative(200, 18);
xcheck_equals(a(func), false); // isNan
a = ASnative(200, 19);
xcheck_equals(a(func), true); // isFinite

xcheck_equals (countVO, 25); // calls to valueOf.
check_equals (countTS, 0); // calls to toString.

// String functions (call toString)

a = ASnative(251, 3); // String.toUpperCase
xcheck_equals(a("Hello World"), "_LEVEL0");
a = ASnative(102, 0); // SWF5 to upper
xcheck_equals(a("Hello World"), "_LEVEL0");


// SWF5 has problems with UTF-8, tested in String.as.
// No need to test here as well.

#if OUTPUT_VERSION > 5
func.a = ASnative(251, 3); // String.toUpperCase
xcheck_equals(func.a(), "GNASH MUST WORK! ÖÜÄÄ€€");

func.a = ASnative(251, 4); // String.toLowerCase
xcheck_equals(func.a(), "gnash must work! öüää€€");
#endif

func.a = ASnative(102, 0); // SWF5 to upper
xcheck_equals(func.a(), "GNASH MUST WORK! öÜäÄ€€");

func.a = ASnative(102, 1); // SWF5 to lower
xcheck_equals(func.a(), "gnash must work! öÜäÄ€€");

#if OUTPUT_VERSION > 5
xcheck_equals (countTS, 4);
#else
xcheck_equals (countTS, 2);
#endif

xcheck_equals (countVO, 25);

#if OUTPUT_VERSION > 5
check_totals(36);
#else
check_totals(34);
#endif
