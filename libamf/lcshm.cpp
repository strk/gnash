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

#include <unistd.h>
#include <cerrno>
#include <vector>
#include <string>
#include <cstring>
#include <boost/cstdint.hpp>

#include "log.h"
#include "amf.h"
#include "shm.h"
#include "element.h"
#include "lcshm.h"

using namespace std;
using namespace amf;

// Some facts:
//     * The header is 16 bytes,
//     * The message can be up to 40k,
//     * The listeners block starts at 40k+16 = 40976 bytes,
//     * To add a listener, simply append its name in the listeners list (null terminated strings)

namespace gnash {

// The maximum 
const int LC_HEADER_SIZE = 16;
const int MAX_LC_HEADER_SIZE = 40960;
const int LC_LISTENERS_START  = MAX_LC_HEADER_SIZE +  LC_HEADER_SIZE;

// This doesn't exist on all systems, but here's the vaue used on Unix.
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

// \class LocalConnection
/// \brief Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
///
LcShm::LcShm() 
    : _baseaddr(0)
{
//    GNASH_REPORT_FUNCTION;
}

LcShm::LcShm(boost::uint8_t *addr)
{
//    GNASH_REPORT_FUNCTION;
    _baseaddr = addr;
}

LcShm::LcShm(key_t key)
{
//    GNASH_REPORT_FUNCTION;
    _shmkey = key;
}

LcShm::~LcShm()
{
//    GNASH_REPORT_FUNCTION;
    
    vector<amf::Element *>::iterator it;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); it++) {
	amf::Element *el = (*(it));
         if (el) {
//             el->dump();
             delete el;
         }
    }
}

Listener::Listener()
    : _baseaddr(0)
{
//    GNASH_REPORT_FUNCTION;
}

Listener::Listener(boost::uint8_t *x)
{
//    GNASH_REPORT_FUNCTION;
    _baseaddr = x;
}

Listener::~Listener()
{
//    GNASH_REPORT_FUNCTION;
}

// see if a connection name exists in our list of listeners
bool
Listener::findListener(std::string &name)
{
//    GNASH_REPORT_FUNCTION;

    boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;
    char *item = reinterpret_cast<char *>(addr);
    // Walk through the list to the end
    while (*item != 0) {
        if (name == item) {
            return true;
        }
        item += strlen(item)+1;
    }
    
    return false;
}

bool
Listener::addListener(std::string &name)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;
    char *item = reinterpret_cast<char *>(addr);
    // Walk to the end of the list
    while ((item[0] != 0) && (item[1] != 0)) {
        item += strlen(item)+1;
    }

    if (findListener(name)) {
        return true;
    }
    
    // Add ourselves to the list
    if (memcpy(item, name.c_str(), name.size()) == 0) {
        return false;
    }

    // Add the two mystery two strings or number that follows the name.
    // These vary somewhat, but as test cases produces these values, we'll
    // use them till we're sure what these actually represent.
    item += name.size() + 1;
    const char *x1 = "::3";
    if (!memcpy(item, x1, 4)) {
        return false;
    }
    item += 4;
    const char *x2 = "::4";
    if (!memcpy(item, x2, 4)) {
        return false;
    }
    
    return true;
}

bool
Listener::removeListener(std::string &name)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;

    char *item = reinterpret_cast<char *>(addr);
    while (*item != 0) {
        if (name == item) {
            int len = strlen(item) + 1;
            while (*item != 0) {
                strcpy(item, item + len);
                item += len + 1;
            }
            return true;
        }
        item += strlen(item) + 1;
    }
    return false;
}

auto_ptr< vector<string> >
Listener::listListeners()
{
//    GNASH_REPORT_FUNCTION;    
    auto_ptr< vector<string> > listeners ( new vector<string> );
    if (_baseaddr != 0) {
        boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;
        
        const char *item = reinterpret_cast<const char *>(addr);
        while (*item != 0) {
            if (item[0] != ':') {
                listeners->push_back(item);
            }
            item += strlen(item) + 1;
        }
    }

    return listeners;
}

/// \brief Closes (disconnects) the LcShm object.
void
LcShm::close()
{
    GNASH_REPORT_FUNCTION;
    closeMem();
}

#if 0
boost::uint8_t *
LcShm::parseElement(amf::Element *el, boost::uint8_t *data)
{
    GNASH_REPORT_FUNCTION;
    boost::uint8_t *ptr = reinterpret_cast<uint8_t *>(data);
    Element::astype_e type = (Element::astype_e)*ptr;
    switch (type) {
      case AMF::NUMBER:
          double dub = 50.0;
          amf_obj.createElement(&el, "gain", dub);
          break;
      case AMF::STRING:
          amf_obj.createElement(&el, name, data);
          break;
      default:
          break;
    };
}
#endif

vector<amf::Element *> 
LcShm::parseBody(boost::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint8_t *ptr = reinterpret_cast<uint8_t *>(data);
    AMF amf;

    while (ptr) {
        amf::Element *el = new Element;
        ptr = amf.extractElement(el, ptr);
        if (el->getType() == Element::NUMBER) {
            if (el->to_number() == 0.0) {
                ptr = 0;
                continue;
            }
        }
        addObject(el);
    };
    
    return _amfobjs;
}

