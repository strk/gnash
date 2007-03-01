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

	function test_laszlo_core()
	{
		var A = function() { this.constructor = arguments.callee; };
		A.prototype.test = function () { return true; }

		var B = function () { this.constructor = arguments.callee; }

		B.prototype = new A();

		B.prototype.test = function () {
			_root.check( this.constructor.prototype.constructor.prototype.test() );
		}

		var binst = new B();
		_root.check_equals(typeof(binst), 'object');
		_root.check_equals(typeof(binst.test), 'function');
		binst.test();
	}

	function test_all()
	{
		_root.check_equals(typeof(this.loadMovie), 'function');
		_root.check_equals(this.loadMovie, super.loadMovie);
		_root.check(this.lineTo != super.lineTo); // overridden
		this.test_laszlo_core();

	}

	static function main(mc)
	{
		var myTest = new Test;
		myTest.test_all();
	}

}
