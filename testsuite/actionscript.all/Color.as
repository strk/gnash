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


// Test case for Color ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Color.as";
#include "check.as"

//--------------------------------
// Color was introduced in SWF5
//--------------------------------
#if OUTPUT_VERSION < 6
Color.prototype.hasOwnProperty = ASnative(101, 5);
#endif

check_equals ( typeof(Color), 'function')
check_equals ( typeof(Color.prototype), 'object')
check_equals ( typeof(Color.prototype.getRGB), 'function')
check_equals ( typeof(Color.prototype.setRGB), 'function')
check_equals ( typeof(Color.prototype.getTransform), 'function')
check_equals ( typeof(Color.prototype.setTransform), 'function')
check_equals ( typeof(Color.getRGB), 'undefined')
check_equals ( typeof(Color.setRGB), 'undefined')
check_equals ( typeof(Color.getTransform), 'undefined')
check_equals ( typeof(Color.setTransform), 'undefined')

check ( Color.prototype.hasOwnProperty('getRGB') );
check ( Color.prototype.hasOwnProperty('setRGB') );
check ( Color.prototype.hasOwnProperty('getTransform') );
check ( Color.prototype.hasOwnProperty('setTransform') );

//-----------------------------------------------------------
// test the Color constuctor
//-----------------------------------------------------------

colorObj = new Color;
check_equals ( typeof(colorObj), 'object')
check ( colorObj instanceof Color );
check ( colorObj instanceof Object );
check_equals ( typeof(colorObj.getRGB()), 'undefined' );
check_equals ( typeof(colorObj.getTransform()), 'undefined' );

check(colorObj.hasOwnProperty("target"));
check_equals(colorObj.target, undefined);

colorObj = new Color(__shared_assets);
check_equals ( typeof(colorObj), 'object')
check ( colorObj instanceof Color );
check ( colorObj instanceof Object );
check(colorObj.hasOwnProperty("target"));
check_equals(colorObj.target.toString(), "[object Object]");

invalidColorObj = new Color(4);
check_equals ( typeof(colorObj), 'object')
check ( colorObj instanceof Color );
check ( colorObj instanceof Object );
check(invalidColorObj.hasOwnProperty("target"));
check_equals(invalidColorObj.target.toString(), "4");

called = "";
o = {};
o.toString = function() { called += "."; return "_root"; };

// Check that target.toString() is called on each method, but not on
// construction

f = new Color(o);
check_equals(called, "");

rgb = f.getRGB();
check_equals(called, ".");

f.setRGB(rgb);
check_equals(called, "..");

// If o is a MovieClip, toString is not called.
called = "";
o = _root;
o.toString = function() { called += ":"; return "mc"; };
f = new Color(o);
check_equals(called, "");

rgb = f.getRGB();
check_equals(called, "");

f.setRGB(rgb);
check_equals(called, "");



//-----------------------------------------------------------
// test the Color::getRGB method
//-----------------------------------------------------------

rgb = colorObj.getRGB();
check_equals ( typeof(rgb), 'number' );
check_equals ( rgb, 0 );

//-----------------------------------------------------------
// test the Color::getTransform method
//
// ra - red multiplier -100 .. +100
// rb - red offset -255 .. +255
// ga = green multiplier -100 .. +100
// gb = green offset -255 .. +255
// ba = blu multiplier -100 .. +100
// bb = blu offset offset -255 .. +255
// aa = alpha multiplier -100 .. +100
// ab = alpha offset -255 .. +255
//
//-----------------------------------------------------------

trans = colorObj.getTransform();
check_equals ( typeof(trans), 'object' );
check ( trans instanceof Object );
check_equals ( trans.ra, 100 );
check_equals ( trans.rb, 0 );
check_equals ( trans.ga, 100 );
check_equals ( trans.gb, 0 );
check_equals ( trans.ba, 100 );
check_equals ( trans.bb, 0 );
check_equals ( trans.aa, 100 );
check_equals ( trans.ab, 0 );

#if OUTPUT_VERSION > 5

// No createEmptyMovieClip in v5

ort = _root.createEmptyMovieClip("ort", 605);
c = new Color(ort);
ctr = c.getTransform();
check_equals (ctr.ra, 100);
check_equals (ctr.ga, 100);
check_equals (ctr.ba, 100);
check_equals (ctr.aa, 100);
check_equals (ctr.rb, 0);
check_equals (ctr.gb, 0);
check_equals (ctr.bb, 0);
check_equals (ctr.ab, 0);

ort._alpha = 0;
ctr = c.getTransform();
check_equals (ctr.ra, 100);
check_equals (ctr.ga, 100);
check_equals (ctr.ba, 100);
check_equals (ctr.aa, 0);
check_equals (ctr.rb, 0);
check_equals (ctr.gb, 0);
check_equals (ctr.bb, 0);
check_equals (ctr.ab, 0);

#endif

//-----------------------------------------------------------
// test the Color::setTransform method
//-----------------------------------------------------------

check_equals ( typeof(colorObj.setTransform), 'function');

