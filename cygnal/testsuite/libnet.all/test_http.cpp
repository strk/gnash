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

#include <string>

#include "GnashSystemIOHeaders.h"
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
#include <ctime>            // std::time

#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "log.h"
#include "http.h"
#include "dejagnu.h"
#include "network.h"
#include "amf.h"
#include "buffer.h"
#include "GnashNumeric.h"

using namespace gnash;
using namespace amf;
using namespace std;

static void usage (void);
static void tests (void);
static void test_post (void);
// static void test_rtmpt (void);

static TestState runtest;

LogFile& dbglogfile = LogFile::getDefaultInstance();

// Uncommenting this enables test cases that attempt to handle
// corrupted packets by randomly trashing data. These are not
// enabled by default, as they aren't valgrind clean by definition
// These tests primary function is to make sure HTTP, RTMPT, and
// AMF parsing is as stable as possible, and can handle some of the
// without crashing.
//#define CORRUPT_MEMORY_TESTS 1

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

    tests();
    test_post();
//    test_rtmpt();
}


void
tests()
{    
    HTTP http;

    http.clearHeader();
    http.formatDate();
//    cerr << "FIXME: " << http.getHeader() << endl;

//     amf::Buffer &buf = http.getBuffer();
//     cerr << "STREAM: " << http.getHeader() << endl;
//     char *ptr = (char *)buf.reference();
//     cerr << "BUFFER: " << buf.reference() << endl;

    regex_t regex_pat;

    // Check the Date field
    // The date should look something like this:
    //     Date: Mon, 10 Dec 2007  GMT
    //     Date: Tue, 1 Apr 2008 19:52:16 GMT\r\n
    regcomp (&regex_pat, "Date: [A-Z][a-z]*, [0-9]* [A-Z][a-z]* [0-9]* [0-9:]* *GMT.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatDate()");
        cerr << http.getHeader() << endl;
    } else {
        runtest.pass ("HTTP::formatDate()");
    }
    regfree(&regex_pat);

    // Check the Content-Length field
    // Content-Length: 12345\r\n"
    http.clearHeader();
    http.formatContentLength(12345);

//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Content-Length: [0-9]*.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatContentLength()");
    } else {
        runtest.pass ("HTTP::formatContentLength()");
    }
    regfree(&regex_pat);


    // Check the Connection field
//     bool formatConnection(const char *data);
    http.clearHeader();
    const char *data = "Keep-Alive";
    http.formatConnection(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Connection: [A-za-z-]*",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatConnection()");
    } else {
        runtest.pass ("HTTP::formatConnection()");
    }
    regfree(&regex_pat);

    // Check the Server field
    http.clearHeader();
    http.formatServer();
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Server: Cygnal (GNU/Linux).*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatServer()");
    } else {
        runtest.pass ("HTTP::formatServer()");
    }
    regfree(&regex_pat);

    // Check the Host field
//     bool formatHost(const char *data);
    http.clearHeader();
    data = "localhost:4080";
    http.formatHost(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Host: [A-za-z-]*:[0-9]*.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatHost()");
    } else {
        runtest.pass ("HTTP::formatHost()");
    }
    regfree(&regex_pat);

// Check the Language field
//     bool formatLanguage(const char *data);
    http.clearHeader();
    data = "en-US,en;q=0.9";
    http.formatLanguage(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Accept-Language: en-US,en;q=0.9.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatLanguage()");
    } else {
        runtest.pass ("HTTP::formatLanguage()");
    }
    regfree(&regex_pat);

//     bool formatCharset(const char *data);
// Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1\r
    http.clearHeader();
    data = "iso-8859-1, utf-8, utf-16, *;q=0.1";
    http.formatCharset(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Accept-Charset: iso-8859-1.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatCharset()");
    } else {
        runtest.pass ("HTTP::formatCharset()");
    }
    regfree(&regex_pat);
        
//     bool formatEncoding(const char *data);
    http.clearHeader();
    data = "deflate, gzip, x-gzip, identity, *;q=0";
    http.formatEncoding(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Accept-Encoding: deflate, gzip.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatEncoding()");
    } else {
        runtest.pass ("HTTP::formatEncoding()");
    }
    regfree(&regex_pat);
        
        
//     bool formatTE(const char *data);
    http.clearHeader();
    data = "deflate, gzip, chunked, identity, trailers";
    http.formatTE(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "TE: deflate, gzip,.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatTE()");
    } else {
        runtest.pass ("HTTP::formatTE()");
    }
    regfree(&regex_pat);

//     bool formatAgent(const char *data);
    http.clearHeader();
    data = "Gnash 0.8.1-cvs (X11; Linux i686; U; en)";
    http.formatAgent(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "User-Agent: Gnash 0.8.1-cvs.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatAgent()");
    } else {
        runtest.pass ("HTTP::formatAgent()");
    }
    regfree(&regex_pat);

    // Check the Content Type field. First we check with a
    // specified field, then next to see if the default works.
//     bool formatContentType();
    http.clearHeader();
    http.formatContentType(DiskStream::FILETYPE_SWF);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Content-Type: application/x-shockwave-flash.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatContentType(type)");
    } else {
        runtest.pass ("HTTP::formatContentType(type)");
    }
    regfree(&regex_pat);

    http.clearHeader();
    http.formatContentType();
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Content-Type: text/html.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatContentType()");
    } else {
        runtest.pass ("HTTP::formatContenType()");
    }
    regfree(&regex_pat);

//     bool formatReferer(const char *data);
    http.clearHeader();
    data = "http://localhost/software/gnash/tests/index.html";
    http.formatReferer(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Referer: http://localhost.*index.html.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatReferer()");
    } else {
        runtest.pass ("HTTP::formatReferer()");
    }
    regfree(&regex_pat);

    // Check formatHeader()
    // HTTP/1.1 200 OK\r\nDate: Tue, 1 Apr 2008 19:58:40 GMT\r\nServer: Cygnal (GNU/Linux)\r\nLast-Modified: Tue, 1 Apr 2008 19:58:40 GMT\r\nEtag: 24103b9-1c54-ec8632c0\r\nAccept-Ranges: bytes\r\nContent-Length: 0\r\nKeep
    http.clearHeader();
    http.formatHeader(HTTP::OK);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "HTTP/[0-9].[0-9] 200 OK.*Date:.*Server:.*:.*-Length.*-Type:.*$",
             REG_NOSUB);        // note that we do want to look for NL
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatHeader(port)");
    } else {
        runtest.pass ("HTTP::formatheader(port)");
    }
    regfree(&regex_pat);

#if 0
    // FIXME: should be moved to server side only test case
    // Check the Server field
    http.clearHeader();
    HTTPServer https;
    https.formatErrorResponse(HTTP::NOT_FOUND);
//    cerr << "FIXME: " << http.getHeader() << endl;
//    cerr << "FIXME: " << http.getBody() << endl;
    regcomp (&regex_pat, "Date:.*Server:.*Content-Length:.*Connection:.*Content-Type:.*$",
             REG_NOSUB);        // note that we do want to look for NL
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatErrorResponse(header)");
    } else {
        runtest.pass ("HTTP::formatErrorResponse(header)");
    }
    regfree(&regex_pat);
#endif
    
