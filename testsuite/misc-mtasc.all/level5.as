#include "check.as"

class Level5
{
	static function main(mc)
	{
                // The ""+ is there to force conversion to a string
                check_equals(""+mc, "_level5");

                // check that we can acess back to _level0
                check_equals(_level0.testvar, 1239);

                // check that we can modify vars on our own level
                check_equals(_level5.testvar, undefined);
                _level5.testvar = 6789;
                check_equals(_level5.testvar, 6789);

                // check that we can modify vars on _level0
                check_equals(_level0.testvar2, undefined);
                _level0.testvar2 = true;
                check_equals(_level0.testvar2, true);


                // load yet another swf
                getURL("level99.swf","_level"+99);
	}
}
