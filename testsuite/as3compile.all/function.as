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
    public class A {
        public function a() { return "string"; }
        public function b() { return 9; }
        public function c() { return 5.65; }
        public function d() { return new Object(); }
        public function e(a1) { return a1; }
        public function f(a1, a2) { return a1 + a2; }
        public function g() {}
        public function h() { return c(); }
        public function i() { return h(); }
        public function j() { return bb(); }

        public static function aa() { return "string"; }
        public static function bb() { return 10; }
        public static function cc() { return 2.45; }
        public static function dd() { return new Object(); }
        public static function ee(a1) { return a1; }
        public static function ff(a1, a2) { return a1 + a2; }
        public static function gg() {}
    }
}
        

package main {

import flash.display.MovieClip;

public class Main extends MovieClip {

    DEJAGNU_OBJ;

    import p1.A;

    public function Main() {

        xcheck_equals(A.aa, "function Function() {}");
        check_equals(typeof(A.aa), "function");
        
        check_equals(A.aa(), "string");
        check_equals(typeof(A.aa()), "string");
        check_equals(A.bb(), 10);
        check_equals(typeof(A.bb()), "number");
        check_equals(A.cc(), 2.45);
        check_equals(typeof(A.cc()), "number");
        check_equals(A.dd(), "[object Object]");
        check_equals(typeof(A.dd()), "object");
        check_equals(A.ee("f"), "f");
        check_equals(typeof(A.ee("f")), typeof("f"));
        check_equals(A.ee(45), 45);
        check_equals(typeof(A.ee(45)), typeof(45));
        check_equals(A.ff(45, 6), 51);
        check_equals(typeof(A.ff(45, 6)), typeof(51));
        check_equals(A.gg(), undefined);
        check_equals(typeof(A.gg()), "undefined");
        

        var a = new A();
        check_equals(a.a(), "string");
        check_equals(typeof(a.a()), "string");
        check_equals(a.b(), 9);
        check_equals(typeof(a.b()), "number");
        check_equals(a.c(), 5.65);
        check_equals(typeof(a.c()), "number");
        check_equals(a.d(), "[object Object]");
        check_equals(typeof(a.d()), "object");
        check_equals(a.e("f"), "f");
        check_equals(typeof(a.e("f")), typeof("f"));
        check_equals(a.e(45), 45);
        check_equals(typeof(a.e(45)), typeof(45));
        check_equals(a.f(45, 6), 51);
        check_equals(typeof(a.f(45, 6)), typeof(51));
        check_equals(a.g(), undefined);
        check_equals(typeof(a.g()), "undefined");

        check_equals(a.h(), a.c());
        check_equals(a.i(), a.h());
        check_equals(a.i(), 5.65);
        check_equals(a.j(), A.bb());
        check_equals(a.j(), 10);
        
        check_equals(a.f(a.h(), a.i()), 11.30);

        totals(40);
        done();
    }
}
         
}
