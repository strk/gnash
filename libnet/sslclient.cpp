// ssl.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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

#include "GnashSystemIOHeaders.h" // read()
#include "sslclient.h"
#include "amf.h"
#include "element.h"
#include "cque.h"
#include "log.h"
#include "network.h"
#include "utility.h"
#include "buffer.h"
#include "diskstream.h"
#include "cache.h"

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

// This is static in this file, instead of being a private variable in
// the SSLCLient class, is so it's accessible from the C function callback,
// which can't access the private data of the class.
// static SSLClient::passwd_t password(SSL_PASSWD_SIZE);
static SSLClient::passwd_t password;

namespace gnash
{

SSLClient::SSLClient()
    : _hostname("localhost"),
      _calist(CA_LIST),
      _keyfile(KEYFILE),
      _rootpath(ROOTPATH),
      _need_server_auth(true)
{
    GNASH_REPORT_FUNCTION;

    setPort(SSL_PORT);
}

SSLClient::~SSLClient()
{
//     GNASH_REPORT_FUNCTION;
}

// Read bytes from the already opened SSL connection
size_t
SSLClient::sslRead(amf::Buffer &buf)
{
//     GNASH_REPORT_FUNCTION;

    return sslRead(buf.reference(), buf.allocated());
}

size_t
SSLClient::sslRead(boost::uint8_t *buf, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
//    return SSL_Read(_ssl.get(), buf, size);
}

// Write bytes to the already opened SSL connection
size_t
SSLClient::sslWrite(amf::Buffer &buf)
{
//     GNASH_REPORT_FUNCTION;

    return sslWrite(buf.reference(), buf.allocated());
}

size_t
SSLClient::sslWrite(boost::uint8_t *buf, size_t length)
{
    GNASH_REPORT_FUNCTION;

    size_t ret = SSL_write(_ssl.get(), buf, length);

    return ret;
}

// Setup the Context for this connection
bool
SSLClient::sslSetupCTX()
{
    return sslSetupCTX(_keyfile, _calist);
}

bool
SSLClient::sslSetupCTX(std::string &keyspec, std::string &caspec)
{
    GNASH_REPORT_FUNCTION;
    SSL_METHOD *meth;
    int ret;
    string keyfile;
    string cafile;

    if (keyspec.find('/', 0) != string::npos) {
	keyfile = keyspec;
    } else {
	keyfile = _rootpath;
	keyfile += "/";
	keyfile += keyspec;
    }
    
    
    if (caspec.find('/', 0) != string::npos) {
	cafile = caspec;
    } else {
	cafile = _rootpath;
	cafile += "/";
	cafile += caspec;
    }
    
    SSL_library_init();
    SSL_load_error_strings();
    
    if (!_bio_error) {
	_bio_error.reset(BIO_new_fp(stderr, BIO_NOCLOSE));
    }

    // create the context
    meth = SSLv23_method();
    _ctx.reset(SSL_CTX_new(meth));
    
    // Load our keys and certificates
    if ((ret = SSL_CTX_use_certificate_chain_file(_ctx.get(), keyfile.c_str())) != 1) {
	log_error("Can't read certificate file \"%s\"!", keyfile);
	return false;
    } else {
	log_error("Read certificate file \"%s\".", keyfile);
    }

    // Set the passwor dcallback
    SSL_CTX_set_default_passwd_cb(_ctx.get(), password_cb);

    // Add the first private key in the keyfile to the context.
    ERR_clear_error();
    if((ret = SSL_CTX_use_PrivateKey_file(_ctx.get(), keyfile.c_str(),
					  SSL_FILETYPE_PEM)) != 1) {
	log_error("Can't read key file \"%s\"!", keyfile);
 	log_error("Error was: \"%s\"!", ERR_reason_error_string(ERR_get_error()));
//	return false;
    } else {
	log_error("Read key file \"%s\".", keyfile);
    }

    // Load the CAs we trust
    ERR_clear_error();
    if (!(SSL_CTX_load_verify_locations(_ctx.get(), cafile.c_str(), 0))) {
	log_error("Can't read CA list from \"%s\"!", cafile);
 	log_error("Error was: \"%s\"!", ERR_reason_error_string(ERR_get_error()));
	return false;
    } else {
	log_error("Read CA list from \"%s\"!", cafile);
    }
    
#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
    SSL_CTX_set_verify_depth(_ctx.get() ,1);
#endif

    return true;
}

// Shutdown the Context for this connection
bool
SSLClient::sslShutdown()
{
    GNASH_REPORT_FUNCTION;

    SSL_CTX_free(_ctx.get());

    return closeNet();
}

// sslConnect() is how the client connects to the server 
bool
SSLClient::sslConnect()
{
    return sslConnect(_hostname);
}

bool
SSLClient::sslConnect(std::string &hostname)
{
    GNASH_REPORT_FUNCTION;

    if (!_ctx) {
	if (!sslSetupCTX()) {
	    return false;
	}
    }

    _ssl.reset(SSL_new(_ctx.get()));

    if (createClient(hostname, getPort()) == false) {
        log_error("Can't connect to SSL server %s", hostname);
        return false;
    }

    _bio.reset(BIO_new_socket(getFileFd(), BIO_NOCLOSE));
    if (SSL_connect(_ssl.get()) <= 0) {
        log_error("Can't connect to SSL server %s", hostname);
        return false;
    }

    if (_need_server_auth) {
	checkCert(hostname);
    }

    return true;
}

// sslAccept() is how the server waits for connections for clients
size_t
SSLClient::sslAccept()
{
    GNASH_REPORT_FUNCTION;

    return 0;
}

bool
SSLClient::checkCert(std::string &hostname)
{
    GNASH_REPORT_FUNCTION;

    if (!_ssl) {
	return false;
    }

    X509 *peer;
    char peer_CN[256];
    
    if (SSL_get_verify_result(_ssl.get()) !=X509_V_OK) {
	log_error("Certificate doesn't verify");
    }

    // Check the cert chain. The chain length
    // is automatically checked by OpenSSL when
    // we set the verify depth in the ctx

    // Check the common name
    peer = SSL_get_peer_certificate(_ssl.get());
    X509_NAME_get_text_by_NID (X509_get_subject_name(peer),
			       NID_commonName, peer_CN, 256);

    if (strcasecmp(peer_CN, hostname.c_str())) {
	log_error("Common name doesn't match host name");
    }

    return true;
}

void
SSLClient::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
        
    log_debug (_("==== The SSL header breaks down as follows: ===="));
}

// void
// SSLClient::setPassword(boost::array<boost::uint8_t> pw) {
//     password = pw;
// }

SSLClient::passwd_t &
SSLClient::getPassword() {
    return password;
}    

extern "C" {

// This is the callback required when setting up the password. 
int
password_cb(char *buf, int size, int rwflag, void *userdata)
{
//    GNASH_REPORT_FUNCTION;
    log_debug("Callback executed to set the SSL password, size", size);
    
    if(size < static_cast<int>(password.size())) {
	log_error("The buffer for the password needs to be %d bytes larger",
		  password.size() - size);
	return(0);
    }

    // copy the password, we'll need it later
    std::copy(buf, buf + size, password.data());    
    
    return(size);
    
}
} // end of extern C

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
