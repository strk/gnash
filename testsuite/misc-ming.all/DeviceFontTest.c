/* 
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010,
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

/*
 * Test DefineEditText tag.
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

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "DeviceFontTest.swf"

SWFDisplayItem add_text_field(SWFMovie mo, SWFBlock font, const char* text,
	float indent, float leftMargin, float rightMargin, SWFTextFieldAlignment align,
	float lineSpacing,
	unsigned int textR, unsigned int textG, unsigned int textB, unsigned int textA);

SWFDisplayItem
add_text_field(SWFMovie mo, SWFBlock font, const char* text, float indent,
		float leftMargin, float rightMargin,
		SWFTextFieldAlignment align, float lineSpacing,
		unsigned int textR, unsigned int textG,
		unsigned int textB, unsigned int textA)
{
  SWFTextField tf;

  tf = newSWFTextField();

  SWFTextField_setFont(tf, font);
  SWFTextField_setIndentation(tf, indent);
  SWFTextField_setLeftMargin(tf, leftMargin);
  SWFTextField_setRightMargin(tf, rightMargin);
  SWFTextField_setAlignment(tf, align);
  SWFTextField_setLineSpacing(tf, lineSpacing);
  SWFTextField_setColor(tf, textR, textG, textB, textA);

  SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX);
  SWFTextField_addChars(tf, text);

  SWFTextField_addString(tf, text);

  SWFTextField_setBounds(tf, 80, 16);
  
  return SWFMovie_add(mo, (SWFBlock)tf);
}

int
main(int argc, char** argv)
{
  SWFMovie mo;
  const char *srcdir=".";
  char fdbfont[256];
  SWFMovieClip  dejagnuclip;
  
  SWFDisplayItem it;

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
 
  mo = newSWFMovie();
  SWFMovie_setRate(mo, 1.0);
  SWFMovie_setDimension(mo, 800, 600);
  
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10,
          0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); // 1st frame 

  /*********************************************
   *
   * Add some textfields
   *
   *********************************************/
  SWFBrowserFont bfont = newSWFBrowserFont("_sans");

  int y = 30;
  int inc = 30;

  it = add_text_field(mo, (SWFBlock)bfont, "Normal", 1, 2, 3,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_setName(it, "tf1");
  
  y += inc;

  it = add_text_field(mo, (SWFBlock)bfont, "Transparent", 1, 2, 3,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 0);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_setName(it, "tf2");
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, no indent or "
          "margin", 0, 0, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  SWFDisplayItem_setName(it, "tf3");

  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, indent 4",
          4, 0, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  SWFDisplayItem_setName(it, "tf4");
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, left margin 4",
          0, 4, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  SWFDisplayItem_setName(it, "tf5");
  
  y += inc;
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, right margin 4",
          0, 0, 4, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  SWFDisplayItem_setName(it, "tf6");
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, left margin 4, "
          "indent 4", 4, 4, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  SWFDisplayItem_setName(it, "tf7");
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 8, no indent or margin",
          0, 0, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 8, 1);
  SWFDisplayItem_setName(it, "tf8");

  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 8, indent 4", 4, 0, 0,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 8, 1);
  SWFDisplayItem_setName(it, "tf9");
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 0.2", 8, 8, 8,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 0.2, 1);
  SWFDisplayItem_setName(it, "tf10");
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "Y scaled by 4", 4, 4, 0,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 1, 4);
  SWFDisplayItem_setName(it, "tf11");
  
  y += inc * 3;
  
  it = add_text_field(mo, (SWFBlock)bfont, "Y scaled by 8", 4, 4, 0,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 1, 8);
  SWFDisplayItem_setName(it, "tf12");


  // As these are device fonts, it's not clear how consistent the textWidth
  // value will be. It's not even clear that it's worth testing it.

  check_equals(mo, "tf1._width", "84");
  check_equals(mo, "tf1._height", "20");
  xcheck_equals(mo, "tf1._x", "48");
  xcheck_equals(mo, "tf1._y", "28");
  xcheck_equals(mo, "tf1.textHeight", "24");
  // Approx 42
  xcheck(mo, "tf1.textWidth >= 40 && tf1.textWidth <= 45");

  check_equals(mo, "tf2._width", "84");
  check_equals(mo, "tf2._height", "20");
  xcheck_equals(mo, "tf2._x", "48");
  xcheck_equals(mo, "tf2._y", "58");
  xcheck_equals(mo, "tf2.textHeight", "24");
  // Approx 69
  xcheck(mo, "tf2.textWidth >= 66 && tf2.textWidth <= 72");

  check_equals(mo, "tf3._width", "1344");
  check_equals(mo, "tf3._height", "20");
  xcheck_equals(mo, "tf3._x", "18");
  xcheck_equals(mo, "tf3._y", "88");
  xcheck_equals(mo, "tf3.textHeight", "24");
  // Approx 13
  xcheck(mo, "tf3.textWidth >= 11 && tf3.textWidth <= 15");

  check_equals(mo, "tf4._width", "1344");
  check_equals(mo, "tf4._height", "20");
  xcheck_equals(mo, "tf4._x", "18");
  xcheck_equals(mo, "tf4._y", "118");
  xcheck_equals(mo, "tf4.textHeight", "24");
  // Approx 9
  xcheck(mo, "tf4.textWidth >= 7 && tf4.textWidth <= 11");

  check_equals(mo, "tf5._width", "1344");
  check_equals(mo, "tf5._height", "20");
  xcheck_equals(mo, "tf5._x", "18");
  xcheck_equals(mo, "tf5._y", "148");
  xcheck_equals(mo, "tf5.textHeight", "24");
  // Approx 10
  xcheck(mo, "tf5.textWidth >= 8 && tf5.textWidth <= 12");

  check_equals(mo, "tf6._width", "1344");
  check_equals(mo, "tf6._height", "20");
  xcheck_equals(mo, "tf6._x", "18");
  xcheck_equals(mo, "tf6._y", "178");
  xcheck_equals(mo, "tf6.textHeight", "24");
  // Approx 11
  xcheck(mo, "tf6.textWidth >= 9 && tf6.textWidth <= 13");

  check_equals(mo, "tf7._width", "1344");
  check_equals(mo, "tf7._height", "20");
  xcheck_equals(mo, "tf7._x", "18");
  xcheck_equals(mo, "tf7._y", "208");
  xcheck_equals(mo, "tf7.textHeight", "24");
  // Approx 14
  xcheck(mo, "tf7.textWidth >= 14 && tf7.textWidth <= 16");

  check_equals(mo, "tf8._width", "672");
  check_equals(mo, "tf8._height", "20");
  xcheck_equals(mo, "tf8._x", "34");
  xcheck_equals(mo, "tf8._y", "238");
  xcheck_equals(mo, "tf8.textHeight", "24");
  // Approx 25
  xcheck(mo, "tf8.textWidth >= 21 && tf8.textWidth <= 27");

  check_equals(mo, "tf9._width", "672");
  check_equals(mo, "tf9._height", "20");
  xcheck_equals(mo, "tf9._x", "34");
  xcheck_equals(mo, "tf9._y", "268");
  xcheck_equals(mo, "tf9.textHeight", "24");
  // Approx 16
  xcheck(mo, "tf9.textWidth >= 14 && tf9.textWidth <= 18");

  check_equals(mo, "tf10._width", "16.8");
  check_equals(mo, "tf10._height", "20");
  xcheck_equals(mo, "tf10._x", "49.6");
  xcheck_equals(mo, "tf10._y", "298");
  xcheck_equals(mo, "tf10.textHeight", "24");
  // Approx 452
  xcheck(mo, "tf10.textWidth >= 440 && tf10.textWidth <= 460");

  // The textHeight for the following two fields varies.
  check_equals(mo, "tf11._width", "84");
  check_equals(mo, "tf11._height", "80");
  xcheck_equals(mo, "tf11._x", "48");
  xcheck_equals(mo, "tf11._y", "322");
  add_actions(mo, "trace(tf11.textWidth);");
  xcheck(mo, "tf11.textHeight >= 23 && tf11.textHeight <= 24");
  // Approx 315
  xcheck(mo, "tf11.textWidth >= 305 && tf11.textWidth <= 325");

  check_equals(mo, "tf12._width", "84");
  check_equals(mo, "tf12._height", "160");
  xcheck_equals(mo, "tf12._x", "48");
  xcheck_equals(mo, "tf12._y", "404");
  add_actions(mo, "trace(tf12.textWidth);");
  xcheck(mo, "tf12.textHeight >= 23 && tf12.textHeight <= 24");
  // Approx 640
  xcheck(mo, "tf12.textWidth >= 625 && tf12.textWidth <= 655");

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
