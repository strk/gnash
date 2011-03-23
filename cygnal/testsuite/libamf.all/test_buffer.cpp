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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_DEJAGNU_H

#include <iostream>
#include <regex.h>
#include <cstdio>
#include <cerrno>
#include <fstream>
#include <cstring>
#include <vector>
#include <boost/cstdint.hpp>

#include "dejagnu.h"
#include "log.h"
#include "rc.h"
#include "network.h"
#ifdef HAVE_MALLINFO
#include "gmemory.h"
#endif
#include "buffer.h"
#include "arg_parser.h"
#include "GnashException.h"

using namespace std;
using namespace amf;
using namespace gnash;
using namespace boost;

static void usage();

// Prototypes for test cases
static void test_resize();
static void test_construct();
static void test_copy();
static void test_find();
static void test_append();
static void test_remove();
static void test_destruct();
static void test_operators();

// Enable the display of memory allocation and timing data
static bool memdebug = false;

// We use the Memory profiling class to check the malloc buffers
// in the kernel to make sure the allocations and frees happen
// the way we expect them too. There is no real other way to tell.
#ifdef HAVE_MALLINFO
Memory *mem = 0;
#endif

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();
RcInitFile& rcfile = RcInitFile::getDefaultInstance();

int
main (int argc, char** argv)
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
              case 'w':
                  rcfile.useWriteLog(true);
                  log_debug(_("Logging to disk enabled"));
                  break;
                  
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }
    
#ifdef HAVE_MALLINFO
    if (memdebug) {
        mem = new Memory;
        mem->startStats();
    }
#endif
    
// these tests are bogus unless you have both mallinfo()
// and also have memory statistics gathering turned on.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    // test creating Buffers
    test_construct();
    // test destroying Buffers
    test_destruct();
#endif

    test_resize();
    test_copy();
    test_find();
    test_append();
    test_remove();
    test_operators();

// amf::Buffer::resize(unsigned int)
    
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        if (mem->analyze()) {
            runtest.pass("Buffer doesn't leak memory");
        } else {
            runtest.fail("Buffer leaks memory!");
        }
    }
#endif
    
    // cleanup
#ifdef HAVE_MALLINFO
    if (mem) {
        delete mem;
    }
#endif
}

void
test_resize()
{
    Buffer buf;
#ifdef HAVE_MALLINFO
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif    
    if (buf.size() == amf::NETBUFSIZE) {
        runtest.pass ("Buffer::size(NETBUFSIZE)");
    } else {
        runtest.fail ("Buffer::size(NETBUFSIZE)");
    }
    
#ifdef HAVE_MALLINFO
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif
    buf.resize(112);
#ifdef HAVE_MALLINFO
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif
    if (buf.size() == 112) {
         runtest.pass ("Buffer::resize(112)");
     } else {
         runtest.fail ("Buffer::resize(112)");
    }
#ifdef HAVE_MALLINFO
    if (memdebug) {
        mem->addStats(__LINE__);             // take a sample
    }
#endif

    string str = "Hello World";
    buf = str;
    buf.resize(5);
    if (memcmp(buf.begin(), str.c_str(), 5) == 0) {
        runtest.pass ("Buffer resize(5)");
    } else {
        runtest.fail ("Buffer resize(5)");
    }
}

