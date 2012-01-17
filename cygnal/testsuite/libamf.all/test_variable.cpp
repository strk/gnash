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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <string>
#include <iostream>
#include <string>

#include "dejagnu.h"
#include "rtmp.h"
#include "amf.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

static int verbosity;

TestState runtest;

// These are used to print more intelligent debug messages
const char *astype_str[] = {
    "Number",
    "Boolean",
    "String",
    "Object",
    "MovieClip",
    "Null",
    "Undefined",
    "Reference",
    "ECMAArray",
    "ObjectEnd",
    "StrictArray",
    "Date",
    "LongString",
    "Unsupported",
    "Recordset",
    "XMLObject",
    "TypedObject"
};

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
            verbosity++;
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
    unsigned char buf[AMF_PACKET_SIZE+1];
    AMF::amf_element_t el;

    // First see if we can read variables. This file is produced by
    // using a network packet sniffer, and should be binary correct.
    memset(buf, 0, AMF_PACKET_SIZE+1);
    string filespec = SRCDIR;
    filespec += "/string-variable.amf";
    fd = open(filespec.c_str(), O_RDONLY);
    if (fd <= 0) {
        cerr << "Couldn't open the binary test file!" << endl;
        exit(1);
    }
    
    ret = read(fd, buf, AMF_PACKET_SIZE);
    close(fd);

    if (amf_obj.extractVariable(&el, buf)) {
        runtest.pass("Got String element");
    } else {
        runtest.fail("Got String element");
    }  

    if ((el.type == amf::AMF::STRING) && (el.length == 25) && (el.name == "tcUrl")) {
        runtest.pass("Got String element data");
    } else {
        runtest.fail("Got String element data");
    }  

    char *out = (char*)amf_obj.encodeVariable("tcUrl", "rtmp://localhost/oflaDemo");
    if ((out[1] == 0x5)
        && (out[2] == 't')
        && (out[3] == 'c')
        && (out[4] == 'U')
        && (out[5] == 'r')
        && (out[6] == 'l')) {
        runtest.pass("String Variable name correct");
    } else {
        runtest.fail("String Variable name correct");
    }    

    if ((out[7] == AMF::STRING)
        && (out[8] == 0x0)
        && (out[9] == 25)
        && (out[10] == 'r')
        && (out[11] == 't')
        && (out[12] == 'm')
        && (out[13] == 'p')
        && (out[14] == ':')) {
        runtest.pass("Variable String data correct");
    } else {
        runtest.fail("Variable String data correct");
    }

    if (memcmp(buf, out, 0x15) == 0) {
        runtest.pass("String Packets match");
    } else {
        runtest.fail("String Packets match");
    }
    
    delete [] out;
    
    // Test number fields
    memset(buf, 0, AMF_PACKET_SIZE+1);
    filespec = SRCDIR;
    filespec += "/number-variable.amf";
    fd = open(filespec.c_str(), O_RDONLY);
    ret = read(fd, buf, AMF_PACKET_SIZE);
    close(fd);

    if (amf_obj.extractVariable(&el, buf)) {
        runtest.pass("Got Number element");
    } else {
        runtest.fail("Got Number element");
    }  

    if ((el.type == amf::AMF::NUMBER)
        && (el.name == "audioCodecs")
        && (el.data[5] == 0x38)
        && (el.data[6] == 0x83)
        && (el.data[7] == 0x40)) {
        runtest.pass("Got Number element data");
    } else {
        runtest.fail("Got Number element data");
    }
    
    amfnum_t bignum = 0x388340L;
    out = (char*)amf_obj.encodeVariable("audioCodecs", bignum);
    if ((out[1] == 11)
        && (out[2] == 'a')
        && (out[3] == 'u')
        && (out[4] == 'd')
        && (out[5] == 'i')
        && (out[6] == 'o')) {
        runtest.pass("Number Variable name correct");
    } else {
        runtest.fail("Number Variable name correct");
    }
    
    if ((out[13] == AMF::NUMBER)
        && (out[14] == 0x40)
        && (out[15] == -125)
        && (out[16] == 0x38)) {
        runtest.pass("Variable Number data correct");
    } else {
        runtest.fail("Variable Number data correct");
    }

    if (memcmp(buf, out, 0x15) == 0) {
        runtest.pass("Number Packets match");
    } else {
        runtest.fail("Number Packets match");
    }

    delete [] out;
}

static void
usage (void)
{
    cerr << "This program tests AMF variables in the AMF library." << endl;
    cerr << "Usage: test_variable [hv]" << endl;
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
