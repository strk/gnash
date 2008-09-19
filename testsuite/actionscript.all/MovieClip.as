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

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: MovieClip.as,v 1.133 2008/05/09 13:21:08 strk Exp $";
#include "check.as"


#if OUTPUT_VERSION == 5
Object.prototype.hasOwnProperty = ASnative(101, 5);
#endif

xcheck(MovieClip.prototype.hasOwnProperty("blendMode"));
check(MovieClip.prototype.hasOwnProperty("attachBitmap"));
xcheck(MovieClip.prototype.hasOwnProperty("cacheAsBitmap"));
check(MovieClip.prototype.hasOwnProperty("enabled"));
xcheck(MovieClip.prototype.hasOwnProperty("filters"));
xcheck(MovieClip.prototype.hasOwnProperty("forceSmoothing"));
xcheck(MovieClip.prototype.hasOwnProperty("opaqueBackground"));
xcheck(MovieClip.prototype.hasOwnProperty("scale9Grid"));
xcheck(MovieClip.prototype.hasOwnProperty("scrollRect"));
xcheck(MovieClip.prototype.hasOwnProperty("tabIndex"));
xcheck(MovieClip.prototype.hasOwnProperty("transform"));
check(MovieClip.prototype.hasOwnProperty("useHandCursor"));
check(MovieClip.prototype.hasOwnProperty("_lockroot"));

check(!MovieClip.prototype.hasOwnProperty("focusEnabled"));
check(!MovieClip.prototype.hasOwnProperty("hitArea"));
check(!MovieClip.prototype.hasOwnProperty("menu"));
check(!MovieClip.prototype.hasOwnProperty("tabChildren"));
check(!MovieClip.prototype.hasOwnProperty("tabEnabled"));
check(!MovieClip.prototype.hasOwnProperty("trackAsMenu"));
check(!MovieClip.prototype.hasOwnProperty("_alpha"));
check(!MovieClip.prototype.hasOwnProperty("_currentframe"));
check(!MovieClip.prototype.hasOwnProperty("_droptarget"));
check(!MovieClip.prototype.hasOwnProperty("_focusrect"));
check(!MovieClip.prototype.hasOwnProperty("_height"));
check(!MovieClip.prototype.hasOwnProperty("_highquality"));
check(!MovieClip.prototype.hasOwnProperty("_name"));
check(!MovieClip.prototype.hasOwnProperty("_parent"));
check(!MovieClip.prototype.hasOwnProperty("_quality"));
check(!MovieClip.prototype.hasOwnProperty("_rotation"));
check(!MovieClip.prototype.hasOwnProperty("_soundbuftime"));
check(!MovieClip.prototype.hasOwnProperty("_target"));
check(!MovieClip.prototype.hasOwnProperty("_totalframes"));
check(!MovieClip.prototype.hasOwnProperty("_url"));
check(!MovieClip.prototype.hasOwnProperty("_visible"));
check(!MovieClip.prototype.hasOwnProperty("_width"));
check(!MovieClip.prototype.hasOwnProperty("_x"));
check(!MovieClip.prototype.hasOwnProperty("_xmouse"));
check(!MovieClip.prototype.hasOwnProperty("_xscale"));
check(!MovieClip.prototype.hasOwnProperty("_y"));
check(!MovieClip.prototype.hasOwnProperty("_ymouse"));
check(!MovieClip.prototype.hasOwnProperty("_yscale"));


// To be called at end of test
endOfTest = function() 
{
#if OUTPUT_VERSION <= 5
	check_totals(276); // SWF5
#endif

#if OUTPUT_VERSION == 6
	check_totals(706); // SWF6
#endif

#if OUTPUT_VERSION == 7
	check_totals(723); // SWF7
#endif

#if OUTPUT_VERSION >= 8
	check_totals(748); // SWF8+
#endif

	play();
};

#if OUTPUT_VERSION < 6
note("WARNING: it has been reported that adobe flash player version 9 fails a few tests here.");
note("         We belive those are actually adobe player bugs since older versions ");
note("         of the player are reported to pass all tests. If you have another idea ");
note("         we'd be glad to hear from you, just check the testcase source code.");
note();
#endif

// Get a reference to a MovieClip
var mc = _root;
check_equals(typeof(mc), "movieclip");

// Check some references
check_equals(typeof(this), 'movieclip');
check_equals(typeof(_parent), 'undefined');
#if OUTPUT_VERSION > 5
 xcheck(!mc.hasOwnProperty('_parent'));
 check(!MovieClip.prototype.hasOwnProperty('_parent'));
#endif
check_equals(_root, this);
check_equals(typeof(this['_root']), 'movieclip');
check_equals(typeof(this['_level0']), 'movieclip');
check_equals(typeof(this['this']), 'undefined');
check_equals(_root['_root'], _root);
check_equals(_level0['_root'], _root);
check_equals(_root['_level0'], _root);
#if OUTPUT_VERSION >= 6
check_equals(typeof(_root['_global']), 'object');
check_equals(_root.hasOwnProperty('_root'), false);
#endif
check_equals(typeof(_global['_root']), 'undefined');
x = new Object();
check_equals(x['_root'], undefined);


// Check inheritance
check(MovieClip);
check_equals(mc.__proto__, MovieClip.prototype);
check_equals(typeof(MovieClip.prototype._width), "undefined");
check_equals(typeof(MovieClip.prototype.attachMovie), "function");
check_equals(typeof(MovieClip.prototype.__proto__), "object");
check_equals(MovieClip.prototype.__proto__, Object.prototype);
check_equals(typeof(mc._width), "number");

// Check methods existance

// SWF5 or higher
check_equals(typeof(mc.attachMovie), 'function');
check_equals(typeof(mc.getBytesLoaded), 'function');
check_equals(typeof(mc.getBytesTotal), 'function');
check_equals(typeof(mc.getBounds), 'function');
check_equals(typeof(mc.globalToLocal), 'function');
check_equals(typeof(mc.localToGlobal), 'function');
check_equals(typeof(mc.unloadMovie), 'function');
check_equals(typeof(mc.meth), 'function');
check_equals(typeof(mc.getSWFVersion), 'function');
check_equals(mc.getSWFVersion(), OUTPUT_VERSION);
check_equals(MovieClip.constructor, Function);

#if OUTPUT_VERSION >= 6
check(MovieClip.prototype.hasOwnProperty('loadMovie'));
check(MovieClip.prototype.hasOwnProperty('_lockroot')); 
check(!MovieClip.prototype.hasOwnProperty('loadMovieNum'));
check(!MovieClip.prototype.hasOwnProperty('valueOf')); 
check(!MovieClip.prototype.hasOwnProperty('toString')); 
check(MovieClip.prototype.hasOwnProperty('meth')); 
check(MovieClip.prototype.hasOwnProperty('useHandCursor')); 
#endif
check_equals(typeof(mc.valueOf), 'function');
check_equals(typeof(mc.toString), 'function');

check_equals(typeof(mc.valueOf()), 'movieclip');

check_equals(typeof(mc.enabled), 'boolean');
#if OUTPUT_VERSION >= 6
check(!mc.hasOwnProperty('enabled'));
check(mc.__proto__.hasOwnProperty('enabled'));
check(!mc.hasOwnProperty('valueOf'));
check(!mc.hasOwnProperty('toString'));
#endif

check_equals(mc.enabled, true);
mc.enabled = false;
check_equals(typeof(mc.enabled), 'boolean');
check_equals(mc.enabled, false);
mc.enabled = 'a string';
check_equals(typeof(mc.enabled), 'string');
check_equals(mc.enabled, 'a string');
mc.enabled = 56.5;
check_equals(typeof(mc.enabled), 'number');
check_equals(mc.enabled, 56.5);
check(delete mc.enabled);
check_equals(typeof(mc.enabled), 'boolean');
check_equals(mc.enabled, true);
mc.__proto__.enabled = 'a string';
check_equals(typeof(mc.enabled), 'string'); // yes, we can set to arbitrary values
check_equals(mc.enabled, 'a string'); // yes, we can set to arbitrary values
mc.__proto__.enabled = true; // better keep as it was initially, who knows what it would do...

