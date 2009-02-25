// http.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
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
#include "GnashSystemIOHeaders.h" // read()

#include "amf.h"
#include "element.h"
#include "cque.h"
#include "http.h"
#include "log.h"
#include "network.h"
#include "handler.h"
#include "utility.h"
#include "buffer.h"
#include "diskstream.h"
#include "cache.h"

// Not POSIX, so best not rely on it if possible.
#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

#if defined(_WIN32) || defined(WIN32)
# define __PRETTY_FUNCTION__ __FUNCDNAME__
# include <winsock2.h>
# include <direct.h>
#else
# include <unistd.h>
# include <sys/param.h>
#endif

using namespace gnash;
using namespace std;

static boost::mutex stl_mutex;

namespace gnash
{

extern map<int, Handler *> handlers;

// FIXME, this seems too small to me.  --gnu
static const int readsize = 1024;

static Cache& cache = Cache::getDefaultInstance();

HTTP::HTTP() 
    : _filetype(DiskStream::FILETYPE_HTML),
      _filesize(0),
      _keepalive(false),
      _handler(0),
      _clientid(0),
      _index(0),
      _max_requests(0)
{
//    GNASH_REPORT_FUNCTION;
//    struct status_codes *status = new struct status_codes;
    _version.major = 0;
    _version.minor = 0;
    
//    _status_codes(CONTINUE, status);
}

HTTP::HTTP(Handler *hand) 
    : _filetype(DiskStream::FILETYPE_HTML),
      _filesize(0),
      _keepalive(false),
      _clientid(0),
      _index(0),
      _max_requests(0)
{
//    GNASH_REPORT_FUNCTION;
    _handler = hand;
    _version.major = 0;
    _version.minor = 0;
}

HTTP::~HTTP()
{
//    GNASH_REPORT_FUNCTION;
}

bool
HTTP::clearHeader()
{
//     _header.str("");
    _buffer.clear();
    _filesize = 0;
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

HTTP::http_method_e
HTTP::processClientRequest(int fd)
{
//    GNASH_REPORT_FUNCTION;
    bool result = false;
    
    boost::shared_ptr<amf::Buffer> buf(_que.peek());
    if (buf) {
	_cmd = extractCommand(buf->reference());
	switch (_cmd) {
	  case HTTP::HTTP_GET:
	      result = processGetRequest(fd);
	      break;
	  case HTTP::HTTP_POST:
	      result = processPostRequest(fd);
	      break;
	  case HTTP::HTTP_HEAD:
	      result = processHeadRequest(fd);
	      break;
	  case HTTP::HTTP_CONNECT:
	      result = processConnectRequest(fd);
	      break;
	  case HTTP::HTTP_TRACE:
	      result = processTraceRequest(fd);
	      break;
	  case HTTP::HTTP_OPTIONS:
	      result = processOptionsRequest(fd);
	      break;
	  case HTTP::HTTP_PUT:
	      result = processPutRequest(fd);
	      break;
	  case HTTP::HTTP_DELETE:
	      result = processDeleteRequest(fd);
	      break;
	  default:
	      break;
	}
    }

    if (result) {
	return _cmd;
    } else {
	return HTTP::HTTP_NONE;
   }
}

// A GET request asks the server to send a file to the client
bool
HTTP::processGetRequest(int fd)
{
    GNASH_REPORT_FUNCTION;

//     boost::uint8_t buffer[readsize+1];
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
//    cerr << hexify(buf->reference(), buf->allocated(), false) << endl;
    
    if (buf == 0) {
     //	log_debug("Que empty, net connection dropped for fd #%d", getFileFd());
	log_debug("Que empty, net connection dropped for fd #%d", fd);
	return false;
    }
    
    clearHeader();
    processHeaderFields(*buf);

    string url = _docroot + _filespec;
    // See if the file is in the cache and already opened.
    boost::shared_ptr<DiskStream> filestream(cache.findFile(url));
    if (filestream) {
	cerr << "FIXME: found file in cache!" << endl;
    } else {
	filestream.reset(new DiskStream);
//	    cerr << "New Filestream at 0x" << hex << filestream.get() << endl;
	
//	    cache.addFile(url, filestream);	FIXME: always reload from disk for now.
	
	// Oopen the file and read the first chunk into memory
	if (filestream->open(url)) {
	    formatErrorResponse(HTTP::NOT_FOUND);
	} else {
	    // Get the file size for the HTTP header
	    if (filestream->getFileType() == DiskStream::FILETYPE_NONE) {
		formatErrorResponse(HTTP::NOT_FOUND);
	    } else {
		cache.addPath(_filespec, filestream->getFilespec());
	    }
	}
    }
    
    // Send the reply
    amf::Buffer &reply = formatHeader(filestream->getFileType(),
					  filestream->getFileSize(),
					  HTTP::OK);
    writeNet(fd, reply);

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
		filestream->loadToMem(page);
		ret = writeNet(fd, filestream->get(), getbytes);
		if (ret <= 0) {
		    break;
		}
		bytes_read += ret;
		page += filestream->getPagesize();
	    } while (bytes_read <= filesize);
	} else {
	    filestream->loadToMem(filesize, 0);
	    ret = writeNet(fd, filestream->get(), filesize);
	}
	filestream->close();
#ifdef USE_STATS_CACHE
	struct timespec end;
	clock_gettime (CLOCK_REALTIME, &end);
	double time = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9);
	cerr << "File " << _filespec
	     << " transferred " << filesize << " bytes in: " << fixed
	     << time << " seconds for net fd #" << fd << endl;
