        // Dejagnu.sc - SWFC script for dejagnu-like testing.
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
        //
        //
        // Original author: David Rorex - drorex@gmail.com
        //
        //

        _root.Dejagnu = _global.Dejagnu = {
            passed:0,
            failed:0,
            xpassed:0,
            xfailed:0,
            untest:0,
            unresolve:0,

            fail : function (why) {
                this.failed++;
                var msg = 'FAILED: '+why;
                this.xtrace(msg);
            },

            xfail: function (why) {
                this.xfailed++;
                var msg = 'XFAILED: '+why;
                this.xtrace(msg);
            },

            pass: function (why) {
                this.passed++;
                var msg = 'PASSED: '+why;
                trace (msg);
            },

            xpass: function (why) {
                this.xpassed++;
                var msg = 'XPASSED: '+why;
                trace (msg);
            },

            totals: function (expectedTestsRun, msg) {
		if ( arguments.length > 1 )
		{
			ttr = 0;
			if ( this.passed ) ttr += this.passed;
			if ( this.failed ) ttr += this.failed;
			if ( this.xpassed ) ttr += this.xpassed;
			if ( this.xfailed ) ttr += this.xfailed;
			if ( this.untest ) ttr += this.untest;
			if ( this.unresolv ) ttr += this.unresolv;
			this.note("Total tests run: "+ttr+" typeof expected: "+typeof(expectedTestsRun));

			if ( expectedTestsRun != ttr )
			{
				this.fail("TOTAL tests run: "+ttr+", expected: "+expectedTestsRun+" ["+msg+"]");
			}
		}
                this.xtrace('#passed: '+ this.passed);
                this.xtrace('#failed: '+ this.failed);
                if ( this.xpassed ) {
                    this.xtrace('#unexpected successes: '+ this.xpassed);
                }
                if ( this.xfailed ) {
                    this.xtrace('#expected failures: '+ this.xfailed);
                }
           
		this.done();
            },

            xtotals: function (expectedTestsRun, msg) {
		if ( arguments.length > 1 )
		{
			ttr = 0;
			if ( this.passed ) ttr += this.passed;
			if ( this.failed ) ttr += this.failed;
			if ( this.xpassed ) ttr += this.xpassed;
			if ( this.xfailed ) ttr += this.xfailed;
			if ( this.untest ) ttr += this.untest;
			if ( this.unresolv ) ttr += this.unresolv;
			this.note("Total tests run: "+ttr+" typeof expected: "+typeof(expectedTestsRun));

			if ( expectedTestsRun != ttr )
			{
				this.xfail("TOTAL tests run: "+ttr+", expected: "+expectedTestsRun+" ["+msg+"]");
			}
			else
			{
				this.xpass("TOTAL tests run: "+ttr+" ["+msg+"]");
			}
		}
                this.xtrace('#passed: '+ this.passed);
                this.xtrace('#failed: '+ this.failed);
                if ( this.xpassed ) {
                    this.xtrace('#unexpected successes: '+ this.xpassed);
                }
                if ( this.xfailed ) {
                    this.xtrace('#expected failures: '+ this.xfailed);
                }
           
		this.done();
            },

            check_equals: function (obt, exp, msg, expression) {
                if(msg == null) msg = "";
                if ( obt == exp ) 
                    this.pass(expression+' == '+exp+" "+msg);
                else 
                    this.fail(expression+': expected: "'+exp+'" , obtained: "'+obt+'" '+msg);
            },

            xcheck_equals: function (obt, exp, msg, expression) {
                if(msg == null) msg = "";
                if ( obt == exp ) 
                    this.xpass(expression+' == '+exp+" "+msg);
                else 
                    this.xfail(expression+': expected: '+exp+' , obtained: '+obt+" "+msg);
            },

            check: function (a, msg) {
                if ( a ) 
                    this.pass(msg != undefined ? msg : a);
                else 
                    this.fail(msg != undefined ? msg : a);
            },

            xcheck: function (a, msg) {
                if ( a ) 
                    this.xpass(msg != undefined ? msg : a);
                else 
                    this.xfail(msg != undefined ? msg : a);
            },

            note: function (msg) {
                this.xtrace(msg);
            },

            xtrace: function (msg) {
            	_level0.textout._height += 20;
                _level0.textout.text += msg + "\n";
                trace(msg);
            },

            untested: function (msg) {
                trace("UNTESTED: "+msg);
            },

            unresolved: function (msg) {
                trace("UNRESOLVED: "+msg);
            },

            done: function () {
                _root.stop();
		trace("__END_OF_TEST__");
		loadMovie("fscommand:quit", _level0);
            }

        };

        if(_level0.dejagnu_module_initialized != 1) {
            // create a textfield to output to
            _level0.createTextField("textout", 99, 10, 10, 600, 0);
	    _level0.textout.autoSize = true;
            _level0.dejagnu_module_initialized = 1;
        }

// helper functions
#include "check.sc"

