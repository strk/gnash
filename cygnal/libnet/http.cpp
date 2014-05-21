// http.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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
//

#include <boost/thread/mutex.hpp>
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
#include "http.h"
#include "amf.h"
#include "element.h"
#include "cque.h"
#include "log.h"
#include "network.h"
// #include "handler.h"
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

using std::string;

static boost::mutex stl_mutex;

namespace gnash
{

// extern map<int, Handler *> handlers;

#if 0
// FIXME, this seems too small to me.  --gnu
static const int readsize = 1024;
#endif

static Cache& cache = Cache::getDefaultInstance();

HTTP::HTTP() 
    : _filetype(DiskStream::FILETYPE_HTML),
      _filesize(0),
      _keepalive(false),
//       _handler(0),
      _clientid(0),
      _index(0),
      _max_requests(0),
      _close(false)
{
//    GNASH_REPORT_FUNCTION;
//    struct status_codes *status = new struct status_codes;
    _version.major = 0;
    _version.minor = 0;
    
//    _status_codes(CONTINUE, status);
}

#if 0
HTTP::HTTP(Handler *hand) 
    : _filetype(DiskStream::FILETYPE_HTML),
      _filesize(0),
      _keepalive(false),
      _clientid(0),
      _index(0),
      _max_requests(0),
      _close(false)

{
//    GNASH_REPORT_FUNCTION;
//     _handler = hand;
    _version.major = 0;
    _version.minor = 0;
}
#endif

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


std::uint8_t *
HTTP::processHeaderFields(cygnal::Buffer *buf)
{
  //    GNASH_REPORT_FUNCTION;
    string head(reinterpret_cast<const char *>(buf->reference()), buf->size());

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
		    _max_requests = strtol(value.c_str(), nullptr, 0);
		    // log_debug("Setting Max Requests for Keep-Alive to %d", _max_requests);
		}
	    }
	    if (name == "connection") {
		if (value.find("keep-alive", 0) != string::npos) {
		    _keepalive = true;
		}
	    }
	    if (name == "content-length") {
		_filesize = strtol(value.c_str(), nullptr, 0);
		log_debug(_("Setting Content Length to %d"), _filesize);
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
		log_debug(_("Setting Content Type to %d"), _filetype);
	    }
	    
//	    cerr << "FIXME: " << (void *)i << " : " << dec <<  end << endl;
	} else {
	    const std::uint8_t *cmd = reinterpret_cast<const std::uint8_t *>(i->c_str());
	    if (extractCommand(const_cast<std::uint8_t *>(cmd)) == HTTP::HTTP_NONE) {
		break;
#if 1
	    } else {
		log_debug(_("Got a request, parsing \"%s\""), *i);
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
		    // log_debug (_("Version: %d.%d"), _version.major, _version.minor);
		    // the filespec in the request is the middle field, deliminated
		    // by a space on each end.
		    if (params != string::npos) {
			_params = i->substr(params+1, end);
			_filespec = i->substr(start+1, params);
			log_debug(_("Parameters for file: \"%s\""), _params);
		    } else {
			_filespec = i->substr(start+1, pos-start-2);
		    }
		    log_debug(_("Requesting file: \"%s\""), _filespec);

		    // HTTP 1.1 enables persistent network connections
		    // by default.
		    if (_version.minor > 0) {
			log_debug(_("Enabling Keep Alive by default for HTTP > 1.0"));
			_keepalive = true;
		    }
		}
	    }
#endif
	}
    }
    
    return buf->reference() + end + 4;
}

// // Parse an Echo Request message coming from the Red5 echo_test. This
// // method should only be used for testing purposes.
// vector<std::shared_ptr<cygnal::Element > >
// HTTP::parseEchoRequest(std::uint8_t *data, size_t size)
// {
// //    GNASH_REPORT_FUNCTION;
    
//     vector<std::shared_ptr<cygnal::Element > > headers;
	
//     // skip past the header bytes, we don't care about them.
//     std::uint8_t *tmpptr = data + 6;
    
//     std::uint16_t length;
//     length = ntohs((*(std::uint16_t *)tmpptr) & 0xffff);
//     tmpptr += sizeof(std::uint16_t);