// NOTE: due to a bug in Ming < 00040005, mc.loadMovie would
//       be converted to lowercase, so we use the [] hack
check_equals(typeof(mc['duplicateMovieClip']), 'function');

#if OUTPUT_VERSION >= 6
    check_equals(typeof(mc.setMask), 'function');
    check_equals(typeof(mc.beginFill), 'function');
    check_equals(typeof(mc.beginGradientFill), 'function');
    check_equals(typeof(mc.clear), 'function');
    check_equals(typeof(mc.createEmptyMovieClip), 'function');
    check_equals(typeof(mc.createTextField), 'function');
    check_equals(typeof(mc.curveTo), 'function');
    check_equals(typeof(mc.lineStyle), 'function');
    check_equals(typeof(mc.lineTo), 'function');
    check_equals(typeof(mc.moveTo), 'function');
    check_equals(typeof(mc.attachAudio), 'function');
    check_equals(typeof(mc.endFill), 'function');
    check_equals(typeof(mc.getDepth), 'function');
    check_equals(typeof(mc.getURL), 'function');
    check_equals(typeof(mc.gotoAndPlay), 'function');
    check_equals(typeof(mc.gotoAndStop), 'function');
    check_equals(typeof(mc.hitTest), 'function');
    check_equals(typeof(mc.nextFrame), 'function');
    check_equals(typeof(mc.play), 'function');
    check_equals(typeof(mc.prevFrame), 'function');
    check_equals(typeof(mc.stop), 'function');
    check_equals(typeof(mc.swapDepths), 'function');
    check_equals(typeof(mc.startDrag), 'function');
    check_equals(typeof(mc.stopDrag), 'function');
    check_equals(typeof(mc.getTextSnapshot), 'function');

    // NOTE: due to a bug in Ming < 00040005, mc.loadMovie would
    //       be converted to lowercase, so we use the [] hack
    check_equals(typeof(mc['loadMovie']), 'function');
    check_equals(typeof(mc['removeMovieClip']), 'function');

    check(MovieClip.prototype.hasOwnProperty('removeMovieClip'));
    check(!mc.hasOwnProperty('removeMovieClip'));

#endif // OUTPUT_VERSION >= 6

#if OUTPUT_VERSION >= 7

    check_equals(typeof(mc.getInstanceAtDepth), 'function');
    check( MovieClip.prototype.hasOwnProperty('getInstanceAtDepth') );
    check( ! mc.hasOwnProperty('getInstanceAtDepth') );

    // can't confirm this works !
    // maybe we should just NOT use the _root for this ?
    //check(mc.loadVariables != undefined);

    // maybe this is the start condition...
    check_equals(mc.focusEnabled, undefined);
    check_equals(mc.hitArea, undefined);
    check_equals(mc.menu, undefined);

    check_equals(mc.getNextHighestDepth(), 0);
#else
    check_equals(mc.getNextHighestDepth(), undefined);
#endif

// Even handlers are initially undefined, user can
// assign them a function to be called on that event...
check_equals(mc.onData, undefined);
check_equals(mc.onDragOut, undefined);
check_equals(mc.onDragOver, undefined);
check_equals(mc.onEnterFrame, undefined);
check_equals(mc.onKeyDown, undefined);
check_equals(mc.onKeyUp, undefined);
check_equals(mc.onKillFocus, undefined);
check_equals(mc.onLoad, undefined);
check_equals(mc.onMouseDown, undefined);
check_equals(mc.onMouseMove, undefined);
check_equals(mc.onMouseUp, undefined);
check_equals(mc.onPress, undefined);
check_equals(mc.onRelease, undefined);
check_equals(mc.onReleaseOutside, undefined);
check_equals(mc.onRollOut, undefined);
check_equals(mc.onRollOver, undefined);
check_equals(mc.onSetFocus, undefined);
check_equals(mc.onUnload, undefined);
#if OUTPUT_VERSION >=6
check(! mc.hasOwnProperty('onData'));
check(! mc.hasOwnProperty('onDragOut'));
check(! mc.hasOwnProperty('onDragOver'));
check_equals(mc.hasOwnProperty('onEnterFrame'), false); 
check(! mc.hasOwnProperty('onKeyDown'));
check(! mc.hasOwnProperty('onKeyUp'));
check(! mc.hasOwnProperty('onKillFocus'));
check(! mc.hasOwnProperty('onLoad'));
check(! mc.hasOwnProperty('onMouseDown'));
check(! mc.hasOwnProperty('onMouseMove'));
check(! mc.hasOwnProperty('onMouseUp'));
check(! mc.hasOwnProperty('onPress'));
check(! mc.hasOwnProperty('onRelease'));
check(! mc.hasOwnProperty('onReleaseOutside'));
check(! mc.hasOwnProperty('onRollOut'));
check(! mc.hasOwnProperty('onRollOver'));
check(! mc.hasOwnProperty('onSetFocus'));
check(! mc.hasOwnProperty('onUnload'));
#endif

// Check property existance

// These are undefined by default
check_equals(mc.tabChildren, undefined);
mc.tabChildren = false;
check_equals(mc.tabChildren, false);
mc.tabChildren = true;
check_equals(mc.tabChildren, true);
check_equals(mc.tabEnabled, undefined);
check_equals(mc.tabIndex, undefined);
check_equals(mc.trackAsMenu, undefined);
check_equals(mc.useHandCursor, true);
mc.useHandCursor = false;
check_equals(mc.useHandCursor, false);
check_equals(mc._alpha, 100);
check(mc._currentframe != undefined);

if (typeof(mc._droptarget) != "string")
{
    fail("typeof(mc._droptarget) = "+typeof(mc._droptarget)+" (expected 'string') ["+__FILE__+":"+__LINE__+"]");
    note(" WARNING: some players have been reported to evaluate _droptarget to undefined, rather then the empty string. Reguardless of SWF target.");
}
else
{
    pass("typeof(mc._droptarget) = "+typeof(mc._droptarget)+" ["+__FILE__+":"+__LINE__+"]");
}

check(mc._focusrect != undefined);
check(mc._framesloaded != undefined);
check(mc._height != undefined);
check(mc._highquality != undefined);
check(mc._y != undefined);
check(mc._ymouse != undefined);
check(mc._yscale != undefined);
check_equals(typeof(MovieClip.prototype._lockroot), 'undefined');
check_equals(typeof(mc._lockroot), 'boolean');
check_equals(mc._lockroot, false);
mc._lockroot = 56;
check_equals(typeof(mc._lockroot), 'boolean');
check_equals(mc._lockroot, true);
mc._lockroot = "";
check_equals(typeof(mc._lockroot), 'boolean');
check_equals(mc._lockroot, false);

#if OUTPUT_VERSION > 5
check_equals(mc._name, "");
check_equals(typeof(mc._name), "string");
mc._name = "changed";
check_equals(typeof(mc._name), "string");
check_equals(typeof(mc), "movieclip");
#else
//WARNING: we have player 9 succeeds on this, and also player 7 fails on this
// don't know which one to trust.
xcheck_equals(mc._name, "");
#endif

check(mc._parent == undefined);
check(mc._rotation != undefined);
check(mc._soundbuftime != undefined);
check(mc._target != undefined);
check(mc._totalframes != undefined);
check(mc._url != undefined);
check(mc._visible != undefined);
check(mc._width != undefined);
check(mc._x != undefined);
check(mc._xmouse != undefined);
check(mc._xscale != undefined);

#if OUTPUT_VERSION >= 6

// focused test on _* properties
check_equals(typeof(mc._x), 'number');
xcheck(!mc.hasOwnProperty("_x"));
check(!mc.__proto__.hasOwnProperty("_x"));
check(!MovieClip.prototype.hasOwnProperty("_x"));

