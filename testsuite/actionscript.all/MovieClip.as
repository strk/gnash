// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//

// Test case for Function ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: MovieClip.as,v 1.7 2006/10/31 12:55:24 strk Exp $";

#include "check.as"

// Get a reference to a MovieClip
var mc = _root;
xcheck(typeof(mc)=="movieclip");

// Check some references
check(this != undefined);
check(_parent == undefined);
check(_root != undefined);
check(_root == this);

// Check inheritance
check(mc.__proto__ == MovieClip.prototype);

// Check methods existance
xcheck(mc.attachAudio != undefined);
xcheck(mc.attachMovie != undefined);
xcheck(mc.beginFill != undefined);
xcheck(mc.beginGradientFill != undefined);
xcheck(mc.clear != undefined);
check(mc.createEmptyMovieClip != undefined);
check(mc.createTextField != undefined);
xcheck(mc.curveTo != undefined);

// not available ?
//check(mc.duplicateMovieClip == undefined);

xcheck(mc.endFill != undefined);
check(mc.getBytesLoaded != undefined);
check(mc.getBytesTotal != undefined);
xcheck(mc.getBounds != undefined);
check(mc.getDepth != undefined);

if (OUTPUT_VERSION >= 7) {
    xcheck(mc.getInstanceAtDepth != undefined);
    xcheck(mc.getSWFVersion != undefined);
    xcheck(mc.getTextSnapshot != undefined);
    xcheck(mc.lineStyle != undefined);
    xcheck(mc.lineTo != undefined);
    xcheck(mc.loadVariables != undefined);
    xcheck(mc.localToGlobal != undefined);
    xcheck(mc.moveTo != undefined);
    xcheck(mc.setMask != undefined);
    xcheck(mc.startDrag != undefined);
    xcheck(mc.stopDrag != undefined);
    xcheck(mc.unloadMovie != undefined);

    xcheck(mc.enabled != undefined);
    xcheck(mc.focusEnabled != undefined);
    xcheck(mc.hitArea != undefined);
    xcheck(mc.menu != undefined);
} else {
   check_equals(mc.getNextHighestDepth(), undefined);
}

xcheck(mc.getURL != undefined);
xcheck(mc.globalToLocal != undefined);
check(mc.gotoAndPlay != undefined);
check(mc.gotoAndStop != undefined);
check(mc.hitTest != undefined);

check(mc.loadMovie != undefined);

check(mc.nextFrame != undefined);
check(mc.play != undefined);
check(mc.prevFrame != undefined);
check(mc.removeMovieClip != undefined);
check(mc.stop != undefined);
check(mc.swapDepths != undefined);

// Check property existance
xcheck(mc.onData != undefined);
xcheck(mc.onDragOut != undefined);
xcheck(mc.onDragOver != undefined);
xcheck(mc.onEnterFrame != undefined);
xcheck(mc.onKeyDown != undefined);
xcheck(mc.onKeyUp != undefined);
xcheck(mc.onKillFocus != undefined);
xcheck(mc.onLoad != undefined);
xcheck(mc.onMouseDown != undefined);
xcheck(mc.onMouseMove != undefined);
xcheck(mc.onMouseUp != undefined);
xcheck(mc.onPress != undefined);
xcheck(mc.onRelease != undefined);
xcheck(mc.onReleaseOutside != undefined);
xcheck(mc.onRollOut != undefined);
xcheck(mc.onRollOver != undefined);
xcheck(mc.onSetFocus != undefined);
xcheck(mc.onUnload != undefined);
xcheck(mc.tabChildren != undefined);
xcheck(mc.tabEnabled != undefined);
xcheck(mc.tabIndex != undefined);
xcheck(mc.trackAsMenu != undefined);
xcheck(mc.useHandCursor != undefined);
check(mc._alpha != undefined);
check(mc._currentframe != undefined);
check(mc._droptarget != undefined);
check(mc._focusrect != undefined);
check(mc._framesloaded != undefined);
check(mc._height != undefined);
check(mc._highquality != undefined);
check(mc._y != undefined);
check(mc._ymouse != undefined);
check(mc._yscale != undefined);
xcheck(mc._lockroot != undefined);
check(mc._name != undefined);
xcheck(mc._parent != undefined);
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

// Test movieclip creation
var mc2 = createEmptyMovieClip("mc2_mc", 50, 0, 0, 0);
xcheck(mc2 != undefined);
