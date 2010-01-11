// Dejagnu.as - HAXE class for dejagnu-like testing.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//

import flash.Lib;
import flash.text.TextField;

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

    static var tf:flash.text.TextField;

    static function init() {
        //if(dejagnu_module_initialized == 1) return;

        // create a textfield to output to
	tf = new flash.text.TextField();
	tf.autoSize = flash.text.TextFieldAutoSize.LEFT;
        //flash.Lib.current.createTextField("textout", 99, 10, 10, 500, 500);
	flash.Lib.current.addChild(tf);

        //dejagnu_module_initialized = 1;

	return null;
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
        flash.Lib.trace(msg);
    }

    static function xpass(why) {
        xpassed++;
        var msg = 'XPASSED: '+why;
        flash.Lib.trace(msg);
    }

    static function testcount() {
        var c = 0;
        if ( passed > 0 ) c += passed;
        if ( failed > 0 ) c += failed;
        if ( xpassed > 0 ) c += xpassed;
        if ( xfailed > 0 ) c += xfailed;
        if ( unresolve > 0 ) c += unresolve;
        return c;
    }

    static function printtotals() {
        xtrace('#passed: '+ passed);
        xtrace('#failed: '+ failed);
        if ( xpassed > 0 ) {
            xtrace('#unexpected successes: '+ xpassed);
        }
        if ( xfailed > 0 ) {
            xtrace('#expected failures: '+ xfailed);
        }
        if ( unresolve > 0) {
            xtrace('#tests unresolved'+ unresolve);
        }
		xtrace('#total tests run: '+ testcount());
    }

    static public function totals(exp:Dynamic, msg:Dynamic) {
        var obt = testcount();
        if ( exp != null && obt != exp ) {
            fail('Test run '+obt+' (expected '+exp+') '+msg);
        } else {
            pass('Test run '+obt+' '+msg);
        }
    }

    static function xtotals(exp:Dynamic, msg:Dynamic) {
        var obt = testcount();
        if ( exp != null && obt != exp ) {
            xfail('Test run '+obt+' (expected '+exp+') ['+msg+']');
        } else {
            xpass('Test run '+obt+' ['+msg+']');
        }
    }

    static public function check_equals(obt:Dynamic, exp:Dynamic, msg) {
        if(msg == null) msg = "";
        if ( obt == exp ) 
            pass(obt+' == '+exp+' '+msg);
        else 
            fail('expected: "'+exp+'" , obtained: "'+obt+'" '+msg);
    }

	static public function xcheck_equals(obt:Dynamic, exp:Dynamic, msg)
	{
		if(msg == null) msg = "";
		if ( obt == exp ) 
		{
			xpass(obt+' == '+exp+' '+msg);
		}
		else 
		{
			xfail('expected: '+exp+' , obtained: '+obt+" "+msg);
		}
	}

	static public function check(a : Dynamic, msg)
	{
		if ( a ) 
		{
			if ( msg != null ) pass(msg);
			else pass(a);
		}
		else 
		{
			if ( msg != null ) fail(msg);
			else fail(a);
		}
	}

	static public function xcheck(a : Dynamic, msg)
	{
		if ( a ) 
		{
			if ( msg != null ) xpass(msg);
			else xpass(a);
		}
		else 
		{
			if ( msg != null ) xfail(msg);
			else xfail(a);
		}
	}

    static public function note(msg) {
        xtrace(msg);
    }

    static function xtrace(msg) {
        tf.text += msg + "\n";
        flash.Lib.trace(msg);
    }

    static function untested(msg) {
        flash.Lib.trace("UNTESTED: "+msg);
    }

    static function unresolved(msg) {
        unresolve++;
        flash.Lib.trace("UNRESOLVED: "+msg);
    }

    static public function done() {
        printtotals();
	flash.Lib.trace("__END_OF_TEST__");
	//loadMovie('fscommand:quit', _root);
    }

}
