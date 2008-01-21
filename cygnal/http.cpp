// http.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/thread/mutex.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
//#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/time_zone_base.hpp>
#include <string>
#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
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
    : _port(80), _filesize(0), _keepalive(false)
{
//    GNASH_REPORT_FUNCTION;
//    struct status_codes *status = new struct status_codes;
    
//    _status_codes(CONTINUE, status);
}

HTTP::~HTTP()
{
//    GNASH_REPORT_FUNCTION;
}

bool
HTTP::clearHeader()
{
    _header.str("");
    _body.str("");
    _charset.clear();
    _connections.clear();
    _language.clear();
    _encoding.clear();
    _te.clear();
    _accept.clear();
    _filesize = 0;
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

    clearHeader();
    extractAccept(buffer);
    extractMethod(buffer);
    extractReferer(buffer);
    extractHost(buffer);
    extractAgent(buffer);
    extractLanguage(buffer);
    extractCharset(buffer);
    extractConnection(buffer);
    extractEncoding(buffer);
    extractTE(buffer);
    dump();

    // See if we got a legit GET request
    if (strncmp(buffer, "GET ", 4) == 0) {
        log_msg (_("Got legit GET request"));
    } else {
        log_error (_("Got bogus GET request"));
    }

    _filespec = _url;
    return _url;
}

bool
HTTP::formatHeader(const short type)
{
    GNASH_REPORT_FUNCTION;

    formatHeader(_filesize, type);
}


bool
HTTP::formatHeader(int filesize, const short type)
{
    GNASH_REPORT_FUNCTION;

    _header << "HTTP/1.1 200 OK" << endl;
    this->formatServer();
    this->formatDate();
    this->formatConnection("close");
//     _header << "Accept-Ranges: bytes" << endl;
    this->formatContentLength(filesize);
    this->formatContentType();
    // All HTTP messages are followed by a blank line.
    this->terminateHeader();
}

bool
HTTP::formatErrorResponse(http_status_e code)
{
    GNASH_REPORT_FUNCTION;

    // First build the message body, so we know how to set Content-Length
    _body << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">" << endl;
    _body << "<html><head>" << endl;
    _body << "<title>" << code << " Not Found</title>" << endl;
    _body << "</head><body>" << endl;
    _body << "<h1>Not Found</h1>" << endl;
    _body << "<p>The requested URL " << _filespec << " was not found on this server.</p>" << endl;
    _body << "<hr>" << endl;
    _body << "<address>Cygnal (GNU/Linux) Server at localhost Port " << _port << " </address>" << endl;
    _body << "</body></html>" << endl;
    _body << endl;

    // First build the header
    _header << "HTTP/1.1 " << code << " Not Found" << endl;
    formatDate();
    formatServer();
    _filesize = _body.str().size();
    formatContentLength(_filesize);
    formatConnection("close");
    formatContentType(HTTP::HTML);
}

bool
HTTP::formatDate()
{
    GNASH_REPORT_FUNCTION;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    
//    cout <<  now.time_of_day() << endl;
    
    boost::gregorian::date d(now.date());
//     boost::gregorian::date d(boost::gregorian::day_clock::local_day());
//     cout << boost::posix_time::to_simple_string(now) << endl;
//     cout << d.day_of_week() << endl;
//     cout << d.day() << endl;
//     cout << d.year() << endl;
//     cout << d.month() << endl;
    
//    boost::date_time::time_zone_ptr zone(new posix_time_zone("MST"));
//    boost::date_time::time_zone_base b(now "MST");
//    cout << zone.dst_zone_abbrev() << endl;

    _header << "Date: " << d.day_of_week();
    _header << ", " << d.day();
    _header << " "  << d.month();
    _header << " "  << d.year();
    _header << " "  << now.time_of_day();
    _header << " GMT" << endl;

}

bool
HTTP::formatServer()
{
    GNASH_REPORT_FUNCTION;
    _header << "Server: Cygnal (GNU/Linux)" << endl;
}

bool
HTTP::formatServer(const char *data)
{
    GNASH_REPORT_FUNCTION;
    _header << "Server: " << data << endl;
}

