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
#include <boost/shared_ptr.hpp>

#include "log.h"
#include "buffer.h"
#include "network.h"
#include "amf.h"
#include "shm.h"
#include "element.h"
#include "GnashException.h"
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
// although a bool is one byte, it appears to be a short in AMF,
// plus the type byte.
const int AMF_BOOLEAN_SIZE = 3;
const int LC_HEADER_SIZE = 16;
const int MAX_LC_HEADER_SIZE = 40960;
const int LC_LISTENERS_START  = MAX_LC_HEADER_SIZE +  LC_HEADER_SIZE;

// This doesn't exist on all systems, but here's the vaue used on Unix.
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

#define ENSUREBYTES(from, toofar, size) { \
	if ( from+size >= toofar ) \
		throw ParserException("Premature end of AMF stream"); \
}

// \class LocalConnection
/// \brief Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
///
LcShm::LcShm() 
    : _baseaddr(0)
{
//    GNASH_REPORT_FUNCTION;
}

LcShm::LcShm(Network::byte_t *addr)
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
}

Listener::Listener()
    : _baseaddr(0)
{
//    GNASH_REPORT_FUNCTION;
}

Listener::Listener(Network::byte_t *x)
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
Listener::findListener(const string &name)
{
//    GNASH_REPORT_FUNCTION;

    Network::byte_t *addr = _baseaddr + LC_LISTENERS_START;
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
Listener::addListener(const string &name)
{
    GNASH_REPORT_FUNCTION;

    Network::byte_t *addr = _baseaddr + LC_LISTENERS_START;
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
    const char *x2 = "::2";
    if (!memcpy(item, x2, 4)) {
        return false;
    }
    
    return true;
}

// I don't believe this function is support by other swf players,
// but we do, as it's nice to remove oneself from the listeners
// list so nobody knows we were here listening.
bool
Listener::removeListener(const string &name)
{
    GNASH_REPORT_FUNCTION;

    Network::byte_t *addr = _baseaddr + LC_LISTENERS_START;

    int len = 0;
    char *item = reinterpret_cast<char *>(addr);
    while (*item != 0) {
        if (name == item) {
            while (*item != 0) {
                len = strlen(item) + 8 + 1;
                strcpy(item, item + len);
                item += len + strlen(item + len);
            }
            
            memset(item - len, 0, len);
            return true;
        }
        item += strlen(item) + 1;
    }
    
    return false;
}

// Get a list of alll the listeners waiting on this channel
auto_ptr< vector<string> >
Listener::listListeners()
{
//    GNASH_REPORT_FUNCTION;    
    auto_ptr< vector<string> > listeners ( new vector<string> );
    if (_baseaddr != 0) {
        Network::byte_t *addr = _baseaddr + LC_LISTENERS_START;
        
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
Network::byte_t *
LcShm::parseElement(boost::shared_ptr<Element> el, Network::byte_t *data)
{
    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<uint8_t *>(data);
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

vector<boost::shared_ptr<Element> > 
LcShm::parseBody(Network::byte_t *data)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = reinterpret_cast<uint8_t *>(data);
    AMF amf;

    while (ptr) {
        boost::shared_ptr<Element> el = amf.extractAMF(ptr);
        if (el) {
            if (el->getType() == Element::NUMBER) {
                if (el->to_number() == 0.0) {
                    break;
                }
            }
            if (el->getType() != Element::NOTYPE) {
                _amfobjs.push_back(el);
                ptr += el->getLength();
            } else {
                break;
            }
        } else {

        }
    };
    
    return _amfobjs;
}
#endif

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
Network::byte_t *
LcShm::parseHeader(Network::byte_t *data, Network::byte_t* tooFar)
{
//    GNASH_REPORT_FUNCTION;
    Network::byte_t *ptr = data;

    if (data == 0) {
        log_debug("No data pointer to parse!");
        return 0;
    }

#ifndef GNASH_TRUST_AMF
    ENSUREBYTES(ptr, tooFar, LC_HEADER_SIZE);
#endif
    
    memcpy(&_header, ptr, LC_HEADER_SIZE);
//     memcpy(&_object, data + LC_HEADER_SIZE, _header.length);
//    log_debug("Timestamp: %d", _header.timestamp);
//    log_debug("Length: %d", _header.length);
//     log_debug("Connection: %s", _object.connection_name);
//     log_debug("name: %s", _object.hostname);
    ptr += LC_HEADER_SIZE;

    
    AMF amf;
    boost::shared_ptr<Element> el = amf.extractAMF(ptr, tooFar);
    if (el == 0) {
        log_debug("Didn't extract an element from the byte stream!");
        return 0;
    }
    
    _object.connection_name = el->to_string();
    
    el = amf.extractAMF(ptr, tooFar);
    if (ptr != 0) {
        _object.hostname = el->to_string();
    }
    
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

Network::byte_t *
LcShm::formatHeader(const std::string &con, const std::string &host, bool /* domain */ )
{
    GNASH_REPORT_FUNCTION;
//    Network::byte_t *ptr = data + LC_HEADER_SIZE;
    int size = con.size() + host.size() + 9;

//    Buffer *buf;
    
    Network::byte_t *header = new Network::byte_t[size + 1];
    Network::byte_t *ptr = header;

    // This is the initial 16 bytes of the header
    memset(ptr, 0, size + 1);
    *ptr = 1;
    ptr += 3;
    *ptr = 1;
    ptr = header + LC_HEADER_SIZE;

    // Which is then always followed by 3 AMF objects.
    boost::shared_ptr<amf::Buffer> buf1 = AMF::encodeString(con);
    memcpy(ptr, buf1->begin(), buf1->size());
    ptr += buf1->size();

    boost::shared_ptr<amf::Buffer> buf2 = AMF::encodeString(host);
    memcpy(ptr, buf2->begin(), buf2->size());
    ptr += buf2->size();
    
    return ptr;
}

/// \brief Prepares the LcShm object to receive commands from a
/// LcShm.send() command.
/// 
/// The name is a symbolic name like "lc_name", that is used by the
/// send() command to signify which local connection to send the
/// object to.
bool
LcShm::connect(const string &name)
{
    GNASH_REPORT_FUNCTION;
    
    _name = name;

    // the name here is optional, Gnash will pick a good default.
    // When using sysv shared memory segments in compatibility mode,
    // the name is ignored, and the SHMkey is specified in the user's
    // ~/.gnashrc file.
    if (Shm::attach(name.c_str(), true) == false) {
        return false;
    }

    if (Shm::getAddr() <= 0) {
        log_error("Failed to open shared memory segment: \"%s\"", name.c_str());
        return false; 
    }
    
    uint8_t* baseAddress = reinterpret_cast<uint8_t *>(Shm::getAddr());
    uint8_t* tooFar = baseAddress+Shm::getSize();
    Listener::setBaseAddress(baseAddress);
    _baseaddr = baseAddress;
    parseHeader(baseAddress, tooFar);
//    vector<boost::shared_ptr<Element> > ellist = parseBody(ptr);
//     log_debug("Base address is: 0x%x, 0x%x",
//               (unsigned int)Listener::getBaseAddress(), (unsigned int)_baseaddr);

    addListener(name);

    return true;
}

bool
LcShm::connect(key_t key)
{
    GNASH_REPORT_FUNCTION;
    
    if (Shm::attach(key, true) == false) {
        return false;
    }

    if (Shm::getAddr() <= 0) {
        log_error("Failed to open shared memory segment: 0x%x", key);
        return false; 
    }
    
    uint8_t* baseAddress = reinterpret_cast<uint8_t *>(Shm::getAddr());
    uint8_t* tooFar = baseAddress+Shm::getSize();
    Listener::setBaseAddress(baseAddress);
    _baseaddr = baseAddress;
    parseHeader(baseAddress, tooFar);
//    vector<boost::shared_ptr<Element> > ellist = parseBody(ptr);
//     log_debug("Base address is: 0x%x, 0x%x",
//               (unsigned int)Listener::getBaseAddress(), (unsigned int)_baseaddr);
    
    return true;
}

/// \brief Invokes a method on a specified LcShm object.
void
LcShm::send(const string & /* name */, const string & /* domainname */,
            vector<boost::shared_ptr<Element> > &/* data */)
{
    GNASH_REPORT_FUNCTION;

//     log_debug("Base address is: 0x%x, 0x%x",
//               (unsigned int)Listener::getBaseAddress(), (unsigned int)_baseaddr);

//    formatHeader(name, domainname, _object.domain);

    // Update the connection name
    Network::byte_t *ptr = Listener::getBaseAddress();
    if (ptr == reinterpret_cast<Network::byte_t *>(0)) {
        log_error("base address not set!");
    }

#if 0
//    Network::byte_t *tmp = AMF::encodeElement(name.c_str());
//     memcpy(ptr, tmp, name.size());
//     ptr +=  name.size() + AMF_HEADER_SIZE;
//     delete[] tmp;

//     tmp = AMF::encodeElement(domainname.c_str());
//     memcpy(ptr, tmp, domainname.size());
//     ptr +=  domainname.size() + AMF_HEADER_SIZE;

//    ptr += LC_HEADER_SIZE;
//    Network::byte_t *x = ptr;    // just for debugging from gdb. temporary

    // This is the initial 16 bytes of the header
    memset(ptr, 0, LC_HEADER_SIZE + 200);
    *buf->at(0) = 1;
//    *ptr = 1;
    ptr += 4;
    *buf->at(4) = 1;
//    *ptr = 1;
    ptr += LC_HEADER_SIZE - 4;
    // Which is then always followed by 3 AMF objects.
    
    Buffer *tmp = AMF::encodeElement(name.c_str());
    memcpy(ptr, tmp, name.size() + AMF_HEADER_SIZE);
    delete[] tmp;

    ptr += name.size() + AMF_HEADER_SIZE;

    // Update the host on the other end of the connection.
    tmp = AMF::encodeElement(domainname.c_str());
    memcpy(ptr, tmp, domainname.size() + AMF_HEADER_SIZE );
    delete[] tmp;

    ptr += domainname.size() + AMF_HEADER_SIZE;

//     // Set the domain flag to whatever it's current value is.
// //    Element domain(_object.domain);
//     tmp = AMF::encodeBoolean(_object.domain);
//     memcpy(ptr, tmp, AMF_BOOLEAN_SIZE);
// //    delete[] tmp;
    
//     ptr += AMF_BOOLEAN_SIZE;
    
    vector<Network::byte_t> *vec = AMF::encodeElement(data);
    vector<Network::byte_t>::iterator vit;
    // Can't do a memcpy with a std::vector
//    log_debug("Number of bytes in the vector: %x", vec->size());
    for (vit = vec->begin(); vit != vec->end(); vit++) {
	*ptr = *vit;
#if 0                           // debugging crapola
        if (isalpha(*ptr))
            printf("%c ", *ptr);
        else
            printf("0x%x ", *ptr);
#endif
        ptr++;
    }
    
//    delete[] tmp;
#endif
    
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
    vector<boost::shared_ptr<Element> >::iterator ait;
    cerr << "# of Elements in file: " << _amfobjs.size() << endl;
    for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ait++) {
	boost::shared_ptr<Element> el = (*(ait));
        el->dump();
    }

    vector<string>::const_iterator lit;
    auto_ptr< vector<string> > listeners ( listListeners() );
    cerr << "# of Listeners in file: " << listeners->size() << endl;
    for (lit=listeners->begin(); lit!=listeners->end(); lit++) {
        string str = *lit;
        if (str[0] != ':') {
            cerr << "Listeners:\t" << str << endl;
        }
    }
}

} // end of gnash namespace
