// Red5 server side support for the echo_test via HTTP
// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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


#include <string>
#include <log.h>
#include <iostream>
#include <string>

#include "amf.h"
#include "arg_parser.h"
#include "buffer.h"
#include "network.h"
#include "element.h"

#include "gateway.h"
#include "diskstream.h"

using namespace amf;
using namespace gnash;
using namespace std;
using namespace cygnal;

static void usage (void);

LogFile& dbglogfile = LogFile::getDefaultInstance();

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

static GatewayTest gateway;
	
extern "C" {
    
    // the standard API
    std::shared_ptr<Handler::cygnal_init_t>
    gateway_init_func(std::shared_ptr<gnash::RTMPMsg> &msg)
    {
	GNASH_REPORT_FUNCTION;
        std::shared_ptr<Handler::cygnal_init_t> init(new Handler::cygnal_init_t);
        
        init->version = "Gateway Test 0.1 (Gnash)";
        init->description = "gateway RTMPT test for Cygnal.\n"
            "\tThis supplies the server side functionality equired for\n"
            "\tCygnal to handle the Red5 Gateway test"; 
        return init;
    }

    std::shared_ptr<amf::Buffer> gateway_read_func()
    {
// 	GNASH_REPORT_FUNCTION;
	
//      GNASH_REPORT_RETURN;
    }

    size_t gateway_write_func(boost::uint8_t *data, size_t size)
    {
// 	GNASH_REPORT_FUNCTION;

//      GNASH_REPORT_RETURN;
    }
    
} // end of extern C

int
main(int argc, char *argv[])
{
    int port = CGIBIN_PORT;
    bool done = false;
    bool gdb = false;
    
    dbglogfile.setLogFilename("gateway-test.log");
//    dbglogfile.setWriteDisk(true);

    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'n', "netdebug",      Arg_parser::no  },
            { 'p', "port",          Arg_parser::yes  },
            { 'g', "gdb",           Arg_parser::no  },
            { 'd', "dump",          Arg_parser::no  },
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
              case 'g':
                  gdb = true;
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

    // if set, wait for us to connectGDB for debugging
    while (gdb) {
        cout << "Waiting for GDB " << getpid() << endl;
        sleep(5);
    }
    
    
    GatewayTest net;

    if (netdebug) {
	net.toggleDebug(true);
    }

    int fd = 0;
    int netfd = 0;
    if (infile.empty()) {
        fd = net.createServer(port);
        if (fd <= 0) {
            exit(EXIT_FAILURE);
        }
        // Only wait for a limited time.
        net.setTimeout(10);
//        netfd = net.newConnection(false, fd);
    }
    
    // Wait for data, and when we get it, process it.
    std::shared_ptr<amf::Buffer> content;
    vector<std::shared_ptr<amf::Element> > headers;
    net.setTimeout(10);
    do {
        netfd = net.newConnection(false, fd);
        if (netfd <= 0) {
            done = true;
            break;
        }
        // See if we have any messages waiting
        if (infile.empty()) {
            std::shared_ptr<amf::Buffer> content = net.readNet();
            if (!content) {
                done = true;
                break;
            }
//            content->dump();
            headers = net.parseEchoRequest(*content);
        } else {
            DiskStream filestream;
            filestream.open(infile);
            filestream.loadToMem(0);
            headers = net.parseEchoRequest(filestream.get(), filestream.getPagesize());
            filestream.close();
            done = true;
            break;
        }
        
  	//std::shared_ptr<amf::Element> &el0 = headers[0];
	
        if (!done) {
            if (headers.size() >= 4) {
                if (headers[3]) {
                    amf::Buffer reply;
                    if (headers[1]->getNameSize()) {
                        reply = net.formatEchoResponse(headers[1]->getName(), *headers[3]);
                    } else {
                        reply = net.formatEchoResponse("no name", *headers[3]);
                    }
                    
                    if (infile.empty()) {
                        int ret = net.writeNet(netfd, reply);
                        if (ret <= 0) {
//                        reply.dump();
                            // For now exit after only one packet
                            done = true;
                        }
                    } else {
                        cerr << hexify(reply.reference(), reply.allocated(), true) << endl;
                    }
                }
            }
        }
    } while(done != true);

    net.closeNet();
}

GatewayTest::GatewayTest()
{
//    GNASH_REPORT_FUNCTION;
}

GatewayTest::~GatewayTest()
{
//    GNASH_REPORT_FUNCTION;
}

