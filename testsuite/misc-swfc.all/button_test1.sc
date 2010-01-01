/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

.flash  bbox=800x600 filename="button_test1.swf" background=white version=6 fps=12

.frame 1

    .action:
#include "Dejagnu.sc"
    .end

.box rbox width=200 height=300 fill=#FF0000 color=#000000
.box ybox_small width=100 height=150 fill=#FFFF00 color=#000000
.box gbox width=200 height=200 fill=#00FF00 color=#000000

.button button1
	.show rbox as=idle
	.show ybox_small as=area
	.show ybox_small as=hover
	.show rbox as=pressed
	.on_press:
		trace("red box");	
		button2._xscale = button2._yscale = 100;
		button1._xscale = button1._yscale = 200;
		runNextTest();
	.end
.end

.button button2
	.show gbox as=idle
	.show gbox as=area
	.show gbox as=hover
	.show gbox as=pressed
	.on_press:
		trace("green box");
		button2._xscale = button2._yscale = 200;
		button1._xscale = button1._yscale = 100;
		runNextTest();
	.end
.end

.sprite buttonContainer
	.put button2 pin=center x=0 y=0 scalex=100% scaley=100% 
	.put button1 pin=center x=20 y=20 scalex=100% scaley=100% alpha=100%
.end

.button button3
	.show buttonContainer as=idle
	.show buttonContainer as=area
	.show buttonContainer as=hover
	.show buttonContainer as=pressed
	.on_press:
		trace("button3");
		button3._xscale = button3._yscale = 200;
	.end
.end

//.put buttonContainer pin=center x=200 y=200 scalex=100% scaley=100% 
.put button3 pin=center x=200 y=300 scalex=100% scaley=50% alpha=100%

.frame 1
.action:
	
	test1 = function()
	{
		note(" - Testing button stuff - ");
		check_equals(typeof(button3), 'object');
		check_equals(button3['_root'], _level0);
		check_equals(button3['_global'], _global);
		check_equals(button3['_level0'], _root);
		check_equals(button3['_target'], "/button3");
		check_equals(button3['_parent'], _root);
		check_equals(typeof(button3.instance1), 'movieclip');
		check_equals(typeof(button3.instANce1), 'movieclip'); // case-insensitive
		check_equals(typeof(button3.instance1.button1), 'object');
		check_equals(typeof(button3.instance1.button2), 'object');
		check_equals(button3.instance1['_target'], "/button3/instance1");
		check_equals(button3.instance1['_parent'], _level0.button3);
		check_equals(button3.instance1.button1._height, 300);
		check_equals(button3.instance1.button1['_target'], "/button3/instance1/button1");
		check_equals(button3.instance1.button1['_parent'], _level0.button3.instance1);
		check_equals(button3.instance1.button2._height, 200);
		check_equals(button3.instance1.button2['_target'], "/button3/instance1/button2");
		check_equals(button3.instance1.button2['_parent'], _level0.button3.instance1);

// Define this when MovieTester-based runners are available, or you 
// intend to run the test manually
#define MOVIETESTER_BASED
#ifndef MOVIETESTER_BASED
		endOfTest();
#else
		note("1. Click on the visible part of the green box.");
		nexttest = test2;
#endif
	};

	test2 = function()
	{
		check_equals(button3.instance1.button1._height, 300);
		check_equals(button3.instance1.button2._height, 400);
		note("2. Now move your mouse on the top-left area of the red box (the box will become yellow), and click where it overlaps with the green one.");
		nexttest = test3;
	};

	test3 = function()
	{
		check_equals(button3.instance1.button1._height, 600);
		check_equals(button3.instance1.button2._height, 200);
		endOfTest();
		nexttest = endOfTest;

	};

	endOfTest = function()
	{
		_root.testcompleted = true;
		totals(22);
	};

	_global.runNextTest = function()
	{
		//note("runNextTest invoked");
		nexttest();
	};

	test1();
.end
  
.end  // file end

