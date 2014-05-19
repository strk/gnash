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

#ifndef __LCSHM_H__
#define __LCSHM_H__

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "amf.h"
#include "element.h"
#include "SharedMem.h"
#include "dsodefs.h"

/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

/// \class Listener
///	Manipulate the list of LocalConnection Listeners. We've made
///	this a separate class from LocalConnection as it's used
///	standalone for the dumpshm utility to dump the Listener
///	lists.
class DSOEXPORT Listener {
public:
    /// \brief Construct a block of Listeners.
    ///	This constructs an uninitialized Listener block.
    Listener();
    
    /// \brief Construct a block Listeners at the specified address.
    ///
    /// @param baseaddr The address to use for the block of
    ///		Listeners.
    Listener(boost::uint8_t *baseaddr);

    /// \brief Delete the Listener block
    ~Listener();

    /// \brief Create a new Listener in the memory segment.
    ///
    /// @param name The name for the Listener.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool addListener(const std::string &name);
    
    /// \brief See if a connection name exists in our list of Listeners.
    ///
    /// @param name An ASCII string that is the name of the Listener
    ///		to search for.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool findListener(const std::string &name);
    
    /// \brief Remove the Listener for this Object.
    ///
    /// @param name An ASCII string that is the name of the Listener
    ///		to remove from the  memory segment..
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool removeListener(const std::string &name);

    /// \brief List the Listeners for this memory segment.
    ///
    /// @return A smart pointer to a vector of Listener names.
    ///
    /// @remarks This is only used for debugging
    std::unique_ptr< std::vector<std::string> > listListeners();

    /// \brief Set the base address for the block of Listeners.
    ///
    /// @param addr The address for the block of Listeners.
    ///
    /// @return nothing.
    void setBaseAddress(boost::uint8_t *addr) { _baseaddr = addr; };

    /// \brief Set the base address for the block of Listeners.
    ///
    /// @return A real pointer to the base address of the block of
    ///		Listeners in the memory segment.
    boost::uint8_t *getBaseAddress() { return _baseaddr; };
    
protected:
    /// \var LcShm::_name
    ///		The name of the Listener.
    std::string _name;

    /// \var LcShm::_baseaddr
    ///		The base address of the block of Listeners.
    boost::uint8_t *_baseaddr;
    
//    std::vector<std::string> _listeners;
};

/// \class LcShm
///	This class is formanipulating the LocalConnection memory segment.
class DSOEXPORT LcShm : public Listener, public gnash::SharedMem {
public:
    /// \struct LcShm::lc_header_t
    ///		Hold the data in the memory segment's header.
    typedef struct {
		boost::uint32_t unknown1;
		boost::uint32_t unknown2;
		boost::uint32_t timestamp;	// number of milliseconds that have
				// elapsed since the system was started
		boost::uint32_t length;
    } lc_header_t;
    /// \struct LcShm::lc_message_t
    ///		Hold the data for a single message in the memory segment.
    typedef struct {
        std::string connection_name;
        std::string protocol;
        std::string method_name;
        std::vector<std::shared_ptr<cygnal::Element> > data; // this can be any AMF data type
    } lc_message_t;
    /// \struct LcShm::lc_object.t
    ///		Hold the data for each connection to the memory segment.
    typedef struct {
	std::string connection_name;
	std::string hostname;
        bool domain;
        double unknown_num1;
        double unknown_num2;
    } lc_object_t;

    /// \brief Construct an uninitialized shared memory segment.
    LcShm();
    
    /// \brief Delete the shared memory segment.
    ~LcShm();

    /// \brief Construct an initialized shared memory segment.
    ///
    /// @param baseaddr The address to use for the memory segment.
    LcShm(boost::uint8_t *baseaddr);
    
    /// \brief Construct an initialized shared memory segment.
    ///
    /// @param key The SYSV style key to use for the memory segment.
    LcShm(key_t key);

    /// \brief Connect to a memory segment.
    ///
    /// @param name The name to use for POSIX shared memory, which is not
    ///		the default type used.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool connect(const std::string &name);

    /// \brief Connect to a memory segment.
    ///
    /// @param key The SYSV style key for the shared memory segment,
    ///		which is the default type used.
    ///
    /// @return true if this succeeded. false if it doesn't.
    bool connect(key_t key);

