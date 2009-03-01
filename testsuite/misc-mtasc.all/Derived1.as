
import Base1;

class Derived1 extends Base1
{
  var derived1CtorCalled;
  var derivedThisPtr;
  var derived1DirectCalled;
  var derived1ViaApplyCalled;
  
  // constructor
  function Derived1()
  {
    super();
    this.derived1CtorCalled = true;
    derivedThisPtr = this;
  }

  function direct()
  {
    super.direct();
    this.derived1DirectCalled = true;
  }

  function viaApply()
  {
    super.viaApply();
    this.derived1ViaApplyCalled = true;
  }
}
