// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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


rcsid="LoadVars.as";
#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(typeof(LoadVars), 'function');

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
check_equals (typeof(loadvarsObj), 'object');

check_totals(2);

#else // OUTPUT_VERSION >= 6

check_equals(typeof(LoadVars), 'function');

// Can only be instantiated with "new"
lv = LoadVars();
check_equals(lv, undefined);
check_equals(typeof(lv), "undefined");

var loadvarsObj = new LoadVars;

// test the LoadVars constuctor
check_equals (typeof(loadvarsObj), 'object');

// test the LoadVars::addrequestheader method
check (LoadVars.prototype.hasOwnProperty('addRequestHeader'));
check_equals (typeof(loadvarsObj.addRequestHeader), 'function');

// test the LoadVars::decode method
check (LoadVars.prototype.hasOwnProperty('decode'));
check_equals (typeof(loadvarsObj.decode), 'function');
ret = loadvarsObj.decode("ud3=3&ud2=2");
check_equals(typeof(ret), 'undefined');
check_equals (loadvarsObj.ud3, 3);
check_equals (loadvarsObj.ud2, 2);
loadvarsObj.decode("ud4=4&ud2=20");
check_equals (loadvarsObj.ud4, 4);
check_equals (loadvarsObj.ud3, 3);
check_equals (loadvarsObj.ud2, 20);
ret = loadvarsObj.decode();
check_equals( typeof(ret), 'boolean');
check_equals( ret, false );
ret = loadvarsObj.decode("");
check_equals( typeof(ret), 'undefined');
o = {};
ret = loadvarsObj.decode(o);
check_equals( typeof(ret), 'undefined');
o.toString = function() { return "ud5=5"; };
ret = loadvarsObj.decode(o);
check_equals( typeof(ret), 'undefined');
check_equals (loadvarsObj.ud5, 5);
bomstarting="﻿ud6=6&ud7=7";
ret = loadvarsObj.decode(bomstarting);
check_equals( typeof(ret), 'undefined');
check_equals (loadvarsObj.ud6, undefined);
check_equals (loadvarsObj['﻿ud6'], 6);
check_equals (loadvarsObj.ud7, 7);

// test the LoadVars::getbytesloaded method
check (LoadVars.prototype.hasOwnProperty('getBytesLoaded'));
check_equals (typeof(loadvarsObj.getBytesLoaded), 'function');

// test the LoadVars::getbytestotal method
check (LoadVars.prototype.hasOwnProperty('getBytesTotal'));
check_equals (typeof(loadvarsObj.getBytesTotal), 'function');

// test the LoadVars::load method
check (LoadVars.prototype.hasOwnProperty('load'));
check_equals (typeof(loadvarsObj.load), 'function');

// test the LoadVars::send method
check (LoadVars.prototype.hasOwnProperty('send'));
check_equals (typeof(loadvarsObj.send), 'function');

// test the LoadVars::sendandload method
check (LoadVars.prototype.hasOwnProperty('sendAndLoad'));
check_equals (typeof(loadvarsObj.sendAndLoad), 'function');

// test the LoadVars::tostring method
check (LoadVars.prototype.hasOwnProperty('toString'));
check_equals (typeof(loadvarsObj.toString), 'function');

// test the LoadVars::tostring method
check (!LoadVars.prototype.hasOwnProperty('valueOf'));
check_equals (loadvarsObj.valueOf, Object.prototype.valueOf);
check_equals (typeof(loadvarsObj.valueOf), 'function');

// test the LoadVars::onData method
check (LoadVars.prototype.hasOwnProperty('onData'));
check_equals (typeof(loadvarsObj.onData), 'function');

// test the LoadVars::onLoad method
check (LoadVars.prototype.hasOwnProperty('onLoad'));
check_equals (typeof(loadvarsObj.onLoad), 'function');

// test the LoadVars::loaded member
check (!LoadVars.prototype.hasOwnProperty('loaded'));
check_equals (typeof(loadvarsObj.loaded), 'undefined');

x = new LoadVars;

r = new Object;
check(!r.hasOwnProperty("loaded"));
check_equals(x.sendAndLoad("some server name", r), true);
check(r.hasOwnProperty("loaded"));

r = new XML;
check(!r.hasOwnProperty("loaded"));
check_equals(x.sendAndLoad("some server name", r), true);
check(!r.hasOwnProperty("loaded"));
check_equals(typeof(r.loaded), "boolean");
check_equals(r.loaded, false);

