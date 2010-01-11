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

package p1 {
    public class A { public function id() { return "p1.A"; } }
    public class B { public static function id() { return "p1.B"; } }
}

package p2 {
    public class A { public function id() { return "p2.A"; } }
    public class B { public static function id() { return "p2.B"; } }
    public class C {}
}

package main {

import flash.display.MovieClip;

public class Main extends MovieClip {

    DEJAGNU_OBJ;

    import p1.*;
    import p2.*;

    // This is not that hard at all, as the compiler should generate
    // the code specifying which namespace to look in.
    public function Main() {

        var a = new A();

        // Check that functions work at all...
        check_equals(typeof(a.id()), "string");

        // Check non-static function lookup
        check_equals(a.id(), "p1.A");
        
        // Check static function lookup.
        check_equals(B.id(), "p1.B");

        done();
    }
}
         
}