//     // Get the first name, which is a raw string, and not preceded by
//     // a type byte.
//     std::shared_ptr<cygnal::Element > el1(new cygnal::Element);
    
//     // If the length of the name field is corrupted, then we get out of
//     // range quick, and corrupt memory. This is a bit of a hack, but
//     // reduces memory errors caused by some of the corrupted tes cases.
//     std::uint8_t *endstr = std::find(tmpptr, tmpptr+length, '\0');
//     if (endstr != tmpptr+length) {
// 	log_debug("Caught corrupted string! length was %d, null at %d",
// 		  length,  endstr-tmpptr);
// 	length = endstr-tmpptr;
//     }
//     el1->setName(tmpptr, length);
//     tmpptr += length;
//     headers.push_back(el1);
    
//     // Get the second name, which is a raw string, and not preceded by
//     // a type byte.
//     length = ntohs((*(std::uint16_t *)tmpptr) & 0xffff);
//     tmpptr += sizeof(std::uint16_t);
//     std::shared_ptr<cygnal::Element > el2(new cygnal::Element);

// //     std::string name2(reinterpret_cast<const char *>(tmpptr), length);
// //     el2->setName(name2.c_str(), name2.size());
//     // If the length of the name field is corrupted, then we get out of
//     // range quick, and corrupt memory. This is a bit of a hack, but
//     // reduces memory errors caused by some of the corrupted tes cases.
//     endstr = std::find(tmpptr, tmpptr+length, '\0');
//     if (endstr != tmpptr+length) {
// 	log_debug("Caught corrupted string! length was %d, null at %d",
// 		  length,  endstr-tmpptr);
// 	length = endstr-tmpptr;
//     }
//     el2->setName(tmpptr, length);
//     headers.push_back(el2);
//     tmpptr += length;

//     // Get the last two pieces of data, which are both AMF encoded
//     // with a type byte.
//     amf::AMF amf;
//     std::shared_ptr<cygnal::Element> el3 = amf.extractAMF(tmpptr, tmpptr + size);
//     headers.push_back(el3);
//     tmpptr += amf.totalsize();
    
//     std::shared_ptr<cygnal::Element> el4 = amf.extractAMF(tmpptr, tmpptr + size);
//     headers.push_back(el4);

//      return headers;
// }

// // format a response to the 'echo' test used for testing Gnash. This
// // is only used for testing by developers. The format appears to be
// // two strings, followed by a double, followed by the "onResult".
// cygnal::Buffer &
// HTTP::formatEchoResponse(const std::string &num, cygnal::Element &el)
// {
// //    GNASH_REPORT_FUNCTION;
//     std::shared_ptr<cygnal::Buffer> data;

//     cygnal::Element nel;
//     if (el.getType() == cygnal::Element::TYPED_OBJECT_AMF0) {
// 	nel.makeTypedObject();
// 	string name = el.getName();
// 	nel.setName(name);
// 	if (el.propertySize()) {
// 	    // FIXME: see about using std::reverse() instead.
// 	    for (int i=el.propertySize()-1; i>=0; i--) {
// // 	    for (int i=0 ; i<el.propertySize(); i++) {
// 		std::shared_ptr<cygnal::Element> child = el.getProperty(i);
// 		nel.addProperty(child);
// 	    }
// 	    data = nel.encode();
// 	} else {
// 	    data = el.encode();
// 	}
//     } else {
// 	data = el.encode();
//     }

//     return formatEchoResponse(num, data->reference(), data->allocated());
// }

#if 0				// FIXME:
// Client side parsing of response message codes
std::shared_ptr<HTTP::http_response_t>
HTTP::parseStatus(const std::string &line)
{
//    GNASH_REPORT_FUNCTION;

    std::shared_ptr<http_response_t> status;
    // The respnse is a number followed by the error message.
    string::size_type pos = line.find(" ", 0);
    if (pos != string::npos) {
	status.reset(new http_response_t);
	status->code = static_cast<HTTP::http_status_e>(strtol(line.substr(0, pos).c_str(), NULL, 0));
	status->msg = line.substr(pos+1, line.size());
    }

    return status;
}