bool
HTTP::formatMethod(const char *data)
{
    GNASH_REPORT_FUNCTION;
    _header << "Method: " << data << endl;
}

bool
HTTP::formatReferer(const char *refer)
{
    GNASH_REPORT_FUNCTION;
    _header << "Referer: " << refer << endl;
}

bool
HTTP::formatConnection(const char *options)
{
    GNASH_REPORT_FUNCTION;
    _header << "Connection: " << options << endl;
}

bool
HTTP::formatContentType()
{
    return formatContentType(_filetype);
}

bool
HTTP::formatContentType(filetype_e filetype)
{
//    GNASH_REPORT_FUNCTION;
    
    switch (filetype) {
      case HTML:
	  _header << "Content-Type: text/html; charset=UTF-8" << endl;
	  break;
      case SWF:
	  _header << "Content-Type: application/x-shockwave-flash" << endl;
//	  _header << "Content-Type: application/futuresplash" << endl;
	  break;
      case VIDEO:
	  _header << "Content-Type: video/flv" << endl;
	  break;
      case MP3:
	  _header << "Content-Type: audio/mpeg" << endl;
	  break;
      default:
	  _header << "Content-Type: text/html; charset=UTF-8" << endl;
    }
}

bool
HTTP::formatContentLength()
{
//    GNASH_REPORT_FUNCTION;
    _header << "Content-Length: " << _filesize << endl;
}

bool
HTTP::formatContentLength(int filesize)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Content-Length: " << filesize << endl;
}

bool
HTTP::formatHost(const char *host)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Host: " << host << endl;
}

bool
HTTP::formatAgent(const char *agent)
{
//    GNASH_REPORT_FUNCTION;
    _header << "User-Agent: " << agent << endl;
}

bool
HTTP::formatLanguage(const char *lang)
{
//    GNASH_REPORT_FUNCTION;

    // For some browsers this appears to also be Content-Language
    _header << "Accept-Language: " << lang << endl;
}

bool
HTTP::formatCharset(const char *set)
{
    GNASH_REPORT_FUNCTION;
    // For some browsers this appears to also be Content-Charset
    _header << "Accept-Charset: " << set << endl;
}

bool
HTTP::formatEncoding(const char *code)
{
    GNASH_REPORT_FUNCTION;
    _header << "Accept-Encoding: " << code << endl;
}

bool
HTTP::formatTE(const char *te)
{
    GNASH_REPORT_FUNCTION;
    _header << "TE: " << te << endl;
}

bool
HTTP::sendGetReply(http_status_e code)
{
    GNASH_REPORT_FUNCTION;
    
    formatHeader(_filesize, HTML);
    int ret = writeNet(_header.str().c_str(), _header.str().size());
    if ( _body.str().size() > 0) {
	ret += writeNet(_body.str().c_str(), _body.str().size());
    }

    if (ret >= 0) {
        log_msg (_("Sent GET Reply"));
//        log_msg (_("Sent GET Reply: %s"), _header.str().c_str());
	clearHeader();
    } else {
        log_msg (_("Couldn't send GET Reply, writeNet returned %d"), ret);
	return false;
    }
//    cout << "GET Header is:" << endl << _header.str() << endl;
    return true; // Default to true
}

bool
HTTP::formatRequest(const char *url, http_method_e req)
{
    GNASH_REPORT_FUNCTION;

    _header.str("");

    _header << req << " " << url << "HTTP/1.1" << endl;
    _header << "User-Agent: Opera/9.01 (X11; Linux i686; U; en)" << endl;
    _header << "Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1" << endl;

    _header << "Accept-Language: en" << endl;
    _header << "Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1" << endl;
    
    _header << "Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0" << endl;
    _header << "Referer: " << url << endl;

    _header << "Connection: Keep-Alive, TE" << endl;
    _header << "TE: deflate, gzip, chunked, identity, trailers" << endl;
    
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
int
HTTP::extractAccept(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end, length, pos;
    string pattern = "Accept: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return -1;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	end = body.find("\n", start);
//	    return "error";
    }

    length = end-start-pattern.size();
    start = start+pattern.size();
    pos = start;
    while (pos <= end) {
	pos = (body.find(",", start) + 2);
	if (pos <= start) {
	    return _encoding.size();
	}
	if ((pos == string::npos) || (pos > end)) {
	    length = end - start;
	} else {
	    length = pos - start - 2;
	}
	string substr = body.substr(start, length);
//	printf("FIXME: \"%s\"\n", substr.c_str());
	_accept.push_back(substr);
	start = pos;
    }

    return _accept.size();
}

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

