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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_DEJAGNU_H

//#include <netinet/in.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <cstdio>

#include "as_object.h"
#include "dejagnu.h"
#include "http.h"
#include "log.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "sol.h"
#include "arg_parser.h"
#include "sshclient.h"
#include "sslclient.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

static TestState runtest;

static string infile;

static void test_client();
static SSHClient client;
static Network net;

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 's', "hostname",      Arg_parser::yes },
            { 'o', "port",          Arg_parser::yes },
            { 'p', "password",      Arg_parser::yes },
            { 'u', "user",          Arg_parser::yes },
            { 'd', "dss",           Arg_parser::no },
            { 'r', "rsa",           Arg_parser::no },
            { 'f', "sftp",          Arg_parser::no },
            { 'a', "raw",           Arg_parser::no },
            { 'n', "netdebug",      Arg_parser::no },
        };
    
    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() ) {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }
    
    for( int i = 0; i < parser.arguments(); ++i ) {
        const int code = parser.code(i);
        try {
            switch( code ) {
              case 'h':
                  usage ();
                  exit(EXIT_SUCCESS);
              case 'v':
                    dbglogfile.setVerbosity();
                    log_debug(_("Verbose output turned on"));
                    break;
              case 's':
                  client.setHostname(parser.argument(i));
                  log_debug(_("Hostname for SSH connection is: %s"),
                            client.getHostname());
                  break;
              case 'o':
                  net.setPort(parser.argument<short>(i));
                  log_debug(_("Port for SSH connections is: %hd"),
                            net.getPort());
                  break; 
              case 'p':
                  client.setPassword(parser.argument(i));
                  log_debug(_("SSH password is: %s"),
                            client.getPassword());
                  break;
              case 'u':
                  client.setUser(parser.argument(i));
                  log_debug(_("SSH user is: %s"),
                            client.getUser());
                  break;
              case 'd':
                  client.setAuthType(SSHClient::DSS);
                  log_debug(_("SSH Authentication type is: %d"),
                            client.getAuthType());
                  break;
              case 'r':
                  client.setAuthType(SSHClient::RSA);
                  log_debug(_("SSH Authentication type is: %d"),
                            client.getAuthType());
                  break;
              case 'a':
                  client.setTransportType(SSHClient::RAW);
                  log_debug(_("SSH Transport type is: %d"),
                            client.getTransportType());
                  break;
              case 'f':
                  client.setTransportType(SSHClient::SFTP);
                  log_debug(_("SSH Transport type is: %d"),
                            client.getTransportType());
                  break;
              case 'n':
                  net.toggleDebug(true);
                  break;
              case 0:
                  infile = parser.argument(i);
                  log_debug(_("Input file for testing the SSH connection is: %s"), infile);
                  break;
            }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }
    
    test_client();
}

static void test_client()
{
    size_t ret;
    bool giveup = false;    

    // Make a tcp/ip connect to the server
    if (net.createClient(client.getHostname(), SSH_PORT) == false) {
	log_error("Can't connect to server %s", client.getHostname());
    }

    if (client.sshConnect(net.getFileFd())) {
        runtest.pass("Connected to SSH server");
    } else {
        runtest.fail("Couldn't connect to SSH server");
        giveup = true;
    }

    // I haven't seen a password with the first character set to
    // zero ever. so we assume it got set correctly by the callback.
    if (client.getPassword()[0] != 0) {
        runtest.pass("Password was set for SSH connection");
    } else {
        if (giveup) {
            runtest.unresolved("Password wasn't set for SSH connection");
        } else {
            runtest.fail("Password wasn't set for SSH connection");
        }
    }

#if 0
    if (giveup) {
        runtest.unresolved("Cert didn't match hostfor SSH connection");
    } else {
        if (client.checkCert()) {
            runtest.xpass("Cert matched host for SSH connection");
        } else {
            runtest.xfail("Cert didn't match host for SSH connection");
        }
    }

    HTTP http;

    if (giveup) {
        runtest.unresolved("Couldn't write to SSH connection");
    } else {
        amf::Buffer &request = http.formatRequest("/crossdomain.xml", HTTP::HTTP_GET);

        if ((ret = client.sshWrite(request)) == request.allocated()) {
            runtest.pass("Wrote bytes to SSH connection");
        } else {
            runtest.fail("Couldn't write to SSH connection.");
        }
    }
#endif
    
#if 0
    // This blocks forever unless data is received.
    if (giveup) {
        runtest.unresolved("Couldn't read bytes from SSH connection");
    } else {
        amf::Buffer buf;
        if ((ret = client.sshRead(buf)) > 0) {
            runtest.pass("Read bytes from SSH connection");
        } else {
            runtest.fail("Couldn't read bytes to SSH connection.");
        }
    }
#endif

    if (giveup) {
        runtest.unresolved("Couldn't shutdown SSH connection");
    } else {
        if (client.sshShutdown()) {
            runtest.pass("Shutdown SSH connection");
        } else {
            runtest.fail("Couldn't shutdown SSH connection");
        }
    }
    
}

static void
usage (void)
{
    cerr << "This program tests SSH support in the libnet library." << endl;
    cerr << "Usage: test_ssh [hvsocpkwar]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    cerr << "-s\thostname" << endl;
    cerr << "-o\tPort" << endl;
    cerr << "-p\tPassword" << endl;
    cerr << "-u\tUser" << endl;
    cerr << "-r\tUse RSA" << endl;
    cerr << "-d\tUse DSS" << endl;
    cerr << "-f\tUse SFTP for transport" << endl;
    cerr << "-a\tUse RAW for transport" << endl;
    cerr << "-n\tEnable network debug" << endl << endl;
    
    cerr << "Libssh version is: " << ssh_version(0) << endl;
    exit (-1);
}

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
    cerr << "This program needs to have DejaGnu installed!" << endl;
    return 0;  
}

#endif