// Parse an Echo Request message coming from the Red5 echo_test. This
// method should only be used for testing purposes.
vector<std::shared_ptr<amf::Element > >
GatewayTest::parseEchoRequest(boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    
    vector<std::shared_ptr<amf::Element > > headers;
	
    // skip past the header bytes, we don't care about them.
    boost::uint8_t *tmpptr = data + 6;
    
    boost::uint16_t length;
    length = ntohs((*(boost::uint16_t *)tmpptr) & 0xffff);
    tmpptr += sizeof(boost::uint16_t);

    // Get the first name, which is a raw string, and not preceded by
    // a type byte.
    std::shared_ptr<amf::Element > el1(new amf::Element);
    
    // If the length of the name field is corrupted, then we get out of
    // range quick, and corrupt memory. This is a bit of a hack, but
    // reduces memory errors caused by some of the corrupted tes cases.
    boost::uint8_t *endstr = std::find(tmpptr, tmpptr+length, '\0');
    if (endstr != tmpptr+length) {
	log_debug("Caught corrupted string! length was %d, null at %d",
		  length,  endstr-tmpptr);
	length = endstr-tmpptr;
    }
    el1->setName(tmpptr, length);
    tmpptr += length;
    headers.push_back(el1);
    
    // Get the second name, which is a raw string, and not preceded by
    // a type byte.
    length = ntohs((*(boost::uint16_t *)tmpptr) & 0xffff);
    tmpptr += sizeof(boost::uint16_t);
    std::shared_ptr<amf::Element > el2(new amf::Element);

//     std::string name2(reinterpret_cast<const char *>(tmpptr), length);
//     el2->setName(name2.c_str(), name2.size());
    // If the length of the name field is corrupted, then we get out of
    // range quick, and corrupt memory. This is a bit of a hack, but
    // reduces memory errors caused by some of the corrupted tes cases.
    endstr = std::find(tmpptr, tmpptr+length, '\0');
    if (endstr != tmpptr+length) {
	log_debug("Caught corrupted string! length was %d, null at %d",
		  length,  endstr-tmpptr);
	length = endstr-tmpptr;
    }
    el2->setName(tmpptr, length);
    headers.push_back(el2);
    tmpptr += length;

    // Get the last two pieces of data, which are both AMF encoded
    // with a type byte.
    amf::AMF amf;
    std::shared_ptr<amf::Element> el3 = amf.extractAMF(tmpptr, tmpptr + size);
    headers.push_back(el3);
    tmpptr += amf.totalsize();
    
    std::shared_ptr<amf::Element> el4 = amf.extractAMF(tmpptr, tmpptr + size);
    headers.push_back(el4);

     return headers;
}

// format a response to the 'echo' test used for testing Gnash. This
// is only used for testing by developers. The format appears to be
// two strings, followed by a double, followed by the "onResult".
amf::Buffer &
GatewayTest::formatEchoResponse(const std::string &num, amf::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<amf::Buffer> data;

    amf::Element nel;
    if (el.getType() == amf::Element::TYPED_OBJECT_AMF0) {
	nel.makeTypedObject();
	string name = el.getName();
	nel.setName(name);
	if (el.propertySize()) {
	    // FIXME: see about using std::reverse() instead.
	    for (int i=el.propertySize()-1; i>=0; i--) {
// 	    for (int i=0 ; i<el.propertySize(); i++) {
		std::shared_ptr<amf::Element> child = el.getProperty(i);
		nel.addProperty(child);
	    }
	    data = nel.encode();
	} else {
	    data = el.encode();
	}
    } else {
	data = el.encode();
    }

    return formatEchoResponse(num, data->reference(), data->allocated());
}

amf::Buffer &
GatewayTest::formatEchoResponse(const std::string &num, amf::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatEchoResponse(num, data.reference(), data.allocated());
}

amf::Buffer &
GatewayTest::formatEchoResponse(const std::string &num, boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    //boost::uint8_t *tmpptr  = data;
    
    // FIXME: temporary hacks while debugging
    amf::Buffer fixme("00 00 00 00 00 01");
    amf::Buffer fixme2("ff ff ff ff");
    
    _buffer = "HTTP/1.1 200 OK\r\n";
    formatContentType(DiskStream::FILETYPE_AMF);
//    formatContentLength(size);
    // FIXME: this is a hack ! Calculate a real size!
    formatContentLength(size+29);
    
    // Don't pretend to be the Red5 server
    formatServer("Cygnal(0.8.6)");
    
    // All HTTP messages are followed by a blank line.
    terminateHeader();

    // Add the binary blob for the header
    _buffer += fixme;

    // Make the result response, which is the 2nd data item passed in
    // the request, a slash followed by a number like "/2".
    string result = num;
    result += "/onResult";
    std::shared_ptr<amf::Buffer> res = amf::AMF::encodeString(result);
    _buffer.append(res->begin()+1, res->size()-1);

    // Add the null data item
    std::shared_ptr<amf::Buffer> null = amf::AMF::encodeString("null");
    _buffer.append(null->begin()+1, null->size()-1);

    // Add the other binary blob
    _buffer += fixme2;

    amf::Element::amf0_type_e type = static_cast<amf::Element::amf0_type_e>(*data);
    if ((type == amf::Element::UNSUPPORTED_AMF0)
	|| (type == amf::Element::NULL_AMF0)) {
	_buffer += type;
	// Red5 returns a NULL object when it's recieved an undefined one in the echo_test
    } else if (type == amf::Element::UNDEFINED_AMF0) {
	_buffer += amf::Element::NULL_AMF0;
    } else {
	// Add the AMF data we're echoing back
	if (size) {
	    _buffer.append(data, size);
	}
    }
    
    return _buffer;
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
         << endl;
}


