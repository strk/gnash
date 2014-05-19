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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/thread/mutex.hpp>
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
#include "log.h"
#include "crc.h"
#include "network.h"
//#include "handler.h"
#include "utility.h"
#include "buffer.h"
#include "http.h"
#include "diskstream.h"

// Cygnal specific headers
#include "http_server.h"
#include "proc.h"
#include "cache.h"

// Not POSIX, so best not rely on it if possible.
#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

using namespace gnash;
using namespace std;

static boost::mutex stl_mutex;

namespace cygnal
{

// The rcfile is loaded and parsed here:
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();
static Cache& cache = Cache::getDefaultInstance();
// static Proc& cgis = Proc::getDefaultInstance();

HTTPServer::HTTPServer() 
{
//    GNASH_REPORT_FUNCTION;
}

HTTPServer::~HTTPServer()
{
//    GNASH_REPORT_FUNCTION;
}

HTTP::http_method_e
HTTPServer::processClientRequest(int /* fd */)
{
    GNASH_REPORT_FUNCTION;
    
    //cygnal::Buffer *buf = new cygnal::Buffer;
    // return processClientRequest(fd, buf);
    
    return HTTP::HTTP_NONE;
}

HTTP::http_method_e
HTTPServer::processClientRequest(Handler *hand, int fd, cygnal::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;
    
    cygnal::Buffer result;

    if (buf) {
	_cmd = extractCommand(buf->reference());
	switch (_cmd) {
	  case HTTP::HTTP_GET:
	      result = processGetRequest(hand, fd, buf);
	      break;
	  case HTTP::HTTP_POST:
	      result = processPostRequest(fd, buf);
	      break;
	  case HTTP::HTTP_HEAD:
	      result = processHeadRequest(fd, buf);
	      break;
	  case HTTP::HTTP_CONNECT:
	      result = processConnectRequest(fd, buf);
	      break;
	  case HTTP::HTTP_TRACE:
	      result = processTraceRequest(fd, buf);
	      break;
	  case HTTP::HTTP_OPTIONS:
	      result = processOptionsRequest(fd, buf);
	      break;
	  case HTTP::HTTP_PUT:
	      result = processPutRequest(fd, buf);
	      break;
	  case HTTP::HTTP_DELETE:
	      result = processDeleteRequest(fd, buf);
	      break;
	  default:
	      break;
	}
    }

#if 0

    writeNet(fd, result);

    _docroot = crcfile.getDocumentRoot();
    
    string url = _docroot + _filespec;
    
    // See if the file is in the cache and already opened.
    std::shared_ptr<DiskStream> filestream(cache.findFile(_filespec));
    if (filestream) {
	log_debug("FIXME: found filestream %s in cache!", _filespec);
	filestream->dump();
    } else {
	filestream.reset(new DiskStream);
	log_network(_("New filestream %s"), _filespec);
	// cache.addFile(url, filestream);	FIXME: always reload from disk for now.
	
	// Oopen the file and read the first chunk into memory
	if (filestream->open(url)) {
	    formatErrorResponse(HTTPServer::NOT_FOUND);
	} else {
	    // Get the file size for the HTTPServer header
	    if (filestream->getFileType() == DiskStream::FILETYPE_NONE) {
		formatErrorResponse(HTTPServer::NOT_FOUND);
	    } else {
		// cache.addPath(_filespec, filestream->getFilespec());
		// cache.addFile(_filespec, filestream);
	    }
	}
	// Close the file but leave resident for now.
	if (filestream->fullyPopulated()) {
	    filestream->close();
	}
//  	cache.addFile(_filespec, filestream);
    }
#endif
    
    return _cmd;
}

// A GET request asks the server to send a file to the client
cygnal::Buffer &
HTTPServer::processGetRequest(Handler *hand, int fd, cygnal::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;

    // cerr << "QUE = " << _que.size() << endl;
    
//    cerr << "YYYYYYY: " << (char *)buf->reference() << endl;
//    cerr << hexify(buf->reference(), buf->allocated(), false) << endl;
    
    if (buf == 0) {
     //	log_debug("Queue empty, net connection dropped for fd #%d", getFileFd());
	log_debug("Queue empty, net connection dropped for fd #%d", fd);
//	cygnal::Buffer buf;
	return _buf;
    }
    
    clearHeader();
    processHeaderFields(buf);

    _docroot = crcfile.getDocumentRoot();
    
    string url = _docroot + _filespec;

    std::shared_ptr<DiskStream> ds = hand->getDiskStream(fd);
    if (ds) {
	_diskstream = ds;
    }
    if (!_diskstream) {
	_diskstream.reset(new DiskStream);
	log_network(_("New filestream %s"), _filespec);
    } else {
	log_network(_("Reusing filestream %s"), _filespec);
    }
    
    // Oopen the file and read the first chunk into memory
    if (_diskstream->open(url)) {
	formatErrorResponse(HTTPServer::NOT_FOUND);
    } else {
	// Get the file size for the HTTPServer header
	if (_diskstream->getFileType() == DiskStream::FILETYPE_NONE) {
	    formatErrorResponse(HTTPServer::NOT_FOUND);
	} else {
	    // cache.addPath(_filespec, filestream->getFilespec());
	    // cache.addFile(_filespec, filestream);
	}
    }
    // Closing the file closes the disk file, but leaves data resident
    // in memory for future access to this file. If we've been opened,
    // the next operation is to start writing the file next time
    // ::play() is called.
    if (_diskstream->fullyPopulated()) {
	_diskstream->close();
    }
    _diskstream->setState(DiskStream::PLAY);
// 	cache.addFile(_filespec, _diskstream);

    // Create the reply message
//     _close = true; Force sending the close connection in the header
    cygnal::Buffer &reply = formatHeader(_diskstream->getFileType(),
				      _diskstream->getFileSize(),
				      HTTPServer::OK);

    writeNet(fd, reply);

    size_t filesize = _diskstream->getFileSize();
    // size_t bytes_read = 0;
    // int ret;
    // size_t page = 0;
    if (filesize) {
#ifdef USE_STATS_CACHE
	struct timespec start;
	clock_gettime (CLOCK_REALTIME, &start);
#endif	
	
#ifdef USE_STATS_CACHE
	struct timespec end;
	clock_gettime (CLOCK_REALTIME, &end);
	double time = (end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9);
	ios::fmtflags f(cerr.flags());
	cerr << "File " << _filespec
	     << " transferred " << filesize << " bytes in: " << fixed
	     << time << " seconds for net fd #" << fd << endl;
	cerr.flags(f);
#endif
    }
    
    return reply;
}

// A POST request asks sends a data from the client to the server. After processing
// the header like we normally do, we then read the amount of bytes specified by
// the "content-length" field, and then write that data to disk, or decode the amf.
std::shared_ptr<cygnal::Buffer>
HTTPServer::processPostRequest(int fd, cygnal::Buffer * /* bufFIXME */)
{
    GNASH_REPORT_FUNCTION;

//    cerr << "QUE1 = " << _que.size() << endl;

    std::shared_ptr<cygnal::Buffer> buf;
    
    if (_que.size() == 0) {
	return buf;
    }
    
    buf = _que.pop();
    if (buf == 0) {
	log_debug("Queue empty, net connection dropped for fd #%d",
		  getFileFd());
	return buf;
    }
//    cerr << __FUNCTION__ << buf->allocated() << " : " << hexify(buf->reference(), buf->allocated(), true) << endl;
    
    clearHeader();
    boost::uint8_t *data = processHeaderFields(buf.get());
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
	if (ret < 0) {
	    log_error(_("couldn't read data!"));
	}
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
	std::shared_ptr<cygnal::Element> el = amf.extractAMF(content.reference(), content.end());
	el->dump();		// FIXME: do something intelligent
				// with this Element
#endif
    }
    
