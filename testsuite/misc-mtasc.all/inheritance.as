#include "check.as"

class Test extends MovieClip
{

	// constructor
	function Test()
	{
		note("Test constructor called");
	}

	// Override MovieClip.lineTo
	function lineTo()
	{
	}

	function test_proto_ext()
	{
		note("This is "+this);
		Function.prototype.make = function() { return true; };
		check(arguments.callee.make());
	}

	function test_laszlo_core()
	{
		var A = function() { this.constructor = arguments.callee; };
		A.prototype.test = function () { return true; }

		var B = function () { this.constructor = arguments.callee; }

		B.prototype = new A();

		B.prototype.test = function () {
			check( this.constructor.prototype.constructor.prototype.test() );
		}

		var binst = new B();
		check_equals(typeof(binst), 'object');
		check_equals(typeof(binst.test), 'function');
		note("test_laszlo_core called");
		binst.test();
	}

	function test_all()
	{
		check_equals(typeof(this.loadMovie), 'function');
		check_equals(this.loadMovie, super.loadMovie);
		check(this.lineTo != super.lineTo); // overridden
		check_equals(typeof(this.test_laszlo_core), 'function');
		check_equals(typeof(this.test_proto_ext), 'function');
		this.test_laszlo_core();
		check_equals(typeof(this.test_laszlo_core), 'function');
		check_equals(typeof(this.test_proto_ext), 'function');
		this.test_proto_ext();
		check_equals(typeof(this.test_laszlo_core), 'function');
		check_equals(typeof(this.test_proto_ext), 'function');
		note("This is "+this);

	}

	static function main(mc)
	{
		var myTest = new Test;
		myTest.test_all();

		check_totals(13);
                Dejagnu.done();
	}

}
