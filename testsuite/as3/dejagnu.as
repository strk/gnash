// Dejagnu.as - MTASC class for dejagnu-like testing.
//
//   Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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
    import flash.display.Sprite;

    public class Dejagnu extends Sprite {

        private var passed;
        private var failed;
        private var xpassed;
        private var xfailed;
        private var untest;
        private var unresolve;
        private var tf:TextField;
        
        public function Dejagnu(o) {
            passed = 0;
            failed = 0;
            xpassed = 0;
            xfailed = 0;
            untest = 0;
            unresolve = 0;
            tf = new TextField();
            tf.autoSize = TextFieldAutoSize.LEFT;
            addChild(tf);
            o.addChild(this);
        } 
       
        public function fail (why) {
            failed++;
            var msg = 'FAILED: '+why;
            xtrace(msg);
        }
        
        public function xfail(why) {
            xfailed++;
            var msg = 'XFAILED: '+why;
            xtrace(msg);
        }

        public function pass(why) {
            passed++;
            var msg = 'PASSED: '+why;
            trace (msg);
        }

        public function xpass(why) {
            xpassed++;
            var msg = 'XPASSED: '+why;
            trace (msg);
        }

        public function totals(exp, msg) {
            var obt = testcount();
            if ( exp != undefined && obt != exp ) {
                fail('Test run '+obt+' (expected '+exp+') ['+msg+']');
            } else {
                pass('Test run '+obt+' ['+msg+']');
            }
        }
    
        public function xtotals(exp, msg) {
            var obt = testcount();
            if ( exp != undefined && obt != exp ) {
                xfail('Test run '+obt+' (expected '+exp+') ['+msg+']');
            } else {
                xpass('Test run '+obt+' ['+msg+']');
            }
        }
    
        public function check_equals(obt, exp, msg, expression) {
            if (msg == null) msg = "";
            if (obt == exp) { 
                pass(expression + ' == ' + exp + ' ' + msg);
            }
            else { 
                fail(expression + ': expected: "' + exp + 
                        '" , obtained: "' + obt + '" ' + msg);
            }
        }
    
        public function xcheck_equals(obt, exp, msg, expression) {
            if (msg == null) msg = "";
            if (obt == exp) { 
                xpass(expression + ' == ' + exp + ' ' + msg);
            }
            else {
                xfail(expression + ': expected: "' + exp + 
                        '" , obtained: "' + obt + '" ' + msg);
            }
        }
    
        public function check(a, msg) {
            if (a) {
                pass(msg != undefined ? msg : a);
            }
            else {
                fail(msg != undefined ? msg : a);
            }
        }
    
        public function xcheck(a, msg) {
            if (a) { 
                xpass(msg != undefined ? msg : a);
            }
            else { 
                xfail(msg != undefined ? msg : a);
            }
        }
    
        public function note(msg) {
            xtrace(msg);
        }
    
        public function untested(msg) {
            trace("UNTESTED: "+msg);
        }
    
        public function unresolved(msg) {
            trace("UNRESOLVED: "+msg);
        }
    
        /// Private functions.

        private function xtrace(msg) {
            tf.text += msg + "\n";
            trace(msg);
        }

        private function done() {
            printtotals();
            trace("__END_OF_TEST__");
        }
        
        private function testcount() {
            var c = 0;
            if ( passed ) c += passed;
            if ( failed ) c += failed;
            if ( xpassed ) c += xpassed;
            if ( xfailed ) c += xfailed;
            return c;
        }
    
        private function printtotals() {
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

