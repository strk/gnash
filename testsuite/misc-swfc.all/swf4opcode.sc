/*
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
 */

//
// Dejagnu clip does not work/compile in swf4.
//

#define _INFO_ concat(' [', concat(__FILE__, concat(':', concat(__LINE__,']'))))

#define pass_check() { trace(concat("PASSED: ", _INFO_)); }
#define xpass_check() { trace(concat("XPASSED: ", _INFO_)); }
#define fail_check() { trace(concat("FAILED: ", _INFO_)); }
#define xfail_check(){ trace(concat("XFAILED: ", _INFO_)); }


//
// Use check_equals(<obtained>, <expected>)
//
#define check_equals(obt, exp)  \
    if ( obt == exp ) pass_check() \
    else fail_check()
    
#define xcheck_equals(obt, exp)  \
        if ( obt == exp ) xpass_check() \
        else xfail_check()
        
#define check(expr)  \
    if ( expr ) pass_check() \
    else fail_check()

#define xcheck(expr)  \
        if ( expr ) xpass_check() \
        else xfail_check() 
    
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
        xcheck_equals( y, 4 ); 
    // exponent       
    y = '4.5e4';
        xcheck_equals( y, 45000 );
    y = '4.5E4';
        xcheck_equals( y, 45000 );
    y = '+4.5e4';
        xcheck_equals( y, 45000 );
    y = '-4.5e4';
        xcheck_equals( y, -45000 );
    y = '4.5e+4';
        xcheck_equals( y, 45000 );
    y = '4.5e-4';
        xcheck_equals( y, 0.00045 );
    y = '-4.5e-4';
        xcheck_equals( y, -0.00045 );
	x = '2e1';
        check_equals(x+1, 21);
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
    .action:
        stop();
    .end
    
.end

