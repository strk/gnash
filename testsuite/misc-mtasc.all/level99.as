// level99.as - Data file for the levels.as test
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

class Level99
{
	static function main(mc)
	{
		mc.createEmptyMovieClip("ch", 1);
		with(mc.ch)
		{
			lineStyle(1, 0x00000);
			beginFill(0x00FF00, 80);
			var x=250;
			var y=250;
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

               	check_equals(_level0.frameno, 2);

                // Check our depth
		check_equals(mc.getDepth(), -16285);

		// _root is relative. TODO: check _lockroot effect !
                check_equals(mc._root, _level99);
                check_equals(_root, _level99);
		// "/" in path is relative !
		check_equals(eval("/ch"), _level99.ch);

                // The ""+ is there to force conversion to a string
                check_equals(""+mc, "_level99");

                // Mc level is _level0 ? why ? 
                check_equals(mc._level, _level0);

                // check that we can acess back to _level0
                check_equals(_level0.testvar, 1239);
                check_equals(_level0.testvar2, true);

                // check that we can acess back to _level5
                check_equals(_level5.testvar, 6789);

                // check that we can modify vars on our own level
                check_equals(_level99.testvar, undefined);
                _level99.testvar = "hello";
                check_equals(_level99.testvar, "hello");

                // check that we can modify vars on _level5
                check_equals(_level5.testvar2, undefined);
                _level5.testvar2 = "goodbye";
                check_equals(_level5.testvar2, "goodbye");

                check_equals(typeof(_level5), 'movieclip');
		var level5ref = _level5;
		check_equals(_level5.getDepth(), -16379);
		_level5.swapDepths(10); 
                check_equals(typeof(_level5), 'undefined');
                check_equals(typeof(level5ref), 'movieclip');
                check_equals(level5ref.getDepth(), '10');
                check_equals(level5ref._target, '_level16394');
                check_equals(typeof(_level16394), 'movieclip');
		level5ref.swapDepths(20); // swapDepth doesn't work now because level5ref is out of the static depth zone
                check_equals(level5ref.getDepth(), '10');
                check_equals(level5ref._target, '_level16394');
                check_equals(level5ref._name, '');
                xcheck_equals(""+level5ref, '_level5');

		var level99ref = _level99;
		_level99.swapDepths(30);
                check_equals(level99ref.getDepth(), '30');
                check_equals(level99ref._target, '_level16414');
		level99ref.swapDepths(40); // swapDepth doesn't work now because level99ref is out of the static depth zone
                check_equals(level99ref.getDepth(), '30');
                check_equals(level99ref._target, '_level16414');
                check_equals(level99ref._name, '');
                xcheck_equals(""+level99ref, '_level99');

		note("Setting up onEnterFrame for "+mc.ch);
		mc.ch.count = 0;
		mc.ch.l5ref = level5ref;
		mc.ch.l99ref = level99ref;
		mc.ch.onEnterFrame = function()
		{
			//note(this+".enterFrame -- l5ref is "+this.l5ref+" -- l99ref is "+this.l99ref);
			if ( this.count > 4 )
			{
				check_equals(this.l5ref._target, '_level16394');
				check_equals(this.l99ref._target, '_level16414');

				_level16394.onUnload = function() {
					check(false); // should not be executed
				}
				_level16394.removeMovieClip();

				check_equals(typeof(this.l5ref), 'movieclip');
				check_equals(typeof(this.l5ref)._target, 'undefined');
				check_equals(typeof(this.l5ref.getDepth), 'undefined');
				check_equals(typeof(_level16364), 'undefined')

				// END OF TEST HERE
				// TODO: add tests for:
				//  - sane swapping between to levels,
				//  - swapping & removing _level0 
				//  
				check_totals(61);
				Dejagnu.done();
				delete this.onEnterFrame;
			}
			else
			{
				++this.count;
				this.l5ref.swapDepths(this.l99ref);
			}
		}
	}
}
