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

/// Add an 'xtrace' function that both traces usin 'trace' and
/// printing to a textfield created using the given depth, position and size
void add_xtrace_function(SWFMovie mo, int depth, int x, int y, int width, int height);

/// Add 'check', 'xcheck', 'check_equals', 'xcheck_equals' functions for
/// use by embedded-swf tests.
/// This function will internally call add_xtrace_function with the
/// given parameters
void add_dejagnu_functions(SWFMovie mo, int depth, int x, int y, int width, int height);

/// Create an outline square shape with given offset, size and colors
SWFShape make_square(int x, int y, int width, int height, byte r, byte g, byte b);

/// Create a filled square shape with given offset, size and colors
SWFShape make_fill_square(int x, int y, int width, int height, byte outline_r, byte outline_g, byte outline_b, byte fill_r, byte fill_g, byte fill_b);

#endif // GNASH_MING_UTILS_H
