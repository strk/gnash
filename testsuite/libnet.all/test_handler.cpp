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
#include "handler.h"
#include "amf.h"

using namespace amf;
using namespace std;
using namespace gnash;
using namespace boost;
using namespace amf;

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main (int /*argc*/, char** /*argv*/) {
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();

    Handler que;

    boost::shared_ptr<amf::Buffer> buf(new Buffer);
//     boost::uint8_t *test = new uint8_t[6];
//     memcpy(test, "hell", 4);

    // Push one buffer on the fifo. The default is the incoming fifo,
    // which is the one where data flows from the network to the queue.
    que.push(buf);
    if ((que.size() == 1) && (que.outsize() == 0)) {
        runtest.pass ("Handler::push(boost::shared_ptr<amf::Buffer> )");
    } else {
        runtest.fail ("Handler::push(boost::shared_ptr<amf::Buffer> )");
    }
    
    // Push one buffer on the outgoing fifo. The default is the incoming fifo,
    // The outgoing fifo is where data flows from the queu to the network. As
    // we can explicitly specufy which queue we write to, we test that here.
    que.pushout(buf);
    if ((que.size() == 1) && (que.outsize() == 1)) {
        runtest.pass ("Handler::pushout(boost::shared_ptr<amf::Buffer> )");
    } else {
        runtest.fail ("Handler::pushout(boost::shared_ptr<amf::Buffer> )");
    }

    // Test pushin. When dumpimg, the second address should be different than the first,
    // as well as the size. The outgoing queue should be uneffected.
    boost::shared_ptr<amf::Buffer> buf1(new Buffer);
    buf1->resize(112);
    que.pushin(buf1);
    if ((que.size() == 2) && (que.outsize() == 1)) {
        runtest.pass ("Handler::pushin(boost::shared_ptr<amf::Buffer> )");
    } else {
        runtest.fail ("Handler::pushin(boost::shared_ptr<amf::Buffer> )");
    }

    // Nuke the array
    que.clearall();
    if ((que.size() == 0) && (que.outsize() == 0)) {
        runtest.pass ("Handler::clearall()");
    } else {
        runtest.fail ("Handler::clearall()");
    }

    // populate the buffer
    boost::uint8_t *ptr = buf->reference();
    for (size_t i=1; i< buf->size(); i++) {
        ptr[i] = i;
    }

    que.push(buf);
    boost::shared_ptr<amf::Buffer> buf2 = que.peek();
    if ((buf2 == buf) && (que.size() == 1)) {
        runtest.pass ("Handler::peek()");
    } else {
        runtest.fail ("Handler::peek()");
    }

    boost::shared_ptr<amf::Buffer> buf3 = que.peek();
     if ((buf3 == buf) && (que.size() == 1)) {
         runtest.pass ("Handler::pop()");
     } else {
         runtest.fail ("Handler::pop()");
     }
     
//     que.dump();
}

