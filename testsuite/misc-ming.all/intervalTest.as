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
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


// These are already checked in Global.as, and it would be
// clear if things don't work as expected
check_equals(typeof(setInterval), 'function');
check_equals(typeof(clearInterval), 'function');

do_this = function() {
	++this_counter;
	xtrace("Doing this "+this_counter);
	if ( this_counter > 3 )
	{
		clearInterval(this_interval);
		xtrace("This interval cleared ");
		if ( this_counter > 4 )
		{
			totals();
			test_completed = 1;
		}
	}
};

do_that = function() {
	++that_counter;
	xtrace("Doing that "+that_counter);
	if ( that_counter > 3 )
	{
		clearInterval(that_interval);
		xtrace("That interval cleared ");
		this_interval = setInterval(do_this, 1);
		// interval 1 is NOT reused
		check_equals(this_interval, 4); // interval 3 is set from within do_that
	}
};

push_args = function() {
	check_equals(arguments.length, 3);
	clearInterval(push_interval);
	xtrace("Pushing "+arguments.length+" args");
	for (var i=0; i<arguments.length; i++)
	{
		pushed_args[i] = arguments[i];
	}
};

this_counter = 0;
this_interval  = setInterval(do_this, 0.0001);
check_equals(this_interval, 1);

that_counter = 0;
that_interval  = setInterval(do_that, 1000);
check_equals(that_interval, 2);

pushed_args = new Array;
push_interval  = setInterval(push_args, 200, 8, 9, 10);
check_equals(push_interval, 3);

stop();
