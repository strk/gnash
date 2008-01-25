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
#include <string>
#include <vector>

#include "amf.h"
#include "element.h"
#include "shm.h"

namespace gnash {

// Manipulate the list of LocalConnection Listeners. We've made this a separate
// class from LocalConnection as it's used standalone for the dumpshm utility to
// dump the Listener lists.
class Listener {
public:
    Listener();
    Listener(boost::uint8_t *baseaddr);
    ~Listener();
    bool addListener(std::string &name);
    bool findListener(std::string &name);
    bool removeListener(std::string &name);
    std::vector<std::string> *listListeners();
    void setBaseAddress(boost::uint8_t *addr) { _baseaddr = addr; };
    boost::uint8_t *getBaseAddress() { return _baseaddr; };
protected:
    std::string _name;
    boost::uint8_t *_baseaddr;
//    std::vector<std::string> _listeners;
};

class LcShm : public Listener, public Shm {
public:
    typedef struct {
        uint32_t unknown1;
        uint32_t unknown2;
        uint32_t timestamp; // number of milliseconds that have elapsed since the system was started
        uint32_t length;
    } lc_header_t;
    typedef struct {
        std::string connection_name;
        std::string protocol;
        std::string method_name;
        std::vector<amf::Element *> data; // this can be any AMF data type
    } lc_message_t;
    typedef struct {
	std::string connection_name;
	std::string hostname;
        bool domain;
        double unknown_num1;
        double unknown_num2;
    } lc_object_t;
    LcShm();
    LcShm(boost::uint8_t *baseaddr);
    LcShm(key_t key);
    ~LcShm();
    bool connect(std::string &name);
    bool connect(key_t key);
    void close(void);
    void send(const std::string &name, const std::string &dataname, amf::Element *data);
    void recv(std::string &name, std::string &dataname, amf::Element *data);
    std::vector<amf::Element *> parseBody(boost::uint8_t *data);
    boost::uint8_t *parseHeader(boost::uint8_t *data);
    boost::uint8_t *formatHeader(boost::uint8_t *data);
    void addObject(amf::Element *el) { _amfobjs.push_back(el); };
    size_t size() { return _amfobjs.size(); };
    std::vector<amf::Element *> getElements() { return _amfobjs; };

    void setBaseAddr(boost::uint8_t *x) { _baseaddr = x; };
    void dump();
private:
    boost::uint8_t *_baseaddr;
    lc_header_t _header;
    lc_object_t _object;
    std::vector<amf::Element *> _amfobjs;
};

} // end of gnash namespace

// __LCSHM_H__
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

