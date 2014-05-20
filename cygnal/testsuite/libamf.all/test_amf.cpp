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
#include "GnashException.h"
#include "check.h"
//#include "dejagnu.h"
#include "arg_parser.h"
#include "amf.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "gmemory.h"

using cygnal::AMF;
using cygnal::Element;
using cygnal::Buffer;
using cygnal::swapBytes;

using namespace gnash;
using namespace std;

static void usage (void);

// Prototypes for test cases
static void test_encoding();
// static void test_string();
static void test_object();
// static void test_boolean();
static void test_array();

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
main(int argc, char **argv)
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
    test_object();
    test_array();

#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
   if (memdebug) {
        if (mem->analyze()) {
            runtest.pass("cygnal::AMF doesn't leak memory");
        } else {
            runtest.fail("cygnal::AMF leaks memory!");
        }
    }
#endif

   // cleanup
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (mem) {
        delete mem;
    }
#endif
}

void
test_encoding()
{
    // This is a 8 byte wide double data type in hex
    std::shared_ptr<Buffer> buf1(new Buffer("40 83 38 00 00 00 00 00"));
    double num = *(reinterpret_cast<double *>(buf1->reference()));
    swapBytes(&num, cygnal::AMF0_NUMBER_SIZE); // we always encode in big endian format

#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif    
    std::shared_ptr<Buffer> encnum = cygnal::AMF::encodeNumber(num);
    // A number cygnal::AMF object has only one header byte, which is the type field.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif
    if ((*encnum->reference() == Element::NUMBER_AMF0) &&
        (memcmp(buf1->reference(), encnum->reference()+1, cygnal::AMF0_NUMBER_SIZE) == 0)) {
        runtest.pass("Encoded cygnal::AMF Number");
    } else {
        runtest.fail("Encoded cygnal::AMF Number");
    }
    
    // Encode a boolean. Although we know a bool is only one character, for cygnal::AMF,
    // it's actually a two byte short instead.
    {
        bool flag = true;
        std::shared_ptr<Buffer> buf2(new Buffer("01 01"));
        std::uint16_t sht = *(std::uint16_t *)buf2->reference();
        swapBytes(&sht, sizeof(std::uint16_t)); // we always encode in big endian format
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
        if (memdebug) {
            mem->addStats(__LINE__);             // take a sample
        }
#endif
        std::shared_ptr<Buffer> encbool = cygnal::AMF::encodeBoolean(flag);
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
        if (memdebug) {
            mem->addStats(__LINE__);             // take a sample
        }
#endif
    
        // A boolean cygnal::AMF object has only one header byte, which is the type field.
        // AMF3 changes this to being two different type, FALSE & TRUE
        // which are finally only one byte apiece.
        if ((*encbool->reference() == Element::BOOLEAN_AMF0) &&
            (encbool->size() == 2) &&
            (memcmp(buf2->reference(), encbool->reference(), sizeof(std::uint16_t)) == 0)) {
            runtest.pass("Encoded cygnal::AMF Boolean");
        } else {
            runtest.fail("Encoded cygnal::AMF Boolean");
        }
    }
    
    // Encode a String.
    {
        string str = "Jerry Garcia rules";
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
        if (memdebug) {
            mem->addStats(__LINE__);             // take a sample
        }
#endif
        std::shared_ptr<Buffer> buf = cygnal::AMF::encodeString(str);
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
        if (memdebug) {
            mem->addStats(__LINE__);             // take a sample
        }
#endif
        check_equals(*buf->reference(), Element::STRING_AMF0);
        check_equals(buf->size(), str.size()+cygnal::AMF_HEADER_SIZE);
        // A String cygnal::AMF object has a 3 bytes head, the type, and a two byte length.
        check((memcmp(buf->reference() + 3, str.c_str(), str.size()) == 0));

        Element el(str);
        buf = cygnal::AMF::encodeElement(el);
        
        check_equals(*buf->reference(), Element::STRING_AMF0);
        check_equals(buf->size(), str.size()+cygnal::AMF_HEADER_SIZE);
        // A String cygnal::AMF object has a 3 bytes head, the type, and a two byte length.
        check((memcmp(buf->reference() + 3, str.c_str(), str.size()) == 0));
    }
    
    // Encode a NULL String.
    {
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
        if (memdebug) {
            mem->addStats(__LINE__);             // take a sample
        }
#endif
        std::shared_ptr<Buffer> buf = cygnal::AMF::encodeNullString();
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
        if (memdebug) {
            mem->addStats(__LINE__);             // take a sample
        }
#endif
        std::uint16_t len = *(std::uint16_t *)(buf->reference() + 1);

        // A NULL String cygnal::AMF object has just 3 bytes, the type, and a two byte length, which is zero.
        check_equals(*buf->reference(), Element::STRING_AMF0);
        check_equals(buf->size(), (size_t)cygnal::AMF_HEADER_SIZE);
        check_equals(len, 0);

        Element el;
        el.makeNullString();
        buf = cygnal::AMF::encodeElement(el);
        len = *(std::uint16_t *)(buf->reference() + 1);

        // A NULL String cygnal::AMF object has just 3 bytes, the type, and a two byte length, which is zero.
        check_equals(*buf->reference(), Element::STRING_AMF0);
        check_equals(buf->size(), (size_t)cygnal::AMF_HEADER_SIZE);
        check_equals(len, 0);
    }

    cygnal::AMF amf;
    Element el1;
    std::uint16_t index = 1;
    el1.makeReference(index);
    if (el1.to_short() == 1) {
        runtest.pass("Made Reference");
    } else {
        runtest.fail("Made Reference");
    }    

    std::shared_ptr<cygnal::Buffer> buf2 = amf.encodeElement(el1);
    if ((*buf2->reference() == Element::REFERENCE_AMF0)
        && (*(buf2->reference() + 1) == 0)
        && (*(buf2->reference() + 2) == 1)) {
        runtest.pass("Encoded Reference");
    } else {
        runtest.fail("Encoded Reference");
    }    

    std::shared_ptr<Buffer> buf3(new Buffer("07 00 01"));
    std::shared_ptr<cygnal::Element> el3 = amf.extractAMF(buf3);
    if ((el3->getType() == Element::REFERENCE_AMF0)
        && (el3->to_short() == 1)) {
        runtest.pass("Extracted Reference");
    } else {
        runtest.fail("Extracted Reference");
    }  

}

