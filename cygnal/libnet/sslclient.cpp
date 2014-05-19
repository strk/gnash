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
#include <openssl/rand.h>
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
// The debug log used by all the gnash libraries.

static LogFile& dbglogfile = LogFile::getDefaultInstance();
RcInitFile& rc = RcInitFile::getDefaultInstance();

// This is static in this file, instead of being a private variable in
// the SSLCLient class, is so it's accessible from the C function callback,
// which can't access the private data of the class.
// static SSLClient::passwd_t password(SSL_PASSWD_SIZE);
static string password;

namespace gnash
{

const size_t SSL_PASSWD_SIZE = 1024;
static const char  *SSL_HOST    = "localhost";
// static const char  *SSL_CA_LIST = "rootcert.pem";
// static const char  *SSL_CLIENT_CERTFILE  = "client.pem";
// static const char  *SSL_CLIENT_ROOTPATH = "/etc/pki/tls";

// const char *RANDOM  = "random.pem";

SSLClient::SSLClient()
    : _hostname("localhost"),
      _calist(rc.getRootCert()),
      _keyfile(rc.getCertFile()),
      _rootpath(rc.getCertDir()),
      _need_server_auth(true)
{
    GNASH_REPORT_FUNCTION;

//     setPort(SSL_PORT);
    setPassword("foobar");

    // Initialize SSL library
    SSL_library_init();
    RAND_load_file("/dev/urandom", 1024);

    // Load the error strings so the SSL_error_*() functions work
    SSL_load_error_strings();
}

SSLClient::~SSLClient()
{
//     GNASH_REPORT_FUNCTION;
}

// Read bytes from the already opened SSL connection
int
SSLClient::sslRead(cygnal::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    return sslRead(buf.reference(), buf.allocated());
}

int
SSLClient::sslRead(boost::uint8_t *buf, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
    ERR_clear_error();
    int ret = SSL_read(_ssl.get(), buf, size);
    if (ret < 0) {
	log_error(_("Error was: \"%s\"!"),
		  ERR_reason_error_string(ERR_get_error()));
    }
    
    return ret;
}

// Write bytes to the already opened SSL connection
int
SSLClient::sslWrite(cygnal::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    return sslWrite(buf.reference(), buf.allocated());
}

int
SSLClient::sslWrite(const boost::uint8_t *buf, size_t length)
{
    GNASH_REPORT_FUNCTION;
    
    ERR_clear_error();
    int ret = SSL_write(_ssl.get(), buf, length);
    if (ret < 0) {
	log_error(_("Error was: \"%s\"!"),
		  ERR_reason_error_string(ERR_get_error()));
    }
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
//     SSL_METHOD *meth;
    int ret;
    string keyfile;
    string cafile;

#if 1
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
#else
    keyfile = keyspec;
    cafile = caspec;
#endif

    // create the context
    _ctx.reset(SSL_CTX_new( SSLv23_method()));
    
    ERR_clear_error();
    if (!(SSL_CTX_load_verify_locations(_ctx.get(), cafile.c_str(),
					_rootpath.c_str()))) {
	log_error(_("Can't read CA list from \"%s\"!"), cafile);
 	log_error(_("Error was: \"%s\"!"),
		  ERR_reason_error_string(ERR_get_error()));
	return false;
    } else {
	log_debug(_("Read CA list from \"%s\""), cafile);
    }
    
    // Load our keys and certificates
    ERR_clear_error();
    if ((ret = SSL_CTX_use_certificate_chain_file(_ctx.get(), keyfile.c_str())) != 1) {
	log_error(_("Can't read certificate file \"%s\"!"), keyfile);
	return false;
    } else {
	log_debug(_("Read certificate file \"%s\"."), keyfile);
    }

    // Set the password as a callback, otherwise we get prompted for it
    SSL_CTX_set_default_passwd_cb(_ctx.get(), password_cb);

    // Add the first private key in the keyfile to the context.
    ERR_clear_error();
    if((ret = SSL_CTX_use_PrivateKey_file(_ctx.get(), keyfile.c_str(),
					  SSL_FILETYPE_PEM)) != 1) {
	log_error(_("Can't read CERT file \"%s\"!"), keyfile);
	log_error(_("Error was: \"%s\"!"),
		  ERR_reason_error_string(ERR_get_error()));
	return false;
    } else {
	log_debug(_("Read key file \"%s\"."), keyfile);
    }

     SSL_CTX_set_verify(_ctx.get(), SSL_VERIFY_PEER, verify_callback);

#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
    SSL_CTX_set_verify_depth(_ctx.get(), 4);
#endif

    return true;
}

// Shutdown the Context for this connection
bool
SSLClient::sslShutdown()
{
//     GNASH_REPORT_FUNCTION;

//     SSL_clear(_ssl.get());
    SSL_free(_ssl.get());
    SSL_CTX_free(_ctx.get());

    return true;
}

// sslConnect() is how the client connects to the server 
bool
SSLClient::sslConnect(int fd)
{
    return sslConnect(fd, _hostname, 0);
}

bool
SSLClient::sslConnect(int fd, std::string &hostname, short port)
{
    GNASH_REPORT_FUNCTION;
    int ret;

    if (!_ctx) {
	if (!sslSetupCTX()) {
	    return false;
	}
    }

    _ssl.reset(SSL_new(_ctx.get()));
	
//     // Make a tcp/ip connect to the server
//     if (createClient(hostname, getPort()) == false) {
//         log_error("Can't connect to server %s", hostname);
//         return false;
//     }

    // Handshake the server
    ERR_clear_error();
#if 0
    _bio.reset(BIO_new_socket(fd, BIO_NOCLOSE));
#else
//     BIO_set_conn_hostname(_bio.get(), _hostname.c_str());
    _bio.reset(BIO_new_connect(const_cast<char *>(_hostname.c_str())));

    BIO_set_conn_int_port(_bio.get(), &port);
    log_debug(_("PORT is: %d"), BIO_get_conn_port(_bio.get()));

    if (BIO_do_connect(_bio.get()) <= 0) {
        log_error(_("Error connecting to remote machine: %s"),
		  ERR_reason_error_string(ERR_get_error()));
    }
#endif

    SSL_set_bio(_ssl.get(), _bio.get(), _bio.get());
    SSL_set_connect_state(_ssl.get());
    
    if ((ret = SSL_connect(_ssl.get())) < 0) {
        log_error(_("Can't connect to SSL server %s"), hostname);
 	log_error(_("Error was: \"%s\"!"),
		  ERR_reason_error_string(ERR_get_error()));
        return false;
    } else {
        log_debug(_("Connected to SSL server %s"), hostname);
    }

    ERR_clear_error();
#if 0
    if (_need_server_auth) {
 	checkCert(hostname);
    }
#endif
    
    return true;
}

bool
SSLClient::checkCert()
{
    GNASH_REPORT_FUNCTION;
    return checkCert(_hostname);
}

bool
SSLClient::checkCert(std::string &hostname)
{
    GNASH_REPORT_FUNCTION;

    if (!_ssl || (hostname.empty())) {
	return false;
    }

    X509 *peer;
    char peer_CN[256];
    
    if (SSL_get_verify_result(_ssl.get()) != X509_V_OK) {
	log_error(_("Certificate doesn't verify"));
	return false;
    } else {
	log_debug(_("Certificate verified."));
    }

    // Check the cert chain. The chain length
    // is automatically checked by OpenSSL when
    // we set the verify depth in the ctx

    // Check the common name
    if ((peer = SSL_get_peer_certificate(_ssl.get())) == 0) {
	log_debug(_("Couldn't get Peer certificate!"));
	return false;
    } else {
	log_debug(_("Got Peer certificate."));
    }
    
    ERR_clear_error();
    X509_NAME_get_text_by_NID (X509_get_subject_name(peer),
			       NID_commonName, peer_CN, 256);

    if (strcasecmp(peer_CN, hostname.c_str())) {
	log_error(_("Common name doesn't match host name"));
    }

    return true;
}

void
SSLClient::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
        