void
test_copy()
{
    // Make some data for the buffers
    Network::byte_t *data = 0;
    data = new Network::byte_t[10];
    memset(data, 0, 10);
    for (size_t i=1; i<10; i++) {
        *(data + i) = i + '0';
    }

    Buffer buf1;
    Network::byte_t *ptr1 = 0;
    ptr1 = buf1.reference();

    buf1.copy(data, 10);
    if (memcmp(ptr1, data, 10) == 0) {
         runtest.pass ("Buffer::copy(Network::byte_t *, size_t)");
    } else {
         runtest.fail ("Buffer::copy(Network::byte_t *, size_t)");
    }

    const char *str = "I'm bored";
    string str1 = str;
    buf1 = str1;
    if (memcmp(ptr1, str, 9) == 0) {
         runtest.pass ("Buffer::operator=(std::string &)");
    } else {
         runtest.fail ("Buffer::operator=(std::string &)");
    }

    Buffer buf2;
    buf2 = str;
    Network::byte_t *ptr2 = buf2.reference();
    if (memcmp(ptr2, str, 9) == 0) {
         runtest.pass ("Buffer::operator=(const char *)");
    } else {
         runtest.fail ("Buffer::operator=(const char *)");
    }

    boost::uint16_t length = 12;
    Buffer buf3;
    buf3 = length;
    Network::byte_t *ptr3 = buf3.reference();
    boost::uint16_t newlen = *(reinterpret_cast<boost::uint16_t *>(ptr3));
    if (length == newlen) {
         runtest.pass ("Buffer::operator=(boost::uint16_t)");
    } else {
         runtest.fail ("Buffer::operator=(boost::uint16_t)");
    }

    double num = 1.2345;
    Buffer buf4;
    buf4 = num;

    // Copy the raw bytes used for the number into the temporary
    // data pointer, so we can do a comparison
    memcpy(data, &num, amf::AMF0_NUMBER_SIZE);

    if (memcmp(data, buf4.reference(), amf::AMF0_NUMBER_SIZE) == 0) {
         runtest.pass ("Buffer::operator=(double)");
    } else {
         runtest.fail ("Buffer::operator=(double)");
    }   

    Network::byte_t byte = 67;
    Buffer buf5;
    buf5 = byte;
    if (*buf5.reference() == 67) {
         runtest.pass ("Buffer::operator=(Network::byte_t)");
    } else {
         runtest.fail ("Buffer::operator=(Network::byte_t)");
    }

    amf::Element::amf0_type_e type = Element::NUMBER_AMF0;
    Buffer buf6;
    buf6 = type;
    if (*buf6.reference() == type) {
         runtest.pass ("Buffer::operator=(amf::Element::amf0_type_e)");
    } else {
         runtest.fail ("Buffer::operator=(amf::Element::amf0_type_e)");
    }
    
    bool flag = true;
    Buffer buf7;
    buf7 = flag;
    if (*buf7.reference() == flag) {
         runtest.pass ("Buffer::operator=(bool)");
    } else {
         runtest.fail ("Buffer::operator=(bool)");
    }
    

        // cleanup the temporary data
    delete[] data;
}

void
test_find()
{
    // Make some data for the buffers
    Network::byte_t *data = new Network::byte_t[10];
    for (size_t i=0; i<10; i++) {
        data[i] = i + 'a';
    }

    Buffer buf1, buf2, buf3;
    Network::byte_t *ptr1 = buf1.reference();

    // populate the buffer
    buf1.copy(data, 10);
    delete[] data;
    
    // See if we can find a character
    Network::byte_t *fptr = std::find(buf1.begin(), buf1.end(), 'c'); 
    if (fptr == (ptr1 + 2)) {
         runtest.pass ("Buffer::find(Network::byte_t)");
    } else {
         runtest.fail ("Buffer::find(Network::byte_t)");
    }

    const char *sub = "fgh";
#if 0
    Network::byte_t *ptr2 = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(sub));
#endif
    fptr = std::search(buf1.begin(), buf1.end(), sub, sub+3);
    if (fptr == (ptr1 + 5)) {
         runtest.pass ("Buffer::find(Network::byte_t *, size_t)");
    } else {
         runtest.fail ("Buffer::find(Network::byte_t *, size_t)");
    }

// amf::Buffer::init(unsigned int)
}

