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

    public class S {
        public static var s : String;
    }

    public class Base {
        public function Base() {
            trace("Base ctor");
            S.s += "Base ";
        }
    }

    public class Derived extends Base {
        public function Derived() {
            S.s += "Derived ";
        }
    }

    public class Hello extends MovieClip {

        DEJAGNU_OBJ;

        public function Hello() {

            xcheck_equals(this, "[object Hello]");
            check_equals(typeof(this), "object");
            xcheck(this instanceof MovieClip);

            xcheck_equals(this.constructor, "[class Hello]");
            xcheck_equals(this.constructor.constructor, "[class Class]");
            xcheck_equals(this.parent, "[object Stage]");

            xcheck_equals(S, "[class S]");
            xcheck_equals(S.constructor, "[class Class]");
            xcheck_equals(S.__constructor__, undefined);
            
            check_equals(S.s, null);
            xcheck_equals(typeof(S.s), "object");

            S.s = "Hello ";
            check_equals(S.s, "Hello ");

            trace(Base);

            xcheck_equals(Base, "[class Base]");
            xcheck_equals(Base.prototype, "[object Object]");
            xcheck_equals(Derived, "[class Derived]");
            xcheck_equals(Base.prototype, "[object Object]");

            xcheck_equals(Derived.constructor, "[class Class]");
            xcheck_equals(Base.constructor, "[class Class]");
            xcheck_equals(Derived.__constructor__, undefined);
            xcheck_equals(Base.__constructor__, undefined);

            var b1 : Base = new Base();
            xcheck(b1 instanceof Base);
            check(!(b1 instanceof Derived));
            check_equals(S.s, "Hello Base ");
            
            xcheck_equals(b1.constructor, "[class Base]");
            
            S.s = "";
            var b2 : Base = new Derived();
            xcheck(b2 instanceof Base);
            xcheck(b2 instanceof Derived);
            xcheck_equals(S.s, "Base Derived ");

            S.s = "";
            var d1 : Derived = new Derived();
            xcheck(d1 instanceof Base);
            xcheck(d1 instanceof Derived);
            xcheck_equals(S.s, "Base Derived ");

            totals(30);

            done();
        }
    }
}
         
