// levels.as - MTASC testcase for loading into _level targets
//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
//
//
// zou lunkai zoulunkai@gmail.com
// 
// Test super in a mutilevel inheritance context(Base1.Derived1.Derived11).
//
#include "check.as"

import Derived1;

class Derived11 extends Derived1
{
  var derived11CtorCalled;
  var thisPtr;
  
  // constructor
  function Derived11()
  {
    super();
    this.derived11CtorCalled = true;
    thisPtr = this;
  }

  static function main()
  {
     // Gnash got an unexpected 'ActionLimit hit' here.
     var derivedObj = new Derived11();
     
     // check that all constructors in the inheritance chain are called.    
     check_equals(derivedObj.baseCtorCalled, true);
     check_equals(derivedObj.derived1CtorCalled, true);
     check_equals(derivedObj.derived11CtorCalled, true);
     
     // check this pointers. 
     check_equals(derivedObj.thisPtr, derivedObj);
     check_equals(derivedObj.derivedThisPtr, derivedObj);
     check_equals(derivedObj.baseThisPtr, derivedObj);

     check_totals(6);
     Dejagnu.done();
  }
}