void
test_append()
{
    Buffer buf1;
    buf1.clear();
//    Network::byte_t *ptr1 = buf1.reference();

    Network::byte_t *data1 = new Network::byte_t[10];
    memset(data1, 0, 10);
    for (size_t i=0; i< 10; i++) {
        data1[i] = i + 'a';
    }
    Network::byte_t *data2 = new Network::byte_t[10];
    memset(data2, 0, 10);
    for (size_t i=0; i< 10; i++) {
        data2[i] = i + 'A';
    }

    // append a string of bytes
    Network::byte_t *data3 = new Network::byte_t[20];
    memcpy(data3, data1, 10);
    memcpy(data3+10, data2, 10);
    buf1.copy(data1, 10);
    buf1.append(data2, 10);
    if (memcmp(data3, buf1.reference(), 20) == 0) {
         runtest.pass ("Buffer::append(Network::byte_t *, size_t)");
    } else {
         runtest.fail ("Buffer::append(Network::byte_t *, size_t)");
    }

    // append an unsigned byte
    Buffer buf2(30);
    buf2.clear();
    buf2.copy(data1, 10);
    Network::byte_t byte = '@';
    buf2 += byte;
    memset(data3, 0, 20);
    memcpy(data3, data1, 10);
    *(data3 + 10) = '@';
    if (memcmp(data3, buf2.reference(), 11) == 0) {
         runtest.pass ("Buffer::operator+=(Network::byte_t)");
    } else {
         runtest.fail ("Buffer::operator+=(Network::byte_t)");
    }

    // Append a number
    double num = 1.2345;
    Buffer buf3;
    buf3.clear();
    buf3.copy(data1, 10);
    buf3 += num;
    
    memset(data3, 0, 20);
    memcpy(data3, data1, 10);
    memcpy(data3 + 10, &num, sizeof(double));
    if (memcmp(data3, buf3.reference(), 10+sizeof(double)) == 0) {
         runtest.pass ("Buffer::operator+=(double)");
    } else {
         runtest.fail ("Buffer::operator+=(double)");
    }

    string str1 = "Writing test cases";
    const char *str2 = "is so tedious";
    string str3 = str1 + str2;
    Buffer buf6(50);
    buf6 = str1;
    buf6 += str2;
    if (memcmp(buf6.reference(), str3.c_str(), str3.size()) == 0) {
        runtest.pass ("Buffer::operator+=(const string &)");
    } else {
        runtest.fail ("Buffer::operator+=(const string &)");
    }

    boost::uint16_t length = 1047;
    Buffer buf7(70);
    buf7.copy(data1, 10);
    buf7 += length;
    if (memcmp(buf7.reference() + 10, &length, sizeof(boost::uint16_t)) == 0) {
        runtest.pass ("Buffer::operator+=(boost::uint16_t)");
    } else {
        runtest.fail ("Buffer::operator+=(boost::uint16_t)");
    }

    buf7 += buf6;
    // Network::byte_t *ptr1 = buf7.reference() + 10 + sizeof(boost::uint16_t);
    // Network::byte_t *ptr2 = buf6.reference();
    if (memcmp(buf7.reference() + 10 + sizeof(boost::uint16_t), buf6.reference(), 30) == 0) {
        runtest.pass ("Buffer::operator+=(Buffer &)");
    } else {
        runtest.fail ("Buffer::operator+=(Buffer &)");
    }

    bool flag = true;
    Buffer buf8;
    buf8.copy(data1, 10);
    buf8 += flag;
    if (*(buf8.reference() + 10) == 1) {
        runtest.pass ("Buffer::operator+=(bool)");
    } else {
        runtest.fail ("Buffer::operator+=(bool)");
    }
    
    // Clean up temporary data
    delete[] data1;
    delete[] data2;
    delete[] data3;
    
}

