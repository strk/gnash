#include "check.as"

import Derived1;

class Derived11 extends Derived1
{
  var derived11CtorCalled;
  
  // constructor
  function Derived11()
  {
    super();
    this.derived11CtorCalled = true;
  }

	static function main()
	{
	   var derivedObj = new Derived11();
	   
	   check_equals(derivedObj.derived11CtorCalled, true);
	   check_equals(derivedObj.derived1CtorCalled, true);
	   check_equals(derivedObj.baseCtorCalled, true);
	   Dejagnu.done();
	}
}