#endif
    }

    log_debug("http_handler all done transferring requested file \"%s\".", _filespec);
    
    return true;
}

// A POST request asks sends a data from the client to the server. After processing
// the header like we normally do, we then read the amount of bytes specified by
// the "content-length" field, and then write that data to disk, or decode the amf.
bool
HTTP::processPostRequest(int fd)
{
    GNASH_REPORT_FUNCTION;

//    cerr << "QUE1 = " << _que.size() << endl;

    if (_que.size() == 0) {
	return false;
    }
    
    boost::shared_ptr<amf::Buffer> buf(_que.pop());
    if (buf == 0) {
	log_debug("Que empty, net connection dropped for fd #%d", getFileFd());
	return false;
    }
//    cerr << __FUNCTION__ << buf->allocated() << " : " << hexify(buf->reference(), buf->allocated(), true) << endl;
    
    clearHeader();
    boost::uint8_t *data = processHeaderFields(*buf);
    size_t length = strtol(getField("content-length").c_str(), NULL, 0);
    boost::shared_ptr<amf::Buffer> content(new amf::Buffer(length));
    int ret = 0;
    if (buf->allocated() - (data - buf->reference()) ) {
//	cerr << "Don't need to read more data: have " << buf->allocated() << " bytes" << endl;
	content->copy(data, length);
	ret = length;
    } else {	
//	cerr << "Need to read more data, only have "  << buf->allocated() << " bytes" << endl;
	ret = readNet(fd, *content, 2);
	data = content->reference();
    }    
    
    if (getField("content-type") == "application/x-www-form-urlencoded") {
	log_debug("Got file data in POST");
	string url = _docroot + _filespec;
	DiskStream ds(url, *content);
	ds.writeToDisk();
//    ds.close();
	// oh boy, we got ourselves some encoded AMF objects instead of a boring file.
    } else if (getField("content-type") == "application/x-amf") {
	log_debug("Got AMF data in POST");
#if 0
	amf::AMF amf;
	boost::shared_ptr<amf::Element> el = amf.extractAMF(content.reference(), content.end());
	el->dump();		// FIXME: do something intelligent
				// with this Element
#endif
    }
    
    // Send the reply

    // NOTE: this is a "special" path we trap until we have real CGI support
    if ((_filespec == "/echo/gateway")
	&& (getField("content-type") == "application/x-amf")) {
//	const char *num = (const char *)buf->at(10);
	log_debug("Got CGI echo request in POST");
//	cerr << "FIXME 2: " << hexify(content->reference(), content->allocated(), true) << endl;

	vector<boost::shared_ptr<amf::Element> > headers = parseEchoRequest(*content);
  	//boost::shared_ptr<amf::Element> &el0 = headers[0];
  	//boost::shared_ptr<amf::Element> &el1 = headers[1];
  	//boost::shared_ptr<amf::Element> &el3 = headers[3];
	
    if (headers.size() >= 4) {
	    if (headers[3]) {
		amf::Buffer &reply = formatEchoResponse(headers[1]->getName(), *headers[3]);
// 	    cerr << "FIXME 3: " << hexify(reply.reference(), reply.allocated(), true) << endl;
// 	    cerr << "FIXME 3: " << hexify(reply.reference(), reply.allocated(), false) << endl;
		writeNet(fd, reply);
	    }
 	}
    } else {
	amf::Buffer &reply = formatHeader(_filetype, _filesize, HTTP::OK);
	writeNet(fd, reply);
    }

    return true;
}

bool
HTTP::processPutRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("PUT request");

    return false;
}

bool
HTTP::processDeleteRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("DELETE request");
    return false;
}

bool
HTTP::processConnectRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("CONNECT request");
    return false;
}

bool
HTTP::processOptionsRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("OPTIONS request");
    return false;
}

bool
HTTP::processHeadRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("HEAD request");
    return false;
}

bool
HTTP::processTraceRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl("TRACE request");
    return false;
}

// http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec5 (5.3 Request Header Fields)
bool
HTTP::checkRequestFields(amf::Buffer & /* buf */)
{
//    GNASH_REPORT_FUNCTION;

#if 0
    const char *foo[] = {
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Authorization",
	"Expect",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Max-Forwards",
	"Proxy-Authorization",
	"Range",
	"Referer",
	"TE",
	"User-Agent",
	0
	};
#endif
    return false;
}

// http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec7 (7.1 Entity Header Fields)
bool
HTTP::checkEntityFields(amf::Buffer & /* buf */)
{
//    GNASH_REPORT_FUNCTION;

#if 0
    const char *foo[] = {
	"Accept",
	"Allow",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",	// Must be used when sending a Response
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",		// Must be used when sending non text/html files
	"Expires",
	"Last-Modified",
	0
	};
    
    return true;
#endif
    return false;
}

