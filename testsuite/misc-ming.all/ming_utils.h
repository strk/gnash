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

#ifndef GNASH_MING_UTILS_H
#define GNASH_MING_UTILS_H

#include <ming.h>

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
 *
 * @param expected_failure
 *   Set to 1 if a failure is expected
 */
void check(SWFMovie mo, const char* expr, int expected_failure);

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
 * @param expected_failure
 *   Set to 1 if a failure is expected
 */
void check_equals(SWFMovie mo, const char* obtained, const char* expected, int expected_failure);

/** \brief
 * Print TestState total summary.
 * (make sure you called add_dejagnu_functions before using this function)
 *
 * @param mo
 *   The SWFMovie to add the DO_ACTION block to
 */
void print_tests_summary(SWFMovie mo);

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


/** \brief
 *  Create an outline square shape with given offset, size and colors
 */
SWFShape make_square(int x, int y, int width, int height, byte r, byte g, byte b);

/** \brief
 *  Create a filled square shape with given offset, size and colors
 */
SWFShape make_fill_square(int x, int y, int width, int height, byte outline_r, byte outline_g, byte outline_b, byte fill_r, byte fill_g, byte fill_b);

#endif // GNASH_MING_UTILS_H