void
test_remove()
{
    Network::byte_t *data1 = new Network::byte_t[20];
    memset(data1, 0, 20);
    Network::byte_t *data2 = new Network::byte_t[20];
    memset(data2, 0, 20);
    Network::byte_t *data3 = new Network::byte_t[20];
    memset(data3, 0, 20);

    // populate a buffer with some data
    for (size_t i=0; i< 19; i++) {
        data1[i] = i + 'a';
    }

    // Build identical buffer nissing one character
    memcpy(data2, data1, 6);
    memcpy(data2 + 6, data1 + 7, 20-7);

    // Remove a single byte
    Network::byte_t byte = 'g';
    Buffer buf1(20);
    buf1.clear();
    buf1.copy(data1, 20);
    buf1.remove(byte);
    if (memcmp(data2, buf1.reference(), 19) == 0) {
         runtest.pass ("Buffer::remove(Network::byte_t)");
    } else {
         runtest.fail ("Buffer::remove(Network::byte_t)");
    }
    
    Buffer buf2(20);
    buf2.clear();
    buf2.copy(data1, 20);
    buf2.remove(6);
    if (memcmp(data2, buf2.reference(), 18) == 0) {
         runtest.pass ("Buffer::remove(int)");
    } else {
         runtest.fail ("Buffer::remove(int)");
    }

    // Remove a range of bytes
    memcpy(data3, data1, 6);
    memcpy(data3 + 6, data1 + 9, 1);
    
    Buffer buf3(20);
    buf3.clear();
    buf3.copy(data1, 20);
    buf3.remove(6, 8);
    if (memcmp(data3, buf3.reference(), 6) == 0) {
         runtest.pass ("Buffer::remove(int, int)");
    } else {
         runtest.fail ("Buffer::remove(int, int)");
    }

    delete[] data1;
    delete[] data2;
    delete[] data3;
}

void
test_construct()
{
// these tests are bogus unless you have both mallinfo()
// and also have memory statistics gathering turned on.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    bool valgrind = false;
    
    size_t fudge = sizeof(long *)*5;
    
    Memory mem(5);
    mem.addStats(__LINE__);             // take a sample
    Buffer buf1;
    mem.addStats(__LINE__);             // take a sample
    size_t diff = mem.diffStats() - sizeof(buf1);    
    if (diff > NETBUFSIZE) {
        valgrind = true;
        log_debug("Running this test case under valgrind screws up mallinfo(), so the results get skewed");
    }
    // Different systems allocate memory slightly differently, so about all we can do to see
    // if it worked is check to make sure it's within a tight range of possible values.
     if ((buf1.size() == NETBUFSIZE) && (diff >= (NETBUFSIZE - fudge)) && diff <= (NETBUFSIZE + fudge)) {
        runtest.pass ("Buffer::Buffer()");
    } else {
        if (valgrind) {
            runtest.unresolved("Buffer::Buffer()) under valgrind");
        } else {
            runtest.fail("Buffer::Buffer()");
        }
    }
    
    mem.addStats(__LINE__);             // take a sample
    Buffer buf2(124);
    mem.addStats(__LINE__);             // take a sample
    diff = mem.diffStats() - sizeof(long *);
    if ((buf2.size() == 124) && (124 - fudge) && diff <= (124 + fudge)) {
        runtest.pass ("Buffer::Buffer(size_t)");
    } else {
        if (valgrind) {
            runtest.unresolved("Buffer::Buffer(size_t) under valgrind");
        } else {
            runtest.fail("Buffer::Buffer(size_t)");
        }
    }
#endif
}

// make sure when we delete a Buffer, *all* the allocated
// memory goes away. As the only way to do this is to examine
// the malloc buffers in the kernel, this will only work on
// POSIX conforming systems, and probabably only Linux & BSD.
void
test_destruct()
{
// these tests are bogus unless you have both mallinfo()
// and also have memory statistics gathering turned on.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    Memory mem(5);
    mem.addStats(__LINE__);             // take a sample
    Buffer *buf1, *buf2;

    mem.startCheckpoint();
    buf1 = new Buffer(NETBUFSIZE);
    delete buf1;
    
    if (mem.endCheckpoint()) {
        runtest.pass ("Buffer::~Buffer()");
    } else {
        runtest.fail ("Buffer::~Buffer()");
    }

    mem.startCheckpoint();
    buf2 = new Buffer(124);
    delete buf2;
    
    if (mem.endCheckpoint()) {
        runtest.pass ("Buffer::~Buffer(size_t)");
    } else {
        runtest.fail ("Buffer::~Buffer(size_t)");
    }
#endif
}

