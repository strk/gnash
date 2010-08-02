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

    const rgba black(0, 0, 0, 255);
    const rgba white(255, 255, 255, 255);
    const rgba blue(0, 0, 255, 255);
    const rgba cyan(0, 255, 255, 255);
    const rgba green(0, 255, 0, 255);
    const rgba yellow(255, 255, 0, 255);
    const rgba magenta(255, 0, 255, 255);
    const rgba red(255, 0, 0, 255);
    const rgba lightgreen(0xaa, 0xff, 0x00, 0xff);
    const rgba funnycyan(0x00, 0xcc, 0xff, 0xff);
    
    tester.advance();
    check_pixel(1, 1, 1, white, 8);

    // Shape 1
    // Red stripe
    check_pixel(15, 15, 1, red, 8);
    check_pixel(15, 85, 1, red, 8);
    // Green stripe
    check_pixel(30, 15, 1, green, 8);
    check_pixel(30, 85, 1, green, 8);
    // Blue
    check_pixel(45, 15, 1, blue, 8);
    check_pixel(45, 85, 1, blue, 8);
    
    // Shape 2
    // Red stripe
    check_pixel(165, 15, 1, red, 8);
    check_pixel(165, 85, 1, red, 8);
    // Green stripe
    check_pixel(180, 15, 1, green, 8);
    check_pixel(180, 85, 1, green, 8);
    // Blue
    check_pixel(195, 15, 1, blue, 8);
    // Cut due to the shape.
    check_pixel(195, 87, 1, white, 8);
    
    // Shape 3
    // Red stripe
    check_pixel(315, 14, 1, white, 8);
    check_pixel(315, 85, 1, red, 8);
    // Green stripe
    check_pixel(330, 15, 1, white, 8);
    check_pixel(330, 85, 1, green, 8);
    // Blue
    check_pixel(345, 15, 1, white, 8);
    // Cut due to the shape.
    check_pixel(345, 85, 1, blue, 8);

    // Shape 5
    check_pixel(30, 315, 1, black, 8);
    check_pixel(70, 315, 1, black, 8);
    check_pixel(140, 315, 1, white, 8);
    check_pixel(170, 315, 1, black, 8);
    check_pixel(30, 330, 1, lightgreen, 8);
    check_pixel(70, 330, 1, lightgreen, 8);
    check_pixel(140, 330, 1, white, 8);
    check_pixel(170, 330, 1, lightgreen, 8);
    check_pixel(30, 345, 1, funnycyan, 8);
    check_pixel(70, 345, 1, funnycyan, 8);
    check_pixel(140, 345, 1, white, 8);
    check_pixel(170, 345, 1, funnycyan, 8);

    // Shape 6
    
    // Shape 7
    
    // Shape 8
    // This should change with the BitmapData.
    xcheck_pixel(20, 465, 1, red, 8);

}

