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

#include <cerrno>
#include <vector>
#include <string>
#include <cstring>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#include "log.h"
#include "buffer.h"
//#include "network.h"
#include "amf.h"
#include "shm.h"
#include "element.h"
#include "GnashException.h"
#include "lcshm.h"

#ifndef VECTOR
#include <vector>
#endif

using namespace std;
using namespace gnash;

// Some facts:
//     * The header is 16 bytes,
//     * The message can be up to 40k,
//     * The listeners block starts at 40k+16 = 40976 bytes,
//     * To add a listener, simply append its name in the listeners list (null terminated strings)

/// \namespace amf
///
/// This namespace is for all the AMF specific classes in libamf.
namespace amf {

// The maximum 
// although a bool is one byte, it appears to be a short in AMF,
// plus the type byte.
const int AMF_BOOLEAN_SIZE = 3;

/// \var LC_HEADER_SIZE
///     The header size for a memory segment.
const int LC_HEADER_SIZE = 16;

/// \var MAX_LC_HEADER_SIZE
///     The maximum size allowed for the header of a memory segment.
const int MAX_LC_HEADER_SIZE = 40960;

/// \var LC_LISTENERS_START
///     The starting address for the block of Listeners in the memory
///     segment.
const int LC_LISTENERS_START  = MAX_LC_HEADER_SIZE +  LC_HEADER_SIZE;
//LC_LISTENERS_START equals to 40976.

/// \def MAXHOSTNAMELEN
///     This doesn't exist on all systems, but here's the value used on Unix.
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

/// \define ENSUREBYTES
///
/// @param from The base address to check.
///
/// @param tooFar The ending address that is one byte too many.
///
/// @param size The number of bytes to check for: from to tooFar.
///
/// @remarks May throw an Exception
#define ENSUREBYTES(from, toofar, size) { \
	if ( from+size >= toofar ) \
		throw ParserException("Premature end of AMF stream"); \
}

/// \brief Construct an uninitialized shared memory segment.
///     Open a connection between two SWF movies so they can send
///     each other Flash Objects, but does not initialize the memory
///     segment.
LcShm::LcShm() 
    : _baseaddr(0)
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief Construct an initialized shared memory segment.
///
/// @param addr The address to use for the memory segment.
LcShm::LcShm(boost::uint8_t *addr)
{
//    GNASH_REPORT_FUNCTION;
    _baseaddr = addr;
}

/// \brief Construct an initialized shared memory segment.
///
/// @param key The SYSV style key to use for the memory segment.
LcShm::LcShm(key_t key)
{
//    GNASH_REPORT_FUNCTION;
    _shmkey = key;
}

/// \brief Delete the shared memory segment.
///
/// @remark This does not clear the content of the memory segment.
LcShm::~LcShm()
{
//    GNASH_REPORT_FUNCTION;    
}

/// \brief Construct a block of Listeners.
///     This constructs an uninitialized Listener block.
Listener::Listener()
    : _baseaddr(0)
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief Construct a block Listeners at the specified address.
///
/// @param baseaddr The address to use for the block of
///     Listeners.
Listener::Listener(boost::uint8_t *x)
{
//    GNASH_REPORT_FUNCTION;
    _baseaddr = x;
}

/// \brief Delete the Listener block
Listener::~Listener()
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief See if a connection name exists in our list of Listeners
///
/// @param name An ASCII string that is the name of the Listener
///		to search for.
///
/// @return true if this succeeded. false if it doesn't.
bool
Listener::findListener(const string &name)
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

/// \brief Create a new Listener in the memory segment.
///
/// @param name The name for the Listener.
///
/// @return true if this succeeded. false if it doesn't.
bool
Listener::addListener(const string &name)
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
    const char *x2 = "::2";
    if (!memcpy(item, x2, 4)) {
        return false;
    }
    
    return true;
}

/// \brief Remove the Listener for this Object.
///
/// @param name An ASCII string that is the name of the Listener
///		to remove from the  memory segment..
///
/// @return true if this succeeded. false if it doesn't.
///
/// @remark
///     I don't believe this function is support by other swf players,
///     but we do, as it's nice to remove oneself from the listeners
///     list so nobody knows we were here listening.
bool
Listener::removeListener(const string &name)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;

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

