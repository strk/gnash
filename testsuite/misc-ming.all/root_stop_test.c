/* 
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * Test for stopping and playing the _root
 * 
 * The _root movie has two frames. 
 * In frame1, there's a sprite which contains 3 frames.
 *    In each of the sprite's 3 frames, there is a red square. 
 * In frame2, there is black square.
 *
 * The sprite will check the counter defined in _root, and then
 * conditionally stops and plays the _root.
 *
 * Normally, you can see both the red and black squares.
 *
 * run as ./root_stop_test
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "root_stop_test.swf"


//actions in _root movie, defines a counter
SWFAction  action_in_root(void);
SWFAction  action_in_root()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
      counter = 0; \
  ");
  return ac;
}

//actions in sprite, conditionally stops and plays the _root
SWFAction  action_in_sprite(void);
SWFAction  action_in_sprite()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
      counter++; \
      if(counter < 2) \
          _root.stop(); \
      else \
          _root.play(); \
  ");
  return ac;
}

int
main(int argc, char** argv)
{
	SWFMovie mo;
	SWFMovieClip mc;
	// SWFMovieClip dejagnuclip;
	SWFShape  sh1,sh2;
	SWFAction ac1, ac2;
	int i;

	const char *srcdir=".";
	if ( argc>1 ) 
		srcdir=argv[1];
	else
	{
   		//fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
   		//return 1;
	}

	Ming_init();
	mo = newSWFMovie();
	SWFMovie_setDimension(mo, 800, 600);

	//dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
	//SWFMovie_add(mo, (SWFBlock)dejagnuclip);
	//SWFMovie_nextFrame(mo); 

	ac2 = action_in_sprite();
	sh2 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
	mc = newSWFMovieClip();
	
	for(i=0; i<3; i++)
	{
		SWFMovieClip_add(mc, (SWFBlock)sh2);  
		SWFMovieClip_add(mc, (SWFBlock)ac2);
		SWFMovieClip_nextFrame(mc);
	}
	
	SWFDisplayItem it;
	ac1 =  action_in_root();
	SWFMovie_add(mo, (SWFBlock)ac1);
	it = SWFMovie_add(mo, (SWFBlock)mc);  //add the movieClip to the _root
	SWFDisplayItem_setName(it, "mc_in_root"); //name the movieclip
	SWFMovie_nextFrame(mo); 

	
	sh1 = make_fill_square(270, 270, 120, 120, 255, 0, 0, 0, 0, 0);
	SWFMovie_add(mo, (SWFBlock)sh1); //add the black square to the _root's 2nd frame
	SWFMovie_nextFrame(mo); 

	//Output movie
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(mo, OUTPUT_FILENAME);

	return 0;
}
