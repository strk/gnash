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
 *  test opcodes defined in swf4
 */

//
// Dejagnu clip does not work/compile in swf4.
//

#define pass_check() { trace("PASSED: "); trace(__FILE__); }
#define xpass_check() { trace("XPASSED: "); trace(__FILE__); }
#define fail_check() { trace("FAILED: " ); trace(__FILE__); }
#define xfail_check(){ trace("XFAILED: "); trace(__FILE__); }


//
// Use check_equals(<obtained>, <expected>)
//
#define check_equals(obt, exp)  \
    if ( obt == exp ) pass_check() \
    else fail_check()
    
#define xcheck_equals(obt, exp)  \
        if ( obt == exp ) xpass_check() \
        else xfail_check()
        

.flash  bbox=800x600 filename="swf4opcode.swf" background=white version=4 fps=12

.frame 1
    .action:
        testvar = (uninitialized1 == '');
        xcheck_equals(testvar, 1);
        testvar = ('' == uninitialized2);
        xcheck_equals(testvar, 1); 
        testvar = ('' == '');  
        xcheck_equals(testvar, 1); 
        testvar = ('xyz' == 'abc');
        //Ref: http://swishtutor.com
        // both hands are converted to zero
        xcheck_equals(testvar, 1); 
        xcheck_equals('xyz', 0);
        xcheck_equals('abc', 0);
        
        // test 'undefined' in swf4
        check_equals(uninitialized2, uninitialized3);
        check_equals(uninitialized2, 0);
        check_equals(undefined, 0);
        check_equals(0, undefined);
        check_equals(undefined, undefined);
    .end

.frame 3
    .action:
        stop();
    .end
    
.end

