/* 
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *
 */ 

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>
#include <stdarg.h>

void add_xtrace_function_clip(SWFMovieClip mo, SWFBlock font, int depth, int x, int y, int width, int height);
static SWFAction get_dejagnu_actions(void);

static const char* asciichars = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{};:.>,</?'\"\\|`~\t";

void
add_xtrace_function_clip(SWFMovieClip mc, SWFBlock font, int depth, int x, int y, int width, int height)
{
	SWFTextField tf;
	SWFDisplayItem it;
	int flags;

	tf = newSWFTextField();

	SWFTextField_setFont(tf, font);

	/* setting flags seem unneeded */
	flags = 0;
	//flags |= SWFTEXTFIELD_USEFONT;
	flags |= SWFTEXTFIELD_WORDWRAP;
	//flags |= SWFTEXTFIELD_NOEDIT;
	SWFTextField_setFlags(tf, flags);

	/* Add all ascii chars */
	SWFTextField_addChars(tf, asciichars);
	SWFTextField_addString(tf, " - xtrace enabled -\n");

	SWFTextField_setBounds(tf, width, height);

	/*
	 * Hopefully we have a *single* _root.
	 */
	SWFTextField_setVariableName(tf, "_root._trace_text");

	/*
	 * Set flags explicitly so that the field is selectable
	 * and you can cut&paste results.
	 * (the default seems to include SWFTEXTFIELD_NOSELECT)
	 */
	SWFTextField_setFlags(tf, SWFTEXTFIELD_NOEDIT);

	/*SWFTextField_setHeight(tf, 240);*/
	/*SWFTextField_setColor(tf, 0x00, 0x00, 0x00, 0xff);*/
	/*SWFTextField_setAlignment(tf, SWFTEXTFIELD_ALIGN_LEFT);*/
	/*SWFTextField_setLeftMargin(tf, 0);*/
	/*SWFTextField_setRightMargin(tf, 0);*/
	/*SWFTextField_setIndentation(tf, 0);*/
	/*SWFTextField_setLineSpacing(tf, 40);*/
	/*SWFTextField_setLineSpacing(tf, 40);*/

	it = SWFMovieClip_add(mc, (SWFBlock)tf);
	SWFDisplayItem_moveTo(it, x, y);
	SWFDisplayItem_setDepth(it, depth);
	SWFDisplayItem_setName(it, "_xtrace_win");

	add_clip_actions(mc,
		" _root.xtrace = function (msg) { "
		" _root._trace_text += msg + '\n'; "
		"};");

}

void add_xtrace_function(SWFMovie mo, SWFBlock font, int depth, int x, int y, int width, int height);

