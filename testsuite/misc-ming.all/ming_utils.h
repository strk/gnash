/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
 *   2011 Free Software Foundation, Inc.
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
* 
* You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

#ifndef GNASH_MING_UTILS_H
#define GNASH_MING_UTILS_H

#include <ming.h>

#if MING_VERSION_CODE >= 00040004
# define MING_SUPPORTS_INIT_ACTIONS
#endif

/*
 * Add symbol from ming src/movie.h as that header file is missing
 * from the ming -dev package.
 */
void SWFMovie_writeExports(SWFMovie movie);

/*
 * This is to avoid the annoying warnings
 * coming from Ming when using the deprecated
 * compileSWFActionCode interface.
 * A cleaner approach is likely switch to
 * using newSWFAction always and change the
 * macro to make it output compileSWFActionCode
 * when MING_VERSION_CODE < 000040004
 */
#if MING_VERSION_CODE >= 00040004
# define compileSWFActionCode newSWFAction
#else
# define newSWFAction compileSWFActionCode
#endif

/*
 * 'callFrame' was drop as a keyword since Ming-0.4.0.beta5
 * and replaced by 'call'. Before that version 'call' would
 * never be recognized as a "callframe" action
 */
#if MING_VERSION_CODE >= 00040006
# define CALLFRAME "call"
#else
# define CALLFRAME "callFrame"
#endif


/* Missing define to allow using older Ming releases */

#ifndef SWFACTION_INIT
# define SWFACTION_INIT       (1<<9)
#endif

#ifndef SWFACTION_PRESS
# define SWFACTION_PRESS              (1<<10)
#endif

#ifndef SWFACTION_RELEASE
# define SWFACTION_RELEASE     (1<<11)
#endif

#ifndef SWFACTION_RELEASEOUTSIDE
# define SWFACTION_RELEASEOUTSIDE (1<<12)
#endif

#ifndef SWFACTION_ROLLOVER
# define SWFACTION_ROLLOVER    (1<<13)
#endif

#ifndef SWFACTION_ROLLOUT
# define SWFACTION_ROLLOUT     (1<<14)
#endif

#ifndef SWFACTION_DRAGOVER
# define SWFACTION_DRAGOVER    (1<<15)
#endif

#ifndef SWFACTION_DRAGOUT
# define SWFACTION_DRAGOUT     (1<<16)
#endif

#ifndef SWFACTION_KEYPRESS
# define SWFACTION_KEYPRESS    (1<<17)
#endif

#ifndef SWFACTION_CONSTRUCT
# define SWFACTION_CONSTRUCT   (1<<18)
#endif


/** \brief
 * Get the default font for Gnash testcases.
 * 
 * @param mediadir
 * 	the 'media' directory under testsuite/ dir of
 *	Gnash source tree.
 */
SWFFont get_default_font(const char* mediadir);

/** \brief
 * Add 'check', 'xcheck', 'check_equals', 'xcheck_equals' ActionScript
 * functions for use by embedded-swf tests, and a textfield to print
 * results of the checks to (results will additionally be 'traced').
 * The textfield uses embedded fonts (only ascii chars loaded).
 *
 * Note that the x, y, width and height parameters will depend on
 * the currently set Ming scale (see Ming_setScale). By default
 * they are pixels (twips*20).
 */
void add_dejagnu_functions(SWFMovie mo, SWFBlock font, int depth, int x, int y, int width, int height);

/** \brief
 * Return a 'dejagnu' clip. This is like add_dejagnu_functions but
 * embeds the functionalities in a movieclip, ready for export.
 *
 * The Dejagnu.c file uses this function to create a Dejagnu.swf
 * file that exports a 'dejagnu' symbol.
 * The architecture still needs a bit of tuning for general use (the goal
 * is making it easy for flash coders to produce standard testcases), anyway
 *
 * A quick test revealed that it is possible, with an SWF targeted
 * at version 5, to 'import' the Dejagnu.swf file and use it's functionalities.
 *
 * For importing it using the command-line actionscript compiler:
 *
 * makeswf -o test.swf -v5 -iDejagnu.swf:dejagnu 0.as test.as
 *
 * Note that the '0.as' is just a placeholder to have a first frame
 * with no actions. This is needed because actions in the main movie
 * (the "importer") are executed *before* actions in the loaded movie
 * (the "exported": Dejagnu.swf). So, in order to use functions defined
 * in the "imported" movie we have to wait the second frame.
 *
 */
SWFMovieClip get_dejagnu_clip(SWFBlock font, int depth, int x, int y, int width, int height);

