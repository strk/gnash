import TestClass;

class Test extends TestClass
{
	function Test()
	{
		_root.check_equals(typeof(super), 'object');
		super(); // how can we invoke an object here !?
	}

	static function main(mc)
	{
		var myTest = new Test;
		// odd enough, if we output an SWF7 movie, this fails
		// with the reference player too !x
		_root.xcheck_equals(myTest.TestClassCtorCalled, 'called');
	}
}