// http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec4 (4.5 General Header Fields)
bool
HTTP::checkGeneralFields(amf::Buffer & /* buf */)
{
//    GNASH_REPORT_FUNCTION;

#if 0
    const char *foo[] = {
	"Cache-Control"
	"Connection",		// Must look for Keep-Alive and close
	"Date",
	"Pragma",
	"Trailer",
	"Transfer-Encoding",	// Must look for Chunked-Body too
	"Upgrade",
	"Via",
	"Warning",
	0
	};
#endif
    return false;
}

boost::uint8_t *
HTTP::processHeaderFields(amf::Buffer &buf)
{
  //    GNASH_REPORT_FUNCTION;
    string head(reinterpret_cast<const char *>(buf.reference()));

    // The end of the header block is always followed by a blank line
    string::size_type end = head.find("\r\n\r\n", 0);
//    head.erase(end, buf.size()-end);
    Tok t(head, Sep("\r\n"));
    for (Tok::iterator i = t.begin(); i != t.end(); ++i) {
	string::size_type pos = i->find(":", 0);
 	if (pos != string::npos) {
	    string name = i->substr(0, pos);
	    string value = i->substr(pos+2, i->size());
 	    std::transform(name.begin(), name.end(), name.begin(), 
 			   (int(*)(int)) tolower);
 	    std::transform(value.begin(), value.end(), value.begin(), 
 			   (int(*)(int)) tolower);
 	    _fields[name] = value;
	    if (name == "keep-alive") {
		_keepalive = true;
		if ((value != "on") && (value != "off")) {
		    _max_requests = strtol(value.c_str(), NULL, 0);
		    log_debug("Setting Max Requests for Keep-Alive to %d", _max_requests);
		}
	    }
	    if (name == "connection") {
		if (value.find("keep-alive", 0) != string::npos) {
		    _keepalive = true;
		}
	    }
	    if (name == "content-length") {
		_filesize = strtol(value.c_str(), NULL, 0);
		log_debug("Setting Content Length to %d", _filesize);
	    }
	    if (name == "content-type") {
		// This is the type used by flash when sending a AMF data via POST
		if (value == "application/x-amf") {
//		    log_debug("Got AMF data in the POST request!");
		    _filetype = DiskStream::FILETYPE_AMF;
		}
		// This is the type used by wget when sending a file via POST
		if (value == "application/x-www-form-urlencoded") {
//		    log_debug("Got file data in the POST request");
		    _filetype = DiskStream::FILETYPE_ENCODED;
		}
		log_debug("Setting Content Type to %d", _filetype);
	    }
	    
//	    cerr << "FIXME: " << (void *)i << " : " << dec <<  end << endl;
	} else {
	    const boost::uint8_t *cmd = reinterpret_cast<const boost::uint8_t *>(i->c_str());
	    if (extractCommand(const_cast<boost::uint8_t *>(cmd)) == HTTP::HTTP_NONE) {
		break;
#if 1
	    } else {
		log_debug("Got a request, parsing \"%s\"", *i);
		string::size_type start = i->find(" ");
		string::size_type params = i->find("?");
		string::size_type pos = i->find("HTTP/");
		if (pos != string::npos) {
		    // The version is the last field and is the protocol name
		    // followed by a slash, and the version number. Note that
		    // the version is not a double, even though it has a dot
		    // in it. It's actually two separate integers.
		    _version.major = i->at(pos+5) - '0';
		    _version.minor = i->at(pos+7) - '0';
		    log_debug (_("Version: %d.%d"), _version.major, _version.minor);
		    // the filespec in the request is the middle field, deliminated
		    // by a space on each end.
		    if (params != string::npos) {
			_params = i->substr(params+1, end);
			_filespec = i->substr(start+1, params);
			log_debug("Parameters for file: \"%s\"", _params);
		    } else {
			_filespec = i->substr(start+1, pos-start-2);
		    }
		    log_debug("Requesting file: \"%s\"", _filespec);

		    // HTTP 1.1 enables persistant network connections
		    // by default.
		    if (_version.minor > 0) {
			log_debug("Enabling Keep Alive by default for HTTP > 1.0");
			_keepalive = true;
		    }
		}
	    }
#endif
	}
    }
    
    return buf.reference() + end + 4;
}

boost::shared_ptr<std::vector<std::string> >
HTTP::getFieldItem(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<std::vector<std::string> > ptr(new std::vector<std::string>);
    Tok t(_fields[name], Sep(", "));
    for (Tok::iterator i = t.begin(), e = t.end(); i != e; ++i) {
	ptr->push_back(*i);
    }

    return ptr;
}

bool
HTTP::startHeader()
{
//    GNASH_REPORT_FUNCTION;

    clearHeader();
    
    return true;
}

amf::Buffer &
HTTP::formatHeader(http_status_e type)
{
//    GNASH_REPORT_FUNCTION;

    return formatHeader(_filesize, type);
}

amf::Buffer &
HTTP::formatCommon(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    _buffer += data;
    _buffer += "\r\n";

    return _buffer;
}

amf::Buffer &
HTTP::formatHeader(size_t size, http_status_e code)
{
//    GNASH_REPORT_FUNCTION;
  return formatHeader(_filetype, size, code);
}