check_equals(typeof(mc._y), 'number');
xcheck(!mc.hasOwnProperty("_y"));
check(!mc.__proto__.hasOwnProperty("_y"));
check(!MovieClip.prototype.hasOwnProperty("_y"));

check_equals(typeof(mc._height), 'number');
xcheck(!mc.hasOwnProperty("_height"));
check(!mc.__proto__.hasOwnProperty("_height"));
check(!MovieClip.prototype.hasOwnProperty("_height"));

check_equals(typeof(mc._width), 'number');
xcheck(!mc.hasOwnProperty("_width"));
check(!mc.__proto__.hasOwnProperty("_width"));
check(!MovieClip.prototype.hasOwnProperty("_width"));

check_equals(typeof(mc._xscale), 'number');
xcheck(!mc.hasOwnProperty("_xscale"));
check(!mc.__proto__.hasOwnProperty("_xscale"));
check(!MovieClip.prototype.hasOwnProperty("_xscale"));

check_equals(typeof(mc._yscale), 'number');
xcheck(!mc.hasOwnProperty("_yscale"));
check(!mc.__proto__.hasOwnProperty("_yscale"));
check(!MovieClip.prototype.hasOwnProperty("_yscale"));

check_equals(typeof(mc._xmouse), 'number');
xcheck(!mc.hasOwnProperty("_xmouse"));
check(!mc.__proto__.hasOwnProperty("_xmouse"));
check(!MovieClip.prototype.hasOwnProperty("_xmouse"));

check_equals(typeof(mc._ymouse), 'number');
xcheck(!mc.hasOwnProperty("_ymouse"));
check(!mc.__proto__.hasOwnProperty("_ymouse"));
check(!MovieClip.prototype.hasOwnProperty("_ymouse"));

check_equals(typeof(mc._rotation), 'number');
xcheck(!mc.hasOwnProperty("_rotation"));
check(!mc.__proto__.hasOwnProperty("_rotation"));
check(!MovieClip.prototype.hasOwnProperty("_rotation"));

check_equals(typeof(mc._totalframes), 'number');
xcheck(!mc.hasOwnProperty("_totalframes"));
check(!mc.__proto__.hasOwnProperty("_totalframes"));
check(!MovieClip.prototype.hasOwnProperty("_totalframes"));

check(!mc.hasOwnProperty("_level"));
check(!mc.__proto__.hasOwnProperty("_level"));
xcheck(!mc.hasOwnProperty("_target"));
xcheck(!mc.hasOwnProperty("_url"));
xcheck(!mc.hasOwnProperty("_soundbuftime"));
xcheck(!mc.hasOwnProperty("_focusrect"));
xcheck(!mc.hasOwnProperty("_framesloaded"));
check(!mc.hasOwnProperty("_lockroot"));
xcheck(!mc.hasOwnProperty("_highquality"));
#endif //if OUTPUT_VERSION >= 6

//----------------------------------------------
// Test createEmptyMovieClip
//----------------------------------------------

#if OUTPUT_VERSION >= 6 // {

// Test movieclip creation
var mc2 = createEmptyMovieClip("mc2_mc", 50, 0, 0, 0);
check(mc2 != undefined);
check_equals(mc2_mc.getBytesLoaded(), 0);
check_equals(mc2_mc.getBytesTotal(), 0);
check_equals(mc2.getBytesLoaded(), 0);
check_equals(mc2.getBytesTotal(), 0);

xcheck(!mc2.hasOwnProperty('_parent'));

#if OUTPUT_VERSION > 6 // {
 check_equals(getInstanceAtDepth(50), mc2);
#endif // }

var mc3 = createEmptyMovieClip("mc3_mc", 50);
check(mc3 != undefined);
check_equals(mc3.getDepth(), 50);

#if OUTPUT_VERSION > 6 // {
check_equals(getInstanceAtDepth(50), mc3);
#endif // }

// By default useHandCursor is true 
check_equals(mc3.useHandCursor, true);
check(!mc3.hasOwnProperty("useHandCursor"));
// We add a mouse event handler, and expect this
// to make useHandCursor true
mc3.onMouseOver = function() { trace("over"); };
check_equals(mc3.useHandCursor, true);
mc3.useHandCursor = false;
check_equals(mc3.useHandCursor, false);
mc3.useHandCursor = "string";
check_equals(mc3.useHandCursor, "string");

check_equals(mc3_mc.getBytesLoaded(), 0);
check_equals(mc3_mc.getBytesTotal(), 0);
check_equals(mc3.getBytesLoaded(), 0);
check_equals(mc3.getBytesTotal(), 0);
check_equals(mc3_mc, _level0.mc3_mc);
check_equals(String(mc3_mc), "_level0.mc3_mc");

#endif // }


// Test the _target property
check_equals(_root._target, "/");

// Test the _level property
check_equals(typeof(_root._level), "movieclip");
check_equals(_root._level, _level0);

#if OUTPUT_VERSION >= 6
// unfortunately we can't use createEmptyMovieClip with
// lower SWF targets...
var mc4 = _root.createEmptyMovieClip("mc4_mc", 60);
check_equals(mc4._parent, _root);
check_equals(mc4._target, "/mc4_mc");
check_equals(targetPath(mc4), "_level0.mc4_mc");
var mc5 = mc4.createEmptyMovieClip("mc5_mc", 60);
check_equals(mc5._target, "/mc4_mc/mc5_mc");
check_equals(targetPath(mc5), "_level0.mc4_mc.mc5_mc");
check_equals(typeof(mc4_mc), 'movieclip');
check_equals(typeof(mc4_mc.mc5_mc), 'movieclip');
check_equals(typeof(mc4), 'movieclip');
check_equals(typeof(mc5), 'movieclip');
mc4._name = 'changed'; 
check_equals(typeof(mc4_mc), 'undefined');
check_equals(typeof(mc4_mc.mc5_mc), 'undefined');
check_equals(typeof(mc4), 'movieclip');
check_equals(typeof(mc5), 'movieclip');
check_equals(mc4._target, "/changed");
check_equals(mc5._target, "/changed/mc5_mc");
check_equals(targetPath(mc4), "_level0.changed");
check_equals(targetPath(mc5), "_level0.changed.mc5_mc");
check_equals(mc4.toString(), "[object Object]");
check_equals(mc5.toString(), "[object Object]");
check_equals(changed._target, "/changed");
check_equals(changed.mc5_mc._target, "/changed/mc5_mc");
check_equals(changed.toString(), "[object Object]");
check_equals(changed.mc5_mc.toString(), "[object Object]");
#endif // OUTPUT_VERSION >= 6

//--------------------------------------------------------------------------
// Test "soft" references
// See http://thread.gmane.org/gmane.comp.web.flashcoders.devel/84030
//--------------------------------------------------------------------------

// There's no such think as a _global.removeMovieClip
// What is referred to the "global" function does actually
// resolve to an ActionRemoveClip tag (0.25)
check_equals(typeof(_global.removeMovieClip), 'undefined');

#if OUTPUT_VERSION >= 6

// Here we create 3 clips
//	- hardref has no onUnload
//	- hardref2 has an onUnload
//	- hardref3 has no onUnload but a child with onUnload
// We'll see that when the three clips are removed from the stage
// those with any onUnload handler (either theirs or in their childrens)
// will still be on the stage, only with their depth shifted at -32769-depth
// (see character::removedRepthOffset)

