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
#include "gmemory.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

// Enable the display of memory allocation and timing data
static bool memdebug = false;

// We use the Memory profiling class to check the malloc buffers
// in the kernel to make sure the allocations and frees happen
// the way we expect them too. There is no real other way to tell.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
Memory *mem = 0;
#endif

static TestState runtest;

static int port = 0;
static string hostname = "localhost";
static string cert;
static string pem;

static void test_client();

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'n', "name",          Arg_parser::yes  },
            { 'o', "port",          Arg_parser::yes  },
            { 'c', "cert",          Arg_parser::yes  },
            { 'p', "pem",           Arg_parser::yes  },
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
                  hostname = parser.argument(i);
                  log_debug(_("Hostname for SSL connection is: %s"), hostname);
                  break;
              case 'o':
                  port = parser.argument<int>(i);
                  log_debug(_("Port for SSL connections is: %hd"), port);
                  break; 
              case 'c':
                  cert = parser.argument(i);
                  log_debug(_("Cert file for SSL connection is: %s"), cert);
                  break;
              case 'p':
                  pem = parser.argument(i);
                  log_debug(_("Pem file for SSL connection is: %s"), pem);
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

    
// T 193.2.4.161:1935 -> 192.168.1.103:34693 [AP]
//   03 00 00 00 00 00 9e 14    00 00 00 00 02 00 06 5f    ..............._
//   65 72 72 6f 72 00 3f f0    00 00 00 00 00 00 05 03    error.?.........
//   00 05 6c 65 76 65 6c 02    00 05 65 72 72 6f 72 00    ..level...error.
//   04 63 6f 64 65 02 00 1e    4e 65 74 43 6f 6e 6e 65    .code...NetConne
//   63 74 69 6f 6e 2e 43 6f    6e 6e 65 63 74 2e 52 65    ction.Connect.Re
//   6a 65 63 74 65 64 00 0b    64 65 73 63 72 69 70 74    jected..descript
//   69 6f 6e 02 00 41 5b 20    53 65 72 76 65 72 2e 52    ion..A[ Server.R
//   65 6a 65 63 74 20 5d 20    3a 20 56 69 72 74 75 61    eject ] : Virtua
//   6c 20 68 6f 73 74 20 5f    64 65 66 61 c3 75 6c 74    l host _defa.ult
//   56 48 6f 73 74 5f 20 69    73 20 6e 6f 74 20 61 76    VHost_ is not av
//   61 69 6c 61 62 6c 65 2e    00 00 09                   ailable....     
// #

// T 193.2.4.161:1935 -> 192.168.1.103:34693 [AP]
//   03 00 00 00 00 00 12 14    00 00 00 00 02 00 05 63    ...............c
//   6c 6f 73 65 00 00 00 00    00 00 00 00 00 05          lose..........  
// #
