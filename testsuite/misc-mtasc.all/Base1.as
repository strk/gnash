
class Base1
{
  var baseCtorCalled;
  var baseThisPtr;
  var baseDirectCalled;
  var baseViaApplyCalled;
  
  // constructor
  function Base1()
  {
    this.baseCtorCalled = true;
    baseThisPtr = this;
  }

  function direct()
  {
    this.baseDirectCalled = true;
  }

  function viaApply()
  {
    this.baseViaApplyCalled = true;
  }
}

