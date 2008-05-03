// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

//#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>

#include "as_object.h"
#include "dejagnu.h"
#include "log.h"
#include "amf.h"
#include "rtmp_server.h"
#include "rtmp.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "sol.h"
#include "arg_parser.h"
#include "gmemory.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

// Enable the display of memory allocation and timing data
static bool memdebug = false;

// We use the Memory profiling class to check the malloc buffers
// in the kernel to make sure the allocations and frees happen
// the way we expect them too. There is no real other way to tell.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
Memory *mem = 0;
#endif

static TestState runtest;

static void test_header();
static void test_types();

LogFile& dbglogfile = LogFile::getDefaultInstance();

// These next two functions are borrowed from Libgloss, part of the GNU binutils,
// of which I am the primary author and copyright holder.
// convert an ascii hex digit to a number.
//      param is hex digit.
//      returns a decimal digit.
Network::byte_t
hex2digit (Network::byte_t digit)
{  
    if (digit == 0)
        return 0;
    
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    
    // shouldn't ever get this far
    return -1;
}

// Convert the hex array pointed to by buf into binary to be placed in mem
Buffer *
hex2mem(const char *str)
{
    size_t count = strlen(str);
    Network::byte_t ch = 0;
    Buffer *buf = new Buffer(count + 12);
    buf->clear();

    Network::byte_t *ptr = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str));
    
    for (size_t i=0; i<count; i++) {
        if (*ptr == ' ') {      // skip spaces.
            ptr++;
            continue;
        }
        ch = hex2digit(*ptr++) << 4;
        ch |= hex2digit(*ptr++);
        buf->append(ch);
    }
    return buf;
}

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
                    log_debug(_("Verbose output turned on"));
                    break;
              case 'm':
                    log_debug(_("Enabling memory statistics"));
                    memdebug = true;
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
    
    test_header();
    test_types();
}

void
test_header()
{
    GNASH_REPORT_FUNCTION;
    RTMP rtmp;
    
    // this is a sample 12 bytes RTMP header
    const char *x = "03 00 00 00 00 01 1f 14 00 00 00 00";
    Buffer *buf1 = hex2mem(x);
    Buffer *head = rtmp.encodeHeader(0x3, RTMP::HEADER_12, 287,
                                     RTMP::INVOKE, RTMP::FROM_CLIENT);
//     cerr << hexify(head->begin(), 12, false) << endl;
    
     if ((memcmp(buf1->reference(), head->reference(), 12) == 0)) {
         runtest.pass("Encoded RTMP header");
     } else {
         runtest.fail("Encoded RTMP header");
     }

     RTMP::rtmp_head_t *header = rtmp.decodeHeader(buf1->reference());
     if ((header->channel == 0x3) && (header->head_size == 12)
         && (header->bodysize == 287) && (header->type ==  RTMP::INVOKE)) {
         runtest.pass("Decoded RTMP header");
     } else {
         runtest.fail("Decoded RTMP header");
     }
     
     // cleanup after ourselves
     delete buf1;
     delete head;
}


void
test_types()
{
    GNASH_REPORT_FUNCTION;
    RTMP rtmp;

    const char *x = "06 00 d2 04 00 00 00 00";
    Buffer *buf1 = hex2mem(x);
    
    RTMP::rtmp_ping_t *ping = rtmp.decodePing(buf1);
    if (ping->type == RTMP::PING_CLIENT) {
         runtest.pass("Decoded RTMP Ping message");
     } else {
         runtest.fail("Decoded RTMP Ping message");
     }
    
    Buffer *buf2 = rtmp.encodePing(RTMP::PING_CLIENT, 53764);
    cerr << hexify(buf2->begin(), 8, false) << endl;
     if ((memcmp(buf1->reference(), buf2->reference(), 8) == 0)) {
         runtest.pass("Encoded RTMP Ping message");
     } else {
         runtest.fail("Encoded RTMP Ping message");
     }
     delete ping;
     delete buf1;
     delete buf2;
}

static void
usage (void)
{
    cerr << "This program tests SOL support in the AMF library." << endl;
    cerr << "Usage: test_sol [hv]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    exit (-1);
}

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
  return 0;  
}

#endif

// 03 00 00 04 00 01 1f 14 00 00 00 00 02 00 07 63   ...............c
// 6f 6e 6e 65 63 74 00 3f f0 00 00 00 00 00 00 03   onnect.?........
// 00 03 61 70 70 02 00 08 6f 66 6c 61 44 65 6d 6f   ..app...oflaDemo
// 00 08 66 6c 61 73 68 56 65 72 02 00 0c 4c 4e 58   ..flashVer...LNX
// 20 39 2c 30 2c 33 31 2c 30 00 06 73 77 66 55 72    9,0,31,0..swfUr
// 6c 02 00 33 68 74 74 70 3a 2f 2f 6c 6f 63 61 6c   l..3http://local
// 68 6f 73 74 2f 73 6f 66 74 77 61 72 65 2f 67 6e   host/software/gn
// 61 73 68 2f 74 65 73 74 73 2f 6f 66 6c 61 5f 64   ash/tests/ofla_d
// 65 6d 6f 2e 73 77 66 00 05 74 63 55 c3 72 6c 02   emo.swf..tcU.rl.
// 00 19 72 74 6d 70 3a 2f 2f 6c 6f 63 61 6c 68 6f   ..rtmp://localho
// 73 74 2f 6f 66 6c 61 44 65 6d 6f 00 04 66 70 61   st/oflaDemo..fpa
// 64 01 00 00 0b 61 75 64 69 6f 43 6f 64 65 63 73   d....audioCodecs
// 00 40 83 38 00 00 00 00 00 00 0b 76 69 64 65 6f   .@.8.......video
// 43 6f 64 65 63 73 00 40 5f 00 00 00 00 00 00 00   Codecs.@_.......
// 0d 76 69 64 65 6f 46 75 6e 63 74 69 6f 6e 00 3f   .videoFunction.?
// f0 00 00 00 00 00 00 00 07 70 61 67 65 55 72 6c   .........pageUrl
// 02 00 26 68 74 74 70 3a 2f 2f 6c 6f 63 c3 61 6c   ..&http://loc.al
// 68 6f 73 74 2f 73 6f 66 74 77 61 72 65 2f 67 6e   host/software/gn
// 61 73 68 2f 74 65 73 74 73 2f 00 00 09

    
