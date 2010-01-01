/*
 *   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

/*
 *  Zou Lunkai, zoulunkai@gmail.com
 *
 *  test opcodes defined in swf4.
 *  
 *  Deduction:
 *
 *     There is no NaN number in swf4 at all, invalid numbers are converted to 0.
 *
 * 2008-11-25 UPDATE: the deduction is wrong, as shown by this:
 *     /mc1.xscale = "0"; // make xscale=0
 *     /mc1.xscale = 100; // make xscale=100
 *     /mc1.xscale = "N"; // doesn't change xscale (still 100)
 * --strk;
 *    
 */

//
// Dejagnu clip does not work/compile in swf4.
//

//-------------------------------------------------------------------------------------------
// Dejagnu-like interface for SWF4
// TODO: expose in check.as based on OUTPUT_VERSION ?
//-------------------------------------------------------------------------------------------

#define _INFO_ concat(' [', concat(__FILE__, concat(':', concat(__LINE__,']'))))

#define pass_check(lbl) { trace(concat("PASSED: ", concat(lbl, _INFO_))); }
#define xpass_check(lbl) { trace(concat("XPASSED: ", concat(lbl, _INFO_))); }
#define fail_check(lbl) { trace(concat("FAILED: ", concat(lbl, _INFO_))); }
#define xfail_check(lbl) { trace(concat("XFAILED: ", concat(lbl, _INFO_))); }


