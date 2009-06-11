// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/cstdint.hpp>
#include <sstream>

#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "cque.h"
#include "network.h"
#include "buffer.h"

namespace gnash
{

const char *CA_LIST = "root.pem";
const char *HOST    = "localhost";
const char *RANDOM  = "random.pem";

class DSOEXPORT SSLClient : public gnash::Network
{
public:
    SSLClient();
    ~SSLClient();

    // Read bytes from the already opened SSL connection
    size_t sslRead(SSL &ssl, amf::Buffer &buf, size_t length);

    // Write bytes to the already opened SSL connection
    size_t sslWrite(SSL &ssl, amf::Buffer &buf, size_t length);

    // Setup the Context for this connection
    size_t sslSetupCTX(SSL &ssl);

    // Shutdown the Context for this connection
    size_t sslShutdown(SSL &ssl);

    // sslConnect() is how the client connects to the server 
    size_t sslConnect(std::string &hostname);

    // sslAccept() is how the server waits for connections for clients
    size_t sslAccept(SSL &ssl);

    void dump();

 private:
    // Check a certificate
    bool checkCert(std::string &hostname);

    boost::scoped_ptr<SSL> _ssl;
    boost::scoped_ptr<SSL_CTX> _ctx;
    boost::scoped_ptr<BIO> _bio;
    boost::scoped_ptr<BIO> _bio_error;
    std::string _keyfile;
    bool _need_server_auth;
};

extern "C" {
    // This is the callback required when setting up the password
    int password_cb(char *buf, int size, int rwflag, void *userdata);
}


} // end of gnash namespace

// end of _SSL_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
