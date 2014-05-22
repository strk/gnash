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

#ifndef GNASH_SSL_SERVER_H
#define GNASH_SSL_SERVER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cstdint>
#include <sstream>

#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "sslclient.h"
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
extern const char *SERVER_KEYFILE;
extern const size_t SSL_PASSWD_SIZE;
extern const char *PASSWORD;
extern const char *DHFILE;

class DSOEXPORT SSLServer : public SSLClient {
 public:
    SSLServer();
    ~SSLServer();
    
    bool loadDhParams(char *file);
    bool loadDhParams(SSL_CTX *ctx, char *file);

    void generateEphRSAKey(SSL_CTX *ctx);
    
    // sslAccept() is how the server waits for connections for clients
    size_t sslAccept(int fd);
    
    // display internal data to the terminal
    void dump();
};
    
} // end of gnash namespace

// end of GNASH_SSL_SERVER_H
#endif 

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
