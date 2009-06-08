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
#include <sstream>

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

#include "cque.h"
#include "network.h"
#include "buffer.h"

namespace gnash
{

#define CA_LIST "root.pem"
#define HOST    "localhost"
#define RANDOM  "random.pem"

const boost::uint16_t PORT = 4433;

class DSOEXPORT SSLClient : public gnash::Network
{
public:
    SSLClient();
    ~SSLClient();

    bool checkCert(SSL *ssl, std::string &hostname);

    void dump();
};

// This is the thread for all incoming SSL connections for the server
extern "C" {
    bool DSOEXPORT ssl_handler(Network::thread_params_t *args);
}


} // end of gnash namespace

// end of _SSL_H_
#endif


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
