// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netinet/in.h>
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

#ifdef HAVE_LIBXML
extern int xml_fd;		// FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.
#endif // HAVE_LIBXML

#include "amf.h"

using namespace amf;
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
    int c;
    bool dump = false;
    string filespec;
    string procname, memname;
    char buffer[300];
    int retries = 3;

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
        filespec = argv[optind];
        cout << "Will use \"" << filespec << "\" for test " << endl;
    }

    test_Number();
    test_Boolean();
//    test_String();

    test_Header();
    test_Body();
    test_Packet();
}

void
test_Number(void)
{
    AMF amf_obj;
    amfnum_t num;
    void *out;
    num = 123456789;

    // Write a number element
    note("Test a Number element");
    out = amf_obj.encodeElement(AMF::Number, &num, 0);
    if (amf_obj.extractElementHeader(out) == AMF::Number) {
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
}

void
test_Boolean(void)
{
    AMF amf_obj;
    bool bo = false;
    void *out;

    // Write a number element
    note("Test a Boolean element");
    out = amf_obj.encodeElement(AMF::Boolean, &bo, 0);
    if (amf_obj.extractElementHeader(out) == AMF::Boolean) {
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
    out = amf_obj.encodeElement(AMF::Boolean, &bo, 0);
    if (*((char *)out + 1) == 1) {
        runtest.pass("Boolean true returned correct");
    } else {
        runtest.fail("Boolean true returned not correct");
    }
}

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
    
    element = amf_obj.encodeElement(AMF::Number, &num, 0);
    head = amf_obj.encodeHeader(&name, true, sizeof(amfnum_t), &num);

    char *ptr = ((char *)head) + 2;
    if ((strncmp(ptr, test, name.length) == 0) && (ntohs(*(short *)head) == name.length)) {
        runtest.pass("Header name correct");
    } else {
        runtest.fail("Header name not correct");
    }

    ptr = ((char *)head) + 2 + name.length + 1;
    if (*ptr == AMF::Number) {
        runtest.pass("Header Object type correct");
    } else {
        runtest.fail("Header Object type not correct");
    }
    
    ptr += 1;
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

static void
usage (void)
{
    cerr << "This program tests the AMF library." << endl;
    cerr << "Usage: amftest [hv]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    exit (-1);
}
