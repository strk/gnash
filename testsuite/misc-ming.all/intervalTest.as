// 
//   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
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


// These are already checked in Global.as, and it would be
// clear if things don't work as expected
check_equals(typeof(setInterval), 'function');
check_equals(typeof(clearInterval), 'function');

do_this = function() {
	++this_counter;
	var now = getTimer();
	var int = now-this_timer;
	this_timer = now;
	check(int >= this_ms, this_ms+" interval (this) called after " + int + " milliseconds [" + __FILE__ + ":" + __LINE__ + "]");
	note("Doing this "+this_counter+" after " + int + " milliseconds");
	if ( this_counter > 3 )
	{
		clearInterval(this_interval);
		note("This interval cleared ");
		if ( this_counter > 4 )
		{
			note('Setting another interval');
			A = function() {};
			A.prototype.name = 'A';
			A.prototype.test = function() { return 'Atest'; };
			B = function() {}; B.prototype = new A;
			B.prototype.test = function() {
				check_equals(super.test(), 'Atest');
				check_equals(super.name, 'A');
				totals(18, __FILE__ + ":" + __LINE__ );
				test_completed = 1;
				clearInterval(method_interval);
				loadMovie("fscommand:quit", _level0);

			};

			o = new B;
			method_interval = setInterval(o, 'test', 1); 
		}
	}
};

do_that = function() {
	++that_counter;
	var now = getTimer();
	var int = now-that_timer;
	that_timer = now;
	check(int >= that_ms, that_ms+" interval (that) called after " + int + " milliseconds [" + __FILE__ + ":" + __LINE__ + "]");
	//note("Doing that "+that_counter+" after " + int + " milliseconds");
	if ( that_counter > 3 )
	{
		clearInterval(that_interval);
		note("That interval cleared ");
		this_time = getTimer();
		this_ms = 1;
		this_interval = setInterval(do_this, 1);
		// interval 1 is NOT reused
		check_equals(this_interval, 4); // interval 3 is set from within do_that
	}
};

push_args = function() {
	check_equals(arguments.length, 3);
	clearInterval(push_interval);
	note("Pushing "+arguments.length+" args");
	for (var i=0; i<arguments.length; i++)
	{
		pushed_args[i] = arguments[i];
	}
};

this_counter = 0;
this_timer = getTimer();
this_ms = 1; // 0.0001;
this_interval  = setInterval(do_this, 1); // 0.0001);
check_equals(this_interval, 1);

that_counter = 0;
that_ms = 1000;
that_timer = getTimer();
that_interval  = setInterval(do_that, 1000);
check_equals(that_interval, 2);

pushed_args = new Array;
push_interval  = setInterval(push_args, 200, 8, 9, 10);
check_equals(push_interval, 3);


stop();
