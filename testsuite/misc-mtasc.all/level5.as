// level5.as - Data file for the levels.as test
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
//
// Original author: David Rorex - drorex@gmail.com
//

#include "check.as"

class Level5
{
  static function main(mc)
  {
    check_equals(mc._currentframe, 1);

    check(_level0.frameno >= 1);

    // Check our depth
    check_equals(mc.getDepth(), -16379);

    // The ""+ is there to force conversion to a string
    check_equals(""+mc, "_level5");

    // Mc level is _level0 ? why ? 
    check_equals(mc._level, _level0);

    // check that we can acess back to _level0
    check_equals(_level0.testvar, 1239);

    // check that we can modify vars on our own level
    check_equals(_level5.testvar, undefined);
    _level5.testvar = 6789;
    check_equals(_level5.testvar, 6789);

    // check that we can modify vars on _level0
    check_equals(_level0.testvar2, undefined);
    _level0.testvar2 = true;
    check_equals(_level0.testvar2, true);

    _level5.onUnload = function()
    {
      check(false); // should not be executed
      note("Unloading "+this);
    }

    mc.createEmptyMovieClip("ch", 1);
    with(mc.ch)
    {
      lineStyle(1, 0x00000);
      beginFill(0xFF0000, 80);
      var x=200;
      var y=200;
      var width=100;
      var height=100;
      moveTo(x, y);
      lineTo(x+width, y);
      lineTo(x+width, y+height);
      lineTo(x, y+height);
      lineTo(x, y);
      endFill();
    };

    check_equals(mc.ch._target, "_level5/ch");

    // load yet another swf
    getURL("level99.swf","_level"+99);

    getURL("level87.swf", "_level87");
  }
}
