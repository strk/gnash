// ssh.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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

#include <boost/thread/mutex.hpp>
#include <boost/shared_array.hpp>
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

extern "C" {
# include <libssh/libssh.h>
# include <libssh/sftp.h>
}

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

SSHClient::SSHClient()
    : _hostname("localhost"),
      _need_server_auth(true),
      _state(0),
      _session(0),
      _channel(0)
{
//     GNASH_REPORT_FUNCTION;

    // Set the default user name
    setUser();
}

SSHClient::~SSHClient()
{
//    GNASH_REPORT_FUNCTION;
    
    sshShutdown();
}

void
SSHClient::setUser()
{
//     GNASH_REPORT_FUNCTION;

    string user = std::getenv("USER");
    if (!user.empty()) {
	_user = user;
    }    
}

// Read bytes from the already opened SSH connection
int
SSHClient::sshRead(cygnal::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    return sshRead(buf.reference(), buf.size());
}

int
SSHClient::sshRead(boost::uint8_t *buf, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
    int ret = channel_read(_channel, buf, size, 0);
    if (ret < 0) {
 	log_error(_("SSH read error was: \"%s\"!"), ssh_get_error(_session));
    }
    
    return ret;
}

// Write bytes to the already opened SSH connection
int
SSHClient::sshWrite(cygnal::Buffer &buf)
{
    GNASH_REPORT_FUNCTION;

    return sshWrite(buf.reference(), buf.allocated());
}

int
SSHClient::sshWrite(const boost::uint8_t *buf, size_t size)
{
    GNASH_REPORT_FUNCTION;
    
    int ret = channel_write(_channel, buf, size);
    if (ret < 0) {
 	log_error(_("SSH write error was: \"%s\"!"), ssh_get_error(_session));
    }
    return ret;
}

// Shutdown the Context for this connection
bool
SSHClient::sshShutdown()
{
//     GNASH_REPORT_FUNCTION;

    if (_session) {
	ssh_disconnect(_session);
	ssh_finalize();
    }
    free(_session);
    _session = 0;

    return true;
}

// sshConnect() is how the client connects to the server 
bool
SSHClient::sshConnect(int fd)
{
    return sshConnect(fd, _hostname);
}

bool
SSHClient::sshConnect(int /* fd */, std::string &hostname)
{
//     GNASH_REPORT_FUNCTION;
    char *password;
    char *banner;
    char *hexa;

    // We always need a hostname to connect to
    if (ssh_options_set(_session, SSH_OPTIONS_HOST, hostname.c_str()) < 0) {
	log_error(_("Couldn't set hostname option"));
	return false;
    }

    // We always need a user name for the connection
    if (_user.empty()) {
	if (ssh_options_set(_session, SSH_OPTIONS_USER, _user.c_str()) < 0) {
	    log_error(_("Couldn't set user name option"));
	    return false;
	}
    }
    
    // Start a new session
    _session = ssh_new();
    if(ssh_connect(_session)){
        log_error(_("Connection failed : %s\n"), ssh_get_error(_session));
	sshShutdown();
        return false;
    }

    _state = ssh_is_server_known(_session);

    unsigned char *hash = 0;
    int hlen = ssh_get_pubkey_hash(_session, &hash);
    if (hlen < 0) {
	sshShutdown();
	return false;
    }
    switch(_state){
      case SSH_SERVER_KNOWN_OK:	// ok
	  log_debug(_("SSH Server is currently known: %d"), _state);
	  break; 
      case SSH_SERVER_KNOWN_CHANGED:
	  log_error(_("Host key for server changed : server's one is now: "));
	  ssh_print_hexa(_("Public key hash"), hash, hlen);
	  free(hash);
	  log_error(_("For security reason, connection will be stopped"));
	  sshShutdown();
	  return false;;
      case SSH_SERVER_FOUND_OTHER:
	  log_error(_("The host key for this server was not found but an other type of key exists."));
	  log_error(_("An attacker might change the default server key to confuse your client"
		    " into thinking the key does not exist"
		      "We advise you to rerun the client with -d or -r for more safety."));
	  sshShutdown();
	  return false;;
      case SSH_SERVER_NOT_KNOWN:
	  hexa = ssh_get_hexa(hash, hlen);
	  free(hash);
	  // FIXME: for now, accecpt all new keys, and update the 
	  // $HOME/.ssh/know_hosts file.
#if 0
	  log_error(_("The server is unknown. Do you trust the host key ? (yes,no)"));
	  log_error(_("Public key hash: %s"), hexa);
	  free(hexa);
	  fgets(buf, sizeof(buf), stdin);
	  if(strncasecmp(buf, "yes", 3) != 0){
	      sshShutdown();
	      return false;
	  }
	  log_error(_("This new key will be written on disk for further usage. do you agree? (yes,no) "));
	  fgets(buf, sizeof(buf), stdin);
	  if(strncasecmp(buf, "yes", 3)==0){
	      if(ssh_write_knownhost(_session))
		  log_error(ssh_get_error(_session));
	  }
#else
	  if(ssh_write_knownhost(_session)) {
	      log_error(ssh_get_error(_session));
	  }
#endif  
	  break;
      case SSH_SERVER_ERROR:
	  free(hash);
	  log_error(ssh_get_error(_session));
	  sshShutdown();
	  return false;
    }
    
    free(hash);
    
    ssh_userauth_none(_session, NULL);
    
    int auth = ssh_auth_list(_session);

//    log_debug("auth: 0x%04x", auth);
    log_debug(_("supported auth methods: "));
    if (auth & SSH_AUTH_METHOD_PUBLICKEY) {
	log_debug(_("\tpublickey"));
    }
    if (auth & SSH_AUTH_METHOD_INTERACTIVE) {
	log_debug(_("\tkeyboard-interactive"));
    }

    /* no ? you should :) */
    auth=ssh_userauth_autopubkey(_session, NULL);
    if(auth == SSH_AUTH_ERROR){
        log_debug(_("Authenticating with pubkey: %s"), ssh_get_error(_session));
	ssh_finalize();
        return false;
    }
    banner = ssh_get_issue_banner(_session);
    if(banner){
        log_debug(banner);
        free(banner);
    }
    if(auth != SSH_AUTH_SUCCESS){
        auth = authKbdint(_session);
        if(auth == SSH_AUTH_ERROR){
            log_error(_("authenticating with keyb-interactive: %s"),
		      ssh_get_error(_session));
	    ssh_finalize();
            return false;
        }
    }
    if(auth != SSH_AUTH_SUCCESS){
        password = getpass("Password: ");
        if(ssh_userauth_password(_session, NULL, password) != SSH_AUTH_SUCCESS){
            log_error(_("Authentication failed: %s"), ssh_get_error(_session));
            ssh_disconnect(_session);
                ssh_finalize();
            return false;
        }
        memset(password, 0, strlen(password));
    }
    ssh_log(_session, SSH_LOG_FUNCTIONS, "Authentication success");

#if 0
    if(strstr(argv[0],"sftp")){
        sftp = 1;
        ssh_log(_session, SSH_LOG_FUNCTIONS, "Doing sftp instead");
    }
    if(!sftp){
        if(!cmds[0])
            shell(_session);
        else
            batch_shell(_session);
    }
    else
        do_sftp(_session);
    if(!sftp && !cmds[0])
        do_cleanup(0);
#endif
    
    return true;
}