amf::Buffer &
HTTP::formatHeader(DiskStream::filetype_e type, size_t size, http_status_e code)
{
//    GNASH_REPORT_FUNCTION;

    clearHeader();

    char num[12];

    _buffer = "HTTP/";
    sprintf(num, "%d.%d", _version.major, _version.minor);
    _buffer += num;
    sprintf(num, " %d ", static_cast<int>(code));
    _buffer += num;
    switch (code) {
      case CONTINUE:
	  _buffer += "Continue";
	  break;
      case SWITCHPROTOCOLS:
	  _buffer += "Switch Protocols";
	  break;
	  // 2xx: Success - The action was successfully received,
	  // understood, and accepted
	  break;
      case OK:
	  _buffer += "OK";
	  break;
      case CREATED:
	  _buffer += "Created";
	  break;
      case ACCEPTED:
	  _buffer += "Accepted";
	  break;
      case NON_AUTHORITATIVE:
	  _buffer += "Non Authoritive";
	  break;
      case NO_CONTENT:
	  _buffer += "No Content";
	  break;
      case RESET_CONTENT:
	  _buffer += "Reset Content";
	  break;
      case PARTIAL_CONTENT:
	  _buffer += "Partial Content";
	  break;
        // 3xx: Redirection - Further action must be taken in order to
        // complete the request
      case MULTIPLE_CHOICES:
	  _buffer += "Multiple Choices";
	  break;
      case MOVED_PERMANENTLY:
	  _buffer += "Moved Permanently";
	  break;
      case FOUND:
	  _buffer += "Found";
	  break;
      case SEE_OTHER:
	  _buffer += "See Other";
	  break;
      case NOT_MODIFIED:
	  _buffer += "Not Modified";
	  break;
      case USE_PROXY:
	  _buffer += "Use Proxy";
	  break;
      case TEMPORARY_REDIRECT:
	  _buffer += "Temporary Redirect";
	  break;
        // 4xx: Client Error - The request contains bad syntax or
        // cannot be fulfilled
      case BAD_REQUEST:
	  _buffer += "Bad Request";
	  break;
      case UNAUTHORIZED:
	  _buffer += "Unauthorized";
	  break;
      case PAYMENT_REQUIRED:
	  _buffer += "Payment Required";
	  break;
      case FORBIDDEN:
	  _buffer += "Forbidden";
	  break;
      case NOT_FOUND:
	  _buffer += "Not Found";
	  break;
      case METHOD_NOT_ALLOWED:
	  _buffer += "Method Not Allowed";
	  break;
      case NOT_ACCEPTABLE:
	  _buffer += "Not Acceptable";
	  break;
      case PROXY_AUTHENTICATION_REQUIRED:
	  _buffer += "Proxy Authentication Required";
	  break;
      case REQUEST_TIMEOUT:
	  _buffer += "Request Timeout";
	  break;
      case CONFLICT:
	  _buffer += "Conflict";
	  break;
      case GONE:
	  _buffer += "Gone";
	  break;
      case LENGTH_REQUIRED:
	  _buffer += "Length Required";
	  break;
      case PRECONDITION_FAILED:
	  _buffer += "Precondition Failed";
	  break;
      case REQUEST_ENTITY_TOO_LARGE:
	  _buffer += "Request Entity Too Large";
	  break;
      case REQUEST_URI_TOO_LARGE:
	  _buffer += "Request URI Too Large";
	  break;
      case UNSUPPORTED_MEDIA_TYPE:
	  _buffer += "Unsupported Media Type";
	  break;
      case REQUESTED_RANGE_NOT_SATISFIABLE:
	  _buffer += "Request Range Not Satisfiable";
	  break;
      case EXPECTATION_FAILED:
	  _buffer += "Expectation Failed";
	  break;
	  // 5xx: Server Error - The server failed to fulfill an apparently valid request
      case INTERNAL_SERVER_ERROR:
	  _buffer += "Internal Server Error";
	  break;
      case NOT_IMPLEMENTED:
	  _buffer += "Method Not Implemented";
	  break;
      case BAD_GATEWAY:
	  _buffer += "Bad Gateway";
	  break;
      case SERVICE_UNAVAILABLE:
	  _buffer += "Service Unavailable";
	  break;
      case GATEWAY_TIMEOUT:
	  _buffer += "Gateway Timeout";
	  break;
      case HTTP_VERSION_NOT_SUPPORTED:
	  _buffer += "HTTP Version Not Supported";
	  break;
	  // Gnash/Cygnal extensions for internal use
      case LIFE_IS_GOOD:
	  break;
      case CLOSEPIPE:
	  _buffer += "Close Pipe";	  
	  break;
      default:
	  break;
    }

    // end the line
    _buffer += "\r\n";

    formatDate();
    formatServer();
    formatLastModified();
    formatAcceptRanges("bytes");
    formatContentLength(size);
    // Apache closes the connection on GET requests, so we do the same.
    // This is a bit silly, because if we close after every GET request,
    // we're not really handling the persistance of HTTP 1.1 at all.
    formatConnection("close");
    formatContentType(type);

    // All HTTP messages are followed by a blank line.
    terminateHeader();

    return _buffer;
}

