
class Base1
{
  var baseCtorCalled;
  var baseThisPtr;
  
  // constructor
  function Base1()
  {
    this.baseCtorCalled = true;
    baseThisPtr = this;
  }
}
