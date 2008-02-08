// level5.as - Data file for the levels.as test
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
//
// Original author: David Rorex - drorex@gmail.com
//

#include "check.as"

class Level87
{
	static function main(mc)
	{
		mc.createEmptyMovieClip("ch", 1);
		with(mc.ch)
		{
			lineStyle(1, 0x00000);
			beginFill(0xFFFF00, 80);
			var x=220;
			var y=170;
			var width=100;
			var height=100;
			moveTo(x, y);
			lineTo(x+width, y);
			lineTo(x+width, y+height);
			lineTo(x, y+height);
			lineTo(x, y);
			endFill();
		};

                check_equals(mc._currentframe, 1);

                // Check our depth
		check_equals(mc.getDepth(), -16297);

                // The ""+ is there to force conversion to a string
                check_equals(""+mc, "_level87");

		if (_level0.level87loaded )
		{
                	check_equals(_level0.frameno, 3);

                	check_equals(typeof(_level5), 'undefined');
			return;
		}

		_level0.level87loaded = true;

               	check_equals(_level0.frameno, 2);

		// This one fails because gnash is executing code
		// in level99 before code in the first load of level87,
		// probably because it is *loading* level99 before level87,
		// which is in the order loads are requested rather then
		// reverse of it as it's common...
                check_equals(_level5._currentframe, 1);

		_level87.loadMovie("level87.swf");
	}
}
