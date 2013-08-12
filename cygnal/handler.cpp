// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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
#include <boost/shared_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <string>
#include <deque>
#include <list>
#include <map>
#include <vector>
#if defined(WIN32) || defined(_WIN32)
# define LIBLTDL_DLL_IMPORT 1
#endif
#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif
#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif

#include "ltdl.h"
#include "log.h"
#include "network.h"
#include "buffer.h"
#include "utility.h"
#include "dsodefs.h" //For DSOEXPORT.
#include "URL.h"
#include "handler.h"
#include "diskstream.h"
#include "rtmp.h"
#include "http.h"
#include "crc.h"
#include "flv.h"

#include "rtmp_server.h"
#include "http_server.h"

using namespace gnash;
using namespace std;
using namespace boost;

namespace cygnal
{

map<int, Handler *> DSOEXPORT handlers;

// The user config for Cygnal is loaded and parsed here:
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

Handler::Handler()
    :_streams(1),	// note that stream 0 is reserved by the system.
     // _diskstreams(new gnash::DiskStream[STREAMS_BLOCK]),     
     _in_fd(0)
{
//    GNASH_REPORT_FUNCTION;
    // reserve memory for the vector as it makes vector operations
    // must faster.
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
Handler::addClient(int fd, Network::protocols_supported_e proto)
{
    // GNASH_REPORT_FUNCTION;

    boost::mutex::scoped_lock lock(_mutex);
    
    log_debug("Adding %d to the client array.", fd);
    switch (proto) {
      case Network::NONE:
	  break;
      case Network::HTTP:
      {
	  boost::shared_ptr<HTTPServer> http(new HTTPServer);
	  _http[fd] = http;
	  break;
      }
      case Network::HTTPS:
	  break;
      case Network::RTMP:
      {
	  boost::shared_ptr<RTMPServer> rtmp(new RTMPServer);
	  _rtmp[fd] = rtmp;
	  break;
      }
      case Network::RTMPT:
      case Network::RTMPTS:
      case Network::RTMPE:
      case Network::RTMPS:
      case Network::DTN:
      default:
	  log_unimpl(_("Protocol %d for Handler::AddClient()"), proto);
	  break;
    }

    _clients.push_back(fd);
    _protocol[fd] = proto;
    
    return _clients.size();
}

// Parse the first nessages when starting a new message handler,
// which is used to determine the name of the resource to
// initialize, or load from the cache.
cygnal::Buffer *
Handler::parseFirstRequest(int fd, gnash::Network::protocols_supported_e proto)
{
    GNASH_REPORT_FUNCTION;
    string key;
    Network net;
    cygnal::Buffer *buf = 0;
    boost::mutex::scoped_lock lock(_mutex);
    
    switch (proto) {
      case Network::NONE:
	  break;
      case Network::HTTP:
      {
#if 0
	  int ret = _http[fd]->readNet(fd, buf);
	  if (ret) {
	      _http[fd]->processHeaderFields(buf);
	      string hostname, path;
	      string::size_type pos = _http[fd]->getField("host").find(":", 0);
	      if (pos != string::npos) {
		  hostname += _http[fd]->getField("host").substr(0, pos);
	      } else {
		  hostname += "localhost";
	      }
	      path = _http[fd]->getFilespec();
	      key = hostname + path;
	      log_debug("HTTP key is: %s", key);
	      _keys[fd] = key;
	  } else {
	      log_error(_("HTTP key couldn't be read!"));
	  }
#else
	  HTTPServer http;
	  size_t bytes = http.sniffBytesReady(fd);
	  if (bytes) {
	      buf = new cygnal::Buffer(bytes);
	  } else {
	      return 0;
	  }
	  int ret = http.readNet(fd, buf);
	  if (ret) {
	      http.processHeaderFields(buf);
	      string hostname, path;
	      string::size_type pos = http.getField("host").find(":", 0);
	      if (pos != string::npos) {
		  hostname += http.getField("host").substr(0, pos);
	      } else {
		  hostname += "localhost";
	      }
	      path = http.getFilespec();
	      key = hostname + path;
	      log_debug("HTTP key is: %s", key);
	      _keys[fd] = key;
	  } else {
	      log_error(_("HTTP key couldn't be read!"));
	  }	  
#endif
	  break;
      }
      case Network::HTTPS:
	  break;
      case Network::RTMP:
      {
	  // _rtmp[fd]->recvMsg(fd);
	  break;
      }
      case Network::RTMPT:
      case Network::RTMPTS:
      case Network::RTMPE:
      case Network::RTMPS:
      case Network::DTN:
      default:
	  log_error(_("FD #%d has no protocol handler registered"), fd);
	  break;
    };

    return buf;
}

int
Handler::recvMsg(int fd)
{
    // GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(_mutex);

    switch (_protocol[fd]) {
      case Network::NONE:
	  break;
      case Network::HTTP:
      {
	  return _http[fd]->recvMsg(fd);
	  break;
      }
      case Network::HTTPS:
	  break;
      case Network::RTMP:
      case Network::RTMPT:
      case Network::RTMPTS:
      case Network::RTMPE:
      case Network::RTMPS:
      case Network::DTN:
      default:
	  log_error(_("FD #%d has no protocol handler registered"), fd);
	  break;
    }

    return 0;
}

void
Handler::removeClient(int x)
{
    // GNASH_REPORT_FUNCTION;

    boost::mutex::scoped_lock lock(_mutex);

    vector<int>::iterator it;
    for (it = _clients.begin(); it < _clients.end(); ++it) {
	if (*it == x) {
	    log_debug("Removing %d from the client array.", *it);
	    _clients.erase(it);
	}
    }
}

void 
Handler::setPlugin(boost::shared_ptr<Handler::cygnal_init_t> &/* init */)
{
//    GNASH_REPORT_FUNCTION;
//     _plugin.reset(init.get());
}

void
Handler::setPlugin(Handler::cygnal_io_read_t /* read_ptr */, Handler::cygnal_io_write_t /* write_ptr */)
{
//    GNASH_REPORT_FUNCTION;

    _plugin.reset(new Handler::cygnal_init_t);
}

boost::shared_ptr<Handler::cygnal_init_t>
Handler::initModule(const std::string& str)
{
    // GNASH_REPORT_FUNCTION;

    if (str.empty()) {
	return _plugin;
    }

    string module = str;
    if (module[0] == '/') {
	module.erase(0,1);
    }
    
    SharedLib *sl;
    string symbol(module);

    _pluginsdir = PLUGINSDIR;
    log_security(_("Initializing module: \"%s\" from %s"), symbol, _pluginsdir);
    
    // Update the list of loaded plugins so we only load them once.
    if (_plugins[module] == 0) {
        sl = new SharedLib(module);
	lt_dlsetsearchpath(_pluginsdir.c_str());
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
	log_network(_("Initialized Plugin: \"%s\": %s"), info->version,
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
     Handler::cygnal_io_write_t write_symptr = reinterpret_cast<Handler::cygnal_io_write_t>
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
    // GNASH_REPORT_FUNCTION;
    size_t ret = 0;
    if (_plugin) {
	ret = _plugin->write_func(data, size);
    }

    return ret;
}

boost::shared_ptr<cygnal::Buffer>
Handler::readFromPlugin()
{
    // GNASH_REPORT_FUNCTION;

    boost::shared_ptr<cygnal::Buffer> buf;
    if (_plugin) {
	buf = _plugin->read_func();
    }

    return buf;
}

bool 
Handler::initialized()
{
    // GNASH_REPORT_FUNCTION;
    if (_files.empty()
	&& (_clients.size() == 1)
	&& !_local
	&& _remote.empty()
	&& !_plugin) {
	return false;
    }

    return true;
}

// Find a stream in the vector or Disk Streams
boost::shared_ptr<gnash::DiskStream>
Handler::findStream(const std::string &filespec)
{
//    GNASH_REPORT_FUNCTION;
    
    for (int i = 0; i < _streams; i++) {
	if (_diskstreams[i]->getFilespec() == filespec) {
	    return _diskstreams[i];
	}
    }

    return _diskstreams[0];
}

// Create a new DiskStream
double
Handler::createStream(double /* transid */)
{
    GNASH_REPORT_FUNCTION;

    _diskstreams[_streams]->setState(DiskStream::CREATED);

    return _streams;
}

// Create a new DiskStream
double
Handler::createStream(double /* transid */, const std::string &filespec)
{
    GNASH_REPORT_FUNCTION;

    if (filespec.empty()) {
	return -1;
    }
    
    _diskstreams[_streams]->setState(DiskStream::CREATED);
    _diskstreams[_streams]->setFilespec(filespec);
    
    return _streams;
}

bool
Handler::playStream()
{
    GNASH_REPORT_FUNCTION;

    // _diskstreams[int(streamid)]->setState(DiskStream::PLAY);

    return false;
}

bool
Handler::playStream(const std::string &filespec)
{
    GNASH_REPORT_FUNCTION;

    boost::shared_ptr<gnash::DiskStream> ds = _diskstreams[_streams];

    string fullpath = crcfile.getDocumentRoot();
    fullpath += "/";
    fullpath += filespec;
    log_debug("FILENAME: %s", fullpath);

    // gnash::DiskStream &ds = findStream(filespec);
    if (ds->getState() == DiskStream::CREATED) {
	if (ds->open(fullpath)) {
	    ds->loadToMem(0); // FIXME: load only part of the whole file for now
	    ds->setState(DiskStream::PLAY);
	    return true;
	}
    }
    
    return false;
}

// Publish a live stream
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

    // _diskstreams[int(streamid)]->setState(DiskStream::PUBLISH);

    return -1;
}

// Seek within the RTMP stream
int
Handler::seekStream()
{
    GNASH_REPORT_FUNCTION;
    // _diskstreams[int(streamid)]->setState(DiskStream::SEEK);
    
    return -1;
}

int
Handler::seekStream(int /* offset */)
{
    GNASH_REPORT_FUNCTION;
    // _diskstreams[int(streamid)]->setState(DiskStream::SEEK);

    return -1;
}

// Pause the RTMP stream
int
Handler::pauseStream(double streamid)
{
    GNASH_REPORT_FUNCTION;

    _diskstreams[int(streamid)]->setState(DiskStream::PAUSE);

    return -1;
}

// Pause the RTMP stream
int
Handler::togglePause(double streamid)
{
    GNASH_REPORT_FUNCTION;

    if (_diskstreams[int(streamid)]->getState() == DiskStream::PAUSE) {
	_diskstreams[int(streamid)]->setState(DiskStream::PLAY);
    } if (_diskstreams[int(streamid)]->getState() == DiskStream::PLAY) {
	_diskstreams[int(streamid)]->setState(DiskStream::PAUSE);
    }

    return -1;
}

// Resume the paused RTMP stream
double
Handler::resumeStream(double streamid)
{
    GNASH_REPORT_FUNCTION;

    togglePause(streamid);
    
    return -1;
}

// Close the RTMP stream
double
Handler::closeStream(double streamid)
{
    GNASH_REPORT_FUNCTION;

    _diskstreams[int(streamid)]->setState(DiskStream::CLOSED);

    return -1;
}

// Delete the RTMP stream
double
Handler::deleteStream(double streamid)
{
    GNASH_REPORT_FUNCTION;

    _diskstreams[int(streamid)]->setState(DiskStream::NO_STATE);

    _streams++;

    return _streams;
}

// Dump internal data.
void
Handler::dump()
{
    const char *proto_str[] = {
	"NONE",
	"HTTP",
	"HTTPS",
	"RTMP",
	"RTMPT",
	"RTMPTS",
	"RTMPE",
	"RTMPS",
	"DTN"
    };

//    GNASH_REPORT_FUNCTION;
    cerr << "Currently there are " <<_clients.size() << " clients connected."
	 << endl;
    for (size_t i = 0; i < _clients.size(); i++) {
	cerr << "Client on fd #" << _clients[i] << " is using  "
	     << proto_str[_protocol[i]] << endl;
    }

    cerr << "Currently there are " << dec <<_diskstreams.size() << " DiskStreams."
	 << endl;
    map<int, boost::shared_ptr<DiskStream> >::iterator it;
    for (it = _diskstreams.begin(); it != _diskstreams.end(); ++it) {
	if (it->second) {
	    cerr << "DiskStream for fd #" << dec << it->first << endl;
	    it->second->dump();
	}
    }
    
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