    // Send the reply

    // NOTE: this is a "special" path we trap until we have real CGI support
    if ((getField("content-type") == "application/x-amf")
	&& (getField("content-type") == "application/x-amf")) {
#ifdef USE_CGIBIN
	if (_filespec == "/echo/gateway") {
	}
	
	Proc cgis;
	string path = _docroot;
	path += _filespec;
  	cgis.startCGI(_filespec, true, CGIBIN_PORT);
 	cgis.createClient("localhost", CGIBIN_PORT);
	cgis.writeNet(*content);
	std::shared_ptr<cygnal::Buffer> reply = cgis.readNet();
	
	writeNet(fd, *reply);
//	cgis.stopCGI(_filespec);
#else
	vector<std::shared_ptr<cygnal::Element> > headers = parseEchoRequest(*content);
  	//std::shared_ptr<cygnal::Element> &el0 = headers[0];

	if (headers.size() >= 4) {
	    if (headers[3]) {
		cygnal::Buffer &reply = formatEchoResponse(headers[1]->getName(), *headers[3]);
// 	    cerr << "FIXME 3: " << hexify(reply.reference(), reply.allocated(), true) << endl;
// 	    cerr << "FIXME 3: " << hexify(reply.reference(), reply.allocated(), false) << endl;
		writeNet(fd, reply);
	    }
 	}
#endif
    } else {
	cygnal::Buffer &reply = formatHeader(_filetype, _filesize, HTTPServer::OK);
	writeNet(fd, reply);
    }

