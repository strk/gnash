// Boolean_as.hx: ActionScript 3 "Boolean" class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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

// Test case for Boolean ActionScript class
// Migrated to haXe by Jonathan Crider

#if flash9
import flash.display.MovieClip;
#else
import flash.MovieClip;
#end

import flash.Lib;
import Type;
import Reflect;
import Std;

// importing our testing api
import DejaGnu;

class Boolean_as {
	static function main() {
	
	DejaGnu.note("N1: "+ Type.typeof(untyped Boolean));
	if ( Type.typeof(untyped Boolean) == ValueType.TFunction) {
			DejaGnu.pass("Boolean class exists");
		} else {
			DejaGnu.fail("Boolean class does not exist");
		}
	
	DejaGnu.note("N2: " + Type.typeof(untyped Boolean()));
	if (Type.typeof(untyped Boolean()) == ValueType.TNull) {
		DejaGnu.pass("Call to Boolean() returns null");
	} else {
		DejaGnu.fail("Call to Boolean() does not return null");
	}
	
	DejaGnu.note("N3: " + Type.typeof(untyped Boolean(true)));
	if (Type.typeof(untyped Boolean(true)) == ValueType.TBool) {
		DejaGnu.pass("");
	} else {
		DejaGnu.fail("");
	}
	
	//NOTE: may need to retain Ming tests at the end of the file somehow
		
	DejaGnu.done();
	}
}

//NOTE: Haxe does not give acces to the Boolean class directly.
//      In haXe Bool is an Enum value and does all the processing in the
//      background. Thus there is no way to test the constructor and
//      subsequently the other methods.
//      These ming test cases may need to be retained unless we can get the haxe
//      folks to add a Boolean class

/*
var boolObj = new Boolean;

// test the Boolean constuctor
check (boolObj);

check (boolObj.toString);
check (boolObj.valueOf);

#if OUTPUT_VERSION > 6
// SWF7 and up is case-sensitive
check_equals (boolObj.tostring, undefined);
check_equals (boolObj.valueof, undefined);
#else
check (boolObj.tostring)
check (boolObj.valueof)
#endif

var defaultBool = new Boolean();
check_equals(defaultBool.toString(), "false");
check_equals(defaultBool.valueOf(), false);

var trueBool = new Boolean(true);
check_equals(trueBool.toString(), "true");
check_equals(trueBool.valueOf(), true);

var falseBool = new Boolean(false);
check_equals(falseBool.toString(), "false");
check_equals(falseBool.valueOf(), false);


//---------------------------------------------------
// Test convertion to boolean
//---------------------------------------------------

// boolean
check( true );
check( ! false );
 
// number
check( 1 );
check( !0 );

// movieclip
check( _root );

// string
check( "1" );
#if OUTPUT_VERSION < 7
 check( ! "0" );
 check( ! "true" );
 check( ! "false" );
#else
 check( "0" );
 check( "true" );
 check( "false" );
#endif

// Null 
check_equals(typeOf(null), "null" );
check( ! null );

// Undefined
check( ! undefined );

// Function
check( play );

// Object - conversion might depend on object type
emptyArray = new Array();
check( emptyArray );
falseBoolean = new Boolean(false);
check( falseBoolean );
trueBoolean = new Boolean(true);
check( trueBoolean );


check_totals(31);
*/
