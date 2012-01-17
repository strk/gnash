// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_LIBNET_HTTP_H
#define GNASH_LIBNET_HTTP_H

#include <string>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "amf.h"
#include "cque.h"
#include "rtmp.h"
//#include "handler.h"
#include "network.h"
#include "buffer.h"
#include "diskstream.h"

namespace gnash
{
    
class DSOEXPORT HTTP : public gnash::Network
{
public:
// as defined by the W3: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
    typedef enum {
        // 1xx: Informational - Request received, continuing process
        CONTINUE = 100,
        SWITCHPROTOCOLS = 101,
        // 2xx: Success - The action was successfully received,
        // understood, and accepted
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        NON_AUTHORITATIVE = 203,
        NO_CONTENT = 204,
        RESET_CONTENT = 205,
        PARTIAL_CONTENT = 206,
        // 3xx: Redirection - Further action must be taken in order to
        // complete the request
        MULTIPLE_CHOICES = 300,
        MOVED_PERMANENTLY = 301,
        FOUND = 302,
        SEE_OTHER = 303,
        NOT_MODIFIED = 304,
        USE_PROXY = 305,
        TEMPORARY_REDIRECT = 307,
        // 4xx: Client Error - The request contains bad syntax or
        // cannot be fulfilled
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        PAYMENT_REQUIRED = 402,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        NOT_ACCEPTABLE = 406,
        PROXY_AUTHENTICATION_REQUIRED = 407,
        REQUEST_TIMEOUT = 408,
        CONFLICT = 409,
        GONE = 410,
        LENGTH_REQUIRED = 411,
        PRECONDITION_FAILED = 412,
        REQUEST_ENTITY_TOO_LARGE = 413,
        REQUEST_URI_TOO_LARGE = 414,
        UNSUPPORTED_MEDIA_TYPE = 415,
        REQUESTED_RANGE_NOT_SATISFIABLE = 416,
        EXPECTATION_FAILED = 417,
        // 5xx: Server Error - The server failed to fulfill an apparently valid request
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503,
        GATEWAY_TIMEOUT = 504,
        HTTP_VERSION_NOT_SUPPORTED = 505,
	// Gnash/Cygnal extensions for internal use
	LIFE_IS_GOOD = 1234,
	CLOSEPIPE = 1235
    } http_status_e;
    typedef enum {
	HTTP_NONE,
        HTTP_OPTIONS,
        HTTP_GET,
        HTTP_HEAD,
        HTTP_POST,
        HTTP_PUT,
        HTTP_DELETE,
        HTTP_TRACE,
        HTTP_CONNECT,
	HTTP_RESPONSE		// unique to gnash
    } http_method_e;
    typedef enum {
	OPEN,
	SEND,
	IDLE,
	CLOSE
    } rtmpt_cmd_e;
    // A response from an FTTP request has a code an an error message
    typedef struct {
	http_status_e code;
	std::string   msg;
    } http_response_t;
    typedef struct {
	int major;
	int minor;
    } http_version_t;
    HTTP();
//     HTTP(Handler *hand);
    ~HTTP();

    // Check the Header fields to make sure they're valid values.
    bool checkRequestFields(cygnal::Buffer &buf);
    bool checkEntityFields(cygnal::Buffer &buf);
    bool checkGeneralFields(cygnal::Buffer &buf);

//     // Parse an Echo Request message coming from the Red5 echo_test.
    std::vector<boost::shared_ptr<cygnal::Element > > parseEchoRequest(cygnal::Buffer &buf) { return parseEchoRequest(buf.reference(), buf.size()); };
    std::vector<boost::shared_ptr<cygnal::Element > > parseEchoRequest(boost::uint8_t *buf, size_t size);
    
    // Convert the Content-Length field to a number we can use
    size_t getContentLength();

    // process all the header fields in the Buffer, storing them internally
    // in _fields. The address returned is the address where the Content data
    // starts, and is "Content-Length" bytes long, of "Content-Type" data.
    boost::uint8_t *processHeaderFields(cygnal::Buffer *buf);
    
    // Get the field for header 'name' that was stored by processHeaderFields()
    std::string &getField(const std::string &name) { return _fields[name]; };
    size_t NumOfFields() { return _fields.size(); };
    void clearFields() { _fields.clear(); };
    std::map<std::string, std::string> &getFields() { return _fields; };

