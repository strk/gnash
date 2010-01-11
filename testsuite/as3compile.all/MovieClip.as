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

#include "check.as"

package hello {

    import flash.display.MovieClip;

    public class Main extends MovieClip {

        DEJAGNU_OBJ;

        public function Main() {

            xcheck_equals(MovieClip, "[class MovieClip]");
            xcheck_equals(MovieClip.prototype, "[object Object]");
            xcheck_equals(MovieClip.constructor, "[class Class]");
            xcheck(!MovieClip.hasOwnProperty("constructor"));

            // Check that this object is a MovieClip and has MovieClip
            // functions (no need to check them all).
            xcheck(this instanceof MovieClip);
            xcheck_equals(typeof(this.nextFrame), "function");
            xcheck_equals(typeof(this.play), "function");
            
            xcheck(MovieClip.prototype.hasOwnProperty("constructor"));
            
            check(!MovieClip.hasOwnProperty("nextFrame"));
            check(!MovieClip.hasOwnProperty("prevFrame"));
            check(!MovieClip.hasOwnProperty("gotoAndStop"));
            check(!MovieClip.hasOwnProperty("nextScene"));
            check(!MovieClip.hasOwnProperty("prevScene"));
            check(!MovieClip.hasOwnProperty("play"));
            check(!MovieClip.hasOwnProperty("stop"));
            check(!MovieClip.hasOwnProperty("addFrameScript"));
            check(!MovieClip.hasOwnProperty("framesLoaded"));
            check(!MovieClip.hasOwnProperty("totalFrames"));
            check(!MovieClip.hasOwnProperty("currentFrame"));

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
            xcheck_equals(m.constructor, "[class MovieClip]");
            xcheck(!m.hasOwnProperty("constructor"));
            
            // MovieClip properties
            xcheck(m.hasOwnProperty("nextFrame"));
            xcheck(m.hasOwnProperty("prevFrame"));
            xcheck(m.hasOwnProperty("gotoAndStop"));
            xcheck(m.hasOwnProperty("nextScene"));
            xcheck(m.hasOwnProperty("prevScene"));
            xcheck(m.hasOwnProperty("play"));
            xcheck(m.hasOwnProperty("stop"));
            xcheck(m.hasOwnProperty("addFrameScript"));
            xcheck(m.hasOwnProperty("framesLoaded"));
            xcheck(m.hasOwnProperty("totalFrames"));
            xcheck(m.hasOwnProperty("currentFrame"));

            // Sprite properties
            xcheck(m.hasOwnProperty("graphics"));
            xcheck(m.hasOwnProperty("buttonMode"));
            xcheck(m.hasOwnProperty("dropTarget"));
            xcheck(m.hasOwnProperty("soundTransform"));
            xcheck(m.hasOwnProperty("useHandCursor"));
            xcheck(m.hasOwnProperty("stopDrag"));
            xcheck(m.hasOwnProperty("startDrag"));

            // DisplayObjectContainer properties
            xcheck(m.hasOwnProperty("addChild"));
            xcheck(m.hasOwnProperty("removeChild"));
            
            // DisplayObject properties (?)
            xcheck(m.hasOwnProperty("transform"));
            xcheck(m.hasOwnProperty("scale9Grid"));
            xcheck(m.hasOwnProperty("localToGlobal"));
            xcheck(m.hasOwnProperty("globalToLocal"));
            xcheck(m.hasOwnProperty("getBounds"));
            xcheck(m.hasOwnProperty("scrollRect"));
            xcheck(m.hasOwnProperty("tabIndex"));
            xcheck(m.hasOwnProperty("opaqueBackground"));
            xcheck(m.hasOwnProperty("filters"));
            xcheck(m.hasOwnProperty("cacheAsBitmap"));
            xcheck(m.hasOwnProperty("getRect"));

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
            xcheck_equals(typeof(m.framesLoaded), "number");
            xcheck_equals(typeof(m.totalFrames), "number");
            xcheck_equals(typeof(m.currentFrame), "number");

            totals(110);

            done();
        }
    }
}
