// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

#ifndef _HTTP_H_
#define _HTTP_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>

#include "amf.h"
#include "rtmp.h"
#include "handler.h"
#include "network.h"
#include "buffer.h"

namespace gnash
{
    
class HTTP
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
        HTTP_OPTIONS,
        HTTP_GET,
        HTTP_HEAD,
        HTTP_POST,
        HTTP_PUT,
        HTTP_DELETE,
        HTTP_TRACE,
        HTTP_CONNECT
    } http_method_e;
    typedef enum {
	OPEN,
	SEND,
	IDLE,
	CLOSE
    } rtmpt_cmd_e;
    struct status_codes {
        const char *code;
        const char *msg;
    };
    HTTP();
    HTTP(Handler *hand);
    ~HTTP();
    bool waitForGetRequest();
    bool waitForGetRequest(gnash::Network &net);
    
    // Handle the GET request response
    bool sendGetReply(http_status_e code);
    bool sendPostReply(rtmpt_cmd_e code);
//    bool sendGetReply(Network &net);

    // Make copies of ourself
    HTTP &operator = (HTTP &obj);

    // These methods extract data from an RTMPT message. RTMP is an
    // extension to HTTP that adds commands to manipulate the
    // connection's persistance.
    rtmpt_cmd_e extractRTMPT(gnash::Network::byte_t *data);
    rtmpt_cmd_e extractRTMPT(boost::shared_ptr<amf::Buffer> data)
	{ return extractRTMPT(data->reference()); };

    // These methods extract the fields in the HTTP header.
    // These all return the number of items found, or 0
    http_method_e extractCommand(gnash::Network::byte_t *data);
    http_method_e extractCommand(boost::shared_ptr<amf::Buffer> data)
	{ return extractCommand(data->reference()); };
    int extractAccept(gnash::Network::byte_t *data);
    int extractAccept(boost::shared_ptr<amf::Buffer> data)
	{ return extractAccept(data->reference()); };
    std::string &extractAcceptRanges(gnash::Network::byte_t *data);
    std::string &extractAcceptRanges(boost::shared_ptr<amf::Buffer> data)
	{ return extractAcceptRanges(data->reference()); };
    int extractLanguage(gnash::Network::byte_t *data);
    int extractLanguage(boost::shared_ptr<amf::Buffer> data)
	{ return extractLanguage(data->reference()); };
    int extractCharset(gnash::Network::byte_t *data);
    int extractCharset(boost::shared_ptr<amf::Buffer> data)
	{ return extractCharset(data->reference()); };
    int extractEncoding(gnash::Network::byte_t *data);
    int extractEncoding(boost::shared_ptr<amf::Buffer> data)
	{ return extractEncoding(data->reference()); };
    int extractTE(gnash::Network::byte_t *data);
    int extractTE(boost::shared_ptr<amf::Buffer> data)
	{ return extractTE(data->reference()); };
    int extractConnection(gnash::Network::byte_t *data);
    int extractConnection(boost::shared_ptr<amf::Buffer> data)
	{ return extractConnection(data->reference()); };
    int extractKeepAlive(gnash::Network::byte_t *data);
    int extractKeepAlive(boost::shared_ptr<amf::Buffer> data)
	{ return extractConnection(data->reference()); };

    // These return the string that was found for this field.
    std::string &extractMethod(gnash::Network::byte_t *data);
    std::string &extractMethod(boost::shared_ptr<amf::Buffer> data)
	{ return extractMethod(data->reference()); };
    std::string &extractReferer(gnash::Network::byte_t *data);
    std::string &extractReferer(boost::shared_ptr<amf::Buffer> data)
	{ return extractReferer(data->reference()); };
    std::string &extractHost(gnash::Network::byte_t *data);
    std::string &extractHost(boost::shared_ptr<amf::Buffer> data)
	{ return extractHost(data->reference()); };
    std::string &extractAgent(gnash::Network::byte_t *data);
    std::string &extractAgent(boost::shared_ptr<amf::Buffer> data)
	{ return extractAgent(data->reference()); };

    // These methods add data to the fields in the HTTP header.
    // These return true if OK, false if error.
    bool clearHeader();
    bool formatHeader(int filesize, http_status_e type);
    bool formatHeader(http_status_e type);
    bool formatRequest(const std::string &url, http_method_e req);
    bool formatMethod(const std::string &data);
    bool formatDate();
    bool formatServer();
    bool formatServer(const std::string &data);
    bool formatReferer(const std::string &data);
    bool formatConnection(const std::string &data);
    bool formatKeepAlive(const std::string &data);
    bool formatContentLength();
    bool formatContentLength(int filesize);
    bool formatContentType();
    bool formatContentType(amf::AMF::filetype_e type);
    bool formatHost(const std::string &data);
    bool formatAgent(const std::string &data);
    bool formatAcceptRanges(const std::string &data);
    bool formatLastModified();
    bool formatLastModified(const std::string &data);
    bool formatEtag(const std::string &data);
    bool formatLanguage(const std::string &data);
    bool formatCharset(const std::string &data);
    bool formatEncoding(const std::string &data);
    bool formatTE(const std::string &data);

    bool formatErrorResponse(http_status_e err);
    
    // All HTTP messages are terminated with a blank line
    void terminateHeader() { _header << "\r\n"; };
    
    // Return the header that's been built up.
    std::string getHeader() { return _header.str(); };

    // Return the body that's been built up.
    std::string getBody() { return _body.str(); };

    // Get the file type, so we know how to set the
    // Content-type in the header.
//    filetype_e getFileType(std::string &filespec);
    amf::AMF::filetype_e getFileStats(std::string &filespec);
    void dump();

    // These accessors are used mostly just for debugging.
    bool keepAlive() { return _keepalive; }
    int getFileSize() { return _filesize; }
    std::string &getFilespec() { return _filespec; }
    std::string &getURL() { return _url; }
    std::map<int, struct status_codes *> getStatusCodes()
	{ return _status_codes; }
    std::string getVersion() { return _version; }
    std::string getMethod() { return _method; }
    std::string getReferer() { return _referer; }
    std::vector<std::string> getLanguage() { return _language;  }
    std::vector<std::string> getConnection() { return _connections; }
    std::vector<std::string> getKeepAlive() { return _kalive; }
    std::vector<std::string> getTE() { return _te; }
    std::vector<std::string> getCharset() { return _charset; }
    std::vector<std::string> getEncoding() { return _encoding; }

    int         getHostPort(){ return _port; }
    std::string getHost() { return _host; }
    std::string getUserAgent() { return _agent; }

    void setHandler(Handler *hand) { _handler = hand; };
private:
    std::stringstream _header;
    std::stringstream _body;
    std::string _command;
    amf::AMF::filetype_e  _filetype;
    std::string _filespec;
    int         _filesize;
    std::string _url;
    std::map<int, struct status_codes *> _status_codes;
    std::string _version;
    std::string _method;
    std::string _referer;
    std::string _host;
    int         _port;
    std::string _agent;
    std::string _acceptranges;
    std::vector<std::string> _connections;
    std::vector<std::string> _language;
    std::vector<std::string> _charset;
    std::vector<std::string> _encoding;
    std::vector<std::string> _te;
    std::vector<std::string> _accept;
    std::vector<std::string> _kalive;
    // Connection parameters we care about
    bool	_keepalive;
    Handler     *_handler;
    // These two field hold the data from an RTMPT message
    int	        _clientid;
    int	        _index;
//    bool	_te;
};  

// This is the thread for all incoming HTTP connections
extern "C" {
    void httphandler(Handler::thread_params_t *args);
}


} // end of gnash namespace

// end of _HTTP_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