HTTP::http_method_e
HTTP::processClientRequest(int fd)
{
//    GNASH_REPORT_FUNCTION;
    bool result = false;
    
    std::shared_ptr<cygnal::Buffer> buf(_que.peek());
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

    return _cmd;
}

// A GET request asks the server to send a file to the client
bool
HTTP::processGetRequest(int fd)
{
    GNASH_REPORT_FUNCTION;

//     std::uint8_t buffer[readsize+1];
//     const char *ptr = reinterpret_cast<const char *>(buffer);
//     memset(buffer, 0, readsize+1);
    
//    _handler->wait();
//    _handler->dump();

    cerr << "QUE = " << _que.size() << endl;

    if (_que.size() == 0) {
	return false;
    }
    
    std::shared_ptr<cygnal::Buffer> buf(_que.pop());
//    cerr << "YYYYYYY: " << (char *)buf->reference() << endl;
//    cerr << hexify(buf->reference(), buf->allocated(), false) << endl;
    
    if (buf == 0) {
     //	log_debug("Que empty, net connection dropped for fd #%d", getFileFd());
	log_debug(_("Que empty, net connection dropped for fd #%d"), fd);
	return false;
    }
    
    clearHeader();
    processHeaderFields(*buf);

    string url = _docroot + _filespec;
    // See if the file is in the cache and already opened.
    std::shared_ptr<DiskStream> filestream(cache.findFile(url));
    if (filestream) {
	log_network(_("FIXME: found file in cache!"));
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
    cygnal::Buffer &reply = formatHeader(filestream->getFileType(),
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

    log_debug(_("http_handler all done transferring requested file \"%s\"."),
	      _filespec);
    
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
    
    std::shared_ptr<cygnal::Buffer> buf(_que.pop());
    if (buf == 0) {
	log_debug(_("Que empty, net connection dropped for fd #%d"), getFileFd());
	return false;
    }
//    cerr << __FUNCTION__ << buf->allocated() << " : " << hexify(buf->reference(), buf->allocated(), true) << endl;
    
    clearHeader();
    std::uint8_t *data = processHeaderFields(*buf);
    size_t length = strtol(getField("content-length").c_str(), NULL, 0);
    std::shared_ptr<cygnal::Buffer> content(new cygnal::Buffer(length));
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
	log_debug(_("Got file data in POST"));
	string url = _docroot + _filespec;
	DiskStream ds(url, *content);
	ds.writeToDisk();
//    ds.close();
	// oh boy, we got ourselves some encoded AMF objects instead of a boring file.
    } else if (getField("content-type") == "application/x-amf") {
	log_debug(_("Got AMF data in POST"));
#if 0
	amf::AMF amf;
	std::shared_ptr<cygnal::Element> el = amf.extractAMF(content.reference(), content.end());
	el->dump();		// FIXME: do something intelligent
				// with this Element
#endif
    }
    
    // Send the reply

    // NOTE: this is a "special" path we trap until we have real CGI support
    if ((_filespec == "/echo/gateway")
	&& (getField("content-type") == "application/x-amf")) {
//	const char *num = (const char *)buf->at(10);
	log_debug(_("Got CGI echo request in POST"));
//	cerr << "FIXME 2: " << hexify(content->reference(), content->allocated(), true) << endl;

	vector<std::shared_ptr<cygnal::Element> > headers = parseEchoRequest(*content);
  	//std::shared_ptr<cygnal::Element> &el0 = headers[0];
  	//std::shared_ptr<cygnal::Element> &el1 = headers[1];
  	//std::shared_ptr<cygnal::Element> &el3 = headers[3];
	
    if (headers.size() >= 4) {
	    if (headers[3]) {
		cygnal::Buffer &reply = formatEchoResponse(headers[1]->getName(), *headers[3]);
// 	    cerr << "FIXME 3: " << hexify(reply.reference(), reply.allocated(), true) << endl;
// 	    cerr << "FIXME 3: " << hexify(reply.reference(), reply.allocated(), false) << endl;
		writeNet(fd, reply);
	    }
 	}
    } else {
	cygnal::Buffer &reply = formatHeader(_filetype, _filesize, HTTP::OK);
	writeNet(fd, reply);
    }

    return true;
}

bool
HTTP::processPutRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("PUT request"));

    return false;
}

