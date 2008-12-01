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

#include "log.h"
#include "http.h"
#include "dejagnu.h"
#include "network.h"
#include "amf.h"
#include "buffer.h"

using namespace gnash;
using namespace amf;
using namespace std;

static void usage (void);
static void tests (void);
static void test_post (void);

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

    tests();
    test_post();
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
    http.formatHeader(HTTP::LIFE_IS_GOOD);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "HTTP/1.* 200 OK.*Date:.*Server:.*:.*-Length.*-Type:.*$",
             REG_NOSUB);        // note that we do want to look for NL
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatHeader(port)");
    } else {
        runtest.pass ("HTTP::formatheader(port)");
    }
    regfree(&regex_pat);

    // Check the Server field
    http.clearHeader();
    http.formatErrorResponse(HTTP::NOT_FOUND);
//    cerr << "FIXME: " << http.getHeader() << endl;
//    cerr << "FIXME: " << http.getBody() << endl;
    regcomp (&regex_pat, "Date:.*Server:.*Content-Length:.*Connection:.*Content-Type:.*$",
             REG_NOSUB);        // note that we do want to look for NL
    if (regexec (&regex_pat, reinterpret_cast<const char*>(http.getHeader()), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("HTTP::formatErrorResponse(header)");
    } else {
        runtest.pass ("HTTP::formatErrorResponse(header)");
    }
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
    Network::byte_t *buffer = (Network::byte_t *)"GET /software/gnash/tests/flvplayer.swf?file=http://localhost/software/gnash/tests/Ouray_Ice_Festival_Climbing_Competition.flv HTTP/1.1\r\n"
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
    Network::byte_t *field1 = (Network::byte_t *)"GET /index.html HTTP/1.1";
    HTTP http1;
    http1.extractMethod(field1);
    if ((http1.keepAlive() == true) && (http1.getVersion()->minor == 1)) {
        runtest.pass ("HTTP::extractMethod(HTTP/1.1)");
    } else {
        runtest.fail ("HTTP::extractMethod(HTTP/1.1)");
    }

    Network::byte_t *field2 = (Network::byte_t *)"GET /index.html HTTP/1.0";
    HTTP http2;
    http2.extractMethod(field2);
    if ((http2.keepAlive() == false) && (http2.getVersion()->minor == 0)) {
        runtest.pass ("HTTP::extractMethod(HTTP/1.0)");
    } else {
        runtest.fail ("HTTP::extractMethod(HTTP/1.0)");
    }

    Network::byte_t *field3 = (Network::byte_t *) "Keep-Alive: 300";
    HTTP http3;
    http3.extractKeepAlive(field3);
    if ((http3.keepAlive() == true) && (http3.getMaxRequests() == 300)) {
        runtest.pass ("HTTP::extractKeepAlive(300)");
    } else {
        runtest.fail ("HTTP::extractKeepAlive(300)");
    }
    
    Network::byte_t *field4 = (Network::byte_t *) "Keep-Alive: On";
    HTTP http4;
    http4.extractKeepAlive(field4);
    if (http4.keepAlive() == true) {
        runtest.pass ("HTTP::extractKeepAlive(On)");
    } else {
        runtest.fail ("HTTP::extractKeepAlive(On)");
    }
    
    Network::byte_t *field5 = (Network::byte_t *) "Keep-Alive: Off";
    HTTP http5;
    http5.extractKeepAlive(field5);
    if (http5.keepAlive() == false) {
        runtest.pass ("HTTP::extractKeepAlive(Off)");
    } else {
        runtest.fail ("HTTP::extractKeepAlive(Off)");
    }

// Some browsers have a different synatax, of course, to keep things
// interesting.
    Network::byte_t *buffer2 = (Network::byte_t *)"GET /software/gnash/tests/flvplayer.swf?file=http://localhost/software/gnash/tests/Ouray_Ice_Festival_Climbing_Competition.flv HTTP/1.1\r\n)"
"Content-Language: en-US,en;q=0.9\r\n"
"Content-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1\r\n"
"Content-Encoding: deflate, gzip, x-gzip, identity, *;q=0\r\n";
//    http.extractMethod(buffer);
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

    AMF amf;
    gnash::Network::byte_t *data1 = http.processHeaderFields(ptr1);
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

#if 0
2...........echo../2............ Hello.world!
    
00 00
00 00 00 01
00 04 65 63 68 6f
   00 02 2f 32 00 00 00 14 0a
00 00 00 01
   02 00 0c 48 65 6c 6c 6f 20 77 6f 72 6c 64 21
#endif
        
    gnash::Network::byte_t *data2 = http.processHeaderFields(ptr2);
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

    if (dbglogfile.getVerbosity() > 0) {
        http.dump();
    }

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
