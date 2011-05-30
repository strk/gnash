// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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

// Test case for Boolean ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Try.as";
#include "check.as"

// Some of the test variants.
// Try catch finally (no throw)
// Try (throw) catch finally
// Try finally
// Try (throw) finally
// Try catch
// Try (throw) catch

#if MING_VERSION_CODE >= 00040100

throwfunc = function()
{
    throw "try";
};

throwfunc2 = function()
{
    try {
        throw "try";
    }
    catch (g) {};
};

throwfunc3 = function()
{
    try {
        throw "throw";
    }
    finally {
        return "finally";
    };
};

r = "1: ";
try { r +="try "; r +="body "; }
catch (a) { r +="catch "; r += a + " "; }
finally { r +="finally "; };
r += ".";

check_equals(r, "1: try body finally .");

r = "2: ";
try { r += "try "; throw ("thrown"); r += "body "; }
catch (b) { r += "catch "; r += b + " "; }
finally { r += "finally "; };
r += ".";
check_equals(r, "2: try catch thrown finally .");

r = "3: ";
try { r += "try "; r += "body "; }
finally { r += "finally "; };
r += ".";
check_equals(r, "3: try body finally .");

// This will interrupt execution without the enclosing try/catch.
try {
    r = "4: ";
    try { r += "try "; throw ("thrown"); r += "body "; }
    finally { r += "finally "; };
}
catch (c) { trace (c); r += c + " "; };
r += ".";
check_equals(r, "4: try finally thrown .");

// Also check that the exception is not
// undefined if nothing is thrown
d = "pre-existing variable d";
r = "5: ";
try { r += "try "; r += "body "; }
catch (d) { r += "catch "; r+= d + " "; };
r += ". ";
r += d;
check_equals(r, "5: try body . pre-existing variable d");

r = "6: ";
try { r += "try "; throw ("thrown"); r += "body "; }
catch (e) { r += "catch "; r += e + " "; };
r += ".";
check_equals(r, "6: try catch thrown .");

try {
    try {
        r = "7: ";
        try { r += "try "; throw ("thrown"); r += "body "; }
        finally { r += "finally "; };
    }
    finally { r += "finally2 "; };
}
catch (f) { r += f + " "; };
r += ".";
check_equals(r, "7: try finally finally2 thrown .");


try {
    try {
        r = "8: ";
        try { r += "try "; throw ("thrown"); r += "body "; }
        finally { r += "finally "; };
    }
    finally {
        r += "finally2 ";
        try {
            r += "try2 ";
        }
        catch (h) { r += "catch2 "; r += h +" "; };
    };
}
catch (i) { r += i + " "; };
r += ".";
check_equals(r, "8: try finally finally2 try2 thrown .");

try {
    try {
        r = "9: ";
        try { r += "try "; throw ("thrown"); r += "body "; }
        finally { r += "finally "; };
    }
    catch (j) { r += "catch "; r += j + " "; }
    finally {
        r += "finally2 ";
        try {
            r += "try2 ";
        }
        catch (k) { r += "catch2 "; r += k +" "; };
    };
}
catch (l) { r+= "catch3 "; r += l + " "; };
r += ".";
check_equals(r, "9: try finally catch thrown finally2 try2 .");

r = "10: ";
try {
    try { throw "try"; }
    catch (e) { throw "catch"; }
    finally { throw "finally"; };
}
catch (m) { r+= "catch " + m; };
r += ".";
check_equals(r, "10: catch finally.");

r = "11: ";
try {
    try { throw "try"; }
    catch (e) { throw "catch"; }
    finally { throw "finally"; };
}
catch (m) { r+= "catch " + m; };
r += ".";
check_equals(r, "11: catch finally.");

throwNoCatchFunc = function()
{
    throw "throw";
    return "return";
};

r = "12: ";
try {
    r += "try ";
    r += throwNoCatchFunc() + " ";
}
catch (n) {
    r += n + " ";
}
finally {
    r += "finally";
};
r += ".";
xcheck_equals(r, "12: try throw finally.");


r = "13: ";
try {
  r += "try ";
  r += throwfunc3();
} catch (e) {
  r += " " + e;
};
r += ".";
xcheck_equals(r, "13: try finally.");


totals(13);

#endif