bool
HTTP::processDeleteRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("DELETE request"));
    return false;
}

bool
HTTP::processConnectRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("CONNECT request"));
    return false;
}

bool
HTTP::processOptionsRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("OPTIONS request"));
    return false;
}

bool
HTTP::processHeadRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("HEAD request"));
    return false;
}

bool
HTTP::processTraceRequest(int /* fd */)
{
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("TRACE request"));
    return false;
}
#endif

// Convert the Content-Length field to a number we can use
size_t
HTTP::getContentLength()
{
    //    GNASH_REPORT_FUNCTION;
    std::string length = getField("content-length");
    if (length.size() > 0) {
	return static_cast<size_t>(strtol(length.c_str(), nullptr, 0));
    }

    return 0;
}


// http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec5 (5.3 Request Header Fields)
bool
HTTP::checkRequestFields(cygnal::Buffer & /* buf */)
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
HTTP::checkEntityFields(cygnal::Buffer & /* buf */)
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
HTTP::checkGeneralFields(cygnal::Buffer & /* buf */)
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

std::shared_ptr<std::vector<std::string> >
HTTP::getFieldItem(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<std::vector<std::string> > ptr(new std::vector<std::string>);
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

cygnal::Buffer &
HTTP::formatHeader(http_status_e type)
{
//    GNASH_REPORT_FUNCTION;

    return formatHeader(_filesize, type);
}

cygnal::Buffer &
HTTP::formatCommon(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    _buffer += data;
    _buffer += "\r\n";

    return _buffer;
}

cygnal::Buffer &
HTTP::formatHeader(size_t size, http_status_e code)
{
//    GNASH_REPORT_FUNCTION;
  return formatHeader(_filetype, size, code);
}

cygnal::Buffer &
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
    if (_close) {
	formatConnection("close");
	_keepalive = false;
    }
    formatContentType(type);

    // All HTTP messages are followed by a blank line.
    terminateHeader();

    return _buffer;
}

cygnal::Buffer &
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

cygnal::Buffer &
HTTP::formatServer()
{
//    GNASH_REPORT_FUNCTION;
    _buffer += "Server: Cygnal (GNU/Linux)\r\n";

    return _buffer;
}

cygnal::Buffer &
HTTP::formatServer(const string &data)
{
//    GNASH_REPORT_FUNCTION;
    _buffer += "Server: ";
    _buffer += data;
    _buffer += "\r\n";

    return _buffer;
}

cygnal::Buffer &
HTTP::formatContentLength()
{
//    GNASH_REPORT_FUNCTION;
    
    return formatContentLength(_filesize);
}

cygnal::Buffer &
HTTP::formatContentLength(std::uint32_t filesize)
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

cygnal::Buffer &
HTTP::formatContentType()
{
//    GNASH_REPORT_FUNCTION;
    return formatContentType(_filetype);
}

cygnal::Buffer &
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
    case DiskStream::FILETYPE_PHP:
	  _buffer += "Content-Type: application/x-httpd-php\r\n";
	  break;
      default:
	  _buffer += "Content-Type: text/html\r\n";
    }

    return _buffer;
}

cygnal::Buffer &
HTTP::formatLastModified()
{
//    GNASH_REPORT_FUNCTION;
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::stringstream date;
    
    boost::gregorian::date d(now.date());
    
    date << d.day_of_week();
    date << ", " << d.day();
    date << " "  << d.month();
    date << " "  << d.year();
    date << " "  << now.time_of_day();
    date << " GMT";

    return formatLastModified(date.str());
}

cygnal::Buffer &
HTTP::formatEchoResponse(const std::string &num, cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatEchoResponse(num, data.reference(), data.allocated());
}

