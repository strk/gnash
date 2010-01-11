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

// Important points about the AS3 Object class:
//
// prototype is a static, read-only property. It doesn't have the same
// role in AS3 as in AS2. You can assign properties to the prototype
// member, but you cannot change what it points to.
//
// The prototype property provides dynamic inheritance. Normally, inheritance
// is done using a Traits object (bytecode, not available to AS). These two
// mechanisms are separate.

package main {

    import flash.display.MovieClip;

    public class Main extends MovieClip {


        DEJAGNU_OBJ;

        public function Main() {

            xcheck_equals(Object, "[class Object]");
            check_equals(Object.prototype, "[object Object]");
            xcheck_equals(Object.constructor, "[class Class]");
            
            xcheck_equals(typeof(Object), "object");
            check_equals(typeof(Object.prototype), "object");
            xcheck_equals(typeof(Object.constructor), "object");

            check(Object.prototype.hasOwnProperty("constructor"));
            check(Object.prototype.hasOwnProperty("hasOwnProperty"));
            check(Object.prototype.hasOwnProperty("isPrototypeOf"));
            xcheck(Object.prototype.hasOwnProperty("propertyIsEnumerable"));
            xcheck(Object.prototype.hasOwnProperty("setPropertyIsEnumerable"));
            check(Object.prototype.hasOwnProperty("toString"));
            check(Object.prototype.hasOwnProperty("valueOf"));
            
            check(!Object.prototype.hasOwnProperty("__proto__"));
            check(!Object.prototype.hasOwnProperty("prototype"));

            check(Object.prototype.isPrototypeOf(MovieClip));
            check(Object.prototype.isPrototypeOf(this));

            var a = new Object();
            check_equals(a, "[object Object]");       
            check(!a.hasOwnProperty("constructor"));
            check(!a.hasOwnProperty("hasOwnProperty"));
            check(!a.hasOwnProperty("isPrototypeOf"));
            check(!a.hasOwnProperty("propertyIsEnumerable"));
            check(!a.hasOwnProperty("setPropertyIsEnumerable"));
            check(!a.hasOwnProperty("toString"));
            check(!a.hasOwnProperty("valueOf"));
            
            xcheck(!a.hasOwnProperty("__proto__"));
            check(!a.hasOwnProperty("prototype"));

            // This crashes the Adobe player 9.
            // check(Object.isPrototypeOf(this));
		

            totals(27);
            done();
        }
    }
}
         
