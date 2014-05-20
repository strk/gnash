// 
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#ifndef _RTMPMSG_H_
#define _RTMPMSG_H_

#include <cstdint>
#include <string>
#include <vector>

#include "amf.h"
#include "rtmp.h"
#include "element.h"
#include "network.h"
#include "buffer.h"

namespace gnash
{

class RTMPMsg
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
	SO_PERSISTENCE_MISMATCH,
	// Anything below here is specific to Gnash's implementation
	NS_CREATE_STREAM,
	NS_DELETE_STREAM
    } rtmp_status_e;
    typedef enum {
	FROM_CLIENT,			  // SWF player
	FROM_SERVER                      // Flash com server
    } rtmp_source_e;
    RTMPMsg();
    ~RTMPMsg();
    
    void addObject(std::shared_ptr<cygnal::Element> el) { _amfobjs.push_back(el); };
    size_t size() { return _amfobjs.size(); };
    std::vector<std::shared_ptr<cygnal::Element> > getElements() { return _amfobjs; };

    void setMethodName(const std::string &name) { _method = name; } ;
    std::string &getMethodName()         { return _method; };

    void setTransactionID(double num)         { _transid = num; };
    double getTransactionID()	         { return _transid; };

    rtmp_status_e checkStatus(std::shared_ptr<cygnal::Element> el);
    void setStatus(rtmp_status_e st)     { _status = st; };
    rtmp_status_e getStatus()	         { return _status; };

    void setChannel(std::uint8_t num) { _channel = num; };
    std::uint8_t getChannel()         { return _channel; } ;

    std::shared_ptr<cygnal::Element> operator[](size_t x);
    std::shared_ptr<cygnal::Element> at(size_t x) { return _amfobjs[x]; };

    /// \brief Find the named property for this Object.
    ///
    /// @param name An ASCII string that is the name of the property to
    ///		search for.
    ///
    /// @return A smart pointer to the Element for this property.
    DSOEXPORT std::shared_ptr<cygnal::Element> findProperty(const std::string &name);

//    void setHeaderData(RTMP::rtmp_head_t &qhead);
			
// Dump internal status to the terminal
    DSOEXPORT void dump();
    
  protected:
    rtmp_source_e	  _routing;
    rtmp_status_e	  _status;
    std::string           _method;
    double                _transid;
    std::vector<std::shared_ptr<cygnal::Element> > _amfobjs;
    std::uint8_t       _channel;
};

} // end of gnash namespace
// end of _RTMPMSG_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