cygnal::Buffer &
HTTP::formatEchoResponse(const std::string &num, std::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    //std::uint8_t *tmpptr  = data;
    
    // FIXME: temporary hacks while debugging
    cygnal::Buffer fixme("00 00 00 00 00 01");
    cygnal::Buffer fixme2("ff ff ff ff");
    
    _buffer = "HTTP/1.1 200 OK\r\n";
    formatContentType(DiskStream::FILETYPE_AMF);
//    formatContentLength(size);
    // FIXME: this is a hack ! Calculate a real size!
    formatContentLength(size+29);
    
    // Don't pretend to be the Red5 server
    formatServer("Cygnal (0.8.6)");
    
    // All HTTP messages are followed by a blank line.
    terminateHeader();

    // Add the binary blob for the header
    _buffer += fixme;

    // Make the result response, which is the 2nd data item passed in
    // the request, a slash followed by a number like "/2".
    string result = num;
    result += "/onResult";
    std::shared_ptr<cygnal::Buffer> res = cygnal::AMF::encodeString(result);
    _buffer.append(res->begin()+1, res->size()-1);

    // Add the null data item
    std::shared_ptr<cygnal::Buffer> null = cygnal::AMF::encodeString("null");
    _buffer.append(null->begin()+1, null->size()-1);

    // Add the other binary blob
    _buffer += fixme2;

    cygnal::Element::amf0_type_e type = static_cast<cygnal::Element::amf0_type_e>(*data);
    if ((type == cygnal::Element::UNSUPPORTED_AMF0)
	|| (type == cygnal::Element::NULL_AMF0)) {
	_buffer += type;
	// Red5 returns a NULL object when it's recieved an undefined one in the echo_test
    } else if (type == cygnal::Element::UNDEFINED_AMF0) {
	_buffer += cygnal::Element::NULL_AMF0;
    } else {
	// Add the AMF data we're echoing back
	if (size) {
	    _buffer.append(data, size);
	}
    }
    
    return _buffer;
}

cygnal::Buffer &
HTTP::formatRequest(const string &url, http_method_e cmd)
{
//    GNASH_REPORT_FUNCTION;
    
    clearHeader();

    switch (cmd) {
    case HTTP::HTTP_GET:
	_buffer = "GET ";
	break;
    case HTTP::HTTP_POST:
	_buffer = "POST ";
	break;
    case HTTP::HTTP_HEAD:
	_buffer = "HEAD ";
	break;
    case HTTP::HTTP_CONNECT:
	_buffer = "CONNECT ";
	break;
    case HTTP::HTTP_TRACE:
	_buffer = "TRACE ";
	break;
    case HTTP::HTTP_OPTIONS:
	_buffer = "OPTIONS ";
	break;
    default:
	break;
    }
    _buffer += url;

    _buffer += " HTTP/1.1";
//     sprintf(num, "%d.%d", _version.major, _version.minor);
//     _buffer += num;
    // end the line
    _buffer += "\r\n";

    formatHost("localhost");
    formatAgent("Gnash");

    // Post messages want a bunch more fields than a GET request
    if (cmd == HTTP::HTTP_POST) {
	formatContentType(DiskStream::FILETYPE_AMF);
	formatEncoding("deflate, gzip, x-gzip, identity, *;q=0");
	formatConnection("Keep-Alive");
    }

//     // All HTTP messages are followed by a blank line.
//     terminateHeader();

    return _buffer;
}


