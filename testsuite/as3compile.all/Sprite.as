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
    import flash.display.Sprite;

    public class Main extends MovieClip {

        DEJAGNU_OBJ;

        public function Main() {

            check_equals(Sprite.prototype, "[object Object]");
            xcheck_equals(Sprite.constructor, "[class Class]");
            
            // The prototype seems really to be just an object. Just
            // test the Sprite properties until there's a reason
            // to check others.
            
            var m = new Sprite();
            xcheck_equals(m.constructor, "[class Sprite]");
            
            // MovieClip properties
            check(!m.hasOwnProperty("nextFrame"));
            check(!m.hasOwnProperty("prevFrame"));
            check(!m.hasOwnProperty("gotoAndStop"));
            check(!m.hasOwnProperty("nextScene"));
            check(!m.hasOwnProperty("prevScene"));
            check(!m.hasOwnProperty("play"));
            check(!m.hasOwnProperty("stop"));
            check(!m.hasOwnProperty("addFrameScript"));
            check(!m.hasOwnProperty("framesLoaded"));
            check(!m.hasOwnProperty("totalFrames"));
            check(!m.hasOwnProperty("currentFrame"));

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

            // Check type of Sprite properties. Inherited properties should be
            // dealt with elsewhere.
            xcheck_equals(typeof(m.graphics), "object");
            xcheck_equals(typeof(m.buttonMode), "boolean");
            xcheck_equals(typeof(m.dropTarget), "object");
            xcheck_equals(typeof(m.soundTransform), "object");
            xcheck_equals(typeof(m.useHandCursor), "boolean");
            check_equals(typeof(m.stopDrag), "function");
            xcheck_equals(typeof(m.startDrag), "function");

            totals(41);
            done();
        }
    }
}
