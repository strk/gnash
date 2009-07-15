// 
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_SSH_CLIENT_H
#define GNASH_SSH_CLIENT_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <boost/array.hpp>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/cstdint.hpp>
#include <sstream>

extern "C" {
#include <libssh/libssh.h>
#include <libssh/sftp.h>
}

#include "cque.h"
#include "network.h"
#include "buffer.h"

namespace gnash
{

extern const char *ROOTPATH;
extern const char *HOST;
extern const char *CA_LIST;
extern const char *RANDOM;
extern const char *KEYFILE;
extern const size_t SSH_PASSWD_SIZE;

class DSOEXPORT SSHClient
{
public:
    typedef enum {NO_AUTHTYPE,  DSS, RSA} authtype_t;
    typedef enum {NO_TRANSPORT, RAW, SFTP} transport_type_t;
    
    SSHClient();
    ~SSHClient();

    // Read bytes from the already opened SSH connection
    int sshRead(amf::Buffer &buf);
    int sshRead(boost::uint8_t *buf, size_t length);
    int sshRead(std::string &buf);

    // Write bytes to the already opened SSH connection
    int sshWrite(amf::Buffer &buf);
    int sshWrite(const boost::uint8_t *buf, size_t length);
    int sshWrite(std::string &buf);

    // Shutdown the Context for this connection
    bool sshShutdown();

    // sshConnect() is how the client connects to the server 
    bool sshConnect(int fd);
    bool sshConnect(int fd, std::string &hostname);
    
    void setUser();
    void setUser(std::string name) { _user = name; };
    std::string &getUser() { return _user; };
    
    void setPassword(std::string pw) { _password = pw; };
    std::string &getPassword() { return _password; };
    
    void setHostname(std::string name) { _hostname = name; };
    std::string &getHostname() { return _hostname; };
    
    void setServerAuth(bool flag) { _need_server_auth = flag; };
    bool getServerAuth() { return _need_server_auth; };
    
    void setAuthType(authtype_t type) { _authtype = type; };
    authtype_t getAuthType() { return _authtype; };
    
    void setTransportType(transport_type_t type) { _transporttype = type; };
    transport_type_t getTransportType() { return _transporttype; };

    int authKbdint();
    int authKbdint(SSH_SESSION *);

    // Channel operations
    CHANNEL *openChannel();
    CHANNEL *openChannel(SSH_SESSION *session);

    void closeChannel();
    void closeChannel(CHANNEL *channel);

    // Accessors
    CHANNEL *getChannel()     { return _channel; };
    SSH_SESSION *getSession() { return _session; };
    boost::shared_ptr<amf::Buffer> &getBuffer()  { return _buffer; };

    // Dump internal data to the screen for debugging
    void dump();
 protected:
    int readChannel(CHANNEL *channel, amf::Buffer &buf);
    int writeChannel(CHANNEL *channel, amf::Buffer &buf);

    std::string		_hostname;
    std::string		_user;
    std::string		_password;
    bool		_need_server_auth;
    authtype_t		_authtype;
    transport_type_t	_transporttype;
    int			_state;
#if 0
    boost::shared_ptr<SSH_SESSION> _session;
    boost::shared_ptr<SSH_OPTIONS> _options;
#else
    SSH_SESSION *_session;
    SSH_OPTIONS *_options;
    CHANNEL	*_channel;
#endif
    boost::shared_ptr<amf::Buffer> _buffer;
};
    
} // end of gnash namespace

// end of GNASH_SSH_CLIENT_H
#endif 

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
