// ssh.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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
#include "sshclient.h"
#include "amf.h"
#include "element.h"
#include "cque.h"
#include "log.h"
#include "network.h"
#include "utility.h"
#include "buffer.h"
#include "diskstream.h"
#include "cache.h"

#ifdef HAVE_OPENSSH_SSH_H
#include <openssh/ssh.h>
#include <openssh/err.h>
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
// the SSHCLient class, is so it's accessible from the C function callback,
// which can't access the private data of the class.
// static SSHClient::passwd_t password(SSH_PASSWD_SIZE);
static string password;

namespace gnash
{

const char *ROOTPATH = "/etc/pki/tls";
const char *HOST    = "localhost";
const char *CA_LIST = "root.pem";
const char *RANDOM  = "random.pem";
const char *KEYFILE  = "client.pem";
const size_t SSH_PASSWD_SIZE = 1024;

SSHClient::SSHClient()
    : _hostname("localhost"),
      _calist(CA_LIST),
      _keyfile(KEYFILE),
      _rootpath(ROOTPATH),
      _need_server_auth(true)
{
    GNASH_REPORT_FUNCTION;

//     setPort(SSH_PORT);
    setPassword("password");
}

SSHClient::~SSHClient()
{
//     GNASH_REPORT_FUNCTION;
}

// Read bytes from the already opened SSH connection
int
SSHClient::sshRead(amf::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    return sshRead(buf.reference(), buf.allocated());
}

int
SSHClient::sshRead(boost::uint8_t *buf, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
//     ERR_clear_error();
//     int ret = SSH_read(_ssh.get(), buf, size);
//     if (ret < 0) {
// 	log_error("Error was: \"%s\"!", ERR_reason_error_string(ERR_get_error()));
//     }
    
//    return ret;
}

// Write bytes to the already opened SSH connection
int
SSHClient::sshWrite(amf::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    return sshWrite(buf.reference(), buf.allocated());
}

int
SSHClient::sshWrite(const boost::uint8_t *buf, size_t length)
{
    GNASH_REPORT_FUNCTION;
    
//     ERR_clear_error();
//     int ret = SSH_write(_ssh.get(), buf, length);
//     if (ret < 0) {
// 	log_error("Error was: \"%s\"!", ERR_reason_error_string(ERR_get_error()));
//     }
//    return ret;
}

// Setup the Context for this connection
bool
SSHClient::sshSetupCTX()
{
//     return sshSetupCTX(_keyfile, _calist);
}

bool
SSHClient::sshSetupCTX(std::string &keyspec, std::string &caspec)
{
    GNASH_REPORT_FUNCTION;
//     SSH_METHOD *meth;
//     int ret;
//     string keyfile;
//     string cafile;

//     if (keyspec.find('/', 0) != string::npos) {
// 	keyfile = keyspec;
//     } else {
// 	keyfile = _rootpath;
// 	keyfile += "/";
// 	keyfile += keyspec;
//     }
    
    
//     if (caspec.find('/', 0) != string::npos) {
// 	cafile = caspec;
//     } else {
// 	cafile = _rootpath;
// 	cafile += "/";
// 	cafile += caspec;
//     }

//     // Initialize SSH library
//     SSH_library_init();

//     // Load the error strings so the SSH_error_*() functions work
//     SSH_load_error_strings();
    
//     // create the context
//     meth = SSHv23_method();
//     _ctx.reset(SSH_CTX_new(meth));
    
//     // Load our keys and certificates
//     if ((ret = SSH_CTX_use_certificate_chain_file(_ctx.get(), keyfile.c_str())) != 1) {
// 	log_error("Can't read certificate file \"%s\"!", keyfile);
// 	return false;
//     } else {
// 	log_debug("Read certificate file \"%s\".", keyfile);
//     }

//     // Set the passwor dcallback
//     SSH_CTX_set_default_passwd_cb(_ctx.get(), password_cb);

//     // Add the first private key in the keyfile to the context.
//     ERR_clear_error();
//     if((ret = SSH_CTX_use_PrivateKey_file(_ctx.get(), keyfile.c_str(),
// 					  SSH_FILETYPE_PEM)) != 1) {
// 	log_error("Can't read key file \"%s\"!", keyfile);
//  	log_error("Error was: \"%s\"!", ERR_reason_error_string(ERR_get_error()));
// 	return false;
//     } else {
// 	log_error("Read key file \"%s\".", keyfile);
//     }

//     // Load the CAs we trust
//     ERR_clear_error();
//     if (!(SSH_CTX_load_verify_locations(_ctx.get(), cafile.c_str(), 0))) {
// 	log_error("Can't read CA list from \"%s\"!", cafile);
//  	log_error("Error was: \"%s\"!", ERR_reason_error_string(ERR_get_error()));
// 	return false;
//     } else {
// 	log_debug("Read CA list from \"%s\"", cafile);
//     }
    
// #if (OPENSSH_VERSION_NUMBER < 0x00905100L)
//     SSH_CTX_set_verify_depth(_ctx.get() ,1);
// #endif

    return true;
}

// Shutdown the Context for this connection
bool
SSHClient::sshShutdown()
{
    GNASH_REPORT_FUNCTION;

//     SSH_CTX_free(_ctx.get());

//     return closeNet();
    return true;
}

// sshConnect() is how the client connects to the server 
bool
SSHClient::sshConnect(int fd)
{
//     return sshConnect(fd, _hostname);
}

bool
SSHClient::sshConnect(int fd, std::string &hostname)
{
    GNASH_REPORT_FUNCTION;
    int ret;

//     if (!_ctx) {
// 	if (!sshSetupCTX()) {
// 	    return false;
// 	}
//     }

//     _ssh.reset(SSH_new(_ctx.get()));

// //     // Make a tcp/ip connect to the server
// //     if (createClient(hostname, getPort()) == false) {
// //         log_error("Can't connect to server %s", hostname);
// //         return false;
// //     }

//     // Handshake the server
//     ERR_clear_error();
//     _bio.reset(BIO_new_socket(fd, BIO_NOCLOSE));
//     SSH_set_bio(_ssh.get(), _bio.get(), _bio.get());

//     if ((ret = SSH_connect(_ssh.get())) < 0) {
//         log_error("Can't connect to SSH server %s", hostname);
//  	log_error("Error was: \"%s\"!", ERR_reason_error_string(ERR_get_error()));
//         return false;
//     } else {
//         log_debug("Connected to SSH server %s", hostname);
//     }

//     ERR_clear_error();
// #if 0
//     if (_need_server_auth) {
//  	checkCert(hostname);
//     }
// #endif
    
    return true;
}

// sshAccept() is how the server waits for connections for clients
size_t
SSHClient::sshAccept()
{
    GNASH_REPORT_FUNCTION;

    return 0;
}

bool
SSHClient::checkCert()
{
    GNASH_REPORT_FUNCTION;
    return checkCert(_hostname);
}

bool
SSHClient::checkCert(std::string &hostname)
{
    GNASH_REPORT_FUNCTION;

//     if (!_ssh || (hostname.empty())) {
// 	return false;
//     }

//     X509 *peer;
//     char peer_CN[256];
    
//     if (SSH_get_verify_result(_ssh.get()) != X509_V_OK) {
// 	log_error("Certificate doesn't verify");
// 	return false;
//     } else {
// 	log_debug("Certificate verified.");
//     }

//     // Check the cert chain. The chain length
//     // is automatically checked by OpenSSH when
//     // we set the verify depth in the ctx

//     // Check the common name
//     if ((peer = SSH_get_peer_certificate(_ssh.get())) == 0) {
// 	log_debug("Couldn't get Peer certificate!");
// 	return false;
//     } else {
// 	log_debug("Got Peer certificate.");
//     }
    
//     ERR_clear_error();
//     X509_NAME_get_text_by_NID (X509_get_subject_name(peer),
// 			       NID_commonName, peer_CN, 256);

//     if (strcasecmp(peer_CN, hostname.c_str())) {
// 	log_error("Common name doesn't match host name");
//     }

    return true;
}

void
SSHClient::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
        
    log_debug (_("==== The SSH header breaks down as follows: ===="));
}

// The password is a global variable so it can be set from a C function
// callback.
void
SSHClient::setPassword(std::string pw) {
    password = pw;
}

std::string &
SSHClient::getPassword() {
    return password;
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
