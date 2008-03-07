#include "check.as"

import TestClass;

class Test extends TestClass
{
  var x;

  function setX(nx)
  {
    check(!arguments.hasOwnProperty('toString'));
    check_equals(arguments.toString(), '2');
    check(arguments instanceof Object);
    check_equals(typeof(arguments.__proto__), 'object');
    check_equals(typeof(arguments.__proto__.constructor), 'function');
    check_equals(typeof(arguments.__proto__.constructor.__proto__), 'object');
    check_equals(typeof(arguments.__proto__.constructor.__proto__.constructor), 'function');
    check_equals(arguments.__proto__, arguments.__proto__.constructor.prototype);
    check(arguments.__proto__.hasOwnProperty('toString'));
    check(arguments.__proto__.toString != Object.prototype.toString);
    check(arguments.__proto__ !=  Object.prototype);
    check(arguments.__proto__.constructor.__proto__ != Object.prototype);
    check(arguments.__proto__.constructor.__proto__ != Array.prototype);
    this.x = nx;
  }

  function Test()
  {
    check_equals(typeof(super), 'object');

    // This should be the effect of an 'extends' op
    check_equals(Test['prototype']['__constructor__'], TestClass);
    check_equals(Test['prototype']['__proto__'], TestClass.prototype);

    // This seems to trigger an ActionCallMethod(undefined, super).
    // It is expected that the VM fetches super.constructor and calls
    // that instead.
    // The *this* pointer should be set by the VM as the current one
    // at time of ActionCallMethod.
    super();

    // This seems to trigger an ActionCallMethod(myTest, setX).
    setX(2);
  }

  static function main(mc)
  {
    var myTest = new Test;
    // odd enough, if we output an SWF7 movie, this fails
    // with the reference player too !
    check_equals(myTest.TestClassCtorCalled, 'called');

    // This checks that the 'this' pointer is properly set
    // (and shows it's NOT properly set with Gnash)
    check_equals(typeof(myTest.__proto__.TestClassCtorCalled), 'undefined');

    // This checks that the 'this' pointer is properly set for "normal"
    // ActionCallMethod (see setX(2) in Test ctor)  
    check_equals(myTest.x, 2);
    check_equals(typeof(myTest.__proto__.x), 'undefined');

    check_totals(20);
    Dejagnu.done();
  }
}