softref = _root.createEmptyMovieClip("hardref", 60);
softref2 = _root.createEmptyMovieClip("hardref2", 70);
softref3 = _root.createEmptyMovieClip("hardref3", 80);
softref3child = softref3.createEmptyMovieClip("hardref3child", 1);
softref3child2 = softref3.createEmptyMovieClip("hardref3child2", 2);
softref3child.onUnload = function() { check_equals(this._target, '/hardref3/hardref3child'); note(this+".onUnload called"); };
hardref2.onUnload = function() { /*note(this+".onUnload called");*/ };
check_equals(typeof(hardref), 'movieclip');
check_equals(typeof(softref), 'movieclip');
check_equals(typeof(hardref2), 'movieclip');
check_equals(typeof(softref2), 'movieclip');
check_equals(typeof(hardref3), 'movieclip');
check_equals(typeof(softref3), 'movieclip');
check_equals(typeof(hardref3.hardref3child), 'movieclip');
check_equals(typeof(softref3child), 'movieclip');
softref.member = 1;
softref2.member = 2;
softref3.member = 3;
softref3child.member = '3child';
softref3child2.member = '3child2';
check_equals(typeof(softref.member), 'number');
check_equals(typeof(softref2.member), 'number');
check_equals(typeof(softref3.member), 'number');
check_equals(typeof(softref3child.member), 'string');
check_equals(softref.member, 1);
check_equals(softref2.member, 2);
check_equals(softref3.member, 3);
check_equals(softref3child.member, '3child');
check_equals(softref._target, "/hardref");
check_equals(softref2._target, "/hardref2");
check_equals(softref3._target, "/hardref3");
check_equals(softref3child._target, "/hardref3/hardref3child");
check_equals(hardref.getDepth(), 60);
check_equals(hardref2.getDepth(), 70);
check_equals(hardref3.getDepth(), 80);
check_equals(softref3child.getDepth(), 1);
check_equals(softref3child2.getDepth(), 2);
#if OUTPUT_VERSION > 6
 check_equals(getInstanceAtDepth(60), hardref);
 check_equals(getInstanceAtDepth(70), hardref2);
 check_equals(getInstanceAtDepth(80), hardref3);
 check_equals(hardref3.getInstanceAtDepth(1), hardref3.hardref3child);
 removeMovieClip(hardref); // using ActionRemoveClip (0x25)
 removeMovieClip(hardref2); // using ActionRemoveClip (0x25)
 removeMovieClip(hardref3); // using ActionRemoveClip (0x25)
 check_equals(getInstanceAtDepth(60), undefined);
 check_equals(getInstanceAtDepth(-32839), hardref2);
#else
 // just to test another way, ActionRemoveClip in SWF6 will work as well
 hardref.removeMovieClip(); // using the sprite's removeMovieClip 
 hardref2.removeMovieClip(); // using the sprite's removeMovieClip
 hardref3.removeMovieClip(); // using the sprite's removeMovieClip
 //softref.removeMovieClip(); // use the softref's removeMovieClip 
#endif

check_equals(typeof(hardref), 'undefined');
check_equals(typeof(hardref2), 'movieclip');
check_equals(typeof(hardref3), 'movieclip'); // still accessible due to onUnload defined for its child
check_equals(hardref2.getDepth(), -32839);
check_equals(hardref3.getDepth(), -32849);
check_equals(hardref3.hardref3child.getDepth(), 1);
check_equals(typeof(softref), 'movieclip');
check_equals(typeof(softref2), 'movieclip');
check_equals(typeof(softref3), 'movieclip');
check_equals(typeof(softref3child), 'movieclip');
check_equals(typeof(softref.member), 'undefined');
check_equals(typeof(softref._target), 'undefined');
check_equals("x"+softref, 'x');
check_equals(softref2.member, 2);
check_equals(softref2._target, '/hardref2');
check_equals(softref3.member, 3);
check_equals(softref3._target, '/hardref3');
check_equals(softref3child.member, '3child');
check_equals(softref3child._target, '/hardref3/hardref3child');
check_equals(softref3child.getDepth(), 1);
removeMovieClip(softref3child); // using ActionRemoveClip (0x25)
check_equals(softref3child2.member, '3child2');
check_equals(softref3child2._target, '/hardref3/hardref3child2');
check_equals(softref3child2.getDepth(), 2);
hardref = 4;
// Delete is needed, or further inspection functions will hit the variable before the character
delete hardref;
sr61 = _root.createEmptyMovieClip("hardref", 61);
hardref.member = 2;
check_equals(typeof(softref.member), 'number');
check_equals(softref.member, 2);

// Two movieclips can have the same name
sr62 = _root.createEmptyMovieClip("hardref", 62);
// Still, soft references correctly point each to
// it's distinct clip !
sr61.member = 6;
check_equals(sr61.member, 6);
check_equals(typeof(sr62.member), 'undefined');
check_equals(sr61._name, "hardref");
check_equals(sr62._name, "hardref");

// When getting a member by name, the one with lowest
// depth comes up first
check_equals(hardref.member, 6); // depth 61 < 62
sr60 = _root.createEmptyMovieClip("hardref", 60);
sr60.member = 60;
check_equals(hardref.member, 60); // depth 60 < 61 < 62

// Here we verify that the "soft-reference" always refers to
// the original target of the sprite it was bound to even
// if it's re-bound to a new sprite. In particular we check that:
//
//  1. Original target of a re-bound target isn't used for further
//     rebounding.
//  2. No rebind is attempted till the original sprite is destroyed (not simply unloaded!)
//  3. After original bounded sprite is unloaded, rebinding is *always*
//     attempted.
//

// _level0.hardref4 created, soft ref sr62 set
sr62 = _root.createEmptyMovieClip("hardref4", 62);
sr62.member = 'hardref4_original';
check_equals(sr62.member, "hardref4_original");

// _level0.hardref4_n created and renamed to _level0.hardref
// this does NOT trigger rebinding of sr62, still bound to
// it's original sprite
//
sr60 = _root.createEmptyMovieClip("hardref4_n", 60);
sr60._name = "hardref4";
check_equals(sr62.member, "hardref4_original");
sr60.removeMovieClip();
check_equals(sr62.member, "hardref4_original");

// _level0.hardref4 unloaded.
// From now on, sr62 is a dangling soft refefence
// and will try to rebind to something else
//
hardref4.removeMovieClip();
check_equals(typeof(hardref4), 'undefined'); 
check_equals(typeof(sr62.member), 'undefined');

// _level0.hardref4_with_another_name created
sr63 = _root.createEmptyMovieClip("hardref4_with_another_name", 63);
sr63.member = "hardref4_63";

// sr62 is still unbound
check_equals(typeof(sr62.member), "undefined");

// renamed _level0.hardref4_with_another_name to _level0.hardref4
// sr62 will now rebind to the new sprite.
// sr63 and sr63_bis refs created.
//
sr63._name = "hardref4";
sr63_bis = hardref4;
check_equals(sr62.member, "hardref4_63");

// A new sprite is created at depth 60 and named 
// _level0.hardref4 again
//
// The sr62 soft-ref is rebound again, even if the
// currently bound sprite wasn't unloaded !
// The sr63 soft-ref, originally bound to the character
// at depth 63, isn't rebound instead.
//
sr60 = _root.createEmptyMovieClip("hardref4_60", 60);
sr60.member = "hardref4_60";
sr60._name = "hardref4";
check_equals(hardref4.member, 'hardref4_60'); 
check_equals(sr62.member, "hardref4_60");
check_equals(sr63.member, "hardref4_63");

// Removing the at depth 60 (currently bound to sr62)
// exposes depth63 again for next binding attempt of sr62
sr62.removeMovieClip();
check_equals(sr62.member, "hardref4_63");

// depth 63 unloaded, sr62 is orphaned again
check_equals(hardref4, sr63);
hardref4.removeMovieClip();
check_equals(typeof(sr63.member), 'undefined');
check_equals(typeof(sr62.member), "undefined");

// Finally, we create a sprite at depth 64 and see how changing
// it's name triggers rebinding
sr64 = _root.createEmptyMovieClip("hardref4_with_yet_another_name", 64);
sr64.member = "hardref4_64";

// Naming it "hardref4" rebinds sr62 (dangling)
// but leaves sr63 orphaned (dangling, but pointing to
// _level0.hardref4_with_another_name)
// 
sr64._name = "hardref4";
check_equals(sr62.member, "hardref4_64");
check_equals(typeof(sr63.member), 'undefined');

