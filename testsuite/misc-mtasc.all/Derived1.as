
import Base1;

class Derived1 extends Base1
{
  var derived1CtorCalled;
  
  // constructor
  function Derived1()
  {
    super();
    this.derived1CtorCalled = true;
  }
}