HTTP::http_method_e
HTTP::extractCommand(std::uint8_t *data)
{
    // GNASH_REPORT_FUNCTION;

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
    } else if (memcmp(data, "HTTP", 4) == 0) {
        cmd = HTTP::HTTP_RESPONSE;
    }

    // For valid requests, the second argument, delimited by spaces
    // is the filespec of the file being requested or transmitted.
    if (cmd != HTTP::HTTP_NONE) {
	std::uint8_t *start = std::find(data, data+7, ' ') + 1;
	std::uint8_t *end   = std::find(start + 2, data+PATH_MAX, ' ');
	std::uint8_t *params = std::find(start, end, '?');
	if (params != end) {
	    _params = std::string(params+1, end);
	    _filespec = std::string(start, params);
	    log_debug(_("Parameters for file: \"%s\""), _params);
	} else {
	    // This is fine as long as end is within the buffer.
	    _filespec = std::string(start, end);
	}
	// log_debug("Requesting file: \"%s\"", _filespec);

	// The third field is always the HTTP version
	// The version is the last field and is the protocol name
	// followed by a slash, and the version number. Note that
	// the version is not a double, even though it has a dot
	// in it. It's actually two separate integers.
	_version.major = *(end+6) - '0';
	_version.minor = *(end+8) - '0';
	// log_debug (_("Version: %d.%d"), _version.major, _version.minor);
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
HTTP::sendMsg(const std::uint8_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;
//    _header

    return Network::writeNet(data, size);
}

size_t
HTTP::recvChunked(std::uint8_t *data, size_t size)
{
//     GNASH_REPORT_FUNCTION;
    bool done = false;
    bool chunks = true;
    size_t total = 0;
    size_t pktsize = 0;
    
    if (size == 0) {
	return 0;
    }
    
    // A chunked transfer sends a count of bytes in ASCII hex first,
    // and that line is terminated with the usual \r\n HTTP header field
    // line number. There is supposed to be a ';' before the \r\n, as this
    // field can have other attributes, but the OpenStreetMap server doesn't
    // use the semi-colon, as it's optional, and rarely used anyway.
    std::shared_ptr<cygnal::Buffer> buf;
    std::uint8_t *start = std::find(data, data+size, '\r') + 2;
    if (start != data+size) {
	// extract the total size of the chunk
	std::string bytes(data, start-2);
	size_t sizesize = start-data;
	total = static_cast<size_t>(strtol(bytes.c_str(), nullptr, 16));
	log_debug(_("%s: Total size for first chunk is: %d, data size %d (%d)"),
		  __PRETTY_FUNCTION__, total, size, sizesize);
	buf.reset(new cygnal::Buffer(total+2));
	// Add the existing data from the previous packet
	buf->copy(data+sizesize, size-sizesize);
    }

    // The size of the packet we need to read has a 2 byte terminator "\r\n",
    // like any other HTTP header field, so we have to read those bytes too
    // so we can stay sychronized with the start of each chunk.
    pktsize = total - buf->allocated() + 2;
//     log_debug("%s: Total Packet size for first chunk is: %d", __PRETTY_FUNCTION__,
// 	      pktsize);
    
    done = false;
    size_t ret = 0;

    // Keep reading chunks as long as they arrive
    while (chunks) {
	do {
	    if (!pktsize) {
		total = 0;
		// Only read a few bytes, as we have todo a resize, so the less
		// data to copy the better. We only do this to get the total size
		// of the chunk, which we need to know when it's done. As we can't
		// tell we're done processing chunks till one has a length of
		// "0\r\n", this is important.
		pktsize = 12;
		buf.reset(new cygnal::Buffer(pktsize+2));
	    }
	    ret = readNet(buf->reference() + buf->allocated(), pktsize, 60);
 	    //buf->dump();
	    // We got data.
	    if (ret == 0) {
		log_debug(_("no data yet for fd #%d, continuing..."),
			  getFileFd());
		done = true;
	    }
	    if (ret) {
		// manually set the seek pointer in the buffer, as we read
		// the data into the raw memory allocated to the buffer. We
		// only want to do this if we got data of course.
		buf->setSeekPointer(buf->end() + ret);
		if (!total) {
		    start = std::find(buf->reference(), buf->reference()+ret, '\r') + 2;
		    if (start != buf->reference()+ret) {
			// extract the total size of the chunk
			std::string bytes(buf->reference(), start-2);
			total = static_cast<size_t>(strtol(bytes.c_str(), nullptr, 16));
			// The total size of the last chunk is always "0"
			if (total == 0) {
			    log_debug(_("%s: end of chunks!"), __PRETTY_FUNCTION__);
			    pktsize = 0;
			    done = true;
			    chunks = false;
			} else {
			    pktsize = total+8; // FIXME: why do we need an 8 here ?
// 			    log_debug("%s: Total size for chunk is: %d (%s), adding %d bytes",
// 				      __PRETTY_FUNCTION__, total, bytes, (start - buf->reference()));
			    cygnal::Buffer tmpbuf(start - buf->reference());
			    // don't forget the two bytes for the "\r\n"
			    tmpbuf.copy(buf->reference() + bytes.size() + 2, (start - buf->reference()));
			    buf->clear(); // FIXME: debug only
			    buf->resize(total);
 			    buf->copy(tmpbuf.reference(), tmpbuf.size());
			}
		    }
		}
		
		// If we got less data than specified, we need to keep reading data
		if (ret < buf->size()) {
		    pktsize -= ret;
		    if (pktsize == 0) {
			done = true;
		    } else {
// 			log_debug("Didn't get a full packet, reading more data");
			continue;
		    }
		}
	    }
	} while (!done);
//  	log_debug("%s: finished packet, ret is %d, pktsize is %d\r\n%s", __PRETTY_FUNCTION__, ret, pktsize,
//  		  hexify(buf->reference(), buf->allocated(), true));
	if (pktsize == 0) {
	    // The last two bytes of the chunk are always "\r\n", which we remove so it
	    // doesn't become part of the binary data being sent in the chunk.
	    if ((*(buf->end()-2) == '\r') && (*(buf->end()-1) == '\n')) {
		*(buf->end()-2) = 0; // '\r'
		*(buf->end()-1) = 0; // '\n'
		buf->setSeekPointer(buf->end() - 2);
	    }
	    _que.push(buf);
	}
	done = false;		// reset
    } // end of while chunks
    
    return _que.size();
}

int
HTTP::recvMsg(int fd)
{
//     GNASH_REPORT_FUNCTION;
    return recvMsg(fd, 0);
}

int
HTTP::recvMsg(int fd, size_t size)
{
    // GNASH_REPORT_FUNCTION;

    size_t ret = 0;

    if (size == 0) {
	size = cygnal::NETBUFSIZE;
    }
    
    log_debug(_("Starting to wait for data in net for fd #%d"), fd);
    Network net;

    do {
	std::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(size));
	ret = net.readNet(fd, *buf, 5);
//	cerr << __PRETTY_FUNCTION__ << ret << " : " << (char *)buf->reference() << endl;

	// the read timed out as there was no data, but the socket is still open.
 	if (ret == 0) {
	    log_debug(_("no data yet for fd #%d, continuing..."), fd);
 	    continue;
 	}
	// ret is "no position" when the socket is closed from the other end of the connection,
	// so we're done.
	if ((ret == static_cast<size_t>(string::npos)) || (static_cast<int>(ret) == -1)) {
	    log_debug(_("socket for fd #%d was closed..."), fd);
	    return 0;
	}
	// We got data. Resize the buffer if necessary.
	if (ret > 0) {
	    buf->setSeekPointer(buf->reference() + ret);
//	    cerr << "XXXXX: " << (char *)buf->reference() << endl;
 	    if (ret < static_cast<int>(cygnal::NETBUFSIZE)) {
// 		buf->resize(ret);	FIXME: why does this corrupt
// 		the buffer ?
		_que.push(buf);
		break;
 	    } else {
		_que.push(buf);
	    }
	    // ret must be more than 0 here
	    if (ret == buf->size()) {
		continue;
	    }
	} else {
	    log_debug(_("no more data for fd #%d, exiting..."), fd);
	    return 0;
	}
	if (static_cast<int>(ret) == -1) {
	    log_debug(_("Handler done for fd #%d, can't read any data..."), fd);
	  return -1;
	}
    } while (ret);
    
    // We're done. Notify the other threads the socket is closed, and tell them to die.
    log_debug(_("Done receiving data for fd #%d..."), fd);

    return ret;
}


void
HTTP::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
        
    log_debug (_("==== The HTTP header breaks down as follows: ===="));
    log_debug (_("Filespec: %s"), _filespec.c_str());
    log_debug (_("Version: %d.%d"), _version.major, _version.minor);

    std::map<string, string>::const_iterator it;
    for (it = _fields.begin(); it != _fields.end(); ++it) {
	log_debug(_("Field: \"%s\" = \"%s\""), it->first, it->second);
    }
    
    // Dump the RTMPT fields
    log_debug(_("RTMPT optional index is: "), _index);
    log_debug(_("RTMPT optional client ID is: "), _clientid);
    log_debug(_("==== ==== ===="));
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
