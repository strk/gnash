//  
//    Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//    Free Software Foundation, Inc.
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
#define INPUT_FILENAME "BitmapDataTest.swf"

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
    
    // Frame 1
    tester.advance();
    check_pixel(15, 15, 1, yellow, 8);
    check_pixel(80, 80, 1, green, 8);
    check_pixel(95, 95, 1, green, 8);
    
    // Frame 2
    tester.click();
    tester.advance();
    check_pixel(15, 15, 1, yellow, 8);
    check_pixel(80, 80, 1, green, 8);
    check_pixel(95, 95, 1, blue, 8);
    check_pixel(115, 115, 1, blue, 8);
    
    // Frame 3
    tester.click();
    tester.advance();
    check_pixel(15, 15, 1, yellow, 8);
    check_pixel(80, 80, 1, white, 8);
    check_pixel(95, 95, 1, white, 8);
    check_pixel(115, 115, 1, white, 8);
    
    // Frame 4
    tester.click();
    tester.advance();
    check_pixel(15, 15, 1, red, 8);
    check_pixel(80, 80, 1, white, 8);
    check_pixel(95, 95, 1, white, 8);
    check_pixel(115, 115, 1, white, 8);
    
    check_pixel(315, 15, 1, magenta, 8);

    // Frame 5 (same as previous).
    tester.click();
    tester.advance();
    check_pixel(15, 15, 1, red, 8);
    check_pixel(80, 80, 1, white, 8);
    check_pixel(95, 95, 1, white, 8);
    check_pixel(115, 115, 1, white, 8);
    
    check_pixel(315, 15, 1, magenta, 8);

    // Frame 6
    tester.click();
    tester.advance();
    check_pixel(15, 15, 1, red, 8);
    check_pixel(80, 80, 1, green, 8);
    check_pixel(95, 95, 1, blue, 8);
    check_pixel(115, 115, 1, blue, 8);

    check_pixel(315, 15, 1, magenta, 8);
    
    // Frame 7 (no change)
    tester.click();
    tester.advance();
    check_pixel(15, 15, 1, red, 8);
    check_pixel(80, 80, 1, green, 8);
    check_pixel(95, 95, 1, blue, 8);
    check_pixel(115, 115, 1, blue, 8);

    check_pixel(315, 15, 1, magenta, 8);
    
    // Frame 8
    tester.click();
    tester.advance();
    check_pixel(1, 1, 1, white, 8);
    check_pixel(15, 15, 1, yellow, 8);
    check_pixel(30, 30, 1, red, 8);
    check_pixel(80, 80, 1, green, 8);
    check_pixel(95, 95, 1, blue, 8);
    check_pixel(115, 115, 1, blue, 8);

    check_pixel(315, 15, 1, white, 8);
    
    // Frame 9
    tester.click();
    tester.advance();
    check_pixel(1, 1, 1, red, 8);
    check_pixel(15, 15, 1, red, 8);
    check_pixel(30, 30, 1, red, 8);
    check_pixel(95, 95, 1, white, 8);
    check_pixel(115, 115, 1, white, 8);

    check_pixel(315, 15, 1, white, 8);
    
    // Frame 10 (no change)
    tester.click();
    tester.advance();
    check_pixel(1, 1, 1, red, 8);
    check_pixel(15, 15, 1, red, 8);
    check_pixel(30, 30, 1, red, 8);
    check_pixel(95, 95, 1, white, 8);
    check_pixel(115, 115, 1, white, 8);

    check_pixel(315, 15, 1, white, 8);

    // Frame 11
    tester.click();
    tester.advance();
    check_pixel(1, 1, 1, white, 8);
    check_pixel(30, 30, 1, blue, 8);
    check_pixel(95, 95, 1, blue, 8);
    check_pixel(115, 115, 1, white, 8);

    check_pixel(315, 15, 1, white, 8);
    
    // Frame 12 (no change)
    tester.click();
    tester.advance();
    check_pixel(1, 1, 1, white, 8);
    check_pixel(30, 30, 1, blue, 8);
    check_pixel(95, 95, 1, blue, 8);
    check_pixel(115, 115, 1, white, 8);

    check_pixel(315, 15, 1, white, 8);
    
    // Frame 13 (red square, same as frame 9).
    tester.click();
    tester.advance();
    check_pixel(1, 1, 1, red, 8);
    check_pixel(15, 15, 1, red, 8);
    check_pixel(30, 30, 1, red, 8);
    check_pixel(95, 95, 1, white, 8);
    check_pixel(115, 115, 1, white, 8);

    check_pixel(315, 15, 1, white, 8);
    
    // Frame 13 
    tester.click();
    tester.advance();
    check_pixel(1, 1, 1, white, 8);
    check_pixel(30, 30, 1, blue, 8);
    check_pixel(95, 95, 1, blue, 8);
    check_pixel(115, 115, 1, white, 8);

    check_pixel(325, 35, 1, blue, 8);
    
    // Frame 14 (noise patterns, doesn't make much sense to test externally).
    tester.click();
    tester.advance();

    return 0;
}

