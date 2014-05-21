// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <string>
#include <map>

#if ! (defined(_WIN32) || defined(WIN32))
#	include <netinet/in.h>
#endif

#include "log.h"
#include "amf.h"
#include "rtmp.h"
#include "rtmp_msg.h"
#include "network.h"
#include "element.h"
// #include "handler.h"
#include "utility.h"
#include "buffer.h"

using std::vector;
using cygnal::Element;

namespace gnash
{

RTMPMsg::RTMPMsg()
    : _routing(FROM_SERVER),
      _status(APP_SHUTDOWN),
      _transid(0),
      _channel(0)
{
//    GNASH_REPORT_FUNCTION;
//     _inbytes = 0;
//     _outbytes = 0;
    
//    _body = new unsigned char(RTMP_HANDSHAKE_SIZE+1);
//    memset(_body, 0, RTMP_HANDSHAKE_SIZE+1);
}

RTMPMsg::~RTMPMsg()
{
//    GNASH_REPORT_FUNCTION;
}

struct RTMPStatusMsgCode {
    const char *msg;
    RTMPMsg::rtmp_status_e code;
};

static RTMPStatusMsgCode rtmp_msg_code_list[] = {
    // Error messages we get as the result of a NetConnection::connect()
    { "NetConnection.Connect.Success",           RTMPMsg::NC_CONNECT_SUCCESS },
    { "NetConnection.Call.Failed",               RTMPMsg::NC_CALL_FAILED },
    { "NetConnection.Call.BadVersion",           RTMPMsg::NC_CALL_BADVERSION },
    { "NetConnection.AppShutdown",               RTMPMsg::NC_CONNECT_APPSHUTDOWN },
    { "NetConnection.Connect.Failed",            RTMPMsg::NC_CONNECT_FAILED },
    { "NetConnection.Invalid.Application",       RTMPMsg::NC_CONNECT_INVALID_APPLICATION },
    { "NetConnection.Connect.Rejected",          RTMPMsg::NC_CONNECT_REJECTED },

    // we get this and then the FLV file data is next
    { "NetStream.Data.Start",                    RTMPMsg::NS_DATA_START },

    // Error messages we get as the result of a NetStream::play()
    { "NetStream.Pause.Notify",                  RTMPMsg::NS_PAUSE_NOTIFY },
    { "NetStream.Play.Complete",                 RTMPMsg::NS_PLAY_COMPLETE },
    { "NetStream.Play.Failed",                   RTMPMsg::NS_PLAY_FAILED },
    { "NetStream.InvalidArg",                    RTMPMsg::NS_INVALID_ARGUMENT },
    { "NetStream.Play.File.Structure.Invalid",   RTMPMsg::NS_PLAY_FILE_STRUCTURE_INVALID },
    { "NetStream.Play.Insufficient.BW",          RTMPMsg::NS_PLAY_INSUFFICIENT_BW },
    { "NetStream.Play.No.Supported.Track.Found", RTMPMsg::NS_PLAY_NO_SUPPORTED_TRACK_FOUND },
    { "NetStream.Play.PublishNotify",            RTMPMsg::NS_PLAY_PUBLISHNOTIFY },
    { "NetStream.Play.StreamNotFound",           RTMPMsg::NS_PLAY_STREAMNOTFOUND },
    { "NetStream.Play.SWITCH",                   RTMPMsg::NS_PLAY_SWITCH },
    { "NetStream.Play.UnpublishNotify",          RTMPMsg::NS_PLAY_UNPUBLISHNOTIFY },
    { "NetStream.Play.Start",                    RTMPMsg::NS_PLAY_START },
    { "NetStream.Play.Stop" ,                    RTMPMsg::NS_PLAY_STOP },
    { "NetStream.Play.Reset",                    RTMPMsg::NS_PLAY_RESET },
    { "NetStream.Publish.Badname",               RTMPMsg::NS_PUBLISH_BADNAME },
    { "NetStream.Publish.Start",                 RTMPMsg::NS_PUBLISH_START },
    { "NetStream.Record.Failed",                 RTMPMsg::NS_RECORD_FAILED },
    { "NetStream.Record.Noaccess",               RTMPMsg::NS_RECORD_NOACCESS },
    { "NetStream.Record.Start",                  RTMPMsg::NS_RECORD_START },
    { "NetStream.Record.Stop",                   RTMPMsg::NS_RECORD_STOP },
    { "NetStream.Seek.Failed",                   RTMPMsg::NS_SEEK_FAILED },
    { "NetStream.Seek.NOTIFY",                   RTMPMsg::NS_SEEK_NOTIFY },
    { "NetStream.Unpause.Notify",                RTMPMsg::NS_UNPAUSE_NOTIFY },
    { "NetStream.Unpublished.Success",           RTMPMsg::NS_UNPUBLISHED_SUCCESS },									

    // Error messages we get as the result of a SharedObject operation
    { "SharedObject.Creation.Failed",            RTMPMsg::SO_CREATION_FAILED },
    { "SharedObject.No.Read.Access",             RTMPMsg::SO_NO_READ_ACCESS },
    { "SharedObject.No.Write.Access",            RTMPMsg::SO_NO_WRITE_ACCESS },
    { "SharedObject.Persistence.Mismatch",       RTMPMsg::SO_PERSISTENCE_MISMATCH },
    { nullptr, RTMPMsg::NC_CONNECT_FAILED }
};

// All the result messages from the server are ASCII text, so they have to be parsed to
// determine what really happened. We return the numerical equivalant for each _result,
// error, or onStatus message, the actual data can be obtained from the Element.
// 
RTMPMsg::rtmp_status_e
RTMPMsg::checkStatus(std::shared_ptr<cygnal::Element>  /* el */)
{
//    GNASH_REPORT_FUNCTION;
    if (_amfobjs.size() > 0) {
	vector<std::shared_ptr<cygnal::Element> >::iterator pit;
	vector<std::shared_ptr<cygnal::Element> >::iterator cit;
//	cerr << "# of Properties in object" << _amfobjs.size() << endl;
	for (pit = _amfobjs.begin(); pit != _amfobjs.end(); ++pit) {
	    std::shared_ptr<cygnal::Element> el = (*(pit));
	    std::vector<std::shared_ptr<cygnal::Element> > props = el->getProperties();
//  	    printf("FIXME2: %d, %s:%s\n", props.size(),
//  		   props[2]->getName(), props[2]->to_string());
	    if (el->getType() == Element::OBJECT_AMF0) {
		for (cit = props.begin(); cit != props.end(); ++cit) {
		    std::shared_ptr<cygnal::Element> child = (*(cit));
//		    child->dump();
		    std::string name = child->getName();
		    std::string value;
		    if (child->getDataSize()) {
			value = child->to_string();
			if (name == "code") {
//			    log_debug("Name is: %s, Value is: %s", name.c_str(), value.c_str());
			    for (RTMPStatusMsgCode *p = rtmp_msg_code_list; p->msg; p++) {
                                if (value == p->msg) {
                                    _status = p->code;
                                    return _status;
                                }
                            }
			}
		    }
		}
	    }
	}
    }
    return _status;
}

// void
// RTMPMsg::setHeaderData(RTMP::rtmp_head_t &qhead)
// {
    
// }

std::shared_ptr<cygnal::Element>
RTMPMsg::operator[](size_t index)
{
//    GNASH_REPORT_FUNCTION;
    if (index < _amfobjs.size()) {
	return _amfobjs[index];
    }
    
    std::shared_ptr<cygnal::Element> el;
    return el;
}

/// \brief Find the named property for this Object.
///
/// @param name An ASCII string that is the name of the property to
///	search for.
///
/// @return A smart pointer to the Element for this property.
std::shared_ptr<cygnal::Element>
RTMPMsg::findProperty(const std::string &name)
{
    if (_amfobjs.size() > 0) {
	vector<std::shared_ptr<Element> >::iterator ait;
//	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ++ait) {
	    std::shared_ptr<cygnal::Element> el = (*(ait));
	    std::shared_ptr<cygnal::Element> prop = el->findProperty(name);
	    if (prop) {
		return prop;
	    }
	}
    }
    std::shared_ptr<Element> el;
    return el;
}


void
RTMPMsg::dump()
{
    using namespace std;
//    GNASH_REPORT_FUNCTION;

//     cerr <<"Timestamp: " << _header.timestamp << endl;
//     cerr << "Length: " << _header.length << endl;
    
    cerr << "Method Name:\t" << _method << endl;
//    cerr << "Transaction ID:\t" << hexify((const unsigned char *)&_transid, 8, false) << endl;
    cerr << "Transaction ID:\t" << _transid << endl;

    vector<std::shared_ptr<cygnal::Element> >::iterator ait;
    cerr << "# of Elements in file: " << _amfobjs.size() << endl;
    for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ++ait) {
	std::shared_ptr<cygnal::Element> el = (*(ait));
        el->dump();
    }
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
