// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include <cstdlib>
#include <sys/types.h>
extern "C"{
        #include "GnashSystemIOHeaders.h"
#ifdef HAVE_GETOPT_H
        #include <getopt.h>
#endif
#ifndef __GNUC__
        extern int optind, getopt(int, char *const *, const char *);
#endif
}
#include <sys/stat.h>
#include <fcntl.h>

#include "dejagnu.h"
#include "log.h"
#include "amf.h"

using namespace amf;
using namespace std;
using namespace gnash;

static void usage (void);

static int verbosity;

static TestState runtest;

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    char buffer[300];
    int c;
    
    memset(buffer, 0, 300);
    
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
    
    // get the file name from the command line
    if (optind < argc) {
        string filespec = argv[optind];
        cout << "Will use \"" << filespec << "\" for test " << endl;
    }

    AMF amf_obj;
    int fd, ret;
    uint8_t buf[AMF_VIDEO_PACKET_SIZE+1];

    // First see if we can read strings. This file is produced by
    // using a network packet sniffer, and should be binary correct.
    memset(buf, 0, AMF_VIDEO_PACKET_SIZE+1);
    string filespec = SRCDIR;
    filespec += "/connect-string.amf";
    fd = open(filespec.c_str(), O_RDONLY);
    ret = read(fd, buf, AMF_VIDEO_PACKET_SIZE);
    close(fd);

    char *connect = "connect";
    int8_t *str = amf_obj.extractString(buf);
    if (memcmp(str, connect, strlen(connect)) == 0) {
        runtest.pass("Extracted \"connect\" string");
    } else {
        runtest.fail("Extracted \"connect\" string");
    }

    // Now make sure we can also create strings. We'll create the same
    // string we just read, and make sure they match.
    void *out = amf_obj.encodeElement(AMF::STRING, connect, strlen(connect));
    if (memcmp(out, buf, 10) == 0) {
        runtest.pass("Encoded \"connect\" string");
    } else {
        runtest.fail("Encoded \"connect\" string");
    }

    delete [] str;
}

static void
usage (void)
{
    cerr << "This program tests string support in the AMF library." << endl;
    cerr << "Usage: test_string [hv]" << endl;
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
