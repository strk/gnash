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

  /* setting flags seem unneeded */
  /*SWFTextField_setFlags(tf, SWFTEXTFIELD_USEFONT|SWFTEXTFIELD_NOEDIT);*/
  SWFTextField_addChars(tf, text);

  SWFTextField_addString(tf, text);

  /*
   * Bounds computed by Ming (if we omit the setBounds call)
   * are 2640, 240. This means that we're shrinking the available
   * space with this explicit setting. Gnash chokes in this case.
   *
   * Ref: https://savannah.gnu.org/bugs/?func=detailitem&item_id=16637.
   */
  SWFTextField_setBounds(tf, 100, 100);
  //SWFTextField_setBounds(tf, 60000, 338);

  /*
   * The following settings (found in the reported SWF)
   * are not needed to exploit the bug.
   */
 
  /*SWFTextField_setHeight(tf, 240);*/
  /*SWFTextField_setColor(tf, 0x00, 0x00, 0x00, 0xff);*/
  /*SWFTextField_setAlignment(tf, SWFTEXTFIELD_ALIGN_LEFT);*/
  /*SWFTextField_setLeftMargin(tf, 0);*/
  /*SWFTextField_setRightMargin(tf, 0);*/
  /*SWFTextField_setIndentation(tf, 0);*/
  /*SWFTextField_setLineSpacing(tf, 40);*/
  /*SWFTextField_setLineSpacing(tf, 40);*/

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
  
  y += inc;

  it = add_text_field(mo, (SWFBlock)bfont, "Transparent", 1, 2, 3,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 0);
  SWFDisplayItem_moveTo(it, 50, y);
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, no indent or "
          "margin", 0, 0, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);

  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, indent 4",
          4, 0, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, left margin 4",
          0, 4, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, right margin 4",
          0, 0, 4, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 16, left margin 4, "
          "indent 4", 4, 4, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 16, 1);
 
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 8, no indent or margin",
          0, 0, 0, SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 8, 1);

  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 8, indent 4", 4, 0, 0,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 8, 1);
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 0.2", 1, 2, 3,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 0.2, 1);
  
  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "X scaled by 8", 0, 0, 0,
          SWFTEXTFIELD_ALIGN_LEFT, 0, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 8, 1);

  y += inc;
  
  it = add_text_field(mo, (SWFBlock)bfont, "Y scaled by 8", 1, 2, 3,
          SWFTEXTFIELD_ALIGN_LEFT, 10, 0, 0, 0, 255);
  SWFDisplayItem_moveTo(it, 50, y);
  SWFDisplayItem_scale(it, 1, 8);
  
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
