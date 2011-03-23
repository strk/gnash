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
  
  SWFMovie_labelFrame(mo, "4.8");
  SWFMovie_nextFrame(mo); /* 5th frame*/
  
  check_equals(mo, "_currentframe", "6");
  add_actions(mo, " gotoAndStop('8'); ");         // ActionGotoLabel
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " gotoAndStop('xxxxxxxx'); ");  // ActionGotoLabel
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " gotoAndStop('Infinity'); ");  // ActionGotoLabel
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " gotoAndStop(Infinity); ");    // ActionGotoExpression
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " x = 0; "
                  " gotoAndStop(x); ");    // ActionGotoExpression
  check_equals(mo, "_currentframe", "2");
  add_actions(mo, " x = -1; "
                  " gotoAndStop(x); ");    // ActionGotoExpression
  check_equals(mo, "_currentframe", "2");  
  add_actions(mo, " x = 4.8; "  // valid frame label                   
                  " gotoAndStop(x); ");    // ActionGotoExpression  
  check_equals(mo, "_currentframe", "5");          
  add_actions(mo, " x = 6.1; "  // invalid frame number                   
                  " gotoAndStop(x); ");    // ActionGotoExpression  
  check_equals(mo, "_currentframe", "5"); 
  add_actions(mo, " gotoAndStop(7); ");    // ActionGotoFrame
  SWFMovie_nextFrame(mo); /* 6th frame */
  
  add_actions(mo, "function func1() {}"
                  "func1.prototype.toString = function() { return '8'; };"
                  "x1 = new func1();"
                  
                  "function func2() {}"
                  "func2.prototype.valueOf = function() { return 8;}; "
                  "x2 = new func2();"
                  
                  "function func3() {}"
                  "func3.prototype.toString = function() { return '8'; }; "
                  "func3.prototype.valueOf = function() { return 8;};"
                  "x3 = new func3();" 
                  
                  "function func4() {}"
                  "func4.prototype.toString = function() { return '4.8'; }; "
                  "func4.prototype.valueOf = function() { return '4.8';};"
                  "x4 = new func4();"
                  
                  "x5 = new Number(3);"
                  "Number.prototype.toString =  function () { return '4'; }; "
                  
                  "x6 = new String('3');"
                  "String.prototype.toString =  function () { return '4'; }; " );
                  
                  
  add_actions(mo, " x = '8'; gotoAndStop(x); ");  // ActionGotoExpression
  /* reach the last frame */
  check_equals(mo, "_currentframe", "7");
  
  add_actions(mo, " x = '8a'; gotoAndStop(x); ");  // ActionGotoExpression
  check_equals(mo, "_currentframe", "3");
  
  add_actions(mo, " x = 'aa'; gotoAndStop(x); ");  // ActionGotoExpression
  check_equals(mo, "_currentframe", "4");
  
  add_actions(mo, " gotoAndStop(x1); ");  // ActionGotoExpression
  /* reach the last frame, toString() invoked */
  check_equals(mo, "_currentframe", "7"); 
  
  /* reset _currentframe to 1 */
  add_actions(mo, " gotoAndStop(1); ");  
  
  add_actions(mo, " gotoAndStop(x2); ");  // ActionGotoExpression
  check_equals(mo, "_currentframe", "1"); 
  
  add_actions(mo, " gotoAndStop(x3); ");  // ActionGotoExpression
  /* reach the last frame, toString() invoked */
  check_equals(mo, "_currentframe", "7");
  
  add_actions(mo, " gotoAndStop(x4); ");  // ActionGotoExpression
  /* "4.8" is a valid frame label, toString() invoked */
  check_equals(mo, "_currentframe", "5");
  
  check_equals(mo, "_root.x5", "3");
  check_equals(mo, "_root.x5.toString()", "'4'");
  check_equals(mo, "_root.x5.toString()", "4");
  add_actions(mo, " gotoAndStop(x5); ");  // ActionGotoExpression
  /* toString() invoked again for Number Object */
  check_equals(mo, "_currentframe", "4");
  
  /* reset _currentframe to 1 */
  add_actions(mo, " gotoAndStop(1); "); 
  
  check_equals(mo, "_root.x6", "'3'");
  check_equals(mo, "_root.x6.toString()", "'4'");
  check_equals(mo, "_root.x6.toString()", "4");
  add_actions(mo, " gotoAndStop(x6); ");  // ActionGotoExpression
  /* toString() not invoked for String Object ??? */
  check_equals(mo, "_currentframe", "3");
  
  /* This ensure the movie stop at the last frame,
   * and the actions in last frame will not be pushed again 
   */
  add_actions(mo, " gotoAndStop(10000); ");  // ActionGotoFrame
  check_equals(mo, "_currentframe", "7");
  add_actions(mo, "gotoAndStop(0);"); // ActionGotoFrame
  check_equals(mo, "_currentframe", "7");
  add_actions(mo, "x = 0; gotoAndStop(x);"); // ActionGotoExpression
  check_equals(mo, "_currentframe", "7");
  add_actions(mo, "x = -1; gotoAndStop(x);"); // ActionGotoExpression
  check_equals(mo, "_currentframe", "7");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 7th frame */

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}






