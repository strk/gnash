// 
//   Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_DEJAGNU_H

#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>

#include "GnashException.h"
#include "check.h"
//#include "dejagnu.h"
#include "arg_parser.h"
#include "amf.h"
#include "amf_msg.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "gmemory.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

// Prototypes for test cases
static void test_encoding();

// Enable the display of memory allocation and timing data
static bool memdebug = false;

// We use the Memory profiling class to check the malloc buffers
// in the kernel to make sure the allocations and frees happen
// the way we expect them too. There is no real other way to tell.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
Memory *mem = 0;
#endif

TestState& runtest=_runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();
RcInitFile& rcfile = RcInitFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'w', "write",         Arg_parser::no  },
// Unless you have support for memory debugging turned on, and
// you have support for the Linux mallinfo() system call,
// this option is totally useless. This doesn't really matter
// as the memory testing is primarily used only during
// debugging or development.
            { 'm', "memstats",      Arg_parser::no  },
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
              case 'm':
                    // This happens once per 'v' flag 
                    log_debug(_("Enabling memory statistics"));
                    memdebug = true;
                    break;
              case 'w':
                  rcfile.useWriteLog(true); // dbglogfile.setWriteDisk(true);
                  log_debug(_("Logging to disk enabled"));
                  break;
                  
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }

#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem = new Memory;
        mem->startStats();
    }
#endif
    
    // run the tests
    test_encoding();
}

void
test_encoding()
{
//  00 06 67 65 74 77 61 79                <- getway, message #1
//  00 04 2f 32 32 39                      <- /229, operation name
//  00 00 00 0e				   <- byte length of message
//     0a 00 00 00 01			   <- array, 1 item
//        00 41 70 43 87 20 00 00 00
//
//  00 06 67 65 74 77 61 79                <- getway, message #2
//  00 04 2f 32 33 30                      <- /230, operation name
//  00 00 00 0e				   <- byte length of message
//     0a 00 00 00 01			   <- array, 1 item
//        00 41 70 43 ba 00 00 00 00
//
//  00 06 67 65 74 77 61 79                <- getway, message #3
//  00 04 2f 32 33 31                      <- /231, operation name
//  00 00 00 0e				   <- byte length of message
//     0a 00 00 00 01			   <- array, 1 item
//        00 41 70 43 ac e0 00 00 00
    boost::shared_ptr<Buffer> buf1(new Buffer("00 00 00 00 00 03 00 06 67 65 74 77 61 79 00 04 2f 32 32 39 00 00 00 0e 0a 00 00 00 01 00 41 70 43 87 20 00 00 00 00 06 67 65 74 77 61 79 00 04 2f 32 33 30 00 00 00 0e 0a 00 00 00 01 00 41 70 43 ba 00 00 00 00 00 06 67 65 74 77 61 79 00 04 2f 32 33 31 00 00 00 0e 0a 00 00 00 01 00 41 70 43 ac e0 00 00 00"));
    double num = *(reinterpret_cast<double *>(buf1->reference()));
    swapBytes(&num, amf::AMF0_NUMBER_SIZE); // we always encode in big endian format

    AMF_msg amsg;
    boost::shared_ptr<AMF_msg::context_header_t> head1 =
        amsg.parseAMFPacket(buf1->reference(), buf1->size());

//    amsg.dump(*head1);
//    amsg.dump();
    double dub1 = amsg.getMessage(0)->data->getProperty(0)->to_number();
    double dub2 = amsg.getMessage(1)->data->getProperty(0)->to_number();
    double dub3 = amsg.getMessage(2)->data->getProperty(0)->to_number();
    
    if ((amsg.messageCount() == 3)
        && (amsg.getMessage(0)->data->getType() == Element::STRICT_ARRAY_AMF0)
        && (dub1 == 0x1043872)
        && (amsg.getMessage(1)->data->getType() == Element::STRICT_ARRAY_AMF0)
        && (amsg.getMessage(2)->data->getType() == Element::STRICT_ARRAY_AMF0)
        ) {
        runtest.pass("AMF_msg::parseAMFPacket()");
    } else {
        runtest.fail("AMF_msg::parseAMFPacket()");
    }

    // Build a new message packet from scratch and make sure it matches the real one
    AMF_msg top;
    
    boost::shared_ptr<amf::Element> getway1(new Element);
    getway1->makeStrictArray();
    boost::shared_ptr<amf::Element> data1(new Element);
    data1->makeNumber(dub1);
    getway1->addProperty(data1);
    boost::shared_ptr<AMF_msg::amf_message_t> msg1(new AMF_msg::amf_message_t);
    msg1->header.target = "getway";
    msg1->header.response = "/229";
    msg1->header.size = 14;
    msg1->data = getway1;
    top.addMessage(msg1);
    
    boost::shared_ptr<amf::Element> getway2(new Element);
    getway2->makeStrictArray();
    boost::shared_ptr<amf::Element> data2(new Element);
    data2->makeNumber(dub2);
    getway2->addProperty(data2);
    boost::shared_ptr<AMF_msg::amf_message_t> msg2(new AMF_msg::amf_message_t);
    msg2->header.target = "getway";
    msg2->header.response = "/230";
    msg2->header.size = 14;
    msg2->data = getway2;
    top.addMessage(msg2);
    
    boost::shared_ptr<amf::Element> getway3(new Element);
    getway3->makeStrictArray();
    boost::shared_ptr<amf::Element> data3(new Element);
    data3->makeNumber(dub3);
    getway3->addProperty(data3);
    boost::shared_ptr<AMF_msg::amf_message_t> msg3(new AMF_msg::amf_message_t);
    msg3->header.target = "getway";
    msg3->header.response = "/231";
    msg3->header.size = 14;
    msg3->data = getway3;
    top.addMessage(msg3);

    boost::shared_ptr<amf::Buffer> buf2 = top.encodeMsgHeader("getway", "/229", 14);
    boost::uint8_t *ptr1 = buf1->reference() + sizeof(AMF_msg::context_header_t);


    if (memcmp(ptr1, buf2->reference(), buf2->size()) == 0) {
        runtest.pass("AMF_msg::encodeMsgHeader()");
    } else {
        runtest.fail("AMF_msg::encodeMsgHeader()");
    }
    
    boost::shared_ptr<amf::Buffer> buf3 = top.encodeAMFPacket();
    if (memcmp(buf1->reference(), buf3->reference(), buf3->allocated()) == 0) {
        runtest.pass("AMF_msg::encodeAMFPacket()");
    } else {
        runtest.fail("AMF_msg::encodeAMFPacket()");
    }
//    buf3->dump();
//    top.dump();
//
}


static void
usage (void)
{
    cerr << "This program tests AMF support in the AMF library." << endl
         << endl
         << _("Usage: test_amf [options...]") << endl
         << _("  -h,  --help          Print this help and exit") << endl
         << _("  -v,  --verbose       Output verbose debug info") << endl
         << _("  -m,  --memdebug      Output memory statistics") << endl
         << endl;
}

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
  return 0;  
}

#endif
