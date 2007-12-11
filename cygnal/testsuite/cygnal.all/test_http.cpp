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

#include <string>

#include <unistd.h>
#ifdef HAVE_GETOPT_H
        #include <getopt.h>
#endif

#ifndef __GNUC__
extern int optind, getopt(int, char *const *, const char *);
#endif

#include <sys/types.h>
#include <iostream>
#include <string>
#include <regex.h>

#include "log.h"
#include "http.h"
#include "dejagnu.h"

using namespace cygnal;
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

    HTTP http;

    http.clearHeader();
    http.formatDate();
//    cerr << "FIXME: " << http.getHeader() << endl;

    regex_t regex_pat;

    // Check the Date field
    // The date should look something like this:
    //     Date: Mon, 10 Dec 2007  GMT
    regcomp (&regex_pat, "[A-Z][a-z]*, [0-9]* [A-Z][a-z]* [0-9]* *GMT$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatDate()");
    } else {
        runtest.pass ("Date::formatDate()");
    }
    regfree(&regex_pat);

    // Check the Content-Length field
    http.clearHeader();
    http.formatContentLength(12345);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Content-Length: [0-9]*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatContentLength()");
    } else {
        runtest.pass ("Date::formatContentLength()");
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
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatConnection()");
    } else {
        runtest.pass ("Date::formatConnection()");
    }
    regfree(&regex_pat);

    // Check the Host field
//     bool formatHost(const char *data);
    http.clearHeader();
    data = "localhost:4080";
    http.formatHost(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Host: [A-za-z-]*:[0-9]*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatHost()");
    } else {
        runtest.pass ("Date::formatHost()");
    }
    regfree(&regex_pat);

// Check the Language field
//     bool formatLanguage(const char *data);
    http.clearHeader();
    data = "en-US,en;q=0.9";
    http.formatLanguage(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Accept-Language: en-US,en;q=0.9$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatLanguage()");
    } else {
        runtest.pass ("Date::formatLanguage()");
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
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatCharset()");
    } else {
        runtest.pass ("Date::formatCharset()");
    }
    regfree(&regex_pat);
        
//     bool formatEncoding(const char *data);
    http.clearHeader();
    data = "deflate, gzip, x-gzip, identity, *;q=0";
    http.formatEncoding(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Accept-Encoding: deflate, gzip.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatEncoding()");
    } else {
        runtest.pass ("Date::formatEncoding()");
    }
    regfree(&regex_pat);
        
        
//     bool formatTE(const char *data);
    http.clearHeader();
    data = "deflate, gzip, chunked, identity, trailers";
    http.formatTE(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "TE: deflate, gzip,.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatTE()");
    } else {
        runtest.pass ("Date::formatTE()");
    }
    regfree(&regex_pat);

//     bool formatAgent(const char *data);
    http.clearHeader();
    data = "Gnash 0.8.1-cvs (X11; Linux i686; U; en)";
    http.formatAgent(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "User-Agent: Gnash 0.8.1-cvs.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatAgent()");
    } else {
        runtest.pass ("Date::formatAgent()");
    }
    regfree(&regex_pat);

    // Check the Content Type field. First we check with a
    // specified field, then next to see if the default works.
//     bool formatContentType();
    http.clearHeader();
    http.formatContentType(HTTP::SWF);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Content-Type: application/futuresplash.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatContentType(type)");
    } else {
        runtest.pass ("Date::formatConetnType(type)");
    }
    regfree(&regex_pat);

    http.clearHeader();
    http.formatContentType();
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Content-Type: text/html.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatContentType()");
    } else {
        runtest.pass ("Date::formatContenType()");
    }
    regfree(&regex_pat);

//     bool formatReferer(const char *data);
    http.clearHeader();
    data = "http://localhost/software/gnash/tests/index.html";
    http.formatReferer(data);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "Referer: http://localhost.*index.html.*$",
             REG_NOSUB|REG_NEWLINE);
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatReferer()");
    } else {
        runtest.pass ("Date::formatReferer()");
    }
    regfree(&regex_pat);

    // Check formatHeader()
    http.clearHeader();
    http.formatHeader(RTMP);
//    cerr << "FIXME: " << http.getHeader() << endl;
    regcomp (&regex_pat, "HTTP/1.1 200 OK.*Date:.*Connection:.*-Length.*-Type:.*$",
             REG_NOSUB);        // note that we do want to look for NL
    if (regexec (&regex_pat, http.getHeader().c_str(), 0, (regmatch_t *)0, 0)) {
        runtest.fail ("Date::formatHeader(port)");
    } else {
        runtest.pass ("Date::formatheader(port)");
    }
    regfree(&regex_pat);


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
