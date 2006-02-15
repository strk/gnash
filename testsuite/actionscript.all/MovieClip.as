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

#include "check.as"

// Get a reference to a MovieClip
var mc = _root;
check(typeof(mc)=="movieclip");

// Check some references
check(this != undefined);
check(_parent == undefined);
check(_root != undefined);
check(_root == this);

// Check inheritance
check(mc.__proto__ == MovieClip.prototype);

// Check methods existance
check(mc.attachAudio != undefined);
check(mc.attachMovie != undefined);
check(mc.beginFill != undefined);
check(mc.beginGradientFill != undefined);
check(mc.clear != undefined);
check(mc.createEmptyMovieClip != undefined);
check(mc.createTextField != undefined);
check(mc.curveTo != undefined);

// not available ?
//check(mc.duplicateMovieClip == undefined);

check(mc.endFill != undefined);
check(mc.getBytesLoaded != undefined);
check(mc.getBytesTotal != undefined);
check(mc.getBounds != undefined);
check(mc.getDepth != undefined);

#if OUTPUT_VERSION >= 7
check(mc.getInstanceAtDepth != undefined);
check(mc.getNextHighestDepth != undefined);
check(mc.getSWFVersion != undefined);
check(mc.getTextSnapshot != undefined);
#endif

check(mc.getURL != undefined);
check(mc.globalToLocal != undefined);
check(mc.gotoAndPlay != undefined);
check(mc.gotoAndStop != undefined);
check(mc.hitTest != undefined);
check(mc.lineStyle != undefined);
check(mc.lineTo != undefined);

check(mc.loadMovie != undefined);

check(mc.loadVariables != undefined);
check(mc.localToGlobal != undefined);
check(mc.moveTo != undefined);
check(mc.nextFrame != undefined);
check(mc.play != undefined);
check(mc.prevFrame != undefined);
check(mc.removeMovieClip != undefined);
check(mc.setMask != undefined);
check(mc.startDrag != undefined);
check(mc.stop != undefined);
check(mc.stopDrag != undefined);
check(mc.swapDepths != undefined);
check(mc.unloadMovie != undefined);

// Check property existance
check(mc.enabled != undefined);
check(mc.focusEnabled != undefined);
check(mc.hitArea != undefined);
check(mc.menu != undefined);
check(mc.onData != undefined);
check(mc.onDragOut != undefined);
check(mc.onDragOver != undefined);
check(mc.onEnterFrame != undefined);
check(mc.onKeyDown != undefined);
check(mc.onKeyUp != undefined);
check(mc.onKillFocus != undefined);
check(mc.onLoad != undefined);
check(mc.onMouseDown != undefined);
check(mc.onMouseMove != undefined);
check(mc.onMouseUp != undefined);
check(mc.onPress != undefined);
check(mc.onRelease != undefined);
check(mc.onReleaseOutside != undefined);
check(mc.onRollOut != undefined);
check(mc.onRollOver != undefined);
check(mc.onSetFocus != undefined);
check(mc.onUnload != undefined);
check(mc.tabChildren != undefined);
check(mc.tabEnabled != undefined);
check(mc.tabIndex != undefined);
check(mc.trackAsMenu != undefined);
check(mc.useHandCursor != undefined);
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
check(mc._lockroot != undefined);
check(mc._name != undefined);
check(mc._parent != undefined);
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
check(mc2 != undefined);
