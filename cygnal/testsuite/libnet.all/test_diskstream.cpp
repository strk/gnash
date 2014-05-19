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

#if !defined(HAVE_WINSOCK_H) && !defined(__riscos__) && !defined(__OS2__)
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <iostream>
#include <string>
#include "as_value.h"
#include "as_object.h"

#include "dejagnu.h"
#include "as_object.h"
#include "arg_parser.h"
#include "buffer.h"
#include "diskstream.h"

using namespace cygnal;
using namespace gnash;
using namespace std;

static void usage (void);

// Prototypes for test cases
static void test();
static void test_mem();
static void create_file(const std::string &, size_t);

// Enable the display of memory allocation and timing data
// static bool memdebug = false;

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();
RcInitFile& rcfile = RcInitFile::getDefaultInstance();
static bool dump = false;

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

    // run the tests
    test();
    test_mem();
}

void
test()
{
    DiskStream ds1;

    // Create an array of printable ASCII characters
    int range = '~' - '!';
    char *buf = new char[range];
    for (int j=0; j<range; j++) {
        buf[j] = '!' + j;
    }

    create_file("outbuf.raw", 500);    
    ds1.open("outbuf.raw");

    // ptr should be the base address of the memory plus the offset
    boost::uint8_t *ptr = ds1.loadToMem(48);
//    boost::uint8_t *dsptr = ds1.get(); // cache the initial base address
    
    if ((ds1.get() == MAP_FAILED) || (ds1.get() == 0)) {
        runtest.unresolved("loadToMem(48)");
    } else {
        if ((memcmp(ds1.get(), buf, 48) == 0)
            && (memcmp(ptr, buf+48, range-48) == 0)) {
            runtest.pass("loadToMem(48)");
        } else {
            runtest.fail("loadToMem(48)");
        }
    }

    // as the offset is less than the memory pagesize, the pointer
    // should be the same as before, as it points to data in the
    // current segment. The temporary pointer should point to the
    // appropriate place in memory page.
    ptr = ds1.loadToMem(128);
    if ((ds1.get() == MAP_FAILED) || (ds1.get() == 0)) {
        runtest.unresolved("loadToMem(128)");
    } else {
        if ((memcmp(ds1.get(), buf, range) == 0)
            && (memcmp(ptr, buf+(128-range), 128-range) == 0)) {
            runtest.pass("loadToMem(128)");
        } else {
            runtest.fail("loadToMem(128)");
        }
    }

    // close the currently opened file
    ds1.close();
    if (ds1.getState() == DiskStream::CLOSED) {
        runtest.pass("close()");
    } else {
        runtest.fail("close()");
    }

    
    DiskStream ds2;
    // Create a bigger file that's larger than the page size
    create_file("outbuf2.raw", 12000);
    ds2.open("outbuf2.raw");
    ptr = ds2.loadToMem(6789);
    if ((ds2.get() == MAP_FAILED) || (ds2.get() == 0)) {
        runtest.unresolved("loadToMem(6789)");
    } else {
        if ((memcmp(ds2.get(), buf, range-4) == 0)
            && (memcmp(ptr, buf, range-4) == 0)) {
            runtest.pass("loadToMem(6789)");
        } else {
            runtest.fail("loadToMem(6789)");
        }
    }

    // test seeking in data files.
    ptr = ds2.seek(5100);
    if ((ds2.get() == MAP_FAILED) || (ds2.get() == 0)) {
        runtest.unresolved("seek(5100)");
    } else {
        if ((memcmp(ds2.get(), buf, range-4) == 0)
            && (memcmp(ptr, buf+78, range-78) == 0)) {
            runtest.pass("seek(5100)");
        } else {
            runtest.fail("seek(5100)");
        }
    }
    
//     if (dump) {
//         ds2.dump();
//     }

    delete[] buf;
    unlink("outbuf.raw");
    unlink("outbuf2.raw");
}

void
test_mem()
{
    std::shared_ptr<cygnal::Buffer> buf1(new cygnal::Buffer(12));
    *buf1 = "Hello World";
    // drop the null terminator byte we inherit when using a simnple
    // string for testing
    buf1->resize(buf1->size() - 1);

    DiskStream ds("fooBar1", *buf1);
    ds.writeToDisk();
    ds.close();

    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    
    if (stat("fooBar1", &st) == 0) {
        runtest.pass("DiskStream::writeToDisk()");
    } else {
        runtest.fail("DiskStream::writeToDisk()");
    }
}

/// \brief create a test file to read in later. This lets us create
/// files of arbitrary sizes.
void
create_file(const std::string &filespec, size_t size)
{
    // Open an output file
//    cerr << "Creating a test file.;
    
    int fd = open(filespec.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRGRP|S_IROTH);
    if (fd < 0) {
        perror("open");
    }

    // Create an array of printable ASCII characters
    size_t range = '~' - '!';
    char *buf = new char[range];
    for (size_t j=0; j<range; j++) {
        buf[j] = '!' + j;
    }

    // Write the false data to disk
    size_t total = 0;
    size_t ret = 0;
    for (size_t i=0; i<size; i+=range) {
        if ((size - total) < range) {
            ret = write(fd, buf, (size - total));
        } else {
            ret = write(fd, buf, range);
        }
        total += ret;
     }

    // cleanup
    delete[] buf;
    close(fd);
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

#else

int
main(int /*argc*/, char**)
{
  // nop
  return 0;  
}

#endif

