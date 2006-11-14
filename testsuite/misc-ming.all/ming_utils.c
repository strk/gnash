/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
 *
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>
#include <ming_utils.h>

void
add_xtrace_function(SWFMovie mo, int depth, int x, int y, int width, int height)
{
	SWFAction ac;

	static const size_t buflen = 1024;

	char buf[buflen];

	snprintf(buf, buflen, "createTextField(\"out\", %d, %d, %d, %d, %d); "
		" xtrace = function (msg) { "
		" trace (msg); "
		" _level0.out.text += msg + '\n'; "
		"};",
		depth, x, y, width, height);
	buf[buflen-1] = '\0';
	ac = compileSWFActionCode(buf);

	SWFMovie_add(mo, (SWFBlock)ac);
}

SWFShape
make_square(int x, int y, int width, int height, byte r, byte g, byte b)
{
	SWFShape sh = newSWFShape();
	SWFShape_setLineStyle(sh, 1, r, g, b, 255);
	SWFShape_movePenTo(sh, x, y);
	SWFShape_drawLineTo(sh, x, y+height);
	SWFShape_drawLineTo(sh, x+width, y+height);
	SWFShape_drawLineTo(sh, x+width, y);
	SWFShape_drawLineTo(sh, x, y);

	return sh;
}

SWFShape
make_fill_square(int x, int y, int width, int height, byte or, byte og, byte ob, byte fr, byte fg, byte fb)
{
	SWFShape sh = newSWFShape();
	SWFFillStyle fs = SWFShape_addSolidFillStyle(sh, fr, fg, fb, 255);
	SWFShape_setLineStyle(sh, 1, or, og, ob, 255);
	SWFShape_setLeftFillStyle(sh, fs);
	SWFShape_movePenTo(sh, x, y);
	SWFShape_drawLineTo(sh, x, y+height);
	SWFShape_drawLineTo(sh, x+width, y+height);
	SWFShape_drawLineTo(sh, x+width, y);
	SWFShape_drawLineTo(sh, x, y);

	return sh;
}

void
add_dejagnu_functions(SWFMovie mo,
	int depth, int x, int y, int width, int height)
{
	SWFAction ac;

	add_xtrace_function(mo, depth, x, y, width, height);

	static const size_t BUFLEN = 2048;

	char buf[BUFLEN];
	snprintf(buf, BUFLEN,
		"function TestState() {\n"
		" this.passed = 0;\n"
		" this.failed = 0;\n"
		" this.untest = 0;\n"
		" this.unresolve = 0;\n"
		"};\n"
		"TestState.prototype.fail = function (why) {\n"
		" this.failed++;\n"
		" xtrace('FAILED: '+why);\n"
		"};\n"
		"TestState.prototype.xfail = function(why) {\n"
		" this.failed++;\n"
		" xtrace('XFAILED: '+why);\n"
		"};\n"
		"TestState.prototype.pass = function(why) {\n"
		" this.passed++;\n"
		" xtrace('PASSED: '+why);\n"
		"};\n"
		"TestState.prototype.xpass = function(why) {\n"
		" this.xpassed++;\n"
		" xtrace('XPASSED: '+why);\n"
		"};\n"
		"TestState.prototype.totals = function() {\n"
		" xtrace('#passed: '+ this.passed);\n"
		" xtrace('#failed: '+ this.failed);\n"
		" if ( this.xpassed ) {\n"
		"   xtrace('#unexpected successes: '+ this.xpassed);\n"
		" }\n"
		" if ( this.xfailed ) {\n"
		"   xtrace('#expected failures: '+ this.xfailed);\n"
		" }\n"
		"};\n"
		"runtest = new TestState();\n"
		"function check_equals(obt, exp) {\n"
		" if ( obt == exp ) runtest.pass(obt+' == '+exp);\n"
		" else runtest.fail('expected: '+exp+' , obtained: '+obt);\n"
		"}\n"
		"function xcheck_equals(obt, exp) {\n"
		" if ( obt == exp ) runtest.xpass(obt+' == '+exp);\n"
		" else runtest.xfail('expected: '+exp+' , obtained: '+obt);\n"
		"}\n"
		"function check(a) {\n"
		" if ( a ) runtest.pass(a);\n"
		" else runtest.fail(a);\n"
		"}\n"
		"function xcheck(a) {\n"
		" if ( a ) runtest.xpass(a);\n"
		" else runtest.xfail(a);\n"
		"}\n"
	);

	/*printf("%s", buf);*/

	ac = compileSWFActionCode(buf);

	SWFMovie_add(mo, (SWFBlock)ac);
}