amf::Buffer &
HTTP::formatErrorResponse(http_status_e code)
{
//    GNASH_REPORT_FUNCTION;

    char num[12];
    // First build the message body, so we know how to set Content-Length
    _buffer += "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n";
    _buffer += "<html><head>\r\n";
    _buffer += "<title>";
    sprintf(num, "%d", code);
    _buffer += num;
    _buffer += " Not Found</title>\r\n";
    _buffer += "</head><body>\r\n";
    _buffer += "<h1>Not Found</h1>\r\n";
    _buffer += "<p>The requested URL ";
    _buffer += _filespec;
    _buffer += " was not found on this server.</p>\r\n";
    _buffer += "<hr>\r\n";
    _buffer += "<address>Cygnal (GNU/Linux) Server at ";
    _buffer += getField("host");
    _buffer += " </address>\r\n";
    _buffer += "</body></html>\r\n";

    // First build the header
    formatDate();
    formatServer();
    formatContentLength(_filesize);
    formatConnection("close");
    formatContentType(_filetype);

    // All HTTP messages are followed by a blank line.
    terminateHeader();

    return _buffer;
}

amf::Buffer &
HTTP::formatDate()
{
//    GNASH_REPORT_FUNCTION;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    
//    cout <<  now.time_of_day() << "\r\n";
    
    boost::gregorian::date d(now.date());
    
    char num[12];

    boost::gregorian::greg_weekday wd = d.day_of_week();
    _buffer += "Date: ";
    _buffer += wd.as_long_string();
    
    _buffer += ", ";
    sprintf(num, "%d", static_cast<int>(d.day()));
    _buffer += num;
    
    _buffer += " ";
    _buffer += boost::gregorian::greg_month(d.month()).as_short_string();

    _buffer += " ";
    sprintf(num, "%d", static_cast<int>(d.year()));
    _buffer += num;
    
    _buffer += " ";
    _buffer += boost::posix_time::to_simple_string(now.time_of_day());
    
    _buffer += " GMT\r\n";

    return _buffer;
}

amf::Buffer &
HTTP::formatServer()
{
//    GNASH_REPORT_FUNCTION;
    _buffer += "Server: Cygnal (GNU/Linux)\r\n";

    return _buffer;
}

amf::Buffer &
HTTP::formatServer(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    _buffer += "Server: ";
    _buffer += data;
    _buffer += "\r\n";

    return _buffer;
}

amf::Buffer &
HTTP::formatContentLength()
{
//    GNASH_REPORT_FUNCTION;
    
    return formatContentLength(_filesize);
}

amf::Buffer &
HTTP::formatContentLength(boost::uint32_t filesize)
{
//    GNASH_REPORT_FUNCTION;
//    _header << "Content-Length: " << filesize << "\r\n";

    _buffer += "Content-Length: ";
    char num[12];
    sprintf(num, "%d", filesize);
    _buffer += num;
    _buffer += "\r\n";
    
    return _buffer;
}

amf::Buffer &
HTTP::formatContentType()
{
//    GNASH_REPORT_FUNCTION;
    return formatContentType(_filetype);
}

amf::Buffer &
HTTP::formatContentType(DiskStream::filetype_e filetype)
{
//    GNASH_REPORT_FUNCTION;
    
    switch (filetype) {
      // default to HTML if the type isn't known
      case DiskStream::FILETYPE_NONE:
	  _buffer += "Content-Type: text/html\r\n";
	  break;
      case DiskStream::FILETYPE_AMF:
	  _buffer += "Content-Type: application/x-amf\r\n";
	  break;
      case DiskStream::FILETYPE_SWF:
	  _buffer += "Content-Type: application/x-shockwave-flash\r\n";
	  break;
      case DiskStream::FILETYPE_HTML:
	  _buffer += "Content-Type: text/html\r\n";
	  break;
    case DiskStream::FILETYPE_PNG:
	  _buffer += "Content-Type: image/png\r\n";
	  break;
    case DiskStream::FILETYPE_JPEG:
	  _buffer += "Content-Type: image/jpeg\r\n";
	  break;
    case DiskStream::FILETYPE_GIF:
	  _buffer += "Content-Type: image/gif\r\n";
	  break;
    case DiskStream::FILETYPE_MP3:
	  _buffer += "Content-Type: audio/mpeg\r\n";
	  break;
    case DiskStream::FILETYPE_MP4:
	  _buffer += "Content-Type: video/mp4\r\n";
	  break;
    case DiskStream::FILETYPE_OGG:
	  _buffer += "Content-Type: audio/ogg\r\n";
	  break;
    case DiskStream::FILETYPE_VORBIS:
	  _buffer += "Content-Type: audio/ogg\r\n";
	  break;
    case DiskStream::FILETYPE_THEORA:
	  _buffer += "Content-Type: video/ogg\r\n";
	  break;
    case DiskStream::FILETYPE_DIRAC:
	  _buffer += "Content-Type: video/dirac\r\n";
	  break;
    case DiskStream::FILETYPE_TEXT:
	  _buffer += "Content-Type: text/plain\r\n";
	  break;
    case DiskStream::FILETYPE_FLV:
	  _buffer += "Content-Type: video/x-flv\r\n";
	  break;
    case DiskStream::FILETYPE_VP6:
	  _buffer += "Content-Type: video/vp6\r\n";
	  break;
    case DiskStream::FILETYPE_XML:
	  _buffer += "Content-Type: application/xml\r\n";
	  break;
    case DiskStream::FILETYPE_FLAC:
	  _buffer += "Content-Type: audio/flac\r\n";
	  break;
      default:
	  _buffer += "Content-Type: text/html\r\n";
    }

    return _buffer;
}

