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
#include <log.h>
#include <iostream>
#include <string>

#include "dejagnu.h"
#include "as_object.h"
#include "arg_parser.h"
#include "amf.h"
#include "buffer.h"
#include "network.h"
#include "element.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

// Prototypes for test cases
static void test_encoding();
static void test_string();
static void test_object();
static void test_boolean();

// Enable the display of memory allocation and timing data
static bool memdebug = false;

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
{    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'w', "write",         Arg_parser::no  },
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

    // run the tests
    test_encoding();
    test_object();
}

void
test_encoding()
{
    // This is a 8 byte wide double data type in hex
    const char *x = "40 83 38 00 00 00 00 00";
    Buffer *buf1 = hex2mem(x);
    double num = *(double *)buf1->reference();
    swapBytes(&num, amf::AMF_NUMBER_SIZE); // we alwasy encode in big endian format
    Buffer *encnum = AMF::encodeNumber(num);
    // A number AMF object has only one header byte, which is the type field.
    if ((*encnum->reference() == Element::NUMBER) &&
        (memcmp(buf1->reference(), encnum->reference()+1, amf::AMF_NUMBER_SIZE) == 0)) {
        runtest.pass("Encoded AMF Number");
    } else {
        runtest.fail("Encoded AMF Number");
    }
    delete buf1;
    delete encnum;
    
    // Encode a boolean. Although we know a bool is only one character, for AMF,
    // it's actually a two byte short instead.
    bool flag = true;
    const char *x2 = "00 01";
    Buffer *buf2 = hex2mem(x2);
    boost::uint16_t sht = *(boost::uint16_t *)buf2->reference();
    swapBytes(&sht, sizeof(boost::uint16_t)); // we always encode in big endian format
    Buffer *encbool = AMF::encodeBoolean(flag);
    // A boolean AMF object has only one header byte, which is the type field.
    // The data is always encoded as a two byte value, even though *we* know a bool is just
    // one. Obviously AMF goes back to the days of 16 bit ints, and bool was the same size.
    // Ya gotta love old K&R C code for binary formats.
    if ((*encbool->reference() == Element::BOOLEAN) &&
        (encbool->size() == 3) &&
        (memcmp(buf2->reference(), encbool->reference()+1, sizeof(boost::uint16_t)) == 0)) {
        runtest.pass("Encoded AMF Boolean");
    } else {
        runtest.fail("Encoded AMF Boolean");
    }
    delete buf2;
    delete encbool;
    
    // Encode a String.
    string str = "Jerry Garcia rules";
    Buffer *encstr = AMF::encodeString(str);
    // A String AMF object has a 3 bytes head, the type, and a two byte length.
    if ((*encstr->reference() == Element::STRING) &&
        (encstr->size() == str.size() + AMF_HEADER_SIZE) &&
        (memcmp(encstr->reference() + 3, str.c_str(), str.size()) == 0)) {
        runtest.pass("Encoded AMF String");
    } else {
        runtest.fail("Encoded AMF String");
    }
    delete encstr;
    
    // Encode a NULL String.
    Buffer *encnull = AMF::encodeNullString();
    boost::uint16_t len = *(boost::uint16_t *)(encnull->reference() + 1);
    // A NULL String AMF object has just 3 bytes, the type, and a two byte length, which is zero.
    if ((*encnull->reference() == Element::STRING) &&
        (encnull->size() == AMF_HEADER_SIZE) && 
        (len == 0)) {
        runtest.pass("Encoded AMF NULL String");
    } else {
        runtest.fail("Encoded AMF NULL String");
    }
    delete encnull;
}

void
test_object()
{
    Element top;
    top.makeObject();

    Element *child1 = new Element;
    child1->makeString("app", "oflaDemo");
    top.addChild(child1);
    
    Element *child2 = new Element;
    child2->makeString("flashVer", "LNX 9,0,31,0");
    top.addChild(child2);

    Element *child3 = new Element;
    child3->makeString("swfUrl", "http://www.red5.nl/tools/publisher/publisher.swf");
    top.addChild(child3);

    if (top.childrenSize() == 3) {
        runtest.pass("Adding children");
    } else {
        runtest.fail("Adding children");
    }

    // Encode an object
//    string str = "appl";
    Buffer *encobj = top.encode();
    if (encobj == 0) {
        runtest.unresolved("Encoded Object");
        return;
    }

    const char *x = "03 00 03 61 70 70 02 00 08 6f 66 6c 61 44 65 6d 6f 00 08 66 6c 61 73 68 56 65 72 02 00 0c 4c 4e 58 20 39 2c 30 2c 33 31 2c 30 00 06 73 77 66 55 72 6c 02 00 30 68 74 74 70 3a 2f 2f 77 77 77 2e 72 65 64 35 2e 6e 6c 2f 74 6f 6f 6c 73 2f 70 75 62 6c 69 73 68 65 72 2f 70 75 62 6c 69 73 68 65 72 2e 73 77 66 09";
    Buffer *buf1 = hex2mem(x);
    if ((*encobj->reference() == Element::OBJECT) &&
        (memcmp(buf1->reference(), encobj->reference(), 102) == 0)) {
        runtest.pass("Encoded Object");
    } else {
        runtest.fail("Encoded Object");
    }
    delete buf1;
}

// amf::encodeDate(unsigned char*)
// amf::AMF::encodeLongString(unsigned char*, int)
// amf::AMF::encodeStrictArray(unsigned char*, int)
// amf::AMF::encodeTypedObject(unsigned char*, int)
// amf::AMF::encodeUnsupported()
// amf::AMF::encodeNull()
// amf::AMF::encodeObject(unsigned char*, int)
// amf::AMF::encodeString(std::string const&)
// amf::AMF::encodeElement(amf::Element*)
// amf::AMF::encodeVariable(amf::Element*)
// amf::AMF::encodeECMAArray(unsigned char*, int)
// amf::AMF::encodeMovieClip(unsigned char*, int)
// amf::AMF::encodeRecordSet(unsigned char*, int)
// amf::AMF::encodeReference(unsigned char*, int)
// amf::AMF::encodeUndefined()
// amf::AMF::encodeXMLObject(unsigned char*, int)

// amf::AMF::extractAMF(unsigned char*)
// amf::AMF::extractVariable(unsigned char*)

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