// Naming it "hardref4_with_another_name"
// makes sr62 orphaned (dangling and pointing to _level0.hardref4),
// rebinds sr63 and sr63_bis to it
// (previously dangling and pointing to _level0.hardref4_with_another_name)
// 
sr64._name = "hardref4_with_another_name";
check_equals(typeof(sr62.member), "undefined");
check_equals(sr63.member, 'hardref4_64');
check_equals(sr63_bis.member, 'hardref4_64');
check_equals(sr64.member, 'hardref4_64');

// Now check that unloaded-but-not-destroyed sprite
// do NOT trigger rebinding
sr59 = createEmptyMovieClip("hardref5", 59);
sr59.member = "hardref5@59";
sr59.onUnload = function() {};
sr60 = createEmptyMovieClip("hardref5", 60);
sr60.member = "hardref5@60";
sr60.onUnload = function() {};
check_equals(sr59.getDepth(), 59);
check_equals(sr60.getDepth(), 60);
check_equals(sr59._target, "/hardref5");
check_equals(sr60._target, "/hardref5");
sr60.removeMovieClip();
check_equals(sr59.getDepth(), 59);
check_equals(sr60.getDepth(), -32829);
check_equals(sr59._target, "/hardref5");
check_equals(sr60._target, "/hardref5");
sr59.removeMovieClip();
// sr59, despite the fact hardref5@59 was unloaded, still
// points to the original sprite. This means no rebind is
// attempted, otherwise hardref5@60 will be found first
// being at a lower depth
check_equals(hardref5.getDepth(), -32829);
check_equals(hardref5._target, "/hardref5");
check_equals(hardref5.member, "hardref5@60");
// Gnash fails because it's "unload" event triggering rebinding
// rather then "destroy" event (unsupported in Gnash).
check_equals(sr59.getDepth(), -32828);
check_equals(sr59.member, "hardref5@59");
check_equals(sr59._target, "/hardref5");
check_equals(sr60.getDepth(), -32829);
check_equals(sr60._target, "/hardref5");
check_equals(sr60.member, "hardref5@60");

#endif // OUTPUT_VERSION >= 6

//----------------------------------------------
// Test duplicateMovieClip
//----------------------------------------------
#if OUTPUT_VERSION >= 6
_root.createEmptyMovieClip("original", 61);
_root.original.createEmptyMovieClip("child1", 1);
_root.original._x = 100;
_root.original.onEnterFrame = function() { };
_root.original.onRollOver = function() { };
check_equals(typeof(_root.original), 'movieclip');
check_equals(typeof(_root.original.child1), 'movieclip');
check_equals(_root.original._x, 100);
check_equals(typeof(_root.original.onEnterFrame), 'function');
check_equals(typeof(_root.original.onRollOver), 'function');

duplicateMovieClip(_root.original, "copy1", 63);
check_equals(typeof(_root.copy1), 'movieclip');
check_equals(typeof(_root.copy1.child1), 'undefined');
check_equals(typeof(_root.copy1.onEnterFrame), 'undefined');
check_equals(typeof(_root.copy1.onRollOver), 'undefined');
check_equals(_root.copy1.getDepth(), 63);
check_equals(_root.copy1._x, 100);

#if OUTPUT_VERSION == 6
// SWF7 and higher removed duplicateMovieClip method of MovieClip class
_root.original.duplicateMovieClip("copy2", 64);
check_equals(typeof(_root.copy2), 'movieclip');
check_equals(typeof(_root.copy2.child1), 'undefined');
check_equals(typeof(_root.copy2.onEnterFrame), 'undefined');
check_equals(typeof(_root.copy2.onRollOver), 'undefined');
check_equals(_root.copy2.getDepth(), 64);
check_equals(_root.copy2._x, 100);
#endif // OUTPUT_VERSION = 6
#endif // OUTPUT_VERSION >= 6

//----------------------------------------------
// Test timeline variables
//----------------------------------------------

var c = 1;
b = 1;
check_equals(c, 1);
check_equals(_root.c, 1);
check_equals(b, 1);
check_equals(_root.b, 1);

//----------------------------------------------
// Test new MovieClip
//----------------------------------------------

var cl = new MovieClip();
check_equals(cl.__proto__.constructor, MovieClip);
check_equals(cl.constructor, MovieClip);
check_equals(cl.__constructor__, MovieClip);
check(cl instanceOf MovieClip);
check(cl instanceOf Object);
check_equals(typeof(cl), "object");
check_equals(typeof(cl.attachMovie), "function");
check_equals(typeof(cl._width), "undefined");
check_equals(typeof(cl._parent), "undefined");

//------------------------------------------------
// Test onLoad to be allowed to be set to anything
//------------------------------------------------

_root.onLoad = 3;
check_equals(typeof(_root.onLoad), 'number');
_root.onLoad = "test";
check_equals(typeof(_root.onLoad), 'string');

//-----------------------------------------------------------
// Test $version
//-----------------------------------------------------------

#if OUTPUT_VERSION >= 6
check(this.hasOwnProperty("$version"));
#endif
check_equals(typeof(this.$version), 'string');

function enumerate(obj, enum)
{
    var enumlen = 0;
    for (var i in obj) {
        enum[i] = obj[i];
        ++enumlen;
    }
    return enumlen;
}

// Check that $version is enumerable
enum = new Object; enumlen = enumerate(this, enum);
check_equals(typeof(enum['$version']), 'string');

// Check that $version is overridable and deletable
this.$version = "fake version";
check_equals(this.$version, 'fake version');
check(delete $version);
check_equals(typeof(this.$version), 'undefined');

//------------------------------------------------
// Test getProperty 
//------------------------------------------------

#ifdef MING_SUPPORTS_ASM_GETPROPERTY

asm {
    push "a"
    push "" // this doesn't resolve to top of with stack
    push 13 // _name
    getproperty
    setvariable
};
#if OUTPUT_VERSION > 5
check_equals(a, "changed");
#else
if ( a == undefined )
{
    pass("<empty>._name (trough getProperty(13)) returns undefined ["+__FILE__+":"+__LINE__+"]");
}
else
{
    // this check fails with Adobe Flash Player 9
    fail("<empty>._name (trough getProperty(13)) returns "+a+" (expected undefined) ["+__FILE__+":"+__LINE__+"]");
    note("Some version of Adobe Flash Player 9 are reported to have this bug");
}
#endif

asm {
    push "b"
    push "" // this doesn't resolve to top of with stack
    push 11 // _target
    getproperty
    setvariable
};
check_equals(b, "/");

asm {
    push "a"
    push "_root"
    push 13 // _name
    getproperty
    setvariable
};
#if OUTPUT_VERSION > 5
check_equals(a, "changed");
#else
if ( a == undefined )
{
    pass("_root._name (trough getProperty(13)) returns undefined ["+__FILE__+":"+__LINE__+"]");
}
else
{
    // this check fails with Adobe Flash Player 9
    fail("_root._name (trough getProperty(13)) returns "+a+" (expected undefined) ["+__FILE__+":"+__LINE__+"]");
    note("Some version of Adobe Flash Player 9 are reported to have this bug");
}
#endif

asm {
    push "b"
    push "_root"
    push 11 // _target
    getproperty
    setvariable
};
check_equals(b, "/");

#endif // MING_SUPPORT_ASM_GETPROPERTY

//------------------------------------------------
// Test createTextField
//------------------------------------------------

t = createTextField("textfieldTest", 3, 0, 100, 100, 100);
#if OUTPUT_VERSION < 8
check_equals(typeof(t), 'undefined');
#else
check_equals(typeof(t), 'object');
check_equals(t, _root.textfieldTest);
#endif // OUTPUT_VERSION >= 8

#if OUTPUT_VERSION > 5
check_equals(typeof(textfieldTest), 'object');
check(textfieldTest instanceof TextField);
#else
xcheck_equals(typeof(textfieldTest), 'movieclip');
xcheck(textfieldTest instanceof MovieClip);
#endif