# if 0
    regfree(&regex_pat);
    regcomp (&regex_pat, "DOCTYPE.*<title>404 Not Found</title>.*$",
             REG_NOSUB);        // note that we do want to look for NL
    if (regexec (&regex_pat, http.getBody().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatErrorResponse(body)");
    } else {
        runtest.pass ("HTTP::formatErrorResponse(body)");
    }
    regfree(&regex_pat);
#endif
    
    //
    // Decoding tests for HTTP
    //
    http.clearHeader();
#if 0
    boost::uint8_t *buffer = (boost::uint8_t *)"GET /software/gnash/tests/flvplayer.swf?file=http://localhost/software/gnash/tests/Ouray_Ice_Festival_Climbing_Competition.flv HTTP/1.1\r\n"
"User-Agent: Gnash/0.8.1-cvs (X11; Linux i686; U; en)\r\n"
"Host: localhost:4080\r\n"
"Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1\r\n"
"Accept-Language: en-US,en;q=0.9\r\n"
"Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1\r\n"
"Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0\r\n"
"If-Modified-Since: Mon, 10 Dec 2007 02:26:31 GMT\r\n"
"If-None-Match: \"4cc434-e266-52ff63c0\"\r\n"
"Connection: Keep-Alive, TE\r\n"
"Referer: http://localhost/software/gnash/tests/index.html\r\n"
"TE: deflate, gzip, chunked, identity, trailers\r\n"
"\r\n";
#endif
    
// GET /software/gnash/tests/ HTTP/1.1
// Host: localhost:4080
// User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.5) Gecko/20070718 Fedora/2.0.0.5-1.fc7 Firefox/2.0.0.5
// Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5
// Accept-Language: en-us,en;q=0.5
// Accept-Encoding: gzip,deflate
// Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
// Keep-Alive: 300
// Connection: keep-alive

// User Agent: Lynx/2.8.6rel.2 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/0.9.8b

    
#if 0
    // FIXME: should be moved to server side only test case
    // Check the Server field
    Buffer field1;
    field1 = "GET /index.html HTTP/1.1";
    //    boost::uint8_t *field1 = (boost::uint8_t *)"GET /index.html HTTP/1.1";
    HTTP http1;
    //    http1.extractCommand(field1);
    http1.extractCommand(field1);
    if ((http1.getVersion()->minor == 1) && (http1.getFilespec() == "/index.html")) {
        runtest.pass ("HTTP::extractCommand(HTTP/1.1)");
    } else {
        runtest.fail ("HTTP::extractCommand(HTTP/1.1)");
    }

    Buffer field2;
    field2 = "GET /index.html HTTP/1.0";
    HTTP http2;
    http2.extractCommand(field2);
    if ((http2.getVersion()->minor == 0) && (http2.getFilespec() == "/index.html")) {
        runtest.pass ("HTTP::extractCommand(HTTP/1.0)");
    } else {
        runtest.fail ("HTTP::extractCommand(HTTP/1.0)");
    }

    Buffer field3;
    field3 = "GET /software/gnash/tests/flvplayer.swf?file=http://localhost/software/gnash/tests/Ouray_Ice_Festival_Climbing_Competition.flv HTTP/1.1\r\n";
    HTTP http3;
    http3.extractCommand(field3);
    if (http3.getFilespec() == "/software/gnash/tests/flvplayer.swf") {
        runtest.pass ("HTTP::extractCommand(filespec)");
    } else {
        runtest.fail ("HTTP::extractCommand(params)");
    }
    if (http3.getParams() == "file=http://localhost/software/gnash/tests/Ouray_Ice_Festival_Climbing_Competition.flv") {
        runtest.pass ("HTTP::extractCommand(params)");
    } else {
        runtest.fail ("HTTP::extractCommand(params)");
    }
#endif
    
#if 0
    boost::uint8_t *field3 = (boost::uint8_t *) "Keep-Alive: 300";
    HTTP http3;
    http3.extractKeepAlive(field3);
    if ((http3.keepAlive() == true) && (http3.getMaxRequests() == 300)) {
        runtest.pass ("HTTP::extractKeepAlive(300)");
    } else {
        runtest.fail ("HTTP::extractKeepAlive(300)");
    }
    
    boost::uint8_t *field4 = (boost::uint8_t *) "Keep-Alive: On";
    HTTP http4;
    http4.extractKeepAlive(field4);
    if (http4.keepAlive() == true) {
        runtest.pass ("HTTP::extractKeepAlive(On)");
    } else {
        runtest.fail ("HTTP::extractKeepAlive(On)");
    }
    
    boost::uint8_t *field5 = (boost::uint8_t *) "Keep-Alive: Off";
    HTTP http5;
    http5.extractKeepAlive(field5);
    if (http5.keepAlive() == false) {
        runtest.pass ("HTTP::extractKeepAlive(Off)");
    } else {
        runtest.fail ("HTTP::extractKeepAlive(Off)");
    }

// Some browsers have a different synatax, of course, to keep things
// interesting.
    boost::uint8_t *buffer2 = (boost::uint8_t *)"GET /software/gnash/tests/flvplayer.swf?file=http://localhost/software/gnash/tests/Ouray_Ice_Festival_Climbing_Competition.flv HTTP/1.1\r\n)"
"Content-Language: en-US,en;q=0.9\r\n"
"Content-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1\r\n"
"Content-Encoding: deflate, gzip, x-gzip, identity, *;q=0\r\n";
//    http.extractCommand(buffer);
    string result;
    result = http.extractReferer(buffer);
    if (result == "http://localhost/software/gnash/tests/index.html") {
        runtest.fail ("HTTP::extractReferer()");
    } else {
        runtest.pass ("HTTP::extractReferer()");
    }
    result = http.extractHost(buffer);
    if (result == "localhost:4080") {
        runtest.fail ("HTTP::extractHost()");
    } else {
        runtest.pass ("HTTP::extractHost()");
    }

    result = http.extractAgent(buffer);
    if (result == "Gnash/0.8.1-cvs (X11; Linux i686; U; en)") {
        runtest.fail ("HTTP::extractAgent()");
    } else {
        runtest.pass ("HTTP::extractAgent()");
    }

    int count;
    count = http.extractLanguage(buffer);
    std::vector<std::string> language = http.getLanguage();
    if ((count > 2) &&
        (language[0] == "en-US") &&
        (language[1] == "en")) {
        runtest.fail ("HTTP::extractLanguage(Accept-)");
    } else {
        runtest.pass ("HTTP::extractLanguage(Accept-)");
    }
    count = http.extractLanguage(buffer2);
    language = http.getLanguage();

    if ((count == 2) &&
        (language[0] == "en-US") &&
        (language[1] == "en")) {
        runtest.fail ("HTTP::extractLanguage(Content-)");
    } else {
        runtest.pass ("HTTP::extractLanguage(Content-)");
    }

    count = http.extractCharset(buffer);
    std::vector<std::string> charsets = http.getCharset();
    
    if ((count == 3) &&
        (charsets[0] == "iso-8859-1") &&
        (charsets[1] == "utf-8") &&
        (charsets[2] == "utf-16")) {
        runtest.fail ("HTTP::extractCharset(Accept-)");
    } else {
        runtest.pass ("HTTP::extractCharset(Accept-)");
    }
    count = http.extractCharset(buffer2);
    charsets = http.getCharset();
    if ((count == 3) &&
        (charsets[0] == "iso-8859-1") &&
        (charsets[1] == "utf-8") &&
        (charsets[2] == "utf-16")) {
        runtest.fail ("HTTP::extractCharset(Content-)");
    } else {
        runtest.pass ("HTTP::extractCharset(Content-)");
    }

    count = http.extractConnection(buffer);
    std::vector<std::string> connections = http.getConnection();
    if ((count == 2) &&
        (connections[0] == "Keep-Alive") &&
        (connections[1] == "TE")) {
        runtest.pass ("HTTP::extractConnection()");
    } else {
        runtest.fail ("HTTP::extractConnection()");
    }

    count = http.extractEncoding(buffer);
    std::vector<std::string> encoding = http.getEncoding();
    if ((count == 4) &&
        (encoding[0] == "deflate") &&
        (encoding[1] == "gzip") &&
        (encoding[2] == "chunked") &&
        (encoding[3] == "identity")) {
        runtest.fail ("HTTP::extractEncoding(Accept-)");
    } else{
        runtest.pass ("HTTP::extractEncoding(Accept-)");
    }

    count = http.extractTE(buffer);
    std::vector<std::string> te = http.getTE();
    if ((count == 5) &&
        (te[0] == "deflate") &&
        (te[1] == "gzip") &&
        (te[2] == "chunked") &&
        (te[3] == "identity") &&
        (te[4] == "trailers")) {
        runtest.pass ("HTTP::extractTE()");
    } else {
        runtest.fail ("HTTP::extractTE()");
    }
#endif
    
//     http.formatHeader(666, RTMP);
//     http.formatRequest("http://localhost:4080", HTTP::GET);
    
//     bool formatMethod(const char *data);
        
        
//     void *out = amf_obj.encodeNumber(*num);

//     if (memcmp(out, buf, 9) == 0) {
//         runtest.pass("Encoded AMF Number");
//     } else {
//         runtest.fail("Encoded AMF Number");
//     }

//    delete num;

    
    if (dbglogfile.getVerbosity() > 0) {
        http.dump();
    }
}