int
HTTP::extractConnection(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end, length, pos;
    string pattern = "Connection: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return -1;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	end = body.find("\n", start);
//	    return "error";
    }

    length = end-start-pattern.size();
    start = start+pattern.size();
    string _connection = body.substr(start, length);
    pos = start;
    while (pos <= end) {
	pos = (body.find(",", start) + 2);
	if (pos <= start) {
	    return _encoding.size();
	}
	if ((pos == string::npos) || (pos > end)) {
	    length = end - start;
	} else {
	    length = pos - start - 2;
	}
	string substr = body.substr(start, length);
//	printf("FIXME: \"%s\"\n", substr.c_str());
	_connections.push_back(substr);
	if (substr == "Keep-Alive") {
	    _keepalive = true;
	}
	start = pos;
    }

    return _connections.size();
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

int
HTTP::extractLanguage(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end, length, pos, terminate;
    // match both Accept-Language and Content-Language
    string pattern = "-Language: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return -1;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	end = body.find("\n", start);
//        return "error";
    }
    length = end-start-pattern.size();
    start = start+pattern.size();
    pos = start;
    terminate = (body.find(";", start));
    if (terminate == string::npos) {
	terminate = end;
    }
    
    while (pos <= end) {
	pos = (body.find(",", start));
	if (pos <= start) {
	    return _encoding.size();
	}
	if ((pos == string::npos) || (pos >= terminate)) {
	    length = terminate - start;
	} else {
	    length = pos - start;
	}
	string substr = body.substr(start, length);
//	printf("FIXME: \"%s\"\n", substr.c_str());
	_language.push_back(substr);
	start = pos + 1;
    }
    
//    _language = body.substr(start+pattern.size(), end-start-1);
    return _language.size();
}

int
HTTP::extractCharset(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end, length, pos, terminate;
// match both Accept-Charset and Content-Charset
    string pattern = "-Charset: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return -1;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	end = body.find("\n", start);
//        return "error";
    }
    
    length = end-start-pattern.size();
    start = start+pattern.size();
    string _connection = body.substr(start, length);
    pos = start;
    terminate = (body.find(";", start));
    if (terminate == string::npos) {
	terminate = end;
    }
    while (pos <= end) {
	pos = (body.find(",", start) + 2);
	if (pos <= start) {
	    return _encoding.size();
	}
	if ((pos == string::npos) || (pos >= terminate)) {
	    length = terminate - start;
	} else {
	    length = pos - start - 2;
	}
	string substr = body.substr(start, length);
//	printf("FIXME: \"%s\"\n", substr.c_str());
	_charset.push_back(substr);
	start = pos;
    }
//    _charset = body.substr(start+pattern.size(), end-start-1);
    return _charset.size();
}

int
HTTP::extractEncoding(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end, length, pos, terminate;
    // match both Accept-Encoding and Content-Encoding
    string pattern = "-Encoding: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return -1;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	end = body.find("\n", start);
//        return "error";
    }
    
   length = end-start-pattern.size();
    start = start+pattern.size();
    string _connection = body.substr(start, length);
    pos = start;
    // Drop anything after a ';' character
    terminate = (body.find(";", start));
    if (terminate == string::npos) {
	terminate = end;
    }
    while (pos <= end) {
	pos = (body.find(",", start) + 2);
	if (pos <= start) {
	    return _encoding.size();
	}
	if ((pos == string::npos) || (pos >= terminate)) {
	    length = terminate - start;
	} else {
	    length = pos - start - 2;
	}
	string substr = body.substr(start, length);
//	printf("FIXME: \"%s\"\n", substr.c_str());
	_encoding.push_back(substr);
	start = pos;
    }

