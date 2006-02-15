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

// Check some references
check(this != undefined);
check(_parent == undefined);
check(_root != undefined);
check(_root == this);

// Check inheritance
check(this.__proto__ == MovieClip.prototype);

#if 0
// Check methods existance
check(this.attachAudio != undefined);
check(this.attachMovie != undefined);
check(this.beginFill != undefined);
check(this.beginGradientFill != undefined);
check(this.clear != undefined);
check(this.createEmptyMovieClip != undefined);
check(this.createTextField != undefined);
check(this.curveTo != undefined);
check(this.duplicateMovieClip != undefined);
check(this.endFill != undefined);
check(this.getBytesLoaded != undefined);
check(this.getBytesTotal != undefined);
check(this.getBounds != undefined);
check(this.getDepth != undefined);
#endif

#if OUTPUT_VERSION >= 7
check(this.getInstanceAtDepth != undefined);
check(this.getNextHighestDepth != undefined);
check(this.getSWFVersion != undefined);
check(this.getTextSnapshot != undefined);
#endif

check(this.getURL != undefined);
check(this.globalToLocal != undefined);
check(this.gotoAndPlay != undefined);
check(this.gotoAndStop != undefined);
check(this.hitTest != undefined);
check(this.lineStyle != undefined);
check(this.lineTo != undefined);
check(this.loadMovie != undefined);
check(this.loadVariables != undefined);
check(this.localToGlobal != undefined);
check(this.moveTo != undefined);
check(this.nextFrame != undefined);
check(this.play != undefined);
check(this.prevFrame != undefined);
check(this.removeMovieClip != undefined);
check(this.setMask != undefined);
check(this.startDrag != undefined);
check(this.stop != undefined);
check(this.stopDrag != undefined);
check(this.swapDepths != undefined);
check(this.unloadMovie != undefined);

// Check property existance
check(this.enabled != undefined);
check(this.focusEnabled != undefined);
check(this.hitArea != undefined);
check(this.menu != undefined);
check(this.onData != undefined);
check(this.onDragOut != undefined);
check(this.onDragOver != undefined);
check(this.onEnterFrame != undefined);
check(this.onKeyDown != undefined);
check(this.onKeyUp != undefined);
check(this.onKillFocus != undefined);
check(this.onLoad != undefined);
check(this.onMouseDown != undefined);
check(this.onMouseMove != undefined);
check(this.onMouseUp != undefined);
check(this.onPress != undefined);
check(this.onRelease != undefined);
check(this.onReleaseOutside != undefined);
check(this.onRollOut != undefined);
check(this.onRollOver != undefined);
check(this.onSetFocus != undefined);
check(this.onUnload != undefined);
check(this.tabChildren != undefined);
check(this.tabEnabled != undefined);
check(this.tabIndex != undefined);
check(this.trackAsMenu != undefined);
check(this.useHandCursor != undefined);
check(this._alpha != undefined);
check(this._currentframe != undefined);
check(this._droptarget != undefined);
check(this._focusrect != undefined);
check(this._framesloaded != undefined);
check(this._height != undefined);
check(this._highquality != undefined);
check(this._y != undefined);
check(this._ymouse != undefined);
check(this._yscale != undefined);
check(this._lockroot != undefined);
check(this._name != undefined);
check(this._parent != undefined);
check(this._rotation != undefined);
check(this._soundbuftime != undefined);
check(this._target != undefined);
check(this._totalframes != undefined);
check(this._url != undefined);
check(this._visible != undefined);
check(this._width != undefined);
check(this._x != undefined);
check(this._xmouse != undefined);
check(this._xscale != undefined);
