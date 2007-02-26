class Test extends MovieClip
{

	// constructor
	function Test()
	{
		_root.note("Test constructor called");
	}

	// Override MovieClip.lineTo
	function lineTo()
	{
	}

	function test_all()
	{
		_root.check_equals(typeof(this.loadMovie), 'function');
		_root.check_equals(this.loadMovie, super.loadMovie);
		_root.check(this.lineTo != super.lineTo); // overridden

	}

	static function main(mc)
	{
		var myTest = new Test;
		myTest.test_all();
	}

}