    return buf;
}

std::shared_ptr<cygnal::Buffer>
HTTPServer::processPutRequest(int /* fd */, cygnal::Buffer */* buf */)
{
    std::shared_ptr<cygnal::Buffer> buf;
//    GNASH_REPORT_FUNCTION;
    log_unimpl(_("PUT request"));

    return buf;
}

std::shared_ptr<cygnal::Buffer>
HTTPServer::processDeleteRequest(int /* fd */, cygnal::Buffer */* buf */)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> buf;
    log_unimpl(_("DELETE request"));
    
    return buf;
}

std::shared_ptr<cygnal::Buffer>
HTTPServer::processConnectRequest(int /* fd */, cygnal::Buffer */* buf */)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> buf;
    log_unimpl(_("CONNECT request"));

    return buf;
}

std::shared_ptr<cygnal::Buffer>
HTTPServer::processOptionsRequest(int /* fd */, cygnal::Buffer */* buf */)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> buf;
    log_unimpl(_("OPTIONS request"));

    return buf;
}

std::shared_ptr<cygnal::Buffer>
HTTPServer::processHeadRequest(int /* fd */, cygnal::Buffer */* buf */)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> buf;
    log_unimpl(_("HEAD request"));
    
    return buf;
}

std::shared_ptr<cygnal::Buffer>
HTTPServer::processTraceRequest(int /* fd */, cygnal::Buffer */* buf */)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> buf;
    log_unimpl(_("TRACE request"));
    
    return buf;
}

cygnal::Buffer &
HTTPServer::formatErrorResponse(http_status_e code)
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

    // All HTTPServer messages are followed by a blank line.
    terminateHeader();

    return _buffer;
}

cygnal::Buffer &
HTTPServer::formatGetReply(http_status_e code)
{

//    GNASH_REPORT_FUNCTION;
    
    return formatHeader(_filesize, code);
}

cygnal::Buffer &
HTTPServer::formatGetReply(size_t size, http_status_e code)
{
//    GNASH_REPORT_FUNCTION;
    
    formatHeader(size, code);
    
//    int ret = Network::writeNet(_header.str());    
//    boost::uint8_t *ptr = (boost::uint8_t *)_body.str().c_str();
//     buf->copy(ptr, _body.str().size());
//    _handler->dump();

#if 0
    if (_header.str().size()) {
        log_debug ("Sent GET Reply");
	return _buffer;
    } else {
	clearHeader();
	log_debug ("Couldn't send GET Reply, no header data");
    }    
#endif
    
    return _buffer;
}

cygnal::Buffer &
HTTPServer::formatPostReply(rtmpt_cmd_e /* code */)
{
    GNASH_REPORT_FUNCTION;

    formatDate();
    formatServer();
    formatContentType(DiskStream::FILETYPE_AMF);
    // All HTTPServer messages are followed by a blank line.
    terminateHeader();
    return _buffer;

#if 0
    formatHeader(_filesize, code);
    std::shared_ptr<cygnal::Buffer> buf = new cygnal::Buffer;
    if (_header.str().size()) {
	buf->resize(_header.str().size());
	string str = _header.str();
	buf->copy(str);
	_handler->pushout(buf);
	_handler->notifyout();
        log_debug ("Sent GET Reply");
	return true; // Default to true
    } else {
	clearHeader();
	log_debug ("Couldn't send POST Reply, no header data");
    }
#endif

    return _buffer;
}

#ifndef USE_CGIBIN
// Parse an Echo Request message coming from the Red5 echo_test. This
// method should only be used for testing purposes.
vector<std::shared_ptr<cygnal::Element > >
HTTPServer::parseEchoRequest(boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    
    vector<std::shared_ptr<cygnal::Element > > headers;
	
    // skip past the header bytes, we don't care about them.
    boost::uint8_t *tmpptr = data + 6;
    
    boost::uint16_t length;
    length = ntohs((*(boost::uint16_t *)tmpptr) & 0xffff);
    tmpptr += sizeof(boost::uint16_t);

    // Get the first name, which is a raw string, and not preceded by
    // a type byte.
    std::shared_ptr<cygnal::Element > el1(new cygnal::Element);
    
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
    std::shared_ptr<cygnal::Element > el2(new cygnal::Element);

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
    std::shared_ptr<cygnal::Element> el3 = amf.extractAMF(tmpptr, tmpptr + size);
    headers.push_back(el3);
    tmpptr += amf.totalsize();
    
    std::shared_ptr<cygnal::Element> el4 = amf.extractAMF(tmpptr, tmpptr + size);
    headers.push_back(el4);

     return headers;
}