/** \brief
 * Evaluate ActionScript 'expr' expression updating the global TestState
 * (make sure you called add_dejagnu_functions before using this function)
 *
 * @param mo
 *   The SWFMovie to add the DO_ACTION block to
 *
 * @param expr
 *   The ActionScript expression
 */
#define check(m, expr)  \
	SWFMovie_add(m, (SWFBlock)compile_actions("\
		if ( %s ) pass( \"%s [%s:%d]\"); \
		else fail( \"%s [%s:%d] \"); \
		", expr, expr, __FILE__, __LINE__, expr, __FILE__, __LINE__));

/** \brief
 * Evaluate ActionScript 'expr' expression updating the global TestState.
 * Expect a failure.
 * (make sure you called add_dejagnu_functions before using this function)
 *
 * @param mo
 *   The SWFMovie to add the DO_ACTION block to
 *
 * @param expr
 *   The ActionScript expression
 */
#define xcheck(m, expr)  \
	SWFMovie_add(m, (SWFBlock)compile_actions("\
		if ( %s ) xpass( \"%s [%s:%d]\"); \
		else xfail( \"%s [%s:%d] \"); \
		", expr, expr, __FILE__, __LINE__, expr, __FILE__, __LINE__));


/** \brief
 * Evaluate equality of two ActionScript expressions updating the global
 * TestState accordingly.
 * (make sure you called add_dejagnu_functions before using this function)
 *
 * @param mo
 *   The SWFMovie to add the DO_ACTION block to
 *
 * @param obtained
 *   The ActionScript expression we are testing
 *
 * @param expected
 *   The ActionScript expression we expect to equal the obtained one
 *
 */
#define check_equals(m, obt, exp)  \
	SWFMovie_add(m, (SWFBlock)compile_actions("\
		if ( %s == %s ) pass( \"%s  ==  %s [%s:%d]\"); \
		else fail( \"expected: %s obtained: \" + %s + \" [%s:%d] \"); \
		", obt, exp, obt, exp, __FILE__, __LINE__, exp, obt, __FILE__, __LINE__));

/** \brief
 * Evaluate equality of two ActionScript expressions updating the global
 * TestState accordingly. Expect a failure.
 * (make sure you called add_dejagnu_functions before using this function)
 *
 * @param mo
 *   The SWFMovie to add the DO_ACTION block to
 *
 * @param obtained
 *   The ActionScript expression we are testing
 *
 * @param expected
 *   The ActionScript expression we expect to equal the obtained one
 *
 */
#define xcheck_equals(m, obt, exp)  \
	SWFMovie_add(m, (SWFBlock)compile_actions("\
		if ( %s == %s ) xpass( \"%s  ==  %s [%s:%d]\"); \
		else xfail( \"expected: %s obtained: \" + %s + \" [%s:%d] \"); \
		", obt, exp, obt, exp, __FILE__, __LINE__, exp, obt, __FILE__, __LINE__));



/** \brief
 * Print TestState total summary.
 * (make sure you called add_dejagnu_functions before using this function)
 *
 * @param mo
 *   The SWFMovie to add the DO_ACTION block to
 */
void print_tests_summary(SWFMovie mo);

/** \brief
 * Compile ActionScript code using printf-like formatting
 */
SWFAction compile_actions(const char* fmt, ...);

/** \brief
 * Add an arbitrary ActionScript code in the given movie
 *
 * @param mo
 *   The SWFMovie to add the DO_ACTION block to.
 *
 * @param code
 *   ActionScript code to be compiled in.
 */
void add_actions(SWFMovie mo, const char* code);

/** \brief
 * Add an arbitrary ActionScript code in the given movieclip
 *
 * @param mc
 *   The SWFMovieClip to add the DO_ACTION block to.
 *
 * @param code
 *   ActionScript code to be compiled in.
 */
void add_clip_actions(SWFMovieClip mc, const char* code);


#ifdef MING_SUPPORTS_INIT_ACTIONS
/** \brief
 * Add an Init ActionScript code in the given movieclip
 *
 * @param mc
 *   The SWFMovieClip to add the DO_INITACTION block to.
 *
 * @param code
 *   Init ActionScript code to be compiled in.
 */
void add_clip_init_actions(SWFMovieClip mo, const char* code);
#endif // MING_SUPPORTS_INIT_ACTIONS

/** \brief
 *  Create an outline square shape with given offset, size and colors
 */
SWFShape make_square(int x, int y, int width, int height, byte r, byte g, byte b);

/** \brief
 *  Create a filled square shape with given offset, size and colors
 */
SWFShape make_fill_square(int x, int y, int width, int height, byte outline_r, byte outline_g, byte outline_b, byte fill_r, byte fill_g, byte fill_b);

#endif // GNASH_MING_UTILS_H
