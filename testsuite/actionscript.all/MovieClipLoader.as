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
//

// Test case for MovieClipLoader ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: MovieClipLoader.as,v 1.19 2008/03/11 19:31:47 strk Exp $";
#include "check.as"

//#define MEDIA(x) x

// MovieClipLoader was added in player7
#if OUTPUT_VERSION >= 7

check_equals(typeOf(MovieClipLoader), 'function');
check_equals(typeOf(MovieClipLoader.prototype), 'object');
check(MovieClipLoader.prototype.hasOwnProperty('loadClip'));
check(MovieClipLoader.prototype.hasOwnProperty('unloadClip'));
check(MovieClipLoader.prototype.hasOwnProperty('getProgress'));
check(MovieClipLoader.prototype.hasOwnProperty('addListener'));
check(MovieClipLoader.prototype.hasOwnProperty('removeListener'));
check(MovieClipLoader.prototype.hasOwnProperty('broadcastMessage'));
check_equals(typeOf(MovieClipLoader._listeners), 'undefined');

var mcl = new MovieClipLoader();
check_equals(typeOf(mcl), 'object');
check(mcl instanceOf MovieClipLoader);

check_equals(typeOf(mcl.loadClip), 'function');
check(!mcl.hasOwnProperty('loadClip'));

check_equals(typeOf(mcl.unloadClip), 'function');
check(!mcl.hasOwnProperty('unloadClip'));

check_equals(typeOf(mcl.getProgress), 'function');
check(!mcl.hasOwnProperty('getProgress'));

check_equals(typeOf(mcl.addListener), 'function');
check(!mcl.hasOwnProperty('addListener'));

check_equals(typeOf(mcl.removeListener), 'function');
check(!mcl.hasOwnProperty('removeListener'));

check_equals(typeOf(mcl.broadcastMessage), 'function');
check(!mcl.hasOwnProperty('broadcastMessage'));

check_equals(typeOf(mcl._listeners), 'object');
check(mcl.hasOwnProperty('_listeners'));
check_equals(mcl._listeners.length, 1);
check_equals(mcl._listeners[0], mcl);

MovieClipLoader.prototype.bm = MovieClipLoader.prototype.broadcastMessage;
MovieClipLoader.prototype.broadcastMessage = function(arg1, arg2, arg3, arg4)
{
	//note("Broadcasting "+arg1);
	//this.bm(arg1, arg2, arg3, arg4);
	this.bm.apply(this, arguments);
};

//---------------------------------------------------------
// Test loadClip and events
//---------------------------------------------------------
//
// TODO: test even handlers (actionscript.all framework
//       not enough for this)
//
// Invoked when a file loaded with MovieClipLoader.loadClip() has failed to load.
// MovieClipLoader.onLoadError
//
// Invoked when a call to MovieClipLoader.loadClip() has successfully begun to download a file.
// MovieClipLoader.onLoadStart
// 
// Invoked every time the loading content is written to disk during the loading process.
// MovieClipLoader.onLoadProgress
//
// Invoked when a file loaded with MovieClipLoader.loadClip() has completely downloaded.
// MovieClipLoader.onLoadComplete
// 
// Invoked when the actions on the first frame of the loaded clip have been executed.
// MovieClipLoader.onLoadInit
//
//---------------------------------------------------------

dumpObj = function(o, lvl)
{
	if ( typeof(lvl) == 'undefined' ) lvl=0;
	for (var i in o)
	{
		note(lvl+' - '+i+': '+o[i]+' ('+typeof(o[i])+')');
		if ( typeof(o[i]) == 'object' ) dumpObj(o[i], lvl+1);
	}
};


createEmptyMovieClip("loadtarget", 10);

expected = {
	target: undefined
};

resetState = function()
{
	state = {
		onLoadStartCalls:0,
		onLoadErrorCalls:0,
		onLoadProgressCalls:0,
		onLoadCompleteCalls:0,
		onLoadInitCalls:0
	};
};
totalProgressCalls=0;

nextTestOrEnd = function()
{
	//note("nextTestOrEnd");
	if ( state.nextFunction == undefined )
	{
		// we don't know how many times onLoadProgress will be called
		// so we have that handler increment a totalProgressCalls and
		// we use that value to  figure out how many tests to expect to
		// be run (6 tests each onLoadProgress call).
		// Note that if we miss to call onLoadProgress at all we'd catch
		// the bug from supposedly subsequent callbacks, which check for
		// a local flag set by the onLoadProgress handler.
		//
		var testsPerProgressCallback = 15;
		progCallbackTests = totalProgressCalls*testsPerProgressCallback;
		note("Number of onLoadProgress runs: "+totalProgressCalls+" - tests: "+progCallbackTests);
		if ( expected.failtotals ) {
			// this is failing due to vars.txt not being loaded
			// (how could it?)
			xcheck_totals(expected.totals + progCallbackTests);
		} else {
			check_totals(expected.totals + progCallbackTests);
		}
		play();
	}
	else
	{
		state.nextFunction();
	}
};

mcl.onLoadError = function(target, msg, n)
{
	check_equals(arguments.length, 3);
	check_equals(target, expected.target);
	note("onLoadError called ("+msg+")");
	nextTestOrEnd();
	//dumpObj(arguments);
};

mcl.onLoadStart = function(target)
{
	check_equals(arguments.length, 1);
	// a bug in Gnash made softrefs always convert
	// to the empty string when not pointing to
	// their original target...
	check(target+"" != "");
	check_equals(target, expected.target);
	state.onLoadStartCalls++;
	note("onLoadStart("+target+", "+target._url+") called");
	//note("onLoadStart called with "+arguments.length+" args:"); dumpObj(arguments);
};

