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

#include "check.as"

package hello {

    import flash.display.MovieClip;

    public class Main extends MovieClip {

        DEJAGNU_OBJ;

        public function Main() {

            check_equals(MovieClip.prototype, "[object Object]");
            check_equals(MovieClip.constructor, "[class Class]");
            
            // The prototype seems really to be just an object. Just
            // test the MovieClip properties until there's a reason
            // to check others.
            check(!MovieClip.prototype.hasOwnProperty("nextFrame"));
            check(!MovieClip.prototype.hasOwnProperty("prevFrame"));
            check(!MovieClip.prototype.hasOwnProperty("gotoAndStop"));
            check(!MovieClip.prototype.hasOwnProperty("nextScene"));
            check(!MovieClip.prototype.hasOwnProperty("prevScene"));
            check(!MovieClip.prototype.hasOwnProperty("play"));
            check(!MovieClip.prototype.hasOwnProperty("stop"));
            check(!MovieClip.prototype.hasOwnProperty("addFrameScript"));
            check(!MovieClip.prototype.hasOwnProperty("framesLoaded"));
            check(!MovieClip.prototype.hasOwnProperty("totalFrames"));
            check(!MovieClip.prototype.hasOwnProperty("currentFrame"));
            
            var m = new MovieClip();
            check_equals(m.constructor, "[class MovieClip]");
            
            // MovieClip properties
            check(m.hasOwnProperty("nextFrame"));
            check(m.hasOwnProperty("prevFrame"));
            check(m.hasOwnProperty("gotoAndStop"));
            check(m.hasOwnProperty("nextScene"));
            check(m.hasOwnProperty("prevScene"));
            check(m.hasOwnProperty("play"));
            check(m.hasOwnProperty("stop"));
            check(m.hasOwnProperty("addFrameScript"));
            check(m.hasOwnProperty("framesLoaded"));
            check(m.hasOwnProperty("totalFrames"));
            check(m.hasOwnProperty("currentFrame"));

            // Sprite properties
            check(m.hasOwnProperty("graphics"));
            check(m.hasOwnProperty("buttonMode"));
            check(m.hasOwnProperty("dropTarget"));
            check(m.hasOwnProperty("soundTransform"));
            check(m.hasOwnProperty("useHandCursor"));
            check(m.hasOwnProperty("stopDrag"));
            check(m.hasOwnProperty("startDrag"));

            // DisplayObjectContainer properties
            check(m.hasOwnProperty("addChild"));
            check(m.hasOwnProperty("removeChild"));
            
            // DisplayObject properties (?)
            check(m.hasOwnProperty("transform"));
            check(m.hasOwnProperty("scale9Grid"));
            check(m.hasOwnProperty("localToGlobal"));
            check(m.hasOwnProperty("globalToLocal"));
            check(m.hasOwnProperty("getBounds"));
            check(m.hasOwnProperty("scrollRect"));
            check(m.hasOwnProperty("tabIndex"));
            check(m.hasOwnProperty("opaqueBackground"));
            check(m.hasOwnProperty("filters"));
            check(m.hasOwnProperty("cacheAsBitmap"));
            check(m.hasOwnProperty("getRect"));

            // AS2-only properties
            check(!m.hasOwnProperty("loadVariables"));
            check(!m.hasOwnProperty("getSWFVersion"));
            check(!m.hasOwnProperty("url"));
            check(!m.hasOwnProperty("meth"));
            check(!m.hasOwnProperty("forceSmoothing"));
            check(!m.hasOwnProperty("lineGradientStyle"));
            check(!m.hasOwnProperty("getTextSnapshot"));
            check(!m.hasOwnProperty("getNextHighestDepth"));
            check(!m.hasOwnProperty("getInstanceAtDepth"));
            check(!m.hasOwnProperty("swapDepths"));
            check(!m.hasOwnProperty("hitTest"));
            check(!m.hasOwnProperty("setMask"));
            check(!m.hasOwnProperty("setMask"));
            check(!m.hasOwnProperty("getDepth"));
            check(!m.hasOwnProperty("getBytesTotal"));
            check(!m.hasOwnProperty("getBytesLoaded"));
            check(!m.hasOwnProperty("duplicateMovieClip"));
            check(!m.hasOwnProperty("beginGradientFill"));
            check(!m.hasOwnProperty("moveTo"));
            check(!m.hasOwnProperty("lineTo"));
            check(!m.hasOwnProperty("lineStyle"));
            check(!m.hasOwnProperty("clear"));
            check(!m.hasOwnProperty("endFill"));
            check(!m.hasOwnProperty("beginFill"));
            check(!m.hasOwnProperty("curveTo"));
            check(!m.hasOwnProperty("createTextField"));
            check(!m.hasOwnProperty("removeMovieClip"));
            check(!m.hasOwnProperty("createEmptyMovieClip"));
            check(!m.hasOwnProperty("beginBitmapFill"));
            check(!m.hasOwnProperty("attachBitmap"));
            check(!m.hasOwnProperty("getURL"));
            check(!m.hasOwnProperty("loadMovie"));
            check(!m.hasOwnProperty("unloadMovie"));
            check(!m.hasOwnProperty("attachMovie"));
            check(!m.hasOwnProperty("attachVideo"));
            check(!m.hasOwnProperty("attachAudio"));

            // Check type of MovieClip properties. Inherited properties should
	    // be dealt with elsewhere.
            check_equals(typeof(m.nextFrame), "function");
            check_equals(typeof(m.prevFrame), "function");
            check_equals(typeof(m.gotoAndStop), "function");
            check_equals(typeof(m.nextScene), "function");
            check_equals(typeof(m.prevScene), "function");
            check_equals(typeof(m.play), "function");
            check_equals(typeof(m.stop), "function");
            check_equals(typeof(m.addFrameScript), "function");
            check_equals(typeof(m.framesLoaded), "number");
            check_equals(typeof(m.totalFrames), "number");
            check_equals(typeof(m.currentFrame), "number");

            done();
        }
    }
}