void
add_xtrace_function(SWFMovie mo, SWFBlock font, int depth, int x, int y, int width, int height)
{
	SWFTextField tf;
	SWFDisplayItem it;

	tf = newSWFTextField();

	SWFTextField_setFont(tf, font);

	/* setting flags seem unneeded */
	/*SWFTextField_setFlags(tf, SWFTEXTFIELD_USEFONT|SWFTEXTFIELD_NOEDIT);*/

	/* Add all ascii chars */
	SWFTextField_addChars(tf, asciichars);
	SWFTextField_addString(tf, " - xtrace enabled -\n");

	SWFTextField_setBounds(tf, width, height);

	/*
	 * Hopefully we have a *single* _root.
	 */
	SWFTextField_setVariableName(tf, "_root._trace_text");

	/*SWFTextField_setHeight(tf, 240);*/
	/*SWFTextField_setColor(tf, 0x00, 0x00, 0x00, 0xff);*/
	/*SWFTextField_setAlignment(tf, SWFTEXTFIELD_ALIGN_LEFT);*/
	/*SWFTextField_setLeftMargin(tf, 0);*/
	/*SWFTextField_setRightMargin(tf, 0);*/
	/*SWFTextField_setIndentation(tf, 0);*/
	/*SWFTextField_setLineSpacing(tf, 40);*/
	/*SWFTextField_setLineSpacing(tf, 40);*/

	it = SWFMovie_add(mo, (SWFBlock)tf);
	SWFDisplayItem_moveTo(it, x, y);
	SWFDisplayItem_setDepth(it, depth);
	SWFDisplayItem_setName(it, "_xtrace_win");

	add_actions(mo,
		" _root.xtrace = function (msg) { "
		" trace (msg); "
		" _root._trace_text += msg + '\n'; "
		"};");
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

static SWFAction
get_dejagnu_actions()
{
	static const char *buf = 
		"TestState = function() {\n"
		" this.passed = 0;\n"
		" this.failed = 0;\n"
		" this.xpassed = 0;\n"
		" this.xfailed = 0;\n"
		" this.untest = 0;\n"
		" this.unresolve = 0;\n"
		"};\n"
		"TestState.prototype.note = function (msg) {\n"
		" _root.xtrace(msg);\n"
		" trace (msg); "
		"};\n"
		"TestState.prototype.fail = function (why) {\n"
		" this.failed++;\n"
		" var msg = 'FAILED: '+why;\n"
		" _root.xtrace(msg);\n"
		" trace (msg); "
		"};\n"
		"TestState.prototype.xfail = function(why) {\n"
		" this.xfailed++;\n"
		" var msg = 'XFAILED: '+why;\n"
		" _root.xtrace(msg);\n"
		" trace (msg); "
		"};\n"
		"TestState.prototype.pass = function(why) {\n"
		" this.passed++;\n"
		" var msg = 'PASSED: '+why;\n"
		// don't visually print successes, still use 'trace' for them
		// " _root.xtrace(msg);\n"
		" trace (msg); "
		"};\n"
		"TestState.prototype.xpass = function(why) {\n"
		" this.xpassed++;\n"
		" var msg = 'XPASSED: '+why;\n"
		// don't visually print successes, even if unexpected,
		// still use 'trace' for them
		// " _root.xtrace(msg);\n"
		" trace (msg); "
		"};\n"
		"TestState.prototype.printtotals = function() {\n"
		" this.note('#passed: '+ this.passed);\n"
		" this.note('#failed: '+ this.failed);\n"
		" if ( this.xpassed ) {\n"
		"   this.note('#unexpected successes: '+ this.xpassed);\n"
		" }\n"
		" if ( this.xfailed ) {\n"
		"   this.note('#expected failures: '+ this.xfailed);\n"
		" }\n"
		" this.note('#total tests run: '+ this.testcount());\n"
		"};\n"
		"TestState.prototype.totals = function(exp, msg) {\n"
		" var obt = this.testcount(); "
		" if ( exp != undefined && obt != exp ) { "
		"   this.fail('Tests run '+obt+' (expected '+exp+') ['+msg+']'); "
		" }"
		" this.printtotals();"
		"};\n"
		"TestState.prototype.xtotals = function(exp, msg) {\n"
		" var obt = this.testcount(); "
		" if ( exp != undefined && obt != exp ) { "
		"   this.xfail('Tests run '+obt+' (expected '+exp+') ['+msg+']'); "
		" } else {"
		"   this.xpass('Tests run: '+obt+' ['+msg+']'); "
		" }"
		" this.printtotals();"
		"};\n"
		"TestState.prototype.testcount = function() {\n"
		" c=this.passed;\n"
		" c+=this.failed;\n"
		" if ( this.xpassed ) c+=this.xpassed;\n"
		" if ( this.xfailed ) c+=this.xfailed;\n"
		" return c;"
		"};\n"

		"_root.runtest = new TestState();\n"

		"_root.check_equals = function(obt, exp, msg) {"
		" if ( obt == exp ) "
        " {"
        "   if ( msg != undefined ) _root.runtest.pass(obt+' == '+exp+' ('+msg+')');"
        "   else _root.runtest.pass(obt+' == '+exp);"
        " }"
		" else "
        " {"
        "   if ( msg != undefined )"
        "   {"
        "       _root.runtest.fail('expected: '+exp+' , obtained: '+obt+' ('+msg+')');"
        "   }"
        "   else _root.runtest.fail('expected: '+exp+' , obtained: '+obt);"
        " }"
		"};\n"

		"_root.xcheck_equals = function(obt, exp) {"
		" if ( obt == exp )"
        " {"
        "   if ( msg != undefined )"
        "   {"
        "     _root.runtest.xpass(obt+' == '+exp+' ('+msg+')');\n"
        "   }"
        "   else"
        "   {"
        "     _root.runtest.xpass(obt+' == '+exp);\n"
        "   }"
        " }"
		" else"
        " {"
        "   if ( msg != undefined )"
        "   {"
        "     _root.runtest.xfail('expected: '+exp+' , obtained: '+obt);\n"
        "   }"
        "   else"
        "   {"
        "     _root.runtest.xfail('expected: '+exp+' , obtained: '+obt);\n"
        "   }"
        " }"
		"};\n"

		"_root.check = function(a, msg) {\n"
		" if ( a ) _root.runtest.pass(msg != undefined ? msg : a);\n"
		" else _root.runtest.fail(msg != undefined ? msg : a);\n"
		"};\n"

		"_root.xcheck = function(a, msg) {\n"
		" if ( a ) _root.runtest.xpass(msg != undefined ? msg : a);\n"
		" else _root.runtest.xfail(msg != undefined ? msg : a);\n"
		"};\n"

		"_root.fail = function(msg) {\n"
		" _root.runtest.fail(msg);\n"
		"};\n"

		"_root.xfail = function(msg) {\n"
		" _root.runtest.xfail(msg);\n"
		"};\n"

		"_root.pass = function(msg) {\n"
		" _root.runtest.pass(msg);\n"
		"};\n"

		"_root.xpass = function(msg) {\n"
		" _root.runtest.xpass(msg);\n"
		"};\n"

		"_root.note = function(msg) {\n"
		" _root.xtrace(msg);\n"
		" trace(msg);\n"
		"};\n"

		"_root.totals = function(exp, info) {\n"
		" _root.runtest.totals(exp, info);\n"
		"};\n"

		"_root.xtotals = function(exp, info) {\n"
		" _root.runtest.xtotals(exp, info);\n"
		"};\n"

		"_root.dejagnu_module_initialized = 1;\n";

	return compileSWFActionCode(buf);
}

SWFMovieClip
get_dejagnu_clip(SWFBlock font, int depth, int x, int y, int width, int height)
{
	SWFMovieClip mc = newSWFMovieClip();
	SWFAction ac = get_dejagnu_actions();

	add_xtrace_function_clip(mc, font, depth, x, y, width, height);

	SWFMovieClip_add(mc, (SWFBlock)ac);

	SWFMovieClip_nextFrame(mc);

	return mc;
}

void
add_dejagnu_functions(SWFMovie mo, SWFBlock font,
	int depth, int x, int y, int width, int height)
{
	SWFAction ac = get_dejagnu_actions();

	add_xtrace_function(mo, font, depth, x, y, width, height);

	SWFMovie_add(mo, (SWFBlock)ac);
}

void
add_clip_actions(SWFMovieClip mo, const char* code)
{
	SWFAction ac;
	ac = compileSWFActionCode(code);
	SWFMovieClip_add(mo, (SWFBlock)ac);
}

#ifdef MING_SUPPORTS_INIT_ACTIONS
void
add_clip_init_actions(SWFMovieClip mo, const char* code)
{
	SWFAction ac;
	ac = compileSWFActionCode(code);
	SWFMovieClip_addInitAction(mo, ac);
}
#endif // MING_SUPPORTS_INIT_ACTIONS

SWFAction 
compile_actions(const char* fmt, ...)
{
	SWFAction ac;
	size_t BUFFER_SIZE = 65535;
	va_list ap;
	char tmp[BUFFER_SIZE];

	va_start (ap, fmt);
	vsnprintf (tmp, BUFFER_SIZE, fmt, ap);
	tmp[BUFFER_SIZE-1] = '\0';

	ac = compileSWFActionCode(tmp);
	return ac;
}

void
add_actions(SWFMovie mo, const char* code)
{
	SWFAction ac;
	ac = compileSWFActionCode(code);
	SWFMovie_add(mo, (SWFBlock)ac);
}

void
print_tests_summary(SWFMovie mo)
{
	add_actions(mo, "runtest.totals();");
}

SWFFont
get_default_font(const char* mediadir)
{
	FILE *font_file;
	char fdbfont[256];

	sprintf(fdbfont, "%s/Bitstream-Vera-Sans.fdb", mediadir);

	font_file = fopen(fdbfont, "r");
	if ( font_file == NULL )
	{
		perror(fdbfont);
		exit(1);
	}
	return loadSWFFontFromFile(font_file);
}