    /// \brief Close a memory segment.
    ///		This closes the shared memory segment, but the data
    ///		remains until the next reboot of the computer.
    ///
    /// @return nothing.    
    void close(void);

    /// \brief Put data in the memory segment
    ///		This puts data into the memory segment
    ///
    /// @param name The connection name for this connection
    ///
    /// @param dataname The name of the data to send.
    ///
    /// @param data A vector of smart pointers to the AMF0 Elements
    ///		containing the data for this memory segment.
    ///
    /// @return nothing.
    void send(const std::string& name, const std::string& dataname,
	      std::vector< cygnal::Element* >& data);

    /// \brief Read the date from the memory
    ///
    /// @param dataname The name of the data to read.
    ///
    /// @param data A vector of smart pointers to the AMF0 Elements in
    ///		this memory segment.
    ///
    /// @return nothing.
	/// We may only need a connection name for the receive function.
    void recv(std::string &name, std::string &dataname, std::shared_ptr<cygnal::Element> data);

    /// \brief Parse the body of a memory segment.
    ///
    /// @param data The real pointer to the address to start parsing from.
    ///
    /// @return A vector of smart pointers to the AMF0 Elements in
    ///		this memopry segment.
    std::vector<std::shared_ptr<cygnal::Element> > parseBody(boost::uint8_t *data);

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
    boost::uint8_t *parseHeader(boost::uint8_t *data, boost::uint8_t* tooFar);

    /// \brief Format the header for the memory segment.
    ///
    /// @param con The name of the connection.
    ///
    /// @param host The bostname of the connection, often "localhost"
    ///
    /// @param domain The domain the hostname is in.
    ///
    /// @return A real pointer to a header for a memory segment.
    boost::uint8_t *formatHeader(const std::string &con, const std::string &host, bool domain);

    /// \brief Set the name for this connection to the memory segment.
    ///
    /// @param name The name for this connection.
    ///
    /// @return nothing.
    void addConnectionName(std::string &name);

    /// \brief Set the hostname used for this connection to the memory segment.
    ///
    /// @param name The hostname for this connection, often "localhost".
    ///
    /// @return nothing.
    void addHostname(std::string &name);

    /// \brief Add an AMF0 Element array of data for this memory segment.
    ///
    /// @return 
    void addObject(std::shared_ptr<cygnal::Element> el) { _amfobjs.push_back(el); };

    /// \brief Get the number of AMF0 Elements stored in this class.
    ///
    /// @return The number of AMF0 Elements stored in this class.
    size_t size() { return _amfobjs.size(); };

    /// \brief Get the array of AMF0 objects stored by this class.
    ///
    /// @return A vector of smart pointers to AMF0 Elements.
    std::vector<std::shared_ptr<cygnal::Element> > getElements() { return _amfobjs; };

    /// \brief Set the base address to be used for the memory segment.
    ///
    /// @param addr The address to use for opening the memory segment.
    ///
    /// @return nothing.
    void setBaseAddr(boost::uint8_t *addr) { _baseaddr = addr; };

    ///  \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump();
	
	//Si
	//Moved this from LocalConnection class to here
	void setconnected(bool trueorfalse) { _connected=trueorfalse; return;  };
	bool getconnected(){return _connected;};
    
private:
    /// \var LcShm::_baseaddr.
    ///		The base address of the memory segment.
    boost::uint8_t *_baseaddr;

    /// \var LcShm::_header
    ///		A stored copy of the header for the memory segment.
    lc_header_t _header;

    /// \var LcShm::_object
    ///		A stored copy of the LocalConnection object for the memory segment.
    lc_object_t _object;

    /// \var LcShm::_amfobjs
    ///		A vector of AMF0 Elements in the memory segment.
	/// Is this necessary if we have put all the things to pass in the memory?
    std::vector<std::shared_ptr<cygnal::Element> > _amfobjs;
	
	///Si added
	/// This is the mutex that controls access to the sharedmemory
    boost::mutex        _localconnection_mutex;

    ///Si 
	///Moved from LocalConnectoin class to here.
    bool _connected;

};

} // end of gnash namespace

// __LCSHM_H__
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

