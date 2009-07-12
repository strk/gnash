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

#ifndef GNASH_LIBNET_SSH_H
#define GNASH_LIBNET_SSH_H

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

#ifdef HAVE_OPENSSH_SSH_H
#include <openssh/ssh.h>
#include <openssh/err.h>
#endif

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

    // Setup the Context for this connection
    bool sshSetupCTX();
    bool sshSetupCTX(std::string &keyfile, std::string &cafile);
    
    // Shutdown the Context for this connection
    bool sshShutdown();

    // sshConnect() is how the client connects to the server 
    bool sshConnect(int fd);
    bool sshConnect(int fd, std::string &hostname);

    // sshAccept() is how the server waits for connections for clients
    size_t sshAccept();

    void setKeyfile(std::string filespec) { _keyfile = filespec; };
    std::string &getKeyfile() { return _keyfile; };
    
    void setCAlist(std::string filespec) { _calist = filespec; };
    std::string &getCAlist() { return _calist; };
    
    void setPassword(std::string pw);
    std::string &getPassword();
    
    void setCert(std::string filespec) { _cert = filespec; };
    std::string &getCert() { return _cert; };
    
    void setRootPath(std::string filespec) { _rootpath = filespec; };
    std::string &getRootPath() { return _rootpath; };
    
    void setPem(std::string filespec) { _pem = filespec; };
    std::string &getPem() { return _pem; };
    
    void setHostname(std::string name) { _hostname = name; };
    std::string &getHostname() { return _hostname; };
    
    void setServerAuth(bool flag) { _need_server_auth = flag; };
    bool getServerAuth() { return _need_server_auth; };
    
    // Check a certificate
    bool checkCert();
    bool checkCert(std::string &hostname);

    void dump();
 private:
//     boost::scoped_ptr<SSH> _ssh;
//     boost::scoped_ptr<SSH_CTX> _ctx;
//     boost::scoped_ptr<BIO> _bio;
//     boost::scoped_ptr<BIO> _bio_error;
    std::string		_hostname;
    std::string		_calist;
    std::string		_keyfile;
    std::string		_cert;
    std::string		_pem;
    std::string		_rootpath;
    bool		_need_server_auth;
};

extern "C" {
    // This is the callback required when setting up the password
    int password_cb(char *buf, int size, int rwflag, void *userdata);
}


} // end of gnash namespace

// end of GNASH_LIBNET_SSH_H
#endif 

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
