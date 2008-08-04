#include "check.as"

// Class must be named with the PP prefix, as that's the name the
// file passed to haxe will have after the preprocessing step
class PP_Hello {
	static function main()
	{

		var a = 1;

		// This is how you call check_equals
		check_equals(a, 1);

		// This is how you call check
		check(true);
		check(!false);

		// This is how you print notes (trace + visual trace)
		note("Hello world");

                // Check number of tests run (for consistency)
		check_totals(3);

                // Call this after finishing all tests. It prints out the totals.
                Dejagnu.done();

	}
}
