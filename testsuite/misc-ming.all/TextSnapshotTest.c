/* 
 *   Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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
 * Uses both "embedded" font and device fonts.
 * The text written is 'Hello world' in both cases.
 * Text at the bottom is the one with embedded fonts.
 * 
 * TODO: add a testrunner for pixel checking.
 * TODO: test autoSize and wordWrap interaction (what takes precedence?)
 *
 * run as ./DefineEditTextTest
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 8
#define OUTPUT_FILENAME "TextSnapshotTest.swf"

SWFDisplayItem
add_text_field(SWFMovie mo, SWFFont font, const char* text);

SWFDisplayItem
add_text_field(SWFMovie mo, SWFFont font, const char* text)
{
  SWFText tf;

  tf = newSWFText();

  SWFText_setFont(tf, font);


  SWFText_addString(tf, text, NULL);

  return SWFMovie_add(mo, (SWFBlock)tf);
}

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
    SWFDisplayItem it;
    FILE *font_file = fopen(fdbfont, "r");
    if ( font_file == NULL )
    {
      perror(fdbfont);
      exit(1);
    }
    SWFBrowserFont bfont = newSWFBrowserFont("_sans");
    SWFFont efont = loadSWFFontFromFile(font_file);

    it = add_text_field(mo, efont, "First text");
    SWFDisplayItem_setName(it, "dtext1");
    SWFDisplayItem_moveTo(it, 0, 200);
    it = add_text_field(mo, efont, "Zweites Textfeld");
    SWFDisplayItem_setName(it, "etext1");
    SWFDisplayItem_moveTo(it, 0, 300);

    it = add_text_field(mo, efont, "Some more static text here... abcdefgh");
    SWFDisplayItem_setName(it, "dtext2");
    SWFDisplayItem_moveTo(it, 0, 400);
  }
  SWFMovie_nextFrame(mo); 

  add_actions(mo, "createTextField('dynamictext1', 99, 10, 10, 10, 10);"
		  "this.dynamictext1.text = 'Dynamic Text';");
  
  add_actions(mo, "ts = this.getTextSnapshot();");
  check(mo, "ts instanceof TextSnapshot");
  check_equals(mo, "ts.getCount()", "64");

  check_equals(mo, "ts.getText(0, 1)", "'F'");
  check_equals(mo, "ts.getText(3, 3)", "'s'");
  check_equals(mo, "ts.getText(-5, 5)", "'First'");
  check_equals(mo, "ts.getText(10, 6)", "'Z'");
  check_equals(mo, "ts.getText(0, 100)", "'First textZweites TextfeldSome more "
		  "static text here... abcdefgh'");
  add_actions(mo, "ss = ts.getText(100, 110);");
  check_equals(mo, "typeof(ss)", "'string'");
  check_equals(mo, "ss", "'h'");
  check_equals(mo, "ss.length", "1");
  
  check_equals(mo, "ts.getText(0, 100, true)", 
          "'First text\nZweites Textfeld\nSome more "
		  "static text here... abcdefgh'");

  add_actions(mo, "ts.setSelected(0, 30, true);");

  add_actions(mo, "ts = this.getTextSnapshot();");
  check_equals(mo, "typeof(ts)", "'object'");
  add_actions(mo, "backup = TextSnapshot;");
  add_actions(mo, "TextSnapshot = undefined;");
  add_actions(mo, "t = new TextSnapshot();");
  check_equals(mo, "typeof(t)", "'undefined'");

  check_equals(mo, "typeof(TextSnapshot)", "'undefined'");
  add_actions(mo, "ts = this.getTextSnapshot();");
  xcheck_equals(mo, "typeof(ts)", "'undefined'");
  add_actions(mo, "TextSnapshot = backup;");
  add_actions(mo, "ts = this.getTextSnapshot();");
  check_equals(mo, "typeof(ts)", "'object'");
  
  add_actions(mo, "backup = TextSnapshot.prototype;");
  add_actions(mo, "TextSnapshot.prototype = undefined;");
  add_actions(mo, "ts = this.getTextSnapshot();");
  check_equals(mo, "typeof(ts)", "'object'");
  add_actions(mo, "TextSnapshot.prototype = backup;");
  add_actions(mo, "ts = this.getTextSnapshot();");
  check_equals(mo, "typeof(ts)", "'object'");
  
  
  add_actions(mo, "totals(); stop();");
  


  
  SWFMovie_nextFrame(mo); 

  /*****************************************************
   *
   * Output movie
   *
   *****************************************************/
  puts("Saving " OUTPUT_FILENAME );

  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
