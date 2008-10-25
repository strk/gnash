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

#ifndef __LCSHM_H__
#define __LCSHM_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "amf.h"
#include "element.h"
#include "shm.h"
#include "dsodefs.h"

namespace gnash {

// Manipulate the list of LocalConnection Listeners. We've made this a separate
// class from LocalConnection as it's used standalone for the
// dumpshm utility to dump the Listener lists.
class DSOEXPORT Listener {
public:
    Listener();
    Listener(gnash::Network::byte_t *baseaddr);
    ~Listener();
    bool addListener(const std::string &name);
    bool findListener(const std::string &name);
    bool removeListener(const std::string &name);
    std::auto_ptr< std::vector<std::string> > listListeners();
    void setBaseAddress(gnash::Network::byte_t *addr) { _baseaddr = addr; };
    gnash::Network::byte_t *getBaseAddress() { return _baseaddr; };
protected:
    std::string _name;
    gnash::Network::byte_t *_baseaddr;
//    std::vector<std::string> _listeners;
};

class DSOEXPORT LcShm : public Listener, public Shm {
public:
    typedef struct {
        uint32_t unknown1;
        uint32_t unknown2;
        uint32_t timestamp;	// number of milliseconds that have
				// elapsed since the system was started
        uint32_t length;
    } lc_header_t;
    typedef struct {
        std::string connection_name;
        std::string protocol;
        std::string method_name;
        std::vector<boost::shared_ptr<amf::Element> > data; // this can be any AMF data type
    } lc_message_t;
    typedef struct {
	std::string connection_name;
	std::string hostname;
        bool domain;
        double unknown_num1;
        double unknown_num2;
    } lc_object_t;
    LcShm();
    LcShm(gnash::Network::byte_t *baseaddr);
    LcShm(key_t key);
    ~LcShm();
    bool connect(const std::string &name);
    bool connect(key_t key);
    void close(void);
    void send(const std::string &name, const std::string &dataname,
	      std::vector<boost::shared_ptr<amf::Element> > &data);
    void recv(std::string &name, std::string &dataname, boost::shared_ptr<amf::Element> data);
    std::vector<boost::shared_ptr<amf::Element> > parseBody(gnash::Network::byte_t *data);

    /// @param in
    ///    Pointer to start parsing from
    //
    /// @param tooFar
    ///    A pointer to one-byte-past the last valid memory
    ///    address within the buffer.
    ///
    /// May throw a ParserException 
    ///
    gnash::Network::byte_t *parseHeader(gnash::Network::byte_t *data, gnash::Network::byte_t* tooFar);

    gnash::Network::byte_t *formatHeader(const std::string &con, const std::string &host, bool domain);
    void addConnectionName(std::string &name);
    void addHostname(std::string &name);
    void addObject(boost::shared_ptr<amf::Element> el) { _amfobjs.push_back(el); };
    size_t size() { return _amfobjs.size(); };
    std::vector<boost::shared_ptr<amf::Element> > getElements() { return _amfobjs; };

    void setBaseAddr(gnash::Network::byte_t *x) { _baseaddr = x; };
    void dump();
private:
    gnash::Network::byte_t *_baseaddr;
    lc_header_t _header;
    lc_object_t _object;
    std::vector<boost::shared_ptr<amf::Element> > _amfobjs;
};

} // end of gnash namespace

// __LCSHM_H__
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

