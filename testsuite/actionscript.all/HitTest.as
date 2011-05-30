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

rcsid="HitTest.as";

#include "check.as"

#if OUTPUT_VERSION > 5

 createEmptyMovieClip("a", 5);
 a.beginFill(0xff00ff);
 a.moveTo(20, 20);
 a.lineTo(20, 60);
 a.lineTo(60, 60);
 a.lineTo(60, 20);
 a.lineTo(20, 20);

 check_equals(a.hitTest(19, 19, false), false);
 check_equals(a.hitTest(20, 20, false), true);
 check_equals(a.hitTest(21, 21, false), true);
 check_equals(a.hitTest(25, 25, false), true);
 check_equals(a.hitTest(60, 60, false), true);
 check_equals(a.hitTest(61, 61, false), false);

 check_equals(a.hitTest(19, 19, true), false);
#if OUTPUT_VERSION < 8
 check_equals(a.hitTest(20, 20, true), true);
#else
 xcheck_equals(a.hitTest(20, 20, true), false);
#endif
 check_equals(a.hitTest(21, 21, true), true);
 check_equals(a.hitTest(25, 25, true), true);
 check_equals(a.hitTest(60, 60, true), false);
 check_equals(a.hitTest(61, 61, true), false);
 
 _x = 30;
 check_equals(a.hitTest(25, 25, false), true);
 check_equals(a.hitTest(85, 25, false), false);
 xcheck_equals(a.hitTest(25, 25, true), true);
 xcheck_equals(a.hitTest(85, 25, true), false);
 _x = 0;

 backup = _width;

 _width = 50;
 check_equals(a.hitTest(25, 25, false), true);
 check_equals(a.hitTest(41, 25, false), true);
 check_equals(a.hitTest(25, 25, true), false);
 check_equals(a.hitTest(41, 25, true), false);
 _width = backup;

 _xscale = 50;
 check_equals(a.hitTest(25, 25, false), true);
 check_equals(a.hitTest(41, 25, false), true);
 xcheck_equals(a.hitTest(25, 25, true), false);
#if OUTPUT_VERSION < 8
 xcheck_equals(a.hitTest(41, 25, true), true);
#else
 check_equals(a.hitTest(41, 25, true), false);
#endif
 _xscale = 100;

 b = a.createEmptyMovieClip("b", 10);
 b.beginFill(0x0000ff);
 b.moveTo(100, 100);
 b.lineTo(150, 100);
 b.lineTo(150, 150);
 b.lineTo(100, 150);
 b.lineTo(100, 100);
 
 check_equals(b.hitTest(99, 99, false), false);
 check_equals(b.hitTest(100, 100, false), true);
 check_equals(b.hitTest(101, 101, false), true);
 check_equals(b.hitTest(120, 120, false), true);
 check_equals(b.hitTest(110, 110, false), true);
 check_equals(b.hitTest(151, 150, false), false);
 check_equals(b.hitTest(151, 151, false), false);
 check_equals(b.hitTest(151, 152, false), false);

 check_equals(b.hitTest(99, 99, true), false);
 xcheck_equals(b.hitTest(100, 100, true), false);
 check_equals(b.hitTest(101, 101, true), true);
 check_equals(b.hitTest(120, 120, true), true);
 check_equals(b.hitTest(110, 110, true), true);
 check_equals(b.hitTest(151, 150, true), false);
 check_equals(b.hitTest(151, 151, true), false);
 check_equals(b.hitTest(151, 152, true), false);

 /// Altering a's properties changes the hitTest for b.
 a._y = 100;

 /// Test where it was before
 check_equals(b.hitTest(99, 99, false), false);
 check_equals(b.hitTest(100, 100, false), false);
 check_equals(b.hitTest(101, 101, false), false);
 check_equals(b.hitTest(120, 120, false), false);
 check_equals(b.hitTest(110, 110, false), false);
 check_equals(b.hitTest(151, 150, false), false);

 check_equals(b.hitTest(99, 99, true), false);
 check_equals(b.hitTest(100, 100, true), false);
 check_equals(b.hitTest(101, 101, true), false);
 check_equals(b.hitTest(120, 120, true), false);
 check_equals(b.hitTest(110, 110, true), false);
 check_equals(b.hitTest(151, 150, true), false);

 /// Test new position
 check_equals(b.hitTest(99, 199, false), false);
 check_equals(b.hitTest(100, 200, false), true);
 check_equals(b.hitTest(101, 201, false), true);
 check_equals(b.hitTest(120, 220, false), true);
 check_equals(b.hitTest(110, 210, false), true);
 check_equals(b.hitTest(151, 250, false), false);
 check_equals(b.hitTest(151, 251, false), false);
 check_equals(b.hitTest(151, 252, false), false);

 /// (Where is it?!) Checking every pixel in +/- _width by +/- _height
 /// comes up with "true" in the shifted area, but doing it directly
 /// returns false.
 check_equals(b.hitTest(99, 199, true), false);
 xcheck_equals(b.hitTest(100, 200, true), false);
 xcheck_equals(b.hitTest(101, 201, true), false);
 xcheck_equals(b.hitTest(120, 220, true), false);
 xcheck_equals(b.hitTest(110, 210, true), false);
 check_equals(b.hitTest(151, 250, true), false);
 check_equals(b.hitTest(151, 251, true), false);
 check_equals(b.hitTest(151, 252, true), false);

 /// Changing _root's matrix does nothing.
 _y = -100;
 _xscale = 0.5;

 check_equals(b.hitTest(99, 99, false), false);
 check_equals(b.hitTest(100, 100, false), false);
 check_equals(b.hitTest(101, 101, false), false);
 check_equals(b.hitTest(120, 120, false), false);
 check_equals(b.hitTest(110, 110, false), false);
 check_equals(b.hitTest(151, 150, false), false);

 check_equals(b.hitTest(99, 199, false), false);
 check_equals(b.hitTest(100, 200, false), true);
 check_equals(b.hitTest(101, 201, false), true);
 check_equals(b.hitTest(120, 220, false), true);
 check_equals(b.hitTest(110, 210, false), true);
 xcheck_equals(b.hitTest(151, 250, false), true);
 check_equals(b.hitTest(151, 251, false), false);
 check_equals(b.hitTest(151, 252, false), false);
 
totals(82);

#endif
