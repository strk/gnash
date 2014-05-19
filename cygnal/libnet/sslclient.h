// 
//   Copyright (C) 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifndef GNASH_LIBNET_SSL_H
#define GNASH_LIBNET_SSL_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <boost/array.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/cstdint.hpp>
#include <sstream>

#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "dsodefs.h"
#include "cque.h"
#include "network.h"
#include "buffer.h"


namespace gnash
{

// extern const char *ROOTPATH;
extern const char *HOST;
// extern const char *CA_LIST;
// extern const char *RANDOM;
// extern const char *KEYFILE;
extern const size_t SSL_PASSWD_SIZE;

class DSOEXPORT SSLClient
{
public:
    SSLClient();
    ~SSLClient();

    // Read bytes from the already opened SSL connection
    int sslRead(cygnal::Buffer &buf);
    int sslRead(boost::uint8_t *buf, size_t length);
    int sslRead(std::string &buf);

    // Write bytes to the already opened SSL connection
    int sslWrite(cygnal::Buffer &buf);
    int sslWrite(const boost::uint8_t *buf, size_t length);
    int sslWrite(std::string &buf);

    // Setup the Context for this connection
    bool sslSetupCTX();
    bool sslSetupCTX(std::string &keyfile, std::string &cafile);
    
    // Shutdown the Context for this connection
    bool sslShutdown();

    // sslConnect() is how the client connects to the server 
    bool sslConnect(int fd);
    bool sslConnect(int fd, std::string &hostname, short port);

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
 protected:
    boost::scoped_ptr<SSL> _ssl;
    boost::scoped_ptr<SSL_CTX> _ctx;
    boost::scoped_ptr<BIO> _bio;
    boost::scoped_ptr<BIO> _bio_error;
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
    int verify_callback(int ok, X509_STORE_CTX *store);
}


} // end of gnash namespace

// end of _SSL_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
