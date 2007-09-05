// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#ifdef HAVE_DEJAGNU_H

//#include <netinet/in.h>
#include <string>
#include <sys/types.h>
extern "C"{
        #include <unistd.h>
#ifdef HAVE_GETOPT_H
        #include <getopt.h>
#endif
#ifndef __GNUC__
        extern int optind, getopt(int, char *const *, const char *);
#endif
}
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <iostream>
#include <string>

#include "dejagnu.h"


#include "amf.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

static int verbosity;

static TestState runtest;

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
            verbosity++;
            break;
            
          default:
            usage ();
            break;
        }
    }
    
    AMF amf_obj;
    int fd, ret;
    char buf[AMF_NUMBER_SIZE+1];
    amfnum_t *num;
    
    memset(buf, 0, AMF_NUMBER_SIZE+1);
    string filespec = SRCDIR;
    filespec += "/f03f.amf";
    fd = open(filespec.c_str(), O_RDONLY);
    ret = read(fd, buf, AMF_NUMBER_SIZE);
    close(fd);

    num = amf_obj.extractNumber(buf);

    if ((((char *)num)[6] == -16) && (((char *)num)[7] == 0x3f)) {
        runtest.pass("Extracted Number AMF object");
    } else {
        runtest.fail("Extracted Number AMF object");
    }

    void *out = amf_obj.encodeNumber(*num);

    if (memcmp(out, buf, 9) == 0) {
        runtest.pass("Encoded AMF Number");
    } else {
        runtest.fail("Encoded AMF Number");
    }

    delete num;
}
static void
usage (void)
{
    cerr << "This program tests Number support in the AMF library." << endl;
    cerr << "Usage: test_number [hv]" << endl;
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
