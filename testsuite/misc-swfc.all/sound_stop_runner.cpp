/* 
 *   Copyright (C) 2011, 2012 Free Software Foundation, Inc.
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
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#define INPUT_FILENAME "sound_stop.swf"

#include "MovieTester.h"
#include "GnashException.h"
#include "log.h"
#include "VM.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
    string filename = string(BUILDDIR) + string("/") + string(INPUT_FILENAME);
    auto_ptr<MovieTester> t;

    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity(2);

    MovieTester tester(filename);

    check_equals(tester.soundsStarted(), 0);
    check_equals(tester.soundsStopped(), 0);

    for (int i=0; i<3; ++i) {
        tester.advance();
        check_equals(tester.soundsStarted(), i+1);
        check_equals(tester.soundsStopped(), 0);
    }

    for (int i=3; i<7; ++i) {
        tester.advance();
        check_equals(tester.soundsStarted(), 3);
        check_equals(tester.soundsStopped(), 0);
    }

    tester.advance();
    check_equals(tester.soundsStarted(), 3);
    check_equals(tester.soundsStopped(), 3);

}

