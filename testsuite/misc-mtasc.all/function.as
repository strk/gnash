import TestClass;

class Test extends TestClass
{
	var x;

	function setX(nx)
	{
		this.x = nx;
	}

	function Test()
	{
		_root.check_equals(typeof(super), 'object');

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
		_root.check_equals(myTest.TestClassCtorCalled, 'called');

		// This checks that the 'this' pointer is properly set
		// (and shows it's NOT properly set with Gnash)
		_root.xcheck_equals(typeof(myTest.__proto__.TestClassCtorCalled), 'undefined');

		// This checks that the 'this' pointer is properly set for "normal"
		// ActionCallMethod (see setX(2) in Test ctor)	
		_root.check_equals(myTest.x, 2);
		_root.check_equals(typeof(myTest.__proto__.x), 'undefined');
	}
}
