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
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
//#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
//#include <boost/date_time/time_zone_base.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <unistd.h>

#include "amf.h"
#include "cque.h"
#include "http.h"
#include "log.h"
#include "network.h"
#include "handler.h"
#include "utility.h"
#include "buffer.h"
#include "diskstream.h"
#include "cache.h"

using namespace gnash;
using namespace std;

static boost::mutex stl_mutex;

namespace gnash
{

extern map<int, Handler *> handlers;

// FIXME, this seems too small to me.  --gnu
static const int readsize = 1024;

// max size of files to map enirely into the cache
static const size_t CACHE_LIMIT = 102400000;

static Cache& cache = Cache::getDefaultInstance();

HTTP::HTTP() 
    : _filetype(amf::AMF::FILETYPE_HTML),
      _filesize(0),
      _port(80),
      _keepalive(false),
      _handler(0),
      _clientid(0),
      _index(0),
      _max_requests(0)
{
//    GNASH_REPORT_FUNCTION;
//    struct status_codes *status = new struct status_codes;
    
//    _status_codes(CONTINUE, status);
}

HTTP::HTTP(Handler *hand) 
    : _filetype(amf::AMF::FILETYPE_HTML),
      _filesize(0),
      _port(80),
      _keepalive(false),
      _clientid(0),
      _index(0),
      _max_requests(0)
{
//    GNASH_REPORT_FUNCTION;
    _handler = hand;
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
    _clientid = 0;
    _index = 0;
    _max_requests = 0;
    
    return true;
}

HTTP &
HTTP::operator = (HTTP& /*obj*/)
{
    GNASH_REPORT_FUNCTION;
//    this = obj;
    // TODO: FIXME !
    return *this; 
}

bool
HTTP::processClientRequest()
{
    GNASH_REPORT_FUNCTION;
    
    boost::shared_ptr<amf::Buffer> buf(_que.peek());
    if (buf) {
	switch (extractCommand(buf->reference())) {
	  case HTTP::HTTP_GET:
	      return processGetRequest();
	      break;
	  case HTTP::HTTP_POST:
	      return processPostRequest();
	      break;
	  case HTTP::HTTP_HEAD:
	      break;
	  case HTTP::HTTP_CONNECT:
	      break;
	  case HTTP::HTTP_TRACE:
	      break;
	  case HTTP::HTTP_OPTIONS:
	      break;
	  case HTTP::HTTP_PUT:
	      break;
	  case HTTP::HTTP_DELETE:
	      break;
	  default:
	      break;
	}
    }
    
}

bool
HTTP::processGetRequest()
{
    GNASH_REPORT_FUNCTION;

//     Network::byte_t buffer[readsize+1];
//     const char *ptr = reinterpret_cast<const char *>(buffer);
//     memset(buffer, 0, readsize+1);
    
//    _handler->wait();
//    _handler->dump();

    cerr << "QUE = " << _que.size() << endl;

    if (_que.size() == 0) {
	return false;
    }
    
    boost::shared_ptr<amf::Buffer> buf(_que.pop());
//    cerr << "YYYYYYY: " << (char *)buf->reference() << endl;
//    cerr << hexify(buf->reference(), buf->size(), false) << endl;
    
    if (buf == 0) {
	log_debug("Que empty, net connection dropped for fd #%d", getFileFd());
	return false;
    }
    
    clearHeader();
    extractCommand(*buf);
    extractConnection(*buf);
    extractAccept(*buf);
    extractMethod(*buf);
    extractReferer(*buf);
    extractHost(*buf);
    extractAgent(*buf);
    extractLanguage(*buf);
    extractCharset(*buf);
    extractKeepAlive(*buf);
    extractEncoding(*buf);
    extractTE(*buf);
//    dump();

    _filespec = _url;

    if (!_url.empty()) {
	return true;
    }
    return false;
}

bool
HTTP::processPostRequest()
{
    GNASH_REPORT_FUNCTION;

    cerr << "QUE = " << _que.size() << endl;

    if (_que.size() == 0) {
	return false;
    }
    
    boost::shared_ptr<amf::Buffer> buf(_que.pop());
    if (buf == 0) {
	log_debug("Que empty, net connection dropped for fd #%d", getFileFd());
	return false;
    }
    
    clearHeader();
    extractCommand(*buf);
    extractAccept(*buf);
    extractMethod(*buf);
    extractReferer(*buf);
    extractHost(*buf);
    extractAgent(*buf);
    extractLanguage(*buf);
    extractCharset(*buf);
    extractConnection(*buf);
    extractKeepAlive(*buf);
    extractEncoding(*buf);
    extractTE(*buf);

    _filespec = _url;

    if (!_url.empty()) {
	return true;
    }
    return false;
}

bool
HTTP::startHeader()
{
//    GNASH_REPORT_FUNCTION;

    clearHeader();
    
    return true;
}

const stringstream &
HTTP::formatHeader(http_status_e type)
{
//    GNASH_REPORT_FUNCTION;

    formatHeader(_filesize, type);
    return _header;
}

const stringstream &
HTTP::formatCommon(const string &data)
{
    _header << data << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatHeader(int filesize, http_status_e /* type */)
{
//    GNASH_REPORT_FUNCTION;

    _header << "HTTP/1.0 200 OK" << "\r\n";
    formatDate();
    formatServer();
//     if (type == NONE) {
// 	formatConnection("close"); // this is the default for HTTP 1.1
//     }
//     _header << "Accept-Ranges: bytes" << "\r\n";
    formatLastModified();
    formatEtag("24103b9-1c54-ec8632c0"); // FIXME: borrowed from tcpdump
    formatAcceptRanges("bytes");
    formatContentLength(filesize);
    formatKeepAlive("timeout=15, max=100");
//    formatConnection("Keep-Alive");
    formatContentType(amf::AMF::FILETYPE_HTML);
    // All HTTP messages are followed by a blank line.
    terminateHeader();

    return _header;
}

const stringstream &
HTTP::formatErrorResponse(http_status_e code)
{
//    GNASH_REPORT_FUNCTION;

    // First build the message body, so we know how to set Content-Length
    _body << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">" << "\r\n";
    _body << "<html><head>" << "\r\n";
    _body << "<title>" << code << " Not Found</title>" << "\r\n";
    _body << "</head><body>" << "\r\n";
    _body << "<h1>Not Found</h1>" << "\r\n";
    _body << "<p>The requested URL " << _filespec << " was not found on this server.</p>" << "\r\n";
    _body << "<hr>" << "\r\n";
    _body << "<address>Cygnal (GNU/Linux) Server at localhost Port " << _port << " </address>" << "\r\n";
    _body << "</body></html>" << "\r\n";
    _body << "\r\n";

    // First build the header
    _header << "HTTP/1.1 " << code << " Not Found" << "\r\n";
    formatDate();
    formatServer();
    _filesize = _body.str().size();
    formatContentLength(_filesize);
    formatConnection("close");
    formatContentType(amf::AMF::FILETYPE_HTML);

    return _header;
}

const stringstream &
HTTP::formatDate()
{
//    GNASH_REPORT_FUNCTION;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    
//    cout <<  now.time_of_day() << "\r\n";
    
    boost::gregorian::date d(now.date());
//     boost::gregorian::date d(boost::gregorian::day_clock::local_day());
//     cout << boost::posix_time::to_simple_string(now) << "\r\n";
//     cout << d.day_of_week() << "\r\n";
//     cout << d.day() << "\r\n";
//     cout << d.year() << "\r\n";
//     cout << d.month() << "\r\n";
    
//    boost::date_time::time_zone_ptr zone(new posix_time_zone("MST"));
//    boost::date_time::time_zone_base b(now "MST");
//    cout << zone.dst_zone_abbrev() << "\r\n";

    _header << "Date: " << d.day_of_week();
    _header << ", " << d.day();
    _header << " "  << d.month();
    _header << " "  << d.year();
    _header << " "  << now.time_of_day();
    _header << " GMT" << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatServer()
{
//    GNASH_REPORT_FUNCTION;
    _header << "Server: Cygnal (GNU/Linux)" << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatServer(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Server: " << data << "\r\n";

    return _header;
}

#if 0
const stringstream &
HTTP::formatMethod(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Method: " << data << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatReferer(const string &refer)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Referer: " << refer << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatConnection(const string &options)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Connection: " << options << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatKeepAlive(const string &options)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Keep-Alive: " << options << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatHost(const string &host)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Host: " << host << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatAgent(const string &agent)
{
//    GNASH_REPORT_FUNCTION;
    _header << "User-Agent: " << agent << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatAcceptRanges(const string &range)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Accept-Ranges: " << range << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatEtag(const string &tag)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Etag: " << tag << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatLastModified(const string &date)
{
    _header << "Last-Modified: " << date << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatLanguage(const string &lang)
{
//    GNASH_REPORT_FUNCTION;

    // For some browsers this appears to also be Content-Language
    _header << "Accept-Language: " << lang << "\r\n";
    return _header;
}

const stringstream &
HTTP::formatCharset(const string &set)
{
//    GNASH_REPORT_FUNCTION;
    // For some browsers this appears to also be Content-Charset
    _header << "Accept-Charset: " << set << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatEncoding(const string &code)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Accept-Encoding: " << code << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatTE(const string &te)
{
//    GNASH_REPORT_FUNCTION;
    _header << "TE: " << te << "\r\n";

    return _header;
}

#endif

const stringstream &
HTTP::formatContentLength()
{
//    GNASH_REPORT_FUNCTION;
    _header << "Content-Length: " << _filesize << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatContentLength(int filesize)
{
//    GNASH_REPORT_FUNCTION;
    _header << "Content-Length: " << filesize << "\r\n";

    return _header;
}

const stringstream &
HTTP::formatContentType()
{
    return formatContentType(_filetype);
}

const stringstream &
HTTP::formatContentType(amf::AMF::filetype_e filetype)
{
//    GNASH_REPORT_FUNCTION;
    
    switch (filetype) {
      case amf::AMF::FILETYPE_HTML:
	  _header << "Content-Type: text/html" << "\r\n";
//	  _header << "Content-Type: text/html; charset=UTF-8" << "\r\n";
	  break;
      case amf::AMF::FILETYPE_SWF:
	  _header << "Content-Type: application/x-shockwave-flash" << "\r\n";
//	  _header << "Content-Type: application/futuresplash" << "\r\n";
	  break;
      case amf::AMF::FILETYPE_VIDEO:
	  _header << "Content-Type: video/flv" << "\r\n";
	  break;
      case amf::AMF::FILETYPE_MP3:
	  _header << "Content-Type: audio/mpeg" << "\r\n";
	  break;
      case amf::AMF::FILETYPE_FCS:
	  _header << "Content-Type: application/x-fcs" << "\r\n";
	  break;
      default:
	  _header << "Content-Type: text/html" << "\r\n";
//	  _header << "Content-Type: text/html; charset=UTF-8" << "\r\n";
    }

    return _header;
}

const stringstream &
HTTP::formatLastModified()
{
//    GNASH_REPORT_FUNCTION;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    stringstream date;
    
    boost::gregorian::date d(now.date());
    
    date << d.day_of_week();
    date << ", " << d.day();
    date << " "  << d.month();
    date << " "  << d.year();
    date << " "  << now.time_of_day();
    date << " GMT";

    return formatLastModified(date.str());
}

const stringstream &
HTTP::formatGetReply(http_status_e code)
{
    GNASH_REPORT_FUNCTION;
    
    formatHeader(_filesize, code);
    
//    int ret = Network::writeNet(_header.str());    
//    Network::byte_t *ptr = (Network::byte_t *)_body.str().c_str();
//     buf->copy(ptr, _body.str().size());
//    _handler->dump();

    if (_header.str().size()) {
        log_debug (_("Sent GET Reply"));
	return _header;
    } else {
	clearHeader();
	log_debug (_("Couldn't send GET Reply, no header data"));
    }    

    return _header;
}

const stringstream &
HTTP::formatPostReply(rtmpt_cmd_e /* code */)
{
    GNASH_REPORT_FUNCTION;

    _header << "HTTP/1.1 200 OK" << "\r\n";
    formatDate();
    formatServer();
    formatContentType(amf::AMF::FILETYPE_FCS);
    // All HTTP messages are followed by a blank line.
    terminateHeader();
    return _header;

#if 0
    formatHeader(_filesize, code);
    boost::shared_ptr<amf::Buffer> buf = new amf::Buffer;
    if (_header.str().size()) {
	buf->resize(_header.str().size());
	string str = _header.str();
	buf->copy(str);
	_handler->pushout(buf);
	_handler->notifyout();
        log_debug (_("Sent GET Reply"));
	return true; // Default to true
    } else {
	clearHeader();
	log_debug (_("Couldn't send POST Reply, no header data"));
    }
#endif

    return _header;
}

const stringstream &
HTTP::formatRequest(const string &url, http_method_e req)
{
//    GNASH_REPORT_FUNCTION;

    _header.str("");

    _header << req << " " << url << "HTTP/1.1" << "\r\n";
    _header << "User-Agent: Opera/9.01 (X11; Linux i686; U; en)" << "\r\n";
    _header << "Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1" << "\r\n";

    _header << "Accept-Language: en" << "\r\n";
    _header << "Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1" << "\r\n";
    
    _header << "Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0" << "\r\n";
    _header << "Referer: " << url << "\r\n";

//    _header << "Connection: Keep-Alive, TE" << "\r\n";
    _header << "TE: deflate, gzip, chunked, identity, trailers" << "\r\n";

    return _header;
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
HTTP::extractAccept(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
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

/// These methods extract data from an RTMPT message. RTMP is an
/// extension to HTTP that adds commands to manipulate the
/// connection's persistance.
//
/// The URL to be opened has the following form:
/// http://server/<comand>/[<client>/]<index>
/// <command>
///    denotes the RTMPT request type, "OPEN", "SEND", "IDLE", "CLOSE")
/// <client>
///    specifies the id of the client that performs the requests
///    (only sent for established sessions)
/// <index>
///    is a consecutive number that seems to be used to detect missing packages
HTTP::rtmpt_cmd_e
HTTP::extractRTMPT(gnash::Network::byte_t *data)
{
    GNASH_REPORT_FUNCTION;

    string body = reinterpret_cast<const char *>(data);
    string tmp, cid, indx;
    HTTP::rtmpt_cmd_e cmd;

    // force the case to make comparisons easier
    std::transform(body.begin(), body.end(), body.begin(), 
               (int(*)(int)) toupper);
    string::size_type start, end;

    // Extract the command first
    start = body.find("OPEN", 0);
    if (start != string::npos) {
        cmd = HTTP::OPEN;
    }
    start = body.find("SEND", 0);
    if (start != string::npos) {
        cmd = HTTP::SEND;
    }
    start = body.find("IDLE", 0);
    if (start != string::npos) {
        cmd = HTTP::IDLE;
    }
    start = body.find("CLOSE", 0);
    if (start != string::npos) {
        cmd = HTTP::CLOSE;
    }

    // Extract the optional client id
    start = body.find("/", start+1);
    if (start != string::npos) {
	end = body.find("/", start+1);
	if (end != string::npos) {
	    indx = body.substr(end, body.size());
	    cid = body.substr(start, (end-start));
	} else {
	    cid = body.substr(start, body.size());
	}
    }

    _index = strtol(indx.c_str(), NULL, 0);
    _clientid = strtol(cid.c_str(), NULL, 0);
    end =  body.find("\r\n", start);
//     if (end != string::npos) {
//         cmd = HTTP::CLOSE;
//     }

    return cmd;
}

HTTP::http_method_e
HTTP::extractCommand(gnash::Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;

    string body = reinterpret_cast<const char *>(data);
    HTTP::http_method_e cmd;

    // force the case to make comparisons easier
//     std::transform(body.begin(), body.end(), body.begin(), 
//                (int(*)(int)) toupper);
    string::size_type start; 

    // Extract the command
    start = body.find("GET", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_GET;
    }
    start = body.find("POST", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_POST;
    }
    start = body.find("HEAD", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_HEAD;
    }
    start = body.find("CONNECT", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_CONNECT;
    }
    start = body.find("TRACE", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_TRACE;
    }
    start = body.find("OPTIONS", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_OPTIONS;
    }
    start = body.find("PUT", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_PUT;
    }
    start = body.find("DELETE", 0);
    if (start != string::npos) {
        cmd = HTTP::HTTP_DELETE;
    }

    _command = cmd;
    return cmd;
}

string &
HTTP::extractAcceptRanges(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end;
    string pattern = "Accept-Ranges: ";
    start = body.find(pattern, 0);
    if (start == string::npos) {
        _acceptranges = "error";
        return _acceptranges;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        _acceptranges = "error";
        return _acceptranges;
    }
    
    _acceptranges = body.substr(start+pattern.size(), end-start-1);
    return _acceptranges;    
}

string &
HTTP::extractMethod(Network::byte_t *data)
{
    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end;
    int length;

    length = body.size();
    start = body.find(" ", 0);
    if (start == string::npos) {
        _method = "error";
        return _method;
    }
    _method = body.substr(0, start);
    end = body.find(" ", start+1);
    if (end == string::npos) {
        _method = "error";
        return _method;
    }
    _url = body.substr(start+1, end-start-1);

    // HTTP 1.0 doesn't support persistant connections by default.
    if (body.substr(end+6, 3) == "1.0") {
	log_debug("Disbling Keep Alive for default");
	_version = 1.0;
	_keepalive = false;
    }
    // HTTP 1.1 does support persistant connections by default.
    if (body.substr(end+6, 3) == "1.1") {
	log_debug("Enabling Keep Alive for default");
	_version = 1.1;
	_keepalive = true;
    }    

    log_debug("HTTP version is: HTTP %g", _version);
    end = _url.find("?", 0);
//    _filespec = _url.substr(start+1, end);
    return _method;
}

string &
HTTP::extractReferer(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end;
    string pattern = "Referer: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
	_referer = "error";
	return _referer;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	_referer = "error";
        return _referer;
    }
    
    _referer = body.substr(start+pattern.size(), end-start-1);
    return _referer;
}

int
HTTP::extractField(const std::string &name, gnash::Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end, length, pos;
    string pattern = name + ": ";
    
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
	// Opera uses upper case first letters, Firefox doesn't.
	if ((substr == "Keep-Alive") || (substr == "keep-alive")) {
	    log_debug("Got a Keep Alive in field!");
	    _keepalive = true;
	}
	start = pos;
    }

    return _fields[name].size();
}

int
HTTP::extractField(const std::string &name, amf::Buffer *data)
{
//    GNASH_REPORT_FUNCTION;
    return extractField(name, data->reference());
}

int
HTTP::extractConnection(Network::byte_t *data)
{
    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end, length, pos;
    string pattern = "Connection: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
	log_error("Couldn't match pattern %s to %s", pattern, body);
        return -1;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	end = body.size();
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
	// Opera uses upper case first letters, Firefox doesn't.
	if ((substr == "Keep-Alive") || (substr == "keep-alive")) {
	    log_debug("Keep Alive connection specified in header");
	    _keepalive = true;
	} else {
	    log_debug("Keep Alive connection not specified in header!");
	}
	
	start = pos;
    }

    return _connections.size();
}

// KeepAlive directive
// Syntax: (Apache 1.1) KeepAlive max-requests
// Default: (Apache 1.1) KeepAlive 5
// Syntax: (Apache 1.2) KeepAlive on|off
// Default: (Apache 1.2) KeepAlive On
// Context: server config
// Status: Core
// Compatibility: KeepAlive is only available in Apache 1.1 and later. 
bool
HTTP::extractKeepAlive(Network::byte_t *data)
{
    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end, length, pos;
    string pattern = "Keep-Alive: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return -1;
    }

    log_debug("Got a Keep Alive!");
    end =  body.find("\r\n", start);
    if (end == string::npos) {
	end = body.size();
//	    return "error";
    }

    length = end-start-pattern.size();
    start = start+pattern.size();
    string tmp = body.substr(start, length);

    std::transform(tmp.begin(), tmp.end(), tmp.begin(), 
               (int(*)(int)) toupper);
    if ((tmp[0] >= '0') && (tmp[0] <= '9')) {
	_max_requests = strtol(tmp.c_str(), NULL, 0);
	_keepalive = true;	// if we get this header setting, we want to keep alive	
    }    
    if (tmp == "ON") {
	_keepalive = true;	// if we get this header setting, we want to keep alive	
    }
    if (tmp == "OFF") {
	_keepalive = false;	// if we get this header setting, we want to not keep alive	
    }
    
    return _keepalive;
}

string &
HTTP::extractHost(Network::byte_t *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end;
    string pattern = "Host: ";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        _host = "error"; 
        return _host;
   }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        _host = "error"; 
        return _host;
    }
    
    _host = body.substr(start+pattern.size(), end-start-1);
    return _host;
}

string &
HTTP::extractAgent(Network::byte_t *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
    string::size_type start, end;
    string pattern = "User-Agent: ";
    _agent = "error";
    
    start = body.find(pattern, 0);
    if (start == string::npos) {
        return _agent;
    }
    end =  body.find("\r\n", start);
    if (end == string::npos) {
        return _agent;
    }
    
    _agent = body.substr(start+pattern.size(), end-start-1);
    return _agent;
}

int
HTTP::extractLanguage(Network::byte_t *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
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
HTTP::extractCharset(Network::byte_t *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
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
HTTP::extractEncoding(Network::byte_t *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
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
HTTP::extractTE(Network::byte_t *data) {
//    GNASH_REPORT_FUNCTION;
    
    string body = reinterpret_cast<const char *>(data);
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
amf::AMF::filetype_e

HTTP::getFileStats(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;    
    bool try_again = true;
    string actual_filespec = filespec;
    struct stat st;

    if (cache.findPath(filespec).empty()) {
	while (try_again) {
	    try_again = false;
//	cerr << "Trying to open " << actual_filespec << "\r\n";
	    if (stat(actual_filespec.c_str(), &st) == 0) {
		// If it's a directory, then we emulate what apache
		// does, which is to load the index.html file in that
		// directry if it exists.
		if (S_ISDIR(st.st_mode)) {
		    log_debug("%s is a directory\n", actual_filespec.c_str());
		    if (actual_filespec[actual_filespec.size()-1] != '/') {
			actual_filespec += '/';
		}
		    actual_filespec += "index.html";
		    try_again = true;
		    continue;
		} else { 		// not a directory
		    log_debug("%s is not a directory\n", actual_filespec.c_str());
		    _filespec = actual_filespec;
		    string::size_type pos;
		    pos = filespec.rfind(".");
		    if (pos != string::npos) {
			string suffix = filespec.substr(pos, filespec.size());
			if (suffix == "html") {
			    _filetype = amf::AMF::FILETYPE_HTML;
			    log_debug("HTML content found");
			}
			if (suffix == "swf") {
			    _filetype = amf::AMF::FILETYPE_SWF;
			    log_debug("SWF content found");
			}
			if (suffix == "flv") {
			    _filetype = amf::AMF::FILETYPE_VIDEO;
			    log_debug("FLV content found");
			}
			if (suffix == "mp3") {
			    _filetype = amf::AMF::FILETYPE_AUDIO;
			    log_debug("MP3 content found");
			}
		    }
		}
	    } else {
		_filetype = amf::AMF::FILETYPE_ERROR;
	    } // end of stat()
	} // end of try_waiting
	
	_filesize = st.st_size;
    }
    
    return _filetype;
}

/// \brief Send a message to the other end of the network connection.
///`	Sends the contents of the _header and _body private data to
///	the already opened network connection.
///
/// @return The number of bytes sent
int DSOEXPORT
HTTP::sendMsg()
{
    GNASH_REPORT_FUNCTION;
    
    return 0; // FIXME
}

/// \brief Send a message to the other end of the network connection.
///`	Sends the contents of the _header and _body private data to
///	the already opened network connection.
///
/// @param fd The file descriptor to use for writing to the network.
///
/// @return The number of bytes sent
int DSOEXPORT
HTTP::sendMsg(int fd)
{
    GNASH_REPORT_FUNCTION;
    
    return 0; // FIXME
}

/// \brief Send a message to the other end of the network connection.
///`	Sends the contents of the _header and _body private data to
///	the already opened network connection.
///
/// @param data A real pointer to the data.
/// @param size The number of bytes of data stored.
///
/// @return The number of bytes sent
int DSOEXPORT
HTTP::sendMsg(const Network::byte_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;
//    _header

    return Network::writeNet(data, size);
}

int
HTTP::recvMsg(int fd)
{
    GNASH_REPORT_FUNCTION;
    int ret = 0;
    
    log_debug("Starting to wait for data in net for fd #%d", fd);
    Network net;

    do {
	boost::shared_ptr<amf::Buffer> buf(new amf::Buffer);
	int ret = net.readNet(fd, buf, 5);

	cerr << __PRETTY_FUNCTION__ << endl << (char *)buf->reference() << endl;
	
	// the read timed out as there was no data, but the socket is still open.
 	if (ret == 0) {
	    log_debug("no data yet for fd #%d, continuing...", fd);
 	    continue;
 	}
	// ret is "no position" when the socket is closed from the other end of the connection,
	// so we're done.
	if ((ret == string::npos) || (ret == 0xffffffff)) {
	    log_debug("socket for fd #%d was closed...", fd);
	    break;
	}
	// We got data. Resize the buffer if necessary.
	if (ret > 0) {
//	    cerr << "XXXXX: " << (char *)buf->reference() << endl;
 	    if (ret < NETBUFSIZE) {
// 		buf->resize(ret);	FIXME: why does this corrupt
// 		the buffer ?
		_que.push(buf);
		break;
 	    } else {
		_que.push(buf);
	    }
	} else {
	    log_debug("no more data for fd #%d, exiting...", fd);
	    break;
	}
    } while (ret);
    
    // We're done. Notify the other threads the socket is closed, and tell them to die.
    log_debug("Handler done for fd #%d...", fd);

    return _que.size();
}

void
HTTP::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
    vector<string>::iterator it;
    
    log_debug (_("==== The HTTP header breaks down as follows: ===="));
    log_debug (_("Filespec: %s"), _filespec.c_str());
    log_debug (_("URL: %s"), _url.c_str());
    log_debug (_("Version: %g"), _version);
    for (it = _accept.begin(); it != _accept.end(); it++) {
        log_debug("Accept param: \"%s\"", (*(it)).c_str());
    }
    log_debug (_("Method: %s"), _method.c_str());
    log_debug (_("Referer: %s"), _referer.c_str());
    log_debug (_("Connections:"));
    for (it = _connections.begin(); it != _connections.end(); it++) {
        log_debug("Connection param is: \"%s\"", (*(it)).c_str());
    }
    log_debug (_("Host: %s"), _host.c_str());
    log_debug (_("User Agent: %s"), _agent.c_str());
    for (it = _language.begin(); it != _language.end(); it++) {
        log_debug("Language param: \"%s\"", (*(it)).c_str());
    }
    for (it = _charset.begin(); it != _charset.end(); it++) {
        log_debug("Charset param: \"%s\"", (*(it)).c_str());
    }
    for (it = _encoding.begin(); it != _encoding.end(); it++) {
        log_debug("Encodings param: \"%s\"", (*(it)).c_str());
    }
    for (it = _te.begin(); it != _te.end(); it++) {
        log_debug("TE param: \"%s\"", (*(it)).c_str());
    }

    // Dump the RTMPT fields
    log_debug("RTMPT optional index is: ", _index);
    log_debug("RTMPT optional client ID is: ", _clientid);
    log_debug (_("==== ==== ===="));
}

extern "C" {
void
http_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
//    struct thread_params thread_data;
    string url, filespec, parameters;
    string::size_type pos;
    Network *net = reinterpret_cast<Network *>(args->handler);
    HTTP www;
    bool done = false;
//    www.setHandler(net);

    log_debug(_("Starting HTTP Handler for fd #%d, tid %ld"),
	      args->netfd, get_thread_id());
    
    string docroot = args->filespec;
    
    log_debug("Starting to wait for data in net for fd #%d", args->netfd);

    // Wait for data, and when we get it, process it.
    do {
#ifdef THREADED_IO
	hand->wait();
	if (hand->timetodie()) {
	    log_debug("Not waiting no more, no more for more HTTP data for fd #%d...", args->netfd);
	    map<int, Handler *>::iterator hit = handlers.find(args->netfd);
	    if ((*hit).second) {
		log_debug("Removing handle %x for HTTP on fd #%d",
			  (void *)hand, args->netfd);
		handlers.erase(args->netfd);
		delete (*hit).second;
	    }
	    return;
	}
#endif
	
#ifdef USE_STATISTICS
	struct timespec start;
	clock_gettime (CLOCK_REALTIME, &start);
#endif
	
// 	conndata->statistics->setFileType(NetStats::RTMPT);
// 	conndata->statistics->startClock();
//	args->netfd = www.getFileFd();
//	www.recvMsg(5);
	www.recvMsg(args->netfd);
	
	if (!www.processGetRequest()) {
//	    hand->die();	// tell all the threads for this connection to die
//	    hand->notifyin();
	    log_debug("Net HTTP done for fd #%d...", args->netfd);
// 	    hand->closeNet(args->netfd);
	    return;
	}
	url = docroot;
	url += www.getURL();
	pos = url.find("?");
	filespec = url.substr(0, pos);
	parameters = url.substr(pos + 1, url.size());

	if (cache.findPath(www.getFilespec()).empty()) {
	    cache.addPath(www.getFilespec(), filespec);
	
	    // Get the file size for the HTTP header
	    if (www.getFileStats(filespec) == amf::AMF::FILETYPE_ERROR) {
		www.formatErrorResponse(HTTP::NOT_FOUND);
	    }
	}
	// Send the reply
//	www.formatGetReply(HTTP::LIFE_IS_GOOD);
//	cerr << "Size = " << www.getHeader().size() << "	" << www.getHeader() << endl;
	
//	www.writeNet(args->netfd, (boost::uint8_t *)www.getHeader().c_str(), www.getHeader().size());
//	hand->writeNet(args->netfd, www.getHeader(), www.getHeader().size());
//	strcpy(thread_data.filespec, filespec.c_str());
//	thread_data.statistics = conndata->statistics;
	
	// Keep track of the network statistics
//	conndata->statistics->stopClock();
// 	log_debug (_("Bytes read: %d"), www.getBytesIn());
// 	log_debug (_("Bytes written: %d"), www.getBytesOut());
//	st.setBytes(www.getBytesIn() + www.getBytesOut());
//	conndata->statistics->addStats();

	if (filespec[filespec.size()-1] == '/') {
	    filespec += "index.html";
	}

	// See if the file is in the cache and already opened.
	boost::shared_ptr<DiskStream> filestream(cache.findFile(www.getFilespec()));
	if (filestream) {
	    cerr << "FIXME: found file in cache!" << endl;
	} else {
	  filestream.reset(new DiskStream);
	}
//	cerr << "New Filestream at 0x" << hex << filestream.get() << endl;
	
	// Oopen the file and read the furst chunk into memory
 	filestream->open(filespec);
	string response = cache.findResponse(www.getFilespec());
	if (response.empty()) {
	    cerr << "FIXME no hit for: " << www.getFilespec() << endl;
	    www.clearHeader();
	    const stringstream &ss = www.formatHeader(filestream->getFileSize(), HTTP::LIFE_IS_GOOD);
	    www.writeNet(args->netfd, (boost::uint8_t *)www.getHeader().c_str(), www.getHeader().size());
	    cache.addResponse(www.getFilespec(), www.getHeader());
	} else {
	    cerr << "FIXME hit on: " << www.getFilespec() << endl;
	    www.writeNet(args->netfd, (boost::uint8_t *)response.c_str(), response.size());
	}	

//	cerr << www.getHeader().c_str() << endl;

	size_t filesize = filestream->getFileSize();
	size_t bytes_read = 0;
	int ret;
	size_t page = 0;
	if (filesize) {
#ifdef USE_STATS_CACHE
	    struct timespec start;
	    clock_gettime (CLOCK_REALTIME, &start);
#endif
	    size_t getbytes = 0;
	    if (filesize <= filestream->getPagesize()) {
		getbytes = filesize;
	    } else {
		getbytes = filestream->getPagesize();
	    }
	    if (filesize >= CACHE_LIMIT) {
		do {
		    boost::uint8_t *ptr = filestream->loadChunk(page);
		    ret = www.writeNet(args->netfd, filestream->get(), getbytes);
		    if (ret <= 0) {
			break;
		    }
		    bytes_read += ret;
		page += filestream->getPagesize();
		} while (bytes_read <= filesize);
	    } else {
		boost::uint8_t *ptr = filestream->loadChunk(filesize, 0);
//		filestream->close();
		ret = www.writeNet(args->netfd, filestream->get(), filesize);
	    }
	    filestream->close();
#ifdef USE_STATS_CACHE
	    struct timespec end;
	    clock_gettime (CLOCK_REALTIME, &end);
	    double time = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9);
	    cerr << "File " << www.getFilespec()
		 << " transferred " << filesize << " bytes in: " << fixed
		 << time << " seconds." << endl;
#endif
//	    filestream->close();
	    cache.addFile(www.getFilespec(), filestream);
	}
	log_debug("http_handler all done transferring requested file...");
//	cache.dump();
	done = true;

	if (!www.keepAlive()) {
	    log_debug("Keep-Alive is off", www.keepAlive());
//	    www.closeConnection();
	    done = true;
	}
#if 0
	if (url != docroot) {
	    log_debug (_("File to load is: %s"), filespec.c_str());
	    log_debug (_("Parameters are: %s"), parameters.c_str());
	    struct stat st;
	    int filefd;
	    size_t ret;
	    if (stat(filespec.c_str(), &st) == 0) {
		filefd = ::open(filespec.c_str(), O_RDONLY);
		log_debug (_("File \"%s\" is %lld bytes in size, disk fd #%d"), filespec,
			   st.st_size, filefd);
		boost::shared_ptr<amf::Buffer> buf(new amf::Buffer);
		log_debug("Done transferring %s to net fd #%d", filespec, args->netfd);

		// See if this is a persistant connection
// 		if (!www.keepAlive()) {
// 		    log_debug("Keep-Alive is off", www.keepAlive());
		hand->closeConnection();
//  		}
	    }
	}
#endif


	
#ifdef USE_STATISTICS
	struct timespec end;
	clock_gettime (CLOCK_REALTIME, &end);
	log_debug("Processing time for GET request was %f seconds",
		  (float)((end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9)));
#endif
//	conndata->statistics->dump();
//    }
//    } while(!hand->timetodie());
    } while(done != true);
    
    log_debug("http_handler all done now finally...");
    
} // end of httphandler
    
} // end of extern C

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