    // Get an array of values for header field 'name'.
    boost::shared_ptr<std::vector<std::string> > getFieldItem(const std::string &name);

    // Client side parsing of response message codes
    boost::shared_ptr<http_response_t> parseStatus(const std::string &line);

    // Handle the response for the request.
    boost::shared_ptr<cygnal::Buffer> formatServerReply(http_status_e code);
    cygnal::Buffer &formatGetReply(DiskStream::filetype_e type, size_t size, http_status_e code); 
    cygnal::Buffer &formatGetReply(size_t size, http_status_e code); 
    cygnal::Buffer &formatGetReply(http_status_e code); 
    cygnal::Buffer &formatPostReply(rtmpt_cmd_e code);

    // Make copies of ourself
    HTTP &operator = (HTTP &obj);

    /// @note These methods add data to the fields in the HTTP header.
    /// \brief clear the data in the stored header
    bool clearHeader();

    /// \brief Start constructing a new HTTP header.
    ///		As it's hard to predict how much storage to allocate,
    ///		all of these methods for formatting  HTTP header
    ///		fields store the header while adding data to it. It
    ///		requires another function to actually send the data.
    bool startHeader();
    
    /// \brief Format the common header fields that need no other processing.
    ///		Most of these fields are purely ASCII based, and so
    ///		chare a common constructor. A few require formatting
    ///		of numerical data into string data, so they can't use
    ///		the common form.
    cygnal::Buffer &formatCommon(const std::string &data);

    cygnal::Buffer &formatHeader(DiskStream::filetype_e type, size_t filesize,
			    http_status_e code);
    cygnal::Buffer &formatHeader(size_t filesize, http_status_e type);
    cygnal::Buffer &formatHeader(http_status_e type);
    cygnal::Buffer &formatRequest(const std::string &url, http_method_e req);
    // format a response to the 'echo' test used for testing Gnash.
    cygnal::Buffer &formatEchoResponse(const std::string &num, cygnal::Element &el);
    cygnal::Buffer &formatEchoResponse(const std::string &num, cygnal::Buffer &data);
    cygnal::Buffer &formatEchoResponse(const std::string &num, boost::uint8_t *data, size_t size);

    cygnal::Buffer &formatMethod(const std::string &data)
 	{return formatCommon("Method: " + data); };
    cygnal::Buffer &formatDate();
    cygnal::Buffer &formatServer();
    cygnal::Buffer &formatServer(const std::string &data);
    cygnal::Buffer &formatReferer(const std::string &data)
 	{return formatCommon("Referer: " + data); };
    cygnal::Buffer &formatConnection(const std::string &data)
 	{return formatCommon("Connection: " + data); };
    cygnal::Buffer &formatKeepAlive(const std::string &data)
 	{return formatCommon("Keep-Alive: " + data); };
    cygnal::Buffer &formatContentLength();
    cygnal::Buffer &formatContentLength(boost::uint32_t filesize);
    cygnal::Buffer &formatContentType();
    cygnal::Buffer &formatContentType(DiskStream::filetype_e type);
    cygnal::Buffer &formatHost(const std::string &data)
 	{return formatCommon("Host: " + data); };
    cygnal::Buffer &formatAgent(const std::string &data)
 	{return formatCommon("User-Agent: " + data); };
    cygnal::Buffer &formatAcceptRanges(const std::string &data)
 	{return formatCommon("Accept-Ranges: " + data); };
    cygnal::Buffer &formatLastModified();
    cygnal::Buffer &formatLastModified(const std::string &data)
 	{return formatCommon("Last-Modified: " + data); }
    cygnal::Buffer &formatEtag(const std::string &data)
 	{return formatCommon("Etag: " + data); };
    cygnal::Buffer &formatLanguage(const std::string &data)
 	{return formatCommon("Accept-Language: " + data); };
    cygnal::Buffer &formatCharset(const std::string &data)
 	{return formatCommon("Accept-Charset: " + data); };
    cygnal::Buffer &formatEncoding(const std::string &data)
 	{return formatCommon("Accept-Encoding: " + data); };
    cygnal::Buffer &formatTE(const std::string &data)
 	{return formatCommon("TE: " + data); };
    // All HTTP messages are terminated with a blank line
    void terminateHeader() { _buffer += "\r\n"; };    
    
//     cygnal::Buffer &formatErrorResponse(http_status_e err);
    
