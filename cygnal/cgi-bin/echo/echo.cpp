// Red5 server side support for the echo_test via RTMP
// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

#include <string>
#include <log.h>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>

// Gnash headers
#include "amf.h"
#include "arg_parser.h"
#include "buffer.h"
#include "network.h"
#include "element.h"

// cygnal headers
#include "echo.h"
#include "cygnal.h"

using namespace amf;
using namespace gnash;
using namespace std;
using namespace cygnal;

static void usage (void);

LogFile& dbglogfile = LogFile::getDefaultInstance();

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

static EchoTest echo;

extern "C" {
    
    boost::shared_ptr<Cygnal::cygnal_init_t>
    echo_class_init()
    {
	GNASH_REPORT_FUNCTION;
        // the standard API
        
        boost::shared_ptr<Cygnal::cygnal_init_t> init(new Cygnal::cygnal_init_t);
//     init.read_func = read_func;
//     init.write_func = write_func;
        
        return init;
    }

    size_t echo_read_func(boost::uint8_t *data, size_t size)
    {
	GNASH_REPORT_FUNCTION;

        GNASH_REPORT_RETURN;
    }
    size_t echo_write_func(boost::uint8_t *data, size_t size)
    {
	GNASH_REPORT_FUNCTION;

        GNASH_REPORT_RETURN;
    }
    
} // end of extern C

int
main(int argc, char *argv[])
{
    int port = CGIBIN_PORT;
    bool done = false;
    
    dbglogfile.setLogFilename("echo-test.log");
//    dbglogfile.setWriteDisk(true);

    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'd', "dump",          Arg_parser::no  },
            { 'n', "netdebug",      Arg_parser::no  },
            { 'p', "port",          Arg_parser::yes  },
        };
    
    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() ) {
        log_error("%s", parser.error());
        exit(EXIT_FAILURE);
    }

    string infile;
    
    for( int i = 0; i < parser.arguments(); ++i ) {
        const int code = parser.code(i);
        try {
            switch( code ) {
              case 'h':
                  usage ();
                  exit(EXIT_SUCCESS);
              case 'v':
                    dbglogfile.setVerbosity();
                    // This happens once per 'v' flag 
                    log_debug(_("Verbose output turned on"));
                    break;
              case 'n':
                  netdebug = true;
                  break;
              case 'p':
                  port = parser.argument<int>(i);
                  break;
              case 0:
                  infile = parser.argument(i);
                  break;
              default:
                  break;
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            log_error(_("Error parsing command line options: %s"), e.what());
        }
    }

    EchoTest net;
    int netfd;
    
    if (infile.empty()) {
        if (netdebug) {
            net.toggleDebug(true);
        }
        int fd = net.createServer(port);
        // Only wait for a limited time.
        net.setTimeout(10);
        netfd = net.newConnection(false, fd);
    }
    
    // This is the main message processing loop for rtmp. All message received require
    // a response.
    do {
        boost::shared_ptr<amf::Buffer> bufptr(new amf::Buffer);
        if (infile.empty()) {
            net.readNet(netfd, *bufptr);
        } else {
            DiskStream filestream(infile);
            filestream.loadToMem(0);
            int ret = net.writeNet(netfd, filestream.get(), filestream.getPagesize());
            if (ret <= 0) {
                break;
            }
        }
        
        vector<boost::shared_ptr<amf::Element> > request = net.parseEchoRequest(
            bufptr->reference(), bufptr->allocated());
        if (request[3]) {
            boost::shared_ptr<amf::Buffer> result = net.formatEchoResponse(request[1]->to_number(), *request[3]);
            if (net.writeNet(netfd, *result)) {
                log_debug("Sent echo test response response to client.");
            }
        } else {
            log_error("Couldn't send echo test response to client!");
            done = true;
        }
    } while (!done);
}

EchoTest::EchoTest()
{
//    GNASH_REPORT_FUNCTION;
}

EchoTest::~EchoTest()
{
//    GNASH_REPORT_FUNCTION;
}

// Parse an Echo Request message coming from the Red5 echo_test. This
// method should only be used for testing purposes.
vector<boost::shared_ptr<amf::Element > >
EchoTest::parseEchoRequest(boost::uint8_t *ptr, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    AMF amf;
    vector<boost::shared_ptr<amf::Element > > headers;

    // The first element is the name of the test, 'echo'
    boost::shared_ptr<amf::Element> el1 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el1);

    // The second element is the number of the test,
    boost::shared_ptr<amf::Element> el2 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el2);

    // This one has always been a NULL object from my tests
    boost::shared_ptr<amf::Element> el3 = amf.extractAMF(ptr, ptr+size);
    ptr += amf.totalsize();
    headers.push_back(el3);

    // This one has always been an NULL or Undefined object from my tests
    boost::shared_ptr<amf::Element> el4 = amf.extractAMF(ptr, ptr+size);
    if (!el4) {
	log_error("Couldn't reliably extract the echo data!");
    }
    ptr += amf.totalsize();
    headers.push_back(el4);
    
    return headers;
}

// format a response to the 'echo' test used for testing Gnash. This
// is only used for testing by developers. The format appears to be
// a string '_result', followed by the number of the test, and then two
// NULL objects.
boost::shared_ptr<amf::Buffer>
EchoTest::formatEchoResponse(double num, amf::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<amf::Buffer> data = amf::AMF::encodeElement(el);
    return formatEchoResponse(num, data->reference(), data->allocated());
}

boost::shared_ptr<amf::Buffer>
EchoTest::formatEchoResponse(double num, amf::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatEchoResponse(num, data.reference(), data.allocated());
}

boost::shared_ptr<amf::Buffer>
EchoTest::formatEchoResponse(double num, boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    string result = "_result";
    Element echo;
    echo.makeString(result);

    Element index;
    index.makeNumber(num);

    Element null;
    null.makeNull();

    boost::shared_ptr<amf::Buffer> encecho = echo.encode();
    boost::shared_ptr<amf::Buffer> encidx  = index.encode();   
    boost::shared_ptr<amf::Buffer> encnull  = null.encode();   

    boost::shared_ptr<amf::Buffer> buf(new amf::Buffer(encecho->size()
						       + encidx->size()
						       + encnull->size() + size));

    *buf = encecho;
    *buf += encidx;
    *buf += encnull;
    buf->append(data, size);

    return buf;
}

static void
usage (void)
{
    cerr << "This program tests AMF support in the AMF library." << endl
         << endl
         << _("Usage: test_amf [options...]") << endl
         << _("  -h,  --help          Print this help and exit") << endl
         << _("  -v,  --verbose       Output verbose debug info") << endl
	 << _("  -n,  --netdebug      Turn on net debugging messages") << endl
	 << _("  -p,  --netdebug      port for network") << endl
         << endl;
}
