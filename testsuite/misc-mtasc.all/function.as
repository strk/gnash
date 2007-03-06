import TestClass;

class Test extends TestClass
{
	function Test()
	{
		super();
	}

	static function main(mc)
	{
		var myTest = new Test;
		_root.check_equals(myTest.TestClassCtorCalled, true);
	}
}
