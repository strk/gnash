// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

//#ifdef HAVE_DEJAGNU_H
#if 1
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
#include <string>
#include <iostream>
#include <string>

#include "log.h"
#include "dejagnu.h"
#include "rtmp.h"
#include "amf.h"
#include "check.h"

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

    //gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    //dbglogfile.setVerbosity(1);

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

//    amf_obj.parseBody(buf + amf_obj.getHeaderSize(), amf_obj.getTotalSize());

    // This extracts a "connect" message from the RTMP data stream. We
    // look for everything ourselves to be the most accurate.
    tmpptr = buf  + amf_obj.getHeaderSize();
    int8_t *str = amf_obj.extractString(tmpptr);
    if (strcmp(reinterpret_cast<const char *>(str), "connect") == 0) {
        runtest.pass("Extracted \"connect\" string");
    } else {
        runtest.fail("Extracted \"connect\" string");
    }
    
    tmpptr += strlen(reinterpret_cast<const char *>(str)) + AMF_HEADER_SIZE;    
    amfnum_t *num = amf_obj.extractNumber(tmpptr);
    char     *numptr = (char *)num;
    if ((numptr[6] == -16)
        && (numptr[7] == 0x3f)) {
        runtest.pass("Extracted \"connect\" number");
    } else {
        runtest.fail("Extracted \"connect\" number");
    }
    tmpptr += AMF_NUMBER_SIZE + 2;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "app") {
        runtest.pass("Extracted \"app\" variable");
    } else {
        runtest.fail("Extracted \"app\" variable");
    }
//    cerr << el.name << endl;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "flashVer") {
        runtest.pass("Extracted \"flashVer\" variable");
    } else {
        runtest.fail("Extracted \"flashVer\" variable");
    }
//    cerr << el.name << endl;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "swfUrl") {
        runtest.pass("Extracted \"swfUrl\" variable");
    } else {
        runtest.fail("Extracted \"swfUrl\" variable");
    }
//    cerr << el.name << endl;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "tcUrl") {
        runtest.pass("Extracted \"tcUrl\" variable");
    } else {
        runtest.fail("Extracted \"tcUrl\" variable");
    }
//    cerr << el.name << endl;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "fpad") {
        runtest.pass("Extracted \"fpad\" variable");
    } else {
        runtest.fail("Extracted \"fpad\" variable");
    }
//    cerr << el.name << endl;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "audioCodecs") {
        runtest.pass("Extracted \"audioCodecs\" variable");
    } else {
        runtest.fail("Extracted \"audioCodecs\" variable");
    }
//    cerr << el.name << endl;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "videoCodecs") {
        runtest.pass("Extracted \"videoCodecs\" variable");
    } else {
        runtest.fail("Extracted \"videoCodecs\" variable");
    }
//    cerr << el.name << endl;
    
    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "videoFunction") {
        runtest.pass("Extracted \"videoFunction\" variable");
    } else {
        runtest.fail("Extracted \"videoFunction\" variable");
    }
//    cerr << el.name << endl;

    tmpptr = amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "pageUrl") {
        runtest.pass("Extracted \"pageURL\" variable");
    } else {
        runtest.fail("Extracted \"pageURL\" variable");
    }
//    cerr << el.name << endl;
    
    amf_obj.extractVariable(&el, tmpptr);
    if (el.name == "objectEncoding") {
        runtest.pass("Extracted \"objectEncoding\" variable");
    } else {
        runtest.fail("Extracted \"objectEncoding\" variable");
    }
