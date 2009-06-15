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

#include "as_object.h"
#include "dejagnu.h"
#include "log.h"
#include "openssl/ssl.h"
#include "sslclient.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "sol.h"
#include "arg_parser.h"
#include "sslclient.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

static TestState runtest;

static string infile;

static void test_client();
static SSLClient client;

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'n', "name",          Arg_parser::yes },
            { 'o', "port",          Arg_parser::yes },
            { 'c', "cert",          Arg_parser::yes },
            { 'p', "pem",           Arg_parser::yes },
            { 'k', "leyfile",       Arg_parser::yes },
            { 'w', "password",      Arg_parser::yes },
            { 'a', "calist",        Arg_parser::yes },
            { 'r', "rootpath",      Arg_parser::yes },
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
              case 'n':
                  client.setHostname(parser.argument(i));
                  log_debug(_("Hostname for SSL connection is: %s"),
                            client.getHostname());
                  break;
              case 'o':
                  client.setPort(parser.argument<short>(i));
                  log_debug(_("Port for SSL connections is: %hd"),
                            client.getPort());
                  break; 
              case 'c':
                  client.setCert(parser.argument(i));
                  log_debug(_("Cert file for SSL connection is: %s"),
                            client.getCert());
                  break;
              case 'p':
                  client.setPem(parser.argument(i));
                  log_debug(_("Pem file for SSL connection is: %s"),
                            client.getPem());
                  break;
              case 'k':
                  client.setKeyfile(parser.argument(i));
                  log_debug(_("Keyfile file for SSL connection is: %s"),
                            client.getKeyfile());
                  break;
              case 'a':
                  client.setCAlist(parser.argument(i));
                  log_debug(_("CA List file for SSL connection is: %s"),
                            client.getCAlist());
                  break;
              case 'r':
                  client.setRootPath(parser.argument(i));
                  log_debug(_("Root path for SSL pem files is: %s"),
                            client.getRootPath());
                  break;
              case 0:
                  infile = parser.argument(i);
                  log_debug(_("Input file for testing the SSL connection is: %s"), infile);
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
    
    if (client.sslConnect()) {
        runtest.pass("Connected to SSL server");
    } else {
        runtest.fail("Couldn't connect to SSL server");
        giveup = true;
    }

    // I haven't seen a password with the first character set to
    // zero ever. so we assume it got set correctly by the callback.
    if (client.getPassword()[0] != 0) {
        runtest.pass("Password was set for SSL connection");
    } else {
        if (giveup) {
            runtest.unresolved("Password wasn't set for SSL connection");
        } else {
            runtest.fail("Password wasn't set for SSL connection");
        }
    }

    
    if (giveup) {
        runtest.unresolved("Couldn't write to SSL connection");
    } else {
        amf::Buffer buf("Hello World");
        if ((ret = client.sslWrite(buf)) == buf.allocated()) {
            runtest.pass("Wrote bytes to SSL connection");
        } else {
            runtest.fail("Couldn't write to SSL connection");
        }
    }
    
    
    if (giveup) {
        runtest.unresolved("Couldn't shutdown SSL connection");
    } else {
        if (client.sslShutdown()) {
            runtest.pass("Shutdown SSL connection");
        } else {
            runtest.fail("Couldn't shutdown SSL connection");
        }
    }
    
}

static void
usage (void)
{
    cerr << "This program tests SSL support in the libnet library." << endl;
    cerr << "Usage: test_ssl [hv]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    cerr << "-n\tname of host" << endl;
    cerr << "-o\tPort" << endl;
    cerr << "-c\tCert File" << endl;
    cerr << "-p\tPem file" << endl;
    cerr << "-k\tKeyfile file" << endl;
    cerr << "-w\tPassword" << endl;
    cerr << "-a\tCA List" << endl;
    cerr << "-r\tRoot path" << endl;
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
