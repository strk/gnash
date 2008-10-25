// 
//   Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef _RTMPMSG_H_
#define _RTMPMSG_H_

#include <boost/cstdint.hpp>
#include <string>
#include <vector>

#include "amf.h"
#include "rtmp.h"
#include "element.h"
#include "handler.h"
#include "network.h"
#include "buffer.h"

namespace gnash
{

class DSOEXPORT RTMPMsg
{
public:
    typedef enum {
	APP_GC,
	APP_RESOURCE_LOWMEMORY,
	APP_SCRIPT_ERROR,
	APP_SCRIPT_WARNING,
	APP_SHUTDOWN,
	NC_CALL_BADVERSION,
	NC_CALL_FAILED,
	NC_CONNECT_APPSHUTDOWN,
	NC_CONNECT_CLOSED,
	NC_CONNECT_FAILED,
	NC_CONNECT_INVALID_APPLICATION,
	NC_CONNECT_REJECTED,
	NC_CONNECT_SUCCESS,
	NS_CLEAR_FAILED,
	NS_CLEAR_SUCCESS,
	NS_DATA_START,
	NS_FAILED,
	NS_INVALID_ARGUMENT,
	NS_PAUSE_NOTIFY,
	NS_PLAY_COMPLETE,
	NS_PLAY_FAILED,
	NS_PLAY_FILE_STRUCTURE_INVALID,
	NS_PLAY_INSUFFICIENT_BW,
	NS_PLAY_NO_SUPPORTED_TRACK_FOUND,
	NS_PLAY_PUBLISHNOTIFY,
	NS_PLAY_RESET,
	NS_PLAY_START,
	NS_PLAY_STOP,
	NS_PLAY_STREAMNOTFOUND,
	NS_PLAY_SWITCH,
	NS_PLAY_UNPUBLISHNOTIFY,
	NS_PUBLISH_BADNAME,
	NS_PUBLISH_START,
	NS_RECORD_FAILED,
	NS_RECORD_NOACCESS,
	NS_RECORD_START,
	NS_RECORD_STOP,
	NS_SEEK_FAILED,
	NS_SEEK_NOTIFY,
	NS_UNPAUSE_NOTIFY,
	NS_UNPUBLISHED_SUCCESS,
	SO_CREATION_FAILED,
	SO_NO_READ_ACCESS,
	SO_NO_WRITE_ACCESS,
	SO_PERSISTENCE_MISMATCH
    } rtmp_status_e;
    typedef enum {
	FROM_SERVER,                      // Flash com server
	FROM_CLIENT			  // SWF player
    } rtmp_source_e;
    RTMPMsg();
    ~RTMPMsg();
    
    void addObject(boost::shared_ptr<amf::Element> el) { _amfobjs.push_back(el); };
    size_t size() { return _amfobjs.size(); };
    std::vector<boost::shared_ptr<amf::Element> > getElements() { return _amfobjs; };

    void setMethodName(const std::string &name) { _method = name; } ;
    std::string &getMethodName()         { return _method; };

    void setStreamID(double num)         { _streamid = num; };
    double getStreamID()	         { return _streamid; };

    rtmp_status_e checkStatus(boost::shared_ptr<amf::Element> el);
    void setStatus(rtmp_status_e st)     { _status = st; };
    rtmp_status_e getStatus()	         { return _status; };

    void setChannel(Network::byte_t num) { _channel = num; };
    Network::byte_t getChannel()         { return _channel; } ;

    boost::shared_ptr<amf::Element> operator[](size_t x);

    // Dump internal status to the terminal
    void dump();
    
  protected:
    rtmp_source_e	  _routing;
    rtmp_status_e	  _status;
    std::string           _method;
    double                _streamid;
    std::vector<boost::shared_ptr<amf::Element> > _amfobjs;
    Network::byte_t       _channel;
};

} // end of gnash namespace
// end of _RTMPMSG_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

