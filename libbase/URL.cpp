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

#include "URL.h"
#include "log.h"

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cassert>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <boost/tokenizer.hpp>
#include <cctype>

// This is for getcwd(2) 

#if !defined(_WIN32) && !defined(WIN32)
# include <unistd.h>
#else
# include <direct.h>
#endif

#include "GnashException.h"

namespace gnash {

URL::URL(const std::string& relative_url, const URL& baseurl)
{
    init_relative(relative_url, baseurl);
}

// only for UNIX
void
URL::normalize_path(std::string& path)
{
#if defined(_WIN32) || defined(WIN32) || defined(__OS2__) || defined(__amigaos4__)
    return;
#endif

    if (path.empty() || path[0] != '/') {
        throw gnash::GnashException("invalid url");
    }
    
    std::vector<std::string> components;
    
    std::string::iterator prev=path.begin();
    for (std::string::iterator curr = prev + 1;
         curr != path.end();
         ++curr ) {
        if ( *curr == '/') {
            std::string comp = std::string(prev+1, curr);
            //cerr << "comp:" << comp << endl;
            prev = curr;
            
            if ( comp == "" || comp == "." ) continue;
            if ( comp == ".." && components.size() )
                components.pop_back();
            else components.push_back(comp);
        }
    }
    // add last component 
    components.emplace_back(prev+1, path.end());
    
    path = "";
    for (std::vector<std::string>::const_iterator i=components.begin(),
             e=components.end();
         i!=e; ++i) {
        path += "/" + *i;
    }
}

void
URL::init_absolute(const std::string& in)
{
    // Find protocol
    std::string::size_type pos = in.find("://");
    if ( pos != std::string::npos ) {
        // copy initial part to protocol
        _proto = in.substr(0, pos);
        
        // advance input pointer to past the :// part
        pos += 3;
        if ( pos == in.size() ) {
            std::cerr << "protocol-only url!" << std::endl;
            throw gnash::GnashException("protocol-only url");
        }
        
        // Find host 
        std::string::size_type pos1 = in.find('/', pos);
        if ( pos1 == std::string::npos ) {
            // no slashes ? all hostname, I presume
            _host = in.substr(pos);
            _path = "/";
            
            // Extract the port number from the hostname, if any
            split_port_from_host();
            
            return;
        }
        
        // copy hostname
        _host = in.substr(pos, pos1-pos);
        
        // next come path
        _path = in.substr(pos1);
    } else {
        _proto = "file";
        _path = in;
    }
    
    // Extract anchor from path, if any
    split_anchor_from_path();
    
    // Extract the port number from the hostname, if any
    split_port_from_host();

    split_querystring_from_path();

    normalize_path(_path);
}


URL::URL(const std::string& absolute_url)
{
    // std::cerr << "URL(" << absolute_url << ")" << std::endl;
    if ( ( absolute_url.size() && absolute_url[0] == '/' )
         || absolute_url.find("://") != std::string::npos 
         || ( absolute_url.size() > 1 && absolute_url[1] == ':' )        //for win32
         || ( absolute_url.size() > 2 && absolute_url.find(':',2) != std::string::npos ) //for aos4
        ) {
        //std::cerr << "It's absolute" << std::endl;
        init_absolute(absolute_url);
        
    } else {
        const size_t incr = 1024;
        // When does it get silly?
        const size_t maxSize = 4096; 
        
        std::unique_ptr<char[]> buf; 
        char* dir = nullptr;
        size_t bufSize = 0;
        
        // This just assumes a failure in getcwd is a name-length error,
        // though that perhaps isn't the case.
        while (!dir) {
            bufSize += incr;
            buf.reset(new char[bufSize]);
            dir = getcwd(buf.get(), bufSize);
            if (bufSize >= maxSize) break;
        }
        
        if (!dir) {
            std::stringstream err;
            err << "getcwd failed: " << std::strerror(errno);
            throw gnash::GnashException(err.str());
        }
        
        std::string currentDir(buf.get());
        currentDir.append("/");
        URL cwd(currentDir);
        init_relative(absolute_url, cwd);
    }
}

void
URL::init_relative(const std::string& relative_url, const URL& baseurl)
{
    // If relative url starts with an hash, it's just
    // an anchor change
    if ( relative_url[0] == '#' ){
        _proto = baseurl._proto;
        _host = baseurl._host;
        _port= baseurl._port;
        _path = baseurl._path;
        _anchor = relative_url.substr(1);
        return;
    }
    
    // If has a protocol, call absolute_url ctor
    if ( relative_url.find("://") != std::string::npos ) {
        init_absolute(relative_url);
        return;
    }
    
    // use protocol, port and host from baseurl
    _proto = baseurl._proto;
    _host = baseurl._host;
    _port = baseurl._port;

    if ( relative_url.size() && relative_url[0] == '/' ) {
        // get path from here
        _path = relative_url;
    } else { // path-relative
        std::string in = relative_url;
        
        // see how many dirs we want to take
        // off the baseurl path
        int dirsback=0;
        std::string::size_type pos;
        while ( ( pos = in.find("../") ) == 0 ) {
            ++dirsback;
            pos+=3;
            while (in[pos] == '/') {
                ++pos;
            }
            in = in.substr(pos);
        }
        
        // find dirsback'th slash from end of
        // baseurl path
        std::string basedir = baseurl._path.substr(0,
                                      baseurl._path.find_last_of("/")+1);
        
        // for WIN32
        if (basedir == "") {
            basedir = baseurl._path.substr(0,
                                      baseurl._path.find_last_of("\\")+1);
        }
        
#ifndef __amigaos4__
        assert(basedir[0] == '/'
               || basedir[1] == ':');  // for WIN32
#ifndef __OS2__
        // On OS/2 - a filepath such as x:file.swf is acceptable.......
        assert(*(basedir.rbegin()) == '/' 
               || *(basedir.rbegin()) == '\\');        // for WIN32
#endif
#endif
        std::string::size_type lpos =  basedir.size()-1;
        for (int i=0; i<dirsback; ++i) {
            if ( lpos == 0 ) break;
            std::string::size_type pos = basedir.rfind('/', lpos-1);
            // no more slashes found, break and set at 1
            if ( pos == std::string::npos ) lpos = 1;
            else lpos = pos;
        }
        basedir.resize(lpos+1);
        
        // get dirname from basurl path
        _path = basedir + in;
        
    }
    
    split_anchor_from_path();

    split_querystring_from_path();

    normalize_path(_path);


}

std::string
URL::str() const
{
    std::string ret = _proto + "://" + _host;

    if (!_port.empty()) {
        ret += ":" + _port;
    }
    
    ret += _path;

    if (!_querystring.empty()) {
        ret += "?" + _querystring;
    }

    if (!_anchor.empty()) {
        ret += "#" + _anchor;
    }
    return ret;
}
        
void
URL::split_anchor_from_path()
{
    assert(_anchor == "");

    // Extract anchor from path, if any
    std::string::size_type hashpos = _path.find('#');
    if ( hashpos != std::string::npos ) {
        _anchor = _path.substr(hashpos+1);
        _path.erase(hashpos);
    }
}

void
URL::split_port_from_host()
{
    assert(_port == "");

    // Extract anchor from path, if any

    // IPV4 addresses have square brackets around the adress like this:
    // http://[2a00:1450:4001:c01::88]/
    
    std::string::size_type ipv6 = _host.find(']');
    if (ipv6 == std::string::npos) {
        // IPV6 address
        std::string::size_type hashpos = _host.find(':');
        if ( hashpos != std::string::npos ) {
            _port = _host.substr(hashpos+1);
            _host.erase(hashpos);
        }
    } else {
        // IPV4 address
        std::string::size_type hashpos = _host.find(':', ipv6);
        if ( hashpos != std::string::npos ) {
            _port = _host.substr(hashpos+1);
            _host.erase(hashpos);
        }        
    }
}

void
URL::split_querystring_from_path()
{
    assert(_querystring == "");

    // extract the parameters from the URL

    std::string::size_type qmpos = _path.find("?");
    if (qmpos == std::string::npos) {
        // no query string
        return;
    }
    
    _querystring = _path.substr(qmpos + 1);

    // update _path
    _path.erase(qmpos);

}

void
URL::parse_querystring(const std::string& query_string,
                       std::map<std::string, std::string>& target_map)
{
    if ( query_string.empty() ) return; // nothing to do

    std::string qstring=query_string;;

    if ( qstring[0] == '?' ) {
        qstring=qstring.substr(1);
    }
    
    typedef boost::char_separator<char> Sep;
    typedef boost::tokenizer< Sep > Tok;
    Tok t1(qstring, Sep("&"));
    for(Tok::iterator tit=t1.begin(); tit!=t1.end(); ++tit) {
        const std::string& nameval = *tit;
        
        std::string name;
        std::string value;
        
        size_t eq = nameval.find("=");
        if ( eq == std::string::npos ) {
            name = nameval;
        } else {
            name = nameval.substr(0, eq);
            value = nameval.substr(eq+1);
        }
        
        decode(name);
        decode(value);
        
        target_map[name] = value;
    }
    
}

void
URL::encode(std::string& input)
{
    const std::string escapees = " \"#$%&+,/:;<=>?@[\\]^`{|}~_.!-(')";
    const std::string hexdigits = "0123456789ABCDEF";

    for (unsigned int i=0;i<input.length(); i++) {
        unsigned c = input[i] & 0xFF;   // ensure value is 0-255 not -ve
        
        if (c < 32 || c > 126 || escapees.find((char)c) != std::string::npos) {
            input[i] = '%';
            input.insert(++i, hexdigits.substr(c >> 4, 1));
            input.insert(++i, hexdigits.substr(c & 0xF, 1));
        } else if ( c == ' ' ) {
            input[i] = '+';
        }
    }
}

std::string
URL::encode(const std::string& str)
{
    std::string escapestring(str);
    encode(escapestring);
    return escapestring;
}

void
URL::decode(std::string& input)
{
    int hexcode;

    for (unsigned int i=0; i<input.length(); i++) {
        if (input[i] == '%' && (input.length() > i + 2) &&
            std::isxdigit(input[i+1]) && std::isxdigit(input[i+2])) {
            input[i+1] = std::toupper(input[i+1]);
            input[i+2] = std::toupper(input[i+2]);
            if (std::isdigit(input[i+1])) {
                hexcode = (input[i+1] - '0') * 16;
            } else {
                hexcode = (input[i+1] - 'A' + 10) * 16;
            }
            
            if (std::isdigit(input[i+2])) {
                hexcode += (input[i+2] - '0');
            } else {
                hexcode += (input[i+2] - 'A' + 10);
            }
            input[i] = hexcode;
            input.erase(i+1, 2);
        } else if ( input[i] == '+' ) {
            input[i] = ' ';
        }
    }
}

std::ostream&
operator<< (std::ostream& o, const URL& u)
{
    return o << u.str();
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