r = new LoadVars;
check(!r.hasOwnProperty("loaded"));
check_equals(x.sendAndLoad("some server name", r), true);
check(r.hasOwnProperty("loaded"));
check_equals(typeof(r.loaded), "boolean");
check_equals(r.loaded, false);

r = new Date(1);
check(!r.hasOwnProperty("loaded"));
check_equals(x.sendAndLoad("some server name", r), true);
check(r.hasOwnProperty("loaded"));
check_equals(typeof(r.loaded), "boolean");
check_equals(r.loaded, false);
t = new Date(1);
check_equals(r.toString(), t.toString());
check(r instanceOf Date);

r = 3;
check(!r.hasOwnProperty("loaded"));
check_equals(x.sendAndLoad("some server name", r), false);
check(!r.hasOwnProperty("loaded"));
check_equals(typeof(r.loaded), "undefined");
check_equals(r.loaded, undefined);

r = "string";
check(!r.hasOwnProperty("loaded"));
check_equals(x.sendAndLoad("some server name", r), false);
check(!r.hasOwnProperty("loaded"));
check_equals(typeof(r.loaded), "undefined");
check_equals(r.loaded, undefined);

r = {};
check(!r.hasOwnProperty("loaded"));
check_equals(x.sendAndLoad("some server name", r), true);
check(r.hasOwnProperty("loaded"));
check_equals(typeof(r.loaded), "boolean");
check_equals(r.loaded, false);

// For checking that the data were loaded with XML.prototype.load.
x.onLoad = function(success) {
    check_equals(x['var2'], 'val2');
    check_totals(144);
    play();
};

// This is called from the next onLoad() handler to make sure there is no
// race condition for end of test.
checkXMLAndLoadVarsInterchangeable = function() {

    // The two objects are also interchangeable for these functions.
    x.sendAndLoad = XML.prototype.sendAndLoad;
    check_equals(x.sendAndLoad("some server name", r), true);
    x.load = XML.prototype.load;
    stop();
    check_equals(x.load( MEDIA(vars.txt) ), true);
};

//--------------------------------------------------------------------------
// Test LoadVars::toString()
//--------------------------------------------------------------------------

lv = new LoadVars();
lv.a = 3;
check_equals(lv.toString(), "a=3");

lv.b = "string";
check_equals(lv.toString(), "b=string&a=3");

lv.c = Mouse.hide;
check_equals(lv.toString(), "c=%5Btype%20Function%5D&b=string&a=3");

lv["3"] = 6;
check_equals(lv.toString(), "3=6&c=%5Btype%20Function%5D&b=string&a=3");

o = { a:5, b:6 };
lv["f"] = o;
check_equals(lv.toString(), "f=%5Bobject%20Object%5D&3=6&c=%5Btype%20Function%5D&b=string&a=3");

lv[4] = "string";
check_equals(lv.toString(), "4=string&f=%5Bobject%20Object%5D&3=6&c=%5Btype%20Function%5D&b=string&a=3");

delete lv[3];
delete lv["f"];
check_equals(lv.toString(), "4=string&c=%5Btype%20Function%5D&b=string&a=3");

lv[o] = o;
check_equals(lv.toString(), "%5Bobject%20Object%5D=%5Bobject%20Object%5D&4=string&c=%5Btype%20Function%5D&b=string&a=3");

lv.b = undefined;
#if OUTPUT_VERSION > 6
check_equals(lv.toString(), "%5Bobject%20Object%5D=%5Bobject%20Object%5D&4=string&c=%5Btype%20Function%5D&b=undefined&a=3");
#else
check_equals(lv.toString(), "%5Bobject%20Object%5D=%5Bobject%20Object%5D&4=string&c=%5Btype%20Function%5D&b=&a=3");
#endif

tsc = 0;
voc = 0;

o = new Object();

o.toString = function() { tsc++; return "fake toString"; };
o.valueOf = function() { voc++; return "fake valueOf"; };

lv2 = new LoadVars();
lv2.a = o;
check_equals(tsc, 0);
check_equals(voc, 0);

check_equals(lv2.toString(), "a=fake%20toString");
check_equals(tsc, 1);
check_equals(voc, 0);

// This should *not* call valueOf.
o.toString = undefined;
xcheck_equals(lv2.toString(), "a=%5Btype%20Object%5D");
check_equals(tsc, 1);
xcheck_equals(voc, 0);

// Check override of _global.escape

bu = _global.escape;
_global.escape = function(str) { return "FAKED!"; };
lv = new LoadVars();
lv.a = "&";
check_equals(lv.toString(), "FAKED!=FAKED!");
_global.escape = bu;
check_equals(lv.toString(), "a=%26");