void
test_operators()
{
    // Make some data for the buffers
    Buffer buf1, buf2;
    // valgrind gets pissed unless we zero the memory. Constructing
    // a buffer doesn't clear the memory to save speed, and it's
    // also unnecessary normally, but makes debugging easier.
    buf1.clear();
    buf2.clear();

    boost::uint8_t *ptr1 = buf1.reference();
    for (size_t i=1; i< buf1.size(); i++) {
        ptr1[i] = i;
    }

    // buf1 has data, but buf2 doesn't, so if the
    // equivalance test fails, then this passed.
    buf2.clear();
    if (buf2 == buf1) {
         runtest.fail ("Buffer::operator==(Buffer &)");
     } else {
         runtest.pass ("Buffer::operator==(Buffer &)");
    }

    // This makes the new buffer be identical to the
    // the source buffer, including copying all the data.
    buf2 = buf1;
    if (buf1 == buf2) {
         runtest.pass ("Buffer::operator=(Buffer &)");
     } else {
         runtest.fail ("Buffer::operator=(Buffer &)");
    }

    Buffer *buf3, *buf4;
    buf3 = new Buffer;
    boost::uint8_t *ptr2 = buf3->reference();
    for (size_t i=1; i< buf3->size(); i++) {
        ptr2[i] = i + 'a';
    }
    buf4 = new Buffer;
    if (buf3 == buf4) {
         runtest.fail ("Buffer::operator==(Buffer *)");
     } else {
         runtest.pass ("Buffer::operator==(Buffer *)");
    }
    delete buf4;

    // This makes the new buffer be identical to the
    // the source buffer, including copying all the data.
    buf4 = buf3;
    if (buf3 == buf4) {
         runtest.pass ("Buffer::operator=(Buffer *)");
    } else {
         runtest.fail ("Buffer::operator=(Buffer *)");
    }
    delete buf3;
    
    Buffer buf5(10);
    buf5.clear();
    boost::uint8_t *ptr3 = buf5.reference();
    buf5 += 'a';
    buf5 += 'b';
    buf5 += 'c';
    if (memcmp(ptr3, "abc", 3) == 0) {
         runtest.pass ("Buffer::operator+=(char)");
    } else {
         runtest.fail ("Buffer::operator+=(char)");
    }

    Buffer buf6(6);
    buf6.clear();
    buf6 += 'D';
    buf6 += 'E';
    buf6 += 'F';    
    buf5 += buf6;
    ptr3 = buf5.reference();    // refresh the pointer, as it changes
                                // on a resize()
    // when appending Buffers, this destination has enough space to hold
    // the allocated bytes from the source.
    if ((memcmp(ptr3, "abcDEF", 6) == 0) && (buf5.size() == 10)) {
         runtest.pass ("Buffer::operator+=(Buffer &)");
    } else {
         runtest.fail ("Buffer::operator+=(Buffer &)");
    }

    bool caught = false;

    // make the source Buffer have more data
    buf6 += 'A';
    buf6 += 'B';
    buf6 += 'C';
    try {
        buf5 += buf6;
    }

    catch (GnashException& ge) {
        caught = true;
//        log_debug("Got exeception from operator+=: %s", ge.what());
    }
    // when appending Buffers, this destination doesn't have enough space to
    // hold the allocated bytes from the source, so is supposed to throw an
    //exception.
    if (caught) {
         runtest.pass ("Buffer::operator+=(Buffer &) error");
    } else {
         runtest.fail ("Buffer::operator+=(Buffer &) error");
    }
}

static void
usage()
{
    cout << _("test_buffer - test Buffer class") << endl
         << endl
         << _("Usage: test_buffer [options...]") << endl
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
