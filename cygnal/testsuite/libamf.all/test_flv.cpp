/// 
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

using namespace cygnal;
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
    std::shared_ptr<cygnal::Buffer> hex1(new Buffer("46 4c 56 01 0d 00 00 00 09 00 00 00 00"));
    std::shared_ptr<Flv::flv_header_t> head = flv.decodeHeader(hex1);
    if (!head) {
        notest = true;
    }

    if (notest) {
        runtest.untested("Decoded FLV header");
    } else {
        boost::uint32_t size = *(reinterpret_cast<boost::uint32_t *>(head->head_size));
        if ((memcmp(head->sig, "FLV", 3) == 0)
            && (head->version == 1)
            && (size == 9)) {
            runtest.pass("Decoded FLV header");
        } else {
            runtest.fail("Decoded FLV header");
        }
    }
    
    std::shared_ptr<cygnal::Buffer> enc1 = flv.encodeHeader(Flv::FLV_AUDIO | Flv::FLV_VIDEO);
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
    std::shared_ptr<cygnal::Buffer> hex2(new Buffer("0f 00 00 a4 0f"));
    boost::uint32_t num = flv.convert24(hex2->reference()+1);
    if (num == 0xa4) {
        runtest.pass("Flv::convert24()");
    } else {
        runtest.fail("Flv::convert24()");
        notest = true;
    }
    
    std::shared_ptr<cygnal::Buffer> hex3(new Buffer("12 00 00 a4 00 00 00 00 00 00 00"));

    std::shared_ptr<Flv::flv_tag_t> tag3 = flv.decodeTagHeader(hex3);
    if ((tag3->type == Flv::TAG_METADATA)
        && (flv.convert24(tag3->bodysize) == 0xa40000)) {
        runtest.pass("Decoded FLV MetaData Tag header");
    } else {
        runtest.fail("Decoded FLV MetaData Tag header");
    }
}

