// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//

// Test case for MovieClipLoader ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: MovieClipLoader.as,v 1.3 2007/07/26 03:41:19 strk Exp $";

#include "check.as"

// MovieClipLoader was added in player7
#if OUTPUT_VERSION >= 7

check_equals(typeOf(MovieClipLoader), 'function');

var mcl = new MovieClipLoader();
check_equals(typeOf(mcl), 'object');
check_equals(typeOf(mcl.addListener), 'function');
check_equals(typeOf(mcl.getProgress), 'function');
check_equals(typeOf(mcl.loadClip), 'function');
check_equals(typeOf(mcl.removeListener), 'function');
check_equals(typeOf(mcl.unloadClip), 'function');
check(mcl instanceOf MovieClipLoader);

// TODO: test even handlers (actionscript.all framework
//       not enough for this)
//
// Invoked when a file loaded with MovieClipLoader.loadClip() has completely downloaded.
// MovieClipLoader.onLoadComplete
// 
// Invoked when a file loaded with MovieClipLoader.loadClip() has failed to load.
// MovieClipLoader.onLoadError
// 
// Invoked when the actions on the first frame of the loaded clip have been executed.
// MovieClipLoader.onLoadInit
// 
// Invoked every time the loading content is written to disk during the loading process.
// MovieClipLoader.onLoadProgress
//
// Invoked when a call to MovieClipLoader.loadClip() has successfully begun to download a file.
// MovieClipLoader.onLoadStart
//

#endif // OUTPUT_VERSION >= 7