amf::Buffer &
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

amf::Buffer &
HTTP::formatGetReply(http_status_e code)
{

//    GNASH_REPORT_FUNCTION;
    
    return formatHeader(_filesize, code);
}

amf::Buffer &
HTTP::formatGetReply(size_t size, http_status_e code)
{
//    GNASH_REPORT_FUNCTION;
    
    formatHeader(size, code);
    
//    int ret = Network::writeNet(_header.str());    
//    boost::uint8_t *ptr = (boost::uint8_t *)_body.str().c_str();
//     buf->copy(ptr, _body.str().size());
//    _handler->dump();

#if 0
    if (_header.str().size()) {
        log_debug (_("Sent GET Reply"));
	return _buffer;
    } else {
	clearHeader();
	log_debug (_("Couldn't send GET Reply, no header data"));
    }    
#endif
    
    return _buffer;
}

amf::Buffer &
HTTP::formatPostReply(rtmpt_cmd_e /* code */)
{
    GNASH_REPORT_FUNCTION;

    formatDate();
    formatServer();
    formatContentType(DiskStream::FILETYPE_AMF);
    // All HTTP messages are followed by a blank line.
    terminateHeader();
    return _buffer;

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

    return _buffer;
}

// Parse an Echo Request message coming from the Red5 echo_test. This
// method should only be used for testing purposes.
vector<boost::shared_ptr<amf::Element > >
HTTP::parseEchoRequest(boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    
    vector<boost::shared_ptr<amf::Element > > headers;
	
    // skip past the header bytes, we don't care about them.
    boost::uint8_t *tmpptr = data + 6;
    
    boost::uint16_t length;
    length = ntohs((*(boost::uint16_t *)tmpptr) & 0xffff);
    tmpptr += sizeof(boost::uint16_t);

    // Get the first name, which is a raw string, and not preceded by
    // a type byte.
    boost::shared_ptr<amf::Element > el1(new amf::Element);
    
    // If the length of the name field is corrupted, then we get out of
    // range quick, and corrupt memory. This is a bit of a hack, but
    // reduces memory errors caused by some of the corrupted tes cases.
    boost::uint8_t *endstr = std::find(tmpptr, tmpptr+length, '\0');
    if (endstr != tmpptr+length) {
	log_debug("Caught corrupted string! length was %d, null at %d",
		  length,  endstr-tmpptr);
	length = endstr-tmpptr;
    }
    el1->setName(tmpptr, length);
    tmpptr += length;
    headers.push_back(el1);
    
    // Get the second name, which is a raw string, and not preceded by
    // a type byte.
    length = ntohs((*(boost::uint16_t *)tmpptr) & 0xffff);
    tmpptr += sizeof(boost::uint16_t);
    boost::shared_ptr<amf::Element > el2(new amf::Element);

//     std::string name2(reinterpret_cast<const char *>(tmpptr), length);
//     el2->setName(name2.c_str(), name2.size());
    // If the length of the name field is corrupted, then we get out of
    // range quick, and corrupt memory. This is a bit of a hack, but
    // reduces memory errors caused by some of the corrupted tes cases.
    endstr = std::find(tmpptr, tmpptr+length, '\0');
    if (endstr != tmpptr+length) {
	log_debug("Caught corrupted string! length was %d, null at %d",
		  length,  endstr-tmpptr);
	length = endstr-tmpptr;
    }
    el2->setName(tmpptr, length);
    headers.push_back(el2);
    tmpptr += length;

    // Get the last two pieces of data, which are both AMF encoded
    // with a type byte.
    amf::AMF amf;
    boost::shared_ptr<amf::Element> el3 = amf.extractAMF(tmpptr, tmpptr + size);
    headers.push_back(el3);
    tmpptr += amf.totalsize();
    
    boost::shared_ptr<amf::Element> el4 = amf.extractAMF(tmpptr, tmpptr + size);
    headers.push_back(el4);

     return headers;
}

// format a response to the 'echo' test used for testing Gnash. This
// is only used for testing by developers. The format appears to be
// two strings, followed by a double, followed by the "onResult".
amf::Buffer &
HTTP::formatEchoResponse(const std::string &num, amf::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<amf::Buffer> data;

    amf::Element nel;
    if (el.getType() == amf::Element::TYPED_OBJECT_AMF0) {
	nel.makeTypedObject();
	string name = el.getName();
	nel.setName(name);
	if (el.propertySize()) {
	    // FIXME: see about using std::reverse() instead.
	    for (int i=el.propertySize()-1; i>=0; i--) {
// 	    for (int i=0 ; i<el.propertySize(); i++) {
		boost::shared_ptr<amf::Element> child = el.getProperty(i);
		nel.addProperty(child);
	    }
	    data = nel.encode();
	} else {
	    data = el.encode();
	}
    } else {
	data = el.encode();
    }

    return formatEchoResponse(num, data->reference(), data->allocated());
}

