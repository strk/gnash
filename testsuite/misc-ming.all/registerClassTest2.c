/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
 *  zou lunkai,  zoulunkai@gmail.com
 *  
 *  Testcase for Object.registerClass().
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "registerClassTest2.swf"

void addExport(SWFMovie mo, SWFMovieClip mc, const char * name);

void
addExport(SWFMovie mo, SWFMovieClip mc, const char * name)
{
  SWFMovie_addExport(mo, (SWFBlock)mc, name);
  SWFMovie_writeExports(mo);
}

int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, mc3, dejagnuclip;
  SWFShape  sh1, sh2, sh3;
  SWFDisplayItem it1, it2, it3;
  const char *srcdir=".";

  if ( argc>1 ) srcdir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s\n", argv[0]);
    return 1;
  }

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  Ming_setScale(20.0); /* let's talk pixels */
 
  mo = newSWFMovie();
  SWFMovie_setRate(mo, 12);
  SWFMovie_setDimension(mo, 640, 400);

  // add the dejagnuclip for testing
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 80, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  
  // define two movieClips
  
  mc1 = newSWFMovieClip();
  sh1 = make_fill_square (100, 100, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh1); 
  SWFMovieClip_nextFrame(mc1);
  
  mc2 = newSWFMovieClip();
  sh2 = make_fill_square (100, 100, 60, 60, 255, 255, 0, 255, 255, 0);
  SWFMovieClip_add(mc2, (SWFBlock)sh2); 
  SWFMovieClip_nextFrame(mc2);
  
  // export them
  addExport(mo, mc1, "libItem1");
  addExport(mo, mc2, "libItem2");  
  SWFMovie_nextFrame(mo);  /* end of frame1 */
  

  add_actions(mo,
    "theClass1 = function() { this.x = 60;};"
    "theClass1.prototype = new MovieClip();"
    "theClass2 = function() { this.x = 600; this._x = 200;}; "
    "Object.registerClass('libItem1', theClass1);"
    "Object.registerClass('libItem2', theClass2);"
    "_root.attachMovie('libItem1', 'clip1', 10);"
    "_root.attachMovie('libItem2', 'clip2', 20);"
    );

  check_equals(mo, "typeof(clip1)", "'movieclip'");
  check_equals(mo, "typeof(clip2)", "'movieclip'");
  check_equals(mo, "clip1.__proto__", "theClass1.prototype");
  check_equals(mo, "clip2.__proto__", "theClass2.prototype");
#if OUTPUT_VERSION > 6
  // succeed in swf7 and swf8
  check_equals(mo, "clip1.constructor", "MovieClip");
#else if OUTPUT_VERSION == 6
  check_equals(mo, "clip1.constructor", "theClass1");
#endif
  check_equals(mo, "clip1.__constructor__", "theClass1");
  check_equals(mo, "clip2.constructor", "theClass2");
  check_equals(mo, "clip2.__constructor__", "theClass2");
  check_equals(mo, "clip1.x", "60");
  check_equals(mo, "clip2.x", "600");
  check_equals(mo, "clip1._x", "0");
  check_equals(mo, "clip2._x", "200");
  check_equals(mo, "typeof(MovieClip.prototype.getDepth)", "'function'");
  check_equals(mo, "clip1.getDepth()", "10");
  // clip2 does not inherit MovieClip
  check_equals(mo, "clip2.getDepth()", "undefined");

  SWFMovie_nextFrame(mo); /* end of frame2 */


 
  // Define movieclip mc2
  mc3 = newSWFMovieClip();
  sh3 = make_fill_square (0, 300, 100, 100, 255, 255, 0, 255, 255, 0);
  SWFMovieClip_add(mc3, (SWFBlock)sh3);
  
  addExport(mo, mc2, "libItem3");  
  
  it3 = SWFMovie_add(mo, mc3);
  SWFDisplayItem_addAction(it3,
    newSWFAction(" _root.note('mc3.onClipInitialize'); " 
                 " _root.xcheck_equals(typeof(_root.clip3), 'movieclip');" 
                 " _root.xcheck_equals(_root.clip3.__proto__, _root.theClass3.prototype);" 
                 ),
    SWFACTION_INIT);
    
  SWFDisplayItem_addAction(it3,
    newSWFAction(" _root.note('mc3.onClipConstruct'); "
                 " _root.check_equals(typeof(_root.clip3), 'movieclip'); "
                 // this one is passed by luck at the moment, both are undefined for Gnash
                 " _root.check_equals(_root.clip3.__proto__, _root.theClass3.prototype);"
                ),
    SWFACTION_CONSTRUCT);

  // add init actions for mc3
  add_clip_init_actions(mc3, " _root.note('initactions for mc3'); "
                             " theClass3 = function () {}; "
                             " theClass3.prototype = new MovieClip(); "
                             " Object.registerClass('libItem3', theClass3); "
                             " _root.attachMovie('libItem3', 'clip3', 30); ");
  SWFMovieClip_nextFrame(mc3);
  
  add_actions(mo, "totals(); stop();");
  SWFMovie_nextFrame(mo); /* end of frame4 */
     
 /*****************************************************
  *
  * Output movie
  *
  *****************************************************/

  puts("Saving " OUTPUT_FILENAME );

  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
