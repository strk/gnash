// extcomm.as - Host container communication (ExternalInterface) tests
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

import flash.external.*;

#include "check.as"

class ExternalCommTest
{
	// Entry point
	public static function main(mc:MovieClip):Void
	{
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
			ExternalInterface.addCallback("script_call", mc,
				function(arg1, arg2):String
				{
					// This function should be called
					check(true);
					check_equals(arg1, "Hello");
					check_equals(arg2, "World");
					return "Too";
				}
			)
		);
	}
}