amf::Buffer &
HTTP::formatEchoResponse(const std::string &num, amf::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatEchoResponse(num, data.reference(), data.allocated());
}

amf::Buffer &
HTTP::formatEchoResponse(const std::string &num, boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    //boost::uint8_t *tmpptr  = data;
    
    // FIXME: temporary hacks while debugging
    amf::Buffer fixme("00 00 00 00 00 01");
    amf::Buffer fixme2("ff ff ff ff");
    
    _buffer = "HTTP/1.1 200 OK\r\n";
    formatContentType(DiskStream::FILETYPE_AMF);
//    formatContentLength(size);
    // FIXME: this is a hack ! Calculate a real size!
    formatContentLength(size+29);
    
    // Pretend to be Red5 server
    formatServer("Jetty(6.1.7)");
    
    // All HTTP messages are followed by a blank line.
    terminateHeader();

    // Add the binary blob for the header
    _buffer += fixme;

    // Make the result response, which is the 2nd data item passed in
    // the request, a slash followed by a number like "/2".
    string result = num;
    result += "/onResult";
    boost::shared_ptr<amf::Buffer> res = amf::AMF::encodeString(result);
    _buffer.append(res->begin()+1, res->size()-1);

    // Add the null data item
    boost::shared_ptr<amf::Buffer> null = amf::AMF::encodeString("null");
    _buffer.append(null->begin()+1, null->size()-1);

    // Add the other binary blob
    _buffer += fixme2;

    amf::Element::amf0_type_e type = static_cast<amf::Element::amf0_type_e>(*data);
    if ((type == amf::Element::UNSUPPORTED_AMF0)
	|| (type == amf::Element::NULL_AMF0)) {
	_buffer += type;
	// Red5 returns a NULL object when it's recieved an undefined one in the echo_test
    } else if (type == amf::Element::UNDEFINED_AMF0) {
	_buffer += amf::Element::NULL_AMF0;
    } else {
	// Add the AMF data we're echoing back
	if (size) {
	    _buffer.append(data, size);
	}
    }
    
    return _buffer;
}

amf::Buffer &
HTTP::formatRequest(const string & /* url */, http_method_e /* req */)
{
//    GNASH_REPORT_FUNCTION;

#if 0
    _header.str("");

    _header << req << " " << url << "HTTP/1.1" << "\r\n";
    _header << "User-Agent: Opera/9.01 (X11; Linux i686; U; en)" << "\r\n";
    _header << "Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1" << "\r\n";

    _header << "Accept-Language: en" << "\r\n";
    _header << "Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1" << "\r\n";
    
    _header << "Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0" << "\r\n";
    _header << "Referer: " << url << "\r\n";

    _header << "TE: deflate, gzip, chunked, identity, trailers" << "\r\n";
#endif
    
    return _buffer;
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
HTTP::extractRTMPT(boost::uint8_t *data)
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
HTTP::extractCommand(boost::uint8_t *data)
{
    GNASH_REPORT_FUNCTION;

//    string body = reinterpret_cast<const char *>(data);
    HTTP::http_method_e cmd = HTTP::HTTP_NONE;

    // force the case to make comparisons easier
//     std::transform(body.begin(), body.end(), body.begin(), 
//                (int(*)(int)) toupper);

    // Extract the command
    if (memcmp(data, "GET", 3) == 0) {
        cmd = HTTP::HTTP_GET;
    } else if (memcmp(data, "POST", 4) == 0) {
        cmd = HTTP::HTTP_POST;
    } else if (memcmp(data, "HEAD", 4) == 0) {
        cmd = HTTP::HTTP_HEAD;
    } else if (memcmp(data, "CONNECT", 7) == 0) {
        cmd = HTTP::HTTP_CONNECT;
    } else if (memcmp(data, "TRACE", 5) == 0) {
        cmd = HTTP::HTTP_TRACE;
    } else if (memcmp(data, "PUT", 3) == 0) {
        cmd = HTTP::HTTP_PUT;
    } else if (memcmp(data, "OPTIONS", 4) == 0) {
        cmd = HTTP::HTTP_OPTIONS;
    } else if (memcmp(data, "DELETE", 4) == 0) {
        cmd = HTTP::HTTP_DELETE;
    }

    // For valid requests, the second argument, delimited by spaces
    // is the filespec of the file being requested or transmitted.
    if (cmd != HTTP::HTTP_NONE) {
	boost::uint8_t *start = std::find(data, data+7, ' ') + 1;
	boost::uint8_t *end   = std::find(start + 2, data+PATH_MAX, ' ');
	boost::uint8_t *params = std::find(start, end, '?');
	if (params != end) {
	    _params = std::string(params+1, end);
	    _filespec = std::string(start, params);
	    log_debug("Parameters for file: \"%s\"", _params);
	} else {
	    // This is fine as long as end is within the buffer.
	    _filespec = std::string(start, end);
	}
	log_debug("Requesting file: \"%s\"", _filespec);

	// The third field is always the HTTP version
	// The version is the last field and is the protocol name
	// followed by a slash, and the version number. Note that
	// the version is not a double, even though it has a dot
	// in it. It's actually two separate integers.
	_version.major = *(end+6) - '0';
	_version.minor = *(end+8) - '0';
	log_debug (_("Version: %d.%d"), _version.major, _version.minor);
    }

    return cmd;
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
HTTP::sendMsg(int /* fd */)
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
HTTP::sendMsg(const boost::uint8_t *data, size_t size)
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
	int ret = net.readNet(fd, *buf, 5);
//	cerr << __PRETTY_FUNCTION__ << ret << " : " << (char *)buf->reference() << endl;

	// the read timed out as there was no data, but the socket is still open.
 	if (ret == 0) {
	    log_debug("no data yet for fd #%d, continuing...", fd);
 	    continue;
 	}
	// ret is "no position" when the socket is closed from the other end of the connection,
	// so we're done.
	if ((ret == static_cast<int>(string::npos)) || (ret == -1)) {
	    log_debug("socket for fd #%d was closed...", fd);
	    return 0;
	}
	// We got data. Resize the buffer if necessary.
	if (ret > 0) {
	    buf->setSeekPointer(buf->reference() + ret);
//	    cerr << "XXXXX: " << (char *)buf->reference() << endl;
 	    if (ret < static_cast<int>(amf::NETBUFSIZE)) {
// 		buf->resize(ret);	FIXME: why does this corrupt
// 		the buffer ?
		_que.push(buf);
		break;
 	    } else {
		_que.push(buf);
	    }

        // ret must be more than 0 here
	    if (static_cast<size_t>(ret) == buf->size()) {
		continue;
	    }
	} else {
	    log_debug("no more data for fd #%d, exiting...", fd);
	    return 0;
	}
	if (ret == -1) {
	  log_debug("Handler done for fd #%d, can't read any data...", fd);
	  return -1;
	}
    } while (ret);
    
    // We're done. Notify the other threads the socket is closed, and tell them to die.
    log_debug("Handler done for fd #%d...", fd);

    return ret;
}

