// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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

// This counts calls to toString() and valueOf() regularly to check that native
// methods are correctly applied. So it saves much effort if new tests are added
// after the end of the present ones.

rcsid="ASnative.as";
#include "check.as"

/// Global

// Old ASnew
a = ASnative(2, 0);

var result = "";
function func() {
    if (a()) { result = "true"; }
    else { result =  "false"; }
};

func();
check_equals (result, "false");
discard = new func();
xcheck_equals (result, "true");

a = ASnative (100, 0); // escape
check_equals(a(" "), "%20");
a = ASnative (100, 1); // unescape
check_equals(a("%20"), " ");
a = ASnative (100, 2); // parseInt
check_equals(a("  566765s"), "566765");
a = ASnative (100, 3); // parseFloat
check_equals(a("8.4e6"), 8.4e6);

// Do this first to make sure ASnative is
// registered before Date class itself is loaded (Gnash loads
// on demand).
a = ASnative(103, 257);
check_equals (a(65, 1, 1, 1, 1, 1, 1), Date.UTC(65, 1, 1, 1, 1, 1, 1));

e = ASnative(103, 256); // _global.Date
xcheck_equals(e().valueOf(), Date().valueOf());
g = e;
xcheck_equals(typeOf(g().valueOf()), 'string');

xcheck_equals(e(100000).valueOf(), Date().valueOf());

g = e();
check_equals(typeOf(g.getMilliseconds), 'undefined');

f = new e(100000000); // not instantiatable
xcheck_equals(typeof(f), 'object');
check_equals(typeof(f.getMilliseconds()), 'undefined');
check_equals(typeof(f().getMilliseconds()), 'undefined');

d = new Date (123456789);

d.a = ASnative(103, 0);
check_equals (d.a(), d.getFullYear());
d.a = ASnative(103, 1);
check_equals (d.a(), d.getYear());
d.a = ASnative(103, 2);
check_equals (d.a(), d.getMonth());
d.a = ASnative(103, 3);
check_equals (d.a(), d.getDate());
d.a = ASnative(103, 4);
check_equals (d.a(), d.getDay());
d.a = ASnative(103, 5);
check_equals (d.a(), d.getHours());
d.a = ASnative(103, 6);
check_equals (d.a(), d.getMinutes());
d.a = ASnative(103, 7);
check_equals (d.a(), d.getSeconds());
d.a = ASnative(103, 8);
check_equals (d.a(), d.getMilliseconds());
d.a = ASnative(103, 16);
check_equals (d.a(), d.getTime());
d.a = ASnative(103, 18);
check_equals (d.a(), d.getTimezoneOffset());
d.a = ASnative(103, 19);
check_equals (d.a(), d.toString());
d.a = ASnative(103, 128);
check_equals (d.a(), d.getUTCFullYear());
d.a = ASnative(103, 129);
check_equals (d.a(), d.getUTCYear());
d.a = ASnative(103, 130);
check_equals (d.a(), d.getUTCMonth());
d.a = ASnative(103, 131);
check_equals (d.a(), d.getUTCDate());
d.a = ASnative(103, 132);
check_equals (d.a(), d.getUTCDay());
d.a = ASnative(103, 133);
check_equals (d.a(), d.getUTCHours());
d.a = ASnative(103, 134);
check_equals (d.a(), d.getUTCMinutes());
d.a = ASnative(103, 135);
check_equals (d.a(), d.getUTCSeconds());
d.a = ASnative(103, 136);
check_equals (d.a(), d.getUTCMilliseconds());
d.a = ASnative(103, 1);
check_equals (d.a(), d.getYear());

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
check_equals(a(func), 0.3); // abs
check_equals(a(0.3), 0.3); // abs

a = ASnative(200, 1);
check_equals(a(func, func + 1), 0.3); // min
check_equals(a(0.3, 1.3), 0.3); // min

a = ASnative(200, 2);
check_equals(a(func, func + 1), 1.3); // max
check_equals(a(0.3, 1.3), 1.3); // max

a = ASnative(200, 3);
check_equals(a(func).toString(), "0.29552020666134"); // sin
check_equals(a(0.3).toString(), "0.29552020666134"); // sin

a = ASnative(200, 4);
check_equals(a(func).toString(), "0.955336489125606"); // cos
check_equals(a(0.3).toString(), "0.955336489125606"); // cos

a = ASnative(200, 5);
check_equals(a(func, func + 1).toString(), "0.226798848053886"); // atan2
check_equals(a(0.3, 1.3).toString(), "0.226798848053886"); // atan2

a = ASnative(200, 6);
check_equals(a(func).toString(), "0.309336249609623"); // tan
a = ASnative(200, 7);
check_equals(a(func).toString(), "1.349858807576"); // exp
a = ASnative(200, 8);
check_equals(a(func).toString(), "-1.20397280432594"); // log
a = ASnative(200, 9);
check_equals(a(func).toString(), "0.547722557505166"); // sqrt
a = ASnative(200, 10);
check_equals(a(func), 0); // round
a = ASnative(200, 11);
check(a(func) >= 0 && a(func < 1)); // random
a = ASnative(200, 12);
check_equals(a(func), 0); // floor
a = ASnative(200, 13);
check_equals(a(func), 1); // ceil
a = ASnative(200, 14);
check_equals(a(func).toString(), "0.291456794477867"); // atan
a = ASnative(200, 15);
check_equals(a(func).toString(), "0.304692654015398"); // asin
a = ASnative(200, 16);
check_equals(a(func).toString(), "1.2661036727795"); // acos
a = ASnative(200, 17);
check_equals(a(func, func + 1).toString(), "0.209053590580785"); // pow
a = ASnative(200, 18);
check_equals(a(func), false); // isNan
a = ASnative(200, 19);
check_equals(a(func), true); // isFinite

