/* 
 *   Copyright (C)  2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * zou lunkai zoulunkai@gmail.com
 *
 * Test DefineEditText tag with VariableName
 *
 * Description:
 *  Frame2: add dtext1, associate it with a variable 'edit_text_var'
 *  Frame3: add dtext2, associate it with a variable 'edit_text_var' again
 *  Frame4: add dtext2, associate it with a variable 'edit_text_var' again
 *  Frame5: set edit_text_var to 'Hahaha'
 *  Frame6: remove dtext1
 *  Frame7: remove dtext2
 *  Frame8: remove dtext3
 *  Frame 9: add dtext4, associate it with a variable 'edit_text_var' again
 *  Frame 10: set edit_text_var to a new Object
 *  Frame 11: provide a user defined toString for edit_text_var
 *  Frame 12: some checks
 *  Frame 13: set edit_text_var to a new Object again
 *  Frame 14: end  
 *
 *  run as ./DefineEditTextVariableNameTest2
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "DefineEditTextVariableNameTest2.swf"

SWFDisplayItem add_text_field(SWFMovie mo, SWFBlock font, const char* varname,  const char* text);

SWFDisplayItem
add_text_field(SWFMovie mo, SWFBlock font, const char* varname, const char* text)
{
  SWFTextField tf;

  tf = newSWFTextField();

  SWFTextField_setFlags(tf, SWFTEXTFIELD_DRAWBOX);

  SWFTextField_setFont(tf, font);
  SWFTextField_addChars(tf, text);
  SWFTextField_addString(tf, text);

  SWFTextField_setBounds(tf, 150, 30);
  
  /* Give the textField a variablename*/
  SWFTextField_setVariableName(tf, varname);

  return SWFMovie_add(mo, (SWFBlock)tf);
}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  const char *srcdir=".";
  char fdbfont[256];
  SWFMovieClip  dejagnuclip;
  SWFFont bfont;
  SWFDisplayItem it1, it2, it3, it4;
  

  // Frame1: Initialization

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
  
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo);  

  // Frame2: add dtext1, associate it with a variable 'edit_text_var'
  
  bfont = get_default_font(srcdir);
       
  it1 = add_text_field(mo, (SWFBlock)bfont, "edit_text_var",  "Hello");
  SWFDisplayItem_setName(it1, "dtext1");
  SWFDisplayItem_moveTo(it1, 0, 100);
  
  check_equals(mo, "edit_text_var", "'Hello'");
  check_equals(mo, "dtext1.text", "'Hello'");
  SWFMovie_nextFrame(mo); 
  
  // Frame3: add dtext2, associate it with a variable 'edit_text_var' again
  
  it2 = add_text_field(mo, (SWFBlock)bfont, "edit_text_var",  "World");
  SWFDisplayItem_setName(it2, "dtext2");
  SWFDisplayItem_moveTo(it2, 0, 200);
  
  check_equals(mo, "edit_text_var", "'Hello'");
  check_equals(mo, "dtext1.text", "'Hello'");
  check_equals(mo, "dtext2.text", "'Hello'");
  SWFMovie_nextFrame(mo); 
  
  // Frame4: add dtext3, associate it with a variable 'edit_text_var' again
  
  it3 = add_text_field(mo, (SWFBlock)bfont, "edit_text_var",  "Hello World");
  SWFDisplayItem_setName(it3, "dtext3");
  SWFDisplayItem_moveTo(it3, 0, 300);
  
  check_equals(mo, "edit_text_var", "'Hello'");
  check_equals(mo, "dtext1.text", "'Hello'");
  check_equals(mo, "dtext2.text", "'Hello'");
  check_equals(mo, "dtext3.text", "'Hello'");
  SWFMovie_nextFrame(mo); 
  
  // Frame5: set edit_text_var to 'Hahaha'
  
  add_actions(mo, "edit_text_var = 'Hahaha'; ");
  check_equals(mo, "edit_text_var", "'Hahaha'");
  check_equals(mo, "dtext1.text", "'Hahaha'");
  check_equals(mo, "dtext2.text", "'Hahaha'");
  check_equals(mo, "dtext3.text", "'Hahaha'");
  
  add_actions(mo, "dtext1.variable = 'newName'; ");
  // Maybe 'variable' is the connection point?
  check_equals(mo, "dtext1.text", "'Hello'");
  // Change 'variable' back to its orignal string.
  add_actions(mo, "dtext1.variable = 'edit_text_var'; ");
  check_equals(mo, "dtext1.text", "'Hahaha'");
  SWFMovie_nextFrame(mo); 
  
  // Frame6: remove dtext1
  
  SWFDisplayItem_remove(it1);
  check_equals(mo, "typeof(edit_text_var)", "'string'" );
  check_equals(mo, "edit_text_var", "'Hahaha'");
  SWFMovie_nextFrame(mo); 
  
  // Frame7: remove dtext2
  
  SWFDisplayItem_remove(it2);
  check_equals(mo, "typeof(edit_text_var)", "'string'" );
  check_equals(mo, "edit_text_var", "'Hahaha'");
  SWFMovie_nextFrame(mo); 

  // Frame8: remove dtext3
  
  SWFDisplayItem_remove(it3);
  check_equals(mo, "typeof(edit_text_var)", "'string'" );
  check_equals(mo, "edit_text_var", "'Hahaha'");
  SWFMovie_nextFrame(mo); 
  
  // Frame 9: add dtext4, associate it with a variable 'edit_text_var' again. 
  
  it4 = add_text_field(mo, (SWFBlock)bfont, "edit_text_var",  "Hello");
  SWFDisplayItem_setName(it4, "dtext4");
  SWFDisplayItem_moveTo(it4, 0, 400);
  check_equals(mo, "edit_text_var", "'Hahaha'");
  check_equals(mo, "dtext4.text", "'Hahaha'");
  SWFMovie_nextFrame(mo); 
  
  // Frame10: set edit_text_var to a new Object
  
  add_actions(mo, "edit_text_var = new Object();");
  check_equals(mo, "typeof(edit_text_var)", "'object'");
  check_equals(mo, "typeof(dtext4.text)", "'string'");
  check_equals(mo, "dtext4.text", "'[object Object]'");
  SWFMovie_nextFrame(mo);
  
  // Frame 11: provide a user defined toString for edit_text_var

  add_actions(mo, "Object.prototype.toString = function() {return 'TO_STRING';}; ");
  check_equals(mo, "typeof(dtext4.text)", "'string'");
  // Object.prototype.toString not invoked for dtext4.text (associated variable didn't change it's value)
  check_equals(mo, "dtext4.text", "'[object Object]'");
  add_actions(mo, "edit_text_var = new Object();");
  check_equals(mo, "dtext4.text", "'TO_STRING'");
  check_equals(mo, "typeof(dtext4.text.toString)", "'function'");
  check_equals(mo, "dtext4.text.toString()", "'TO_STRING'");
  check_equals(mo, "dtext4.text.valueOf()", "'TO_STRING'");
  SWFMovie_nextFrame(mo);
  
  // Frame 12: dtext4.text still not updated
  // Deduction: dtext4.text won't update if edit_text_var is untouched.
  check_equals(mo, "edit_text_var.toString()", "'TO_STRING'");
  check_equals(mo, "dtext4.text", "'TO_STRING'");
  SWFMovie_nextFrame(mo);
  
  // Frame 13: dtext4.text updated. 
  // Deduction: setting edit_text_var triggered updating dtext4.text.
  add_actions(mo, "edit_text_var = new Object();");
  check_equals(mo, "edit_text_var.toString()", "'TO_STRING'");
  check_equals(mo, "dtext4.text", "'TO_STRING'");
  SWFMovie_nextFrame(mo);
  
  // Frame 14: end
  add_actions(mo, "totals(36); stop();");
  SWFMovie_nextFrame(mo); 
 
  // Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