void
HTTP::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
        
    log_debug (_("==== The HTTP header breaks down as follows: ===="));
    log_debug (_("Filespec: %s"), _filespec.c_str());
    log_debug (_("Version: %d.%d"), _version.major, _version.minor);

    map<string, string>::const_iterator it;
    for (it = _fields.begin(); it != _fields.end(); ++it) {
	log_debug("Field: \"%s\" = \"%s\"", it->first, it->second);
    }
    
    // Dump the RTMPT fields
    log_debug("RTMPT optional index is: ", _index);
    log_debug("RTMPT optional client ID is: ", _clientid);
    log_debug (_("==== ==== ===="));
}

extern "C" {
bool
http_handler(Network::thread_params_t *args)
{
//    GNASH_REPORT_FUNCTION;
//    struct thread_params thread_data;
    string url, filespec, parameters;
    HTTP *www = new HTTP;
    bool result = false;
    
//    Network *net = reinterpret_cast<Network *>(args->handler);
    bool done = false;
//    www.setHandler(net);

    log_debug(_("Starting HTTP Handler for fd #%d, tid %ld"),
	      args->netfd, get_thread_id());
    
    string docroot = args->filespec;

    www->setDocRoot(docroot);
    log_debug("Starting to wait for data in net for fd #%d", args->netfd);

    // Wait for data, and when we get it, process it.
    do {
	
#ifdef USE_STATISTICS
	struct timespec start;
	clock_gettime (CLOCK_REALTIME, &start);
#endif

	// See if we have any messages waiting
	if (www->recvMsg(args->netfd) == 0) {
	    done = true;
	}

	// Process incoming messages
	if (!www->processClientRequest(args->netfd)) {
//	    hand->die();	// tell all the threads for this connection to die
//	    hand->notifyin();
	    log_debug("Net HTTP done for fd #%d...", args->netfd);
//	    done = true;
	}
//	www.dump();
	
#if 0
	string response = cache.findResponse(filestream->getFilespec());
	if (response.empty()) {
	    cerr << "FIXME no cache hit for: " << www.getFilespec() << endl;
//	    www.clearHeader();
// 	    amf::Buffer &ss = www.formatHeader(filestream->getFileSize(), HTTP::LIFE_IS_GOOD);
// 	    www.writeNet(args->netfd, (boost::uint8_t *)www.getHeader().c_str(), www.getHeader().size());
// 	    cache.addResponse(www.getFilespec(), www.getHeader());
	} else {
	    cerr << "FIXME cache hit on: " << www.getFilespec() << endl;
	    www.writeNet(args->netfd, (boost::uint8_t *)response.c_str(), response.size());
	}	
#endif
	
	// Unless the Keep-Alive flag is set, this isn't a persisant network
	// connection.
	if (!www->keepAlive()) {
	    log_debug("Keep-Alive is off", www->keepAlive());
	    result = false;
	    done = true;
	} else {
	    log_debug("Keep-Alive is on", www->keepAlive());
	    result = true;
//	    done = true;
	}
#ifdef USE_STATISTICS
	struct timespec end;
	clock_gettime (CLOCK_REALTIME, &end);
	log_debug("Processing time for GET request was %f seconds",
		  (float)((end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9)));
#endif
    } while(done != true);
    
//    hand->notify();
    
    log_debug("http_handler all done now finally...");

    return result;
} // end of httphandler
    
} // end of extern C

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
