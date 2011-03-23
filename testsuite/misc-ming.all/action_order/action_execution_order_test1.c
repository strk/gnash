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
 * Within the same frame, ActionScript in DoAction tags can referance movieClip 
 * placed after DoAction tags. 
 *
 * Within the same frame, ActionScript in onClipLoad handlers can referance
 * movieClip placed after the PlaceObject2 tag defining the clip event. 
 *
 * The actual order of tags are dependent on compiler, so you need to 
 * verify first if the order of tags is what you expect.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test1.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc_red, mc_blu, dejagnuclip;
  SWFShape  sh_red, sh_blu;
  SWFDisplayItem it_red, it_blu;

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

  /* This will add a DoAction Tag, which be compiled before PlaceObject(mc_red) tag. */
  check_equals(mo, "typeof(_root.mc_red)", "'movieclip'");
  
  mc_red = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red, (SWFBlock)sh_red);  
  SWFMovieClip_nextFrame(mc_red);//1st frame

  mc_blu = newSWFMovieClip();
  sh_blu = make_fill_square (0, 300, 60, 60, 0, 0, 255, 0, 0, 255);
  SWFMovieClip_add(mc_blu, (SWFBlock)sh_blu);  
  SWFMovieClip_nextFrame(mc_blu);//1st frame
 
  /*
   * Add mc_red to _root and name it as "mc_red"
   * Have onClipLoad reference mc_blu (yet to be placed)
   */
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red);  
  SWFDisplayItem_addAction(it_red,
    compileSWFActionCode(
	"_root.mcRedLoadCalls++;"
	"_root.typeofMcBluFromMcRedLoad = typeof(_root.mc_blu);"
    	"_root.typeofMcRedFromMcRedLoad = typeof(_root.mc_red);"
	), SWFACTION_ONLOAD);
  SWFDisplayItem_setDepth(it_red, 3); 
  SWFDisplayItem_setName(it_red, "mc_red"); 

  /*
   * Add mc_blu to _root and name it as "mc_blu"
   * Have onClipLoad reference mc_red (placed before)
   */
  it_blu = SWFMovie_add(mo, (SWFBlock)mc_blu);  
  SWFDisplayItem_addAction(it_blu,
    compileSWFActionCode(
	"_root.mcBluLoadCalls++;"
	"_root.typeofMcRedFromMcBluLoad = typeof(_root.mc_red);"
    	"_root.typeofMcBluFromMcBluLoad = typeof(_root.mc_blu);"
	), SWFACTION_ONLOAD);
  SWFDisplayItem_setDepth(it_blu, 4); 
  SWFDisplayItem_setName(it_blu, "mc_blu"); 
  SWFDisplayItem_moveTo(it_blu, 200, 0);

  SWFMovie_nextFrame(mo); /* 2nd frame */

  // onLoad handler for mc_red DO can see mc_blu (not yet placed)
  check_equals(mo, "_root.typeofMcBluFromMcRedLoad", "'movieclip'");

  // onLoad handler for mc_red DO can see itself 
  check_equals(mo, "_root.typeofMcRedFromMcRedLoad", "'movieclip'");

  // onLoad handler for mc_blu DO can see mc_red (placed before)
  check_equals(mo, "_root.typeofMcRedFromMcBluLoad", "'movieclip'");

  // onLoad handler for mc_blu DO can see itself 
  check_equals(mo, "_root.typeofMcBluFromMcBluLoad", "'movieclip'");

  // onLoad handlers of mc_red and mc_blu called once
  check_equals(mo, "_root.mcBluLoadCalls", "1");
  check_equals(mo, "_root.mcRedLoadCalls", "1");

  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 3rd frame */

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