//----------------------------------------------
// Test getDepth
//----------------------------------------------

// A getDepth call against a script-created clip
// is already tested in the createEmptyMovieClip test
// section. Here we try to test it against a statically
// defined movie. We hope that the 'dejagnu' clip
// is statically defined, if it's not we'll raise a 
// warning about it.

// getDepth was not available as of SWF5
#if OUTPUT_VERSION > 5

// _level0 is at depth 0 !
// _level1 is at depth 1 ! (and so on)..
check_equals(_root.getDepth(), -16384);

static_clip_name = "__shared_assets";
static_clip = eval(static_clip_name);
if ( typeof(static_clip) == 'movieclip' )
{
    check_equals(static_clip.getDepth(), -16383);
}
else
{
    note("There is not '"+static_clip_name+"' clip statically-defined, so we could not test getDepth() against it");

}

#endif // OUTPUT_VERSION > 5


//----------------------------------------------
// Test _width, _height and getBounds
//----------------------------------------------

#if OUTPUT_VERSION >= 6

createEmptyMovieClip("container", 5);
check(!container.hasOwnProperty("$version"));
check_equals(typeof(container['$version']), 'undefined');
container.createEmptyMovieClip("draw", 5);
draw = container.draw;

check_equals(draw._width, 0);
check_equals(draw._height, 0);
b = draw.getBounds();
check_equals(typeof(b), 'object');
check_equals(typeof(b.xMin), 'number');
check_equals(typeof(b.xMax), 'number');
check_equals(typeof(b.yMin), 'number');
check_equals(typeof(b.yMax), 'number');
// Returned number is (2^28/2)-1 twips : any ringing bell ?
//check_equals(b.xMin, 6710886.35);
//check_equals(b.xMax, 6710886.35);
//check_equals(b.yMin, 6710886.35);
//check_equals(b.yMax, 6710886.35);
check(b.xMin-6710886.35 < 0.001);
check(b.xMax-6710886.35 < 0.001);
check(b.yMin-6710886.35 < 0.001);
check(b.yMax-6710886.35 < 0.001);

with (draw)
{
    lineStyle(0, 0x000000);
    moveTo(10, 10);
    lineTo(10, 30);
    lineTo(20, 30);
    lineTo(20, 10);
    lineTo(10, 10);
}
check_equals(draw._width, 10);
check_equals(draw._height, 20);
b = draw.getBounds();
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
b = draw.getBounds(container);
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);

draw._x += 20;
b = draw.getBounds();
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
b = draw.getBounds(container);
check_equals(b.xMin, 30);
check_equals(b.xMax, 40);
container._x -= 20;
b = draw.getBounds();
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
b = draw.getBounds(container);
check_equals(b.xMin, 30);
check_equals(b.xMax, 40);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
check_equals(draw._width, 10); 
check_equals(draw._height, 20);
b = draw.getBounds(_root);
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
draw._x -= 20;
container._x += 20;

draw._rotation = 90;
check_equals(draw._width, 20); 
check_equals(draw._height, 10); 
b = draw.getBounds(); // these are local, untransformed
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
b = draw.getBounds(container); // these are transformed by container draw matrix
check_equals(b.xMin, -30);
check_equals(b.xMax, -10);
check_equals(b.yMin, 10);
check_equals(b.yMax, 20);

draw._visible = false;
check_equals(draw._width, 20);
check_equals(draw._height, 10);
b = draw.getBounds(); // these are local, untransformed
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
b = draw.getBounds(container); // these are transformed by container draw matrix
check_equals(b.xMin, -30);
check_equals(b.xMax, -10);
check_equals(b.yMin, 10);
check_equals(b.yMax, 20);

draw._xscale = 200;
check_equals(draw._width, 20);
check_equals(draw._height, 20);

draw._rotation = 0;
check_equals(draw._width, 20);
check_equals(draw._height, 20);

draw._visible = true;
draw._xscale = 100;
check_equals(draw._width, 10);
check_equals(draw._height, 20);

draw._yscale = 50;
check_equals(draw._width, 10);
check_equals(draw._height, 10);
check_equals(container._width, 10);
check_equals(container._height, 10);

container._xscale = 800;
check_equals(draw._width, 10);
check_equals(draw._height, 10);
check_equals(container._width, 80);
check_equals(container._height, 10);
b = draw.getBounds(); // these are local, untransformed
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
b = draw.getBounds(container); // these are transformed by container draw matrix
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 5);
check_equals(b.yMax, 15);

draw._yscale = -50;
check_equals(draw._yscale, -50);
check_equals(draw._height, 10);

draw._xscale = -50;
check_equals(draw._xscale, -50);
check_equals(draw._width, 5);

draw._width = 10;
check_equals(draw._xscale, 100); // reset to positive on setting _width

draw._height = 10;
check_equals(draw._yscale, 50); // reset to positive on setting _height

container._xscale = 100;
container._yscale = 100;
draw._yscale = 100;
draw._xscale = 100;
b = draw.getBounds(container); // these are transformed by container draw matrix
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
container.createEmptyMovieClip("draw2", 6);
draw = container.draw2;
with (draw)
{
    lineStyle(0, 0x000000);
    moveTo(60, 20);
    lineTo(60, 40);
    lineTo(80, 40);
    lineTo(80, 20);
    lineTo(60, 20);
}

b = container.draw.getBounds(container); 
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
b = container.draw.getBounds(); 
check_equals(b.xMin, 10);
check_equals(b.xMax, 20);
check_equals(b.yMin, 10);
check_equals(b.yMax, 30);
b = container.draw2.getBounds(container); 
check_equals(b.xMin, 60);
check_equals(b.xMax, 80);
check_equals(b.yMin, 20);
check_equals(b.yMax, 40);
b = container.draw2.getBounds(container.draw);
check_equals(b.xMin, 60);
check_equals(b.xMax, 80);
check_equals(b.yMin, 20);
check_equals(b.yMax, 40);
b = container.draw2.getBounds(invalid);
check_equals(typeof(b), 'undefined');
b = container.draw2.getBounds(__shared_assets);
check_equals(b.xMin, 60);
check_equals(b.xMax, 80);
check_equals(b.yMin, 20);
check_equals(b.yMax, 40);

b = container.draw2.getBounds(); 
check_equals(b.xMin, 60);
check_equals(b.xMax, 80);
check_equals(b.yMin, 20);
check_equals(b.yMax, 40);
b = container.getBounds(); 
check_equals(b.xMin, 10);
check_equals(b.xMax, 80);
check_equals(b.yMin, 10);
check_equals(b.yMax, 40);

container.draw2._x += 20;
b = container.getBounds(); 
check_equals(b.xMin, 10);
check_equals(b.xMax, 100);
check_equals(b.yMin, 10);
check_equals(b.yMax, 40);
check_equals(container._width, 90);
check_equals(container._height, 30);

container.draw2._xscale = 200;
b = container.getBounds(); 
check_equals(b.xMin, 10);
check_equals(b.xMax, 180);
check_equals(b.yMin, 10);
check_equals(b.yMax, 40);

// Check effects of unload on bounds
// TODO: test this with bounds defined by SWF tags !
container.onUnload = function() {};
container_ref = container;
container.removeMovieClip();
check_equals(typeof(container), 'movieclip');
check_equals(typeof(container_ref.getBounds), 'function');
b = container.getBounds(); 
// check_equals(b.xMin, 6710886.35);
// check_equals(b.xMax, 6710886.35);
// check_equals(b.yMin, 6710886.35);
// check_equals(b.yMax, 6710886.35);
check(b.xMin - 6710886.35 < 0.0001);
check(b.xMax - 6710886.35 < 0.0001);
check(b.yMin - 6710886.35 < 0.0001);
check(b.yMax - 6710886.35 < 0.0001);

