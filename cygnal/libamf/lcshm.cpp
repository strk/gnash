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
#include "SharedMem.h"
#include "element.h"
#include "GnashException.h"
#include "lcshm.h"

#ifndef VECTOR
#include <vector>
#endif

using std::string;
using std::vector;
using gnash::log_debug;
using gnash::log_error;

// Some facts:
//     * The header is 16 bytes,
//     * The message can be up to 40k,
//     * The listeners block starts at 40k+16 = 40976 bytes,
//     * To add a listener, simply append its name in the listeners list (null terminated strings)

/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

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
		throw gnash::ParserException("Premature end of AMF stream"); \
}

/// \brief Construct an uninitialized shared memory segment.
///     Open a connection between two SWF movies so they can send
///     each other Flash Objects, but does not initialize the memory
///     segment.
LcShm::LcShm() 
    :
    SharedMem(64528),
    _baseaddr(0)
{
//    GNASH_REPORT_FUNCTION;
}

/// \brief Construct an initialized shared memory segment.
///
/// @param addr The address to use for the memory segment.
LcShm::LcShm(boost::uint8_t *addr)
    :
    SharedMem(64528)
{
//    GNASH_REPORT_FUNCTION;
    _baseaddr = addr;
}

/// \brief Construct an initialized shared memory segment.
///
/// @param key The SYSV style key to use for the memory segment.
LcShm::LcShm(key_t /*key*/)
    :
    SharedMem(64528)
{
//    GNASH_REPORT_FUNCTION;
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
        if (name == item) 	{
            return true;
        }
        item += strlen(item)+8+1;
		
		//Si has rewritten thise, please test before using.
		//This version should be right now.
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

	 if (findListener(name)) {
        return true;
    }

    // Walk to the end of the list
    while ((item[0] != 0) && (item[1] != 0)) {
        item += strlen(item)+1;
		// This is not the proper way.  But it works now.	
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
	// Si: I have tested more swf files from the internet
	// Not all of them are 3 and 2. 
	// Could be 3 2, 4 1, 3 1, etc.
	
	//Si has rewritten thise, please test before using.
	//This version should be right now.
    
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
	int dest= 0;	
	int source =0;
    char *item = reinterpret_cast<char *>(addr);
    while (*item != 0) {
        if (name == item) {         
			len =strlen(item) +8+1; //The length of the removed string, including '\n' 				
			
			while (*item != 0) {
                
				// len = strlen(item) + 8 + 1;
                // strcpy(item, item + len);
                // item += len + strlen(item + len);
				// These old codes are wrong !!!!  
				// Rewrite them!!!
				
				if (source!=0)
					dest  += strlen(item+source)+8+1;
									
				source+= strlen(item+dest)+8+1;
				strcpy(item+dest,item+source);		
            }
                                  	        				  
            memset(item+dest+strlen(item+source)+8+1, 0, len);
            return true;
        }
        //item += strlen(item) + 1;
		//This is not right.
		
		item += strlen(item) + 8 + 1;
		//You will only know weather this funtion is right or not after you really remove some listeneres.
		//Si has rewritten thise, please test before using.
		//This version should be right now.
		
	}
    
    return false;
}

/// \brief List the Listeners for this memory segment.
///
/// @return A smart pointer to a vector of Listener names.
///
/// @remarks This is only used for debugging
std::auto_ptr< vector<string> >
Listener::listListeners()
{
//    GNASH_REPORT_FUNCTION;    
    std::auto_ptr< vector<string> > listeners ( new vector<string> );
    if (_baseaddr != 0) {
        boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;
        
        const char *item = reinterpret_cast<const char *>(addr);
        while (*item != 0) {
            if (item[0] != ':') {
                listeners->push_back(item);
            }
            item += strlen(item) + 1;
			// This is not very effective but looks right.
			// A better way should be jump to the next listener directly.
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
/// Si: This value could be false.
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
        log_debug(_("No data pointer to parse!"));
        return 0;
    }

#ifndef GNASH_TRUST_AMF
    ENSUREBYTES(ptr, tooFar, LC_HEADER_SIZE);
#endif
    
    memcpy(&_header, ptr, LC_HEADER_SIZE);
//    memcpy(&_object, data + LC_HEADER_SIZE, _header.length);
//    log_debug("Timestamp: %d", _header.timestamp);
//    log_debug("Length: %d", _header.length);
//    log_debug("Connection: %s", _object.connection_name);
//    log_debug("name: %s", _object.hostname);
    ptr += LC_HEADER_SIZE;
    
    AMF amf;
    boost::shared_ptr<Element> el = amf.extractAMF(ptr, tooFar);
    if (el == 0) {
        log_debug(_("Didn't extract an element from the byte stream!"));
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
//  GNASH_REPORT_FUNCTION;
//  boost::uint8_t *ptr = data + LC_HEADER_SIZE;
    int size = con.size() + host.size() + 9;

//    Buffer *buf;
    
//    Si:
//    Assign the value of header and ptr directly.
//    boost::uint8_t *header = new boost::uint8_t[size + 1];
//    boost::uint8_t *ptr = header;

    boost::uint8_t *header    = Listener::getBaseAddress();
    boost::uint8_t *ptr_FH    = Listener::getBaseAddress();
//	log_debug("Base address in 'formatHeader' is: 0x%x, 0x%x",(unsigned int) header, (unsigned int) ptr_FH);

    // This is the initial 16 bytes of the header
    memset(ptr_FH, 0, 16 + size + 1);
    *ptr_FH = 1;
    ptr_FH += 4;  
    //Si changes this value from 3 to 4.
    *ptr_FH = 1;
    ptr_FH = header + LC_HEADER_SIZE;
    //Do you want to assign the value of timestamp and message size?

//  Si has rewritten the following code.
//  The protocol is set to be localhost now. 
//  Make sure it is always right. Probably wrong.

    // Which is then always followed by 3 AMF objects.
    boost::shared_ptr<cygnal::Buffer> buf1 = AMF::encodeString(con);
    memcpy(ptr_FH, buf1->begin(), buf1->size());
    ptr_FH += buf1->size();

    const std::string protocol="localhost";
	// This could equal to the domain name.
    boost::shared_ptr<cygnal::Buffer> buf2 = AMF::encodeString(protocol);
    memcpy(ptr_FH, buf2->begin(), buf2->size());
    ptr_FH += buf2->size();

    boost::shared_ptr<cygnal::Buffer> buf3 = AMF::encodeString(host);
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
LcShm::connect(const string& names)
{
    //GNASH_REPORT_FUNCTION;

    //log_debug(" The connect function is called");	
    log_debug(_(" The size of %s is %d "), names, names.size()); 
	
    if (names == "") {
	return false;
    }
    
    _name = names;

    // the name here is optional, Gnash will pick a good default.
    // When using sysv shared memory segments in compatibility mode,
    // the name is ignored, and the SHMkey is specified in the user's
    // ~/.gnashrc file.
    if (SharedMem::attach() == false) {
        return false;
    }

    if (SharedMem::begin() <= static_cast<unsigned char *>(0)) {
        log_error(_("Failed to open shared memory segment: \"%s\""), names.c_str());
        return false; 
    }
    
	boost::uint8_t* baseAddress = reinterpret_cast<boost::uint8_t *>(SharedMem::begin());
	
	boost::uint8_t* tooFar = SharedMem::end();
    Listener::setBaseAddress(baseAddress);
    _baseaddr = baseAddress;
    parseHeader(baseAddress, tooFar);
//	log_debug("Base address in 'connect' is: 0x%x, 0x%x",(unsigned int) SharedMem::begin(), (unsigned int) _baseaddr);
//  vector<boost::shared_ptr<Element> > ellist = parseBody(ptr);
//  log_debug("Base address is: 0x%x, 0x%x",
//               (unsigned int)Listener::getBaseAddress(), (unsigned int)_baseaddr);

    addListener(names);
	
//	Make sure this one is already connected
	setconnected(true);
//	system("ipcs");

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
	boost::mutex::scoped_lock lock(_localconnection_mutex);
   // GNASH_REPORT_FUNCTION;
    
    if (SharedMem::attach() == false) {
        return false;
    }

    if (SharedMem::begin() <= static_cast<unsigned char *>(0)) {
        log_error(_("Failed to open shared memory segment: 0x%x"), key);
        return false; 
    }
    
	boost::uint8_t* baseAddress = reinterpret_cast<boost::uint8_t *>(SharedMem::begin());
	boost::uint8_t* tooFar = SharedMem::end();
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

// Si have rewrittten all of these!
// We test several test cases, and it looks like the memory can be written in the right way.
// Please make further check after 'receiving' is completed.

void
LcShm::send(const string&  name , const string&  domainname ,
            vector<cygnal::Element* >& data )
{
    //GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_localconnection_mutex);
	
	std::vector<cygnal::Element* >::iterator iter;
		   
	if (!Listener::getBaseAddress()) return;

    //The base address
     boost::uint8_t *baseptr = Listener::getBaseAddress();  	   
     boost::uint8_t *ptr = baseptr;     

// Compute the time
// Please check before use.
// Put this value into the header if necessary.
//	int timestamp=GetTickCount();
	
// Compute the size of the message.
// Please check before use.
// Put this value into the header if necessary.
      int message_size=0;
      if (data.size()!=0){	
		   for(iter = data.begin(); iter != data.end(); ++iter){
			    boost::shared_ptr<Buffer> buf = AMF::encodeElement(*iter);									
				message_size+=buf->size();
			}
	}	
	 

// This function write the first 16 bytes and the following three messages into the memory.
// ptr should be moved
// ptr=formatHeader(name, domainname, _object.domain);
// The ptr is now pointing to the start of the message
	
	int size = name.size() + domainname.size() + 9;
    // This is the initial 16 bytes of the header
    memset(ptr, 0, 16 + size + 1);
    *ptr = 1;
    ptr += 4;
    //Si changes this value from 3 to 4.
    *ptr = 1;
    ptr = baseptr + LC_HEADER_SIZE;

//  Si has rewritten the following code.
//  duplicate as in formatheader
//  The protocol is set to be localhost now. 
//  Make sure it is right later.

    // Which is then always followed by 3 AMF objects.
    boost::shared_ptr<cygnal::Buffer> buf1 = AMF::encodeString(name);
    memcpy(ptr, buf1->begin(), buf1->size());
    ptr += buf1->size();
	
    const std::string protocol="localhostf";
    boost::shared_ptr<cygnal::Buffer> buf2 = AMF::encodeString(protocol);
    memcpy(ptr, buf2->begin(), buf2->size());
    ptr += buf2->size();

    boost::shared_ptr<cygnal::Buffer> buf3 = AMF::encodeString(domainname);
    memcpy(ptr, buf3->begin(), buf3->size());
    ptr += buf3->size();
	
//Put the date into memory when it is not empty

  	log_debug(_(" ***** The size of the data is %s *****"),data.size() ); 
      if (data.size()==0){	    	  
		   for(iter = data.begin(); iter != data.end(); ++iter){
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
    
//	system("ipcs");
       return;
}






/// \brief Read the date from the memory
///
/// @param dataname The name of the data to read.
///
/// @param data A vector of smart pointers to the AMF0 Elements in
///		this memory segment.
///
/// @return nothing.
/// We may only need a connection name for the receive function.
///void recv(std::string &name, std::string &dataname, boost::shared_ptr<cygnal::Element> data)
//{
	 //GNASH_REPORT_FUNCTION;

///	log_debug(_(" ***** The recv function is called *****") );

///  TODO:
///  This function should at do the following work:
///  1: Lock the shared memory
///			boost::mutex::scoped_lock lock(_localconnection_mutex);
///  2: Check if the current object is the listener
///         if findListener()
///         	Make sure the object is the listener for certain connection name
///  2: Parse the header
///         boost::uint8_t *parseHeader(boost::uint8_t *data, boost::uint8_t* tooFar);
///         	This should be easy if parseHeader function has been finished.
///  3: Parse the body of the shared memory
/// 	    std::vector<boost::shared_ptr<cygnal::Element> > parseBody(boost::uint8_t *data);
///         	This should be easy if parseHeader function has been finished.
///  4: The listened should implement these commands somehow automatically .
///         Handler?

///	return;	
//	}


///  \brief Dump the internal data of this class in a human readable form.
/// @remarks This should only be used for debugging purposes.
void
LcShm::dump()
{
//    GNASH_REPORT_FUNCTION;

//     cerr <<"Timestamp: " << _header.timestamp << endl;
//     cerr << "Length: " << _header.length << endl;
    using namespace std;
    cerr << "Connection Name:\t" << _object.connection_name << endl;
    cerr << "Hostname Name:\t\t" << _object.hostname << endl;
    cerr << "Domain Allowed:\t\t" << ((_object.domain) ? "true" : "false") << endl;
    vector<boost::shared_ptr<Element> >::iterator ait;
    cerr << "# of Elements in file: " << _amfobjs.size() << endl;
    for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ++ait) {
	boost::shared_ptr<Element> el = (*(ait));
        el->dump();
    }

    vector<string>::const_iterator lit;
    auto_ptr< vector<string> > listeners ( listListeners() );
    cerr << "# of Listeners in file: " << listeners->size() << endl;
    for (lit=listeners->begin(); lit!=listeners->end(); ++lit) {
        string str = *lit;
        if (str[0] != ':') {
            cerr << "Listeners:\t" << str << endl;
        }
    }
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