mcl.onLoadProgress = function(target, bytesLoaded, bytesTotal)
{
	//note("onLoadProgress("+target+", "+target._url+") called");

	check_equals(arguments.length, 3);
	check_equals(target, expected.target);
	check_equals(state.onLoadStartCalls, 1);
	check_equals(typeof(bytesLoaded), 'number')
	check_equals(typeof(bytesTotal), 'number')
	check(bytesTotal <= bytesTotal);

	check_equals(this, mcl);

	var tmp = this.getProgress();
	check_equals(typeof(tmp), 'undefined');

	var prog = this.getProgress(target);
	check_equals(typeof(prog), 'object');
	check_equals(prog.__proto__, undefined);
	check_equals(prog.bytesLoaded, bytesLoaded);
	check_equals(prog.bytesTotal, bytesTotal);
	var progcopy = {}; var progcount=0;
	for (var i in prog) { progcopy[i] = prog[i]; progcount++; }
	check_equals(progcount, 2);
	check_equals(progcopy.bytesLoaded, bytesLoaded);
	check_equals(progcopy.bytesTotal, bytesTotal);

	++state.onLoadProgressCalls;
	++totalProgressCalls;
	//note("onLoadProgress called with "+arguments.length+" args:"); dumpObj(arguments);
};

mcl.onLoadComplete = function(target, n)
{
	note("onLoadComplete("+target+", "+target._url+") called");
	check_equals(arguments.length, 2);
	check_equals(target, expected.target);
	check_equals(state.onLoadStartCalls, 1);
	check(state.onLoadProgressCalls > 0);
	check_equals(typeof(n), 'number'); // what is this ?
	state.onLoadCompleteCalls++;

	note("onLoadComplete second arg is "+n+" ("+typeof(n)+")");
};

mcl.onLoadInit = function(target)
{
	note("onLoadInit("+target+", "+target._url+") called");
	check_equals(arguments.length, 1);
	check_equals(target, expected.target);
	check_equals(state.onLoadStartCalls, 1);
	check(state.onLoadProgressCalls > 0);
	check_equals(state.onLoadCompleteCalls, 1);
	state.onLoadInitCalls++;

	check_equals(this, mcl);

	// At onLoadInit time the _url parameter is the url of
	// old target with appended url of new target (?)
	// So we test if it ends with it instead
	var indexOfGreenJpeg = target._url.indexOf('green.jpg');
	var isGreenJpeg = ( (target._url.length - indexOfGreenJpeg) == 9 );
	if ( isGreenJpeg )
	{
		//trace("It's the jpeg go !");
		with (target)
		{
			note("_lockroot: "+_lockroot);
			note("---- Target's root is "+_root);
			check_equals(_root, _level0);
			_lockroot = true;
			check_equals(_root, _level0.loadtarget);
			note("---- After setting _lockroot to true, target's root is "+_root);
			_lockroot = false;
			check_equals(_root, _level0);
			note("---- After setting _lockroot to false, target's root is "+_root);
		}
		// NOTE: these two tests show the bug preventing
		//       YouTube active_sharing.swf movie from working
		check_equals(target._width, 170); 
		check_equals(target._height, 170);
	}

	//note("target.var1: "+target.var1);
	//note("onLoadInit called with "+arguments.length+" args:"); dumpObj(arguments);
	nextTestOrEnd();
};

check( ! mcl.loadClip() );
check( ! mcl.loadClip( MEDIA(vars.txt) ) );

check( ! mcl.loadClip( MEDIA(vars.txt), 'unexistent' ) );

function test1()
{
	resetState();
	state.nextFunction = test2;
	expected.target = _root.loadtarget;
	check( mcl.loadClip( MEDIA(unexistent), 'loadtarget' ) );
}

function test2()
{
	resetState();
	state.nextFunction = test3;
	expected.target = _root.loadtarget;
	check( mcl.loadClip( MEDIA(vars.txt), 'loadtarget' ) );
}

function test3()
{
	// getProgress can be called using *any* target
	// and will return the target's actual size
	var prog = mcl.getProgress(_level0);
	check_equals(typeof(prog), 'object');
	check_equals(prog.__proto__, undefined);
	check_equals(prog.bytesLoaded, prog.bytesTotal);
	check_equals(prog.bytesTotal, _level0.getBytesTotal());

	resetState();
	state.nextFunction = undefined;
	expected.target = _root.loadtarget;

	// set expected.totals when willing to end the test
	// we don't know how many times onLoadProgress will be called
	// so we only count the tests we can determine.
	// The  onLoadInit function will do the rest, by checking the actual
	// number of times onLoadProgress was called and add to our expected
	// count (appropriately multiplied to count *every* test in the
	// onLoadProgress)
	//
	// subtract the number of progress callback runs reported when playing from the totals to get the correct number
	// BUT MAKE SURE nextTestOrEnd CONTAINS THE CORRECT testsPerProgressCallback INFO !!
	//
	expected.totals = 72;
	// gnash doesn't call onLoadInit if the data at the url is not an SWF or JPG
	// (or whatever else can become a movie_instance), while the PP does.
	// So in this testcase, the attempt to load vars.txt is invalid for Gnash
	// (triggers onLoadError) 
	// TODO: fix gnash to be compatible and find out if there's anything
	//       actually dont for loadVariable-like data
	//
	expected.failtotals = true;


	loadtarget._y = 200;
	loadtarget._x = 200;
	loadtarget._alpha = 20;
	check( mcl.loadClip( MEDIA(green.jpg), 'loadtarget' ) );

}

// Due to a bug in Gnash we must stop() before calling test1.
// This is because Gnash's version of loadClip is blocking !!
// TODO: fix it !
stop();

test1();

#else // OUTPUT_VERSION < 7

totals();

#endif
