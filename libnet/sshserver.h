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

#ifndef GNASH_SSH_SERVER_H
#define GNASH_SSH_SERVER_H

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

extern "C" {
#include <libssh/libssh.h>
#include <libssh/sftp.h>
}

#include "sshclient.h"
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

class DSOEXPORT SSHServer : public SSHClient {
 public:
    typedef enum {NO_AUTHTYPE,  DSS, RSA} authtype_t;
    typedef enum {NO_TRANSPORT, RAW, SFTP} transport_type_t;
    
    SSHServer();
    ~SSHServer();
    
    bool authPassword(std::string &user, std::string &passwd);

    bool commandLoop();

    void dump();
 protected:
    SSH_SESSION *_session;
    SSH_OPTIONS *_options;
};
    
} // end of gnash namespace

// end of GNASH_SSH_SERVER_H
#endif 

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
