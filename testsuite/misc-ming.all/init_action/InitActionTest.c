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
 */

/*
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * Test for DoInitAction tag.
 *
 * Tags order(compiled with Ming0.4.beta4):
 *   DefineMovieClip(mc1)
 *   DoInitAction(mc1)
 *   PlaceObject2(mc1)
 *   DefineMovieClip(mc2)
 *   DoInitAction(mc2);
 *   PlaceObject2(mc2);
 *
 * Tests show that the actions order is like this:
 *     mc1.init_actions
 *     mc1.onClipInitialize
 *     mc2.init_actions
 *     mc2.onClipInitialize
 *     mc1.onClipConstruct
 *     mc2.onClipConstruct
 *     _root.actions
 *     mc1.onClipLoad
 *     mc1.actions
 *     mc2.onClipLoad
 *     mc2.actions
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "InitActionTest.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
  SWFShape  sh1, sh2;

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

  add_actions(mo, " _root.x = '0+'; ");
  SWFMovie_nextFrame(mo); /* 1st frame */


  mc1 = newSWFMovieClip();
  sh1 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh1);
  /* add init actions */
  add_clip_init_actions(mc1, " _root.note('mc1.init_actions'); "
                             " _root.x += '1+'; " 
                             " y = 'var_of_root'; " );

  /* add actions */
  add_clip_actions(mc1, " _root.note('mc1.actions');  _root.x += '9+'; ");
  SWFMovieClip_nextFrame(mc1);//1st frame

  mc2 = newSWFMovieClip();
  sh2 = make_fill_square (600, 600, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc2, (SWFBlock)sh2);
  /* add init actions */
  add_clip_init_actions(mc2, " _root.note('mc2.init_actions'); _root.x += '3+'; ");
  /* add actions */
  add_clip_actions(mc2, " _root.note('mc2.actions');  _root.x += '11+'; ");
  SWFMovieClip_nextFrame(mc2);//1st frame
  

  /* add mc1 to _root */
  SWFDisplayItem it1;
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);
  SWFDisplayItem_setDepth(it1, 10);
  SWFDisplayItem_setName(it1, "mc1");

  SWFDisplayItem_addAction(it1,
    newSWFAction(" _root.note('mc1.onClipInitialize'); _root.x += '2+'; "
	"_root.check_equals(this.__proto__, MovieClip.prototype);"
	),
    SWFACTION_INIT);
    
  SWFDisplayItem_addAction(it1,
    newSWFAction(" _root.note('mc1.onClipConstruct');  _root.x += '5+'; "),
    SWFACTION_CONSTRUCT);
    
  SWFDisplayItem_addAction(it1,
    newSWFAction(" _root.note('mc1.onClipLoad');  _root.x += '8+'; "),
    SWFACTION_ONLOAD);


  /* add mc2 to _root */
  SWFDisplayItem it2;
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);
  SWFDisplayItem_setDepth(it2, 11);
  SWFDisplayItem_setName(it2, "mc2");
 
   SWFDisplayItem_addAction(it2,
    newSWFAction(" _root.note('mc2.onClipInitialize'); _root.x += '4+'; "),
    SWFACTION_INIT);
    
  SWFDisplayItem_addAction(it2,
    newSWFAction(" _root.note('mc2.onClipConstruct');  _root.x += '6+'; "),
    SWFACTION_CONSTRUCT);
    
  SWFDisplayItem_addAction(it2,
    newSWFAction(" _root.note('mc2.onClipLoad');  _root.x += '10+'; "),
    SWFACTION_ONLOAD);
    
    
  /* add main timeline actions */
  add_actions(mo, "_root.note('_root.actions');  _root.x += '7+'; ");
  SWFMovie_nextFrame(mo); /* 2nd frame */

  /* The check below used to succeeds, and started failing when
   * executing init actions "after" DLIST tags.
   * Should be fixed if we postpone the call to ::construct
   * to "after" init actions are executed, which would require
   * some book keeping in sprite_instance class
   */
  check_equals(mo, "_root.x", "'0+1+2+3+4+5+6+7+8+9+10+11+'");

  check_equals(mo, "_root.y", "'var_of_root'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
