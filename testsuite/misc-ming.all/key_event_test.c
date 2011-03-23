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
 */ 


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  6
#define OUTPUT_FILENAME  "key_event_test.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth)
{
  SWFShape  sh;
  SWFMovieClip mc;
  SWFDisplayItem it;

  mc = newSWFMovieClip();
  sh = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc, (SWFBlock)sh);  
  SWFMovieClip_nextFrame(mc);

  it = SWFMovie_add(mo, (SWFBlock)mc);
  SWFDisplayItem_setDepth(it, depth); 
  SWFDisplayItem_setName(it, name);

  return it;
}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFDisplayItem  it, it1, it2, it3;

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
  // low frame rate is needed for visual checking
  SWFMovie_setRate (mo, 1.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, 
    "test1=0; test2=0; test3=0; test4=0; test5='0'; test6=0; "
    "keyPressed=false; keyReleased=false;"
    "haslooped1=false; haslooped2=false;");
  SWFMovie_nextFrame(mo);  // _root frame1

  // test1: 
  //    (1)onKeyDown, onKeyPress and onKeyUp are not global functions
  //    (2)test that global Key object can be overridden
  //    (3)after overriden, previously registered handlers could still respond to new key events
  add_actions(mo, 
    "_root.var1 = 0; _root.var2 = 0;"
    "l = new Object();"
    "l.onKeyDown = function () { _root.note('l.onKeyDown'); _root.var1+=1; _root.Play(); }; "
    "l.onKeyUp = function () { _root.note('l.onKeyUp'); _root.var2+=1;}; "
    " Key.addListener(l);"
    "check_equals(typeof(Key), 'object');"
    "check_equals(typeof(onKeyUp), 'undefined');"
    "check_equals(typeof(onKeyDown), 'undefined');"
    "check_equals(typeof(onKeyPress), 'undefined');"
    "stop();"
    "_root.note('1. Press a single key to continue the test');"
  );
  SWFMovie_nextFrame(mo);  // _root frame2

  SWFMovie_nextFrame(mo);  // _root frame3
  
  add_actions(mo, 
    "stop();"
    "check_equals(var1, 1); "
    "check_equals(var2, 1); "
    "Key = 3;"
    "check_equals(typeof(Key), 'number');"
    "_root.note('2. Press a single key to continue the test');"
    );
  SWFMovie_nextFrame(mo);  // _root frame4
  
  SWFMovie_nextFrame(mo);  // _root frame5
  
  add_actions(mo,
    "stop();"
    "check_equals(var1, 2); "
    "check_equals(var2, 2);"
    "delete Key; "
    "check_equals(typeof(Key), 'object');"
    "Key.removeListener(l);"
    "_root.note('3. Press a single key to continue the test');"
    "obj1=new Object(); "
    " obj1.onKeyDown=function() {"
    "   _root.note('obj1.onKeyDown');"
    "   _root.play();"
    "}; "
    " Key.addListener(obj1); "
  );
  SWFMovie_nextFrame(mo);  // _root frame6
   
  add_actions(mo, 
    "check_equals(var1, 2);"
    "check_equals(var2, 2);"
    "Key.removeListener(obj1);"
    "delete l; delete obj1; "
  );
  SWFMovie_nextFrame(mo);  // _root frame7
  
  // test2:
  //    test removing of static clip key listeners
  SWFMovie_nextFrame(mo);  // _root frame8
  
  it = add_static_mc(mo, "listenerClip1", 20);
  SWFDisplayItem_addAction(it,
    newSWFAction(" _root.note('listenerClip2.onClipKeyDown'); "
                 " _root.test2++; "
                 "if(!_root.haslooped1){"
                 "   _root.haslooped1=true;"
                 "   _root.gotoAndPlay(_root._currentframe-1);"
                 "} else {"
                 "   _root.gotoAndPlay(_root._currentframe+1);"
                 "}"
                ), 
    SWFACTION_KEYDOWN);  
  add_actions(mo,
    "stop();"
    "_root.note('4. Press a single key to continue the test');"
  );
  SWFMovie_nextFrame(mo);  // _root frame9
  
  check_equals(mo, "_root.test2", "2");
  SWFDisplayItem_remove(it);
  SWFMovie_nextFrame(mo);  // _root frame10
  
  
  // test3:
  //    test removing of dynamic sprite key listeners 
  SWFMovie_nextFrame(mo);  // _root frame11
  
  add_actions(mo, 
    "stop();"
    "_root.note('5. Press a single key to continue the test');"
    "_root.createEmptyMovieClip('dynamic_mc', -10);"
    "dynamic_mc.onKeyDown = function() "
    "{"
    "   _root.note('dynamic_mc.onKeyDown triggered');"
    "   _root.check_equals(this, _root.dynamic_mc);"
    "   _root.test3++;"
    "   if(!_root.haslooped2){"
    "       _root.haslooped2=true;"
    "       _root.gotoAndPlay(_root._currentframe-1);"
    "       _root.check_equals(_root._currentframe, 11);"
    "   } else {"
    "       _root.gotoAndPlay(_root._currentframe+1);"
    "       _root.check_equals(_root._currentframe, 13);"
    "   }"
    "};"
    "Key.addListener(dynamic_mc);"
  );
  SWFMovie_nextFrame(mo);  // _root frame12
  
  check_equals(mo, "_root.test3", "2");
  add_actions(mo, "dynamic_mc.swapDepths(10);  dynamic_mc.removeMovieClip();");
  SWFMovie_nextFrame(mo);  // _root frame13
  
  // test4:
  //    GC test
  add_actions(mo, 
    "_root.note('6. Press a single key to continue the test');"
    " obj2 = new Object(); "
    " obj2.x = 100; "
    " obj2.onKeyDown = function () { "
    "   _root.note('obj2.onKeyDown triggered');"
    "   _root.test4++; "
    "   _root.objRef = this; "
    "   _root.play();"
    " };" 
    " Key.addListener(obj2); "
    // After deleting obj2, we still have a key listener kept alive!
    " delete obj2; "
    " stop();"
  );
  check_equals(mo, "_root.test4", "0");
  SWFMovie_nextFrame(mo);  // _root frame14
  
  check_equals(mo, "objRef.x", "100");
  check_equals(mo, "_root.test4", "1");
  add_actions(mo,
    "stop();"
    "_root.note('7. Press a single key to continue the test');"
    "Key.removeListener(objRef); "
    // check that objRef is still alive
    "check_equals(typeof(objRef), 'object');"
    // delete the objRef, no object and no key listener now.
    "delete objRef;"
    "obj3=new Object(); "
    "obj3.onKeyDown=function() {"
    "   _root.note('obj3.onKeyDown');"
    "   _root.gotoAndPlay(_currentframe+1);"
    "}; "
    "Key.addListener(obj3); "
  );
  SWFMovie_nextFrame(mo);  // _root frame15
  
  check_equals(mo, "_root.test4", "1");
  add_actions(mo, 
    "Key.removeListener(obj3);"
    "delete obj3; "
  );
  SWFMovie_nextFrame(mo);  // _root frame16

  // test5:
  //   test key listeners invoking order.
  //   expected behaviour:
  //   (1)for DisplayObject key listeners, first added last called
  //   (2)for general object listeners, first added first called
  //   (3)for DisplayObject listeners, user defined onKeyDown/Up won't be called
  //      if not registered to the global Key object.
  it1 = add_static_mc(mo, "ls1", 30);
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(
       "_root.note('ls1.onClipKeyDown');"
       "_root.test5 += '+ls1';"
    ),
    SWFACTION_KEYDOWN);
  SWFMovie_nextFrame(mo);  // _root frame17
  
  it2 = add_static_mc(mo, "ls2", 31);
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(
       "_root.note('ls2.onClipKeyDown');"
       "_root.test5 += '+ls2';"
    ),
    SWFACTION_KEYDOWN);
  SWFMovie_nextFrame(mo);  // _root frame18
   
  it3 = add_static_mc(mo, "ls3", 29);
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(
       "_root.note('ls3.onClipKeyDown');"
       "_root.test5 += '+ls3';"
    ),
    SWFACTION_KEYDOWN);
  SWFMovie_nextFrame(mo);  // _root frame19

  add_actions(mo, 
    "obj1=new Object();"
    "obj1.onKeyDown = function () { "
    "  _root.note('obj1.onKeyDown');"
    "  _root.test5 += '+obj1'; "
    "  _root.gotoAndPlay(_root._currentframe+1);"
    "}; "
    "Key.addListener(obj1);"
    "ls1.onKeyDown = function () {"
    "  _root.note('ls1.onKeyDown');"
    "  _root.test5 += '+ls1';"
    "}; "
    "Key.addListener(ls1);"
    "obj2=new Object();"
    "obj2.onKeyDown = function () {"
    "  _root.note('obj2.onKeyDown');"
    "  _root.test5 += '+obj2';"
    "}; "
    "Key.addListener(obj2);"
    "ls2.onKeyDown = function () {"
    "  _root.note('ls2.onKeyDown');"
    "  _root.test5 += '+ls2';"
    "}; "
    "Key.addListener(ls2);"
    "obj3=new Object();"
    "obj3.onKeyDown = function () {"
    "  _root.note('obj3.onKeyDown');"
    "  _root.test5 += '+obj3';"
    "}; "
    "Key.addListener(obj3);"
    "ls3.onKeyDown = function () {"
    "  _root.note('ls3.onKeyDown');"
    "  _root.test5 += '+ls3';"
    "}; "
    "stop(); "
    "_root.note('8. Press a single key to continue the test');"
  );
  SWFMovie_nextFrame(mo);  // _root frame20

  SWFMovie_nextFrame(mo);  // _root frame21
  
  add_actions(mo,
    "stop(); "
    "_root.note('9. Press a single key to continue the test');"
  );
  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  SWFDisplayItem_remove(it3);
  SWFMovie_nextFrame(mo);  // _root frame22
 
  check_equals(mo, "test5", "'0+ls3+ls2+ls1+obj1+ls1+obj2+ls2+obj3+obj1+obj2+obj3'");

  add_actions(mo,
     "o = new Object();"
     "_root.t = '';"
     "o.onKeyDown = function() { t = _root.ff.text; play(); };"
     "Key.addListener(o);"
     "_root.createTextField('ff', 987, 300, 20, 200, 40);"
     "_root.ff.type = 'input';"
     "_root.ff.text = 'Input here';"
     "_root.ff.border = true;"
    "_root.note('10. Click on the TextField and type \"i\"');"
    "stop();"
  );
  
  SWFMovie_nextFrame(mo);  // _root frame23

  // The listener is called before text is updated!
  check_equals(mo, "_root.t", "'Input here'");
  check_equals(mo, "_root.ff.text", "'Input herei'");


  add_actions(mo, "totals(); stop();");
  SWFMovie_nextFrame(mo);  // _root frame24
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



