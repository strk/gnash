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


// Test case for Color ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


#include "check.as"

//--------------------------------
// Color was introduced in SWF5
//--------------------------------

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

#if OUTPUT_VERSION > 5
check ( Color.prototype.hasOwnProperty('getRGB') );
check ( Color.prototype.hasOwnProperty('setRGB') );
check ( Color.prototype.hasOwnProperty('getTransform') );
check ( Color.prototype.hasOwnProperty('setTransform') );
#endif

//-----------------------------------------------------------
// test the Color constuctor
//-----------------------------------------------------------

colorObj = new Color;
check_equals ( typeof(colorObj), 'object')
check ( colorObj instanceof Color );
check ( colorObj instanceof Object );
check_equals ( typeof(colorObj.getRGB()), 'undefined' );
check_equals ( typeof(colorObj.getTransform()), 'undefined' );

colorObj = new Color(__shared_assets);
check_equals ( typeof(colorObj), 'object')
check ( colorObj instanceof Color );
check ( colorObj instanceof Object );

invalidColorObj = new Color(4);
check_equals ( typeof(colorObj), 'object')
check ( colorObj instanceof Color );
check ( colorObj instanceof Object );

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
xcheck_equals ( Math.round(trans2.ba*100)/100, 31.64 ); // gnash returns 32, who's right ?
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

//-----------------------------------------------------------
// end of test
//-----------------------------------------------------------


totals();