//
// Use check_equals(<obtained>, <expected>)
//
#define check_equals(obt, exp)  \
    if ( obt == exp ) pass_check( concat(#obt, concat(" == ", #exp)) ) \
    else fail_check( concat("expected: ", concat(#exp, concat(" obtained: ", obt))) )
    
#define xcheck_equals(obt, exp)  \
    if ( obt == exp ) xpass_check( concat(#obt, concat(" == ", #exp)) ) \
    else xfail_check( concat("expected: ", concat(#exp, concat(" obtained: ", obt))) )
        
#define check(expr)  \
    if ( expr ) pass_check(#expr) \
    else fail_check(#expr)

#define xcheck(expr)  \
        if ( expr ) xpass_check(#expr) \
        else xfail_check(#expr) 

//-------------------------------------------------------------------------------------------
    
.flash  bbox=800x600 filename="swf4opcode.swf" background=white version=4 fps=12

.frame 1
    .action:
        // 
        //  test opcode ActionEquals
        //
        testvar = (uninitialized1 == '');
        check_equals(testvar, 1);
        testvar = ('' == uninitialized2);
        check_equals(testvar, 1); 
        testvar = ('' == '');  
        check_equals(testvar, 1); 
        testvar = ('xyz' == 'abc');
        //Ref: http://swishtutor.com
        // both hands are converted to zero
        check_equals(testvar, 1); 
        check_equals('xyz', 0);
        check_equals('abc', 0);
        check_equals('xyz', 'xyz');
        check_equals('xyz', 'abc');
        
        // test 'undefined' in swf4
        check_equals(uninitialized2, uninitialized3);
        check_equals(uninitialized2, 0);
        check_equals(undefined, 0);
        check_equals(0, undefined);
        check_equals(undefined, undefined);
        
        // test 'Infinity' in swf4
        // there's no 'Infinity' constant in swf4
        check_equals(Infinity, undefined);
        check_equals(Infinity, Infinity);
        check_equals(Infinity, -Infinity);

        // test 'null' in swf4
        // there's no null in swf4
        check_equals(null, undefined);
        check_equals(null, 0);
        
        // test 'NaN' in swf4
        // there's no 'NaN' constant in swf4
        check_equals(NaN, 0);
    .end


.frame 2
    .action:

        //
        // test convertion to number (and thus bool)
        //
        x = '2/';
        y = '3/';
        // x and y are converted to number 0 before comparision
        check_equals( (x+y), 5 );
        check( y > x );
        neg = !x;
	check(!neg);
        neg = !y;
	check(!neg);
	y = '/';
        check_equals( y, 0 );
	y = '  4';
        check_equals( y, 4 );
    // This needs to work in all locales
    y = '4.5';
        check_equals( y, 4.5 );
    y = '4,5';
        check_equals( y, 4 ); 
    // exponent       
    y = '4.5e4';
        check_equals( y, 45000 );
    y = '4.5E4';
        check_equals( y, 45000 );
    y = '+4.5e4';
        check_equals( y, 45000 );
    y = '-4.5e4';
        check_equals( y, -45000 );
    y = '4.5e+4';
        check_equals( y, 45000 );
    y = '4.5e-4';
        check_equals( y, 0.00045 );
    y = '-4.5e-4';
        check_equals( y, -0.00045 );
	x = '2e1';
        check_equals(x+1, 21);
        //
        // test ActionLogicalNot (0x12)
        //
        check(!"");
        check(!"a");
        check(!"true");
        check(!"false");
        check("1");
        check(!"0000.000");
        check(!false); // doh !
        check(true); // doh !
        check(!0); 
        check(4); 
        check(!_root);  // undefined ?
        check(!_level0);  // undefined ?
        check(!null); 
        check(!undefined); 

        //
        // test ActionLessThan
        //
        x = 'ab';
        y = 'abc';
        // should return 0(false)
        // x and y are converted to number 0 before comparision
        check( ! (x < y) );
        check( ! (x > y) );
        check( x == y );
        check( x == 0);
        
        //
        // test swf4 ActionMultiply, ActionDivide, ActionAdd, ActionSubstract
        //
        x = "abc";
        y = 0;
        z = x * y;
        check_equals(z, 0);
        z = x / 1;
        check_equals(z, 0);
        z = x + 1;
        check_equals(z, 1);
        z = x - 1;
        check_equals(z, -1);
        
        //
        // TODO: add tests for ActionStringEq, ActionStringGreater,
        // ActionStringCompare
        //
        // Question: how to generate the above opcodes?
    .end
    
.frame 3

    .box b1 color=white fill=green width=50 height=50 
    .sprite mc1
        .put shape1=b1 x=100 y=100
    .end
    .put mc1

    .action:
        //
        // Test assigning undefined/null to properties
        //

        // test with _x
        check_equals(/mc1:_x, undefined); // no variable, only property
        /mc1.x = 100;
        check_equals(/mc1.x, 100);
        /mc1.x=undefined; // evaluates to 0, not NaN !
        check_equals(/mc1.x, 100);
        /mc1.x=null; // evaluates to 0, not NaN !
        check_equals(/mc1.x, 100);
        /mc1.x=0; // this is a real zero
        check_equals(/mc1.x, 0);
        /mc1.x=100; // reset
        check_equals(/mc1.x, 100);
        s="0";
        /mc1.x=s; // evaluates to 0
        check_equals(/mc1.x, 0);
        /mc1.x=100; // reset
        s="not a number";
        /mc1.x=s; // evaluates to ? 
        xcheck_equals(/mc1.x, 100); // not a number it seems? 

        // test with _y
        check_equals(/mc1:_y, undefined); // no variable, only property
        /mc1.y = 100;
        check_equals(/mc1.y, 100);
        /mc1.y=undefined; // evaluates to 0, not NaN !
        check_equals(/mc1.y, 100);
        /mc1.y=null; // evaluates to 0, not NaN !
        check_equals(/mc1.y, 100);
        /mc1.y=0; // this is a real zero
        check_equals(/mc1.y, 0);
        /mc1.y=100; // reset
        check_equals(/mc1.y, 100);
        s="0";
        /mc1.y=s; // evaluates to 0
        check_equals(/mc1.y, 0);
        /mc1.y=100; // reset
        s="not a number";
        /mc1.y=s; // evaluates to ? 
        xcheck_equals(/mc1.y, 100); // not a number it seems? 

        // test with _xscale
        xcheck_equals(/mc1:_xscale, undefined); // no variable, only property
        check_equals(/mc1.xscale, 100);
        /mc1.xscale=undefined; // evaluates to 0, not NaN !
        check_equals(/mc1.xscale, 100);
        /mc1.xscale=null; // evaluates to 0, not NaN !
        check_equals(/mc1.xscale, 100);
        /mc1.xscale=0; // this is a real zero
        check_equals(/mc1.xscale, 0);
        /mc1.xscale=100; // reset
        check_equals(/mc1.xscale, 100);
        s="0";
        /mc1.xscale=s; // evaluates to 0
        check_equals(/mc1.xscale, 0);
        /mc1.xscale=100; // reset
        s="not a number";
        /mc1.xscale=s; // evaluates to ? 
        xcheck_equals(/mc1.xscale, 100); // not a number it seems? 
        /mc1.xscale=100; // reset

        // test with _yscale
        xcheck_equals(/mc1:_yscale, undefined); // no variable, only property
        check_equals(/mc1.yscale, 100);
        /mc1.yscale=undefined; // evaluates to 0, not NaN !
        check_equals(/mc1.yscale, 100);
        /mc1.yscale=null; // evaluates to 0, not NaN !
        check_equals(/mc1.yscale, 100);
        /mc1.yscale=0; // this is a real zero
        check_equals(/mc1.yscale, 0);
        /mc1.yscale=100; // reset
        check_equals(/mc1.yscale, 100);
        s="0";
        /mc1.yscale=s; // evaluates to 0
        check_equals(/mc1.yscale, 0);
        /mc1.yscale=100; // reset
        s="not a number";
        /mc1.yscale=s; // evaluates to ? 
        xcheck_equals(/mc1.yscale, 100); // not a number it seems? 
        /mc1.yscale=100; // reset

        // test with _alpha
        xcheck_equals(/mc1:_alpha, undefined); // no variable, only property
        check_equals(/mc1.alpha, 100);
        /mc1.alpha=undefined; // evaluates to 0, not NaN !
        check_equals(/mc1.alpha, 100);
        /mc1.alpha=null; // evaluates to 0, not NaN !
        check_equals(/mc1.alpha, 100);
        /mc1.alpha=0; // this is a real zero
        check_equals(/mc1.alpha, 0);
        /mc1.alpha=100; // reset
        check_equals(/mc1.alpha, 100);
        s="0";
        /mc1.alpha=s; // evaluates to 0
        check_equals(/mc1.alpha, 0);
        /mc1.alpha=100; // reset
        s="not a number";
        /mc1.alpha=s; // evaluates to ? 
        xcheck_equals(/mc1.alpha, 100); // not a number it seems? 
        /mc1.alpha=100; // reset

        // test with _visible
        xcheck_equals(/mc1:_visible, undefined); // no variable, only property
        check_equals(/mc1.visible, 1);
        /mc1.visible=undefined; // evaluates to 0, not NaN !
        check_equals(/mc1.visible, 1);
        /mc1.visible=null; // evaluates to 0, not NaN !
        check_equals(/mc1.visible, 1);
        /mc1.visible=0; // this is a real zero
        check_equals(/mc1.visible, 0);
        /mc1.visible=100; // reset
        check_equals(/mc1.visible, 1);
        s="0";
        /mc1.visible=s; // evaluates to 0
        check_equals(/mc1.visible, 0);
        /mc1.visible=100; // reset
        s="not a number";
        /mc1.visible=s; // evaluates to ? 
        xcheck_equals(/mc1.visible, 1); // not a number it seems? 
        /mc1.visible=100; // reset

        // test with _rotation
        check_equals(/mc1:_rotation, undefined); // no variable, only property
        /mc1.rotation = 90;
        check_equals(/mc1.rotation, 90);
        /mc1.rotation=undefined; // evaluates to 0, not NaN !
        check_equals(/mc1.rotation, 90);
        /mc1.rotation=null; // evaluates to 0, not NaN !
        check_equals(/mc1.rotation, 90);
        /mc1.rotation=0; // this is a real zero
        check_equals(/mc1.rotation, 0);
        /mc1.rotation=90; // reset
        check_equals(/mc1.rotation, 90);
        s="0";
        /mc1.rotation=s; // evaluates to 0
        check_equals(/mc1.rotation, 0);
        /mc1.rotation=90; // reset
        s="not a number";
        /mc1.rotation=s; // evaluates to ? 
        xcheck_equals(/mc1.rotation, 90); // not a number it seems? 
        /mc1.rotation=0; // reset

        // 'mc1' is undefined as a variable
        check_equals(mc1, undefined);
        check_equals(/mc1, undefined);
        check_equals(/:mc1, undefined);

        // You can define your own 'mc1' variable though
        mc1=54;
        check_equals(mc1, 54);

    .end
	
    
.frame 4
    .action:
        stop();
    .end
    
.end

