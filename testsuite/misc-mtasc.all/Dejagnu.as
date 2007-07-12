class Dejagnu {
    static var passed = 0;
    static var failed = 0;
    static var xpassed = 0;
    static var xfailed = 0;
    static var untest = 0;
    static var unresolve = 0;

    // This is a trick to force our 'init' function
    // to be automatically called at the start of the movie.
    static var inithack = init();

    static function init() {
        if(_level0.dejagnu_module_initialized == 1) return;

        // create a textfield to output to
        _level0.createTextField("textout", 99, 10, 10, 500, 500);
        _level0.dejagnu_module_initialized = 1;
    }

    static function fail (why) {
        failed++;
        var msg = 'FAILED: '+why;
        xtrace(msg);
    }

    static function xfail(why) {
        xfailed++;
        var msg = 'XFAILED: '+why;
        xtrace(msg);
    }

    static function pass(why) {
        passed++;
        var msg = 'PASSED: '+why;
        trace (msg);
    }

    static function xpass(why) {
        xpassed++;
        var msg = 'XPASSED: '+why;
        trace (msg);
    }

    static function totals() {
        xtrace('#passed: '+ passed);
        xtrace('#failed: '+ failed);
        if ( xpassed ) {
            xtrace('#unexpected successes: '+ xpassed);
        }
        if ( xfailed ) {
            xtrace('#expected failures: '+ xfailed);
        }
   
    }

    static function check_equals(obt, exp, msg) {
        if(msg == null) msg = "";
        if ( obt == exp ) 
            pass(obt+' == '+exp);
        else 
            fail('expected: "'+exp+'" , obtained: "'+obt+'" '+msg);
    }

    static function xcheck_equals(obt, exp, msg) {
        if(msg == null) msg = "";
        if ( obt == exp ) 
            xpass(obt+' == '+exp);
        else 
            xfail('expected: '+exp+' , obtained: '+obt+" "+msg);
    }

    static function check(a, msg) {
        if ( a ) 
            pass(msg != undefined ? msg : a);
        else 
            fail(msg != undefined ? msg : a);
    }

    static function xcheck(a, msg) {
        if ( a ) 
            xpass(msg != undefined ? msg : a);
        else 
            xfail(msg != undefined ? msg : a);
    }

    static function note(msg) {
        xtrace(msg);
    }

    static function xtrace(msg) {
        _level0.textout.text += msg + "\n";
        trace(msg);
    }

    static function untested(msg) {
        trace("UNTESTED: "+msg);
    }

    static function unresolved(msg) {
        trace("UNRESOLVED: "+msg);
    }

    static function done() {
        totals();
    }

}
