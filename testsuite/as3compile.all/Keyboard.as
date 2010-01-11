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

package main {

    import flash.display.MovieClip;
    import flash.ui.Keyboard;

    public class Main extends MovieClip {

        DEJAGNU_OBJ;

        public function Main() {

            // Keyboard cannot be instantiated; it throws ArgumentError #2102
            xcheck_equals(Keyboard.prototype, "[object Object]");
            xcheck_equals(Keyboard.constructor, "[class Class]");
            xcheck(Keyboard.hasOwnProperty("capsLock"));
            xcheck(Keyboard.hasOwnProperty("numLock"));
            check(Keyboard.hasOwnProperty("isAccessible"));

            // TODO: lots more
            check(Keyboard.hasOwnProperty("BACKSPACE"));
            xcheck(Keyboard.hasOwnProperty("CAPS_LOCK"));

            // These are in AIR, but not flash.
            check(!Keyboard.hasOwnProperty("A"));
            check(!Keyboard.hasOwnProperty("B"));
            check(!Keyboard.hasOwnProperty("C"));
            check(!Keyboard.hasOwnProperty("D"));
            
            totals(11);
            done();
        }
    }
}
         
