#include "check.as"

class LevelsMain
{
	static function main(mc)
	{
                // create a var that other swfs can test
                _level0.testvar = 1239;

                var test = new LevelsMain();
                test.run();

                // The ""+ is there to force conversion to a string
                check_equals(""+mc, "_level0");
	}

        function run() {
            trace("main class running");
            getURL("level5.swf","_level"+5);
        }
}