//--------------------------------------------------------------------------
// Test LoadVars::load()
//--------------------------------------------------------------------------

varsloaded = 0;
datareceived = 0;
//var1 = undefined;
//var2 = undefined;
loadvarsObj.onLoad = function(success) {
	varsloaded++;
	note("LoadVars.onLoad called "+varsloaded+". "
		+"Bytes loaded: "+loadvarsObj.getBytesLoaded()
		+"/"+loadvarsObj.getBytesTotal());

	//delete loadvarsObj; // this to test robustness

	check_equals (loadvarsObj.getBytesTotal(), loadvarsObj.getBytesLoaded());
	check_equals (loadvarsObj.getBytesLoaded(), 1126);
	check_equals (loadvarsObj.getBytesTotal(), loadvarsObj._bytesTotal);
	check_equals (loadvarsObj._bytesLoaded, loadvarsObj._bytesTotal);
	check_equals (loadvarsObj._bytesLoaded, 1126);
	check_equals (this, loadvarsObj);
	check_equals(arguments.length, 1);
	check_equals(typeof(success), 'boolean');
	check_equals(success, true);
	check_equals(this.loaded, success);
	check_equals(typeof(this.loaded), 'boolean');
	this.loaded = 5;
	check_equals(typeof(this.loaded), 'number');

	check(varsloaded < 3);

	// onLoad is called after all vars have been called
	check_equals( loadvarsObj.getBytesLoaded(), loadvarsObj.getBytesTotal() );

	//for (var i in _root) { note("_root["+i+"] = "+_root[i]); }

    loadvarsObj._bytesTotal = 3;
	check_equals (loadvarsObj.getBytesTotal(), 3);
    loadvarsObj._bytesLoaded = 5;
	check_equals (loadvarsObj.getBytesLoaded(), 5);

	if ( varsloaded == 1 )
	{
		check_equals(loadvarsObj['var1'], 'val1');
		//check_equals(loadvarsObj['var1_check'], 'previous val1');
		check_equals(loadvarsObj['var2'], 'val2');
		//check_equals(loadvarsObj['v2_var1'], 'val1');
		//check_equals(loadvarsObj['v2_var2'], 'val2');

		// Gnash insists in looking for an ending & char !!		
		check_equals(loadvarsObj['var3'], 'val3\n');

        checkXMLAndLoadVarsInterchangeable();

		stop();
	}
};

// onData is called once with full parsed content.
loadvarsObj.onDataReal = loadvarsObj.onData;
loadvarsObj.onData = function(src) {
	check_equals (this, loadvarsObj);
	check_equals(typeof(this.loaded), 'boolean');
	check_equals(this.loaded, false);
	check_equals(arguments.length, 1);
	check_equals(typeof(src), 'string');
	check_equals(src.substr(0, 10), 'var1=val1&');
	check_equals(src.substr(loadvarsObj.getBytesTotal()-13), 'var3=val3\n');
	check_equals(datareceived, 0);
	datareceived++; // we expecte it to be called only once ?
	note("LoadVars.onData called ("+datareceived+"), byte loaded: "
		+loadvarsObj.getBytesLoaded()
		+"/"+loadvarsObj.getBytesTotal());
	this.onDataReal(src);
	//check_equals(loadvarsObj['var1'], 'val1');
	//check_equals(loadvarsObj['var2'], 'val2');
	//for (var i in _root) { note("_root["+i+"] = "+_root[i]); }
	//play();
};

loadvarsObj.var1 = "previous val1";

// We expect the loaded file to return this string:
//
// 	"var1=val1&var2=val2&"
//
// The final '&' char is important, and it must
// not start with a '?' char.
// 
check( loadvarsObj instanceOf LoadVars );
check(!loadvarsObj.hasOwnProperty('_bytesLoaded'));
check(!loadvarsObj.hasOwnProperty('_bytesTotal'));
check( loadvarsObj.load( MEDIA(vars.txt) ) );

check(loadvarsObj.hasOwnProperty('_bytesLoaded'));
check(loadvarsObj.hasOwnProperty('_bytesTotal'));

check_equals(loadvarsObj._bytesLoaded, 0);
check_equals(loadvarsObj._bytesTotal, undefined);
check_equals(loadvarsObj.getBytesLoaded(), 0);
check_equals(loadvarsObj.getBytesTotal(), undefined);

check_equals(typeof(this.loaded), 'undefined');
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

