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
 *  Key events are provided by the testrunner or manual input. Neither is perfect.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  6
#define OUTPUT_FILENAME  "Keycodes_test.swf"



int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  dejagnuclip;
  SWFDisplayItem  it;

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

  add_actions(mo,
    "var keys = array();"
    "var keynames = array();"
    
    /* Basic test for keys (Good for testrunner) */
    "keys[0] = array (39, 65, 66);"
    "keynames[0] = ('right arrow, A, B');"

    /* More basic tests (Good for testrunner) */    
    "keys[1] = array (13, 17, 112);"
    "keynames[1] = ('Ctrl, Enter, F1');"  

    /* Tests for extended ascii. On many keyboards, some keys should result
    in multiple keycodes being marked as down, but we can't tell which keys
    or keycodes without knowing the keyboard layout. My Essszet (ß) should have
    codes 223 ('ß'), 220 ('\'), and 191 ('?').
    So the test is good for checking isDown for the main DisplayObjects, but the passes
    for !isDown can be bogus, depending on keyboard.    
     */
    "keys[2] = array (223, 228);"
    "keynames[2] = ('Extended ascii: (no shift) a-Umlaut, Essszet\n"
    "(Not a reliable test, because the keycodes supposed to be down are "
    "keymap dependent.');"

    /* More basic tests. (Good for testrunner)*/    
    "keys[3] = array (16, 34, 222);"
    "keynames[3] = ('shift, page-down, doublequote');" 

    /* Test indifference to shift for alphabetic DisplayObjects ... */
    "keys[4] = array (16, 65, 70);"
    "keynames[4] = ('keypress order: press shift, A, F');"

    /* ... Releasing shift before the DisplayObject keys should still remove A
    from the isDown array. Good for testrunner. */
    "keys[5] = array (70, 74);"
    "keynames[5] = ('key order: release shift, release A, press j');"

    /* Test indifference to shift for non-alphabetic DisplayObjects ... */    
    "keys[6] = array (16, 49, 222);"
    "keynames[6] = ('keypress order: shift, !, doublequote');"

    /* ... Releasing shift before the DisplayObject keys should remove ! and " from the
    isDown array. This will not work on non-US keyboards, where " and 2 are on the
    same key. But testing this automatically is not possible in this framework. */
    "keys[7] = array ();"
    "keynames[7] = ('key release order: release shift, release !, release doublequote.\n"
    "Will fail manual test on non-US keyboards');"

    /* Test indifference to shift for extended-ascii alphabetic DisplayObjects ... */    
    "keys[8] = array (228, 246);"
    "keynames[8] = ('(no shift: a-umlaut, o-umlaut');"

    /* ... ä and Ä have different keycodes, but releasing one should mean the other
    is removed from the isDown array when they are on the same key. In fact,
    pressing either on a German keyboard means that codes for both are marked
    as down. Is this true when the DisplayObjects are not on the same key? Probably not. So
    this test might only be possible manually. */
    "keys[9] = array ();"
    "keynames[9] = ('key order: press shift, release A-Umlaut, release O-umlaut, release shift.\n"
    "Will fail manual test on some keyboards');"
    
    "var test = 0;"
    );

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);

  SWFMovie_nextFrame(mo);
   
  int i;
  for (i = 0; i <= 9; i++) {
 
  add_actions(mo, 

    "keys[test].push (256);"
    "_root.createEmptyMovieClip('dynamic_mc', -10);"
    "_root.note(keynames[test]);"
    "Key.addListener(dynamic_mc);"
    "_root.onMouseDown = function () { _root.nextFrame(); };"
    "stop();"
  );

  SWFMovie_nextFrame(mo);  // 2nd frame
  
  add_actions(mo, 
    "var j = 0;"
    "for(var i=0; i < 256; i++) {"
        "if (keys[test].length > 0 && i < keys[test][j]) {"
            "xcheck(!Key.isDown(i), 'Key '+ i + ' should not be marked as down!');"
         "} else {"
            "xcheck (Key.isDown(i), 'Key '+ i + ' should be marked as down!');"
            "j++;"
         "};"
      "};"
      "test++;" /* Go on to next test array */
      "stop();"
  );
  }
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
