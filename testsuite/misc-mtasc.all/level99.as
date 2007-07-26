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
                // The ""+ is there to force conversion to a string
                check_equals(""+mc, "_level99");

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

                Dejagnu.done();
	}
}
