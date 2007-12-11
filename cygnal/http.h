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

#ifndef _HTTP_H_
#define _HTTP_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "network.h"
#include <string>
#include <map>

namespace cygnal
{
    
class HTTP : public gnash::Network
{
public:
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
        HTTP_VERSION_NOT_SUPPORTED = 505
    } http_status_e;
    typedef enum {
        OPTIONS,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT
    } http_method_e;
    struct status_codes {
        const char *code;
        const char *msg;
    };
    typedef enum {
	NONE,
	HTML,
	SWF,
	VIDEO,
	AUDIO,
	MP3,
	OSCP
    } filetype_e;
    HTTP();
    ~HTTP();
    std::string waitForGetRequest();
    std::string waitForGetRequest(Network &net);
    
    // Handle the GET request response
    bool sendGetReply(int filesize);
//    bool sendGetReply(Network &net);

    // Make copies of ourself
    HTTP &operator = (HTTP &obj);

    // These methods extract the fields in the HTTP header.
    std::string extractMethod(const char *data);
    std::string extractReferer(const char *data);
    std::string extractConnection(const char *data);
    std::string extractHost(const char *data);
    std::string extractAgent(const char *data);
    std::string extractLanguage(const char *data);
    std::string extractCharset(const char *data);
    std::string extractEncoding(const char *data);
    std::string extractTE(const char *data);

    // These methods add data to the fields in the HTTP header.
    bool clearHeader() { _header.str(""); };
    bool formatHeader(int filesize, const short type);
    bool formatHeader(const short type);
    bool formatRequest(const char *url, http_method_e req);
    bool formatMethod(const char *data);
    bool formatDate();
    bool formatReferer(const char *data);
    bool formatConnection(const char *data);
    bool formatContentLength(int filesize);
    bool formatContentType();
    bool formatContentType(filetype_e type);
    bool formatHost(const char *data);
    bool formatAgent(const char *data);
    bool formatLanguage(const char *data);
    bool formatCharset(const char *data);
    bool formatEncoding(const char *data);
    bool formatTE(const char *data);

    bool keepAlive(const char *data);
    bool keepAlive();

    // All HTTP messages are terminated with a blank line
    void terminateHeader() { _header << std::endl; };
    
    // Return the header that's been built up.
    std::string getHeader() { return _header.str(); };

    // Get the file type, so we know how to set the
    // Content-type in the header.
    filetype_e getFileType(std::string filespec);
    void dump();
private:
    filetype_e  _filetype;
    std::string _filespec;
    std::string _url;
    std::map<int, struct status_codes *> _status_codes;
    std::string _version;
    std::string _method;
    std::string _referer;
    std::string _connection;
    std::string _host;
    int         _port;
    std::string _agent;
    std::string _language;
    std::string _charset;
    std::string _encoding;
    std::string _te;
    std::stringstream _header;
};  
    
} // end of cygnal namespace

// end of _HTTP_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
