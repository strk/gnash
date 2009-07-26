// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
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

using namespace gnash;
using namespace std;
using namespace amf;

namespace gnash
{

RTMPMsg::RTMPMsg()
    : _routing(FROM_SERVER),
      _status(APP_SHUTDOWN),
      _streamid(0),
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

// All the result messages from the server are ASCII text, so they have to be parsed to
// determine what really happened. We return the numerical equivalant for each _result,
// error, or onStatus message, the actual data can be obtained from the Element.
// 
RTMPMsg::rtmp_status_e
RTMPMsg::checkStatus(boost::shared_ptr<amf::Element>  /* el */)
{
//    GNASH_REPORT_FUNCTION;
    if (_amfobjs.size() > 0) {
	vector<boost::shared_ptr<amf::Element> >::iterator pit;
	vector<boost::shared_ptr<amf::Element> >::iterator cit;
//	cerr << "# of Properties in object" << _amfobjs.size() << endl;
	for (pit = _amfobjs.begin(); pit != _amfobjs.end(); pit++) {
	    boost::shared_ptr<amf::Element> el = (*(pit));
	    std::vector<boost::shared_ptr<amf::Element> > props = el->getProperties();
//  	    printf("FIXME2: %d, %s:%s\n", props.size(),
//  		   props[2]->getName(), props[2]->to_string());
	    if (el->getType() == Element::OBJECT_AMF0) {
		for (cit = props.begin(); cit != props.end(); cit++) {
		    boost::shared_ptr<amf::Element> child = (*(cit));
//		    child->dump();
		    string name = child->getName();
		    string value;
		    if (child->getDataSize()) {
			value = child->to_string();
			if (name == "code") {
//			    log_debug("Name is: %s, Value is: %s", name.c_str(), value.c_str());
			    // Error messages we get as the result of a NetConnection::connect()
			    if (value == "NetConnection.Connect.Success") {
				_status = RTMPMsg::NC_CONNECT_SUCCESS;
				return _status;
			    }
			    if (value == "NetConnection.Call.Failed") {
				_status = RTMPMsg::NC_CALL_FAILED;
				return _status;
			    }
			    if (value == "NetConnection.Call.BadVersion") {
				_status = RTMPMsg::NC_CALL_BADVERSION;
				return _status;
			    }
			    if (value == "NetConnection.AppShutdown") {
				_status = RTMPMsg::NC_CONNECT_APPSHUTDOWN;
				return _status;
			    }
			    if (value == "NetConnection.Connect.Failed") {
				_status = RTMPMsg::NC_CONNECT_FAILED;
				return _status;
			    }
			    if (value == "NetConnection.Invalid.Application") {
				_status = RTMPMsg::NC_CONNECT_INVALID_APPLICATION;
				return _status;
			    }
			    if (value == "NetConnection.Connect.Rejected") {
				_status = RTMPMsg::NC_CONNECT_REJECTED;
				return _status;
			    }
			    
			    // we get this and then the FLV file data is next
			    if (value == "NetStream.Data.Start") {
				_status = RTMPMsg::NS_DATA_START;
				return _status;
			    }
			    // Error messages we get as the result of a NetStream::play()
			    if (value == "NetStream.Pause.Notify") {
				_status = RTMPMsg::NS_PAUSE_NOTIFY;
				return _status;
			    }
			    if (value == "NetStream.Play.Complete") {
				_status = RTMPMsg::NS_PLAY_COMPLETE;
				return _status;
			    }
			    if (value == "NetStream.Play.Failed") {
				_status = RTMPMsg::NS_PLAY_FAILED;
				return _status;
			    }
			    if (value == "NetStream.InvalidArg") {
				_status = RTMPMsg::NS_INVALID_ARGUMENT;
				return _status;
			    }
			    if (value == "NetStream.Play.File.Structure.Invalid") {
				_status = RTMPMsg::NS_PLAY_FILE_STRUCTURE_INVALID;
				return _status;
			    }
			    if (value == "NetStream.Play.Insufficient.BW") {
				_status = RTMPMsg::NS_PLAY_INSUFFICIENT_BW;
				return _status;
			    }
			    if (value == "NetStream.Play.No.Supported.Track.Found") {
				_status = RTMPMsg::NS_PLAY_NO_SUPPORTED_TRACK_FOUND;
				return _status;
			    }
			    if (value == "NetStream.Play.PublishNotify") {
				_status = RTMPMsg::NS_PLAY_PUBLISHNOTIFY;
				return _status;
			    }
			    if (value == "NetStream.Play.StreamNotFound") {
				_status = RTMPMsg::NS_PLAY_STREAMNOTFOUND;
				return _status;
			    }
			    if (value == "NetStream.Play.SWITCH") {
				_status = RTMPMsg::NS_PLAY_SWITCH;
				return _status;
			    }
			    if (value == "NetStream.Play.UnpublishNotify") {
				_status = RTMPMsg::NS_PLAY_UNPUBLISHNOTIFY;
				return _status;
			    }
			    if (value == "NetStream.Play.Start") {
				_status = RTMPMsg::NS_PLAY_START;
				return _status;
			    }
			    if (value == "NetStream.Play.Stop") {
				_status = RTMPMsg::NS_PLAY_STOP;
				return _status;
			    }
			    if (value == "NetStream.Play.Reset") {
				_status = RTMPMsg::NS_PLAY_RESET;
				return _status;
			    }
			    if (value == "NetStream.Publish.Badname") {
				_status = RTMPMsg::NS_PUBLISH_BADNAME;
				return _status;
			    }
			    if (value == "NetStream.Publish.Start") {
				_status = RTMPMsg::NS_PUBLISH_START;
				return _status;
			    }

			    if (value == "NetStream.Record.Failed") {
				_status = RTMPMsg::NS_RECORD_FAILED;
				return _status;
			    }
			    if (value == "NetStream.Record.Noaccess") {
				_status = RTMPMsg::NS_RECORD_NOACCESS;
				return _status;
			    }
			    if (value == "NetStream.Record.Start") {
				_status = RTMPMsg::NS_RECORD_START;
				return _status;
			    }
			    if (value == "NetStream.Record.Stop") {
				_status = RTMPMsg::NS_RECORD_STOP;
				return _status;
			    }
			    if (value == "NetStream.Seek.Failed") {
				_status = RTMPMsg::NS_SEEK_FAILED;
				return _status;
			    }
			    if (value == "NetStream.Seek.NOTIFY") {
				_status = RTMPMsg::NS_SEEK_NOTIFY;
				return _status;
			    }
			    if (value == "NetStream.Unpause.Notify") {
				_status = RTMPMsg::NS_UNPAUSE_NOTIFY;
				return _status;
			    }
			    if (value == "NetStream.Unpublished.Success") {
				_status = RTMPMsg::NS_UNPUBLISHED_SUCCESS;
				return _status;
			    }

			    // Error messages we get as the result of a SharedObject operation
			    if (value == "SharedObject.Creation.Failed") {
				_status = RTMPMsg::SO_CREATION_FAILED;
				return _status;
			    }
			    if (value == "SharedObject.No.Read.Access") {
				_status = RTMPMsg::SO_NO_READ_ACCESS;
				return _status;
			    }
			    if (value == "SharedObject.No.Write.Access") {
				_status = RTMPMsg::SO_NO_WRITE_ACCESS;
				return _status;
			    }
			    if (value == "SharedObject.Persistence.Mismatch") {
				_status = RTMPMsg::SO_PERSISTENCE_MISMATCH;
				return _status;
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

boost::shared_ptr<amf::Element>
RTMPMsg::operator[](size_t index)
{
//    GNASH_REPORT_FUNCTION;
    if (index <= _amfobjs.size()) {
	return _amfobjs[index];
    }
    
    boost::shared_ptr<amf::Element> el;
    return el;
};

/// \brief Find the named property for this Object.
///
/// @param name An ASCII string that is the name of the property to
///	search for.
///
/// @return A smart pointer to the Element for this property.
boost::shared_ptr<amf::Element> 
RTMPMsg::findProperty(const std::string &name)
{
    if (_amfobjs.size() > 0) {
	vector<boost::shared_ptr<Element> >::iterator ait;
//	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ait++) {
	    boost::shared_ptr<amf::Element> el = (*(ait));
	    boost::shared_ptr<amf::Element> prop = el->findProperty(name);
	    if (prop) {
		return prop;
	    }
	}
    }
    boost::shared_ptr<Element> el;
    return el;
}


void
RTMPMsg::dump()
{
//    GNASH_REPORT_FUNCTION;

//     cerr <<"Timestamp: " << _header.timestamp << endl;
//     cerr << "Length: " << _header.length << endl;
    
    cerr << "Method Name:\t" << _method << endl;
//    cerr << "Stream ID:\t" << hexify((const unsigned char *)&_streamid, 8, false) << endl;
    cerr << "Stream ID:\t" << _streamid << endl;

    vector<boost::shared_ptr<amf::Element> >::iterator ait;
    cerr << "# of Elements in file: " << _amfobjs.size() << endl;
    for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ait++) {
	boost::shared_ptr<amf::Element> el = (*(ait));
        el->dump();
    }
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
