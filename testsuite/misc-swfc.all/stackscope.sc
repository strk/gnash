/*
 *   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
 * Sandro Santilli, strk@keybit.net
 * 
 * Test ActionScript stack lifetime
 *
 * Description:
 * 
 *   First DoAction in frame1: push variable and value on stack
 *  Second DoAction in frame1: setvariable, push variable and value on stack
 *  DoAction in placed movieclip frame1: setvariable
 *
 * Expected behaviour:
 *  Stack is NOT cleared between DoAction blocks nor it is between
 *  execution of code in root movie and code in child.
 *
 * 
 */

.flash  bbox=800x600 filename="stackscope.swf" background=white version=6 fps=12

.frame 1

  .action:
   #include "Dejagnu.sc"

	trace("doaction1");
    	asm {
		push '_root.var1'
		push 'val1'
	};
  .end
  .action:
	trace("doaction2");
    	asm {
		setvariable
		push '_root.var2'
		push 'val2'
	};
  .end

  .sprite mc1 
    .frame 1
      .action:
	trace("mc1");
        asm {
		setvariable
	};
      .end
  .end

  .put mc1

.frame 2
  .action:
    xcheck_equals(var1, 'val1');
    xcheck_equals(var2, 'val2');
    totals(2);
    stop();
  .end
.end