void
test_tags()
{
    Flv flv;
    bool notest = false;

    std::shared_ptr<cygnal::Buffer> hex1(new Buffer("02 00 0a 6f 6e 4d 65 74 61 44 61 74 61 08 00 00 00 00 00 08 64 75 72 61 74 69 6f 6e 00 40 6d 6e 24 dd 2f 1a a0 00 0c 76 69 64 65 6f 63 6f 64 65 63 69 64 00 40 00 00 00 00 00 00 00 00 0c 61 75 64 69 6f 63 6f 64 65 63 69 64 00 40 00 00 00 00 00 00 00 00 0c 63 61 6e 53 65 65 6b 54 6f 45 6e 64 01 00 00 09 63 72 65 61 74 65 64 62 79 02 00 07 46 4d 53 20 33 2e 30 00 0c 63 72 65 61 74 69 6f 6e 64 61 74 65 02 00 18 54 75 65 20 4a 75 6e 20 32 34 20 30 38 3a 30 33 3a 34 38 20 32 30 30 38 00 00 09"));
    std::shared_ptr<cygnal::Element> el1 = flv.decodeMetaData(hex1);
    el1->dump();
    if (!el1) {
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
    }

    // Test decoding Audio tags
    std::shared_ptr<cygnal::Buffer> hex2(new Buffer("09 00 00 00 00 00 00 00 00 00 00 00"));
    std::shared_ptr<Flv::flv_tag_t> tag2 = flv.decodeTagHeader(hex2);
    std::shared_ptr<Flv::flv_audio_t> data2 = flv.decodeAudioData(*(hex2->reference() + 11));
    if ((tag2->type && Flv::TAG_AUDIO)
        && (data2->type == Flv::AUDIO_MONO)
        && (data2->size == Flv::AUDIO_8BIT)
        && (data2->rate == Flv::AUDIO_55KHZ)
        && (data2->format == Flv::AUDIO_UNCOMPRESSED)) {
        runtest.pass("Decoded FLV Audio Data flags");
    } else {
        runtest.fail("Decoded FLV Audio Data flags");
    }

    std::shared_ptr<cygnal::Buffer> hex3(new Buffer("08 00 00 1b 00 00 00 00 00 00 00 2a"));
    std::shared_ptr<Flv::flv_tag_t> tag3 = flv.decodeTagHeader(hex3);
    std::shared_ptr<Flv::flv_video_t> data3 = flv.decodeVideoData(*(hex3->reference() + 11));
    if ((tag3->type && Flv::TAG_VIDEO)
        && (data3->codecID == Flv::VIDEO_H263)
        && (data3->type == Flv::KEYFRAME)) {
        runtest.pass("Decoded FLV Video Data flags");
    } else {
        runtest.fail("Decoded FLV Video Data flags");
    }

    std::shared_ptr<cygnal::Buffer> hex4(new Buffer("00 0a 6f 6e 4d 65 74 61 44 61 74 61 08 00 00 00 00 00 08 64 75 72 61 74 69 6f 6e 00 40 6d 6e 24 dd 2f 1a a0 00 0c 76 69 64 65 6f 63 6f 64 65 63 69 64 00 40 00 00 00 00 00 00 00 00 0c 61 75 64 69 6f 63 6f 64 65 63 69 64 00 40 00 00 00 00 00 00 00 00 0c 63 61 6e 53 65 65 6b 54 6f 45 6e 64 01 00 00 09 63 72 65 61 74 65 64 62 79 02 00 07 46 4d 53 20 33 2e 30 00 0c 63 72 65 61 74 69 6f 6e 64 61 74 65 02 00 18 54 75 65 20 4a 75 6e 20 32 34 20 30 38 3a 30 33 3a 34 38 20 32 30 30 38 00 00 09"));
    std::shared_ptr<cygnal::Element> el4 = flv.decodeMetaData(hex4);
    if (!el4) {
        notest = true;
    } 
    if (notest) {
        runtest.untested("Decoded FLV MetaData object without type");
    } else {
//        el1->dump();
        if ((el4->getType() == Element::ECMA_ARRAY_AMF0)
            && (el4->propertySize() == 6)) {
            runtest.pass("Decoded FLV MetaData object without type");
        } else {
            runtest.fail("Decoded FLV MetaData object without type");
        }
    }

#if 0
    // These are the packets as FLVParser handes them off
    std::shared_ptr<cygnal::Buffer> hex5(new Buffer("00 0a 6f 6e 4d 65 74 61 44 61 74 61 08 00 00 00 0a 00 08 64 75 72 61 74 69 6f 6e 00 40 82 5c fd f3 b6 45 a2 00 0d 76 69 64 65 6f 64 61 74 61 72 61 74 65 00 40 74 8a 51 f7 27 8d c5 00 15 6c 61 73 74 6b 65 79 66 72 61 6d 65 74 69 6d 65 73 74 61 6d 70 00 40 82 53 5a 1c ac 08 31 00 14 6c 61 73 74 6b 65 79 66 72 61 6d 65 6c 6f 63 61 74 69 6f 6e 00 41 77 01 7f 10 00 00 00 00 07 63 72 65 61 74 6f 72 02 00 0d 59 6f 75 54 75 62 65 2c 20 49 6e 63 2e 00 0f 6d 65 74 61 64 61 74 61 63 72 65 61 74 6f 72 02 00 1a 59 6f 75 54 75 62 65 20 4d 65 74 61 64 61 74 61 20 49 6e 6a 65 63 74 6f 72 2e 00 09 66 6c 76 73 6f 75 72 63 65 02 00 04 63 64 62 70 00 0c 68 61 73 6b 65 79 66 72 61 6d 65 73 01 01 00 0b 68 61 73 6d 65 74 61 64 61 74 61 01 01 00 09 6b 65 79 66 72 61 6d 65 73 03 00 05 74 69 6d 65 73 0a 00 00 01 33 00 00 00 00 00 00 00 00 00 00 3f ff 5c 28 f5 c2 8f 5c 00 40 0f 5e 35 3f 7c ed 91 00 40 17 86 24 dd 2f 1a a0 00 40 1f 5d 2f 1a 9f be 77 00 40 23 9a 1c ac 08 31 27 00 40 26 1b 22 d0 e5 60 42 00 40 26 30 20 c4 9b a5 e3 00 40 2a 1c 28 f5 c2 8f 5c 00 40 2e 07 ae 14 7a e1 48 00 40 30 f9 99 99 99 99 9a 00 40 32 ef 9d b2 2d 0e 56 00 40 34 e5 60 41 89 37 4c 00 40 36 db 22 d0 e5 60 42 00 40 38 d0 e5 60 41 89 37 00 40 3a c6 e9 78 d4 fd f4 00 40 3c bc ac 08 31 26 e9 00 40 3e b2 6e 97 8d 4f df 00 40 40 54 39 58 10 62 4e 00 40 41 4f 1a 9f be 76 c9 00 40 42 49 fb e7 6c 8b 44 00 40 43 44 dd 2f 1a 9f be 00 40 44 3f df 3b 64 5a 1d 00 40 45 3a c0 83 12 6e 98 00 40 46 35 a1 ca c0 83 12 00 40 47 30 a3 d7 0a 3d 71 00 40 48 2b 85 1e b8 51 ec 00 40 49 26 66 66 66 66 66 00 40 4a 21 68 72 b0 20 c5 00 40 4b 1c 49 ba 5e 35 3f 00 40 4c 17 2b 02 0c 49 ba 00 40 4d 12 0c 49 ba 5e 35 00 40 4e 0d 0e 56 04 18 93 00 40 4f 07 ef 9d b2 2d 0e 00 40 50 01 68 72 b0 20 c5 00 40 50 7e e9 78 d4 fd f4 00 40 50 fc 5a 1c ac 08 31 00 40 51 79 ca c0 83 12 6f 00 40 51 f7 3b 64 5a 1c ac 00 40 52 74 bc 6a 7e f9 db 00 40 52 f2 2d 0e 56 04 19 00 40 53 6f 9d b2 2d 0e 56 00 40 53 ed 1e b8 51 eb 85 00 40 54 6a 8f 5c 28 f5 c3 00 40 54 ea b0 20 c4 9b a6 00 40 55 68 20 c4 9b a5 e3 00 40 55 e5 a1 ca c0 83 12 00 40 56 63 12 6e 97 8d 50 00 40 56 e0 83 12 6e 97 8d 00 40 57 5d f3 b6 45 a1 cb 00 40 57 db 74 bc 6a 7e fa 00 40 58 58 e5 60 41 89 37 00 40 58 d6 56 04 18 93 75 00 40 59 53 d7 0a 3d 70 a4 00 40 59 d1 47 ae 14 7a e1 00 40 5a 4e b8 51 eb 85 1f 00 40 5a cc 28 f5 c2 8f 5c 00 40 5b 49 a9 fb e7 6c 8b 00 40 5b c7 1a 9f be 76 c9 00 40 5c 44 8b 43 95 81 06 00 40 5c c2 0c 49 ba 5e 35 00 40 5d 3f 7c ed 91 68 73 00 40 5d bc ed 91 68 72 b0 00 40 5e 3a 5e 35 3f 7c ee 00 40 5e b7 df 3b 64 5a 1d 00 40 5f 35 4f df 3b 64 5a 00 40 5f b2 c0 83 12 6e 98 00 40 60 18 20 c4 9b a5 e3 00 40 60 56 d9 16 87 2b 02 00 40 60 95 91 68 72 b0 21 00 40 60 d4 51 eb 85 1e b8 00 40 61 13 0a 3d 70 a3 d7 00 40 61 51 c2 8f 5c 28 f6 00 40 61 90 7a e1 47 ae 14 00 40 61 cf 3b 64 5a 1c ac 00 40 62 0d f3 b6 45 a1 cb 00 40 62 4c ac 08 31 26 e9 00 40 62 8b 6c 8b 43 95 81 00 40 62 ca 24 dd 2f 1a a0 00 40 63 08 dd 2f 1a 9f be 00 40 63 47 95 81 06 24 dd 00 40 63 86 56 04 18 93 75 00 40 63 c5 0e 56 04 18 93 00 40 64 03 c6 a7 ef 9d b2 00 40 64 42 87 2b 02 0c 4a 00 40 64 81 3f 7c ed 91 68 00 40 64 bf f7 ce d9 16 87 00 40 64 fe b0 20 c4 9b a6 00 40 65 3d 70 a3 d7 0a 3d 00 40 65 7c 28 f5 c2 8f 5c 00 40 65 ba e1 47 ae 14 7b 00 40 65 f9 a1 ca c0 83 12 00 40 66 38 5a 1c ac 08 31 00 40 66 77 12 6e 97 8d 50 00 40 66 b5 ca c0 83 12 6f 00 40 66 f4 8b 43 95 81 06 00 40 67 33 43 95 81 06 25 00 40 67 71 fb e7 6c 8b 44 00 40 67 b0 bc 6a 7e f9 db 00 40 67 ef 74 bc 6a 7e fa 00 40 68 2e 2d 0e 56 04 19 00 40 68 6c ed 91 68 72 b0 00 40 68 ab a5 e3 53 f7 cf 00 40 68 ea 5e 35 3f 7c ee 00 40 69 29 16 87 2b 02 0c 00 40 69 67 d7 0a 3d 70 a4 00 40 69 a6 8f 5c 28 f5 c3 00 40 69 e5 47 ae 14 7a e1 00 40 6a 24 08 31 26 e9 79 00 40 6a 62 c0 83 12 6e 98 00 40 6a a1 78 d4 fd f3 b6 00 40 6a e0 31 26 e9 78 d5 00 40 6b 15 99 99 99 99 9a 00 40 6b 24 49 ba 5e 35 3f 00 40 6b 63 02 0c 49 ba 5e 00 40 6b a1 ba 5e 35 3f 7d 00 40 6b e0 72 b0 20 c4 9c 00 40 6c 1f 33 33 33 33 33 00 40 6c 5d eb 85 1e b8 52 00 40 6c 9c a3 d7 0a 3d 71 00 40 6c db 64 5a 1c ac 08 00 40 6d 12 1c ac 08 31 27 00 40 6d 50 d4 fd f3 b6 46 00 40 6d 8f 8d 4f df 3b 64 00 40 6d ce 4d d2 f1 a9 fc 00 40 6e 0b ae 14 7a e1 48 00 40 6e 4a 66 66 66 66 66 00 40 6e 89 26 e9 78 d4 fe 00 40 6e c7 df 3b 64 5a 1d 00 40 6f 06 97 8d 4f df 3b 00 40 6f 39 4f df 3b 64 5a 00 40 6f 78 08 31 26 e9 79 00 40 6f b6 c8 b4 39 58 10 00 40 6f bc 20 c4 9b a5 e3 00 40 6f bd 70 a3 d7 0a 3d 00 40 6f fc 31 26 e9 78 d5 00 40 70 1d 74 bc 6a 7e fa 00 40 70 3c d0 e5 60 41 89 00 40 70 5c 31 26 e9 78 d5 00 40 70 7b 8d 4f df 3b 64 00 40 70 96 e9 78 d4 fd f4 00 40 70 98 3d 70 a3 d7 0a 00 40 70 b6 45 a1 ca c0 83 00 40 70 d5 a1 ca c0 83 12 00 40 70 f5 02 0c 49 ba 5e 00 40 71 14 5e 35 3f 7c ee 00 40 71 33 ba 5e 35 3f 7d 00 40 71 53 1a 9f be 76 c9 00 40 71 72 76 c8 b4 39 58 00 40 71 91 d2 f1 a9 fb e7 00 40 71 b1 2f 1a 9f be 77 00 40 71 d0 8f 5c 28 f5 c3 00 40 71 ef eb 85 1e b8 52 00 40 72 0f 47 ae 14 7a e1 00 40 72 2e a7 ef 9d b2 2d 00 40 72 4e 04 18 93 74 bc 00 40 72 6d 60 41 89 37 4c 00 40 72 8c bc 6a 7e f9 db 00 40 72 ac 1c ac 08 31 27 00 40 72 cb 78 d4 fd f3 b6 00 40 72 ea d4 fd f3 b6 46 00 40 73 0a 35 3f 7c ed 91 00 40 73 29 91 68 72 b0 21 00 40 73 48 ed 91 68 72 b0 00 40 73 68 4d d2 f1 a9 fc 00 40 73 87 a9 fb e7 6c 8b 00 40 73 a7 06 24 dd 2f 1b 00 40 73 c6 62 4d d2 f1 aa 00 40 73 e5 c2 8f 5c 28 f6 00 40 74 05 1e b8 51 eb 85 00 40 74 24 7a e1 47 ae 14 00 40 74 43 db 22 d0 e5 60 00 40 74 63 37 4b c6 a7 f0 00 40 74 82 93 74 bc 6a 7f 00 40 74 a1 ef 9d b2 2d 0e 00 40 74 c1 4f df 3b 64 5a 00 40 74 e0 ac 08 31 26 e9 00 40 75 00 08 31 26 e9 79 00 40 75 1f 68 72 b0 20 c5 00 40 75 3e c4 9b a5 e3 54 00 40 75 5e 20 c4 9b a5 e3 00 40 75 7d 81 06 24 dd 2f 00 40 75 9c dd 2f 1a 9f be 00 40 75 bc 39 58 10 62 4e 00 40 75 db 95 81 06 24 dd 00 40 75 fa f5 c2 8f 5c 29 00 40 76 1a 51 eb 85 1e b8 00 40 76 39 ae 14 7a e1 48 00 40 76 59 0e 56 04 18 93 00 40 76 78 6a 7e f9 db 23 00 40 76 97 c6 a7 ef 9d b2 00 40 76 b7 22 d0 e5 60 42 00 40 76 d6 83 12 6e 97 8d 00 40 76 f5 df 3b 64 5a 1d 00 40 77 15 3b 64 5a 1c ac 00 40 77 34 9b a5 e3 53 f8 00 40 77 53 f7 ce d9 16 87 00 40 77 73 53 f7 ce d9 17 00 40 77 92 b0 20 c4 9b a6 00 40 77 b2 10 62 4d d2 f2 00 40 77 d1 6c 8b 43 95 81 00 40 77 f0 c8 b4 39 58 10 00 40 78 10 28 f5 c2 8f 5c 00 40 78 2f 85 1e b8 51 ec 00 40 78 4e e1 47 ae 14 7b 00 40 78 6e 3d 70 a3 d7 0a 00 40 78 8d 9d b2 2d 0e 56 00 40 78 ac f9 db 22 d0 e5 00 40 78 cc 56 04 18 93 75 00 40 78 eb b6 45 a1 ca c1 00 40 79 0b 12 6e 97 8d 50 00 40 79 2a 6e 97 8d 4f df 00 40 79 49 ce d9 16 87 2b 00 40 79 69 2b 02 0c 49 ba 00 40 79 88 87 2b 02 0c 4a 00 40 79 a7 e3 53 f7 ce d9 00 40 79 c7 43 95 81 06 25 00 40 79 e6 9f be 76 c8 b4 00 40 7a 05 fb e7 6c 8b 44 00 40 7a 25 5c 28 f5 c2 8f 00 40 7a 44 b8 51 eb 85 1f 00 40 7a 64 14 7a e1 47 ae 00 40 7a 83 70 a3 d7 0a 3d 00 40 7a a2 d0 e5 60 41 89 00 40 7a c2 2d 0e 56 04 19 00 40 7a e1 89 37 4b c6 a8 00 40 7b 00 e9 78 d4 fd f4 00 40 7b 20 45 a1 ca c0 83 00 40 7b 3f a1 ca c0 83 12 00 40 7b 5f 02 0c 49 ba 5e 00 40 7b 7e 5e 35 3f 7c ee 00 40 7b 9d ba 5e 35 3f 7d 00 40 7b bd 16 87 2b 02 0c 00 40 7b dc 76 c8 b4 39 58 00 40 7b fb d2 f1 a9 fb e7 00 40 7c 1b 2f 1a 9f be 77 00 40 7c 3a 8f 5c 28 f5 c3 00 40 7c 59 eb 85 1e b8 52 00 40 7c 79 47 ae 14 7a e1 00 40 7c 98 a3 d7 0a 3d 71 00 40 7c b8 04 18 93 74 bc 00 40 7c d7 60 41 89 37 4c 00 40 7c f6 bc 6a 7e f9 db 00 40 7d 16 1c ac 08 31 27 00 40 7d 35 78 d4 fd f3 b6 00 40 7d 54 d4 fd f3 b6 46 00 40 7d 74 31 26 e9 78 d5 00 40 7d 93 91 68 72 b0 21 00 40 7d b2 ed 91 68 72 b0 00 40 7d d2 49 ba 5e 35 3f 00 40 7d f1 a9 fb e7 6c 8b 00 40 7e 11 06 24 dd 2f 1b 00 40 7e 30 62 4d d2 f1 aa 00 40 7e 4f c2 8f 5c 28 f6 00 40 7e 55 16 87 2b 02 0c 00 40 7e 74 72 b0 20 c4 9c 00 40 7e 93 d2 f1 a9 fb e7 00 40 7e b3 2f 1a 9f be 77 00 40 7e d2 8b 43 95 81 06 00 40 7e f1 eb 85 1e b8 52 00 40 7f 11 47 ae 14 7a e1 00 40 7f 30 a3 d7 0a 3d 71 00 40 7f 50 00 00 00 00 00 00 40 7f 6f 60 41 89 37 4c 00 40 7f 8e bc 6a 7e f9 db 00 40 7f ae 18 93 74 bc 6a 00 40 7f cd 78 d4 fd f3 b6 00 40 7f ec d4 fd f3 b6 46 00 40 80 06 18 93 74 bc 6a 00 40 80 15 c8 b4 39 58 10 00 40 80 25 76 c8 b4 39 58 00 40 80 35 24 dd 2f 1a a0 00 40 80 44 d2 f1 a9 fb e7 00 40 80 54 83 12 6e 97 8d 00 40 80 64 31 26 e9 78 d5 00 40 80 73 df 3b 64 5a 1d 00 40 80 83 8f 5c 28 f5 c3 00 40 80 93 3d 70 a3 d7 0a 00 40 80 a2 eb 85 1e b8 52 00 40 80 b2 99 99 99 99 9a 00 40 80 c2 49 ba 5e 35 3f 00 40 80 d1 f7 ce d9 16 87 00 40 80 e1 a5 e3 53 f7 cf 00 40 80 f1 56 04 18 93 75 00 40 81 01 04 18 93 74 bc 00 40 81 10 b2 2d 0e 56 04 00 40 81 20 62 4d d2 f1 aa 00 40 81 30 10 62 4d d2 f2 00 40 81 3f be 76 c8 b4 39 00 40 81 4f 6c 8b 43 95 81 00 40 81 5f 1c ac 08 31 27 00 40 81 6e ca c0 83 12 6f 00 40 81 7e 78 d4 fd f3 b6 00 40 81 8e 28 f5 c2 8f 5c 00 40 81 9d d7 0a 3d 70 a4 00 40 81 ad 85 1e b8 51 ec 00 40 81 bd 33 33 33 33 33 00 40 81 cc e3 53 f7 ce d9 00 40 81 dc 91 68 72 b0 21 00 40 81 ec 3f 7c ed 91 68 00 40 81 fb ef 9d b2 2d 0e 00 40 82 0b 9d b2 2d 0e 56 00 40 82 1b 4b c6 a7 ef 9e 00 40 82 2a f9 db 22 d0 e5 00 40 82 33 fd f3 b6 45 a2 00 40 82 43 ac 08 31 26 e9 00 40 82 53 5a 1c ac 08 31 00 0d 66 69 6c 65 70 6f 73 69 74 69 6f 6e 73 0a 00 00 01 33 00 40 b6 d2 00 00 00 00 00 00 40 db 4f 80 00 00 00 00 00 40 f0 67 e0 00 00 00 00 00 40 f8 41 50 00 00 00 00 00 41 00 30 98 00 00 00 00 00 41 04 fd 28 00 00 00 00 00 41 07 a2 10 00 00 00 00 00 41 07 e6 68 00 00 00 00 00 41 11 da 7c 00 00 00 00 00 41 16 a7 dc 00 00 00 00 00 41 1b 35 6c 00 00 00 00 00 41 1f d4 bc 00 00 00 00 00 41 22 4b 5e 00 00 00 00 00 41 24 ab 78 00 00 00 00 00 41 27 20 e8 00 00 00 00 00 41 29 a7 04 00 00 00 00 00 41 2c 14 f6 00 00 00 00 00 41 2e 82 f8 00 00 00 00 00 41 30 a9 29 00 00 00 00 00 41 32 1f e6 00 00 00 00 00 41 33 69 a0 00 00 00 00 00 41 34 cb df 00 00 00 00 00 41 36 33 96 00 00 00 00 00 41 37 94 db 00 00 00 00 00 41 38 e2 54 00 00 00 00 00 41 3a 4d 10 00 00 00 00 00 41 3b a4 1e 00 00 00 00 00 41 3d 31 ba 00 00 00 00 00 41 3e b3 75 00 00 00 00 00 41 40 09 72 80 00 00 00 00 41 40 a7 34 00 00 00 00 00 41 41 1f bf 80 00 00 00 00 41 41 a1 79 80 00 00 00 00 41 42 27 3c 80 00 00 00 00 41 42 b0 11 00 00 00 00 00 41 43 3c 27 80 00 00 00 00 41 43 c6 d7 80 00 00 00 00 41 44 4e 36 80 00 00 00 00 41 44 d9 50 80 00 00 00 00 41 45 60 9f 00 00 00 00 00 41 45 c5 39 80 00 00 00 00 41 46 00 9d 00 00 00 00 00 41 46 32 5e 00 00 00 00 00 41 46 64 c6 00 00 00 00 00 41 46 c3 cc 00 00 00 00 00 41 47 5a 90 80 00 00 00 00 41 48 00 08 80 00 00 00 00 41 48 cd 6d 80 00 00 00 00 41 49 98 39 80 00 00 00 00 41 4a 5b 8b 80 00 00 00 00 41 4b 2c db 00 00 00 00 00 41 4c 0f 67 00 00 00 00 00 41 4c ed c9 80 00 00 00 00 41 4d c9 d3 80 00 00 00 00 41 4e a7 83 80 00 00 00 00 41 4f 82 e1 00 00 00 00 00 41 50 2a 2b 00 00 00 00 00 41 50 89 39 c0 00 00 00 00 41 50 df 38 40 00 00 00 00 41 51 37 93 40 00 00 00 00 41 51 9c 32 80 00 00 00 00 41 52 0b 0b c0 00 00 00 00 41 52 68 52 80 00 00 00 00 41 52 bb 21 80 00 00 00 00 41 53 08 7d c0 00 00 00 00 41 53 5a 6b c0 00 00 00 00 41 53 ab e1 00 00 00 00 00 41 53 fc d3 c0 00 00 00 00 41 54 4b b2 40 00 00 00 00 41 54 97 2d 40 00 00 00 00 41 54 dd d1 40 00 00 00 00 41 55 28 c8 40 00 00 00 00 41 55 74 d6 c0 00 00 00 00 41 55 c6 28 40 00 00 00 00 41 56 11 3a 40 00 00 00 00 41 56 5b a7 40 00 00 00 00 41 56 c2 97 80 00 00 00 00 41 57 18 d0 00 00 00 00 00 41 57 6f 38 00 00 00 00 00 41 57 c1 05 80 00 00 00 00 41 58 0e f9 40 00 00 00 00 41 58 61 d9 40 00 00 00 00 41 58 be 21 c0 00 00 00 00 41 59 1b a3 00 00 00 00 00 41 59 7d b7 80 00 00 00 00 41 59 de c4 80 00 00 00 00 41 5a 38 6f c0 00 00 00 00 41 5a 97 be 40 00 00 00 00 41 5b 06 5f 00 00 00 00 00 41 5b 7b 62 80 00 00 00 00 41 5b e8 4c c0 00 00 00 00 41 5c 59 4c 40 00 00 00 00 41 5c c6 99 00 00 00 00 00 41 5d 37 eb 00 00 00 00 00 41 5d a1 4a 00 00 00 00 00 41 5e 05 00 00 00 00 00 00 41 5e 69 d8 00 00 00 00 00 41 5e d1 e9 80 00 00 00 00 41 5f 35 61 80 00 00 00 00 41 5f 99 a3 80 00 00 00 00 41 5f f8 ea 40 00 00 00 00 41 60 2a b8 c0 00 00 00 00 41 60 5c 12 40 00 00 00 00 41 60 90 a9 60 00 00 00 00 41 60 c6 b7 c0 00 00 00 00 41 60 fe 01 00 00 00 00 00 41 61 31 40 00 00 00 00 00 41 61 5e 86 00 00 00 00 00 41 61 8a fc a0 00 00 00 00 41 61 b9 43 00 00 00 00 00 41 61 e7 16 80 00 00 00 00 41 62 17 b3 80 00 00 00 00 41 62 40 7d 40 00 00 00 00 41 62 44 d4 60 00 00 00 00 41 62 72 5d 00 00 00 00 00 41 62 a4 b7 00 00 00 00 00 41 62 e2 3d a0 00 00 00 00 41 63 1c b5 a0 00 00 00 00 41 63 59 85 c0 00 00 00 00 41 63 91 80 00 00 00 00 00 41 63 c4 75 c0 00 00 00 00 41 63 f6 d6 20 00 00 00 00 41 64 1a 9d 80 00 00 00 00 41 64 42 0c e0 00 00 00 00 41 64 66 c9 a0 00 00 00 00 41 64 86 97 80 00 00 00 00 41 64 a7 b3 e0 00 00 00 00 41 64 ca 51 a0 00 00 00 00 41 64 ed 31 60 00 00 00 00 41 65 18 18 60 00 00 00 00 41 65 42 43 00 00 00 00 00 41 65 70 d6 00 00 00 00 00 41 65 a0 a8 00 00 00 00 00 41 65 a5 0d 40 00 00 00 00 41 65 a6 30 a0 00 00 00 00 41 65 c8 96 40 00 00 00 00 41 65 e5 8e 80 00 00 00 00 41 66 05 ed 20 00 00 00 00 41 66 24 bc 40 00 00 00 00 41 66 3f a7 00 00 00 00 00 41 66 5d 8e c0 00 00 00 00 41 66 5f 95 40 00 00 00 00 41 66 85 5a c0 00 00 00 00 41 66 ac 52 20 00 00 00 00 41 66 d3 8b e0 00 00 00 00 41 66 fa e4 00 00 00 00 00 41 67 24 d0 c0 00 00 00 00 41 67 4e d9 c0 00 00 00 00 41 67 7a 55 a0 00 00 00 00 41 67 a3 1d e0 00 00 00 00 41 67 cb f9 40 00 00 00 00 41 67 f7 24 e0 00 00 00 00 41 68 23 ce a0 00 00 00 00 41 68 3c fb 00 00 00 00 00 41 68 5d f1 00 00 00 00 00 41 68 7d 50 40 00 00 00 00 41 68 9a 37 40 00 00 00 00 41 68 b6 4b a0 00 00 00 00 41 68 d6 08 c0 00 00 00 00 41 68 f7 fd e0 00 00 00 00 41 69 1a 44 c0 00 00 00 00 41 69 3a 2a 80 00 00 00 00 41 69 5c 07 20 00 00 00 00 41 69 7e ec 20 00 00 00 00 41 69 9e 44 20 00 00 00 00 41 69 b8 f3 c0 00 00 00 00 41 69 d5 ec 40 00 00 00 00 41 69 f3 d9 00 00 00 00 00 41 6a 10 74 e0 00 00 00 00 41 6a 2d 33 00 00 00 00 00 41 6a 4b 9e a0 00 00 00 00 41 6a 72 eb 00 00 00 00 00 41 6a 9b fd 40 00 00 00 00 41 6a cd 8e e0 00 00 00 00 41 6b 01 aa 60 00 00 00 00 41 6b 33 d5 a0 00 00 00 00 41 6b 64 ca a0 00 00 00 00 41 6b 8f 90 00 00 00 00 00 41 6b bf f1 00 00 00 00 00 41 6b f0 4f 20 00 00 00 00 41 6c 1d 1f c0 00 00 00 00 41 6c 47 b7 00 00 00 00 00 41 6c 72 ee e0 00 00 00 00 41 6c 9a dc 60 00 00 00 00 41 6c c0 04 a0 00 00 00 00 41 6c e1 5a a0 00 00 00 00 41 6d 03 4f e0 00 00 00 00 41 6d 28 24 60 00 00 00 00 41 6d 4c da e0 00 00 00 00 41 6d 71 93 80 00 00 00 00 41 6d 99 50 00 00 00 00 00 41 6d c0 4f e0 00 00 00 00 41 6d e5 c9 40 00 00 00 00 41 6e 08 fe c0 00 00 00 00 41 6e 2d 90 c0 00 00 00 00 41 6e 48 6a 40 00 00 00 00 41 6e 7b 65 40 00 00 00 00 41 6e ab 11 c0 00 00 00 00 41 6e e1 0e 00 00 00 00 00 41 6f 18 8b 00 00 00 00 00 41 6f 4c 1d 60 00 00 00 00 41 6f 7c 8f 00 00 00 00 00 41 6f ad 1e 20 00 00 00 00 41 6f dd 1e 60 00 00 00 00 41 70 06 b0 60 00 00 00 00 41 70 1d 3c 70 00 00 00 00 41 70 33 e5 60 00 00 00 00 41 70 49 a9 00 00 00 00 00 41 70 5d 8d f0 00 00 00 00 41 70 71 9c 80 00 00 00 00 41 70 84 c4 c0 00 00 00 00 41 70 99 79 30 00 00 00 00 41 70 ac ec 30 00 00 00 00 41 70 c0 bd f0 00 00 00 00 41 70 d5 bb 00 00 00 00 00 41 70 ea c4 a0 00 00 00 00 41 70 ff 77 10 00 00 00 00 41 71 13 be 30 00 00 00 00 41 71 28 10 40 00 00 00 00 41 71 3c e0 10 00 00 00 00 41 71 52 e4 b0 00 00 00 00 41 71 6a 7f 70 00 00 00 00 41 71 81 b6 20 00 00 00 00 41 71 97 a1 d0 00 00 00 00 41 71 ae 53 b0 00 00 00 00 41 71 c4 2a 50 00 00 00 00 41 71 d9 b0 30 00 00 00 00 41 71 ef 51 20 00 00 00 00 41 72 05 34 90 00 00 00 00 41 72 1b 83 f0 00 00 00 00 41 72 30 d1 e0 00 00 00 00 41 72 45 bc 00 00 00 00 00 41 72 5d 36 00 00 00 00 00 41 72 73 aa 30 00 00 00 00 41 72 89 37 20 00 00 00 00 41 72 9d b9 50 00 00 00 00 41 72 b1 3c 50 00 00 00 00 41 72 c6 08 40 00 00 00 00 41 72 da dc 60 00 00 00 00 41 72 ef 73 a0 00 00 00 00 41 73 05 1b c0 00 00 00 00 41 73 1a bc 80 00 00 00 00 41 73 2f fb c0 00 00 00 00 41 73 46 ff 30 00 00 00 00 41 73 5d 18 00 00 00 00 00 41 73 75 51 60 00 00 00 00 41 73 8a df 80 00 00 00 00 41 73 a0 69 d0 00 00 00 00 41 73 b6 e4 b0 00 00 00 00 41 73 ce 66 50 00 00 00 00 41 73 e5 91 b0 00 00 00 00 41 73 fa 77 50 00 00 00 00 41 74 0f 1a f0 00 00 00 00 41 74 24 e8 10 00 00 00 00 41 74 29 e9 a0 00 00 00 00 41 74 37 50 10 00 00 00 00 41 74 40 3f b0 00 00 00 00 41 74 49 41 40 00 00 00 00 41 74 53 1b 60 00 00 00 00 41 74 5e 1c 30 00 00 00 00 41 74 68 73 80 00 00 00 00 41 74 72 4d d0 00 00 00 00 41 74 7c 61 40 00 00 00 00 41 74 87 63 20 00 00 00 00 41 74 92 10 e0 00 00 00 00 41 74 9b 9a d0 00 00 00 00 41 74 a7 96 70 00 00 00 00 41 74 b0 93 20 00 00 00 00 41 74 be 44 70 00 00 00 00 41 74 c5 96 e0 00 00 00 00 41 74 d2 f4 70 00 00 00 00 41 74 e3 19 00 00 00 00 00 41 74 f3 c5 30 00 00 00 00 41 75 04 72 d0 00 00 00 00 41 75 15 c5 00 00 00 00 00 41 75 28 b8 30 00 00 00 00 41 75 3b eb 10 00 00 00 00 41 75 4f 45 40 00 00 00 00 41 75 63 1e b0 00 00 00 00 41 75 77 6e e0 00 00 00 00 41 75 8b af 50 00 00 00 00 41 75 9e ed 40 00 00 00 00 41 75 b2 1d 80 00 00 00 00 41 75 c5 66 90 00 00 00 00 41 75 d9 14 c0 00 00 00 00 41 75 ec aa 20 00 00 00 00 41 75 ff 1d 90 00 00 00 00 41 76 10 ed 90 00 00 00 00 41 76 23 32 20 00 00 00 00 41 76 38 3d 20 00 00 00 00 41 76 50 26 60 00 00 00 00 41 76 68 93 f0 00 00 00 00 41 76 7b eb d0 00 00 00 00 41 76 83 d6 d0 00 00 00 00 41 76 8f 6a 90 00 00 00 00 41 76 97 5c 00 00 00 00 00 41 76 9f 4a 40 00 00 00 00 41 76 a7 37 00 00 00 00 00 41 76 af 9f f0 00 00 00 00 41 76 c2 85 f0 00 00 00 00 41 76 cb f1 70 00 00 00 00 41 76 d5 56 00 00 00 00 00 41 76 de b9 30 00 00 00 00 41 76 e8 17 90 00 00 00 00 41 76 f4 57 c0 00 00 00 00 41 76 fb 87 20 00 00 00 00 41 77 01 7f 10 00 00 00 00"));
//    std::shared_ptr<cygnal::Buffer> hex5 = hex2mem("00 0a 6f 6e 4d 65 74 61 44 61 74 61 08 00 00 00 0a 00 08 64 75 72 61 74 69 6f 6e 00 40 6e 47 74 bc 6a 7e fa 00 0d 76 69 64 65 6f 64 61 74 61 72 61 74 65 00 40 67 28 32 e3 7f 02 e6 00 15 6c 61 73 74 6b 65 79 66 72 61 6d 65 74 69 6d 65 73 74 61 6d 70 00 40 6e 03 2b 02 0c 49 ba 00 14 6c 61 73 74 6b 65 79 66 72 61 6d 65 6c 6f 63 61 74 69 6f 6e 00 41 55 41 92 80 00 00 00 00 07 63 72 65 61 74 6f 72 02 00 0d 59 6f 75 54 75 62 65 2c 20 49 6e 63 2e 00 0f 6d 65 74 61 64 61 74 61 63 72 65 61 74 6f 72 02 00 1a 59 6f 75 54 75 62 65 20 4d 65 74 61 64 61 74 61 20 49 6e 6a 65 63 74 6f 72 2e 00 09 66 6c 76 73 6f 75 72 63 65 02 00 04 63 64 62 70 00 0c 68 61 73 6b 65 79 66 72 61 6d 65 73 01 01 00 0b 68 61 73 6d 65 74 61 64 61 74 61 01 01 00 09 6b 65 79 66 72 61 6d 65 73 03 00 05 74 69 6d 65 73 0a 00 00 00 7a 00 00 00 00 00 00 00 00 00 00 40 00 16 87 2b 02 0c 4a 00 40 0c be 76 c8 b4 39 58 00 40 14 92 6e 97 8d 4f df 00 40 1b 8f 5c 28 f5 c2 8f 00 40 20 49 37 4b c6 a7 f0 00 40 23 d8 93 74 bc 6a 7f 00 40 27 57 0a 3d 70 a3 d7 00 40 2a a3 53 f7 ce d9 17 00 40 2d de 35 3f 7c ed 91 00 40 30 9d 70 a3 d7 0a 3d 00 40 32 b1 26 e9 78 d4 fe 00 40 34 13 b6 45 a1 ca c1 00 40 36 05 a1 ca c0 83 12 00 40 37 ab 85 1e b8 51 ec 00 40 39 05 a1 ca c0 83 12 00 40 3a cd 4f df 3b 64 5a 00 40 3c 5a 1c ac 08 31 27 00 40 3e 21 ca c0 83 12 6f 00 40 3f c7 ae 14 7a e1 48 00 40 40 9d 91 68 72 b0 21 00 40 41 8e 14 7a e1 47 ae 00 40 42 82 d0 e5 60 41 89 00 40 43 62 6e 97 8d 4f df 00 40 44 4e b8 51 eb 85 1f 00 40 44 ef 1a 9f be 76 c9 00 40 45 ec 49 ba 5e 35 3f 00 40 46 ed b2 2d 0e 56 04 00 40 47 a7 6c 8b 43 95 81 00 40 48 8f 7c ed 91 68 73 00 40 49 80 00 00 00 00 00 00 40 4a 78 f5 c2 8f 5c 29 00 40 4b 61 06 24 dd 2f 1b 00 40 4c 99 58 10 62 4d d3 00 40 4d 5f 9d b2 2d 0e 56 00 40 4e 54 5a 1c ac 08 31 00 40 4f 44 dd 2f 1a 9f be 00 40 50 1e f9 db 22 d0 e5 00 40 50 8a 8f 5c 28 f5 c3 00 40 50 f1 eb 85 1e b8 52 00 40 51 7d 2f 1a 9f be 77 00 40 51 e0 51 eb 85 1e b8 00 40 52 84 ed 91 68 72 b0 00 40 53 01 68 72 b0 20 c5 00 40 53 82 1c ac 08 31 27 00 40 53 fa 5e 35 3f 7c ee 00 40 54 70 83 12 6e 97 8d 00 40 54 d3 b6 45 a1 ca c1 00 40 55 3d 2f 1a 9f be 77 00 40 55 ce c8 b4 39 58 10 00 40 56 3e 97 8d 4f df 3b 00 40 56 bb 12 6e 97 8d 50 00 40 57 2f 1a 9f be 76 c9 00 40 57 af ce d9 16 87 2b 00 40 58 28 10 62 4d d2 f2 00 40 58 af 1a 9f be 76 c9 00 40 59 1a c0 83 12 6e 98 00 40 59 95 1e b8 51 eb 85 00 40 5a 07 0a 3d 70 a3 d7 00 40 5a 96 87 2b 02 0c 4a 00 40 5b 21 ba 5e 35 3f 7d 00 40 5b 93 b6 45 a1 ca c1 00 40 5b ff 4b c6 a7 ef 9e 00 40 5c 88 72 b0 20 c4 9c 00 40 5c fa 5e 35 3f 7c ee 00 40 5d 76 d9 16 87 2b 02 00 40 5d e4 8b 43 95 81 06 00 40 5e 6b 95 81 06 24 dd 00 40 5e f8 f5 c2 8f 5c 29 00 40 5f 8e c8 b4 39 58 10 00 40 60 0a e9 78 d4 fd f4 00 40 60 47 0a 3d 70 a3 d7 00 40 60 89 81 06 24 dd 2f 00 40 60 c8 cc cc cc cc cd 00 40 61 00 b4 39 58 10 62 00 40 61 49 81 06 24 dd 2f 00 40 61 91 37 4b c6 a7 f0 00 40 61 e3 85 1e b8 51 ec 00 40 62 43 85 1e b8 51 ec 00 40 62 7d 89 37 4b c6 a8 00 40 62 c9 81 06 24 dd 2f 00 40 63 12 45 a1 ca c0 83 00 40 63 5c 20 c4 9b a5 e3 00 40 63 9b 6c 8b 43 95 81 00 40 63 db c6 a7 ef 9d b2 00 40 64 16 d9 16 87 2b 02 00 40 64 55 16 87 2b 02 0c 00 40 64 8e 14 7a e1 47 ae 00 40 64 d7 e7 6c 8b 43 96 00 40 65 18 41 89 37 4b c7 00 40 65 51 37 4b c6 a7 f0 00 40 65 9e 3d 70 a3 d7 0a 00 40 65 e2 d0 e5 60 41 89 00 40 66 14 62 4d d2 f1 aa 00 40 66 60 5a 1c ac 08 31 00 40 66 a1 c2 8f 5c 28 f6 00 40 66 d9 a9 fb e7 6c 8b 00 40 67 1e 3d 70 a3 d7 0a 00 40 67 67 0a 3d 70 a3 d7 00 40 67 a9 81 06 24 dd 2f 00 40 67 f7 8d 4f df 3b 64 00 40 68 43 85 1e b8 51 ec 00 40 68 89 26 e9 78 d4 fe 00 40 68 d9 4f df 3b 64 5a 00 40 69 29 81 06 24 dd 2f 00 40 69 68 cc cc cc cc cd 00 40 69 aa 35 3f 7c ed 91 00 40 69 fc 7a e1 47 ae 14 00 40 6a 31 37 4b c6 a7 f0 00 40 6a 7c 20 c4 9b a5 e3 00 40 6a d0 83 12 6e 97 8d 00 40 6b 1e 97 8d 4f df 3b 00 40 6b 70 dd 2f 1a 9f be 00 40 6b aa e9 78 d4 fd f4 00 40 6b f6 d9 16 87 2b 02 00 40 6c 40 b4 39 58 10 62 00 40 6c 80 00 00 00 00 00 00 40 6c d2 45 a1 ca c0 83 00 40 6d 20 5a 1c ac 08 31 00 40 6d 68 18 93 74 bc 6a 00 40 6d b6 24 dd 2f 1a a0 00 40 6e 03 2b 02 0c 49 ba 00 0d 66 69 6c 65 70 6f 73 69 74 69 6f 6e 73 0a 00 00 00 7a 00 40 a3 a0 00 00 00 00 00 00 40 e6 1a 40 00 00 00 00 00 40 f4 b9 40 00 00 00 00 00 40 fe 2a 10 00 00 00 00 00 41 04 6d b8 00 00 00 00 00 41 09 13 30 00 00 00 00 00 41 0e 65 48 00 00 00 00 00 41 11 ac 10 00 00 00 00 00 41 14 60 bc 00 00 00 00 00 41 17 09 38 00 00 00 00 00 41 19 ce f4 00 00 00 00 00 41 1c 7a 90 00 00 00 00 00 41 1e e9 4c 00 00 00 00 00 41 21 0d 16 00 00 00 00 00 41 22 84 dc 00 00 00 00 00 41 23 bb be 00 00 00 00 00 41 25 2d b6 00 00 00 00 00 41 26 76 5e 00 00 00 00 00 41 27 d8 50 00 00 00 00 00 41 29 44 e6 00 00 00 00 00 41 2a 90 6e 00 00 00 00 00 41 2b e5 2c 00 00 00 00 00 41 2d 27 36 00 00 00 00 00 41 2e 65 fe 00 00 00 00 00 41 2f c3 02 00 00 00 00 00 41 30 73 07 00 00 00 00 00 41 31 26 69 00 00 00 00 00 41 31 d5 84 00 00 00 00 00 41 32 67 fc 00 00 00 00 00 41 33 0f c8 00 00 00 00 00 41 33 c3 ec 00 00 00 00 00 41 34 75 9c 00 00 00 00 00 41 35 29 c6 00 00 00 00 00 41 35 f8 37 00 00 00 00 00 41 36 99 17 00 00 00 00 00 41 37 4d 7f 00 00 00 00 00 41 37 fb e3 00 00 00 00 00 41 38 a0 fc 00 00 00 00 00 41 39 46 90 00 00 00 00 00 41 39 e7 17 00 00 00 00 00 41 3a 9c dc 00 00 00 00 00 41 3b 40 c0 00 00 00 00 00 41 3c 0e ff 00 00 00 00 00 41 3c bb e0 00 00 00 00 00 41 3d 77 91 00 00 00 00 00 41 3e 26 48 00 00 00 00 00 41 3e d2 57 00 00 00 00 00 41 3f 79 f2 00 00 00 00 00 41 40 13 eb 80 00 00 00 00 41 40 6e 58 00 00 00 00 00 41 40 c1 2d 80 00 00 00 00 41 41 17 c0 80 00 00 00 00 41 41 6b 52 80 00 00 00 00 41 41 c5 2e 00 00 00 00 00 41 42 17 da 80 00 00 00 00 41 42 76 36 00 00 00 00 00 41 42 c7 be 00 00 00 00 00 41 43 25 e5 80 00 00 00 00 41 43 7a b1 00 00 00 00 00 41 43 d4 07 80 00 00 00 00 41 44 2c e0 00 00 00 00 00 41 44 7d fe 00 00 00 00 00 41 44 d0 61 80 00 00 00 00 41 45 27 55 00 00 00 00 00 41 45 7c c3 80 00 00 00 00 41 45 d4 0a 00 00 00 00 00 41 46 24 09 80 00 00 00 00 41 46 7a 64 80 00 00 00 00 41 46 d8 6d 00 00 00 00 00 41 47 39 0a 80 00 00 00 00 41 47 94 d5 00 00 00 00 00 41 47 e8 6d 00 00 00 00 00 41 48 3f f1 80 00 00 00 00 41 48 a0 5e 80 00 00 00 00 41 48 f4 7d 00 00 00 00 00 41 49 52 36 00 00 00 00 00 41 49 b4 80 00 00 00 00 00 41 4a 1b 96 80 00 00 00 00 41 4a 86 8c 00 00 00 00 00 41 4a d6 96 00 00 00 00 00 41 4b 34 06 80 00 00 00 00 41 4b 8d 4b 80 00 00 00 00 41 4b ee 20 00 00 00 00 00 41 4c 45 14 80 00 00 00 00 41 4c 9e 5b 00 00 00 00 00 41 4c fc a7 00 00 00 00 00 41 4d 56 b1 80 00 00 00 00 41 4d ac f2 00 00 00 00 00 41 4e 12 de 80 00 00 00 00 41 4e 6b 88 80 00 00 00 00 41 4e bf 7b 00 00 00 00 00 41 4f 21 e4 00 00 00 00 00 41 4f 82 04 80 00 00 00 00 41 4f d4 10 80 00 00 00 00 41 50 1a 94 80 00 00 00 00 41 50 48 ce 40 00 00 00 00 41 50 70 de 80 00 00 00 00 41 50 9f 3a 40 00 00 00 00 41 50 cf 1d 80 00 00 00 00 41 50 fc a0 80 00 00 00 00 41 51 2b a4 80 00 00 00 00 41 51 58 7d 80 00 00 00 00 41 51 86 45 40 00 00 00 00 41 51 b6 0f 80 00 00 00 00 41 51 e6 71 40 00 00 00 00 41 52 10 8d 00 00 00 00 00 41 52 3c e0 40 00 00 00 00 41 52 6d 33 c0 00 00 00 00 41 52 96 68 40 00 00 00 00 41 52 c5 8a 40 00 00 00 00 41 52 f6 6a c0 00 00 00 00 41 53 25 4e 40 00 00 00 00 41 53 58 7d 80 00 00 00 00 41 53 85 b4 00 00 00 00 00 41 53 ba 0c 40 00 00 00 00 41 53 f4 6f c0 00 00 00 00 41 54 2d 9d c0 00 00 00 00 41 54 70 96 80 00 00 00 00 41 54 a7 13 80 00 00 00 00 41 54 d8 8f 00 00 00 00 00 41 55 0f ae 80 00 00 00 00 41 55 41 92 80 00 00 00 00");
    Element *el5 = flv.decodeMetaData(hex5);
    if (el5 == 0) {
        notest = true;
    } 
    if (notest) {
        runtest.untested("Decoded FLV MetaData object FLVParser");
    } else {
//        el1->dump();
        if ((el5->getType() == Element::ECMA_ARRAY_AMF0)
            && (el5->propertySize() == 10)) {
            runtest.pass("Decoded FLV MetaData object FLVParser");
        } else {
            runtest.fail("Decoded FLV MetaData object from FLVParser");
        }
        delete el5;
    }
#endif
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