b = container_ref.getBounds(); 
// check_equals(b.xMin, 6710886.35);
// check_equals(b.xMax, 6710886.35);
// check_equals(b.yMin, 6710886.35);
// check_equals(b.yMax, 6710886.35);
check(b.xMin - 6710886.35 < 0.0001);
check(b.xMax - 6710886.35 < 0.0001);
check(b.yMin - 6710886.35 < 0.0001);
check(b.yMax - 6710886.35 < 0.0001);

container.createEmptyMovieClip("draw3",13);
draw = container.draw3;
with (draw)
{
    lineStyle(10, 0x000000);
    moveTo(10, 10);
    lineTo(10, 30);
    lineTo(20, 30);
    lineTo(20, 10);
    lineTo(10, 10);
}
#if OUTPUT_VERSION < 8
 check_equals(draw._width, 30);
 check_equals(draw._height, 40);
#else
 // SWF8 results are more correct (half-thickness added on each side)
 check_equals(draw._width, 20);
 check_equals(draw._height, 30);
#endif

// TODO: check bounds of non-scaled strokes 
//       relative to a container which is scaled

#endif // OUTPUT_VERSION >= 6

//----------------------------------------------
// Test _alpha
//----------------------------------------------

_alpha = "string";
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 100);

_alpha = 25;
check_equals(_alpha, 25);

o = {}; o.valueOf = function() { return 50; };
_alpha = o;
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 50);

_alpha = undefined;
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 50);

_alpha = null;
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 50);

_alpha = NaN;
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 50);

_alpha = Infinite;
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 50);

_alpha = 0/0;
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 50);

_alpha = -50;
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, -50);

_alpha = -Infinite;
check_equals(typeof(_alpha), 'number');
#if OUTPUT_VERSION < 7
 check_equals(_alpha, 0);
#else
 check_equals(_alpha, -50);
#endif

_alpha = 100;
_alpha = -(0/0);
check_equals(typeof(_alpha), 'number');
check_equals(_alpha, 100);


_alpha = 100;

//----------------------------------------------
// Test transform
//----------------------------------------------

#if OUTPUT_VERSION < 8
check_equals(typeof(_root.transform), 'undefined'); 
#else

// TODO: test these !!
xcheck_equals(typeof(_root.transform), 'object'); 

oldTransform = _root.transform;
check(oldTransform === oldTransform); 
check(oldTransform != _root.transform); // everytime transform is accessed, it's a new object!

Matrix = flash.geom.Matrix;
xcheck(_root.transform instanceOf Object);
check(!_root.transform instanceOf Matrix);
props = []; for (var i in _root.transform) props.push(i); props.sort();
xcheck_equals(props.toString(), "colorTransform,concatenatedColorTransform,concatenatedMatrix,matrix,pixelBounds");

xcheck_equals(typeof(_root.transform.colorTransform), 'object');
// TODO: test colorTransform

xcheck_equals(typeof(_root.transform.concatenatedColorTransform), 'object');
// TODO: test concatenatedColorTransform

xcheck_equals(typeof(_root.transform.concatenatedMatrix), 'object');
xcheck(_root.transform.concatenatedMatrix instanceOf Matrix);

xcheck_equals(typeof(_root.transform.matrix), 'object');
xcheck(_root.transform.matrix instanceOf Matrix);

note('x:'+_root._x+' y:'+_root._y+' rot:'+_root._rotation+' xs:'+_root._xscale+' yx:'+_root._yscale);

xcheck_equals(_root.transform.matrix.a, 1);
xcheck_equals(_root.transform.matrix.b, 0);
xcheck_equals(_root.transform.matrix.c, 0);
xcheck_equals(_root.transform.matrix.d, 1);
xcheck_equals(_root.transform.matrix.tx, 0);
xcheck_equals(_root.transform.matrix.ty, 0);
// TODO: test concatenatedMatrix

_root._x = 30;
_root._y = 20;
//_root._xscale = -300; // NOTE: gnash breaks the _root's matrix if we set _xscale here ! you can tell by failing localToGLobal/globalToLocal tests
_root._yscale = -200;
_root._rotation = -90;

xcheck_equals(_root.transform.matrix.a, 0);
xcheck_equals(_root.transform.matrix.b, -1); // would be 3 if we did set _xscale=-300 above
xcheck_equals(_root.transform.matrix.c, -2);
xcheck_equals(_root.transform.matrix.d, 0);
xcheck_equals(_root.transform.matrix.tx, 30);
xcheck_equals(_root.transform.matrix.ty, 20);
// TODO: test concatenatedMatrix

_root.transform.matrix.ty = 300;
check_equals(_root._y, 20); // changing the AS matrix doesn't change the actual matrix

_root._x = _root._y = _root._rotation = 0;
_root._xscale = _root._yscale = 100;

#endif

//----------------------------------------------
// Test localToGlobal and globalToLocal
//----------------------------------------------

static_clip_name = "__shared_assets";
static_clip = __shared_assets;
if ( typeof(static_clip) == 'movieclip' )
{
    o = {x:0, y:0};
    static_clip.localToGlobal(o);
    check_equals(o.x, 0);
    check_equals(o.y, 0);
    static_clip.globalToLocal(o);
    check_equals(o.x, 0);
    check_equals(o.y, 0);

    static_clip._x += 50;
    static_clip._y -= 30;

    o = {x:0, y:0};
    static_clip.localToGlobal(o);
    check_equals(o.x, 50);
    check_equals(o.y, -30);
    static_clip.globalToLocal(o);
    check_equals(o.x, 0);
    check_equals(o.y, 0);

    o = {x:1, y:1};
    static_clip.localToGlobal(o);
    check_equals(o.x, 51);
    check_equals(o.y, -29);
    static_clip.globalToLocal(o);
    check_equals(o.x, 1);
    check_equals(o.y, 1);

    static_clip._xscale *= 2;
    static_clip._yscale *= 0.5;

    o = {x:0, y:0};
    static_clip.localToGlobal(o);
    check_equals(o.x, 50);
    check_equals(o.y, -30);
    static_clip.globalToLocal(o);
    check_equals(o.x, 0);
    check_equals(o.y, 0);

    o = {x:2, y:2};
    static_clip.localToGlobal(o);
    check_equals(o.x, 54);
    check_equals(o.y, -29);
    static_clip.globalToLocal(o);
    check_equals(o.x, 2);
    check_equals(o.y, 2);

    static_clip._rotation = 90;

    o = {x:0, y:0};
    static_clip.localToGlobal(o);
    check_equals(o.x, 50);
    check_equals(o.y, -30);
    static_clip.globalToLocal(o);
    check_equals(o.x, 0);
    check_equals(o.y, 0);

    o = {x:2, y:2};
    static_clip.localToGlobal(o);
    check_equals(o.x, 49);
    check_equals(o.y, -26);
    static_clip.globalToLocal(o);
    check_equals(o.x, 2);
    check_equals(o.y, 2);

    // omit the 'y' member (invalid call)
    o = {x:2};
    static_clip.localToGlobal(o);
    check_equals(o.x, 2);
    check_equals(typeof(o.y), 'undefined');
    static_clip.globalToLocal(o);
    check_equals(o.x, 2);
    check_equals(typeof(o.y), 'undefined');

    // Upper case
    o = {X:2, Y:2};
    static_clip.localToGlobal(o);
#if OUTPUT_VERSION < 7
    check_equals(o.X, 49);
    check_equals(o.Y, -26);
#else // OUTPUT_VERSION >= 7
    check_equals(o.X, 2);
    check_equals(o.Y, 2);
    check_equals(typeof(o.x), 'undefined');
    check_equals(typeof(o.y), 'undefined');
#endif
    static_clip.globalToLocal(o);
    check_equals(o.X, 2);
    check_equals(o.Y, 2);

    static_clip._rotation = 0;
    static_clip._x -= 50;
    static_clip._y += 30;
    static_clip._xscale *= 0.5;
    static_clip._yscale *= 2;

    // If it's not one twip, it's nothing
    static_clip._y += 0.04;
    check_equals(static_clip._y, 0);

    static_clip._y += 0.04;
    check_equals(static_clip._y, 0);
    
    static_clip._y += 0.04;
    check_equals(static_clip._y, 0);
    
    static_clip._y = 0.09;
    check(static_clip._y > 0.04999 && static_clip._y < 0.050001);

    // Gnash can't do this because _x and _y are floats at the
    // moment, but it probably should be able to.
    check_equals(static_clip._y, 0.05);

    // If it's not one twip, it's nothing
    static_clip._x += 0.04;
    check_equals(static_clip._x, 0);

    static_clip._x += 0.04;
    check_equals(static_clip._x, 0);
    
    static_clip._x += 0.04;
    check_equals(static_clip._x, 0);
    
    static_clip._x = 20.09;
    check(static_clip._x > 20.049999 && static_clip._x < 20.050001);
    check_equals(static_clip._x, 20.05);
    static_clip._x = 0;

    // TODO: try with x/y being getter-setter of the localToGlobal and globalToLocal parameter
    
}
else
{
    note("There is not '"+static_clip_name+"' clip statically-defined, so we could not test localToGlobal() and globalToLocal() against it");

}