void
test_post()
{

    HTTP http;

    boost::shared_ptr<amf::Buffer> encstr = AMF::encodeString("Hello World!");
    boost::shared_ptr<amf::Buffer> encnum = AMF::encodeNumber(1.2345);

    amf::Buffer ptr1;
    ptr1 = "POST /echo/gateway HTTP/1.1\r\n";
    ptr1 += "User-Agent: Opera/9.62 (X11; Linux i686; U; en) Presto/2.1.1\r\n";
    ptr1 += "Host: localhost:4080\r\n";
    ptr1 += "Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap;q=0.1\r\n";
    ptr1 += "Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0\r\n";
    ptr1 += "Referer: http://localhost:5080/demos/echo_test.swf\r\n";
    ptr1 += "Connection: Keep-Alive, TE\r\n";
    ptr1 += "Content-Length: 15\r\n";
    ptr1 += "Content-Type: application/x-amf\r\n";
    ptr1 += "\r\n";
    ptr1 += *encstr;
    ptr1.resize();              // shrink the buffer to be the exact size of the data

#if 1
    // FIXME: should be moved to server side only test case
    // Check the Server field
    AMF amf;
    boost::uint8_t *data1 = http.processHeaderFields(&ptr1);
    boost::shared_ptr<amf::Element> el1 = amf.extractAMF(data1, data1 + 15);
    string str1 = el1->to_string();

    if ((http.getField("host") == "localhost:4080")
        && (str1 == "Hello World!")
        && (http.getField("content-length") == "15")) {
        runtest.pass("HTTP::processHeaderFields(POST) + STRING");
    } else {
        runtest.fail("HTTP::processHeaderFields(POST) + STRING");
    }

    amf::Buffer ptr2;
    ptr2 += "POST /echo/gateway HTTP/1.1\r\n";
    ptr2 += "User-Agent: Opera/9.62.(X11;.Linux.i686;.U;.en) Presto/2.1.1\r\n";
    ptr2 += "Host: localhost:5080\r\n";
    ptr2 += "Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap\r\n";
    ptr2 += "Keep-Alive: 300\r\n";
    ptr2 += "Accept-Language: en\r\n";
    ptr2 += "Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1\r\n";
    ptr2 += "Accept-Encoding: deflate, gzip,.x-gzip, identity, *;q=0\r\n";
    ptr2 += "Referer: http://localhost:5080/demos/echo_test.swf\r\n";
    ptr2 += "Connection: Keep-Alive, TE. TE: deflate, gzip, chunked, identity, trailers\r\n";
    ptr2 += "Content-Length:.9\r\n";
    ptr2 += "Content-Type: application/x-amf\r\n";
    ptr2 += "\r\n";
    ptr2 += *encnum;
    ptr2.resize();              // shrink the buffer to be the exact size of the data

    boost::uint8_t *data2 = http.processHeaderFields(&ptr2);
    boost::shared_ptr<amf::Element> el2 = amf.extractAMF(data2, data2 + 15);
    if ((http.getField("host") == "localhost:5080")
        && (el2->to_number() == 1.2345)
        && (http.getField("content-length") == "9")) {
        runtest.pass("HTTP::processHeaderFields(POST) + NUMBER");
    } else {
        runtest.fail("HTTP::processHeaderFields(POST) + NUMBER");
    }

    boost::shared_ptr<std::vector<std::string> > item2 = http.getFieldItem("accept");
    if (!item2) {
        runtest.unresolved("HTTP::getFieldItem(Accept)");
    } else {
        if (item2->at(2) == "application/xhtml+xml") {
            runtest.pass("HTTP::getFieldItem(Accept)");
        } else {
            runtest.fail("HTTP::getFieldItem(Accept)");
        }
    }

    boost::shared_ptr<std::vector<std::string> > item3 = http.getFieldItem("connection");
    if (!item3) {
        runtest.unresolved("HTTP::getFieldItem(POST)");
    } else {
        if (item3->at(0) == "keep-alive") {
            runtest.pass("HTTP::getFieldItem(Connection)");
        } else {
            runtest.fail("HTTP::getFieldItem(Connection)");
        }
    }

#if 0
    // Make sure we can parse the Red5 echo_test client messages.
    boost::shared_ptr<Buffer> hex1(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 14 0a 00 00 00 01 02 00 0c 48 65 6c 6c 6f 20 77 6f 72 6c 64 21"));
    boost::shared_ptr<Buffer> hex2(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 02 00 0c 48 65 6c 6c 6f 20 77 6f 72 6c 64 21"));
//    http.clearFields();
    vector<boost::shared_ptr<amf::Element> > headers = http.parseEchoRequest(*hex1);

    if ((strncmp(headers[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers[1]->getName(), "/2", 2) == 0)
        && (strncmp(headers[3]->to_string(), "Hello world!", 12) == 0)) {
        runtest.pass("HTTP::parseEchoRequest()");
    } else {
        runtest.fail("HTTP::parseEchoRequest()");
    }

    amf::Buffer &buff = http.formatEchoResponse(headers[1]->getName(), *headers[3]);
    string head(reinterpret_cast<const char *>(buff.reference()));
    const char *ptr3 = reinterpret_cast<const char *>(hex2->reference());
    const char *ptr4 = reinterpret_cast<const char *>(buff.reference()) + head.size();
    
    if (memcmp(ptr3, ptr4, hex2->allocated()) == 0) {
        runtest.pass("HTTP::formatEchoResponse()");
    } else {
        runtest.fail("HTTP::formatEchoResponse()");
    }
#endif
#endif
    
    if (dbglogfile.getVerbosity() > 0) {
        http.dump();
    }
}

#if 0
void
test_rtmpt (void)
{
    HTTP http;

    // Boolean True request
    boost::shared_ptr<Buffer> hex_req1(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 07 0a 00 00 00 01 01 01"));
    vector<boost::shared_ptr<amf::Element> > headers1 = http.parseEchoRequest(*hex_req1);
    if ((strncmp(headers1[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers1[1]->getName(), "/1", 2) == 0)
        && (headers1[3]->getType() == Element::BOOLEAN_AMF0)
        && (headers1[3]->to_bool() == true)) {
        runtest.pass("HTTP::parseEchoRequest(Boolean TRUE)");
    } else {
        runtest.fail("HTTP::parseEchoRequest(Boolean TRUE)");
    }
//    hex_req1->corrupt();
    
    // Boolean True response
    boost::shared_ptr<Buffer> hex_res1(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 01 01"));
    amf::Buffer &buf1 = http.formatEchoResponse(headers1[1]->getName(), *headers1[3]);
    string head1(reinterpret_cast<const char *>(buf1.reference()));
    const char *ptr1a = reinterpret_cast<const char *>(hex_res1->reference());
    const char *ptr1b = reinterpret_cast<const char *>(buf1.reference()) + head1.size();
    if (memcmp(ptr1a, ptr1b, hex_res1->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Boolean TRUE)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Boolean TRUE)");
    }    
    

    // Boolean false request
    boost::shared_ptr<Buffer> hex_req2(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 07 0a 00 00 00 01 01 00"));
    vector<boost::shared_ptr<amf::Element> > headers2 = http.parseEchoRequest(*hex_req2);
    if ((strncmp(headers2[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers2[1]->getName(), "/2", 2) == 0)
        && (headers2[3]->getType() == Element::BOOLEAN_AMF0)
        && (headers2[3]->to_bool() == false)) {
        runtest.pass("HTTP::parseEchoRequest(Boolean FALSE)");
    } else {
        runtest.fail("HTTP::parseEchoRequest(Boolean FALSE)");
    }
    // Boolean False response
    boost::shared_ptr<Buffer> hex_res2(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 01 00"));
    amf::Buffer &buf2 = http.formatEchoResponse(headers2[1]->getName(), *headers2[3]);
    string head2(reinterpret_cast<const char *>(buf2.reference()));
    const char *ptr2a = reinterpret_cast<const char *>(hex_res2->reference());
    const char *ptr2b = reinterpret_cast<const char *>(buf2.reference()) + head2.size();
    if (memcmp(ptr2a, ptr2b, hex_res2->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Boolean FALSE)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Boolean FALSE)");
    }    

    // NULL Object request
    boost::shared_ptr<Buffer> hex_req3(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 06 0a 00 00 00 01 05"));
    vector<boost::shared_ptr<amf::Element> > headers3 = http.parseEchoRequest(*hex_req3);
    if ((strncmp(headers3[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers3[1]->getName(), "/1", 2) == 0)
        && (headers3[3]->getType() == Element::NULL_AMF0)) {
        runtest.pass("HTTP::parseEchoRequest(NULL Object)");
    } else {
        runtest.fail("HTTP::parseEchoRequest(NULL Object)");
    }
    // NULL Object response
    boost::shared_ptr<Buffer> hex_res3(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 05"));
    amf::Buffer &buf3 = http.formatEchoResponse(headers3[1]->getName(), *headers3[3]);
    string head3(reinterpret_cast<const char *>(buf3.reference()));
    const char *ptr3a = reinterpret_cast<const char *>(hex_res3->reference());
    const char *ptr3b = reinterpret_cast<const char *>(buf3.reference()) + head3.size();
    if (memcmp(ptr3a, ptr3b, hex_res3->allocated()) == 0) {
        runtest.pass("HTTP::formatEchoResponse(NULL Object)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(NULL Object)");
    }    

    // UNDEFINED Object request
    boost::shared_ptr<Buffer> hex_req4(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 06 0a 00 00 00 01 06"));
    vector<boost::shared_ptr<amf::Element> > headers4 = http.parseEchoRequest(*hex_req4);
    if ((strncmp(headers4[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers4[1]->getName(), "/1", 2) == 0)
        && (headers4[3]->getType() == Element::UNDEFINED_AMF0)) {
        runtest.pass("HTTP::parseEchoRequest(UNDEFINED Object)");
    } else {
        runtest.fail("HTTP::parseEchoRequest(UNDEFINED Object)");
    }
    // UNDEFINED Object response
    boost::shared_ptr<Buffer> hex_res4(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 05"));
    amf::Buffer &buf4 = http.formatEchoResponse(headers4[1]->getName(), *headers4[3]);
    string head4(reinterpret_cast<const char *>(buf4.reference()));
    const char *ptr4a = reinterpret_cast<const char *>(hex_res4->reference());
    const char *ptr4b = reinterpret_cast<const char *>(buf4.reference()) + head4.size();
    if (memcmp(ptr4a, ptr4b, hex_res4->allocated()) == 0) {
        runtest.pass("HTTP::formatEchoResponse(UNDEFINED Object, NULL response)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(UNDEFINED Object, NULL response)");
    }    

    // Date Object request
    boost::shared_ptr<Buffer> hex_req5(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 10 0a 00 00 00 01 0b 42 71 e4 ca 4e 32 d0 00 01 a4"));
    vector<boost::shared_ptr<amf::Element> > headers5 = http.parseEchoRequest(*hex_req5);
    if (headers5[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(DATE Object)");
    } else {
        double swapped = *reinterpret_cast<const double*>(headers5[3]->to_reference());
        swapBytes(&swapped, amf::AMF0_NUMBER_SIZE);
        if ((strncmp(headers5[0]->getName(), "echo", 4) == 0)
            && (strncmp(headers5[1]->getName(), "/1", 2) == 0)
            && (headers5[3]->getType() == Element::DATE_AMF0)
            && (headers5[3]->getDataSize() > 0 )
            && (memcmp(hex_req5->reference()+26, &swapped, amf::AMF0_NUMBER_SIZE) == 0)) {
            runtest.pass("HTTP::parseEchoRequest(DATE Object)");
        } else {
            runtest.fail("HTTP::parseEchoRequest(DATE Object)");
        }
    }
    // Date Object response
    boost::shared_ptr<Buffer> hex_res5(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0b 42 71 e4 ca 4e 32 d0 00 fe 5c"));
    if (headers5[3] == 0) {
        runtest.unresolved("HTTP::formatEchoResponse(DATE Object)");
    } else {
        amf::Buffer &buf5 = http.formatEchoResponse(headers5[1]->getName(), *headers5[3]);
        string head5(reinterpret_cast<const char *>(buf5.reference()));
        const char *ptr5a = reinterpret_cast<const char *>(hex_res5->reference()+30);
        const char *ptr5b = reinterpret_cast<const char *>(buf5.reference() + 124);
        if (memcmp(ptr5a, ptr5b, amf::AMF0_NUMBER_SIZE) == 0) {
            runtest.pass("HTTP::formatEchoResponse(DATE Object)");
        } else {
            runtest.fail("HTTP::formatEchoResponse(DATE Object)");
        }
    }

    // Date Array request
    boost::shared_ptr<Buffer> hex_req6(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 18 0a 00 00 00 01 0a 00 00 00 02 0b 42 71 e4 ca 4e 32 d0 00 01 a4 07 00 01"));
    vector<boost::shared_ptr<amf::Element> > headers6 = http.parseEchoRequest(*hex_req6);
    if (headers6[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(DATE Array)");
    } else {
        if ((strncmp(headers6[0]->getName(), "echo", 4) == 0)
            && (strncmp(headers6[1]->getName(), "/2", 2) == 0)
            && (headers6[3]->getType() == Element::STRICT_ARRAY_AMF0)) {
            runtest.pass("HTTP::parseEchoRequest(DATE Array)");
        } else {
            runtest.fail("HTTP::parseEchoRequest(DATE Array)");
        }
    }
    // Date Array response
    boost::shared_ptr<Buffer> hex_res6(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 02 0b 42 71 e4 ca 4e 32 d0 00 fe 5c 0b 42 71 e4 ca 4e 32 d0 00 fe 5c"));
    
    // Undefined Array request
    boost::shared_ptr<Buffer> hex_req7(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 0a 0a 00 00 00 01 0a 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers7 = http.parseEchoRequest(*hex_req7);
    if ((strncmp(headers7[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers7[1]->getName(), "/1", 2) == 0)
        && (headers7[3]->getType() == Element::STRICT_ARRAY_AMF0)) {
        runtest.pass("HTTP::parseEchoRequest(Undefined Strict Array)");
    } else {
        runtest.fail("HTTP::parseEchoRequest(Undefined Strict Array)");
    }
    // Undefined Array response
    boost::shared_ptr<Buffer> hex_res7(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 00"));
    amf::Buffer &buf7 = http.formatEchoResponse(headers7[1]->getName(), *headers7[3]);

    //    cerr << hexify(hex_res7->reference(), hex_res7->allocated(), false) << endl;
    string head7(reinterpret_cast<const char *>(buf7.reference()));
    const char *ptr7a = reinterpret_cast<const char *>(hex_res7->reference());
    const char *ptr7b = reinterpret_cast<const char *>(buf7.reference()) + head7.size();
    //    cerr << hexify(buf7.reference() + head7.size(), buf7.allocated() - head7.size(), false) << endl;
    if (memcmp(ptr7a, ptr7b, hex_res7->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Undefined Strict Array)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Undefined Strict Array)");
    }
    
    // Number 1
    // Array request
    boost::shared_ptr<Buffer> hex_req8(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 13 0a 00 00 00 01 0a 00 00 00 01 00 3f f0 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers8 = http.parseEchoRequest(*hex_req8);
    if (headers8[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(Simple Strict Array of Numbers, 1 item)");
    } else {
        if (headers8[3]->propertySize() > 0) {
            std::vector<boost::shared_ptr<amf::Element> > props8 = headers8[3]->getProperties();
            if ((strncmp(headers8[0]->getName(), "echo", 4) == 0)
                && (strncmp(headers8[1]->getName(), "/2", 2) == 0)
                && (headers8[3]->getType() == Element::STRICT_ARRAY_AMF0)
                && (props8[0]->getType() == Element::NUMBER_AMF0)
                && (props8[0]->to_number() == 1)
                ) {
                runtest.pass("HTTP::parseEchoRequest(Simple Strict Array of Numbers. 1 item)");
            } else {
                runtest.fail("HTTP::parseEchoRequest(Simple Strict Array of Numbers, 1 item)");
            }
        } else {
            runtest.fail("HTTP::parseEchoRequest(Simple Strict Array of Numbers, 1 item)");
        }
    }
    // Undefined Array response
    boost::shared_ptr<Buffer> hex_res8(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 01 00 3f f0 00 00 00 00 00 00"));
    amf::Buffer &buf8 = http.formatEchoResponse(headers8[1]->getName(), *headers8[3]);
    //    cerr << hexify(hex_res8->reference()+30, amf::AMF0_NUMBER_SIZE, false) << endl;
    //    cerr << hexify(buf8.reference() + 124, amf::AMF0_NUMBER_SIZE, false) << endl;
    string head8(reinterpret_cast<const char *>(buf8.reference()));
    const char *ptr8a = reinterpret_cast<const char *>(hex_res8->reference());
    const char *ptr8b = reinterpret_cast<const char *>(buf8.reference()) + head8.size();
    if (memcmp(ptr8a, ptr8b, hex_res8->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Simple Strict Array of Numbers, 1 item)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Simple Strict Array of Numbers, 1 item)");
    }

    // Number 1,2
    // Array request
    boost::shared_ptr<Buffer> hex_req9(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 33 00 00 00 1c 0a 00 00 00 01 0a 00 00 00 02 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers9 = http.parseEchoRequest(*hex_req9);
    if ((strncmp(headers9[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers9[1]->getName(), "/3", 2) == 0)
        && (headers9[3]->getType() == Element::STRICT_ARRAY_AMF0)) {
        runtest.pass("HTTP::parseEchoRequest(Simple Strict Array of Numbers, 2 items)");
    } else {
        
        runtest.fail("HTTP::parseEchoRequest(Simple Strict Array of Numbers, 2 items)");
    }
    // Undefined Array response
    boost::shared_ptr<Buffer> hex_res9(new Buffer("00 00 00 00 00 01 00 0b 2f 33 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 02 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00"));
    amf::Buffer &buf9 = http.formatEchoResponse(headers9[1]->getName(), *headers9[3]);
    string head9(reinterpret_cast<const char *>(buf9.reference()));
    const char *ptr9a = reinterpret_cast<const char *>(hex_res9->reference());
    const char *ptr9b = reinterpret_cast<const char *>(buf9.reference()) + head9.size();
    if (memcmp(ptr9a, ptr9b, hex_res9->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Simple Strict Array of Numbers, 2 items)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Simple Strict Array of Numbers, 2 items)");
    }

    // Number 1,2,3
    // Array request
    boost::shared_ptr<Buffer> hex_req10(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 34 00 00 00 25 0a 00 00 00 01 0a 00 00 00 03 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 00 40 08 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers10 = http.parseEchoRequest(*hex_req10);
    if ((strncmp(headers10[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers10[1]->getName(), "/4", 2) == 0)
        && (headers10[3]->getType() == Element::STRICT_ARRAY_AMF0)) {
        runtest.pass("HTTP::parseEchoRequest(Simple Strict Array of Numbers, 3 items)");
    } else {
        
        runtest.fail("HTTP::parseEchoRequest(Simple Strict Array of Numbers, 3 items)");
    }
    // Undefined Array response
    boost::shared_ptr<Buffer> hex_res10(new Buffer("00 00 00 00 00 01 00 0b 2f 34 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 03 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 00 40 08 00 00 00 00 00 00"));
    amf::Buffer &buf10 = http.formatEchoResponse(headers10[1]->getName(), *headers10[3]);
    string head10(reinterpret_cast<const char *>(buf10.reference()));
    const char *ptr10a = reinterpret_cast<const char *>(hex_res10->reference());
    const char *ptr10b = reinterpret_cast<const char *>(buf10.reference()) + head10.size();
    if (memcmp(ptr10a, ptr10b, hex_res10->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Simple Strict Array of Numbers, 3 items)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Simple Strict Array of Numbers, 3 items)");
    }

    // Number 0 Request
    boost::shared_ptr<Buffer> hex_req11(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 0e 0a 00 00 00 01 00 00 00 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers11 = http.parseEchoRequest(*hex_req11);
    if ((strncmp(headers11[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers11[1]->getName(), "/1", 2) == 0)
        && (headers11[3]->getType() == Element::NUMBER_AMF0)
        && (headers11[3]->to_number() == 0)) {
        runtest.pass("HTTP::parseEchoRequest(Number 0)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number 0)");
    }
    // Number 0 Response
    boost::shared_ptr<Buffer> hex_res11(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 00 00 00 00 00 00 00 00"));
    amf::Buffer &buf11 = http.formatEchoResponse(headers11[1]->getName(), *headers11[3]);
    string head11(reinterpret_cast<const char *>(buf11.reference()));
    const char *ptr11a = reinterpret_cast<const char *>(hex_res11->reference());
    const char *ptr11b = reinterpret_cast<const char *>(buf11.reference()) + head11.size();
    if (memcmp(ptr11a, ptr11b, hex_res11->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number 0)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number 0)");
    }    

    // Number 1 Request
    boost::shared_ptr<Buffer> hex_req12(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 0e 0a 00 00 00 01 00 3f f0 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers12 = http.parseEchoRequest(*hex_req12);
    if ((strncmp(headers12[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers12[1]->getName(), "/2", 2) == 0)
        && (headers12[3]->getType() == Element::NUMBER_AMF0)
        && (headers12[3]->to_number() == 1)) {
        runtest.pass("HTTP::parseEchoRequest(Number 1)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number 1)");
    }
    // Number 1 Response
    boost::shared_ptr<Buffer> hex_res12(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 3f f0 00 00 00 00 00 00"));
    amf::Buffer &buf12 = http.formatEchoResponse(headers12[1]->getName(), *headers12[3]);
    string head12(reinterpret_cast<const char *>(buf12.reference()));
    const char *ptr12a = reinterpret_cast<const char *>(hex_res12->reference());
    const char *ptr12b = reinterpret_cast<const char *>(buf12.reference()) + head12.size();
    if (memcmp(ptr12a, ptr12b, hex_res12->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number 1)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number 1)");
    }    

    // Number -1 Request
    boost::shared_ptr<Buffer> hex_req13(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 33 00 00 00 0e 0a 00 00 00 01 00 bf f0 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers13 = http.parseEchoRequest(*hex_req13);
    if ((strncmp(headers13[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers13[1]->getName(), "/3", 2) == 0)
        && (headers13[3]->getType() == Element::NUMBER_AMF0)
        && (headers13[3]->to_number() == -1)) {
        runtest.pass("HTTP::parseEchoRequest(Number -1)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number -1)");
    }
    // Number -1 Response
    boost::shared_ptr<Buffer> hex_res13(new Buffer("00 00 00 00 00 01 00 0b 2f 33 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 bf f0 00 00 00 00 00 00"));
    amf::Buffer &buf13 = http.formatEchoResponse(headers13[1]->getName(), *headers13[3]);
    string head13(reinterpret_cast<const char *>(buf13.reference()));
    const char *ptr13a = reinterpret_cast<const char *>(hex_res13->reference());
    const char *ptr13b = reinterpret_cast<const char *>(buf13.reference()) + head13.size();
    if (memcmp(ptr13a, ptr13b, hex_res13->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number -1)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number -1)");
    }

    // Number 256 Request
    boost::shared_ptr<Buffer> hex_req14(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 34 00 00 00 0e 0a 00 00 00 01 00 40 70 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers14 = http.parseEchoRequest(*hex_req14);
    if ((strncmp(headers14[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers14[1]->getName(), "/4", 2) == 0)
        && (headers14[3]->getType() == Element::NUMBER_AMF0)
        && (headers14[3]->to_number() == 256)) {
        runtest.pass("HTTP::parseEchoRequest(Number 256)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number 256)");
    }
    // Number 256 Response
    boost::shared_ptr<Buffer> hex_res14(new Buffer("00 00 00 00 00 01 00 0b 2f 34 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 40 70 00 00 00 00 00 00"));
    amf::Buffer &buf14 = http.formatEchoResponse(headers14[1]->getName(), *headers14[3]);
    string head14(reinterpret_cast<const char *>(buf14.reference()));
    const char *ptr14a = reinterpret_cast<const char *>(hex_res14->reference());
    const char *ptr14b = reinterpret_cast<const char *>(buf14.reference()) + head14.size();
    if (memcmp(ptr14a, ptr14b, AMF0_NUMBER_SIZE) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number 256)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number 256)");
    }

    // Number -256 Request
    boost::shared_ptr<Buffer> hex_req15(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 35 00 00 00 0e 0a 00 00 00 01 00 c0 70 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers15 = http.parseEchoRequest(*hex_req15);
    if ((strncmp(headers15[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers15[1]->getName(), "/5", 2) == 0)
        && (headers15[3]->getType() == Element::NUMBER_AMF0)
        && (headers15[3]->to_number() == -256)) {
        runtest.pass("HTTP::parseEchoRequest(Number -256)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number -256)");
    }
    // Number -256 Response
    boost::shared_ptr<Buffer> hex_res15(new Buffer("00 00 00 00 00 01 00 0b 2f 35 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 c0 70 00 00 00 00 00 00"));
    amf::Buffer &buf15 = http.formatEchoResponse(headers15[1]->getName(), *headers15[3]);
    string head15(reinterpret_cast<const char *>(buf15.reference()));
    const char *ptr15a = reinterpret_cast<const char *>(hex_res15->reference());
    const char *ptr15b = reinterpret_cast<const char *>(buf15.reference()) + head15.size();
    if (memcmp(ptr15a, ptr15b, hex_res15->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number -256)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number -256)");
    }

    // Number 65536 Request
    boost::shared_ptr<Buffer> hex_req16(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 36 00 00 00 0e 0a 00 00 00 01 00 40 f0 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers16 = http.parseEchoRequest(*hex_req16);
    if ((strncmp(headers16[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers16[1]->getName(), "/6", 2) == 0)
        && (headers16[3]->getType() == Element::NUMBER_AMF0)
        && (headers16[3]->to_number() == 65536)) {
        runtest.pass("HTTP::parseEchoRequest(Number 65536)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number 65536)");
    }
    // Number 65536 Response
    boost::shared_ptr<Buffer> hex_res16(new Buffer("00 00 00 00 00 01 00 0b 2f 36 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 40 f0 00 00 00 00 00 00"));
    amf::Buffer &buf16 = http.formatEchoResponse(headers16[1]->getName(), *headers16[3]);
    string head16(reinterpret_cast<const char *>(buf16.reference()));
    const char *ptr16a = reinterpret_cast<const char *>(hex_res16->reference());
    const char *ptr16b = reinterpret_cast<const char *>(buf16.reference()) + head16.size();
    if (memcmp(ptr16a, ptr16b, hex_res16->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number 65536)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number 65536)");
    }

    // Number -655536 Request
    boost::shared_ptr<Buffer> hex_req16x(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 37 00 00 00 0e 0a 00 00 00 01 00 c0 f0 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers16x = http.parseEchoRequest(*hex_req16x);
    if ((strncmp(headers16x[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers16x[1]->getName(), "/7", 2) == 0)
        && (headers16x[3]->getType() == Element::NUMBER_AMF0)
        && (headers16x[3]->to_number() == -65536)) {
        runtest.pass("HTTP::parseEchoRequest(Number -65536)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number -65536)");
    }
    // Number -655536 Response
    boost::shared_ptr<Buffer> hex_res17(new Buffer("00 00 00 00 00 01 00 0b 2f 37 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 c0 f0 00 00 00 00 00 00"));
    amf::Buffer &buf17 = http.formatEchoResponse(headers16x[1]->getName(), *headers16x[3]);
    string head17(reinterpret_cast<const char *>(buf17.reference()));
    const char *ptr17a = reinterpret_cast<const char *>(hex_res17->reference());
    const char *ptr17b = reinterpret_cast<const char *>(buf17.reference()) + head17.size();
    if (memcmp(ptr17a, ptr17b, hex_res17->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number -65536)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number -65536)");
    }

    // Number 0 Request
    boost::shared_ptr<Buffer> hex_req18(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 38 00 00 00 0e 0a 00 00 00 01 00 00 00 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers18 = http.parseEchoRequest(*hex_req18);
    if ((strncmp(headers18[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers18[1]->getName(), "/8", 2) == 0)
        && (headers18[3]->getType() == Element::NUMBER_AMF0)
        && (headers18[3]->to_number() == 0)) {
        runtest.pass("HTTP::parseEchoRequest(Number 0)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number 0)");
    }
    // Number 0 Response
    boost::shared_ptr<Buffer> hex_res18(new Buffer("00 00 00 00 00 01 00 0b 2f 38 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 00 00 00 00 00 00 00 00"));
    amf::Buffer &buf18 = http.formatEchoResponse(headers18[1]->getName(), *headers18[3]);
    string head18(reinterpret_cast<const char *>(buf18.reference()));
    const char *ptr18a = reinterpret_cast<const char *>(hex_res18->reference());
    const char *ptr18b = reinterpret_cast<const char *>(buf18.reference()) + head18.size();
    if (memcmp(ptr18a, ptr18b, hex_res18->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number 0)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number 0)");
    }

    // Number 1.5 Request
    boost::shared_ptr<Buffer> hex_req19(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 39 00 00 00 0e 0a 00 00 00 01 00 3f f8 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers19 = http.parseEchoRequest(*hex_req19);
    if ((strncmp(headers19[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers19[1]->getName(), "/9", 2) == 0)
        && (headers19[3]->getType() == Element::NUMBER_AMF0)
        && (headers19[3]->to_number() == 1.5)) {
        runtest.pass("HTTP::parseEchoRequest(Number 1.5)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number 1.5)");
    }
    // Number 1.5 Response
    boost::shared_ptr<Buffer> hex_res19(new Buffer("00 00 00 00 00 01 00 0b 2f 39 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 3f f8 00 00 00 00 00 00"));
    amf::Buffer &buf19 = http.formatEchoResponse(headers19[1]->getName(), *headers19[3]);
    string head19(reinterpret_cast<const char *>(buf19.reference()));
    const char *ptr19a = reinterpret_cast<const char *>(hex_res19->reference());
    const char *ptr19b = reinterpret_cast<const char *>(buf19.reference()) + head19.size();
    if (memcmp(ptr19a, ptr19b, hex_res19->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number 1.5");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number 1.5");
    }

    // Number -1.5 Request
    boost::shared_ptr<Buffer> hex_req20(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 03 2f 31 30 00 00 00 0e 0a 00 00 00 01 00 bf f8 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers20 = http.parseEchoRequest(*hex_req20);
    if ((strncmp(headers20[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers20[1]->getName(), "/10", 2) == 0)
        && (headers20[3]->getType() == Element::NUMBER_AMF0)
        && (headers20[3]->to_number() == -1.5)) {
        runtest.pass("HTTP::parseEchoRequest(Number -1.5)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number -1.5)");
    }
    // Number -1.5 Response
    boost::shared_ptr<Buffer> hex_res20(new Buffer("00 00 00 00 00 01 00 0c 2f 31 30 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 bf f8 00 00 00 00 00 00"));
    amf::Buffer &buf20 = http.formatEchoResponse(headers20[1]->getName(), *headers20[3]);
    string head20(reinterpret_cast<const char *>(buf20.reference()));
    const char *ptr20a = reinterpret_cast<const char *>(hex_res20->reference());
    const char *ptr20b = reinterpret_cast<const char *>(buf20.reference()) + head20.size();
    if (memcmp(ptr20a, ptr20b, hex_res20->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number -1.5");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number -1.5");
    }

    // Number NaN Request
    boost::shared_ptr<Buffer> hex_req21(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 03 2f 31 31 00 00 00 0e 0a 00 00 00 01 00 ff f8 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers21 = http.parseEchoRequest(*hex_req21);
    if ((strncmp(headers21[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers21[1]->getName(), "/11", 2) == 0)
        && (headers21[3]->getType() == Element::NUMBER_AMF0)
        && (isnan(headers21[3]->to_number()))) {
        runtest.pass("HTTP::parseEchoRequest(Number NaN)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number Nan)");
    }
    // Number NaN Response
    boost::shared_ptr<Buffer> hex_res21(new Buffer("00 00 00 00 00 01 00 0c 2f 31 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 ff f8 00 00 00 00 00 00"));
    amf::Buffer &buf21 = http.formatEchoResponse(headers21[1]->getName(), *headers21[3]);
    string head21(reinterpret_cast<const char *>(buf21.reference()));
    const char *ptr21a = reinterpret_cast<const char *>(hex_res21->reference());
    const char *ptr21b = reinterpret_cast<const char *>(buf21.reference()) + head21.size();
    if (memcmp(ptr21a, ptr21b, hex_res21->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number Nan");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number Nan");
    }

    // Number -Infinity Request
    boost::shared_ptr<Buffer> hex_req22(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 03 2f 31 32 00 00 00 0e 0a 00 00 00 01 00 ff f0 00 00 00 00 00 00"));

    // Number Infinity Request
    boost::shared_ptr<Buffer> hex_req22x(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 03 2f 31 34 00 00 00 0e 0a 00 00 00 01 00 7f ef ff ff ff ff ff ff"));
    vector<boost::shared_ptr<amf::Element> > headers22x = http.parseEchoRequest(*hex_req22x);
    if ((strncmp(headers22x[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers22x[1]->getName(), "/14", 2) == 0)
        && (headers22x[3]->getType() == Element::NUMBER_AMF0)
        && (isFinite(headers22x[3]->to_number()))) {
        runtest.pass("HTTP::parseEchoRequest(Number Infinity)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Number Infinity)");
    }
    // Number Infinity Response
    boost::shared_ptr<Buffer> hex_res23(new Buffer("00 00 00 00 00 01 00 0c 2f 31 33 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 7f f0 00 00 00 00 00 00"));
#if 0
    amf::Buffer &buf23 = http.formatEchoResponse(headers22x[1]->getName(), *headers22x[3]);
    string head23(reinterpret_cast<const char *>(buf23.reference()));
    const char *ptr23a = reinterpret_cast<const char *>(hex_res23->reference());
    const char *ptr23b = reinterpret_cast<const char *>(buf23.reference()) + head23.size();
    if (memcmp(ptr23a, ptr23b, hex_res23->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Number Infinity)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Number Infinity)");
    }
#endif
    
    // Number 1.79769313486231e+308 Request
    boost::shared_ptr<Buffer> hex_req24(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 03 2f 31 35 00 00 00 0e 0a 00 00 00 01 00 00 00 00 00 00 00 00 01"));
    // Number 1.79769313486231e+308 Response
    boost::shared_ptr<Buffer> hex_res24(new Buffer("00 00 00 00 00 01 00 0c 2f 31 34 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 7f ef ff ff ff ff ff ff"));

    // Number 4.940656484124654e-324 Request
    boost::shared_ptr<Buffer> hex_req25(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 03 2f 31 36 00 00 00 0e 0a 00 00 00 01 00 00 00 00 00 00 00 00 00"));
    // Number 4.940656484124654e-324 Response
    boost::shared_ptr<Buffer> hex_res25(new Buffer("00 00 00 00 00 01 00 0c 2f 31 35 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 00 00 00 00 00 00 00 00 01"));

    // Number 1,2,1,2
    // Array request
    boost::shared_ptr<Buffer> hex_req26(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 35 00 00 00 33 0a 00 00 00 01 0a 00 00 00 03 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 0a 00 00 00 02 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers26 = http.parseEchoRequest(*hex_req26);
    std::vector<boost::shared_ptr<amf::Element> > props26 = headers26[3]->getProperties();
    std::vector<boost::shared_ptr<amf::Element> > props26a = props26[2]->getProperties();
    if ((strncmp(headers26[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers26[1]->getName(), "/5", 2) == 0)
        && (headers26[3]->getType() == Element::STRICT_ARRAY_AMF0)
	&& (props26[0]->getType() == Element::NUMBER_AMF0)
	&& (props26[0]->to_number() == 1)
	&& (props26[2]->getType() == Element::STRICT_ARRAY_AMF0)
	&& (props26a[1]->getType() == Element::NUMBER_AMF0)
	&& (props26a[1]->to_number() == 2)
	) {
        runtest.pass("HTTP::parseEchoRequest(Strict Array of Numbers, 3 items)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Strict Array of Numbers, 3 items)");
    }
    // Undefined Array response
    boost::shared_ptr<Buffer> hex_res26(new Buffer("00 00 00 00 00 01 00 0b 2f 35 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 03 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 0a 00 00 00 02 00 3f f0 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00"));
    amf::Buffer &buf26 = http.formatEchoResponse(headers26[1]->getName(), *headers26[3]);
    string head26(reinterpret_cast<const char *>(buf26.reference()));
    //    cerr << hexify(hex_res26->reference()+30, amf::AMF0_NUMBER_SIZE, false) << endl;
    const char *ptr26a = reinterpret_cast<const char *>(hex_res26->reference());
    //    cerr << hexify(buf26.reference() + 124, amf::AMF0_NUMBER_SIZE, false) << endl;
    const char *ptr26b = reinterpret_cast<const char *>(buf26.reference()) + head26.size();
    if (memcmp(ptr26a, ptr26b, hex_res26->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Strict Array of Numbers, 3 items)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Strict Array of Numbers, 3 items)");
    }

    // Number 1,,,,,,,100
    // Array request
    boost::shared_ptr<Buffer> hex_req27(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 36 00 00 00 7f 0a 00 00 00 01 0a 00 00 00 65 00 3f f0 00 00 00 00 00 00 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 06 00 40 59 00 00 00 00 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers27 = http.parseEchoRequest(*hex_req27);
    std::vector<boost::shared_ptr<amf::Element> > props27 = headers27[3]->getProperties();
    if ((strncmp(headers27[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers27[1]->getName(), "/6", 2) == 0)
        && (headers27[3]->getType() == Element::STRICT_ARRAY_AMF0)
        && (props27[0]->to_number() == 1)
        && (props27[1]->getType() == Element::UNDEFINED_AMF0)
        && (props27[0]->getType() == Element::NUMBER_AMF0)
        && (props27[0]->to_number() == 1)
        && (props27[100]->to_number() == 100)
        ) { // FIXME: add test for array values
        runtest.pass("HTTP::parseEchoRequest(Strict Array - Number, undefines, Number)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Strict Array - Number, undefines, Number)");
    }
    // Undefined Array response
    boost::shared_ptr<Buffer> hex_res27(new Buffer("00 00 00 00 00 01 00 0b 2f 36 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 08 00 00 00 66 00 01 30 00 3f f0 00 00 00 00 00 00 00 03 31 30 30 00 40 59 00 00 00 00 00 00 00 06 6c 65 6e 67 74 68 00 40 59 80 00 00 00 00 00 00 00 09"));
#if 0
    amf::Buffer &buf27 = http.formatEchoResponse(headers27[1]->getName(), *headers27[3]);
    string head27(reinterpret_cast<const char *>(buf27.reference()));
    cerr << hexify(hex_res27->reference()+29, hex_res27->allocated()-29 , false) << endl;
    cerr << hexify(buf27.reference() + 123, buf27.allocated()-123, false) << endl;
    const char *ptr27a = reinterpret_cast<const char *>(hex_res27->reference());
    const char *ptr27b = reinterpret_cast<const char *>(buf27.reference()) + head27.size();
    if (memcmp(ptr27a, ptr27b, hex_res27->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Strict Array - Number, undefines, Number)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Strict Array  - Number, undefines, Number)");
    }
#endif

#if 0
    // Array request
    boost::shared_ptr<Buffer> hex_req28(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 37 00 00 00 38 0a 00 00 00 01 08 00 00 00 01 00 06 6c 65 6e 67 74 68 00 3f f0 00 00 00 00 00 00 00 01 30 00 3f f0 00 00 00 00 00 00 00 03 6f 6e 65 00 3f f0 00 00 00 00 00 00 00 00 09"));
    vector<boost::shared_ptr<amf::Element> > headers28 = http.parseEchoRequest(*hex_req28);
    std::vector<boost::shared_ptr<amf::Element> > props28 = headers28[3]->getProperties();
    if ((strncmp(headers28[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers28[1]->getName(), "/7", 2) == 0)
        && (headers28[3]->getType() == Element::ECMA_ARRAY_AMF0)
        && (props28[0]->getType() == Element::NUMBER_AMF0)
        && (strcmp(props28[0]->getName(), "length") == 0)
        && (props28[0]->to_number() == 1)
        && (props28[2]->getType() == Element::NUMBER_AMF0)
        && (strcmp(props28[2]->getName(), "one") == 0)
        && (props28[2]->to_number() == 1)
        ) {
        runtest.pass("HTTP::parseEchoRequest(ECMA Array, 2 Numbers)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(ECMA Array, 2 Numbers)");
    }
    
    // Undefined Array response
    boost::shared_ptr<Buffer> hex_res28(new Buffer("00 00 00 00 00 01 00 0b 2f 37 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 08 00 00 00 01 00 03 6f 6e 65 00 3f f0 00 00 00 00 00 00 00 01 30 00 3f f0 00 00 00 00 00 00 00 06 6c 65 6e 67 74 68 00 3f f0 00 00 00 00 00 00 00 00 09"));
    amf::Buffer &buf28 = http.formatEchoResponse(headers28[1]->getName(), *headers28[3]);
//     cerr << hexify(hex_res28->reference()+30, hex_res28->allocated()-30, false) << endl;
//     cerr << hexify(buf28.reference() + 124, buf28.allocated() - 124, false) << endl;
    string head28(reinterpret_cast<const char *>(buf28.reference()));
    const char *ptr28a = reinterpret_cast<const char *>(hex_res28->reference());
    const char *ptr28b = reinterpret_cast<const char *>(buf28.reference()) + head28.size();
    if (memcmp(ptr28a, ptr28b, hex_res28->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(ECMA Array, 2 Numbers)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(ECMA Array, 2 Numbers)");
    }
#endif

    // NULL String request, ie.. no data
    boost::shared_ptr<Buffer> hex_req29(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 08 0a 00 00 00 01 02 00 00"));
    vector<boost::shared_ptr<amf::Element> > headers29 = http.parseEchoRequest(*hex_req29);
    if ((strncmp(headers29[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers29[1]->getName(), "/1", 2) == 0)
        && (headers29[3]->getType() == Element::STRING_AMF0)
        && (headers29[3]->to_string() == 0)) {
        runtest.pass("HTTP::parseEchoRequest(NULL String)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(NULL String)");
    }
    // NULL String response
    boost::shared_ptr<Buffer> hex_res29(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 02 00 00"));
#if 0                           // FIXME: why does this core dump ?
    amf::Buffer &buf29 = http.formatEchoResponse(headers29[1]->getName(), *headers29[3]);
    string head29(reinterpret_cast<const char *>(buf29.reference()));
    const char *ptr29a = reinterpret_cast<const char *>(hex_res29->reference());
    const char *ptr29b = reinterpret_cast<const char *>(buf29.reference()) + head29.size();
    if (memcmp(ptr29a, ptr29b, hex_res29->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(NULL String)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(NULL String)");
    }
#endif
    
    // String request
    // "Hello world!"
    boost::shared_ptr<Buffer> hex_req30(new Buffer(" 00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 14 0a 00 00 00 01 02 00 0c 48 65 6c 6c 6f 20 77 6f 72 6c 64 21"));
    vector<boost::shared_ptr<amf::Element> > headers30 = http.parseEchoRequest(*hex_req30);
    if ((strncmp(headers30[0]->getName(), "echo", 4) == 0)
        && (strncmp(headers30[1]->getName(), "/2", 2) == 0)
        && (headers30[3]->getType() == Element::STRING_AMF0)
        && (strcmp(headers30[3]->to_string(), "Hello world!") == 0)) {
        runtest.pass("HTTP::parseEchoRequest(Simple String)");
    } else {        
        runtest.fail("HTTP::parseEchoRequest(Simple String)");
    }
    // String response
    boost::shared_ptr<Buffer> hex_res30(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 02 00 0c 48 65 6c 6c 6f 20 77 6f 72 6c 64 21"));
    amf::Buffer &buf30 = http.formatEchoResponse(headers30[1]->getName(), *headers30[3]);
    string head30(reinterpret_cast<const char *>(buf30.reference()));
    const char *ptr30a = reinterpret_cast<const char *>(hex_res30->reference());
    const char *ptr30b = reinterpret_cast<const char *>(buf30.reference()) + head30.size();
    if (memcmp(ptr30a, ptr30b, hex_res30->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Simple String)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Simple String)");
    }
    
    // Array of Strings request
    // test1,test2,test3,test4
    boost::shared_ptr<Buffer> hex_req31(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 33 00 00 00 2a 0a 00 00 00 01 0a 00 00 00 04 02 00 05 74 65 73 74 31 02 00 05 74 65 73 74 32 02 00 05 74 65 73 74 33 02 00 05 74 65 73 74 34"));
    vector<boost::shared_ptr<amf::Element> > headers31 = http.parseEchoRequest(*hex_req31);
    if (headers31.size() == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(Simple String Array)");
    } else {

        std::vector<boost::shared_ptr<amf::Element> > props31 = headers31[3]->getProperties();
        if ((strncmp(headers31[0]->getName(), "echo", 4) == 0)
            && (strncmp(headers31[1]->getName(), "/3", 2) == 0)
            && (headers31[3]->getType() == Element::STRICT_ARRAY_AMF0)
            && (strcmp(props31[0]->to_string(), "test1") == 0)
            && (strcmp(props31[1]->to_string(), "test2") == 0)
            && (strcmp(props31[2]->to_string(), "test3") == 0)
            && (strcmp(props31[3]->to_string(), "test4") == 0)
            ) {
            runtest.pass("HTTP::parseEchoRequest(Simple String Array)");
        } else {        
            runtest.fail("HTTP::parseEchoRequest(Simple String Array)");
        }
    }
    
    // Array of Strings response
    boost::shared_ptr<Buffer> hex_res31(new Buffer("00 00 00 00 00 01 00 0b 2f 33 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 04 02 00 05 74 65 73 74 31 02 00 05 74 65 73 74 32 02 00 05 74 65 73 74 33 02 00 05 74 65 73 74 34"));
    amf::Buffer &buf31 = http.formatEchoResponse(headers31[1]->getName(), *headers31[3]);
    string head31(reinterpret_cast<const char *>(buf31.reference()));
    const char *ptr31a = reinterpret_cast<const char *>(hex_res31->reference());
    const char *ptr31b = reinterpret_cast<const char *>(buf31.reference()) + head31.size();
    if (memcmp(ptr31a, ptr31b, hex_res31->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(Simple String Array)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(Simple String Array)");
    }

    // Custom class Request
    // [object EchoClass]                    [object.Object]
    boost::shared_ptr<Buffer> hex_req40(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 26 0a 00 00 00 01 03 00 05 61 74 74 72 32 00 3f f0 00 00 00 00 00 00 00 05 61 74 74 72 31 02 00 03 6f 6e 65 00 00 09"));
    vector<boost::shared_ptr<amf::Element> > headers40 = http.parseEchoRequest(*hex_req40);
    if (headers40[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(object CustomClass)");
    } else {
        std::vector<boost::shared_ptr<amf::Element> > props40 = headers40[3]->getProperties();
        if ((strncmp(headers40[0]->getName(), "echo", 4) == 0)
            && (strncmp(headers40[1]->getName(), "/1", 2) == 0)
            && (headers40[3]->getType() == Element::OBJECT_AMF0)
            && (strcmp(props40[0]->getName(), "attr2") == 0)
            && (props40[0]->to_number() == 1)
            && (strcmp(props40[1]->getName(), "attr1") == 0)
            && (strcmp(props40[1]->to_string(), "one") == 0)) {
            runtest.pass("HTTP::parseEchoRequest(object CustomClass)");
        } else {        
            runtest.fail("HTTP::parseEchoRequest(object CustomClass)");
        }
    }
    boost::shared_ptr<Buffer> hex_res40(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 03 00 05 61 74 74 72 32 00 3f f0 00 00 00 00 00 00 00 05 61 74 74 72 31 02 00 03 6f 6e 65 00 00 09"));
    amf::Buffer &buf40 = http.formatEchoResponse(headers40[1]->getName(), *headers40[3]);

    string head40(reinterpret_cast<const char *>(buf40.reference()));
    const char *ptr40a = reinterpret_cast<const char *>(hex_res40->reference());
    const char *ptr40b = reinterpret_cast<const char *>(buf40.reference()) + head40.size();
    if (memcmp(ptr40a, ptr40b, hex_res40->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(object CustomClass)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(object CustomClass)");
    }
    
    // [object EchoClass],[object EchoClass] [object.Object],[object.Object]
    boost::shared_ptr<Buffer> hex_req41(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 2e 0a 00 00 00 01 0a 00 00 00 02 03 00 05 61 74 74 72 32 00 3f f0 00 00 00 00 00 00 00 05 61 74 74 72 31 02 00 03 6f 6e 65 00 00 09 07 00 01"));
    vector<boost::shared_ptr<amf::Element> > headers41 = http.parseEchoRequest(*hex_req41);
    if (headers41[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(object CustomClass Array)");
    } else {
        if ((headers41[3]->getType() == Element::STRICT_ARRAY_AMF0)
	    && (headers41[3]->propertySize() == 2)) {
            runtest.pass("HTTP::parseEchoRequest(object CustomClass Array)");
        } else {        
            runtest.fail("HTTP::parseEchoRequest(object CustomClass Array)");
        }
    }
    boost::shared_ptr<Buffer> hex_res41(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 02 03 00 05 61 74 74 72 32 00 3f f0 00 00 00 00 00 00 00 05 61 74 74 72 31 02 00 03 6f 6e 65 00 00 09 07 00 01"));
    amf::Buffer &buf41 = http.formatEchoResponse(headers41[1]->getName(), *headers41[3]);
    string head41(reinterpret_cast<const char *>(buf41.reference()));
    const char *ptr41a = reinterpret_cast<const char *>(hex_res41->reference());
    const char *ptr41b = reinterpret_cast<const char *>(buf41.reference()) + head41.size();
    if (memcmp(ptr41a, ptr41b, hex_res41->allocated()-4) == 0) {
        runtest.pass("HTTP::formatEchoResponse(object CustomClass Array)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(object CustomClass Array)");
    }

    // Remote Class
    // [object RemoteClass]                      [object RemoteClass]
    boost::shared_ptr<Buffer> hex_req42(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 31 00 00 00 59 0a 00 00 00 01 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 32 00 40 00 00 00 00 00 00 00 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 03 6f 6e 65 00 00 09"));
    vector<boost::shared_ptr<amf::Element> > headers42 = http.parseEchoRequest(*hex_req42);
    if (headers42[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(Remote Class)");
    } else {    
        if ((strncmp(headers42[0]->getName(), "echo", 4) == 0)
            && (strncmp(headers42[1]->getName(), "/1", 2) == 0)
            && (headers42[3]->getType() == Element::TYPED_OBJECT_AMF0)
            && (headers42[3]->propertySize() == 2)) {
            runtest.pass("HTTP::parseEchoRequest(object RemoteClass)");
        } else {        
            runtest.fail("HTTP::parseEchoRequest(object RemoteClass)");
        }
    }

    boost::shared_ptr<Buffer> hex_res42(new Buffer("00 00 00 00 00 01 00 0b 2f 31 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 03 6f 6e 65 00 0a 61 74 74 72 69 62 75 74 65 32 00 40 00 00 00 00 00 00 00 00 00 09"));
    amf::Buffer &buf42 = http.formatEchoResponse(headers42[1]->getName(), *headers42[3]);
    string head42(reinterpret_cast<const char *>(buf42.reference()));
    const char *ptr42a = reinterpret_cast<const char *>(hex_res42->reference());
    const char *ptr42b = reinterpret_cast<const char *>(buf42.reference()) + head42.size();
    if (memcmp(ptr42a, ptr42b, hex_res42->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(object RemoteClass)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(object RemoteClass)");
    }
    
    // An array of RemoteClass objects
    // org.red5.server.webapp.echo.RemoteClass
    // [object RemoteClass],[object RemoteClass] [object RemoteClass],[object RemoteClass]
    boost::shared_ptr<Buffer> hex_req43(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 32 00 00 00 b2 0a 00 00 00 01 0a 00 00 00 02 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 32 00 3f f0 00 00 00 00 00 00 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 03 6f 6e 65 00 00 09 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 32 00 40 00 00 00 00 00 00 00 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 03 74 77 6f 00 00 09"));
    vector<boost::shared_ptr<amf::Element> > headers43 = http.parseEchoRequest(*hex_req43);
    if (headers43[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(object RemoteClass Array, 2 items)");
    } else {
	std::vector<boost::shared_ptr<amf::Element> > props43 = headers43[3]->getProperties();
	std::vector<boost::shared_ptr<amf::Element> > props43a = props43[0]->getProperties();
	std::vector<boost::shared_ptr<amf::Element> > props43b = props43[1]->getProperties();
        if ((strncmp(headers43[0]->getName(), "echo", 4) == 0)
            && (strncmp(headers43[1]->getName(), "/2", 2) == 0)
            && (headers43[3]->getType() == Element::STRICT_ARRAY_AMF0)
	    && (props43[0]->getType() == Element::TYPED_OBJECT_AMF0)
	    && (props43a[0]->getType() == Element::NUMBER_AMF0)
	    && (props43[1]->getType() == Element::TYPED_OBJECT_AMF0)
	    && (props43a[1]->getType() == Element::STRING_AMF0)
            && (strncmp(props43a[0]->getName(), "attribute2", 10) == 0)
            && (props43a[0]->to_number() == 1)
            && (strncmp(props43a[1]->getName(), "attribute1", 10) == 0)
            && (strncmp(props43a[1]->to_string(), "one", 3) == 0)
            && (strncmp(props43b[0]->getName(), "attribute2", 10) == 0)
            && (props43b[0]->to_number() == 2)
            && (strncmp(props43b[1]->getName(), "attribute1", 10) == 0)
            && (strncmp(props43b[1]->to_string(), "two", 3) == 0)
	    ) {
            runtest.pass("HTTP::parseEchoRequest(object RemoteClass Array, 2 items)");
        } else {        
            runtest.fail("HTTP::parseEchoRequest(object RemoteClass Array, 2 items)");
        }
    }
    boost::shared_ptr<Buffer> hex_res43(new Buffer("00 00 00 00 00 01 00 0b 2f 32 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 0a 00 00 00 02 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 03 6f 6e 65 00 0a 61 74 74 72 69 62 75 74 65 32 00 3f f0 00 00 00 00 00 00 00 00 09 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 03 74 77 6f 00 0a 61 74 74 72 69 62 75 74 65 32 00 40 00 00 00 00 00 00 00 00 00 09"));
    amf::Buffer &buf43 = http.formatEchoResponse(headers43[1]->getName(), *headers43[3]);
    std::vector<boost::shared_ptr<amf::Element> > props43 = headers43[3]->getProperties();
    //    std::vector<boost::shared_ptr<amf::Element> > props43a = props43[0]->getProperties();
//     cerr << hexify(hex_res43->reference()+29, hex_res43->allocated()-29 , false) << endl;
//     cerr << hexify(buf43.reference(), buf43.allocated(), true) << endl;
//     cerr << hexify(buf43.reference() + 124, buf43.allocated()-124, false) << endl;
    string head43(reinterpret_cast<const char *>(buf43.reference()));
    const char *ptr43a = reinterpret_cast<const char *>(hex_res43->reference());
    const char *ptr43b = reinterpret_cast<const char *>(buf43.reference()) + head43.size();
#if 0
    if (memcmp(ptr43a, ptr43b, hex_res43->allocated()-4) == 0) {
        runtest.xpass("HTTP::formatEchoResponse(object RemoteClass Array, 2 items)");
    } else {
        runtest.xfail("HTTP::formatEchoResponse(object RemoteClass Array, 2 items)");
    }
#endif
    
    // [object RemoteClass]                      [object RemoteClass]
    boost::shared_ptr<Buffer> hex_req44(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 33 00 00 00 5b 0a 00 00 00 01 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 32 00 41 d2 65 80 b4 80 00 00 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 05 74 68 72 65 65 00 00 09"));
    vector<boost::shared_ptr<amf::Element> > headers44 = http.parseEchoRequest(*hex_req44);
    if (headers44[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(object RemoteClass Array)");
    } else {
        if ((strncmp(headers44[0]->getName(), "echo", 4) == 0)
            && (strncmp(headers44[1]->getName(), "/3", 2) == 0)
            && (headers44[3]->getType() == Element::TYPED_OBJECT_AMF0)) {
            runtest.pass("HTTP::parseEchoRequest(object RemoteClass Array, 2 items)");
        } else {        
            runtest.fail("HTTP::parseEchoRequest(object RemoteClass Array, 2 items)");
        }
    }
    boost::shared_ptr<Buffer> hex_res44(new Buffer("00 00 00 00 00 01 00 0b 2f 33 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 05 74 68 72 65 65 00 0a 61 74 74 72 69 62 75 74 65 32 00 41 d2 65 80 b4 80 00 00 00 00 09"));
    amf::Buffer &buf44 = http.formatEchoResponse(headers44[1]->getName(), *headers44[3]);
    string head44(reinterpret_cast<const char *>(buf44.reference()));
    const char *ptr44a = reinterpret_cast<const char *>(hex_res44->reference());
    const char *ptr44b = reinterpret_cast<const char *>(buf44.reference()) + head44.size();
    if (memcmp(ptr44a, ptr44b, hex_res44->allocated()-1) == 0) {
        runtest.pass("HTTP::formatEchoResponse(object RemoteClass)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(object RemoteClass)");
    }

#if 0
    // [object RemoteClass]                      [object RemoteClass]
    boost::shared_ptr<Buffer> hex_req45(new Buffer("00 00 00 00 00 01 00 04 65 63 68 6f 00 02 2f 34 00 00 00 5a 0a 00 00 00 01 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 32 00 42 71 3f 8f 4d 00 00 00 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 04 66 6f 75 72 00 00 09"));
    vector<boost::shared_ptr<amf::Element> > headers45 = http.parseEchoRequest(*hex_req45);
    if (headers45[3] == 0) {
        runtest.unresolved("HTTP::parseEchoRequest(object RemoteClass Array)");
    } else {
	std::vector<boost::shared_ptr<amf::Element> > props45 = headers45[3]->getProperties();
        if (props45.size() == 2) {
            if ((strncmp(headers45[0]->getName(), "echo", 4) == 0)
                && (strncmp(headers45[1]->getName(), "/4", 2) == 0)
                && (headers45[3]->getType() == Element::TYPED_OBJECT_AMF0)
                && (strcmp(headers45[3]->getName(), "org.red5.server.webapp.echo.RemoteClass") == 0)
                && (props45[0]->getType() == Element::NUMBER_AMF0)
                && (strncmp(props45[0]->getName(), "attribute2", 10) == 0)
                && (props45[1]->getType() == Element::STRING_AMF0)
                && (strncmp(props45[1]->getName(), "attribute1", 10) == 0)
                && (strncmp(props45[1]->to_string(), "four", 4) == 0)
                ) {
                runtest.pass("HTTP::parseEchoRequest(object RemoteClass)");
            } else {        
                runtest.fail("HTTP::parseEchoRequest(object RemoteClass)");
            }
        } else {
            runtest.untested("HTTP::parseEchoRequest(object RemoteClass)");
        }
    }
    
    boost::shared_ptr<Buffer> hex_res45(new Buffer("00 00 00 00 00 01 00 0b 2f 34 2f 6f 6e 52 65 73 75 6c 74 00 04 6e 75 6c 6c ff ff ff ff 10 00 27 6f 72 67 2e 72 65 64 35 2e 73 65 72 76 65 72 2e 77 65 62 61 70 70 2e 65 63 68 6f 2e 52 65 6d 6f 74 65 43 6c 61 73 73 00 0a 61 74 74 72 69 62 75 74 65 31 02 00 04 66 6f 75 72 00 0a 61 74 74 72 69 62 75 74 65 32 00 c1 9c 2c c0 00 00 00 00 00 00 09"));
    amf::Buffer &buf45 = http.formatEchoResponse(headers45[1]->getName(), *headers45[3]);
    string head45(reinterpret_cast<const char *>(buf45.reference()));
    const char *ptr45a = reinterpret_cast<const char *>(hex_res45->reference());
    const char *ptr45b = reinterpret_cast<const char *>(buf45.reference()) + head45.size();
//     cerr << hexify(hex_res45->reference()+29, hex_res45->allocated()-29 , false) << endl;
//     cerr << hexify(buf45.reference()+124, buf45.allocated()-124, true) << endl;
//     cerr << hexify(buf45.reference()+123, buf45.allocated()-123, false) << endl;
    if (memcmp(ptr45a, ptr45b, hex_res45->allocated()-11) == 0) {
        runtest.pass("HTTP::formatEchoResponse(object RemoteClass)");
    } else {
        runtest.fail("HTTP::formatEchoResponse(object RemoteClass)");
    }
#endif
    
    // String test with 40000 characters
    // String test with 70000 characters
    // String test with 1000000 characters

#ifdef CORRUPT_MEMORY_TESTS
    // Use the existing binary data and see if we can survive decoding corupted
    // packets. While the data may be bogus in the returned Elements, we shouldn't
    // ever crash,. so these are primarily a stress test.
//    cerr << hexify(hex_req1->reference(), hex_req1->allocated(), false) << endl;
    hex_req1->corrupt(6);
//    cerr << hexify(hex_req1->reference(), hex_req1->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt1 = http.parseEchoRequest(*hex_req1);    
    if (corrupt1.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Boolean TRUE)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Boolean TRUE)");
    }

//    cerr << hexify(hex_req2->reference(), hex_req2->allocated(), false) << endl;
    hex_req2->corrupt(4);
//    cerr << hexify(hex_req2->reference(), hex_req2->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt2 = http.parseEchoRequest(*hex_req2);    
    if (corrupt2.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Boolean FALSE)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Boolean FALSE)");
    }

//    cerr << hexify(hex_req3->reference(), hex_req3->allocated(), false) << endl;
    hex_req3->corrupt(3);
//    cerr << hexify(hex_req3->reference(), hex_req3->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt3 = http.parseEchoRequest(*hex_req3);    
    if (corrupt3.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(NULL Object)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(NULL Object)");
    }

//    cerr << hexify(hex_req4->reference(), hex_req4->allocated(), false) << endl;
    hex_req4->corrupt(7);
//    cerr << hexify(hex_req4->reference(), hex_req4->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt4 = http.parseEchoRequest(*hex_req4);    
    if (corrupt4.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(UNDEFINED Object)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(UNDEFINED Object)");
    }
    
    cerr << hexify(hex_req5->reference(), hex_req5->allocated(), false) << endl;
    hex_req5->corrupt(5);
    cerr << hexify(hex_req5->reference(), hex_req5->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt5 = http.parseEchoRequest(*hex_req5);    
    if (corrupt5.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(DATE Object)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(DATE Object)");
    }

//    cerr << hexify(hex_req6->reference(), hex_req6->allocated(), false) << endl;
    hex_req6->corrupt(7);
//    cerr << hexify(hex_req6->reference(), hex_req6->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt6 = http.parseEchoRequest(*hex_req6);    
    if (corrupt6.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(DATE Array)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(DATE Array)");
    }

//    cerr << hexify(hex_req7->reference(), hex_req7->allocated(), false) << endl;
    hex_req7->corrupt(5);
//    cerr << hexify(hex_req7->reference(), hex_req7->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt7 = http.parseEchoRequest(*hex_req7);    
    if (corrupt7.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Undefined Strict Array)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Undefined Strict Array)");
    }

//    cerr << hexify(hex_req8->reference(), hex_req8->allocated(), false) << endl;
    hex_req8->corrupt(2);
//    cerr << hexify(hex_req8->reference(), hex_req8->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt8 = http.parseEchoRequest(*hex_req8);    
    if (corrupt8.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Simple Strict Array of Numbers. 1 item)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Simple Strict Array of Numbers. 1 item)");
    }

//    cerr << hexify(hex_req9->reference(), hex_req9->allocated(), false) << endl;
    hex_req9->corrupt(3);
//    cerr << hexify(hex_req9->reference(), hex_req9->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt9 = http.parseEchoRequest(*hex_req9);    
    if (corrupt9.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Simple Strict Array of Numbers. 2 items)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Simple Strict Array of Numbers. 2 items)");
    }

//    cerr << hexify(hex_req10->reference(), hex_req10->allocated(), false) << endl;
    hex_req10->corrupt(2);
//    cerr << hexify(hex_req10->reference(), hex_req10->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt10 = http.parseEchoRequest(*hex_req10);    
    if (corrupt10.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Simple Strict Array of Numbers. 3 items)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Simple Strict Array of Numbers. 3 items)");
    }

//    cerr << hexify(hex_req11->reference(), hex_req11->allocated(), false) << endl;
    hex_req11->corrupt(2);
//    cerr << hexify(hex_req11->reference(), hex_req11->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt11 = http.parseEchoRequest(*hex_req11);
    if (corrupt11.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number 0)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number 0)");
    }

//    cerr << hexify(hex_req12->reference(), hex_req12->allocated(), false) << endl;
    hex_req12->corrupt(4);
//    cerr << hexify(hex_req12->reference(), hex_req12->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt12 = http.parseEchoRequest(*hex_req12);
    if (corrupt12.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number 1)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number 1)");
    }

//    cerr << hexify(hex_req13->reference(), hex_req13->allocated(), false) << endl;
    hex_req13->corrupt(1);
//    cerr << hexify(hex_req13->reference(), hex_req13->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt13 = http.parseEchoRequest(*hex_req13);
    if (corrupt13.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number -1)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number -1)");
    }

//    cerr << hexify(hex_req14->reference(), hex_req14->allocated(), false) << endl;
    hex_req14->corrupt(5);
//    cerr << hexify(hex_req14->reference(), hex_req14->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt14 = http.parseEchoRequest(*hex_req14);
    if (corrupt14.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number 256)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number 256)");
    }

//    cerr << hexify(hex_req15->reference(), hex_req15->allocated(), false) << endl;
    hex_req15->corrupt(5);
//    cerr << hexify(hex_req15->reference(), hex_req15->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt15 = http.parseEchoRequest(*hex_req15);
    if (corrupt15.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number -256)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number -256)");
    }

//    cerr << hexify(hex_req16->reference(), hex_req16->allocated(), false) << endl;
    hex_req16->corrupt(2);
//    cerr << hexify(hex_req16->reference(), hex_req16->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt16 = http.parseEchoRequest(*hex_req16);
    if (corrupt16.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number 65536)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number 65536)");
    }

//    cerr << hexify(hex_req17->reference(), hex_req17->allocated(), false) << endl;
    hex_req16x->corrupt(6);
//    cerr << hexify(hex_req17->reference(), hex_req17->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt17 = http.parseEchoRequest(*hex_req16x);
    if (corrupt17.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number -65536)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number -65536)");
    }

//    cerr << hexify(hex_req19->reference(), hex_req19->allocated(), false) << endl;
    hex_req19->corrupt(1);
//    cerr << hexify(hex_req19->reference(), hex_req19->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt19 = http.parseEchoRequest(*hex_req19);
    if (corrupt19.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number 1.5)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number 1.5)");
    }

//    cerr << hexify(hex_req20->reference(), hex_req20->allocated(), false) << endl;
    hex_req20->corrupt(4);
//    cerr << hexify(hex_req20->reference(), hex_req20->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt20 = http.parseEchoRequest(*hex_req20);
    if (corrupt20.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number -1.5)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number -1.5)");
    }

//    cerr << hexify(hex_req21->reference(), hex_req21->allocated(), false) << endl;
    hex_req21->corrupt(8);
//    cerr << hexify(hex_req21->reference(), hex_req21->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt21 = http.parseEchoRequest(*hex_req21);
    if (corrupt21.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number NaN)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number NaN)");
    }

//    cerr << hexify(hex_req22->reference(), hex_req22->allocated(), false) << endl;
    hex_req22->corrupt(1);
//    cerr << hexify(hex_req22->reference(), hex_req22->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt22 = http.parseEchoRequest(*hex_req22);
    if (corrupt22.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Number Infinity)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Number Infinity)");
    }

//    cerr << hexify(hex_req26->reference(), hex_req26->allocated(), false) << endl;
    hex_req26->corrupt(5);
//    cerr << hexify(hex_req26->reference(), hex_req26->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt26 = http.parseEchoRequest(*hex_req26);
    if (corrupt26.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Strict Array of Numbers, 3 items)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Strict Array of Numbers, 3 items)");
    }

//    cerr << hexify(hex_req27->reference(), hex_req27->allocated(), false) << endl;
    hex_req27->corrupt(3);
//    cerr << hexify(hex_req27->reference(), hex_req27->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt27 = http.parseEchoRequest(*hex_req27);
    if (corrupt27.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Strict Array - Number, undefines, Number)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Strict Array - Number, undefines, Number)");
    }

//    cerr << hexify(hex_req29->reference(), hex_req29->allocated(), false) << endl;
    hex_req29->corrupt(8);
//    cerr << hexify(hex_req29->reference(), hex_req29->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt29 = http.parseEchoRequest(*hex_req29);
    if (corrupt29.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(NULL String)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(NULL String)");
    }

//    cerr << hexify(hex_req30->reference(), hex_req30->allocated(), false) << endl;
    hex_req30->corrupt(7);
//    cerr << hexify(hex_req30->reference(), hex_req30->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt30 = http.parseEchoRequest(*hex_req30);
    if (corrupt30.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Simple String)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Simple String)");
    }

//    cerr << hexify(hex_req31->reference(), hex_req31->allocated(), false) << endl;
    hex_req31->corrupt(2);
//    cerr << hexify(hex_req31->reference(), hex_req31->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt31 = http.parseEchoRequest(*hex_req31);
    if (corrupt31.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(Simple String Array)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(Simple String Array)");
    }

//    cerr << hexify(hex_req40->reference(), hex_req40->allocated(), false) << endl;
    hex_req40->corrupt(6);
//    cerr << hexify(hex_req40->reference(), hex_req40->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt40 = http.parseEchoRequest(*hex_req40);
    if (corrupt40.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(object CustomClass)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(object CustomClass)");
    }

//    cerr << hexify(hex_req41->reference(), hex_req41->allocated(), false) << endl;
    hex_req41->corrupt(1);
//    cerr << hexify(hex_req41->reference(), hex_req41->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt41 = http.parseEchoRequest(*hex_req41);
    if (corrupt41.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(object CustomClass Array)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(object CustomClass Array)");
    }

//    cerr << hexify(hex_req42->reference(), hex_req42->allocated(), false) << endl;
    hex_req42->corrupt(2);
//    cerr << hexify(hex_req42->reference(), hex_req42->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt42 = http.parseEchoRequest(*hex_req42);
    if (corrupt42.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(object RemoteClass)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(object RemoteClass)");
    }

//    cerr << hexify(hex_req43->reference(), hex_req43->allocated(), false) << endl;
    hex_req43->corrupt(4);
//    cerr << hexify(hex_req43->reference(), hex_req43->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt43 = http.parseEchoRequest(*hex_req43);
    if (corrupt43.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(object RemoteClass Array, 2 items)");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(object RemoteClass Array, 2 items)");
    }

//    cerr << hexify(hex_req44->reference(), hex_req44->allocated(), false) << endl;
    hex_req44->corrupt(3);
//    cerr << hexify(hex_req44->reference(), hex_req44->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt44 = http.parseEchoRequest(*hex_req44);
    if (corrupt44.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(object RemoteClass");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(object RemoteClass");
    }
#endif

//    cerr << hexify(hex_req45->reference(), hex_req45->allocated(), false) << endl;
#if 0
    hex_req45->corrupt(6);
//    cerr << hexify(hex_req45->reference(), hex_req45->allocated(), false) << endl;
    vector<boost::shared_ptr<amf::Element> > corrupt45 = http.parseEchoRequest(*hex_req45);
    if (corrupt45.size()) {
        runtest.pass("Corrupted HTTP::parseEchoRequest(object RemoteClass");
    } else {
        runtest.fail("Corrupted HTTP::parseEchoRequest(object RemoteClass");
    }
#endif
}
#endif

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
main(int /*argc*/, char**)
{
  // nop
  return 0;  
}

#endif
