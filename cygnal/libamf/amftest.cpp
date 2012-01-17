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

#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include "GnashSystemIOHeaders.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <iostream>
#include <string>

#include "dejagnu.h"


bool gofast = false;		// FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,
bool nodelay = false;           // FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,

#include "amf.h"

using namespace cygnal;
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

    test_Number();
    test_Boolean();
    test_String();

//     test_Header();
//     test_Body();
//     test_Packet();
}

void
test_Number(void)
{
    AMF amf_obj;
#if 0
    amfnum_t num = 123456789;

    // Write a number element
    note("Test a Number element");
    void *out = amf_obj.encodeElement(AMF::NUMBER, &num, 0);
    if (amf_obj.extractElementHeader(out) == AMF::NUMBER) {
        runtest.pass("Number header correct");
    } else {
        runtest.fail("Number header not correct");
    }
    if (amf_obj.extractElementLength(out) == 8) {
        runtest.pass("Number length returned correct");
    } else {
        runtest.fail("Number length returned not correct");
    }
    
    char *numptr = (char *)&num;
    char *outptr = (char *)out+1;
    if ((numptr[0] == outptr[7]) && (numptr[1] == outptr[6])
        && (numptr[2] == outptr[5]) && (numptr[3] == outptr[4])
        && (numptr[4] == outptr[3]) && (numptr[5] == outptr[2])
        && (numptr[6] == outptr[1]) && (numptr[7] == outptr[0])
        ) {
        pass("Number swapped correct");
    } else {
        fail("Number swapped not correct");
    }
#endif
    int fd, ret;
    char buf[AMF_NUMBER_SIZE+1];
    amfnum_t value = 0xf03fL;
    amfnum_t *num;
    
    memset(buf, 0, AMF_NUMBER_SIZE+1);
    fd = open("number.amf", O_RDONLY);
    ret = read(fd, buf, 12);
    close(fd);

    num = amf_obj.extractNumber(buf);

//     unsigned char hexint[32];
//     hexify((unsigned char *)hexint, (unsigned char *)num, 8, false);
//     cerr << "AMF number is: 0x" << hexint << endl;
//     hexify((unsigned char *)hexint, (unsigned char *)&value, 8, false);
//     cerr << "AMF value is: 0x" << hexint << endl;

    if (((char *)num)[7] == 0x3f) {
//    if (memcmp(num, &value, AMF_NUMBER_SIZE) == 0) {
        runtest.pass("Extracted Number AMF object");
    } else {
        runtest.fail("Extracted Number AMF object");
    }

    void *out = amf_obj.encodeNumber(*num);
//     hexify((unsigned char *)hexint, (unsigned char *)out, 9, false);
//     cerr << "AMF encoded number is: 0x" << hexint << endl;

//     hexify((unsigned char *)hexint, (unsigned char *)buf, 9, false);
//     cerr << "AMF buff number is: 0x" << hexint << endl;

    if (memcmp(out, buf, 9) == 0) {
        runtest.pass("Encoded AMF Number");
    } else {
        runtest.fail("Encoded AMF Number");
    }

    delete num;
}

void
test_Boolean(void)
{
    AMF amf_obj;
    bool bo = false;

    // Write a number element
    void *out = amf_obj.encodeElement(AMF::BOOLEAN, &bo, 0);
    if (amf_obj.extractElementHeader(out) == AMF::BOOLEAN) {
        runtest.pass("Boolean header correct");
    } else {
        runtest.fail("Boolean header not correct");
    }
    if (amf_obj.extractElementLength(out) == 1) {
        runtest.pass("Boolean length returned correct");
    } else {
        runtest.fail("Boolean length returned not correct");
    }
    if (*((char *)out + 1) == 0) {
        pass("Boolean false returned correct");
    } else {
        runtest.fail("Boolean false returned not correct");
    }
    bo = true;
    out = amf_obj.encodeElement(AMF::BOOLEAN, &bo, 0);
    if (*((char *)out + 1) == 1) {
        runtest.pass("Boolean true returned correct");
    } else {
        runtest.fail("Boolean true returned not correct");
    }
}

// Make sure we can read and write binary AMF strings
void
test_String(void)
{
    AMF amf_obj;
    int fd, ret;
    char buf[AMF_VIDEO_PACKET_SIZE+1];

    // First see if we can read strings. This file is produced by
    // using a network packet sniffer, and should be binary correct.
    memset(buf, 0, AMF_VIDEO_PACKET_SIZE+1);
    fd = open("string1.amf", O_RDONLY);
    ret = read(fd, buf, AMF_VIDEO_PACKET_SIZE);
    close(fd);

    char *str = amf_obj.extractString(buf);
    if (strcmp(str, "connect") == 0) {
        runtest.pass("Extracted \"connect\" string");
    } else {
        runtest.fail("Extracted \"connect\" string");
    }

    // Now make sure we can also create strings. We'll create the same
    // string we just read, and make sure they match.
    char *connect = "connect";
    void *out = amf_obj.encodeElement(AMF::STRING, connect, strlen(connect));
    if (memcmp(out, buf, 10) == 0) {
        runtest.pass("Encoded \"connect\" string");
    } else {
        runtest.fail("Encoded \"connect\" string");
    }

    delete str;
}

#if 0
// Each header consists of the following:
//
// * UTF string (including length bytes) - name
// * Boolean - specifies if understanding the header is `required'
// * Long - Length in bytes of header
// * Variable - Actual data (including a type code)
void
test_Header(void){
    AMF amf_obj;
    amfutf8_t name, headname;
    amfnum_t num;
    void *element;
    amfhead_t *head;

    note("Test the Header");

    num = 123456789;

    char *test = "NumberTest";
    name.length = strlen(test);
    name.data = test;
    
    element = amf_obj.encodeElement(AMF::NUMBER, &num, 0);
    head = amf_obj.encodeHeader(&name, true, sizeof(amfnum_t), &num);

    char *ptr = ((char *)head) + 2;
    if ((strncmp(ptr, test, name.length) == 0) && (ntohs(*(short *)head) == name.length)) {
        runtest.pass("Header name correct");
    } else {
        runtest.fail("Header name not correct");
    }

    ptr = ((char *)head) + 2 + name.length + 1;
    if (*ptr == AMF::NUMBER) {
        runtest.pass("Header Object type correct");
    } else {
        runtest.fail("Header Object type not correct");
    }
    
    ptr++;
    if (*ptr == htonl(num)) {
        runtest.pass("Header Object data correct");
    } else {
        runtest.fail("Header Object data not correct");
    }    
}

void
test_Body(void)
{
    AMF amf_obj;
    void *out;

    // Write a number element
    note("Test the Body");
}

void
test_Packet(void)
{
    AMF amf_obj;
    void *out;

    // Write a number element
    note("Test the Packet");
}
#endif

static void
usage (void)
{
    cerr << "This program tests the AMF library." << endl;
    cerr << "Usage: amftest [hv]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    exit (-1);
}

#endif
