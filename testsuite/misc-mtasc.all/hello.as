#include "check.as"

class Test extends MovieClip
{

	function test_all()
	{
		check_equals(typeof(this.loadMovie), 'function');
	}

        // Main entry point, provided by MTASC
        // mc == the root MC this class is compiled into.
        // If running standalone, mc == _root == _level0
        // If loaded into another swf, it may vary.
	static function main(mc)
	{
		var myTest = new Test();
		myTest.test_all();

		var a = 1;

		// This is how you call check_equals
		check_equals(a, 1);

		// This is how you print notes (trace + visual trace)
		note("Hello world");

                // Check number of tests run (for consistency)
		check_totals(2);

                // Call this after finishing all tests. It prints out the totals.
                Dejagnu.done();
	}

}
