// Dejagnu.hx - HAXE class for dejagnu-like testing.
//
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

    static private function callerInfo(p:haxe.PosInfos) {
        return p.fileName + ": " + p.lineNumber;
    }

    // This is a trick to force our 'init' function
    // to be automatically called at the start of the movie.
    static var inithack = init();

    static var tf:TextField; 
	
    static function init() {
        //if(dejagnu_module_initialized == 1) return;
        
     // create a textfield to output to
#if flash9
 	tf = new TextField();
	tf.wordWrap = true;
	tf.width = 390;
	tf.height = 300;
	flash.Lib.current.addChild(tf);
#else
	flash.Lib.current.createTextField("tfsoft", 100,0,0,390,290);
	untyped tfsoft.multiline = true;
	untyped tfsoft.wordWrap = true;
	untyped tfsoft.border = true;
	untyped tfsoft.type = "input";
#end
        //dejagnu_module_initialized = 1;

	return null;
    }

    static public function fail (why, ?p:haxe.PosInfos) {
        failed++;
        var msg = 'FAILED: ' + why + " " + callerInfo(p);
        xtrace(msg);
    }

    static public function xfail(why, ?p:haxe.PosInfos) {
        xfailed++;
        var msg = 'XFAILED: '+why+ " " + callerInfo(p);
        xtrace(msg);
    }

    static public function pass(why, ?p:haxe.PosInfos) {
        passed++;
        var msg = 'PASSED: '+why+ " " + callerInfo(p);
        xtrace(msg);
    }

    static public function xpass(why, ?p:haxe.PosInfos) {
        xpassed++;
        var msg = 'XPASSED: '+why+ " " + callerInfo(p);
        xtrace(msg);
    }
    
	static public function check_equals(obt:Dynamic, exp:Dynamic, msg, ?p:haxe.PosInfos) {
        if(msg == null) msg = "";
        if (obt == exp) {
            // Pass the calling posinfos.
            pass(obt+' == ' + exp + ' ' + msg, p);
        } else {
            fail('expected: "' + exp + '" , obtained: "' + obt + '" ' + msg, p);
		}
    }
	
	static public function xcheck_equals(obt:Dynamic, exp:Dynamic, msg, ?p:haxe.PosInfos)
	{
		if(msg == null) msg = "";
		if ( obt == exp ) 
		{
			xpass(obt+' == '+exp+' '+msg, p);
		}
		else 
		{
			xfail('expected: '+exp+' , obtained: '+obt+" "+msg, p);
		}
	}
	
	static public function check(a : Dynamic, msg, ?p:haxe.PosInfos)
	{
		if ( a ) 
		{
			if ( msg != null ) pass(msg, p);
			else pass(a, p);
		}
		else 
		{
			if ( msg != null ) fail(msg, p);
			else fail(a, p);
		}
	}
	
	static public function xcheck(a : Dynamic, msg, ?p:haxe.PosInfos)
	{
		if ( a ) 
		{
			if ( msg != null ) xpass(msg, p);
			else xpass(a, p);
		}
		else 
		{
			if ( msg != null ) xfail(msg, p);
			else xfail(a, p);
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

    static public function totals(exp:Dynamic, msg:Dynamic, ?p:haxe.PosInfos) {
        var obt = testcount();
        if ( exp != null && obt != exp ) {
            fail('Test run '+obt+' (expected '+exp+') '+msg, p);
        } else {
            pass('Test run '+obt+' '+msg, p);
        }
    }

    static public function xtotals(exp:Dynamic, msg:Dynamic, ?p:haxe.PosInfos) {
        var obt = testcount();
        if ( exp != null && obt != exp ) {
            xfail('Test run '+obt+' (expected '+exp+') ['+msg+']', p);
        } else {
            xpass('Test run '+obt+' ['+msg+']', p);
        }
    }    
    
    static public function note(msg, ?p:haxe.PosInfos) {
        xtrace(msg + " " + p.lineNumber);
    }
    
    static function xtrace(msg) {
#if flash9
        tf.text += msg+"\n";
#else
		untyped tfsoft.text += msg+"\n";
#end
		flash.Lib.trace(msg);
    }

    static public function untested(msg, ?p:haxe.PosInfos) {
#if flash9
        tf.text += "UNTESTED: "+msg+"\n";
#else
		untyped tfsoft.text += "UNTESTED: "+msg+"\n";
#end
		flash.Lib.trace("UNTESTED: " + msg );
    }

    static public function unresolved(msg) {
		unresolve++;
#if flash9
        tf.text += "UNRESOLVED: "+msg+"\n";
#else
		untyped tfsoft.text += "UNRESOLVED: "+msg+"\n";
#end
		flash.Lib.trace("UNRESOLVED: "+msg);
    }
    
    static public function done() {
        printtotals();
#if flash9
        tf.text += "__END_OF_TEST__";
#else	
		untyped tfsoft.text += "__END_OF_TEST__";
#end
		flash.Lib.trace("__END_OF_TEST__");
    }
}
