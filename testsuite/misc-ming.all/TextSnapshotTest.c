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
  
  check_equals(mo, "ts.getText(0, 14, true)", "'First text\nZwei'");

  check_equals(mo, "ts.findText(0, '', false)", "-1");
  check_equals(mo, "ts.findText(0, 'f', false)", "0");
  check_equals(mo, "ts.findText(0, 'f', true)", "22");
  check_equals(mo, "ts.findText(1, 'Rst', false)", "2");
  check_equals(mo, "ts.findText(3, 'RSt', false)", "-1");
  check_equals(mo, "ts.findText(100, 'h', false)", "-1");
  check_equals(mo, "ts.findText(64, 'h', false)", "-1");
  check_equals(mo, "ts.findText(-5, 'Zwei', true)", "-1");
  check_equals(mo, "ts.findText(-5, 'gh', true)", "-1");

  add_actions(mo, "ts.setSelected(5, 30, true);");
  check_equals(mo, "ts.getSelectedText()", "' textZweites TextfeldSome'");
  check_equals(mo, "ts.getSelectedText(true)", "' text\nZweites "
          "Textfeld\nSome'");
  check_equals(mo, "ts.getSelectedText(false)", "' textZweites TextfeldSome'");
  check_equals(mo, "ts.getSelected(0, 4)", "false");
  check_equals(mo, "ts.getSelected(1, 9)", "true");
  check_equals(mo, "ts.getSelected(-4, 10)", "true");
  check_equals(mo, "ts.getSelected(-4, 6)", "true");
  check_equals(mo, "ts.getSelected(28, 23)", "true");
  check_equals(mo, "ts.getSelected(20, 20)", "true");
  check_equals(mo, "ts.getSelected(20, 27)", "true");
  check_equals(mo, "ts.getSelected(-3, -1)", "false");
  check_equals(mo, "ts.getSelected(30, 31)", "false");
  check_equals(mo, "ts.getSelected(0, 5)", "false");
  check_equals(mo, "ts.getSelected(40, 45)", "false");
  check_equals(mo, "ts.getSelected(31, 34)", "false");

  check_equals(mo, "ts.getSelected(0)", "undefined");
  check_equals(mo, "ts.getSelected(1)", "undefined");
  check_equals(mo, "ts.getSelected(-4)", "undefined");
  check_equals(mo, "ts.getSelected(-4)", "undefined");
  check_equals(mo, "ts.getSelected(28)", "undefined");
  check_equals(mo, "ts.getSelected(20)", "undefined");

  // Selected text is stored in the textfield and reset when a new
  // snapshot is taken.
  add_actions(mo, "ts2 = new TextSnapshot(this);");
  check_equals(mo, "ts.getSelectedText(false)", "''");
  check_equals(mo, "ts2.getCount()", "64");
  check_equals(mo, "ts2.getSelectedText()", "''");
  add_actions(mo, "ts2 = this.getTextSnapshot();");
  check_equals(mo, "ts2.getCount()", "64");
  check_equals(mo, "ts2.getSelectedText()", "''");

  add_actions(mo, "ts2.setSelected(3, 10, true);");
  check_equals(mo, "ts2.getSelectedText(false).length", "7");
  check_equals(mo, "ts.getSelectedText(false).length", "7");

  add_actions(mo, "ts.setSelectedColor(0xffff00);");
  add_actions(mo, "ts2.setSelectedColor(0x0000ff);");

  check_equals(mo, "ts.getSelectedText(false)", "'st text'");
  add_actions(mo, "ri = ts.getTextRunInfo(4, 10);");
  check_equals(mo, "typeof(ri)", "'object'");
  check(mo, "ri instanceof Array");
  check_equals(mo, "ri.length", "7");

  add_actions(mo, "el = ri[1];");
  check_equals(mo, "typeof(el)", "'object'");
  check(mo, "!el.hasOwnProperty('indexInRun')");
  check_equals(mo, "el.indexInRun", "5");
  check_equals(mo, "el.selected", "true");
  check_equals(mo, "el.font", "'Bitstream Vera Sans'");
  check_equals(mo, "el.color", "0");
  check_equals(mo, "el.height", "12");
  check_equals(mo, "el.matrix_a", "1");
  check_equals(mo, "el.matrix_b", "0");
  check_equals(mo, "el.matrix_c", "0");
  check_equals(mo, "el.matrix_d", "1");
  check_equals(mo, "el.matrix_tx", "25.95");
  check_equals(mo, "el.matrix_ty", "200");
  xcheck_equals(mo, "el.corner0x", "25.95");
  xcheck_equals(mo, "el.corner0y", "202.8");
  xcheck_equals(mo, "el.corner1x", "29.75");
  xcheck_equals(mo, "el.corner1y", "202.8");
  xcheck_equals(mo, "el.corner2x", "29.75");
  xcheck_equals(mo, "el.corner2y", "188.85");
  xcheck_equals(mo, "el.corner3x", "25.95");
  xcheck_equals(mo, "el.corner3y", "188.85");

  // Check properties individually
  check_equals(mo, "ri[2].height", "12");
  check_equals(mo, "ri[3].height", "12");
  check_equals(mo, "ri[4].height", "12");
  check_equals(mo, "ri[5].height", "12");
  check_equals(mo, "ri[6].height", "12");

  check_equals(mo, "ri[2].selected", "true");
  check_equals(mo, "ri[3].selected", "true");
  check_equals(mo, "ri[4].selected", "true");
  check_equals(mo, "ri[5].selected", "true");
  check_equals(mo, "ri[6].selected", "false");

  check_equals(mo, "ri[2].matrix_tx", "29.75");
  check_equals(mo, "ri[2].matrix_ty", "200");
  check_equals(mo, "ri[3].matrix_tx", "34.4");
  check_equals(mo, "ri[3].matrix_ty", "200");
  check_equals(mo, "ri[4].matrix_tx", "41.75");
  check_equals(mo, "ri[4].matrix_ty", "200");

  xcheck_equals(mo, "ri[2].corner0x", "29.75");
  xcheck_equals(mo, "ri[2].corner0y", "202.8");
  xcheck_equals(mo, "ri[3].corner0x", "34.4");
  xcheck_equals(mo, "ri[3].corner0y", "202.8");
  xcheck_equals(mo, "ri[4].corner0x", "41.75");
  xcheck_equals(mo, "ri[4].corner0y", "202.8");
  
  xcheck_equals(mo, "ri[2].corner2y", "188.85");
  xcheck_equals(mo, "ri[3].corner2y", "188.85");
  xcheck_equals(mo, "ri[4].corner2y", "188.85");

  add_actions(mo, "ts.setSelected(0, 10, true);");
  add_actions(mo, "ts.setSelected(15, 20, false);");
  check_equals(mo, "ts2.getSelectedText().length", "10");

  add_actions(mo, "ri2 = ts.getTextRunInfo(0, 100);");

  check_equals(mo, "ri2[0].selected", "true");
  check_equals(mo, "ri2[1].selected", "true");
  check_equals(mo, "ri2[2].selected", "true");
  check_equals(mo, "ri2[3].selected", "true");
  check_equals(mo, "ri2[4].selected", "true");
  check_equals(mo, "ri2[5].selected", "true");
  check_equals(mo, "ri2[6].selected", "true");
  check_equals(mo, "ri2[15].selected", "false");
  check_equals(mo, "ri2[16].selected", "false");
  check_equals(mo, "ri2[17].selected", "false");
  check_equals(mo, "ri2[18].selected", "false");

  xcheck_equals(mo, "ri2[50].corner2y", "388.85");
  xcheck_equals(mo, "ri2[50].corner2x", "156.6");
  xcheck_equals(mo, "ri2[51].corner2y", "388.85");
  xcheck_equals(mo, "ri2[51].corner2x", "163.95");

  check_equals(mo, "ri2[50].matrix_tx", "151.65");
  check_equals(mo, "ri2[51].matrix_tx", "156.55");

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