// From what I can tell by exaimining the memory segment, after the
// raw 16 bytes is a LocalConnection object. This appears to have the
// following data types:
// String - This appears to the connection name, and looks like
//          "localhost:lc_replay"
// String - This appears to be the hostname of the connection, and at
//          least in my tests, has always been "localhost".
// Boolean - In all the files I've looked at, this has always been
//           TRUE. I assume this is the domain security flag.
// Number - No idea what this number represents.
// Number - No idea what this number represents.
// NULL terminator
// AMF objects - this is followed by the AMF objects that have been
// added to the LocalConnection. This can be up to 40k long. While
// other web sites have claimed there is a length field in the initial
// shared memory segment header, I've never seen one in my tests.
boost::uint8_t *
LcShm::parseHeader(boost::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint8_t *ptr = data;
    
    memcpy(&_header, ptr, LC_HEADER_SIZE);
//     memcpy(&_object, data + LC_HEADER_SIZE, _header.length);
//    log_debug("Timestamp: %d", _header.timestamp);
//    log_debug("Length: %d", _header.length);
//     log_debug("Connection: %s", _object.connection_name);
//     log_debug("name: %s", _object.hostname);
    ptr += LC_HEADER_SIZE;

    
    Element *el = new amf::Element;
    AMF amf;
    ptr = amf.extractElement(el, ptr);
    _object.connection_name = el->to_string();
    delete el;
    
    el = new amf::Element;
    ptr = amf.extractElement(el, ptr);
    _object.hostname = el->to_string();
    delete el;
    
//     el = new amf::Element;
//     ptr = amf.extractElement(el, ptr);
//     _object.domain = el->to_bool();
//     delete el;
    
//     el = new amf::Element;
//     ptr = amf.extractElement(el, ptr);
//     _object.unknown_num1 = el->to_number();
//     delete el;
    
//     el = new amf::Element;
//     ptr = amf.extractElement(el, ptr);
//     _object.unknown_num2 = el->to_number();
//     delete el;
    
//    memcpy(&_object, data + LC_HEADER_SIZE, _header.length);
//     log_debug("Connection: %s", _object.connection_name.c_str());
//     log_debug("name: %s", _object.hostname.c_str());
//     log_debug("domain: %s", (_object.domain) ? "true" : "false");
//     log_debug("unknown_num1: %f", _object.unknown_num1);
//     log_debug("unknown_num2: %f", _object.unknown_num2);
    
//    ptr += 3;                   // skip past the NULL terminator
    return ptr;
}

boost::uint8_t *
LcShm::formatHeader(boost::uint8_t * /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    return NULL;
}

/// \brief Prepares the LcShm object to receive commands from a
/// LcShm.send() command.
/// 
/// The name is a symbolic name like "lc_name", that is used by the
/// send() command to signify which local connection to send the
/// object to.
bool
LcShm::connect(string &name)
{
//    GNASH_REPORT_FUNCTION;
    
    _name = name;
    
    if (Shm::attach(name.c_str(), true) == false) {
        return false;
    }

    if (Shm::getAddr() <= 0) {
        log_error("Failed to open shared memory segment: \"%s\"", name.c_str());
        return false; 
    }
    
    Listener::setBaseAddress(reinterpret_cast<uint8_t *>(Shm::getAddr()));
    boost::uint8_t *ptr = parseHeader(Listener::getBaseAddress());
    vector<amf::Element *> ellist = parseBody(ptr);
    
    return true;
}

bool
LcShm::connect(key_t key)
{
//    GNASH_REPORT_FUNCTION;
    
    if (Shm::attach(key, true) == false) {
        return false;
    }

    if (Shm::getAddr() <= 0) {
        log_error("Failed to open shared memory segment: 0x%x", key);
        return false; 
    }
    
    Listener::setBaseAddress(reinterpret_cast<uint8_t *>(Shm::getAddr()));
    boost::uint8_t *ptr = parseHeader(Listener::getBaseAddress());
    vector<amf::Element *> ellist = parseBody(ptr);
    
    return true;
}

/// \brief Invokes a method on a specified LcShm object.
void
LcShm::send(const std::string & /*name*/, const std::string & /*dataname*/, amf::Element * /*data*/)
{
    
    log_unimpl (__FUNCTION__);
}

void
LcShm::dump()
{
//    GNASH_REPORT_FUNCTION;

//     cerr <<"Timestamp: " << _header.timestamp << endl;
//     cerr << "Length: " << _header.length << endl;

    cerr << "Connection Name:\t" << _object.connection_name << endl;
    cerr << "Hostname Name:\t\t" << _object.hostname << endl;
    cerr << "Domain Allowed:\t\t" << ((_object.domain) ? "true" : "false") << endl;
    vector<amf::Element *>::iterator ait;
//    cerr << "# of Elements in file: " << _amfobjs.size() << endl;
    for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ait++) {
	amf::Element *el = (*(ait));
        el->dump();
    }

    vector<string>::const_iterator lit;
    auto_ptr< vector<string> > listeners ( listListeners() );
    for (lit=listeners->begin(); lit!=listeners->end(); lit++) {
        string str = *lit;
        if (str[0] != ':') {
            cerr << "Listeners:\t" << str << endl;
// 		total++;
        }
    }
}

} // end of gnash namespace