void
test_array()
{
    Element top;
    cygnal::AMF amf;
    top.makeObject();

    std::shared_ptr<Buffer> hex1(new Buffer("08 00 00 00 0a 00 08 64 75 72 61 74 69 6f 6e 00 40 ad 04 14 7a e1 47 ae 00 05 77 69 64 74 68 00 40 74 00 00 00 00 00 00 00 06 68 65 69 67 68 74 00 40 6e 00 00 00 00 00 00 00 0d 76 69 64 65 6f 64 61 74 61 72 61 74 65 00 40 72 c0 00 00 00 00 00 00 09 66 72 61 6d 65 72 61 74 65 00 40 39 00 00 00 00 00 00 00 0c 76 69 64 65 6f 63 6f 64 65 63 69 64 00 40 10 00 00 00 00 00 00 00 0d 61 75 64 69 6f 64 61 74 61 72 61 74 65 00 40 58 00 00 00 00 00 00 00 0a 61 75 64 69 6f 64 65 6c 61 79 00 3f a3 74 bc 6a 7e f9 db 00 0c 61 75 64 69 6f 63 6f 64 65 63 69 64 00 40 00 00 00 00 00 00 00 00 0c 63 61 6e 53 65 65 6b 54 6f 45 6e 64 01 01 00 00 09"));
    std::shared_ptr<cygnal::Element> el1 = amf.extractAMF(hex1);
    if ((el1->getType() == Element::ECMA_ARRAY_AMF0)
        && (el1->propertySize() == 10)) {
        runtest.pass("Extracted ECMA Array");
    } else {
        runtest.fail("Extracted ECMA Array");
    }

    std::shared_ptr<Buffer> hex2(new Buffer("0a 00 00 00 c8 00 3f a4 7a e1 47 ae 14 7b 00 40 03 d7 0a 3d 70 a3 d7 00 40 13 85 1e b8 51 eb 85 00 40 1d 1e b8 51 eb 85 1f 00 40 23 5c 28 f5 c2 8f 5c 00 40 28 28 f5 c2 8f 5c 29 00 40 2c f5 c2 8f 5c 28 f6 00 40 30 e1 47 ae 14 7a e1 00 40 33 47 ae 14 7a e1 48 00 40 35 ae 14 7a e1 47 ae 00 40 38 14 7a e1 47 ae 14 00 40 3a 7a e1 47 ae 14 7b 00 40 3c e1 47 ae 14 7a e1 00 40 3f 47 ae 14 7a e1 48 00 40 40 d7 0a 3d 70 a3 d7 00 40 42 0a 3d 70 a3 d7 0a 00 40 43 3d 70 a3 d7 0a 3d 00 40 44 70 a3 d7 0a 3d 71 00 40 45 a3 d7 0a 3d 70 a4 00 40 46 d7 0a 3d 70 a3 d7 00 40 48 0a 3d 70 a3 d7 0a 00 40 49 3d 70 a3 d7 0a 3d 00 40 4a 70 a3 d7 0a 3d 71 00 40 4b a3 d7 0a 3d 70 a4 00 40 4c d7 0a 3d 70 a3 d7 00 40 4e 0a 3d 70 a3 d7 0a 00 40 4f 3d 70 a3 d7 0a 3d 00 40 50 38 51 eb 85 1e b8 00 40 50 d1 eb 85 1e b8 52 00 40 51 6b 85 1e b8 51 ec 00 40 52 05 1e b8 51 eb 85 00 40 52 9e b8 51 eb 85 1f 00 40 53 38 51 eb 85 1e b8 00 40 53 d1 eb 85 1e b8 52 00 40 54 6b 85 1e b8 51 ec 00 40 55 05 1e b8 51 eb 85 00 40 55 9e b8 51 eb 85 1f 00 40 56 38 51 eb 85 1e b8 00 40 56 d1 eb 85 1e b8 52 00 40 57 6b 85 1e b8 51 ec 00 40 58 05 1e b8 51 eb 85 00 40 58 9e b8 51 eb 85 1f 00 40 59 38 51 eb 85 1e b8 00 40 59 d1 eb 85 1e b8 52 00 40 5a 6b 85 1e b8 51 ec 00 40 5b 05 1e b8 51 eb 85 00 40 5b 9e b8 51 eb 85 1f 00 40 5c 38 51 eb 85 1e b8 00 40 5c d1 eb 85 1e b8 52 00 40 5d 6b 85 1e b8 51 ec 00 40 5e 05 1e b8 51 eb 85 00 40 5e 9e b8 51 eb 85 1f 00 40 5f 38 51 eb 85 1e b8 00 40 5f d1 eb 85 1e b8 52 00 40 60 35 c2 8f 5c 28 f6 00 40 60 82 8f 5c 28 f5 c3 00 40 60 cf 5c 28 f5 c2 8f 00 40 61 1c 28 f5 c2 8f 5c 00 40 61 68 f5 c2 8f 5c 29 00 40 61 b5 c2 8f 5c 28 f6 00 40 62 02 8f 5c 28 f5 c3 00 40 62 4f 5c 28 f5 c2 8f 00 40 62 9c 28 f5 c2 8f 5c 00 40 62 e8 f5 c2 8f 5c 29 00 40 63 35 c2 8f 5c 28 f6 00 40 63 82 8f 5c 28 f5 c3 00 40 63 cf 5c 28 f5 c2 8f 00 40 64 1c 28 f5 c2 8f 5c 00 40 64 68 f5 c2 8f 5c 29 00 40 64 b5 c2 8f 5c 28 f6 00 40 65 02 8f 5c 28 f5 c3 00 40 65 4f 5c 28 f5 c2 8f 00 40 65 9c 28 f5 c2 8f 5c 00 40 65 e8 f5 c2 8f 5c 29 00 40 66 35 c2 8f 5c 28 f6 00 40 66 82 8f 5c 28 f5 c3 00 40 66 cf 5c 28 f5 c2 8f 00 40 67 1c 28 f5 c2 8f 5c 00 40 67 68 f5 c2 8f 5c 29 00 40 67 b5 c2 8f 5c 28 f6 00 40 68 02 8f 5c 28 f5 c3 00 40 68 4f 5c 28 f5 c2 8f 00 40 68 9c 28 f5 c2 8f 5c 00 40 68 e8 f5 c2 8f 5c 29 00 40 69 35 c2 8f 5c 28 f6 00 40 69 82 8f 5c 28 f5 c3 00 40 69 cf 5c 28 f5 c2 8f 00 40 6a 1c 28 f5 c2 8f 5c 00 40 6a 68 f5 c2 8f 5c 29 00 40 6a b5 c2 8f 5c 28 f6 00 40 6b 02 8f 5c 28 f5 c3 00 40 6b 4f 5c 28 f5 c2 8f 00 40 6b 9c 28 f5 c2 8f 5c 00 40 6b e8 f5 c2 8f 5c 29 00 40 6c 35 c2 8f 5c 28 f6 00 40 6c 82 8f 5c 28 f5 c3 00 40 6c cf 5c 28 f5 c2 8f 00 40 6d 1c 28 f5 c2 8f 5c 00 40 6d 68 f5 c2 8f 5c 29 00 40 6d b5 c2 8f 5c 28 f6 00 40 6e 02 8f 5c 28 f5 c3 00 40 6e 4f 5c 28 f5 c2 8f 00 40 6e 9c 28 f5 c2 8f 5c 00 40 6e e8 f5 c2 8f 5c 29 00 40 6f 35 c2 8f 5c 28 f6 00 40 6f 82 8f 5c 28 f5 c3 00 40 6f cf 5c 28 f5 c2 8f 00 40 70 0e 14 7a e1 47 ae 00 40 70 34 7a e1 47 ae 14 00 40 70 5a e1 47 ae 14 7b 00 40 70 81 47 ae 14 7a e1 00 40 70 a7 ae 14 7a e1 48 00 40 70 ce 14 7a e1 47 ae 00 40 70 f4 7a e1 47 ae 14 00 40 71 1a e1 47 ae 14 7b 00 40 71 41 47 ae 14 7a e1 00 40 71 67 ae 14 7a e1 48 00 40 71 8e 14 7a e1 47 ae 00 40 71 b4 7a e1 47 ae 14 00 40 71 da e1 47 ae 14 7b 00 40 72 01 47 ae 14 7a e1 00 40 72 27 ae 14 7a e1 48 00 40 72 4e 14 7a e1 47 ae 00 40 72 74 7a e1 47 ae 14 00 40 72 9a e1 47 ae 14 7b 00 40 72 c1 47 ae 14 7a e1 00 40 72 e7 ae 14 7a e1 48 00 40 73 0e 14 7a e1 47 ae 00 40 73 34 7a e1 47 ae 14 00 40 73 5a e1 47 ae 14 7b 00 40 73 81 47 ae 14 7a e1 00 40 73 a7 ae 14 7a e1 48 00 40 73 ce 14 7a e1 47 ae 00 40 73 f4 7a e1 47 ae 14 00 40 74 1a e1 47 ae 14 7b 00 40 74 41 47 ae 14 7a e1 00 40 74 67 ae 14 7a e1 48 00 40 74 8e 14 7a e1 47 ae 00 40 74 b4 7a e1 47 ae 14 00 40 74 da e1 47 ae 14 7b 00 40 75 01 47 ae 14 7a e1 00 40 75 27 ae 14 7a e1 48 00 40 75 4e 14 7a e1 47 ae 00 40 75 74 7a e1 47 ae 14 00 40 75 9a e1 47 ae 14 7b 00 40 75 c1 47 ae 14 7a e1 00 40 75 e7 ae 14 7a e1 48 00 40 76 0e 14 7a e1 47 ae 00 40 76 34 7a e1 47 ae 14 00 40 76 5a e1 47 ae 14 7b 00 40 76 81 47 ae 14 7a e1 00 40 76 a7 ae 14 7a e1 48 00 40 76 ce 14 7a e1 47 ae 00 40 76 f4 7a e1 47 ae 14 00 40 77 1a e1 47 ae 14 7b 00 40 77 41 47 ae 14 7a e1 00 40 77 67 ae 14 7a e1 48 00 40 77 8e 14 7a e1 47 ae 00 40 77 b4 7a e1 47 ae 14 00 40 77 da e1 47 ae 14 7b 00 40 78 01 47 ae 14 7a e1 00 40 78 27 ae 14 7a e1 48 00 40 78 4e 14 7a e1 47 ae 00 40 78 74 7a e1 47 ae 14 00 40 78 9a e1 47 ae 14 7b 00 40 78 c1 47 ae 14 7a e1 00 40 78 e7 ae 14 7a e1 48 00 40 79 0e 14 7a e1 47 ae 00 40 79 34 7a e1 47 ae 14 00 40 79 5a e1 47 ae 14 7b 00 40 79 81 47 ae 14 7a e1 00 40 79 a7 ae 14 7a e1 48 00 40 79 ce 14 7a e1 47 ae 00 40 79 f4 7a e1 47 ae 14 00 40 7a 1a e1 47 ae 14 7b 00 40 7a 41 47 ae 14 7a e1 00 40 7a 67 ae 14 7a e1 48 00 40 7a 8e 14 7a e1 47 ae 00 40 7a b4 7a e1 47 ae 14 00 40 7a da e1 47 ae 14 7b 00 40 7b 01 47 ae 14 7a e1 00 40 7b 27 ae 14 7a e1 48 00 40 7b 4e 14 7a e1 47 ae 00 40 7b 74 7a e1 47 ae 14 00 40 7b 9a e1 47 ae 14 7b 00 40 7b c1 47 ae 14 7a e1 00 40 7b e7 ae 14 7a e1 48 00 40 7c 0e 14 7a e1 47 ae 00 40 7c 34 7a e1 47 ae 14 00 40 7c 5a e1 47 ae 14 7b 00 40 7c 81 47 ae 14 7a e1 00 40 7c a7 ae 14 7a e1 48 00 40 7c ce 14 7a e1 47 ae 00 40 7c f4 7a e1 47 ae 14 00 40 7d 1a e1 47 ae 14 7b 00 40 7d 41 47 ae 14 7a e1 00 40 7d 67 ae 14 7a e1 48 00 40 7d 82 8f 5c 28 f5 c3 00 40 7d 83 33 33 33 33 33 00 40 7d a9 99 99 99 99 9a"));
    std::shared_ptr<cygnal::Element> el2 = amf.extractAMF(hex2);
    if ((el2->getType() == Element::STRICT_ARRAY_AMF0)
        && (el2->propertySize() == 200)) {
        runtest.pass("Extracted Strict Array");
    } else {
        runtest.fail("Extracted Strict Array");
    }
//    delete hex2;
}
    