// format a response to the 'echo' test used for testing Gnash. This
// is only used for testing by developers. The format appears to be
// two strings, followed by a double, followed by the "onResult".
cygnal::Buffer &
HTTPServer::formatEchoResponse(const std::string &num, cygnal::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> data;

    cygnal::Element nel;
    if (el.getType() == cygnal::Element::TYPED_OBJECT_AMF0) {
	nel.makeTypedObject();
	string name = el.getName();
	nel.setName(name);
	if (el.propertySize()) {
	    // FIXME: see about using std::reverse() instead.
	    for (int i=el.propertySize()-1; i>=0; i--) {
// 	    for (int i=0 ; i<el.propertySize(); i++) {
		std::shared_ptr<cygnal::Element> child = el.getProperty(i);
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

cygnal::Buffer &
HTTPServer::formatEchoResponse(const std::string &num, cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatEchoResponse(num, data.reference(), data.allocated());
}

cygnal::Buffer &
HTTPServer::formatEchoResponse(const std::string &num, boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    //boost::uint8_t *tmpptr  = data;
    
    // FIXME: temporary hacks while debugging
    cygnal::Buffer fixme("00 00 00 00 00 01");
    cygnal::Buffer fixme2("ff ff ff ff");
    
    _buffer = "HTTPServer/1.1 200 OK\r\n";
    formatContentType(DiskStream::FILETYPE_AMF);
//    formatContentLength(size);
    // FIXME: this is a hack ! Calculate a real size!
    formatContentLength(size+29);
    
    // Don't pretend to be the Red5 server
    formatServer("Cygnal (0.8.6)");
    
    // All HTTPServer messages are followed by a blank line.
    terminateHeader();

    // Add the binary blob for the header
    _buffer += fixme;

    // Make the result response, which is the 2nd data item passed in
    // the request, a slash followed by a number like "/2".
    string result = num;
    result += "/onResult";
    std::shared_ptr<cygnal::Buffer> res = amf::AMF::encodeString(result);
    _buffer.append(res->begin()+1, res->size()-1);

    // Add the null data item
    std::shared_ptr<cygnal::Buffer> null = amf::AMF::encodeString("null");
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
#endif

#if 0
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

#endif

/// These methods extract data from an RTMPT message. RTMP is an
/// extension to HTTPServer that adds commands to manipulate the
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
HTTPServer::rtmpt_cmd_e
HTTPServer::extractRTMPT(boost::uint8_t *data)
{
    GNASH_REPORT_FUNCTION;

    string body = reinterpret_cast<const char *>(data);
    string tmp, cid, indx;
    HTTPServer::rtmpt_cmd_e cmd;

    // force the case to make comparisons easier
    std::transform(body.begin(), body.end(), body.begin(), 
               (int(*)(int)) toupper);
    string::size_type start, end;

    // Extract the command first
    start = body.find("OPEN", 0);
    if (start != string::npos) {
        cmd = HTTPServer::OPEN;
    }
    start = body.find("SEND", 0);
    if (start != string::npos) {
        cmd = HTTPServer::SEND;
    }
    start = body.find("IDLE", 0);
    if (start != string::npos) {
        cmd = HTTPServer::IDLE;
    }
    start = body.find("CLOSE", 0);
    if (start != string::npos) {
        cmd = HTTPServer::CLOSE;
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
//         cmd = HTTPServer::CLOSE;
//     }

    return cmd;
}

#if 0
HTTPServer::http_method_e
HTTPServer::extractCommand(boost::uint8_t *data)
{
    GNASH_REPORT_FUNCTION;

//    string body = reinterpret_cast<const char *>(data);
    HTTPServer::http_method_e cmd = HTTP::HTTP_NONE;

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
	// log_debug("Requesting file: \"%s\"", _filespec);

	// The third field is always the HTTP version
	// The version is the last field and is the protocol name
	// followed by a slash, and the version number. Note that
	// the version is not a double, even though it has a dot
	// in it. It's actually two separate integers.
	_version.major = *(end+6) - '0';
	_version.minor = *(end+8) - '0';
	// log_debug("Version: %d.%d", _version.major, _version.minor);
    }

    return cmd;
}

boost::uint8_t *
HTTPServer::processHeaderFields(cygnal::Buffer &buf)
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
		    log_debug("Setting Max Requests for Keep-Alive to %d",
			      _max_requests);
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
		    // log_debug ("Version: %d.%d", _version.major, _version.minor);
		    // the filespec in the request is the middle field, deliminated
		    // by a space on each end.
		    if (params != string::npos) {
			_params = i->substr(params+1, end);
			_filespec = i->substr(start+1, params);
			log_debug("Parameters for file: \"%s\"", _params);
		    } else {
			_filespec = i->substr(start+1, pos-start-2);
		    }
		    // log_debug("Requesting file: \"%s\"", _filespec);

		    // HTTP 1.1 enables persistent network connections
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
#endif

void
HTTPServer::dump()
{
//    GNASH_REPORT_FUNCTION;
    if (_diskstream) {
	_diskstream->dump();
    }
}
    
bool
HTTPServer::http_handler(Handler *hand, int netfd, cygnal::Buffer *buf)
{
    GNASH_REPORT_FUNCTION;

    // Handler *hand = reinterpret_cast<Handler *>(args->handler);
    // cygnal::Buffer *buf = args->buffer;
    // std::shared_ptr<HTTPServer> www(new HTTPServer); // = hand->getHTTPHandler    (args->netfd);
    
    string url, parameters;
    // by default, only look once unless changed later
    // www->setDocRoot(crcfile.getDocumentRoot());
    // log_network("Docroot for HTTP files is %s", crcfile.getDocumentRoot());
    log_network(_("Processing HTTP data for fd #%d"), netfd);
    
    // Wait for data, and when we get it, process it.
#ifdef USE_STATISTICS
    struct timespec start;
    clock_gettime (CLOCK_REALTIME, &start);
#endif

    if (buf) {
	log_network(_("FIXME: Existing data in packet!"));
    } else {
	log_network(_("FIXME: No existing data in packet!"));
	// See if we have any messages waiting
	if (recvMsg(netfd) == 0) {
	    log_debug("Net HTTP server failed to read from fd #%d...", netfd);
	    return false;
	}
    }
    
    // Process incoming messages
    HTTP::http_method_e cmd = processClientRequest(hand, netfd, buf);    
    if (cmd != HTTP::HTTP_GET) {
	log_debug("No active DiskStreams for fd #%d: %s...", netfd,
		  _filespec);
    } else {
	if (_diskstream) {
	    log_debug("Found active DiskStream! for fd #%d: %s", netfd,
		      _filespec);
	    hand->setDiskStream(netfd, _diskstream);
 	    cache.addFile(_filespec, _diskstream);
// Send the first chunk of the file to the client.
	    // log_network("Sending first chunk of %s", _filespec);
	    _diskstream->play(netfd, false);
	}
    }
    
//	www->dump();
    if ((getField("content-type") == "application/x-amf")
	&& (getField("content-type") == "application/x-amf")
	&& (getFilespec() == "/echo/gateway")) {
	cerr << "GOT A GATEWAY REQUEST" << endl;
    }
    
#if 0
    string response = cache.findResponse(filestream->getFilespec());
    if (response.empty()) {
	cerr << "FIXME no cache hit for: " << www.getFilespec() << endl;
//	    www.clearHeader();
// 	    cygnal::Buffer &ss = www.formatHeader(filestream->getFileSize(), HTTP::LIFE_IS_GOOD);
// 	    www.writeNet(args->netfd, (boost::uint8_t *)www.getHeader().c_str(), www.getHeader().size());
// 	    cache.addResponse(www.getFilespec(), www.getHeader());
    } else {
	cerr << "FIXME cache hit on: " << www.getFilespec() << endl;
	www.writeNet(args->netfd, (boost::uint8_t *)response.c_str(), response.size());
    }	
#endif
    
    // Unless the Keep-Alive flag is set, this isn't a persisant network
    // connection.
    if (!keepAlive()) {
	log_debug("Keep-Alive is off", keepAlive());
    } else {
	log_debug("Keep-Alive is on", keepAlive());
    }
#ifdef USE_STATISTICS
    struct timespec end;
    clock_gettime (CLOCK_REALTIME, &end);
    log_debug("Processing time for GET request was %f seconds",
	      static_cast<float>(((end.tv_sec - start.tv_sec) +
				  ((end.tv_nsec - start.tv_nsec)/1e9))));
#endif
    
    return keepAlive();
    
} // end of http_handler
    
} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
