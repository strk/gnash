// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <string>
#include <deque>
#include <list>
#include <map>
#include <vector>

#include "log.h"
#include "network.h"
#include "buffer.h"
#include "utility.h"
#include "dsodefs.h" //For DSOEXPORT.
#include "handler.h"

#include "rtmp.h"
#include "http.h"

using namespace gnash;
using namespace std;
using namespace boost;

namespace cygnal
{

const char *proto_str[] = {
    "NONE",
    "HTTP",
    "RTMP",
    "RTMPT",
    "RTMPTS",
    "RTMPE",
    "RTMPS",
    "DTN"
};

map<int, Handler *> DSOEXPORT handlers;

Handler::Handler()
    : _streams(2),		// streams 0 and 1 appear to be
				// reserved by the system.      
      _in_fd(0)
{
//    GNASH_REPORT_FUNCTION;
}

Handler::~Handler()
{
//    GNASH_REPORT_FUNCTION;
}

bool
Handler::sync(int /* in_fd */)
{
//    GNASH_REPORT_FUNCTION;

    return false;
}

size_t
Handler::addClient(int x, protocols_supported_e proto)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    _clients.push_back(x);
    _protocol[x] = proto;
    
    return _clients.size();
};

void
Handler::removeClient(int x)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);
    _clients.erase(_clients.begin()+x);
}

void 
Handler::setPlugin(boost::shared_ptr<Handler::cygnal_init_t> &/* init */)
{
//    GNASH_REPORT_FUNCTION;
//     _plugin.reset(init.get());
}

void
Handler::setPlugin(Handler::cygnal_io_t /* read_ptr */, Handler::cygnal_io_t /* write_ptr */)
{
//    GNASH_REPORT_FUNCTION;

    _plugin.reset(new Handler::cygnal_init_t);
}

boost::shared_ptr<Handler::cygnal_init_t>
Handler::initModule(const std::string& module)
{
    GNASH_REPORT_FUNCTION;

    SharedLib *sl;
    std::string symbol(module);

    log_security(_("Initializing module: \"%s\""), symbol);
    
    _pluginsdir = "/usr/local/lib/cygnal/";
    lt_dlsetsearchpath(_pluginsdir.c_str());

    // Update the list of loaded plugins so we only load them once.
    if (_plugins[module] == 0) {
        sl = new SharedLib(module, "CYGNAL_PLUGINS");
        sl->openLib();
        _plugins[module] = sl;
    } else {
        sl = _plugins[module];
    }

    _plugin.reset(new Handler::cygnal_init_t);

    symbol = module;
    symbol.append("_init_func");
    Handler::cygnal_io_init_t init_symptr = reinterpret_cast<Handler::cygnal_io_init_t>
	(sl->getInitEntry(symbol));
    if (!init_symptr) {
	log_network(_("No %s symbol in plugin"), symbol);
    } else {
	boost::shared_ptr<cygnal_init_t> info = init_symptr(_netconnect);
	log_network("Initialized Plugin: \"%s\": %s", info->version,
		    info->description);
    }
    
    // Look for the "module"_read_init function we'll use to get data
    // from the cgi-bin as a dynamically loadable plugin.
    symbol = module;
    symbol.append("_read_func");
    
    Handler::cygnal_io_read_t read_symptr = reinterpret_cast<Handler::cygnal_io_read_t>
	(sl->getInitEntry(symbol));

     if (!read_symptr) {    
         log_error(_("Couldn't get %s symbol"), symbol);
	 _plugin.reset();
 	 return _plugin;
     }

     _plugin->read_func = read_symptr;

     // Look for the "module"_write_init function we'll use to send data
     // to the cgi-bin as a dynamically loadable plugin.
     symbol = module;
     symbol.append("_write_func");
     Handler::cygnal_io_t write_symptr = reinterpret_cast<Handler::cygnal_io_t>
	(sl->getInitEntry(symbol));

     if (!write_symptr) {    
         log_error(_("Couldn't get %s symbol"), symbol);
	 _plugin.reset();
	 return _plugin;
     }

     _plugin->write_func = write_symptr;

    return _plugin;
}

size_t
Handler::writeToPlugin(boost::uint8_t *data, size_t size)
{
    GNASH_REPORT_FUNCTION;

    size_t ret = 0;
    if (_plugin) {
	ret = _plugin->write_func(data, size);
    }

    return ret;
}

boost::shared_ptr<amf::Buffer> &
Handler::readFromPlugin()
{
    GNASH_REPORT_FUNCTION;
    size_t ret = 0;
    boost::shared_ptr<amf::Buffer> buf;
    if (_plugin) {
	buf = _plugin->read_func();
    }

    return buf;
}

bool 
Handler::initialized()
{
//    GNASH_REPORT_FUNCTION;
    if (_files.empty()
	&& (_clients.size() == 1)
	&& !_local
	&& _remote.empty()
	&& !_plugin) {
	return false;
    }

    return true;
}

int 
Handler::createStream()
{
    GNASH_REPORT_FUNCTION;

    return createStream("");
}

int
Handler::createStream(const std::string &filespec)
{
    GNASH_REPORT_FUNCTION;

    int streamid = _streams;

    if (filespec.empty()) {
	return -1;
    }
    _streams++;
    return streamid;
}

int
Handler::playStream()
{
    GNASH_REPORT_FUNCTION;

    return -1;
}

int
Handler::playStream(const std::string &/* filespec */)
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

// Publish a live RTMP stream
int
Handler::publishStream()
{
    GNASH_REPORT_FUNCTION;
    return publishStream("", Handler::LIVE);
}

int
Handler::publishStream(const std::string &/*filespec */, Handler::pub_stream_e /* op
									   */)
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

// Seek within the RTMP stream
int
Handler::seekStream()
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

int
Handler::seekStream(int /* offset */)
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

// Pause the RTMP stream
int
Handler::pauseStream()
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

// Pause the RTMP stream
int
Handler::togglePause()
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

// Resume the paused RTMP stream
int
Handler::resumeStream()
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

// Close the RTMP stream
int
Handler::closeStream()
{
    GNASH_REPORT_FUNCTION;
    return -1;
}

// Dump internal data.
void
Handler::dump()
{
//    GNASH_REPORT_FUNCTION;
    for (size_t i = 0; i < _clients.size(); i++) {
	cerr << "Client on fd #" << _clients[i] << " is using  "
	     << proto_str[_protocol[i]] << endl;
    }
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

