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
		_root.check_equals(myTest.TestClassCtorCalled, 'called');
	}
}
