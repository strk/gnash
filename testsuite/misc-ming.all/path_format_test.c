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
 */ 

/*
    _root
      |------mc1
      |       |------mc11
      |-------3 (mvoieClip with a integral number name)
      |       |-------1(mvoieClip with a integral number name)
      |-------4.1 (movieClip with a float number name )
*/

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME  "path_format_test.swf"


int
main(int argc, char** argv)
{
  
  SWFMovie mo;
  
  SWFMovieClip mc1, mc11;
  SWFMovieClip int_named_parent, int_named_child;
  SWFMovieClip float_named_mc;
  SWFMovieClip dejagnuclip;
  
  SWFDisplayItem it1, it11;
  SWFDisplayItem int_named_parent_it, int_named_child_it;
  SWFDisplayItem float_named_it;
  
  SWFShape  sh_red;


  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      return 1;
  }

  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 12.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, " x1=0; x2=0; x3=0; x4=0; x5=0; x6=0; x7=0; x8=0; x9=0; x10=0; ");
  SWFMovie_nextFrame(mo); /* 1st frame of _root */

  
  mc11 = newSWFMovieClip();
  sh_red = make_fill_square (400, 200, 30, 30, 0, 0, 0, 0, 0, 0);
  SWFMovieClip_add(mc11, (SWFBlock)sh_red);  
  add_clip_actions(mc11, "stop();");
  SWFMovieClip_nextFrame(mc11); 
  SWFMovieClip_nextFrame(mc11); 
  SWFMovieClip_nextFrame(mc11); 
  
  add_clip_actions(mc11, "_root.x1 = 'mc11_frame4'; stop(); "); 
  SWFMovieClip_labelFrame(mc11, "frame4");
  SWFMovieClip_nextFrame(mc11); 
  add_clip_actions(mc11, "_root.x2 = 'mc11_frame5'; stop(); ");
  SWFMovieClip_labelFrame(mc11, "frame5");
  SWFMovieClip_nextFrame(mc11); 
  add_clip_actions(mc11, "_root.x3 = 'mc11_frame6'; stop(); ");
  SWFMovieClip_labelFrame(mc11, "frame6");
  SWFMovieClip_nextFrame(mc11); 
  
    
  mc1 = newSWFMovieClip();
  sh_red = make_fill_square (300, 200, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh_red);  
  add_clip_actions(mc1, "stop();");
  it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11);  
  SWFDisplayItem_setDepth(it11, 10); 
  SWFDisplayItem_setName(it11, "mc11"); 
  SWFMovieClip_nextFrame(mc1); 
  SWFMovieClip_nextFrame(mc1); 
  SWFMovieClip_nextFrame(mc1); 
  
  add_clip_actions(mc1, "_root.x4 = 'mc1_frame4'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame4");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x5 = 'mc1_frame5'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame5");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x6 = 'mc1_frame6'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame6");
  SWFMovieClip_nextFrame(mc1); 


  /* construct a movieClip named by a integral number 1 */
  int_named_child = newSWFMovieClip();
  sh_red = make_fill_square (400, 300, 30, 30, 0, 0, 0, 0, 0, 0);
  SWFMovieClip_add(int_named_child, (SWFBlock)sh_red); 
  add_clip_actions(int_named_child, " stop(); "
                                     " _root.x9 = this._name; " );
  SWFMovieClip_nextFrame(int_named_child);   
    
  /* construct a movieClip named by a integral number 3 */
  int_named_parent = newSWFMovieClip();
  sh_red = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(int_named_parent, (SWFBlock)sh_red);  
  add_clip_actions(int_named_parent, " stop(); "
                                     " _root.x7 = this._name; " );
  SWFMovieClip_nextFrame(int_named_parent); 
   
  /* add movieClip "1" to movieClip "3" */                             
  int_named_child_it = SWFMovieClip_add(int_named_parent, (SWFBlock)int_named_child); 
  SWFDisplayItem_setDepth(int_named_child_it, 3);
  SWFDisplayItem_setName(int_named_child_it, "1");

  
  /* construct a movieClip named by a float number */
  float_named_mc = newSWFMovieClip();
  sh_red = make_fill_square (300, 400, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(float_named_mc, (SWFBlock)sh_red);  
  add_clip_actions(float_named_mc, " stop(); "
                                   " _root.x8 = this._name; " );
  SWFMovieClip_nextFrame(float_named_mc); 
  
    
  /* place _root.mc1, give it a name "mc1" */
  it1 = SWFMovie_add(mo, (SWFBlock)mc1); 
  SWFDisplayItem_setDepth(it1, 20); 
  SWFDisplayItem_setName(it1, "mc1"); 
  
  /* place _root.int_named_parent, give it a name "3" */
  int_named_parent_it = SWFMovie_add(mo, (SWFBlock)int_named_parent); 
  SWFDisplayItem_setDepth(int_named_parent_it, 21); 
  SWFDisplayItem_setName(int_named_parent_it, "3"); 
  
  /* place _root.float_named_mc, give it a name "4.1" */
  float_named_it = SWFMovie_add(mo, (SWFBlock)float_named_mc); 
  SWFDisplayItem_setDepth(float_named_it, 21); 
  SWFDisplayItem_setName(float_named_it, "4.1"); 
  
  SWFMovie_nextFrame(mo); /* 2nd frame of _root */

  check_equals(mo, "typeOf(_root.mc1)", "'movieclip'");
  check_equals(mo, "typeOf(_root.mc1.mc11)", "'movieclip'");
  check_equals(mo, "_root.x7", "'3'"); // check for movieClip "3"
  check_equals(mo, "_root.x9", "0"); // check for movieClip "3"."1"
  if( OUTPUT_VERSION == 5)
  {
    xcheck_equals(mo, "_root.x8", "'4.1'"); //check for moveiClip "4.1"
  }
  else
  {
    check_equals(mo, "_root.x8", "0");
  }
  
  if(OUTPUT_VERSION == 5)
  {
    xcheck_equals(mo, "_root.x8", "'4.1'");
  }
  else if(OUTPUT_VERSION >= 6)
  {
    check_equals(mo, "_root.x8", "0");
  }
  
 
  add_actions(mo, "asm{"
                       " push 'x10' "
                       " push '_root.3' "
                       " getvariable "
                       " typeof "
                       " setvariable "
                   " }; ");
  /* seems we could access _root.3(movieClip) successfully!!! */
  check_equals(mo, "_root.x10", "'movieclip'" );
  
  add_actions(mo, "asm{"
                       " push 'x10' "
                       " push '_root.4.1' "
                       " getvariable "
                       " typeof "
                       " setvariable "
                   " }; ");
  /* accessing _root.4.1 failed */
  check_equals(mo, "_root.x10", "'undefined'" ); 
 
  add_actions(mo, "asm{"
                       " push 'x10' "
                       " push '/4.1' "
                       " getvariable "
                       " typeof "
                       " setvariable "
                   " }; ");
  /* accessing _root.4.1 failed */
  check_equals(mo, "_root.x10", "'undefined'" ); 
   
  add_actions(mo, "asm{"
                       " push 'x10' "
                       " push '_root.3.1' "
                       " getvariable "
                       " typeof "
                       " setvariable "
                   " }; ");
  /* Notice that _root.3.1 is not even displayed on screen 
     with all versions. accessing _root.3.1 failed!  */
  check_equals(mo, "_root.x10", "'undefined'" ); 


  add_actions(mo, "asm{"
                       " push 'x10' "
                       " push '/3/1' "
                       " getvariable "
                       " typeof "
                       " setvariable "
                   " }; ");
  /* Notice that _root.3.1 is not even displayed on screen 
     with all versions. accessing _root.3.1 failed! */
  check_equals(mo, "_root.x10", "'undefined'" ); 
  add_actions(mo, CALLFRAME"('/mc1/mc11/:4');");
  /* the above path format is supported */
  check_equals(mo, "_root.x1", "'mc11_frame4'" ); 
  
  /* reset _root.x1 to 0*/
  add_actions(mo, "_root.x1 = 0;");
  add_actions(mo, CALLFRAME"('_root.mc1.mc11.4');");
  /* the above path format is supported */
  check_equals(mo, "_root.x1", "'mc11_frame4'" ); 

  /* reset _root.x1 to 0*/
  add_actions(mo, "_root.x1 = 0;");
  add_actions(mo, CALLFRAME"('_root.mc1.mc11:4');");
  /* the above path format is supported */
  check_equals(mo, "_root.x1", "'mc11_frame4'" ); 
  
  /* reset _root.x1 to 0*/
  add_actions(mo, "_root.x1 = 0;");
  add_actions(mo, CALLFRAME"('/mc1/mc11/4');");
  /* the above path format is not supported */
  check_equals(mo, "_root.x1", "0" ); 

  /* reset _root.x1~10 to 0 */
  add_actions(mo, CALLFRAME"('/:1');");
 
  /* test valid path format for ActionGotoExpression */
  add_actions(mo, " path = '/mc1/mc11/4'; "        // not supported
                  " gotoAndStop(path); " 
                  " path = '/mc1/mc11/frame5'; "   // not supported
                  " gotoAndStop(path); "
                  " path = '/mc1/mc11/:frame6'; "  // supported format
                  " gotoAndStop(path); "
                  " path = '_root.mc1.4';"         // supported format
                  " gotoAndStop(path);"
                  " path = '_root.mc1.frame5'; "   // supported format
                  " gotoAndStop(path); " 
                  " path = '_root.mc1:frame6'; "   // supported format
                  " gotoAndStop(path); " );
  SWFMovie_nextFrame(mo); /* 3rd frame of _root */
   
   
  /* checks */
  check_equals(mo, "_root.x1", "0");
  check_equals(mo, "_root.x2", "0");
  check_equals(mo, "_root.x3", "'mc11_frame6'");
  check_equals(mo, "_root.x4", "'mc1_frame4'");
  check_equals(mo, "_root.x5", "'mc1_frame5'");
  check_equals(mo, "_root.x6", "'mc1_frame6'");
  
  add_actions(mo,  " checkpoint = 100;"
                     "asm{"
                          " push 'checkpoint' "
                          " push '/_root/x1' "  // not supported
                          " getvariable "
                          " setvariable "
                     "};");
  check_equals(mo, "checkpoint", "undefined");                
  
  add_actions(mo,  " checkpoint = 100;"
                     "asm{"
                          " push 'checkpoint' "
                          " push '_root.x1' " 
                          " getvariable "
                          " setvariable "
                     "};");
  check_equals(mo, "checkpoint", "0"); 
  
  add_actions(mo,  " checkpoint = 100;"
                     "asm{"
                          " push 'checkpoint' "
                          " push '_root:x1' "
                          " getvariable "
                          " setvariable "
                     "};");
  check_equals(mo, "checkpoint", "0"); 
   
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 4th frame of _root */
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

