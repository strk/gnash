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


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  7
#define OUTPUT_FILENAME  "KeyEventOrder.swf"

/// This test checks the event order of key events.
//
/// Known listeners are:
/// 1. MovieClips with a defined key event
/// 2. Button with a defined key event
/// 3. Anything added to Key listeners in ActionScript.
//
/// The test adds objects in this order:
//
/// Frame 1:
/// 1. mc1
/// 3. button1 (responds to 'a')
/// 3. o1 (actionscript key listener object)
/// 4. mc2
///
/// Frame 2:
/// 5. button2 (responds to 'a')
/// 6. button3 (responds to 'b')
/// 3. o2 (actionscript key listener object)
//
/// The test shows that, irrespective of construction order:
/// 1. MovieClips are notified first
/// 2. ActionScript listeners are notified second.
/// 3. Buttons are notified last.
//
/// Additionally:
//
/// 1. Only one button action can respond to any key.
int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc, dejagnuclip;
  SWFButtonRecord br;
  SWFButton bu;
  SWFDisplayItem it1, it, it2;
  SWFShape sh1, sh2;

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

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir),
          10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);

  SWFMovie_nextFrame(mo);

  add_actions(mo,
          "_root.order = '';"
          "_root.note('Press \"a\" then \"b\"');"
          "_root.note('Do not press any other keys!');"
          );

  mc = newSWFMovieClip();
  it = SWFMovie_add(mo, (SWFBlock)mc);
  SWFDisplayItem_setName(it, "mc1");
  SWFDisplayItem_addAction(it,
          newSWFAction("trace('mc1'); _root.order += 'mc1,';"),
          SWFACTION_KEYDOWN);

  bu = newSWFButton();
  
  sh1 = make_fill_square(200, 0, 40, 40, 0, 0, 0, 0, 0, 0);
  sh2 = make_fill_square(200, 0, 40, 40, 0, 0, 0, 0, 255, 0);

  br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_HIT);
  SWFButtonRecord_setDepth(br, 3);
  
  br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_UP);
  SWFButtonRecord_setDepth(br, 4);

  br = SWFButton_addCharacter(bu, (SWFCharacter)sh2, SWFBUTTON_OVER);
  SWFButtonRecord_setDepth(br, 5);

  SWFButton_addAction(bu,
          newSWFAction("trace('button1'); _root.order += 'button1,';"),
		  SWFBUTTON_KEYPRESS('a'));

  it1 = SWFMovie_add(mo, (SWFBlock)bu);
  SWFDisplayItem_setName(it, "button1");

  SWFMovie_add(mo, newSWFAction(
              "o1 = {};"
              "o1.onKeyDown = function() {"
              "    trace('o1'); "
              "    _root.order += 'o1,';"
              "};"
              "Key.addListener(o1);"
              ));

  mc = newSWFMovieClip();
  it = SWFMovie_add(mo, (SWFBlock)mc);
  SWFDisplayItem_setName(it, "mc2");
  SWFDisplayItem_addAction(it,
          newSWFAction("trace('mc2'); _root.order += 'mc2,';"),
          SWFACTION_KEYDOWN);

  SWFMovie_nextFrame(mo); // Frame 2

  bu = newSWFButton();
  
  sh1 = make_fill_square(240, 0, 40, 40, 0, 0, 0, 0, 0, 0);
  sh2 = make_fill_square(240, 0, 40, 40, 0, 0, 0, 0, 255, 0);

  br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_HIT);
  SWFButtonRecord_setDepth(br, 3);
  
  br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_UP);
  SWFButtonRecord_setDepth(br, 4);

  br = SWFButton_addCharacter(bu, (SWFCharacter)sh2, SWFBUTTON_OVER);
  SWFButtonRecord_setDepth(br, 5);

  SWFButton_addAction(bu,
          newSWFAction("trace('button2'); _root.order += 'button2,';"),
		  SWFBUTTON_KEYPRESS('a'));

  it2 = SWFMovie_add(mo, (SWFBlock)bu);
  SWFDisplayItem_setName(it, "button2");
  
  bu = newSWFButton();
  
  sh1 = make_fill_square(280, 0, 40, 40, 0, 0, 0, 0, 0, 0);
  sh2 = make_fill_square(280, 0, 40, 40, 0, 0, 0, 0, 255, 0);

  br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_HIT);
  SWFButtonRecord_setDepth(br, 3);
  
  br = SWFButton_addCharacter(bu, (SWFCharacter)sh1, SWFBUTTON_UP);
  SWFButtonRecord_setDepth(br, 4);

  br = SWFButton_addCharacter(bu, (SWFCharacter)sh2, SWFBUTTON_OVER);
  SWFButtonRecord_setDepth(br, 5);

  SWFButton_addAction(bu,
          newSWFAction(
              "trace('button3'); "
              "_root.order += 'button3,';"
              "play();"
              ),
		  SWFBUTTON_KEYPRESS('b'));

  it = SWFMovie_add(mo, (SWFBlock)bu);
  SWFDisplayItem_setName(it, "button3");
  
  SWFMovie_add(mo, newSWFAction(
              "o2 = {};"
              "o2.onKeyDown = function() {"
              "    trace('o2'); "
              "    _root.order += 'o2,';"
              "};"
              "Key.addListener(o2);"
              "stop();"
              ));

  SWFMovie_nextFrame(mo);

  check_equals(mo, "_root.order",
          "'mc2,mc1,o1,o2,button1,mc2,mc1,o1,o2,button3,'");
  
  SWFMovie_nextFrame(mo);

  SWFDisplayItem_remove(it2);
  
  add_actions(mo,
          "_root.order = '';"
          "_root.note('Press \"a\" then \"b\" again');"
          "_root.note('Do not press any other keys!');"
          "stop();"
          );
  SWFMovie_nextFrame(mo);

  // Check that removing the second button associated with 'a' does not
  // remove the key trigger for button2. There's no reason to think it should,
  // but it could happen if it's implemented badly!
  check_equals(mo, "_root.order",
          "'mc2,mc1,o1,o2,button1,mc2,mc1,o1,o2,button3,'");

  SWFMovie_add(mo, newSWFAction("stop();"));

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