void
test_object()
{
    Element top;
    top.makeObject();

    std::shared_ptr<cygnal::Element> prop1(new Element);
    prop1->makeString("app", "oflaDemo");
    top.addProperty(prop1);
    
    std::shared_ptr<cygnal::Element> prop2(new Element);
    prop2->makeString("flashVer", "LNX 9,0,31,0");
    top.addProperty(prop2);

    std::shared_ptr<cygnal::Element> prop3(new Element);
    prop3->makeString("swfUrl", "http://www.red5.nl/tools/publisher/publisher.swf");
    top.addProperty(prop3);

    if (top.propertySize() == 3) {
        runtest.pass("Adding property");
    } else {
        runtest.fail("Adding property");
    }

    // Encode an object
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif
    std::shared_ptr<Buffer> encobj = top.encode();
//    std::shared_ptr<Buffer> encobj;
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif
    if (encobj == 0) {
        runtest.unresolved("Encoded Object");
        return;
    }

//    encobj->dump();
    std::shared_ptr<Buffer> buf1(new Buffer("03 00 03 61 70 70 02 00 08 6f 66 6c 61 44 65 6d 6f 00 08 66 6c 61 73 68 56 65 72 02 00 0c 4c 4e 58 20 39 2c 30 2c 33 31 2c 30 00 06 73 77 66 55 72 6c 02 00 30 68 74 74 70 3a 2f 2f 77 77 77 2e 72 65 64 35 2e 6e 6c 2f 74 6f 6f 6c 73 2f 70 75 62 6c 69 73 68 65 72 2f 70 75 62 6c 69 73 68 65 72 2e 73 77 66 00 00 09"));
    if ((*encobj->reference() == Element::OBJECT_AMF0) &&
        (memcmp(buf1->reference(), encobj->reference(), 101) == 0)) {
        runtest.pass("Encoded Object");
    } else {
        runtest.fail("Encoded Object");
    }

//    buf1->dump();
    
    cygnal::AMF amf_obj;
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif
    std::shared_ptr<cygnal::Element> newtop = amf_obj.extractAMF(buf1);
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif
    
    // FIXME: this test is a bit overly simple. We can tell from
    // the debugging messages plus this has been examined with GDB,
    // but this should have a more accurate test to make sure all
    // the child elements are correct all the time.
    if ((newtop->getType() == Element::OBJECT_AMF0)
        && (newtop->propertySize() >= 3)) {
        runtest.pass("Extracted Object");
    } else {
        runtest.fail("Extracted Object");
    }

}

// cygnal::cygnal::AMF::extractAMF(unsigned char*)
// cygnal::cygnal::AMF::extractVariable(unsigned char*)

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
