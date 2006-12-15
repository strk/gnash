/***********************************************************************
 *
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
 *
 * You should have received a copy of the GNU General Public License
 *
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 ***********************************************************************
 * zoulunkai,  zoulunkai@gmail.com
 * Test case for variable defined in main timeline:
 ***********************************************************************/


#include <stdio.h>
#include <ming.h>

#define OUTPUT_FILENAME "timeline_var_test.swf"


SWFAction  action_in_frame1()
{
	SWFAction ac;
	ac = compileSWFActionCode( "trace(var_at_frame2);" );
	return ac;
}

SWFAction  action_in_frame2()
{
	SWFAction ac;
	ac = compileSWFActionCode( "var var_at_frame2 = \"var_defined_at_frame2\";" );
	return ac;
}

SWFAction  action_in_frame3()
{
	SWFAction ac;
	ac = compileSWFActionCode( "gotoAndStop(1);" );
	return ac;
}

int main()
{
	SWFMovie  movie;
	SWFAction ac;


	Ming_init();
	movie = newSWFMovie();

	// Add frame ActionScipts to frames
	ac = action_in_frame1();
	SWFMovie_add(movie, (SWFBlock)ac);
	SWFMovie_nextFrame(movie); 
	
	ac = action_in_frame2();
	SWFMovie_add(movie, (SWFBlock)ac);
	SWFMovie_nextFrame(movie); 
	
	ac = action_in_frame3();
	SWFMovie_add(movie, (SWFBlock)ac);
	SWFMovie_nextFrame(movie); 
	
	//save files
	puts("Saving " OUTPUT_FILENAME );
	SWFMovie_save(movie, OUTPUT_FILENAME);

	return 0;
}
