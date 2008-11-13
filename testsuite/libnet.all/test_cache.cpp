 // 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifndef __GNUC__
extern int optind, getopt(int, char *const *, const char *);
#endif

#include <sys/types.h>
#include <iostream>
#include <string>
#include <vector>
#include <regex.h>
#include <fcntl.h>

#include "log.h"
#include "cache.h"
#include "dejagnu.h"
#include "network.h"
#include "diskstream.h"

using namespace gnash;
using namespace std;

static void usage (void);
static void test (void);
static void test_errors (void);
static void create_file(const std::string &, size_t);

static TestState runtest;

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    int c;
    
    while ((c = getopt (argc, argv, "hdvsm:")) != -1) {
        switch (c) {
          case 'h':
              usage ();
              break;
              
          case 'v':
              dbglogfile.setVerbosity();
              break;
              
          default:
              usage ();
              break;
        }
    }

    test();
    test_errors();

    unlink("outbuf1.raw");
    unlink("outbuf2.raw");
    unlink("outbuf3.raw");
    unlink("outbuf4.raw");
}

static void
test (void)
{
    Cache cache;

    // Add a few path names
    cache.addPath("foo", "/bar/foo");
    cache.addPath("bar", "/foo/bar");
    cache.addPath("barfoo", "/foo/bar/barfoo");
    cache.addPath("foobar", "/foo/bar/foobar");

    if ((cache.findPath("foo") == "/bar/foo")
        && (cache.findPath("bar") == "/foo/bar")
        && (cache.findPath("barfoo") == "/foo/bar/barfoo")
        && (cache.findPath("foobar") == "/foo/bar/foobar")) {
        runtest.pass("addPath()/findPath()");
    } else {
        runtest.fail("addPath()/findPath()");
    }
    
    // FIXME: make these different, although it probably makes not difference.
    std::string resp1 = "HTTP/1.1\r\n404 Object Not Found\r\nServer: Microsoft-IIS/4.0\r\n\r\nDate: Sat, 08 Dec 2007 20:32:20 GMT\r\nConnection: close\r\nContent-Length: 461\r\nContent-Type: text/html";
    std::string resp2 = "HTTP/1.1\r\n404 Object Not Found\r\nServer: Microsoft-IIS/4.0\r\n\r\nDate: Sat, 08 Dec 2007 20:32:20 GMT\r\nConnection: close\r\nContent-Length: 461\r\nContent-Type: text/html";
    std::string resp3 = "HTTP/1.1\r\n404 Object Not Found\r\nServer: Microsoft-IIS/4.0\r\n\r\nDate: Sat, 08 Dec 2007 20:32:20 GMT\r\nConnection: close\r\nContent-Length: 461\r\nContent-Type: text/html";
    std::string resp4 = "HTTP/1.1\r\n404 Object Not Found\r\nServer: Microsoft-IIS/4.0\r\n\r\nDate: Sat, 08 Dec 2007 20:32:20 GMT\r\nConnection: close\r\nContent-Length: 461\r\nContent-Type: text/html";

    cache.addResponse("foo", resp1);
    cache.addResponse("bar", resp2);
    cache.addResponse("barfoo", resp3);
    cache.addResponse("foobar", resp4);
    if ((cache.findResponse("foo") == resp1)
        && (cache.findResponse("bar") == resp2)
        && (cache.findResponse("barfoo") == resp3)
        && (cache.findResponse("foobar") == resp4)) {
        runtest.pass("addResponse()/findResponse()");
    } else {
        runtest.fail("addResponse()/findResponse()");
    }

    DiskStream file1;
    create_file("outbuf1.raw", 100);
    file1.open("outbuf1.raw");

    DiskStream file2;
    create_file("outbuf2.raw", 200);
    file1.open("outbuf2.raw");

    DiskStream file3;
    create_file("outbuf3.raw", 300);
    file1.open("outbuf3.raw");

    DiskStream file4;
    create_file("outbuf4.raw", 400);
    file1.open("outbuf4.raw");

    cache.addFile("foo", &file1);
    cache.addFile("bar", &file2);
    cache.addFile("barfoo", &file3);
    cache.addFile("foobar", &file4);
    if ((cache.findFile("foo")->getFileSize() == file1.getFileSize())
         && (cache.findFile("bar")->getFileSize() == file2.getFileSize())) {
        runtest.pass("addFile()/findFile()");
    } else {
        runtest.fail("addFile()/findFile()");
    }
    
    if (dbglogfile.getVerbosity() > 0) {
        cache.dump();
    }

    file1.close();
    
}

static void
test_errors (void)
{
    Cache cache;
    // Add a few path names
    cache.addPath("foo", "/bar/foo");
    cache.addPath("bar", "/foo/bar");

    if ((cache.findPath("foo") == "/bar/foo")
        && (cache.findPath("bar") == "/foo/bar")
        && (cache.findPath("foobar").size() == 0)) {
        runtest.pass("Cache::findPath()");
    } else {
        runtest.fail("Cache::findPath()");
    }

    std::string resp1 = "HTTP/1.1\r\n404 Object Not Found\r\nServer: Microsoft-IIS/4.0\r\n\r\nDate: Sat, 08 Dec 2007 20:32:20 GMT\r\nConnection: close\r\nContent-Length: 461\r\nContent-Type: text/html";
    cache.addResponse("foo", resp1);
    cache.addResponse("bar", resp1);
    if ((cache.findResponse("foo") == resp1)
        && (cache.findResponse("bar") == resp1)
        && (cache.findResponse("foobar").size() == 0)) {
        runtest.pass("Cache::findResponse()");
    } else {
        runtest.fail("Cache::findResponse()");
    }

    DiskStream file1;
//    create_file("outbuf1.raw", 100);   it's created aleady in test().
    file1.open("outbuf1.raw");

    cache.addFile("foo", &file1);
    if ((cache.findFile("foo")->getFileSize() == file1.getFileSize())
         && (cache.findFile("bar") == 0)) {
        runtest.pass("Cache::addFile()/findFile()");
    } else {
        runtest.fail("Cache::addFile()/findFile()");
    }

    // see what happens if we try an uninitialized Cache.
    Cache c1;
    
    if (c1.findPath("foo").size() == 0) {
        runtest.pass("Cache::findPath(empty)");
    } else {
        runtest.fail("Cache::findPath(empty)");
    }

    if (c1.findResponse("foo").size() == 0) {
        runtest.pass("Cache::findResponse(empty)");
    } else {
        runtest.fail("Cache::findResponse(empty)");
    }

    if (c1.findFile("foo") == 0) {
        runtest.pass("Cache::findFile(empty)");
    } else {
        runtest.fail("Cache::findFile(empty)");
    }
    
}

/// \brief create a test file to read in later. This lets us create
/// files of arbitrary sizes.
void
create_file(const std::string &filespec, size_t size)
{
    // Open an output file
//    cerr << "Creating a test file.;
    
    int fd = open(filespec.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
    if (fd < 0) {
        perror("open");
    }

    // Create an array of printable ASCII characters
    int range = '~' - '!';
    char *buf = new char[range];
    for (int j=0; j<range; j++) {
        buf[j] = '!' + j;
    }

    // Write the false data to disk
    int total = 0;
    int ret = 0;
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
    cerr << "This program tests HTTP protocol support." << endl;
    cerr << "Usage: test_http [hv]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    exit (-1);
}

#else  // no DejaGnu support

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
  return 0;  
}

#endif
