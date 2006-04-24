// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include "log.h"
#include "NetConnection.h"

using namespace std;

namespace gnash {

/// \class NetConnection
/// \brief Opens a local connection through which you can play
/// back video (FLV) files from an HTTP address or from the local file
/// system.

/// First introduced for swf v7

/// protocol:[//host][:port]/appname/[instanceName]

/// For protocol, specify either rtmp or rtmpt. If rtmp is specified,
/// Flash Player will create a persistent Communication Server. If
/// rtmpt is specified, Flash Player will create an HTTP "tunneling"
/// connection to the server. For more information on RTMP and RTMPT,
/// see the description section below.
//
/// You can omit the host parameter if the Flash movie is served from
/// the same host where Flash Communication Server is installed.
/// If the instanceName parameter is omitted, Flash Player connects to
/// the application's default instance (_definst_).
/// By default, RTMP connections use port 1935, and RTMPT connections
/// use port 80.
NetConnection::NetConnection() {
}

NetConnection::~NetConnection() {
}

/// \brief Open a connection to stream FLV files.
/// \param the URL
/// \return nothing
/// \note Older Flash movies can only take a NULL value as
/// the parameter, which therefor only connects to the localhost using
/// RTMP. Newer Flash movies have a parameter to connect which is a
/// URL string like rtmp://foobar.com/videos/bar.flv
void
NetConnection::connect(const char *arg)
{
    log_msg("%s: \n", __PRETTY_FUNCTION__);
    
    string::size_type first_colon;
    string::size_type second_colon;
    string::size_type single_slash;
    string::size_type double_slash;

    if (arg != 0) {
        if (strcmp(arg, "null") == 0) {
            log_warning("No URL specified!\n");
            return;
        }
        _url = arg;
        // protocol:[//host][:port]/appname/[instanceName]
        first_colon = _url.find(':', 0);
        second_colon = _url.find(':', first_colon + 1);
        double_slash = _url.find("//", 0) + 2;
        single_slash = _url.find("/", double_slash);
        _protocol = _url.substr(0, first_colon);
        if (second_colon != string::npos) {
            _host = _url.substr(double_slash, second_colon - double_slash);
            _portstr = _url.substr(second_colon + 1, single_slash - second_colon - 1);
        } else {
            _host = _url.substr(double_slash, single_slash - double_slash);
        }
        _path = _url.substr(single_slash, _url.size());

        if (_portstr.size() == 0) {
            log_msg("Loading FLV file from: %s://%s%s\n",
                    _protocol.c_str(), _host.c_str(), _path.c_str());
        } else {
            log_msg("Loading FLV file from: %s://%s:%s%s\n",
                    _protocol.c_str(), _host.c_str(), _portstr.c_str(), _path.c_str());
        }
    } else {
        log_msg("Connecting to localhost\n");
    }
    
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

/// \brief callback to instantiate a new NetConnection object.
/// \param fn the parameters from the Flash movie
/// \return nothing from the function call.
/// \note The return value is returned through the fn.result member.
void
netconnection_new(const fn_call& fn)
{
        log_msg("%s:unimplemented %d\n", __FUNCTION__, __LINE__);
    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
        
    netconnection_as_object *netconnection_obj = new netconnection_as_object;

    netconnection_obj->set_member("connect", &netconnection_connect);
#ifdef ENABLE_TESTING
    netconnection_obj->set_member("geturl",  &network_geturl);
    netconnection_obj->set_member("getprotocol",  &network_getprotocol);
    netconnection_obj->set_member("gethost", &network_gethost);
    netconnection_obj->set_member("getport", &network_getport);
    netconnection_obj->set_member("getpath", &network_getpath);
#endif
    fn.result->set_as_object(netconnection_obj);
}

void netconnection_connect(const fn_call& fn)
{
    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    
    string filespec;
    netconnection_as_object *ptr = (netconnection_as_object*)fn.this_ptr;
    
    assert(ptr);
    if (fn.nargs != 0) {
        filespec = fn.env->bottom(fn.first_arg_bottom_index).to_string();
        ptr->obj.connect(filespec.c_str());
    } else {
        ptr->obj.connect(0);
    }
    
    log_msg("%s: partially implemented\n", __FUNCTION__);
}

} // end of gnash namespace

