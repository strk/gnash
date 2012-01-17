// ssl.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/cstdint.hpp>
#include <boost/array.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstdlib> // getenv

#include "GnashSystemIOHeaders.h" // read()
#include "sslserver.h"
#include "amf.h"
#include "element.h"
#include "cque.h"
#include "log.h"
#include "network.h"
#include "utility.h"
#include "buffer.h"
#include "diskstream.h"
#include "cache.h"
#include "sslclient.h"
#include "sslserver.h"

#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif


#if defined(_WIN32) || defined(WIN32)
# define __PRETTY_FUNCTION__ __FUNCDNAME__
# include <winsock2.h>
# include <direct.h>
#else
# include <unistd.h>
# include <sys/param.h>
#endif

using namespace gnash;
using namespace std;

static boost::mutex stl_mutex;

namespace gnash
{

const char *SERVER_KEYFILE  = "server.pem";

static unsigned char dh512_p[]={
        0xA6,0xBB,0x71,0x4A,0xE2,0x37,0x18,0x30,0xD9,0x0C,0x21,0x94,
        0x6C,0x0E,0xC7,0xBB,0x0A,0xA5,0x5B,0x28,0x9D,0x9F,0x85,0x4A,
        0x69,0x7F,0x3E,0x4E,0x28,0x2F,0x43,0x1D,0xE5,0x84,0x94,0x41,
        0xC3,0x09,0xFA,0xC3,0x32,0xDE,0x9A,0xF1,0x92,0x4D,0xAA,0x30,
        0x1E,0x39,0x98,0x0A,0xD3,0x87,0xC1,0xC8,0xE5,0xEC,0x9E,0x45,
        0x37,0x7B,0xB8,0xAB,
        };
static unsigned char dh512_g[]={
        0x02,
        };

const char *SERVER_PASSWORD  = "none";
const char *DHFILE  = "dh1024.pem";

SSLServer::SSLServer()
{
//     GNASH_REPORT_FUNCTION;
}

SSLServer::~SSLServer()
{
//    GNASH_REPORT_FUNCTION;
    
    sslShutdown();
}

bool
SSLServer::loadDhParams(char *file)
{
//    GNASH_REPORT_FUNCTION;
    return loadDhParams(_ctx.get(), file);
}

bool
SSLServer::loadDhParams(SSL_CTX *ctx, char *file)
{
//    GNASH_REPORT_FUNCTION;
    DH *dh = 0;
    
//    ret = get_dh512();
    if ((dh = DH_new()) == NULL) {
	return false;
    } else {
	dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), NULL);
	dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), NULL);
	if ((dh->p == NULL) || (dh->g == NULL)) {
	    free(dh);
	    dh = NULL;
	}
    }

    if (dh && ctx) {
	if (SSL_CTX_set_tmp_dh(ctx, dh) < 0) {
	    log_error(_("ssl!!", "Couldn't set DH parameters: %s "),
		      ERR_reason_error_string(ERR_get_error()));
	    return false;
	}
    }
    return true;
}

// sslAccept() is how the server waits for connections for clients
size_t
SSLServer::sslAccept(int fd)
{
    GNASH_REPORT_FUNCTION;

    setKeyfile(SERVER_KEYFILE);
    if (!_ctx) {
	if (!sslSetupCTX()) {
	    return false;
	}
    }

    loadDhParams(_ctx.get(), const_cast<char *>(DHFILE));

    log_debug(_("Got an incoming SSL connection request"));

    _bio.reset(BIO_new_socket(fd, BIO_NOCLOSE));

    _ssl.reset(SSL_new(_ctx.get()));
    SSL_set_accept_state(_ssl.get());
    SSL_set_bio(_ssl.get(), _bio.get(), _bio.get());

    int ret = 0;
    if((ret = SSL_accept(_ssl.get()) <= 0)) {
 	log_error(_("Error was: \"%s\"!"),
		  ERR_reason_error_string(ERR_get_error()));
    }

    return 0;
}

void
SSLServer::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
  
    log_debug (_("==== The SSL header breaks down as follows: ===="));
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
