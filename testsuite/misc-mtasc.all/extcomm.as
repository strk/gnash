// extcomm.as - Host container communication (ExternalInterface) tests
//
//   Copyright (C) 2015, 2016 Free Software Foundation, Inc.
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

import flash.external.*;

#include "check.as"

class ExternalCommTest
{
	var testVariable = "Variable in this";

	// Entry point
	public static function main(mc:MovieClip):Void
	{
		var app:ExternalCommTest;

		app = new ExternalCommTest(mc);
	}

	public function ExternalCommTest(mc:MovieClip)
	{
		var obj:Object;

		obj = new Object();
		obj.testVariable = "Variable in obj";
		_root.testVariable = "Variable in _root";

		// ExternalInterface should be available
		check(ExternalInterface.available);

		mc.script_call = function(arg1, arg2):String
		{
			// This function should NOT be called
			check(false);
			check_equals(arg1, "Hello");
			check_equals(arg2, "World");
			return "Too";
		};

		// Registering callback shouldn't fail
		check(
			ExternalInterface.addCallback("script_call", obj,
				function(arg1, arg2):String
				{
					// This function should be called
					check(true);
					check_equals(arg1, "Hello");
					check_equals(arg2, "World");

					// `this` should point to user-specified object
					check_equals(this.testVariable, "Variable in obj");

					return "Too";
				}
			)
		);

		// Invalid callback registrations should fail
		check(!ExternalInterface.addCallback("invalid_reg1", mc, null));
		check(!ExternalInterface.addCallback("invalid_reg2", mc, undefined));
		check(!ExternalInterface.addCallback("invalid_reg3", null, null));
		check(!ExternalInterface.addCallback("invalid_reg4", null, undefined));
		check(!ExternalInterface.addCallback("invalid_reg5", undefined, null));
		check(!ExternalInterface.addCallback("invalid_reg6", undefined, undefined));

		// Registering callbacks with no `this` instance shouldn't fail
		check(
			ExternalInterface.addCallback("script_nothis1", null,
				function():Void
				{
					// `this` should be an "undefined" object like one in
					// a function called via `function.call(null)`
					xcheck_equals(typeof(this), "object");
					check(this == undefined);
					check(this == null);
					check(this !== undefined);
					xcheck(this !== null);
					xcheck_equals("" + this, "undefined");
				}
			)
		);
		check(
			ExternalInterface.addCallback("script_nothis2", undefined,
				function():Void
				{
					// `this` should be an "undefined" object like one in
					// a function called via `function.call(undefined)`
					xcheck_equals(typeof(this), "object");
					check(this == undefined);
					check(this == null);
					check(this !== undefined);
					xcheck(this !== null);
					xcheck_equals("" + this, "undefined");
				}
			)
		);

		// Registering another ordinary callback shouldn't fail
		check(
			ExternalInterface.addCallback("script_longarglist", mc,
				function(arg1:String, arg2:String, arg3:String,
				         arg4:String, arg5:String, arg6:String,
				         arg7:String, arg8:String, arg9:String):String
				{
					// Long argument list should be passed correctly
					check_equals(arg1, "The");
					check_equals(arg2, "quick");
					check_equals(arg3, "brown");
					check_equals(arg4, "fox");
					check_equals(arg5, "jumps");
					check_equals(arg6, "over");
					check_equals(arg7, "the");
					check_equals(arg8, "lazy");
					check_equals(arg9, "dog");

					return "Pangram";
				}
			)
		);

		// Calling JavaScript function without any argument should give
		// a correct return value
		check_equals(ExternalInterface.call("js_simple"), "Correct");
	}
}