int
SSHClient::authKbdint()
{
//    GNASH_REPORT_FUNCTION;
    return authKbdint(_session);
}

int
SSHClient::authKbdint(ssh_session session)
{
//    GNASH_REPORT_FUNCTION;
    int err = ssh_userauth_kbdint(session, NULL, NULL);
    char *name,*instruction,*prompt,*ptr;
    char buffer[128];
    int i,n;
    char echo;
    while (err == SSH_AUTH_INFO){
        name = const_cast<char *>(ssh_userauth_kbdint_getname(session));
        instruction = const_cast<char *>(ssh_userauth_kbdint_getinstruction(session));
        n=ssh_userauth_kbdint_getnprompts(session);
        if(strlen(name)>0)
            log_debug(name);
        if(strlen(instruction)>0)
            log_debug(instruction);
        for(i=0; i<n; ++i){
            prompt = const_cast<char *>(ssh_userauth_kbdint_getprompt(session, i, &echo));
            if(echo){
                log_debug(prompt);
                fgets(buffer,sizeof(buffer),stdin);
                buffer[sizeof(buffer)-1]=0;
                if((ptr=strchr(buffer,'\n')))
                    *ptr=0;
                if (ssh_userauth_kbdint_setanswer(session, i, buffer) < 0) {
                  return SSH_AUTH_ERROR;
                }
                memset(buffer,0,strlen(buffer));
            } else {
                ptr=getpass(prompt);
                if (ssh_userauth_kbdint_setanswer(session, i, ptr) < 0) {
                  return SSH_AUTH_ERROR;
                }
            }
        }
        err=ssh_userauth_kbdint(session, NULL, NULL);
    }

    return err;
}

// Channel operations
ssh_channel
SSHClient::openChannel()
{
//    GNASH_REPORT_FUNCTION;
    return openChannel(_session);
}


ssh_channel
SSHClient::openChannel(ssh_session session)
{
//    GNASH_REPORT_FUNCTION;
    if (session) {
	_channel = channel_new(session);
	if (_channel) {
	    if (channel_open_session(_channel) != SSH_OK) {
		log_error(_("Can't open the SSH channel!"));
	    }
	} else {
	    log_error(_("Can't allocate memory for new SSH channel!"));
	}
    }

    return _channel;
}

int 
SSHClient::readChannel(ssh_channel channel, cygnal::Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    int ret = -1;

    if (channel) {
	ret = channel_read(channel, buf.reference(), buf.size(), 0);
    } else {
	log_error(_("Can't read from a non-existent channel!"));
    }

    return ret;
}

int 
SSHClient::writeChannel(ssh_channel channel, cygnal::Buffer &buf)
{
//    GNASH_REPORT_FUNCTION;
    int ret = -1;

    if (channel) {
	ret = channel_write(channel, buf.reference(), buf.size());
    } else {
	log_error(_("Can't write to a non-existent channel!"));
    }

    return ret;
}

void 
SSHClient::closeChannel()
{
//    GNASH_REPORT_FUNCTION;
    return closeChannel(_channel);
}

void 
SSHClient::closeChannel(ssh_channel channel)
{
//    GNASH_REPORT_FUNCTION;

    if (channel) {
	channel_close(channel);
//	free(channel);
	_channel = 0;
    }
}


void
SSHClient::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
  
    log_debug (_("==== The SSH header breaks down as follows: ===="));

    ssh_version(0);
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
