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

// Test case for Boolean ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: Try.as,v 1.1 2008/06/11 12:09:33 bwy Exp $";
#include "check.as"

// Some of the test variants.
// Try catch finally (no throw)
// Try (throw) catch finally
// Try finally
// Try (throw) finally
// Try catch
// Try (throw) catch

throwfunc = function()
{
    throw "try";
};

throwfunc2 = function()
{
    try {
        throw "try";
    }
    catch (e) {};
};

r = "1: ";
try { r +="try "; r +="body "; }
catch (a) { r +="catch "; r += a + " "; }
finally { r +="finally "; };
r += ".";

#if OUTPUT_VERSION < 7
xcheck_equals(r, "1: try body catch  finally .");
#else
check_equals(r, "1: try body catch undefined finally .");
#endif

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
catch (c) { r += c + " "; };
r += ".";
xcheck_equals(r, "4: try finally thrown .");

// Also check that the exception is not
// undefined if nothing is thrown
d = "pre-existing variable d";
r = "5: ";
try { r += "try "; r += "body "; }
catch (d) { r += "catch "; r+= d + " "; };
r += ".";
xcheck_equals(r, "5: try body catch pre-existing variable d .");

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
xcheck_equals(r, "7: try finally finally2 thrown .");


//try { throwfunc(); }
//catch (g) { trace ("catch"); trace (g); };

//try { throw "thrown"; }
//finally { };
//trace ("Don't reach this point");
