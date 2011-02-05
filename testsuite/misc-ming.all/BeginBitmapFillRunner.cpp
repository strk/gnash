//  
//    Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
    const rgba lightcyan(0xaa, 0xff, 0xaa, 0xff);
    
    tester.advance();
    check_pixel(1, 1, 1, white, 2);

    // Shape 1
    // Red stripe
    check_pixel(15, 15, 1, red, 2);
    check_pixel(15, 85, 1, red, 2);
    // Green stripe
    check_pixel(30, 15, 1, green, 2);
    check_pixel(30, 85, 1, green, 2);
    // Blue
    check_pixel(45, 15, 1, blue, 2);
    check_pixel(45, 85, 1, blue, 2);
    
    // Shape 2
    // Red stripe
    check_pixel(165, 15, 1, red, 2);
    check_pixel(165, 85, 1, red, 2);
    // Green stripe
    check_pixel(180, 15, 1, green, 2);
    check_pixel(180, 85, 1, green, 2);
    // Blue
    check_pixel(195, 15, 1, blue, 2);
    // Cut due to the shape.
    check_pixel(195, 87, 1, white, 2);
    
    // Shape 3
    // Red stripe
    check_pixel(315, 14, 1, white, 2);
    check_pixel(315, 85, 1, red, 2);
    // Green stripe
    check_pixel(330, 15, 1, white, 2);
    check_pixel(330, 85, 1, green, 2);
    // Blue
    check_pixel(345, 15, 1, white, 2);
    // Cut due to the shape.
    check_pixel(345, 85, 1, blue, 2);

    // Shape 4
    // Skewed, scaled shape.
    check_pixel(100, 200, 1, magenta, 2);
    check_pixel(150, 200, 1, yellow, 2);
    check_pixel(230, 200, 1, cyan, 2);
    check_pixel(235, 250, 1, magenta, 2);
    check_pixel(315, 250, 1, yellow, 2);
    check_pixel(395, 250, 1, white, 2);

    // Shape 5
    check_pixel(30, 315, 1, black, 2);
    check_pixel(70, 315, 1, black, 2);
    check_pixel(140, 315, 1, white, 2);
    check_pixel(170, 315, 1, black, 2);
    check_pixel(30, 330, 1, lightgreen, 2);
    check_pixel(70, 330, 1, lightgreen, 2);
    check_pixel(140, 330, 1, white, 2);
    check_pixel(170, 330, 1, lightgreen, 2);
    check_pixel(30, 345, 1, funnycyan, 2);
    check_pixel(70, 345, 1, funnycyan, 2);
    check_pixel(140, 345, 1, white, 2);
    check_pixel(170, 345, 1, funnycyan, 2);

    // Shape 6
    // Edge pixels extend to edge of shape
    // Top left
    check_pixel(298, 298, 1, white, 2);
    check_pixel(302, 302, 1, black, 2);
    // Centre of fill
    check_pixel(352, 352, 1, black, 2);
    check_pixel(352, 362, 1, lightcyan, 2);
    check_pixel(362, 362, 1, lightgreen, 2);
    // Bottom left
    check_pixel(302, 398, 1, lightcyan, 2);
    check_pixel(298, 398, 1, white, 2);
    // Bottom right
    check_pixel(448, 398, 1, lightgreen, 2);
    check_pixel(448, 402, 1, white, 2);

    // Shape 7: tiled 20x20 blocks centred at (550, 550)
    check_pixel(498, 298, 1, white, 2);
    check_pixel(552, 352, 1, black, 2);
    check_pixel(552, 362, 1, lightcyan, 2);
    check_pixel(562, 362, 1, lightgreen, 2);
    // 20 right
    check_pixel(572, 352, 1, black, 2);
    check_pixel(572, 362, 1, lightcyan, 2);
    check_pixel(582, 362, 1, lightgreen, 2);
    // 20 down
    check_pixel(552, 372, 1, black, 2);
    check_pixel(552, 382, 1, lightcyan, 2);
    check_pixel(562, 382, 1, lightgreen, 2);
    // 20 right and down
    check_pixel(572, 372, 1, black, 2);
    check_pixel(572, 382, 1, lightcyan, 2);
    check_pixel(582, 382, 1, lightgreen, 2);
    // Some random blocks.
    check_pixel(512, 312, 1, black, 2);
    check_pixel(532, 382, 1, lightcyan, 2);
    check_pixel(522, 362, 1, lightgreen, 2);
    
    // Shape 8
    // This should change with the BitmapData.
    check_pixel(20, 465, 1, red, 2);
    
    // Shape 9
    // This should not display because the BitmapData is disposed.
    check_pixel(460, 460, 1, white, 2);
    check_pixel(500, 460, 1, white, 2);
    check_pixel(460, 500, 1, white, 2);

}