trans.rb = 255;
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0xFF0000 );
trans2 = colorObj.getTransform();
check_equals ( trans2.ra, 100 );
check_equals ( trans2.rb, 255 );
check_equals ( trans2.ga, 100 );
check_equals ( trans2.gb, 0 );
check_equals ( trans2.ba, 100 );
check_equals ( trans2.bb, 0 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

trans.gb = 128;
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0xFF8000 );
trans2 = colorObj.getTransform();
check_equals ( trans2.ra, 100 );
check_equals ( trans2.rb, 255 );
check_equals ( trans2.ga, 100 );
check_equals ( trans2.gb, 128 );
check_equals ( trans2.ba, 100 );
check_equals ( trans2.bb, 0 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

trans.bb = 32; 
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0xFF8020 );
trans2 = colorObj.getTransform();
check_equals ( trans2.ra, 100 );
check_equals ( trans2.rb, 255 );
check_equals ( trans2.ga, 100 );
check_equals ( trans2.gb, 128 );
check_equals ( trans2.ba, 100 );
check_equals ( trans2.bb, 32 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

trans = { ra:-100, ga:-50, ba:50 };
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0xFF8020 );
trans2 = colorObj.getTransform();
check_equals ( trans2.ra, -100 );
check_equals ( trans2.rb, 255 );
check_equals ( trans2.ga, -50 );
check_equals ( trans2.gb, 128 );
check_equals ( trans2.ba, 50 );
check_equals ( trans2.bb, 32 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

trans = { rb:0 }; // only modify the red channel
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0x008020 );
trans2 = colorObj.getTransform();
check_equals ( trans2.ra, -100 );
check_equals ( trans2.rb, 0 );
check_equals ( trans2.ga, -50 );
check_equals ( trans2.gb, 128 );
check_equals ( trans2.ba, 50 );
check_equals ( trans2.bb, 32 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

o = {}; o.valueOf = function() { return 255; };
trans = { gb:o }; // only modify the green channel
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0x00FF20 );
trans2 = colorObj.getTransform();
check_equals ( trans2.ra, -100 );
check_equals ( trans2.rb, 0 );
check_equals ( trans2.ga, -50 );
check_equals ( trans2.gb, 255 );
check_equals ( trans2.ba, 50 );
check_equals ( trans2.bb, 32 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

trans = { bb:2 }; // only modify the blue channel
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0x00FF02 );
trans2 = colorObj.getTransform();
check_equals ( trans2.ra, -100 );
check_equals ( trans2.rb, 0 );
check_equals ( trans2.ga, -50 );
check_equals ( trans2.gb, 255 );
check_equals ( trans2.ba, 50 );
check_equals ( trans2.bb, 2 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

trans = { ba:32 }; // modify the scale of blue channel
colorObj.setTransform(trans);
rgb = colorObj.getRGB();
check_equals ( rgb, 0x00FF02 );

trans2 = colorObj.getTransform();
check_equals ( trans2.ra, -100 );
check_equals ( trans2.rb, 0 );
check_equals ( trans2.ga, -50 );
check_equals ( trans2.gb, 255 );
// pp uses 1/256 accuracy, 31.640625 == int(0.32*256)*100/256.0f
check( trans2.ba - 31.640625 < 0.000001 ); // Don't use check_equals or Math.round here.
check_equals ( trans2.bb, 2 );
check_equals ( trans2.aa, 100 );
check_equals ( trans2.ab, 0 );

//-----------------------------------------------------------
// test the Color::setRGB method
//-----------------------------------------------------------

check_equals ( typeof(colorObj.setRGB), 'function');
colorObj.setRGB(0x667799);
check_equals ( colorObj.getRGB(), 0x667799 );
trans = colorObj.getTransform();
check_equals ( trans.ra, 0 );
check_equals ( trans.rb, 102 );
check_equals ( trans.ga, 0 );
check_equals ( trans.gb, 119 );
check_equals ( trans.ba, 0 );
check_equals ( trans.bb, 153 );
check_equals ( trans.aa, 100 );
check_equals ( trans.ab, 0 );

//
// Accuracy test
//
trans.ra = 99.9;
trans.rb = 99.9;
colorObj.setTransform(trans);
trans2 = colorObj.getTransform();
// 99.609375 == int(0.999*256)*100/256.0
check(trans2.ra - 99.609375 < 0.0000001); // Don't use check_equals or Math.round here.
check_equals(trans2.rb, 99);

#if OUTPUT_VERSION >= 6
trans.aa = 12800; // 0x80 * 100
trans.ab = 0;
_root.createEmptyMovieClip("mc1", 10);
check_equals(mc1._alpha, 100);
colorObj = new Color(mc1);
colorObj.setTransform(trans);
trans2 = colorObj.getTransform();
// (int16)(12800 / 100.0 * 256) == -12800
// Gnash failed, but not due to accuracy problem,
// _alpha is not calculated correctly.
check_equals(mc1._alpha, -12800);

trans.ab = 10;
// _alpha is not calculated correctly. Not sure about the algorithm at the moment. 
check_equals(mc1._alpha, -12800);

mc1._alpha = 60;
trans2 = colorObj.getTransform();
// 59.765625: value retrieved from AS
// int(60 / 100.0 * 256): value stored in cxform. 
// 59.765625 == int(60 / 100.0 * 256) / 2.56
check(trans2.aa - 59.765625 < 0.0000001);
check_equals(trans.ab, 10);
#endif

//
// Some tests for same-named (case-insensitive) variables in SWF6
//
#if OUTPUT_VERSION == 6
color = 8;

c = new Color;
check_equals(c, undefined);
c = new color;
check_equals(c, undefined);

delete color;
c = new Color;
check_equals (typeof(c), 'object');
c = new color;
check_equals (typeof(c), 'object');

color = 8;
check_equals (typeof(c), 'object');
check_equals (typeof(c), 'object');

delete c;
c = new Color;
check_equals (typeof(c), 'undefined');
c = new color;
check_equals (typeof(c), 'undefined');

delete color; // variable
delete color; // class
c = new Color;
check_equals (typeof(c), 'undefined');
c = new color;
check_equals (typeof(c), 'undefined');

// Do not add any tests after here (color deleted).
#endif

//-----------------------------------------------------------
// end of test
//-----------------------------------------------------------


#if OUTPUT_VERSION == 5
totals(134);
#elif OUTPUT_VERSION == 6
totals(165);
#else
totals(155);
#endif