    log_debug(_("==== The SSL header breaks down as follows: ===="));
}

// The password is a global variable so it can be set from a C function
// callback.
void
SSLClient::setPassword(std::string pw) {
    password = pw;
}

std::string &
SSLClient::getPassword() {
    return password;
}    

extern "C" {

// This is the callback required when setting up the password. 
int
password_cb(char *buf, int size, int /* rwflag */, void * /* userdata */)
{
    GNASH_REPORT_FUNCTION;

    log_debug(_("Callback executed to set the SSL password, size is: %d"),
	      password.size());
    
    if(size <= static_cast<int>(password.size()+1)) {
	log_error(_("The buffer for the password needs to be %d bytes larger"),
		  password.size() - size);
	return(0);
    }

    // copy the password, we'll need it later
//     std::copy(buf, buf + size, password.data());
    std::copy(password.begin(), password.end(), buf);
    
    
    return(password.size());
    
}

int
verify_callback(int ok, X509_STORE_CTX *store)
{
    GNASH_REPORT_FUNCTION;

    char data[256];
 
    if (!ok) {
        X509 *cert = X509_STORE_CTX_get_current_cert(store);
        int  depth = X509_STORE_CTX_get_error_depth(store);
        int  err = X509_STORE_CTX_get_error(store);
	
        log_error(_("-Error with certificate at depth: %i\n"), depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
        log_error(_("  issuer   = %s\n"), data);
        X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
        log_error(_("  subject  = %s\n"), data);
        log_error(_("err %i:%s\n"), err, X509_verify_cert_error_string(err));
    }
 
    return ok;
}

} // end of extern C

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
