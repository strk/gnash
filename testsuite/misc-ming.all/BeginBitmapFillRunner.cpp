//  
//    Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
//  
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// 
#define INPUT_FILENAME "BeginBitmapFill.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"

#include "check.h"
#include <string>
#include <cassert>
#include <sstream>

using namespace gnash;
using namespace gnash::geometry;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
    string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
    MovieTester tester(filename);

    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity(1);

    MovieClip* root = tester.getRootMovie();
    assert(root);

    const rgba white(255, 255, 255, 255);
    const rgba blue(0, 0, 255, 255);
    const rgba cyan(0, 255, 255, 255);
    const rgba green(0, 255, 0, 255);
    const rgba yellow(255, 255, 0, 255);
    const rgba magenta(255, 0, 255, 255);
    const rgba red(255, 0, 0, 255);
    
    // Only one frame 
    tester.advance();
    check_pixel(1, 1, 1, white, 8);

    // TODO: add real tests!

}