/// \brief List the Listeners for this memory segment.
///
/// @return A smart pointer to a vector of Listener names.
///
/// @remarks This is only used for debugging
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

/// \brief Close a memory segment.
///		This closes the shared memory segment, but the data
///		remains until the next reboot of the computer.
///
/// @return nothing.    
void
LcShm::close()
{
    GNASH_REPORT_FUNCTION;
    closeMem();
}

/// @note
///     From what I can tell by exaimining the memory segment, after
///     the raw 16 bytes is a LocalConnection object. This appears to
///     have the following data types:
/// String - This appears to the connection name, and looks like
///          "localhost:lc_replay"
/// String - This appears to be the hostname of the connection, and at
///          least in my tests, has always been "localhost".
/// Boolean - In all the files I've looked at, this has always been
///           TRUE. I assume this is the domain security flag.
/// Number - No idea what this number represents.
/// Number - No idea what this number represents.
/// NULL terminator
///     AMF objects - this is followed by the AMF objects that have
///     been added to the LocalConnection. This can be up to 40k
///     long. While other web sites have claimed there is a length
///     field in the initial shared memory segment header, I've never
///     seen one in my tests.

/// \brief Parse the header of the memory segment.
///
/// @param data real pointer to start parsing from.
///
/// @param tooFar A pointer to one-byte-past the last valid memory
///		address within the buffer.
///
/// @return A real pointer to the data after the headers has been parsed.
///
/// @remarks May throw a ParserException
boost::uint8_t *
LcShm::parseHeader(boost::uint8_t *data, boost::uint8_t* tooFar)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint8_t *ptr = data;

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

/// \brief Format the header for the memory segment.
///
/// @param con The name of the connection.
///
/// @param host The hostname of the connection, often "localhost"
///
/// @param domain The domain the hostname is in.
///
/// @return A real pointer to a header for a memory segment.
boost::uint8_t *
LcShm::formatHeader(const std::string &con, const std::string &host, bool /* domain */ )
{
    GNASH_REPORT_FUNCTION;
//  boost::uint8_t *ptr = data + LC_HEADER_SIZE;
    int size = con.size() + host.size() + 9;

//    Buffer *buf;
    
//    Si: Do not understand why use new here. 
//    Assign the value of header and ptr directly.
//    boost::uint8_t *header = new boost::uint8_t[size + 1];
//    boost::uint8_t *ptr = header;

    boost::uint8_t *header = Listener::getBaseAddress();
    boost::uint8_t *ptr_FH    = Listener::getBaseAddress();

    // This is the initial 16 bytes of the header
    memset(ptr_FH, 0, 16 + size + 1);
    *ptr_FH = 1;
    ptr_FH += 4;  
    //Si changes this value from 3 to 4.
    *ptr_FH = 1;
    ptr_FH = header + LC_HEADER_SIZE;

//  Si has rewritten the following code.
//  The protocol is set to be localhost now. 
//  Make sure it is right later.

    // Which is then always followed by 3 AMF objects.
    boost::shared_ptr<amf::Buffer> buf1 = AMF::encodeString(con);
    memcpy(ptr_FH, buf1->begin(), buf1->size());
    ptr_FH += buf1->size();

    const std::string protocol="localhost";
    boost::shared_ptr<amf::Buffer> buf2 = AMF::encodeString(protocol);
    memcpy(ptr_FH, buf2->begin(), buf2->size());
    ptr_FH += buf2->size();

    boost::shared_ptr<amf::Buffer> buf3 = AMF::encodeString(host);
    memcpy(ptr_FH, buf3->begin(), buf3->size());
    ptr_FH += buf3->size();
    
    return ptr_FH;
}

/// \brief Connect to a memory segment.
///     Prepares the LcShm object to receive commands from a
///     LcShm.send() command.
///
/// @param name The name to use for POSIX shared memory, which is not
///		the default type used.
///
/// @return true if this succeeded. false if it doesn't.
///
/// @remarks The name is a symbolic name like "lc_name", that is used
///     by the send() command to signify which local connection to
///     send the object to.
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
    
	boost::uint8_t* baseAddress = reinterpret_cast<boost::uint8_t *>(Shm::getAddr());
	boost::uint8_t* tooFar = baseAddress+Shm::getSize();
    Listener::setBaseAddress(baseAddress);
    _baseaddr = baseAddress;
    parseHeader(baseAddress, tooFar);
