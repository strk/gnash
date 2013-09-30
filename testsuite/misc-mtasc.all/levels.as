// levels.as - MTASC testcase for loading into _level targets
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

class LevelsMain
{
  static function main(mc)
  {
    // create a var that other swfs can test
    _level0.testvar = 1239;

    var test = new LevelsMain();
    test.run();

    // Check our depth
    check_equals(mc.getDepth(), -16384);

    // The ""+ is there to force conversion to a string
    check_equals(""+mc, "_level0");

    mc.onEnterFrame = function ()
    {
      this.frameno++;
    }
  }

  function run() {
    trace("main class running");
    getURL("level5.swf","_level"+5);
  }
}
