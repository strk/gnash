// http.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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
//

/* $Id: http.cpp,v 1.9 2007/07/01 10:53:49 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/thread/mutex.hpp>
#include <string>
#include <iostream>
#include <cstring>
#include "http.h"
#include "log.h"

using namespace gnash;
using namespace std;

static boost::mutex stl_mutex;


namespace cygnal
{

// FIXME, this seems too small to me.  --gnu
static const int readsize = 1024;

HTTP::HTTP()
{
//    GNASH_REPORT_FUNCTION;
//    struct status_codes *status = new struct status_codes;
    
//    _status_codes(CONTINUE, status);
}

HTTP::~HTTP()
{
//    GNASH_REPORT_FUNCTION;
}

HTTP &
HTTP::operator = (HTTP& /*obj*/)
{
    GNASH_REPORT_FUNCTION;
//    this = obj;
    // TODO: FIXME !
    return *this; 
}

string
HTTP::waitForGetRequest(Network& /*net*/)
{
    GNASH_REPORT_FUNCTION;
    return ""; // TODO: FIXME !
}

string
HTTP::waitForGetRequest()
{
    GNASH_REPORT_FUNCTION;

    char buffer[readsize+1];
    memset(buffer, 0, readsize+1);
    if (readNet(buffer, readsize) > 0) {
        log_msg (_("Read initial GET Request"));
    } else {
        log_error (_("Couldn't read initial GET Request"));
    }
    
    extractMethod(buffer);
//     extractReferer(buffer);
//     extractHost(buffer);
    extractAgent(buffer);
    extractLanguage(buffer);
    extractCharset(buffer);
    extractConnection(buffer);
//     extractEncoding(buffer);
//     extractTE(buffer);
//    dump();

    // See if we got a legit GET request
    if (strncmp(buffer, "GET ", 4) == 0) {
        log_msg (_("Got legit GET request"));
    } else {
        log_error (_("Got bogus GET request"));
    }

    return _url;
}

bool
HTTP::sendGetReply(int filesize)
{
    GNASH_REPORT_FUNCTION;
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

    now.time_of_day();
    
    const char reply[] =
        "HTTP/1.1 200 OK\r\n"
        "Date: Sun, 20 Apr 2006 04:20:00 GMT\r\n"
        "Content-Type: application/futuresplash\r\n"
        "Connection: close\r\n"
        "Content-Length: XX      \r\n"
        "\r\n"                  // All HTTP messages are followed by a blank line.
        ;

    // This is a bit ugly, but we splice the file size onto the request string
    // without any memory allocation, which could incur a small performance hit.
    char *length = strstr(reply, " XX");
    sprintf(length, " %d", filesize);
// FIXME:  Doesn't this write to a const array?  And doesn't it fail to
// supply the two \r\n's needed to finish the string?  --gnu
    
    int ret = writeNet(reply, strlen(reply));

    if (ret >= 0 && (unsigned)ret == strlen(reply)) {
        log_msg (_("Sent GET Reply: %s"), reply);
    } else {
        log_msg (_("Couldn't send GET Reply, writeNet returned %d"), ret);
	// TODO: FIXME: shouldn't we return false here ?
    }
    return true; // Default to true
}

// bool
// HTTP::sendGetReply(Network &net)
// {
//     GNASH_REPORT_FUNCTION;    
// }

// This is what a GET request looks like.
// GET /software/gnash/tests/flvplayer2.swf?file=http://localhost:4080/software/gnash/tests/lulutest.flv HTTP/1.1
// User-Agent: Opera/9.01 (X11; Linux i686; U; en)
// Host: localhost:4080
// Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1
// Accept-Language: en
// Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1
// Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0
// Referer: http://localhost/software/gnash/tests/
// Connection: Keep-Alive, TE
// TE: deflate, gzip, chunked, identity, trailers
string
HTTP::extractMethod(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
    string body = data;
    string::size_type start, end;
    int length;

    length = body.size();
    start = body.find(" ", 0);
    if (start == string::npos) {
        return "error";
    }
    _method = body.substr(0, start);
    end = body.find(" ", start+1);
    if (end == string::npos) {
        return "error";
    }
    _url = body.substr(start+1, end-start-1);
    _version = body.substr(end+1, length);

    end = _url.find("?", 0);
//    _filespec = _url.substr(start+1, end);
    return "error";
}

string 
HTTP::extractReferer(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "Referer: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _referer = body.substr(start+pattern.size(), end-start-1);
    return _referer;
}

string 
HTTP::extractConnection(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "Connection: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _connection = body.substr(start+pattern.size(), end-start-1);
    return _connection;
}

string
HTTP::extractHost(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "Host: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _host = body.substr(start+pattern.size(), end-start-1);
    return _host;
}

string 
HTTP::extractAgent(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "User-Agent: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _agent = body.substr(start+pattern.size(), end-start-1);
    return _agent;
}

string 
HTTP::extractLanguage(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "Accept-Language: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _language = body.substr(start+pattern.size(), end-start-1);
    return _language;
}

string 
HTTP::extractCharset(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "Accept-Charset: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _charset = body.substr(start+pattern.size(), end-start-1);
    return _charset;
}

string 
HTTP::extractEncoding(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "Accept-Encoding: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _encoding = body.substr(start+pattern.size(), end-start-1);
    return _encoding;
}

string 
HTTP::extractTE(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end;
    string pattern = "TE: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return "error";
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return "error";
    }
    
    _te = body.substr(start+pattern.size(), end-start-1);
    return _te;
}

bool
HTTP::keepAlive(const char *data)
{
//    GNASH_REPORT_FUNCTION;

    if (strcasecmp(data, "Keep-Alive")) {
	return true;
    } else {
	return false;
    }
}

bool
HTTP::keepAlive()
{
//    GNASH_REPORT_FUNCTION;
    // FIXME: is their a way to make find case insensitive that's
    // less than 20 lines long ?
    return keepAlive(_connection.c_str());
}

void
HTTP::dump() {
    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
    
    log_msg (_("==== The HTTP header breaks down as follows: ===="));
    log_msg (_("Filespec: %s"), _filespec.c_str());
    log_msg (_("URL: %s"), _url.c_str());
    log_msg (_("Version: %s"), _version.c_str());
    log_msg (_("Method: %s"), _method.c_str());
    log_msg (_("Referer: %s"), _referer.c_str());
    log_msg (_("Connection: %s"), _connection.c_str());
    log_msg (_("Host: %s"), _host.c_str());
    log_msg (_("User Agent: %s"), _agent.c_str());
    log_msg (_("Language: %s"), _language.c_str());
    log_msg (_("Charset: %s"), _charset.c_str());
    log_msg (_("Encoding: %s"), _encoding.c_str());
    log_msg (_("TE: %s"), _te.c_str());
    log_msg (_("==== ==== ===="));
}

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
