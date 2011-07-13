/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *  Testcases for Object.registerClass().
 *
 *  -=[ CASE1 ]=- 
 *
 *    (1) Export 'libItem1' and 'libItem2' symbols
 *    (2) DOACTION block
 *      (2.1) Register 'theClass1' and 'theClass2' to the symbols above
 *      (2.2) Attach the symbols as instances 'clip1' and 'clip2'
 *    (3) Other DOACTION blocks 
 *      (3.1) Verify that 'clip1' and 'clip2' are instances of 'theClass1' and 'theClass2' respectively
 *
 *
 *  -=[ CASE2 ]=-
 *
 *    (1) Export 'libItem3'
 *    (2) INITACTION block
 *      (2.1) Register 'theClass3' to 'libItem3'
 *      (2.2) Attach 'libItem3' as instance 'clip3'
 *      (2.3) Verify that 'clip3' is an instance of 'theClass3'
 *    (3) INITCLIP event for 'clip3'
 *      (3.1) Verify thet 'clip3' is an instance of 'theClass3'
 *    (4) CONSTRUCT event for 'clip3'
 *      (4.1) Verify thet 'clip3' is an instance of 'theClass3'
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
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
  SWFDisplayItem it3, it4;
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

  add_actions(mo,
		"_root.theClass1onLoadCalls = new Array();"
		"_root.theClass2onLoadCalls = new Array();"
		"_root.theClass3onLoadCalls = new Array();"
	);
  
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
    "theClass1 = function() { this.x = 60; this._alpha=20; };"
    "theClass1.prototype = new MovieClip();"
    "theClass1.prototype.onLoad = function() { "
    " trace('theClass1 proto onLoad ('+this+')');"
    " _root.theClass1onLoadCalls.push(this);"
    "};"
    "theClass2 = function() { this.x = 600; this._x = 200; this._alpha=20;}; "
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
#elif OUTPUT_VERSION == 6
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


 
  // Define movieclip mc3
  mc3 = newSWFMovieClip();
  sh3 = make_fill_square (0, 300, 100, 100, 255, 255, 0, 255, 255, 0);
  SWFMovieClip_add(mc3, (SWFBlock)sh3);
  SWFMovieClip_nextFrame(mc3);
  
  // Export mc3
  addExport(mo, mc3, "libItem3");  
  
  check_equals(mo, "_root.theClass1onLoadCalls.length", "1");
  check_equals(mo, "_root.theClass1onLoadCalls[0]", "_level0.clip1");
   
  // add init actions for mc3
  add_clip_init_actions(mc3, " _root.note('mc3.initactions'); "
                             " theClass3 = function () {}; "
                             " theClass3.prototype = new MovieClip(); "
                             " theClass3.prototype.onLoad = function() {"
			     "  trace('theClass3 proto onLoad ('+this+')');"
			     "  _root.theClass3onLoadCalls.push(this);"
			     " };"
                             " Object.registerClass('libItem3', theClass3); "
                             " _root.attachMovie('libItem3', 'clip3', 30); "
                             // clip3.__proto__ is initialized before executing onClipConstruct
                             "_root.check_equals(clip3.__proto__, _root.theClass3.prototype); ");
                             
  it3 = SWFMovie_add(mo, mc3);
  SWFDisplayItem_setName(it3, "clipevs");
  SWFDisplayItem_addAction(it3,
    newSWFAction(" _root.note('mc3.onClipInitialize'); " 
                 " _root.check_equals(typeof(_root.clip3), 'movieclip');" 
                 " _root.check_equals(_root.clip3.__proto__, _root.theClass3.prototype);" 
                 ),
    SWFACTION_INIT);
    
  SWFDisplayItem_addAction(it3,
    newSWFAction(" _root.note('mc3.onClipConstruct'); "
                 " _root.check_equals(typeof(_root.clip3), 'movieclip'); "
                 " _root.check(_root.clip3.__proto__ != undefined); "
                 " _root.check_equals(_root.clip3.__proto__, _root.theClass3.prototype);"
                ),
    SWFACTION_CONSTRUCT);

  /* Place it again, no clip events this time */
  it4 = SWFMovie_add(mo, mc3);
  SWFDisplayItem_setName(it4, "noclipevs");

  SWFMovie_nextFrame(mo); /* end of frame3 */

  check_equals(mo, "_root.theClass3onLoadCalls.length", "3");
  check_equals(mo, "_root.theClass3onLoadCalls[0]", "_level0.clipevs");
  // Gnash gets the onLoad events of 'clip3' and 'noclipevs' swapped !!
  xcheck_equals(mo, "_root.theClass3onLoadCalls[1]", "_level0.clip3");
  xcheck_equals(mo, "_root.theClass3onLoadCalls[2]", "_level0.noclipevs"); /* it4 ... */
  add_actions(mo, "totals(27); stop();");
    
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
