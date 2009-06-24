// Dejagnu.hx - HAXE class for dejagnu-like testing.
//
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

import flash.Lib;
#if flash9
import flash.text.TextField;
import flash.text.TextFieldType;
import flash.external.ExternalInterface;
import flash.display.MovieClip;
#else
import flash.TextField;
import flash.MovieClip;
#end
import haxe.PosInfos;

class DejaGnu {
    static var passed = 0;
    static var failed = 0;
    static var xpassed = 0;
    static var xfailed = 0;
    static var untest = 0;
    static var unresolve = 0;
    static var position:PosInfos;
	static var trace = untyped __global__['trace'];

    // This is a trick to force our 'init' function
    // to be automatically called at the start of the movie.
    static var inithack = init();

    static var tf:TextField; 
	
    static function init() {
        //if(dejagnu_module_initialized == 1) return;
        
        // create a textfield to output to
#if flash9
 	tf = new TextField();
	//tf.autoSize = flash.text.TextFieldAutoSize.LEFT;
	//tf.multiline = true;
	tf.wordWrap = true;
	tf.width = 390;
	tf.height = 300;
	//tf.border = true;
    //flash.Lib.current.createTextField("textout", 99, 10, 10, 500, 500);
	flash.Lib.current.addChild(tf);
#else
	flash.Lib.current.createTextField("tf", 10,0,0,390,390);
	tf.multiline = true;
	tf.wordWrap = true;
	tf.autoSize = "left";
	tf.border = true;
#end
        //dejagnu_module_initialized = 1;

	return null;
    }

    static public function fail (why) {
        failed++;
        var msg = 'FAILED: '+why;
        xtrace(msg);
    }

    static public function xfail(why) {
        xfailed++;
        var msg = 'XFAILED: '+why;
        xtrace(msg);
    }

    static public function pass(why) {
        passed++;
        var msg = 'PASSED: '+why;
        xtrace(msg);
    }

    static public function xpass(why) {
        xpassed++;
        var msg = 'XPASSED: '+why;
        xtrace(msg);
    }
    
	static public function check_equals(obt:Dynamic, exp:Dynamic, msg) {
        if(msg == null) msg = "";
        if ( obt == exp ) {
            pass(obt+' == '+exp+' '+msg);
        } else {
            fail('expected: "'+exp+'" , obtained: "'+obt+'" '+msg);
		}
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
        if ( unresolve > 0 ) {
            xtrace('#tests unresolved: '+ unresolve);
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

    static function publicxtotals(exp:Dynamic, msg:Dynamic) {
        var obt = testcount();
        if ( exp != null && obt != exp ) {
            xfail('Test run '+obt+' (expected '+exp+') ['+msg+']');
        } else {
            xpass('Test run '+obt+' ['+msg+']');
        }
    }    
    
    static public function note(msg) {
        xtrace(msg);
    }
    
    static function xtrace(msg) {
        tf.text += msg+"\n";
		//trace(msg);
		//tf.text = "<head>\n<style type=\"text/css\">\n</style>\n</head>\n<body>\n<textarea cols=\"30\" rows=\"4\">\nHere's enough text to make this textarea grow scrollbars....\nHTML scrollbars, scrollbar color, change browser scrollbars,\ncss scrollbar, change color of the scrollbar...\n</textarea>\n</body>";
		//tf.htmlText = "<head>\n<style type=\"text/css\">\n</style>\n</head>\n<body>\n<textarea cols=\"30\" rows=\"4\">\nHere's enough text to make this textarea grow scrollbars....\nHTML scrollbars, scrollbar color, change browser scrollbars,\ncss scrollbar, change color of the scrollbar...\n</textarea>\n</body>";
//#if flash9
        flash.Lib.trace(msg);
//#else	
//		untyped flash.Boot.__trace(msg,position);
//#end
    }

    static public function untested(msg) {
//#if flash9
        flash.Lib.trace("UNTESTED: "+msg);
//#else	
//		untyped flash.Boot.__trace("UNTESTED: "+msg,position);
//#end
    }

    static public function unresolved(msg) {
//#if flash9
        unresolve++;
        flash.Lib.trace("UNRESOLVED: "+msg);
//#else	
//		untyped flash.Boot.__trace("UNRESOLVED: "+msg,position);
//#end
    }
    
    static public function done() {
        printtotals();
//#if flash9
        flash.Lib.trace("__END_OF_TEST__");
//#else	
//		untyped flash.Boot.__trace("__END_OF_TEST__",position);
		//untyped flash.Boot.__trace(""+flash.Lib._root._width,position);
		//flash.Lib.current._width = 800;
//#end
	
	//loadMovie('fscommand:quit', _root);
    }
}