//    vector<boost::shared_ptr<Element> > ellist = parseBody(ptr);
//     log_debug("Base address is: 0x%x, 0x%x",
//               (unsigned int)Listener::getBaseAddress(), (unsigned int)_baseaddr);

    addListener(name);

    return true;
}

/// \brief Connect to a memory segment.
///
/// @param key The SYSV style key for the shared memory segment,
///	which is the default type used.
///
/// @return true if this succeeded. false if it doesn't.
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
    
	boost::uint8_t* baseAddress = reinterpret_cast<boost::uint8_t *>(Shm::getAddr());
	boost::uint8_t* tooFar = baseAddress+Shm::getSize();
    Listener::setBaseAddress(baseAddress);
    _baseaddr = baseAddress;
    parseHeader(baseAddress, tooFar);
//    vector<boost::shared_ptr<Element> > ellist = parseBody(ptr);
//     log_debug("Base address is: 0x%x, 0x%x",
//               (unsigned int)Listener::getBaseAddress(), (unsigned int)_baseaddr);
    
    return true;
}













/// \brief Put data in the memory segment
///		This puts data into the memory segment
///
/// @param name The connection name for this connection
///
/// @param dataname The name of the data to send.
///
/// @param data A vector of smart pointers to the AMF0 Elements
///		contaiing the data for this memory segment.
///
/// @return nothing.

// Si have rewrittten these!

void
LcShm::send(const string&  name , const string&  domainname ,
            vector<amf::Element* >& data )
{
    //GNASH_REPORT_FUNCTION;

     log_debug(_(" ***** The send function is called *****") ); 

//     cout<<" The send function is called ! "<<endl;
     log_debug("Base address is: 0x%x, 0x%x",
               (unsigned int)Listener::getBaseAddress(), (unsigned int)_baseaddr);

//The base address
     boost::uint8_t *baseptr = Listener::getBaseAddress();
   	   
     boost::uint8_t *ptr = baseptr;     

// Check if the base address exists
    if (baseptr == reinterpret_cast<boost::uint8_t *>(0)) {
        log_error("***** base address not set! *****");
	}

// This function write the first 16 bytes and the following three messages into the memory.
// ptr should be moved
    ptr=formatHeader(name, domainname, _object.domain);
// The ptr is now pointing to the start of the message

//Put the date into memory when it is not empty

  	log_debug(_(" ***** The size of the data is %s *****"),data.size() ); 
      if (data.size()==0){	
    	   std::vector<amf::Element* >::iterator iter;
	   for(iter = data.begin(); iter != data.end(); iter++)
		{
		// temporary buf for element
		boost::shared_ptr<Buffer> buf = AMF::encodeElement(*iter);		

		memcpy(ptr, buf->begin(), buf->size() );
		ptr+= buf->size();		
		}
	}	
	
// Update the connection name
	   
#if 0
//     boost::uint8_t *tmp = AMF::encodeElement(name.c_str());
//     memcpy(ptr, tmp, name.size());
//     ptr +=  name.size() + AMF_HEADER_SIZE;
//     delete[] tmp;

//     tmp = AMF::encodeElement(domainname.c_str());
//     memcpy(ptr, tmp, domainname.size());
//     ptr +=  domainname.size() + AMF_HEADER_SIZE;

//    ptr += LC_HEADER_SIZE;
//    boost::uint8_t *x = ptr;    // just for debugging from gdb. temporary

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

// //  Set the domain flag to whatever it's current value is.
// //  Element domain(_object.domain);
//     tmp = AMF::encodeBoolean(_object.domain);
//     memcpy(ptr, tmp, AMF_BOOLEAN_SIZE);
// //  delete[] tmp;
    
//     ptr += AMF_BOOLEAN_SIZE;
    
    vector<boost::uint8_t> *vec = AMF::encodeElement(data);
    vector<boost::uint8_t>::iterator vit;
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

///  \brief Dump the internal data of this class in a human readable form.
/// @remarks This should only be used for debugging purposes.
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

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
