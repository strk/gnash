// 
//   Copyright (C) 2008, 2009, 2010, 2011. 2012 Free Software Foundation, Inc.
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

#ifdef HAVE_POLL
# include <sys/poll.h>
#else 
# ifdef HAVE_EPOLL
#  include <sys/epoll.h>
# endif
#endif

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

#include "arg_parser.h"
#include "log.h"
#include "buffer.h"
#include "handler.h"
#include "amf.h"

using namespace cygnal;
using namespace std;
using namespace gnash;
using namespace boost;

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();
static bool dump = false;
static const char *result;

static void usage (void);
//static void test_pollfds();
//static void test_que();

void test1(Network::thread_params_t *args);
void test2(Network::thread_params_t *args);

int
main (int argc, char* argv[]) {
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'd', "dump",          Arg_parser::no  },
        };
    
    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() ) {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }
    
    for( int i = 0; i < parser.arguments(); ++i ) {
        const int code = parser.code(i);
        try {
            switch( code ) {
              case 'h':
                  usage ();
                  exit(EXIT_SUCCESS);
              case 'v':
                  dbglogfile.setVerbosity();
                  // This happens once per 'v' flag 
                  log_debug(_("Verbose output turned on"));
                  break;
              case 'd':
                  dump= true;
                  break;
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }

    log_unimpl("FIXME: this test case is mostly depreciated due to refactoring");
    // run the tests
//    test_que();
//    test_pollfds();
}

#if 0
FIXME: needs to be moved to network tests
void
test_pollfds()
{
    Handler hand;
    struct pollfd fds1;
    Network::entry_t *func1 = test1;
    fds1.fd = 3;
    fds1.events = POLLIN |  POLLRDHUP;

    hand.addPollFD(fds1, test1);
    if ((hand.getPollFD(0).fd == 3) && (hand.getEntry(3) == func1)) {
        runtest.pass ("Handler::addPollFD(0)");
    } else {
        runtest.fail ("Handler::addPollFD(0)");
    }

    struct pollfd fds2;
    Network::entry_t *func2 = test2;
    fds2.fd = 4;
    fds2.events = POLLIN |  POLLRDHUP;

    hand.addPollFD(fds2, test2);
    if ((hand.getPollFD(1).fd == 4)&& (hand.getEntry(4) == func2)) {
        runtest.pass ("Handler::addPollFD(1)");
    } else {
        runtest.fail ("Handler::addPollFD(1)");
    }

    struct pollfd *fdsptr = hand.getPollFDPtr();
    if ((fdsptr[0].fd == 3) && (fdsptr[1].fd == 4)) {
        runtest.pass ("Handler::getPollFDPtr()");
    } else {
        runtest.fail ("Handler::getPollFDPtr()");
    }
    
    Network::thread_params_t args;
    Network::entry_t *ptr1 = hand.getEntry(3);
    if (ptr1) {
        ptr1(&args);
        if (strcmp(result, "test1") == 0) {
            runtest.pass ("test1()");
        } else {
            runtest.fail ("test1()");
        }
    } else {
        runtest.unresolved ("test1()");
    }

    Network::entry_t *ptr2 = hand.getEntry(4);
    if (ptr2) {
        ptr2(&args);
        if (strcmp(result, "test2") == 0) {
            runtest.pass ("test2()");
        } else {
            runtest.fail ("test2()");
        }
    } else {
        runtest.unresolved ("test2()");
    }
    
}
#endif

#if 0
FIXME: needs to be moved to qcue tests
void
test_que()
{
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
#endif

void
test1(Network::thread_params_t *args)
{
    result = "test1";
}

void
test2(Network::thread_params_t *args)
{
    result = "test2";
}


static void
usage (void)
{
    cerr << "This program tests diskstream support in the cygnal library." << endl
         << endl
         << _("Usage: test_diskstream [options...]") << endl
         << _("  -h,  --help          Print this help and exit") << endl
         << _("  -v,  --verbose       Output verbose debug info") << endl
         << _("  -d,  --dump          Dump data structures") << endl
         << endl;
}

