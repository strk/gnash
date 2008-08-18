// rtmp.cpp:  Adobe/Macromedia Real Time Message Protocol handler, for Gnash.
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
#include "handler.h"
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
      _channel(9)
{
//    GNASH_REPORT_FUNCTION;
//     _inbytes = 0;
//     _outbytes = 0;
    
//    _body = new unsigned char(RTMP_BODY_SIZE+1);
//    memset(_body, 0, RTMP_BODY_SIZE+1);
}

RTMPMsg::~RTMPMsg()
{
//    GNASH_REPORT_FUNCTION;
    vector<amf::Element *>::iterator it;
    for (it = _amfobjs.begin(); it != _amfobjs.end(); it++) {
	amf::Element *el = (*(it));
	if (el) {
//             el->dump();
	    delete el;
	}
    }
    
}

RTMPMsg::rtmp_status_e
RTMPMsg::checkStatus(amf::Element * /* el */)
{
//    GNASH_REPORT_FUNCTION;
    if (_amfobjs.size() > 0) {
	vector<amf::Element *>::iterator pit;
	vector<amf::Element *>::iterator cit;
//	cerr << "# of Properties in object" << _amfobjs.size() << endl;
	for (pit = _amfobjs.begin(); pit != _amfobjs.end(); pit++) {
	    amf::Element *el = (*(pit));
	    std::vector<amf::Element *> props = el->getProperties();
//  	    printf("FIXME2: %d, %s:%s\n", props.size(),
//  		   props[2]->getName(), props[2]->to_string());
	    if (el->getType() == Element::OBJECT_AMF0) {
		for (cit = props.begin(); cit != props.end(); cit++) {
		    amf::Element *child = (*(cit));
//		    child->dump();
		    string name = child->getName();
		    string value;
		    if (child->getLength()) {
			value = child->to_string();
			if (name == "code") {
//			    log_debug("Name is: %s, Value is: %s", name.c_str(), value.c_str());
			    if (value == "NetConnection.Connect.Success") {
				_status = RTMPMsg::NC_CONNECT_SUCCESS;
				return _status;
			    }
			    if (value == "NetConnection.Connect.Failed") {
				_status = RTMPMsg::NC_CONNECT_FAILED;
				return _status;
			    }
			    if (value == "NetConnection.Call.Failed") {
				_status = RTMPMsg::NC_CALL_FAILED;	
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

void
RTMPMsg::dump()
{
//    GNASH_REPORT_FUNCTION;

//     cerr <<"Timestamp: " << _header.timestamp << endl;
//     cerr << "Length: " << _header.length << endl;
    
    cerr << "Method Name:\t" << _method << endl;
//    cerr << "Stream ID:\t" << hexify((const unsigned char *)&_streamid, 8, false) << endl;
    cerr << "Stream ID:\t" << _streamid << endl;

    vector<amf::Element *>::iterator ait;
    cerr << "# of Elements in file: " << _amfobjs.size() << endl;
    for (ait = _amfobjs.begin(); ait != _amfobjs.end(); ait++) {
	amf::Element *el = (*(ait));
        el->dump();
    }
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
