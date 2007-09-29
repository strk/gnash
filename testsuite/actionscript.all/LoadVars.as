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


// Test case for LoadVars ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: LoadVars.as,v 1.15 2007/09/29 16:22:57 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION < 6

xcheck_equals(typeof(LoadVars), 'function');

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
xcheck_equals (typeof(loadvarsObj), 'object');

#else // OUTPUT_VERSION >= 6

check_equals(typeof(LoadVars), 'function');

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
check_equals (typeof(loadvarsObj), 'object');

// test the LoadVars::addrequestheader method
check_equals (typeof(loadvarsObj.addRequestHeader), 'function');
// test the LoadVars::decode method
check_equals (typeof(loadvarsObj.decode), 'function');
// test the LoadVars::getbytesloaded method
check_equals (typeof(loadvarsObj.getBytesLoaded), 'function');
// test the LoadVars::getbytestotal method
check_equals (typeof(loadvarsObj.getBytesTotal), 'function');
// test the LoadVars::load method
check_equals (typeof(loadvarsObj.load), 'function');
// test the LoadVars::send method
check_equals (typeof(loadvarsObj.send), 'function');
// test the LoadVars::sendandload method
check_equals (typeof(loadvarsObj.sendAndLoad), 'function');
// test the LoadVars::tostring method
check_equals (typeof(loadvarsObj.toString), 'function');

var loadvarsObj = new LoadVars; // STRK REMOVEME

//--------------------------------------------------------------------------
// Test LoadVars::load()
//--------------------------------------------------------------------------

varsloaded = 0;
datareceived = 0;
//var1 = undefined;
//var2 = undefined;
loadvarsObj.onLoad = function() {
	varsloaded++;
	note("LoadVars.onLoad called "+varsloaded+". "
		+"Bytes loaded: "+loadvarsObj.getBytesLoaded()
		+"/"+loadvarsObj.getBytesTotal());

	//delete loadvarsObj; // this to test robustness

	check(varsloaded < 3);

	// onLoad is called after all vars have been called
	check_equals( loadvarsObj.getBytesLoaded(), loadvarsObj.getBytesTotal() );

	check_equals( loadvarsObj.loaded, true );

	//for (var i in _root) { note("_root["+i+"] = "+_root[i]); }

	if ( varsloaded == 1 )
	{
		check_equals(loadvarsObj['var1'], 'val1');
		//check_equals(loadvarsObj['var1_check'], 'previous val1');
		check_equals(loadvarsObj['var2'], 'val2');
		//check_equals(loadvarsObj['v2_var1'], 'val1');
		//check_equals(loadvarsObj['v2_var2'], 'val2');
		play();
	}
};

// If LoadVars.onData is defined, onLoad won't be called !
// (at least this is what I see)
#if 0
loadvarsObj.onData = function() {
	datareceived++;
	note("LoadVars.onData called ("+datareceived+"), byte loaded: "
		+loadvarsObj.getBytesLoaded()
		+"/"+loadvarsObj.getBytesTotal());
	//check_equals(loadvarsObj['var1'], 'val1');
	//check_equals(loadvarsObj['var2'], 'val2');
	//for (var i in _root) { note("_root["+i+"] = "+_root[i]); }
	//play();
};
#endif

loadvarsObj.var1 = "previous val1";

// We expect the loaded file to return this string:
//
// 	"var1=val1&var2=val2&"
//
// The final '&' char is important, and it must
// not start with a '?' char.
// 
check( loadvarsObj instanceOf LoadVars );
//check( loadvarsObj.sendAndLoad( 'http://localhost/vars.php', loadvarsObj ) );
check( loadvarsObj.load( MEDIA(vars.txt) ) );
//loadvarsObj.load( 'vars.cgi' );

check_equals( loadvarsObj.loaded, false );
//loadvars.Obj.loaded = true;
//check_equals( loadvarsObj.loaded, false );
check_equals(varsloaded, 0);
check_equals(loadvarsObj['var1'], 'previous val1'); // will be overridden
check_equals(loadvarsObj['var2'], undefined);
//delete loadvarsObj; // this to test robustness

stop();

#endif //  OUTPUT_VERSION >= 6
totals();
