/* 
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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

/*
 * Test DefineText tag.
 * 
 * run as ./DefineTextTest <mediadir> to produce DefineTextTest.swf
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "DefineTextTest.swf"

int
main(int argc, char** argv)
{
  SWFMovie mo;
  const char *srcdir=".";
  char fdbfont[256];
  SWFMovieClip  dejagnuclip;
  
  /*********************************************
   *
   * Initialization
   *
   *********************************************/

  if ( argc>1 ) srcdir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
    return 1;
  }

  sprintf(fdbfont, "%s/Bitstream-Vera-Sans.fdb", srcdir);

  puts("Setting things up");

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  //Ming_setScale(20.0); /* so we talk twips */
 
  mo = newSWFMovie();
  SWFMovie_setRate(mo, 1.0);
  SWFMovie_setDimension(mo, 800, 600);
  
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); // 1st frame 

  /*********************************************
   *
   * Add some textfields
   *
   *********************************************/
  {
    SWFMovieClip mc; // to check sizes
    SWFDisplayItem it;
    SWFText tf;

    FILE *font_file = fopen(fdbfont, "r");
    if ( font_file == NULL )
    {
      perror(fdbfont);
      exit(1);
    }
    SWFFont efont = loadSWFFontFromFile(font_file);

    tf = newSWFText();

    SWFText_setFont(tf, efont);
    SWFText_setHeight(tf, 200);
    SWFText_setColor(tf, 0, 255, 0, 0xff);
    SWFText_addString(tf, "O", NULL);

    SWFText_setFont(tf, efont);
    SWFText_setHeight(tf, 200);
    SWFText_setColor(tf, 255, 0, 0, 0xff);
    SWFText_addString(tf, "X", NULL);

    mc = newSWFMovieClip();
    it = SWFMovieClip_add(mc, (SWFBlock)tf);
    SWFDisplayItem_setName(it, "stext1");
    SWFMovieClip_nextFrame(mc);

    it = SWFMovie_add(mo, mc);
    SWFDisplayItem_setName(it, "mc");
    SWFDisplayItem_moveTo(it, 0, 400);
  }
  SWFMovie_nextFrame(mo);  // 2nd frame


  add_actions(mo,
          "_root.note('Follow the instructions. "
                    "Click means press and release');"
		  "instructions = [	"
          "     'Move the mouse onto the green O',"
		  "		'Click',"
	      "		'Move the mouse to the centre of the O',"
          "     'Click as much as you like then move back onto the green O',"
          "     'Click',"
          "     'Move outside the green O' ];"
		  "_global.events = 0;"
          "_global.clicks = 0;"
          "_global.mouseInOut = 0;"
          "checkEvents = function() {"
          "     if (_global.events == instructions.length) {"
          "         play();"
          "     }"
          "     else { _root.note(instructions[_global.events++]); };"
          "};"
		  "mc.onPress = function() {"
		  "	    _global.clicks++;"
          "     checkEvents();"
		  "};"
		  "mc.onRollOver = function() {"
          "     _global.mouseInOut++;"
          "     checkEvents();"
		  "};"
		  "mc.onRollOut = function() {"
          "     _global.mouseInOut++;"
          "     checkEvents();"
		  "};"
          "checkEvents();"
		  );


  // static text is not a referenceable char
  check_equals(mo, "mc.stext1", "mc");
  check_equals(mo, "typeof(mc.stext1)", "'movieclip'");
  check_equals(mo, "mc.stext1._target", "'/mc'");

  check_equals(mo, "mc._width", "288.05");

  // Wait for mouse clicks.
  add_actions(mo, "stop();");

  SWFMovie_nextFrame(mo);

  xcheck_equals(mo, "_global.clicks", "2");
  xcheck_equals(mo, "_global.mouseInOut", "4");
  add_actions(mo, "endoftest=true; totals(); stop();");
  SWFMovie_nextFrame(mo);  // 3rd frame

  /*****************************************************
   *
   * Output movie
   *
   *****************************************************/
  puts("Saving " OUTPUT_FILENAME );

  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
