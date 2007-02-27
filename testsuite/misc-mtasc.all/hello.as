class Test extends MovieClip
{

	function test_all()
	{
		_root.check_equals(typeof(this.loadMovie), 'function');

	}

	static function main(mc)
	{
		var myTest = new Test;
		myTest.test_all();

		var a = 1;

		// This is how you call check_equals
		_root.check_equals(a, 1);

		// This is how you print notes (trace + visual trace)
		_root.note("Hello world");
	}

}