    // Return the header that's been built up.
    boost::uint8_t *getHeader() { return _buffer.reference(); };

    // Return the header that's been built up.
    cygnal::Buffer &getBuffer() { return _buffer; };

//     // Return the body that's been built up.
//     std::string getBody() { return _body.str(); };

    // Get the file type, so we know how to set the
    // Content-type in the header.
//    filetype_e getFileType(std::string &filespec);
//    amf::AMF::filetype_e getFileStats(std::string &filespec);
    void dump();

    /// \brief Receive a message from the other end of the network connection.
    ///
    /// @param fd The file descriptor to read from
    ///
    /// @return The number of bytes sent
    int recvMsg(int fd);
    int recvMsg(int fd, size_t size);

    size_t recvChunked(boost::uint8_t *data, size_t size);
    
    /// \brief Send a message to the other end of the network connection.
    ///
    /// @param data A real pointer to the data.
    /// @param size The number of bytes of data stored.
    /// @param buf A smart pointer to a Buffer class.
    /// @param sstr A smart pointer to a Buffer class.
    /// @param fd The file descriptor to use for writing to the network.
    /// @param void Send the contents of the _header and _body.
    ///
    /// @return The number of bytes sent
    int sendMsg();
    int sendMsg(int fd);
    int sendMsg(const boost::uint8_t *data, size_t size);
    int sendMsg(boost::shared_ptr<cygnal::Buffer> &buf)
	{ return sendMsg(buf->reference(), buf->size()); };
    int sendMsg(std::stringstream &sstr)
	{ return sendMsg(reinterpret_cast<const boost::uint8_t *>(sstr.str().c_str()), sstr.str().size()); };
    
    // These accessors are used mostly just for debugging.
    bool keepAlive() { return _keepalive; }
    void keepAlive(bool x) { _keepalive = x; };
    
    int getMaxRequests() { return _max_requests; }
    int getFileSize() { return _filesize; }
    std::string &getFilespec() { return _filespec; }
    std::string &getParams() { return _params; }
  //    std::string &getURL() { return _url; }
    std::map<int, struct status_codes *> getStatusCodes()
	{ return _status_codes; }
    http_version_t *getVersion() { return &_version; }
    
//     void setHandler(Handler *hand) { _handler = hand; };
    void setDocRoot(const std::string &path) { _docroot = path; };
    std::string &getDocRoot() { return _docroot; };
    
    // Pop the first date element off the que
    boost::shared_ptr<cygnal::Buffer> DSOEXPORT popChunk() { return _que.pop(); };
    // Peek at the first date element witjhout removing it from the que
    boost::shared_ptr<cygnal::Buffer> DSOEXPORT peekChunk() { return _que.peek(); };
    // Get the number of elements in the que
    size_t DSOEXPORT sizeChunks() { return _que.size(); };

    boost::shared_ptr<cygnal::Buffer> DSOEXPORT mergeChunks() { return _que.merge(); };

    http_method_e getOperation() { return _cmd; };
    
protected:
    // Examine the beginning of the data for an HTTP request command
    // like GET or POST, etc...
    http_method_e extractCommand(boost::uint8_t *data);
    http_method_e extractCommand(cygnal::Buffer &data)
	{ return extractCommand(data.reference()); };    

    typedef boost::char_separator<char> Sep;
    typedef boost::tokenizer<Sep> Tok;
    http_method_e	_cmd;

    cygnal::Buffer		_buffer;
    CQue		_que;
    
    DiskStream::filetype_e  _filetype;
    std::string		_filespec;
    std::string		_params;
    boost::uint32_t     _filesize;
    std::map<int, struct status_codes *> _status_codes;
    
    std::map<std::string, std::string> _fields;
    http_version_t	_version;
    
    // Connection parameters we care about
    bool		_keepalive;
//     Handler		*_handler;
    // These two field hold the data from an RTMPT message
    int			_clientid;
    int			_index;
    int			_max_requests;
    std::string		_docroot;

    bool		_close;
};  

// This is the thread for all incoming HTTP connections for the server
extern "C" {
    bool DSOEXPORT http_handler(Network::thread_params_t *args);
}


} // end of gnash namespace

// end of _HTTP_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
