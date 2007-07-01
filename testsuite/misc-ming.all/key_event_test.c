/*
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
 *  Key events are: KeyUp, KeyDown, KeyPress
 *  (1)KeyUp and KeyDown event handler is unrelated to any key;
 *  (2)KeyPress event handler should bind a valid key, which is hard-coded in the placement tag;
 *  (3)Use Key.addListener() to register user defined key event handler, otherwise it will not
 *     be triggered.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  6
#define OUTPUT_FILENAME  "key_event_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc, dejagnuclip;
  SWFDisplayItem  it;
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
  SWFMovie_setRate (mo, 1.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, "x1=0; x2=0; x3=0; x4=0; x5=0; x6=0; ");
  SWFMovie_nextFrame(mo);  /* 1st frame */

  
  mc = newSWFMovieClip();
  sh_red = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc, (SWFBlock)sh_red);  
  SWFMovieClip_nextFrame(mc);


  it = SWFMovie_add(mo, (SWFBlock)mc); 
  SWFDisplayItem_setDepth(it, 20); 
  SWFDisplayItem_setName(it, "mc"); 
  /* Define onClipKeyDown */
  SWFDisplayItem_addAction(it,
    newSWFAction(" _root.x1 = Key.getAscii(); "
                " _root.note('onClipKeyDown triggered'); "
                " _root.note('key ASCII value: ' + _root.x1); "), 
    SWFACTION_KEYDOWN); 
  /* Define onClipKeyUp */
  SWFDisplayItem_addAction(it,
    newSWFAction(" _root.x2 = Key.getCode(); "
                 " _root.note('onClipKeyUp triggered'); "
                 " _root.note('key code: ' + _root.x2); "), 
    SWFACTION_KEYUP); 
  /* Define onClipKeyUp. Question: how to bind a key code with the KeyPress event??? */ 
  SWFDisplayItem_addAction(it,
    newSWFAction(" _root.note('onClipKeyPress triggered'); "
                 " _root.x3 = _root.x1; "), 
    SWFACTION_KEYPRESS);     
    
  /* add user defined events */
  add_actions(mo, " mc.onKeyDown = function () { " 
                  " _root.x4 = Key.getAscii(); "
                  " _root.note('user defined onKeyDown triggered');  "
                  " _root.note('key ASCII value: ' + _root.x4); }; "
                  
                  " mc.onKeyUp = function () { "
                  " _root.x5 = Key.getCode(); "
                  " _root.note('user defined onKeyUp triggered'); "
                  " _root.note('key code: ' +  _root.x5);  }; "
                  
                  " mc.onKeyPress = function () { "
                  " _root.x6 = _root.x3; "
                  " _root.note('user defined onKeyPress triggered'); }; "
              );
  
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  int frame_num = 3;
  for(frame_num; frame_num<=30; frame_num++)
  {
      SWFMovie_nextFrame(mo); 
  }
  
  add_actions(mo, "_root.note('mc registered by addListener()'); ");
  /* register "mc" to receive onKeyDown and onKeyUp notification after 30 seconds(30 frames). */
  add_actions(mo, " Key.addListener(mc); stop(); ");
  SWFMovie_nextFrame(mo); /* 31th frame */
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}


