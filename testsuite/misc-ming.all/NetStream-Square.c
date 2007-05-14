/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
 * Plays an external FLV video
 * Should be used with the MovieTester to test if the video decoder works.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "NetStream-ExtSquare.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);

  SWFVideoStream stream;
  SWFDisplayItem item;
  SWFAction a;
  char* buffer= 
  	"nc=new NetConnection();"
	"nc.connect(null);"
	"_global.stream = new NetStream(nc);"
	"video.attachVideo(_global.stream); "
	"_global.stream.setBufferTime(2); "
	"_global.stream.play(\"../media/square.flv\");";

  int video_width = 128;
  int video_height = 96;

  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 128, 96);

  SWFMovie_setRate(mo, 15);
 
  stream = newSWFVideoStream();
  SWFVideoStream_setDimension(stream, video_width, video_height);
  item = SWFMovie_add(mo, (SWFBlock)stream);
//  SWFDisplayItem_moveTo(item, (400 - video_width / 2), 0);
  SWFDisplayItem_setName(item, "video");

  a = newSWFAction(buffer);

  if(a == NULL) return -1;
  SWFMovie_add(mo, (SWFBlock)a);

  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
