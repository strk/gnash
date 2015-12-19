// extgetvariable.as - Built-in GetVariable() plugin function tests
//
//   Copyright (C) 2015 Free Software Foundation, Inc.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
//
//
// Original author: Nutchanon Wetchasit <Nutchanon.Wetchasit@gmail.com>
//

// Once this Flash code is run, it stores multiple types of value to variables
// inside root movie, which could be accessed by plugin's `GetVariable(path)`
// function.

var string_variable="This is a string";
var integer_variable=9876;
var float_variable=9876.5432;
var infinite_variable=Infinity;
var neginfinite_variable=-Infinity;
var nan_variable=NaN;
var boolean_variable=true;
var null_variable=null;
var unassigned_variable;
var undefined_variable=undefined;
// `nonexistent_variable` is omitted
var array_variable=new Array("The","quick","brown","fox","jumps","over","the","lazy","dog");
var object_variable=new Object();
var object_variable_customstring=new Object();
object_variable_customstring.toString=function() {
	return "This is a custom Object.toString()";
};
var function_variable=function() {
	trace("This code should not run!");
};
trace("ENDOFTEST");
