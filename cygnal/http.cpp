// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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
//

/* $Id: http.cpp,v 1.2 2006/12/17 23:50:48 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
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

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

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
HTTP::operator = (HTTP &obj)
{
    GNASH_REPORT_FUNCTION;
//    this = obj;
}

string
HTTP::waitForGetRequest(Network &net)
{
    GNASH_REPORT_FUNCTION;
    
}

string
HTTP::waitForGetRequest()
{
    GNASH_REPORT_FUNCTION;

    char buffer[readsize+1];
    memset(buffer, 0, readsize+1);
    if (readNet(buffer, readsize) > 0) {
        dbglogfile << "Read initial GET Request" << endl;
    } else {
        dbglogfile << "Couldn't read initial GET Request" << endl;
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
        dbglogfile << "Got legit GET request: " << endl;
    } else {
        dbglogfile << "ERROR: Got bogus GET request!" << endl;
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
    
    if (writeNet(reply, strlen(reply)) == strlen(reply)) {
        dbglogfile << "Sent GET Reply: " << reply << endl;
    } else {
        dbglogfile << "Couldn't send GET Reply" << endl;
    }    
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
};

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

    if (strcasestr(data, "Keep-Alive")) {
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
    
    dbglogfile << "==== The HTTP header breaks down as follows: ====" << endl;
    dbglogfile << "Filespec: " << _filespec.c_str() << endl;
    dbglogfile << "URL: " << _url.c_str() << endl;
    dbglogfile << "Version: " << _version.c_str() << endl;
    dbglogfile << "Method: " << _method.c_str() << endl;
    dbglogfile << "Referer: " << _referer.c_str() << endl;
    dbglogfile << "Connection: " << _connection.c_str() << endl;
    dbglogfile << "Host: " << _host.c_str() << endl;
    dbglogfile << "User Agent: " << _agent.c_str() << endl;
    dbglogfile << "Language: " << _language.c_str() << endl;
    dbglogfile << "Charset: " << _charset.c_str() << endl;
    dbglogfile << "Encoding: " << _encoding << endl;
    dbglogfile << "TE: " << _te.c_str() << endl;
    dbglogfile << "==== ==== ====" << endl;
}

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