//    _encoding = body.substr(start+pattern.size(), end-start-1);
    return _encoding.size();
}

int
HTTP::extractTE(const char *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = data;
    string::size_type start, end, length, pos;
    string pattern = "TE: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return -1;
    }
    end = body.find("\r\n", start);
    if (end == string::npos) {
	end = body.find("\n", start);
//        return "error";
    }
    
    length = end-start-pattern.size();
    start = start+pattern.size();
    pos = start;
    while (pos <= end) {
	pos = (body.find(",", start));
	if (pos <= start) {
	    return _encoding.size();
	}
	if ((pos == string::npos) || (pos >= end)) {
	    length = end - start;
	} else {
	    length = pos - start;
	}
	string substr = body.substr(start, length);
//	printf("FIXME: \"%s\"\n", substr.c_str());
	_te.push_back(substr);
	start = pos + 2;
    }
    return _te.size();
}

// Get the file type, so we know how to set the
// Content-type in the header.
HTTP::filetype_e
HTTP::getFileStats(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;    
    bool try_again = true;
    string actual_filespec = filespec;
    struct stat st;

    while (try_again) {
	try_again = false;
//	cerr << "Trying to open " << actual_filespec << endl;
	if (stat(actual_filespec.c_str(), &st) == 0) {
	    // If it's a directory, then we emulate what apache
	    // does, which is to load the index.html file in that
	    // directry if it exists.
	    if (S_ISDIR(st.st_mode)) {
		log_msg("%s is a directory\n", actual_filespec.c_str());
		if (actual_filespec[actual_filespec.size()-1] != '/') {
		    actual_filespec += '/';
		}
		actual_filespec += "index.html";
		try_again = true;
		continue;
	    } else { 		// not a directory
		log_msg("%s is not a directory\n", actual_filespec.c_str());
		string::size_type pos;
		pos = filespec.rfind(".");
		if (pos != string::npos) {
		    string suffix = filespec.substr(pos, filespec.size());
		    if (suffix == "html") {
			_filetype = HTML;
			log_msg("HTML content found");
		    }
		    if (suffix == "swf") {
			_filetype = SWF;
			log_msg("SWF content found");
		    }
		    if (suffix == "flv") {
			_filetype = VIDEO;
			log_msg("FLV content found");
		    }
		    if (suffix == "mp3") {
			_filetype = AUDIO;
			log_msg("MP3 content found");
		    }
		}
	    }
	} else {
	    _filetype = HTTP::ERROR;
	} // end of stat()
    } // end of try_waiting

    _filesize = st.st_size;
    return _filetype;
}

void
HTTP::dump() {
    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
    vector<string>::iterator it;
    
    log_msg (_("==== The HTTP header breaks down as follows: ===="));
    log_msg (_("Filespec: %s"), _filespec.c_str());
    log_msg (_("URL: %s"), _url.c_str());
    log_msg (_("Version: %s"), _version.c_str());
    for (it = _accept.begin(); it != _accept.end(); it++) {
        log_msg("Accept param: \"%s\"", (*(it)).c_str());
    }
    log_msg (_("Method: %s"), _method.c_str());
    log_msg (_("Referer: %s"), _referer.c_str());
    log_msg (_("Connections:"));
    for (it = _connections.begin(); it != _connections.end(); it++) {
        log_msg("Connection param is: \"%s\"", (*(it)).c_str());
    }
    log_msg (_("Host: %s"), _host.c_str());
    log_msg (_("User Agent: %s"), _agent.c_str());
    for (it = _language.begin(); it != _language.end(); it++) {
        log_msg("Language param: \"%s\"", (*(it)).c_str());
    }
    for (it = _charset.begin(); it != _charset.end(); it++) {
        log_msg("Charset param: \"%s\"", (*(it)).c_str());
    }
    for (it = _encoding.begin(); it != _encoding.end(); it++) {
        log_msg("Encodings param: \"%s\"", (*(it)).c_str());
    }
    for (it = _te.begin(); it != _te.end(); it++) {
        log_msg("TE param: \"%s\"", (*(it)).c_str());
    }
    log_msg (_("==== ==== ===="));
}

} // end of cygnal namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