//---------------------------------------------------------------------
// TODO: Test getInstanceAtDepth
//       (tested somehow above, but I mean a well-focused test here)
//---------------------------------------------------------------------

#if OUTPUT_VERSION >= 7

check_equals(typeof(getInstanceAtDepth(-6)), 'undefined');
check_equals(typeof(getInstanceAtDepth()), 'undefined');
createEmptyMovieClip("tt2", -6);
check_equals(typeof(getInstanceAtDepth(-6)), 'movieclip');
o = new Object; o.valueOf = function() { return -6; };
check_equals(typeof(getInstanceAtDepth(o)), 'movieclip');
check_equals(getInstanceAtDepth(o), tt2);
check_equals(getInstanceAtDepth(-6.7), tt2);
check_equals(getInstanceAtDepth(-6.2), tt2);

// what if we point to a non-sprite instance ? is it still considered a "movieclip" ?

#endif // OUTPUT_VERSION >= 7

//---------------------------------------------------------------------
// Test the MovieClip.prototype.meth function
//---------------------------------------------------------------------

ret = _root.meth();
check_equals(typeof(ret), 'number');
check_equals(ret, 0);

ret = _root.meth(1);
check_equals(typeof(ret), 'number');
check_equals(ret, 0);

Number.prototype.toLowerCase = function() { retCaller=arguments.caller; return "post"; };
ret = _root.meth(1);
#if OUTPUT_VERSION < 6
 check_equals(retCaller, _root.meth); // in gnash works because functions resolve equal to undefined
#else
 xcheck_equals(retCaller, _root.meth); // check that arguments.caller is also set for builtin functions
#endif
check_equals(typeof(ret), 'number');
check_equals(ret, 2);

ret = _root.meth('post');
check_equals(typeof(ret), 'number');
check_equals(ret, 2);

ret = _root.meth('POSt');
check_equals(typeof(ret), 'number');
check_equals(ret, 2);

ret = _root.meth('pOStIcipate');
check_equals(typeof(ret), 'number');
check_equals(ret, 0);

ret = _root.meth('G');
check_equals(typeof(ret), 'number');
check_equals(ret, 0);

ret = _root.meth('geT');
check_equals(typeof(ret), 'number');
check_equals(ret, 1);

ret = _root.meth('geTty');
check_equals(typeof(ret), 'number');
check_equals(ret, 0);

o = {}; o.toString = function() { return "post"; };
ret = _root.meth(o);
check_equals(typeof(ret), 'number');
check_equals(ret, 0);

o.toLowerCase = function() { return "get"; };
ret = _root.meth(o);
check_equals(typeof(ret), 'number');
check_equals(ret, 1);

//---------------------------------------------------------------------
// Test native properties (getter-setter?)
//---------------------------------------------------------------------

#if OUTPUT_VERSION > 5 // addProperty needs SWF6

//createEmptyMovieClip('mc', 10);
mc = _root;
mc._x = 20;
check_equals(mc._x, 20);
get = function() { /*note('get called');*/ getCalls++; return 17; };
set = function() { /*note('set called');*/ setCalls++; };
ret = mc.addProperty('_x', get, set);
check_equals(ret, true);
getCalls=0; setCalls=0;
inspect = mc._x;
check_equals(inspect, 17); // getMember invokes the getter
check_equals(getCalls, 1);
check_equals(setCalls, 0);
mc.x = 10; // sets the property, but doesn't call the setter !
check_equals(getCalls, 1); // assignment did call the getter 
check_equals(setCalls, 0);
#ifdef MING_SUPPORTS_ASM
asm {
	push 'propinspect'
	push 'mc'
	push 0
	getproperty
	setvariable
};
// setMember did set the prop, didn't call the setter
xcheck_equals(propinspect, 20);
#endif //MING_SUPPORTS_ASM

createEmptyMovieClip('mc', 10);
mc._x = 1;
ret = mc.addProperty('_x', get, set);
check_equals(ret, true);
getCalls=0; setCalls=0;
inspect = mc._x;
check_equals(inspect, 17); // getMember invokes the getter
check_equals(getCalls, 1);
check_equals(setCalls, 0);
mc.x = 10; // does NOT set the property, but doesn't call the setter !
check_equals(getCalls, 1); // assignment did call the getter 
check_equals(setCalls, 0);
#ifdef MING_SUPPORTS_ASM
asm {
	push 'propinspect'
	push 'mc'
	push 0
	getproperty
	setvariable
};
// setMember did NOT set the prop, didn't call the setter
check_equals(propinspect, 0);
#endif //MING_SUPPORTS_ASM


#endif // OUTPUT_VERSION > 5


//---------------------------------------------------------------------
// Test the MovieClip.loadVariables function
//---------------------------------------------------------------------

dataLoaded = 0;

onData = function()
{
	note("onData called, dataLoaded: "+dataLoaded);
	check_equals(arguments.length, 0);
#if OUTPUT_VERSION > 5
	check_equals(_root.var1, 'val1');
#else
	// leading BOM confuses player 5 (but not gnash)
	xcheck_equals(typeof(_root.var1), 'undefined');
#endif
	check_equals(_root.var2, 'val2');
	check_equals(_root.var3, 'val3\n');
	delete _root.var1; // = 'val1custom';
	delete _root.var2; // = 'val2custom';

	if ( dataLoaded++ )
	{
		//note("Clearing data load interval "+dataLoadInterval);
		clearInterval(dataLoadInterval);
		endOfTest();
	}
	else
	{
		// This should use GetURL
		loadVariables(MEDIA(vars.txt), "_root");
	}
};

stop();

ret = _root.loadVariables(MEDIA(vars.txt), "GET");
check_equals(dataLoaded, 0);
check_equals(typeof(ret), 'undefined');

#if OUTPUT_VERSION <= 5
	// It seems for SWF5 onData isn't invoked,
	// neighter does onEnterFrame work..
	onDataCheck = function()
	{
		//note("1000 interval called");
		if ( _root.var3 != undefined ) onData();
	};
	//dataLoadInterval = setInterval(onData, 1000);
	dataLoadInterval = setInterval(onDataCheck, 1000);
#endif

/// Depth tests for createEmptyMovieClip, which was introduced
/// in SWF6.


#if OUTPUT_VERSION > 5
createEmptyMovieClip("d1", -200000000);
check_equals(d1.getDepth(), -200000000);

createEmptyMovieClip("d2", -0xffffffff);
check_equals(d2.getDepth(), 1);

createEmptyMovieClip("d3", 0xffffffff);
check_equals(d3.getDepth(), -1);

createEmptyMovieClip("d4", 0x80000000);
check_equals(d4.getDepth(), -2147483648);

createEmptyMovieClip("d5", 0x79999999);
check_equals(d5.getDepth(), 2040109465);

#endif

//_root.loadVariables(MEDIA(vars.txt), "GET");

// Can't rely on this to call onData!

//endOfTest();