check_equals (countVO, 25); // calls to valueOf.
check_equals (countTS, 0); // calls to toString.

// String functions (call toString)

a = ASnative(251, 3); // String.toUpperCase
check_equals(a("Hello World"), "_LEVEL0");
a = ASnative(102, 0); // SWF5 to upper
check_equals(a("Hello World"), "_LEVEL0");


// SWF5 has problems with UTF-8, tested in String.as.
// No need to test here as well.

check_equals (countTS, 0); // calls to toString.

#if OUTPUT_VERSION > 5
func.a = ASnative(251, 3); // String.toUpperCase
check_equals(func.a(), "GNASH MUST WORK! ÖÜÄÄ€€");

func.a = ASnative(251, 4); // String.toLowerCase
check_equals(func.a(), "gnash must work! öüää€€");

// Check calls to toString.
check_equals (countTS, 2);
#endif

func.a = ASnative(102, 0); // SWF5 to upper
check_equals(func.a(), "GNASH MUST WORK! öÜäÄ€€");

func.a = ASnative(102, 1); // SWF5 to lower
check_equals(func.a(), "gnash must work! öÜäÄ€€");

// Check calls to toString.
#if OUTPUT_VERSION > 5
check_equals (countTS, 4);
#else
check_equals (countTS, 2);
#endif

// Stage
st = ASnative(666, 2);
st("exactFit");
st = ASnative(666, 1);
check_equals (st(), "exactFit");

st = ASnative(666, 4);
st("BRL");

st = ASnative(666, 3);
check_equals (st(), "LRB");

//Stage.width - read only!
st = ASnative(666, 6);
st(402);
st = ASnative(666, 5);
check_equals (st(), 640);

// Stage.height - read only!
st = ASnative(666, 8);
st(402);
st = ASnative(666, 7);
check_equals (st(), 480);

// Stage.showMenu
st = ASnative(666, 10);
st = ASnative(666, 9);

#if OUTPUT_VERSION > 5
check_equals (countTS, 4);
#else
check_equals (countTS, 2);
#endif

check_equals (countVO, 25);

/// SharedObject undocumented functions.

a = ASnative(2106, 202);
f = a("level1/level2/settings", "/", undefined); 
xcheck_equals(typeof(f), "null");

a = ASnative(2106, 204);
f = new SharedObject;
check_equals (typeof(f.data), "undefined");
ret = a(f, "level1/level2/settings", "/", undefined); 
xcheck_equals(ret, true);
xcheck_equals (typeof(f.data), "object");

// Check that ASnative returns a new function, not the same one.
a = ASnative(2106, 204);
b = ASnative(2106, 204);
#if OUTPUT_VERSION < 6
check(a == b);
#else
check(a != b);
#endif

/// Test ASconstructor
//
/// The important things seem to be:
/// 1. a new prototype is created every time; it's not the same object.
/// 2. native functions may only work with an appropriate prototype.

// This is _global.Number
f = ASconstructor(106, 2);
check_equals(typeof(f), "function");
check_equals(typeof(f.prototype), "object");

// Attach number natives and it works.
ASSetNative(f.prototype, 106, "valueOf,toString");

obj = new f(6);
check_equals(obj.__proto__, f.prototype);
#if OUTPUT_VERSION > 5
check(obj.__proto__ != Number.prototype);
#else
check_equals(obj.__proto__, undefined);
#endif

check_equals(typeof(obj), "object");
check_equals(obj.toString(), "6");

// Attach boolean natives and it fails.
ASSetNative(f.prototype, 107, "valueOf,toString");
check_equals(typeof(obj), "object");
check_equals(obj.toString(), undefined);

// Attach number natives to prototype again and it works.
ASSetNative(f.prototype, 106, "valueOf,toString");
check_equals(obj.toString(), "6");

g = ASnative(106, 2);
check_equals(typeof(g), "function");
check_equals(typeof(g.prototype), "undefined");

// This is the ASnative function Number.toString. It does not attach
// Number.prototype, so the Number.toString function fails.
f = ASconstructor(106, 1);
ASSetNative(f.prototype, 106, "valueOf,toString");
check_equals(typeof(f), "function");
check_equals(typeof(f.prototype), "object");
obj = new f(6);
xcheck_equals(typeof(obj), "object");
check_equals(obj.toString(), undefined);

obj = new f();
check_equals(obj.toString(), undefined);

ba = ASnative;
ASnative = 78;

// ASconstructor doesn't rely on ASnative.
f = ASconstructor(106, 2);
check_equals(typeof(f), "function");
check_equals(typeof(f.prototype), "object");

g = ASnative(106, 2);
check_equals(typeof(g), "undefined");
check_equals(typeof(g.prototype), "undefined");

ASnative = ba;

#if OUTPUT_VERSION > 5
check_totals(104);
#else
check_totals(101);
#endif