//    cerr << el.name << endl;
    
    // Now build our own connect message with the same data, which
    // should give us an exact copy.
    int amf_index = amf_obj.getAMFIndex();
    AMF::amf_headersize_e head_size = AMF::HEADER_12;
    int total_size = amf_obj.getTotalSize();
    AMF::content_types_e type = AMF::INVOKE;
    amfsource_e routing = amf_obj.getRouting();
    AMF rtmp;

    // First build and test the header. This uses the same data as the
    // previous one
    unsigned char *out = reinterpret_cast<unsigned char *>(rtmp.encodeRTMPHeader(amf_index, head_size, total_size, type, routing));
    tmpptr = out;
    rtmp.parseHeader(out);
    if (rtmp.getTotalSize() == 269) {
        runtest.pass("New Message Header Total Size");
    } else {
        runtest.fail("New Message Header Total Size");
    }
    
    if (rtmp.getHeaderSize() == 12) {
        runtest.pass("New Message Header Size");
    } else {
        runtest.fail("New Message Header Size");
    }
    
    if (rtmp.getMysteryWord() == 0) {
        runtest.pass("New Message Mystery Word");
    } else {
        runtest.fail("Message Mystery Word");
    }
    
    if (rtmp.getRouting() == CLIENT) {
        runtest.pass("New Message Routing");
    } else {
        runtest.fail("New Message Routing");
    }

    check_equals(rtmp.getHeaderSize(), 12);

    if (memcmp(out, buf, 12) == 0) {
        runtest.pass("RTMP Headers match");
    } else {
        size_t s = 12;
        runtest.fail("RTMP Headers mismatch");
        cerr << "buf is: 0x" << hexify(buf, s, true) << endl;
        cerr << "out is: 0x" << hexify(out, s, true) << endl;
    }

    tmpptr += rtmp.getHeaderSize();
    
    // Now build up a body of a connect message
    unsigned char *var;

    var = (unsigned char *)rtmp.encodeString("connect");
    int8_t *c_out = rtmp.extractString(var);
    if ( ! c_out )
    {
        runtest.fail("Encoded \"connect\" string could not be extracted");
    }
    else
    {
        std::string s_in("connect");
        std::string s_out(reinterpret_cast<const char *>(c_out));

        if (s_in == s_out) {
            runtest.pass("Encoded \"connect\" string");
        } else {
            runtest.fail("Encoded \"connect\" string");
            cerr << "Encoded 'connect' returned as as" << s_out << endl;
        }
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, strlen("connect") + 3);
    delete [] var;

    amfnum_t bignum = 0x3ff0000000000000LL;
    numptr = (char *)&bignum;
    var = (unsigned char *)rtmp.encodeNumber(bignum);
    if (*rtmp.extractNumber(var) == bignum) {
        runtest.pass("Encoded \"connect\" number");
    } else {
        runtest.fail("Encoded \"connect\" number");
    }

    tmpptr = rtmp.appendPtr(tmpptr, var, AMF_NUMBER_SIZE + 1);
    delete [] var;

    // Start the object
    *tmpptr++ = AMF::OBJECT;
    
    var = (unsigned char *)rtmp.encodeVariable("app", "oflaDemo");
    rtmp.extractVariable(&el, var);
    if ((el.name == "app") && (strncmp((char *)el.data, "oflaDemo", 8) == 0)) {
        runtest.pass("Encoded \"app\" variable");
    } else {
        runtest.fail("Encoded \"app\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.length + strlen("app") + 5);
    delete [] var;
    
    var = (unsigned char *)rtmp.encodeVariable("flashVer", "LNX 9,0,31,0");
    rtmp.extractVariable(&el, var);
    if ((el.name == "flashVer") && (strncmp((char *)el.data, "LNX 9,0,31,0", el.length) == 0)) {
        runtest.pass("Encoded \"flashVer\" variable");
    } else {
        runtest.fail("Encoded \"flashVer\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.length + strlen("flashVer") + 5);
    delete [] var;
    
    var = (unsigned char *)rtmp.encodeVariable("swfUrl", "http://www.red5.nl/tools/publisher/publisher.swf");
    rtmp.extractVariable(&el, var);
    if ((el.name == "swfUrl") && (strncmp((char *)el.data, "http://www.red5.nl/tools/publisher/publisher.swf", el.length) == 0)) {
        runtest.pass("Encoded \"swfUrl\" variable");
    } else {
        runtest.fail("Encoded \"swfUrl\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.length + strlen("swfUrl") + 5);
    delete [] var;
    
    var = (unsigned char *)rtmp.encodeVariable("tcUrl", "rtmp://localhost/oflaDemo");
    rtmp.extractVariable(&el, var);
    if ((el.name == "tcUrl") && (strncmp((char *)el.data, "rtmp://localhost/oflaDemo", 25) == 0)) {
        runtest.pass("Encoded \"tcUrl\" variable");
    } else {
        runtest.fail("Encoded \"tcUrl\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.length + strlen("tcUrl") + 5);
    delete [] var;

    var = (unsigned char *)rtmp.encodeVariable("fpad", false);
    rtmp.extractVariable(&el, var);
    if ((el.name == "fpad") && (*el.data == 0)) {
        runtest.pass("Encoded \"fpad\" Boolean variable");
    } else {
        runtest.fail("Encoded \"fpad\" Boolean variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, 1 + strlen("fpad") + 3);
    delete [] var;
    
    bignum = 0x388340LL;
    numptr = (char *)&bignum;
    var = (unsigned char *)rtmp.encodeVariable("audioCodecs", bignum);
    rtmp.extractVariable(&el, var);
    
    if ((el.type == amf::AMF::NUMBER)
        && (el.name == "audioCodecs")
        && (el.data[5] == 0x38)
        && (el.data[6] == 0x83)
        && (el.data[7] == 0x40)) {
        runtest.pass("Encoded \"audioCodecs\" variable");
    } else {
        runtest.fail("Encoded \"audioCodecs\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.name.size() + AMF_NUMBER_SIZE + 3);
    delete [] var;
    
    bignum = 0x5f40LL;
    numptr = (char *)&bignum;
    var = (unsigned char *)rtmp.encodeVariable("videoCodecs", bignum);
    rtmp.extractVariable(&el, var);
    
    if ((el.type == amf::AMF::NUMBER)
        && (el.name == "videoCodecs")
        && (el.data[6] == 0x5f)
        && (el.data[7] == 0x40)) {
        runtest.pass("Encoded \"videoCodecs\" variable");
    } else {
        runtest.fail("Encoded \"videoCodecs\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.name.size() + AMF_NUMBER_SIZE + 3);
    delete [] var;
    
    bignum = 0xf03fLL;
    numptr = (char *)&bignum;
    var = (unsigned char *)rtmp.encodeVariable("videoFunction", bignum);
    rtmp.extractVariable(&el, var);
    
    if ((el.type == amf::AMF::NUMBER)
        && (el.name == "videoFunction")
        && (el.data[6] == 0xf0)
        && (el.data[7] == 0x3f)) {
        runtest.pass("Encoded \"videoFunction\" variable");
    } else {
        runtest.fail("Encoded \"videoFunction\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.name.size() + AMF_NUMBER_SIZE + 3);
    delete [] var;
    
    var = (unsigned char *)rtmp.encodeVariable("pageUrl");
    rtmp.extractVariable(&el, var);
    if ((el.type == amf::AMF::UNDEFINED)
        && (el.name == "pageUrl")) {
        runtest.pass("Encoded \"pageUrl\" undefined variable");
    } else {
        runtest.fail("Encoded \"pageUrl\" undefined variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.name.size() + 3);
    delete [] var;
    
    bignum = 0x0;
    numptr = (char *)&bignum;
    var = (unsigned char *)rtmp.encodeVariable("objectEncoding", bignum);
    rtmp.extractVariable(&el, var);
    
    if ((el.type == amf::AMF::NUMBER)
        && (el.name == "objectEncoding")
        && (el.data[6] == 0x0)
        && (el.data[7] == 0x0)) {
        runtest.pass("Encoded \"objectEncoding\" variable");
    } else {
        runtest.fail("Encoded \"objectEncoding\" variable");
    }
    tmpptr = rtmp.appendPtr(tmpptr, var, el.name.size() + AMF_NUMBER_SIZE + 3);
    delete [] var;

    // Start the object
    *tmpptr++ = AMF::OBJECT_END;
    
    if (memcmp(buf, out, amf_obj.getTotalSize()) == 0) {
        runtest.pass("Object Packets match");
    } else {
        runtest.fail("Object Packets mismatch");
    }    

    size_t hexsize = std::max(AMF_PACKET_SIZE, amf_obj.getTotalSize())*2;
    cerr << "buf is: 0x" << hexify(buf, amf_obj.getTotalSize() + 10, true) << ", size is: " << amf_obj.getTotalSize() << endl;
    cerr << "out is: 0x" << hexify(out, rmtp.getTotalSize() + 10, true) << ", size is: " << rtmp.getTotalSize() << endl;
    
//    delete out;
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

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
  return 0;  
}
#endif
