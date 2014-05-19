// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifdef HAVE_STDARG_H
#include <cstdarg>
#endif

#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include "GnashSystemIOHeaders.h"
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
#include "buffer.h"
#include "network.h"
#include "cque.h"
#include "amf.h"

using namespace cygnal;
using namespace std;
using namespace gnash;
using namespace boost;

TestState runtest;
//LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main (int /*argc*/, char** /*argv*/) {
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();

    CQue que;

    std::shared_ptr<cygnal::Buffer> buf(new Buffer(50));
    // populate the buffer
    boost::uint8_t *ptr = buf->reference();
    for (Network::byte_t i=1; i< buf->size(); i++) {
        *buf += i;
        *buf += ' ';
    }

//     boost::uint8_t *test = new uint8_t[6];
//     memcpy(test, "hell", 4);

    // Push one buffer on the fifo. The default is the incoming fifo,
    // which is the one where data flows from the network to the queue.
    que.push(buf);
    if (que.size() == 1) {
        runtest.pass ("CQue::push(std::shared_ptr<cygnal::Buffer> )");
    } else {
        runtest.fail ("CQue::push(std::shared_ptr<cygnal::Buffer> )");
    }
    
    // Test push. When dumpimg, the second address should be different than the first,
    // as well as the size. The outgoing queue should be uneffected.
    std::shared_ptr<cygnal::Buffer> buf1(new Buffer(112));
    que.push(buf1);
    if (que.size() == 2) {
        runtest.pass ("CQue::pushin(std::shared_ptr<cygnal::Buffer> )");
    } else {
        runtest.fail ("CQue::pushin(std::shared_ptr<cygnal::Buffer> )");
    }

    // Nuke the array
    que.clear();
    if (que.size() == 0) {
        runtest.pass ("CQue::clearall()");
    } else {
        runtest.fail ("CQue::clearall()");
    }

    
    que.push(buf);
    std::shared_ptr<cygnal::Buffer> buf2 = que.peek();
    if ((buf2 == buf) && (que.size() == 1)) {
        runtest.pass ("CQue::peek()");
    } else {
        runtest.fail ("CQue::peek()");
    }

    std::shared_ptr<cygnal::Buffer> buf3 = que.peek();
     if ((buf3 == buf) && (que.size() == 1)) {
         runtest.pass ("CQue::pop()");
     } else {
         runtest.fail ("CQue::pop()");
     }

     que.push(buf1);
     que.push(buf1);
     size_t firstsize = que.size();
     que.remove(buf);
     if (que.size() == firstsize - 1) {
         runtest.pass ("CQue::remove()");
     } else {
         runtest.fail ("CQue::remove()");
     }

     // Make some test buffers
     std::shared_ptr<cygnal::Buffer> merge1(new Buffer);
     std::shared_ptr<cygnal::Buffer> merge2(new Buffer);
     std::shared_ptr<cygnal::Buffer> merge3(new Buffer);
     size_t i;
     ptr = merge1->reference();
     for (i=0; i<cygnal::NETBUFSIZE; i++) {
         ptr[i] = i*'A';
     }
     que.push(merge1);
     
     ptr = merge2->reference();
     for (i=0; i<cygnal::NETBUFSIZE; i++) {
         ptr[i] = i+'a';
     }
     que.push(merge2);

     merge3->resize(96);
     ptr = merge3->reference();
     for (i=0; i<96; i++) {
         ptr[i] = i+' ';
     }
     que.push(merge3);

//     que.dump();
     // A merge gives us one big buffer where there were several buffers
     std::shared_ptr<cygnal::Buffer> foo = que.merge(merge1);
     if (foo == 0) {
         runtest.unresolved("CQue::merge()");
     } else {
         if (foo->size() == (cygnal::NETBUFSIZE * 2) + 120) {
             runtest.pass("CQue::merge()");
         } else {
             runtest.fail("CQue::merge()");
         }
     }

//     que.pop();

}

