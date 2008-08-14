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
#include <iostream>
#include <string>

#include "log.h"
#include "dejagnu.h"
#include "as_object.h"
#include "arg_parser.h"
#include "amf.h"
#include "flv.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "gmemory.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

// Prototypes for test cases
static void test_headers();
static void test_tags();

// We use the Memory profiling class to check the malloc buffers
// in the kernel to make sure the allocations and frees happen
// the way we expect them too. There is no real other way to tell.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
Memory *mem = 0;
#endif

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();
RcInitFile& rcfile = RcInitFile::getDefaultInstance();

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
    Buffer *buf = new Buffer((count/3) + 1);
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
    
    // run the tests
    test_headers();
    test_tags();
}

void
test_headers()
{
    Flv flv;
    bool notest = false;

// 02 00 00 a4 00 00 00 00 00 00 00
// 02 00 0a 6f 6e 4d 65 74 61 44 61 74 61		<--- onMetaData
// 08 00 00 00 00
//  00 08 64 75 72 61 74 69 6f 6e			duration
//     00 40 6d 6e 24 dd 2f 1a a0
//  00 0c 76 69 64 65 6f 63 6f 64 65 63 69 64	videocodecid
//     00 40 00 00 00 00 00 00 00
//  00 0c 61 75 64 69 6f 63 6f 64 65 63 69 64	audiocodecid
//     00 40 00 00 00 00 00 00 00
//  00 0c 63 61 6e 53 65 65 6b 54 6f 45 6e 64	canSeekToEnd
//     01 00
//  00 09 63 72 65 61 74 65 64 62 79			createdby
//     02 00 07 46 4d 53 20 33 2e 30			FMS 3.0
//  00 0c 63 72 65 61 74 69 6f 6e 64 61 74 65	creationdate
//     02 00 18 54 75 65 20 4a 75 6e 20 32 34 20 30 38 3a 30 33 3a 34 38 20 32 30 30 38
//  00 00 09					Tue Jun 24 08:03:48 2008
    Buffer *hex1 = hex2mem("46 4c 56 01 0d 00 00 00 09 00 00 00 00");
    Flv::flv_header_t *head = flv.decodeHeader(hex1);
    if (head == 0) {
        notest = true;
    }

    if (notest) {
        runtest.untested("Decoded FLV header");
    } else {
        boost::uint32_t size = *(reinterpret_cast<boost::uint32_t *>(head->head_size));
        if ((memcmp(head->sig, "FLV", 0) == 0)
            && (head->version == 1)
            && (size == 9)) {
            runtest.pass("Decoded FLV header");
        } else {
            runtest.fail("Decoded FLV header");
        }
    }
    delete hex1;

    Buffer *enc1 = flv.encodeHeader(Flv::FLV_AUDIO | Flv::FLV_VIDEO);
    Network::byte_t *ptr = enc1->reference();
    if ((enc1->size() == sizeof(Flv::flv_header_t))
        && (ptr[3] == 0x1)
        && (ptr[4] == 0x5)
        && (ptr[8] == 0x9)) {
        runtest.pass("Encoded FLV header");
    } else {
        runtest.fail("Encoded FLV header");
    }

    // Test converting 3 byte "integers" to a real 4 byte one. The
    // 0xf on each end should be ignore to be correct.
    Buffer *hex2 = hex2mem("0f 00 00 a4 0f");
    boost::uint32_t num = flv.convert24(hex2->reference()+1);
    if (num == 0xa4) {
        runtest.pass("Flv::convert24()");
    } else {
        runtest.fail("Flv::convert24()");
        notest = true;
    }
    delete hex2;

    if (notest) {
        runtest.unresolved("Decoded FLV MetaData header");
    } else {
        Buffer *hex3 = hex2mem("12 00 00 a4 00 00 00 00 00 00 00");
        Flv::flv_tag_t *tag3 = flv.decodeTagHeader(hex3);
        if ((tag3->type == Flv::TAG_METADATA)
            && (flv.convert24(tag3->bodysize) == 164)) {
            runtest.pass("Decoded FLV MetaData header");
        } else {
            runtest.fail("Decoded FLV MetaData header");
        }
        delete tag3;
        delete hex3;
    }
}

void
test_tags()
{
    Flv flv;
    bool notest = false;

    Buffer *hex1 = hex2mem("02 00 0a 6f 6e 4d 65 74 61 44 61 74 61 08 00 00 00 00 00 08 64 75 72 61 74 69 6f 6e 00 40 6d 6e 24 dd 2f 1a a0 00 0c 76 69 64 65 6f 63 6f 64 65 63 69 64 00 40 00 00 00 00 00 00 00 00 0c 61 75 64 69 6f 63 6f 64 65 63 69 64 00 40 00 00 00 00 00 00 00 00 0c 63 61 6e 53 65 65 6b 54 6f 45 6e 64 01 00 00 09 63 72 65 61 74 65 64 62 79 02 00 07 46 4d 53 20 33 2e 30 00 0c 63 72 65 61 74 69 6f 6e 64 61 74 65 02 00 18 54 75 65 20 4a 75 6e 20 32 34 20 30 38 3a 30 33 3a 34 38 20 32 30 30 38 00 00 09");
    Element *el1 = flv.decodeMetaData(hex1);
    if (el1 == 0) {
        notest = true;
    } 
    if (notest) {
        runtest.untested("Decoded FLV MetaData object");
    } else {
//        el1->dump();
        if ((el1->getType() == Element::ECMA_ARRAY_AMF0)
            && (el1->propertySize() == 6)) {
            runtest.pass("Decoded FLV MetaData object");
        } else {
            runtest.fail("Decoded FLV MetaData object");
        }
        delete hex1;
        delete el1;
    }

    // Test decoding Audio tags
    Buffer *hex2 = hex2mem("09 00 00 00 00 00 00 00 00 00 00 00");    
    Flv::flv_tag_t *tag2 = flv.decodeTagHeader(hex2);
    Flv::flv_audio_t *data2 = flv.decodeAudioData(*(hex2->reference() + 11));
    if ((tag2->type && Flv::TAG_AUDIO)
        && (data2->type == Flv::AUDIO_MONO)
        && (data2->size == Flv::AUDIO_8BIT)
        && (data2->rate == Flv::AUDIO_55KHZ)
        && (data2->format == Flv::AUDIO_UNCOMPRESSED)) {
        runtest.pass("Decoded FLV Audio Data flags");
    } else {
        runtest.fail("Decoded FLV Audio Data flags");
    }
    delete hex2;
    delete tag2;
    delete data2;

    Buffer *hex3 = hex2mem("08 00 00 1b 00 00 00 00 00 00 00 2a");
    Flv::flv_tag_t *tag3 = flv.decodeTagHeader(hex3);
    Flv::flv_video_t *data3 = flv.decodeVideoData(*(hex3->reference() + 11));
    if ((tag3->type && Flv::TAG_VIDEO)
        && (data3->codecID == Flv::VIDEO_H263)
        && (data3->type == Flv::KEYFRAME)) {
        runtest.pass("Decoded FLV Video Data flags");
    } else {
        runtest.fail("Decoded FLV Video Data flags");
    }
    delete hex3;
    delete tag3;
    delete data3;
}

static void
usage (void)
{
    cerr << "This program tests FLV support in the AMF library." << endl
         << endl
         << _("Usage: test_amf [options...]") << endl
         << _("  -h,  --help          Print this help and exit") << endl
         << _("  -v,  --verbose       Output verbose debug info") << endl
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
