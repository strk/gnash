/*
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
 */ 

/*
 * Zou Lunkai, zoulunkai@gmail.com
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME  "get_frame_number_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  dejagnuclip;

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
  SWFMovie_nextFrame(mo); /* 1st frame */


  SWFMovie_labelFrame(mo, "8");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  SWFMovie_labelFrame(mo, "8a");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  SWFMovie_labelFrame(mo, "aa");
  SWFMovie_nextFrame(mo); /* 4th frame*/
  
  check_equals(mo, "_currentframe", "5");
  add_actions(mo, " gotoAndStop('8'); ");         // ActionGotoLabel
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " gotoAndStop('xxxxxxxx'); ");  // ActionGotoLabel
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " gotoAndStop('Infinity'); ");  // ActionGotoLabel
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " gotoAndStop(Infinity); ");    // ActionGotoExpression
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " x = 0; "
                  " gotoAndStop(x); ");         // ActionGotoExpression
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " x = -1; "
                  " gotoAndStop(x); ");         // ActionGotoExpression
  check_equals(mo, "_currentframe", "2");             
  add_actions(mo, " gotoAndStop(6); ");         // ActionGotoFrame
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  add_actions(mo, "function func1() {}"
                  "func1.prototype.toString = function() { return '8'; };"
                  "x1 = new func1();"
                  
                  "function func2() {}"
                  "func2.prototype.valueOf = function() { return 8;}; "
                  "x2 = new func2();"
                  
                  "function func3() {}"
                  "func3.prototype.toString = function() { return '8'; }; "
                  "func3.prototype.valueOf = function() { return 8;};"
                  "x3 = new func3();" );
                  
                  
  add_actions(mo, " x = '8'; gotoAndStop(x); ");     // ActionGotoExpression
  check_equals(mo, "_currentframe", "6");
  
  add_actions(mo, " x = '8a'; gotoAndStop(x); ");    // ActionGotoExpression
  check_equals(mo, "_currentframe", "3");
  
  add_actions(mo, " x = 'aa'; gotoAndStop(x); ");    // ActionGotoExpression
  check_equals(mo, "_currentframe", "4");
  
  add_actions(mo, " gotoAndStop(x1); ");             // ActionGotoExpression
  /* reach the last frame */
  check_equals(mo, "_currentframe", "6"); 
  
  /* reset _currentframe to 1 */
  add_actions(mo, " gotoAndStop(1); ");  
  add_actions(mo, " gotoAndStop(x2); ");             // ActionGotoExpression
  check_equals(mo, "_currentframe", "1"); 
  
  add_actions(mo, " gotoAndStop(x3); ");             // ActionGotoExpression
  /* reach the last frame */
  check_equals(mo, "_currentframe", "6");
  
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 6th frame */

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}






