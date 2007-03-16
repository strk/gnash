// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_DEJAGNU_H

#include <sys/types.h>
#include <unistd.h>
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

void test_Number(void);
void test_Boolean(void);
void test_String(void);

void test_Header(void);
void test_Body(void);
void test_Packet(void);

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
    bool dump = false;
    char buffer[300];
    int c, retries = 3;

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
    unsigned char *tmpptr;
    short length;
    AMF::amf_element_t el;

    // First see if we can read strings. This file is produced by
    // using a network packet sniffer, and should be binary correct.
    memset(buf, 0, AMF_PACKET_SIZE+1);
    string filespec = SRCDIR;
    filespec += "/connect-object.amf";
    
    fd = open(filespec.c_str(), O_RDONLY);
    ret = read(fd, buf, AMF_PACKET_SIZE);
    close(fd);

    amf_obj.parseHeader(buf);
    if (amf_obj.getTotalSize() == 269) {
        runtest.pass("Message Header Total Size");
    } else {
        runtest.fail("Message Header Total Size");
    }
    
    if (amf_obj.getHeaderSize() == 12) {
        runtest.pass("Message Header Size");
    } else {
        runtest.fail("Message Header Size");
    }
    
    if (amf_obj.getMysteryWord() == 0) {
        runtest.pass("Message Mystery Word");
    } else {
        runtest.fail("Message Mystery Word");
    }
    
    if (amf_obj.getRouting() == 0) {
        runtest.pass("Message Routing");
    } else {
        runtest.fail("Message Routing");
    }

    amf_obj.parseBody(buf + amf_obj.getHeaderSize(), amf_obj.getTotalSize());

    tmpptr = buf;

    
//    tmpptr = amf_obj.extractVariables(el, tmpptr);
    
//     char *str = amf_obj.extractObject(buf);
//     if (strcmp(str, "connect") == 0) {
//         runtest.pass("Extracted \"connect\" string");
//     } else {
//         runtest.fail("Extracted \"connect\" string");
//     }

}

static void
usage (void)
{
    cerr << "This program tests Shared Object support in the AMF library." << endl;
    cerr << "Usage: test_object [hv]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    exit (-1);
}

#endif
