// TextFieldTest.as - Tests for text
//
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
// Original author: Asger Ottar Alstrup - asger@ottaralstrup.dk
//

#include "check.as"

class Text
{
	static function main(mc)
	{
		mc.createTextField("textf", 100, 200, 100, 100, 100);
		_level0.createTextField("textout", 99, 10, 10, 500, 500);
		var tf = _level0.textf;
                check_equals(tf._height, 100);
		tf.autoSize = true;
		tf.text = "Hello world";
		tf.backgroundColor = "0xFFEEEE";
		tf.background = true;
		var height = tf._height;
		note("textfield height is "+height);
                check(height < 50);
		check_totals(2);
		Dejagnu.done();
	}
}

