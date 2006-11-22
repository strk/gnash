// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Test case for Video ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Video.as,v 1.7 2006/11/22 13:05:38 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION < 6

xcheck_equals(Video, undefined);

#else

check(Video);

var videoObj = new Video;

// test the Video constuctor
check (videoObj != undefined);

// test the Video::attachVideo method
check (videoObj.attachVideo != undefined);
// test the Video::clear method
check (videoObj.clear != undefined);

#endif
