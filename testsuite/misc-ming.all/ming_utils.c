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
 * Linking Gnash statically or dynamically with other modules is making a
 * combined work based on Gnash. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of Gnash give you
 * permission to combine Gnash with free software programs or libraries
 * that are released under the GNU LGPL and with code included in any
 * release of Talkback distributed by the Mozilla Foundation. You may
 * copy and distribute such a system following the terms of the GNU GPL
 * for all but the LGPL-covered parts and Talkback, and following the
 * LGPL for the LGPL-covered parts.
 *
 * Note that people who make modified versions of Gnash are not obligated
 * to grant this special exception for their modified versions; it is their
 * choice whether to do so. The GNU General Public License gives permission
 * to release a modified version without this exception; this exception
 * also makes it possible to release a modified version which carries
 * forward this exception.
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
	char buf[1024];

	sprintf(buf, "createTextField(\"out\", %d, %d, %d, %d, %d); "
		" xtrace = function (msg) { "
		" trace (msg); "
		" _level0.out.text = msg; "
		"};",
		depth, x, y, width, height);
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

