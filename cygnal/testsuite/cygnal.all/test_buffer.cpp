// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <regex.h>
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <boost/cstdint.hpp>

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

#include "log.h"
#include "gmemory.h"
#include "buffer.h"

using namespace std;
using namespace gnash;
using namespace cygnal;
using namespace boost;

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();

// This must match the value in buffer.h
const size_t BUFFERSIZE = 128;

int
main (int /*argc*/, char** /*argv*/) {
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();
    // We use the Memory profiling class to check the malloc buffers
    // in the kernel to make sure the allocations and frees happen
    // the way we expect them too. There is no real other way to tell.
    Memory mem;
    mem.startStats();

    Buffer buf;
    mem.addStats(__LINE__);             // take a sample
    
    if (buf.size() == BUFFERSIZE) {
         runtest.pass ("Buffer::size()");
     } else {
         runtest.fail ("Buffer::size()");
    }

    mem.addStats(__LINE__);             // take a sample
    buf.resize(112);
    mem.addStats(__LINE__);             // take a sample

    if ((buf.size() == 112)  && (mem.diffStats() == -16)) {
         runtest.pass ("Buffer::resize()");
     } else {
         runtest.fail ("Buffer::resize()");
    }
    mem.addStats(__LINE__);             // take a sample

//    buf.dump();    
    mem.addStats(__LINE__);             // take a sample
    buf.empty();                        //empty just nukes the contents
    if ((buf.size() == 112) && (mem.diffStats() == 0)) {
         runtest.pass ("Buffer::empty()");
     } else {
         runtest.fail ("Buffer::empty()");
    }

    // populate the buffer
    boost::uint8_t *ptr = buf.reference();
    for (size_t i=1; i< buf.size(); i++) {
        ptr[i] = i;
    }

    Buffer buf2;
    if (buf2 == buf) {
         runtest.fail ("Buffer::operator==");
     } else {
         runtest.pass ("Buffer::operator==");
    }

    buf2 = buf;
    if (buf2 == buf) {
         runtest.pass ("Buffer::operator=");
     } else {
         runtest.fail ("Buffer::operator=");
    }

//    mem.analyze();
}

