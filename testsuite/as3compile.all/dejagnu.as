// Dejagnu.as - MTASC class for dejagnu-like testing.
//
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
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

package dejagnu {

    import flash.text.*;

    public class Dejagnu {

        private static var passed = 0;
        private static var failed = 0;
        private static var xpassed = 0;
        private static var xfailed = 0;
        private static var untest = 0;
        private static var unresolve = 0;
        private static var tf:TextField = new TextField();
        
        public static function fail (why) {
            failed++;
            var msg = 'FAILED: '+why;
            xtrace(msg);
        }
        
        public static function xfail(why) {
            xfailed++;
            var msg = 'XFAILED: '+why;
            xtrace(msg);
        }

        public static function pass(why) {
            passed++;
            var msg = 'PASSED: '+why;
            trace (msg);
        }

        public static function xpass(why) {
            xpassed++;
            var msg = 'XPASSED: '+why;
            trace (msg);
        }

        public static function totals(exp, msg) {
            var obt = testcount();
            if ( exp != undefined && obt != exp ) {
                fail('Test run '+obt+' (expected '+exp+') ['+msg+']');
            } else {
                pass('Test run '+obt+' ['+msg+']');
            }
        }
    
        public static function xtotals(exp, msg) {
            var obt = testcount();
            if ( exp != undefined && obt != exp ) {
                xfail('Test run '+obt+' (expected '+exp+') ['+msg+']');
            } else {
                xpass('Test run '+obt+' ['+msg+']');
            }
        }
    
        public static function check_equals(obt, exp, msg, expression) {
            if (msg == null) msg = "";
            if (obt == exp) { 
                pass(expression + ' == ' + exp + ' ' + msg);
            }
            else { 
                fail(expression + ': expected: "' + exp + 
                        '" , obtained: "' + obt + '" ' + msg);
            }
        }
    
        public static function xcheck_equals(obt, exp, msg, expression) {
            if (msg == null) msg = "";
            if (obt == exp) { 
                xpass(expression + ' == ' + exp + ' ' + msg);
            }
            else {
                xfail(expression + ': expected: "' + exp + 
                        '" , obtained: "' + obt + '" ' + msg);
            }
        }
    
        public static function check(a, msg) {
            if (a) {
                pass(msg != undefined ? msg : a);
            }
            else {
                fail(msg != undefined ? msg : a);
            }
        }
    
        public static function xcheck(a, msg) {
            if (a) { 
                xpass(msg != undefined ? msg : a);
            }
            else { 
                xfail(msg != undefined ? msg : a);
            }
        }
    
        public static function note(msg) {
            xtrace(msg);
        }
    
        public static function untested(msg) {
            trace("UNTESTED: "+msg);
        }
    
        public static function unresolved(msg) {
            trace("UNRESOLVED: "+msg);
        }
    
        /// Private functions.

        private static function xtrace(msg) {
            //tf.text += msg + "\n";
            trace(msg);
        }

        private static function done() {
            printtotals();
            trace("__END_OF_TEST__");
        }
        
        private static function testcount() {
            var c = 0;
            if ( passed ) c += passed;
            if ( failed ) c += failed;
            if ( xpassed ) c += xpassed;
            if ( xfailed ) c += xfailed;
            return c;
        }
    
        private static function printtotals() {
            xtrace('#passed: '+ passed);
            xtrace('#failed: '+ failed);
            if ( xpassed ) {
                xtrace('#unexpected successes: '+ xpassed);
            }
            if ( xfailed ) {
                xtrace('#expected failures: '+ xfailed);
            }
            xtrace('#total tests run: '+ testcount());
        }
    
    
    }

}

