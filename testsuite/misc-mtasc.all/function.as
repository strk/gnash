import TestClass;

class Test extends TestClass
{
	function Test()
	{
		_root.check_equals(typeof(super), 'object');

		// This seems to trigger an ActionCallMethod(undefined, super).
		// It is expected that the VM fetches super.constructor and calls
		// that instead.
		super();
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
	}
}
