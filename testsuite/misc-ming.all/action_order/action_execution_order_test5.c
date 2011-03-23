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
 *
 * For nested movieClips:
 * (1) parent onLoad called first; 
 * (2) child unload called first;  
 * (3) child onEnterFrame called first;
 *
 * yes, parent could be a child of another parent,
 *    and child could be a parent of another child:)
 * Just always keep (1)(2)(3) in mind.
 *
 * movieClips hiberarchy:
 * _root  (5 frames)
 *   |----dejagnuclip(placed at 1st frame of main timeline)
 *   |----mc1
 *         |----mc11
 *         |----mc12
 *               |----mc121
 *                      |----mc1211    
 *
 * expected onClipEvents order:
 * mc1.OnLoad;
 * mc11.OnLoad;
 * mc12.OnLoad; mc121.OnLoad; mc1211.OnLoad;
 * mc1211.OnEnterFrame; mc121.OnEnterFrame; mc12.OnEnterFrame
 * mc11.OnEnterFrame; 
 * mc1.OnEnterFrame;
 * mc1211.OnUnload; mc121.OnUnload; mc12.OnUnload;
 * mc11.OnUnload;
 * mc1.OnUnload;
 *
 * The actual order of tags are dependent on compiler, so you need to 
 * verify first if the order of tags is what you expect.  
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test5.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc11, mc12, mc121, mc1211, dejagnuclip;
  SWFDisplayItem it1, it11, it12, it121, it1211;

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
  SWFMovie_nextFrame(mo); /* 1st frame of _root */

  /*===================== Start of defining movieClips ==========================*/
  
  mc1211 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc1211); /* mc1211, 1st frame */
  SWFMovieClip_nextFrame(mc1211); /* mc1211, 2nd frame */
    
  mc121 = newSWFMovieClip();
  
  /* add mc1211 to mc121 and name it as "mc1211" */
  it1211 = SWFMovieClip_add(mc121, (SWFBlock)mc1211);  
  SWFDisplayItem_setDepth(it1211, 1); 
  SWFDisplayItem_setName(it1211, "mc1211"); 
  
  /* add onClipEvents */
  SWFDisplayItem_addAction(it1211, // the inner most child
    compileSWFActionCode(" _root.note('mc1211 onInitialize called'); "
                         " _root.check_equals(this.__proto__, MovieClip.prototype);"
                         " _root.check_equals(this._parent.__proto__, MovieClip.prototype);"
                         " _root.check_equals(this._parent._parent.__proto__, MovieClip.prototype);"),
    SWFACTION_INIT);

  SWFDisplayItem_addAction(it1211,
    compileSWFActionCode(" _root.note('mc1211 onLoad called'); "
                         " _root.x1 += '5+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it1211,
    compileSWFActionCode(" _root.note('mc1211 onUnload called'); "
                         " _root.x1 += '11+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_addAction(it1211,
    compileSWFActionCode(" _root.note('mc1211 onEnterFrame called'); "
                         " _root.x1 += '6+'; "),
    SWFACTION_ENTERFRAME);
    
  SWFMovieClip_nextFrame(mc121); /* mc121, 1st frame */
  SWFMovieClip_nextFrame(mc121); /* mc121, 2nd frame */

  
  mc12 = newSWFMovieClip();
  /* add mc121 to mc12 and name it as "mc121" */
  it121 = SWFMovieClip_add(mc12, (SWFBlock)mc121);   
  SWFDisplayItem_setDepth(it121, 1); 
  SWFDisplayItem_setName(it121, "mc121"); 
  
  /* add onClipEvents */
  SWFDisplayItem_addAction(it121,
    compileSWFActionCode(" _root.note('mc121 onLoad called'); "
                         " _root.x1 += '4+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it121,
    compileSWFActionCode(" _root.note('mc121 onUnload called'); "
                         " _root.x1 += '12+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_addAction(it121,
    compileSWFActionCode(" _root.note('mc121 onEnterFrame called'); "
                         " _root.x1 += '7+'; "),
    SWFACTION_ENTERFRAME);
    
  SWFMovieClip_nextFrame(mc12); /* mc12, 1st frame */
  SWFMovieClip_nextFrame(mc12); /* mc12, 2nd frame */
  
  
  mc11 = newSWFMovieClip(); 
  SWFMovieClip_nextFrame(mc11); /* mc11, 1st frame */
  SWFMovieClip_nextFrame(mc11); /* mc11, 2nd frame */
 
 
  mc1 = newSWFMovieClip();
  
  /* add mc11 to mc1 and name it as "mc11" */
  it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11);   
  SWFDisplayItem_setDepth(it11, 2); 
  
  /* add onClipEvents */
  SWFDisplayItem_setName(it11, "mc11"); 
    SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onLoad called'); "
                         " _root.x1 += '2+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onUnload called'); "
                         " _root.x1 += '14+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onEnterFrame called'); "
                         " _root.x1 += '9+'; "),
    SWFACTION_ENTERFRAME);
    
  /* add mc12 to mc1 and name it as "mc12" */
  it12 = SWFMovieClip_add(mc1, (SWFBlock)mc12);   
  SWFDisplayItem_setDepth(it12, 1); 
  SWFDisplayItem_setName(it12, "mc12"); 
  
  /* add onClipEvents */
  SWFDisplayItem_addAction(it12, 
    compileSWFActionCode(" _root.note('mc12 onInitialize called'); "
                         " _root.check_equals(this, _root.mc1.mc12);"
                         " _root.check_equals(this.__proto__, MovieClip.prototype);"
                         " _root.check_equals(this._parent.__proto__, MovieClip.prototype);"
                         " _root.check_equals(this._parent.mc11.__proto__, MovieClip.prototype);"
                         // test child __proto__
                         " _root.check_equals(this.mc121.__proto__, MovieClip.prototype);"),
    SWFACTION_INIT);

  SWFDisplayItem_addAction(it12, // the inner most child
    compileSWFActionCode(" _root.note('mc12 onConstruct called'); "
                         " _root.check_equals(this, _root.mc1.mc12);"
                         " _root.check_equals(this.__proto__, MovieClip.prototype);"
                         " _root.check_equals(this._parent.__proto__, MovieClip.prototype);"
                         " _root.check_equals(this._parent.mc11.__proto__, MovieClip.prototype);"
                         " _root.check_equals(this.mc121.__proto__, MovieClip.prototype);"),
    SWFACTION_CONSTRUCT);
    
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onLoad called'); "
                         " _root.x1 += '3+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onUnload called'); "
                         " _root.x1 += '13+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onEnterFrame called'); "
                         "  _root.x1 += '8+'; "),
    SWFACTION_ENTERFRAME);
    
  SWFMovieClip_nextFrame(mc1); /* mc1, 1st frame */
  SWFMovieClip_nextFrame(mc1); /* mc1, 2nd frame */
  
  /*===================== End of defining movieClips ==========================*/
  
  

  it1 = SWFMovie_add(mo, (SWFBlock)mc1); 
  
  /* add onClipEvents */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onLoad called'); "
                         " _root.x1 += '1+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onUnload called'); "
                         " _root.x1 += '15+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onEnterFrame called'); "
                         " _root.x1 += '10+'; "),
    SWFACTION_ENTERFRAME);
 
  
  /* place _root.mc1 */
  SWFDisplayItem_setDepth(it1, 10); 
  SWFDisplayItem_setName(it1, "mc1"); 
  SWFMovie_nextFrame(mo); /* 2nd frame of _root */


  SWFMovie_nextFrame(mo); /* 3rd frame of _root */
  
  /* remove _root.mc1 */
  SWFDisplayItem_remove(it1);  
  SWFMovie_nextFrame(mo); /* 4th frame of _root */

  /* checks */
  check_equals(mo, "_root.x1", "'1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame of _root */
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
