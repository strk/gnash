// Red5 server side support for the oflaDemo_test via RTMP
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
#include "URL.h"

// cygnal headers
#include "crc.h"
#include "oflaDemo.h"
#include "cygnal.h"
#include "handler.h"
#include "rtmp_server.h"

#if defined(WIN32) || defined(_WIN32)
int        lt_dlsetsearchpath   (const char *search_path);
int        lt_dlinit           (void);
void *     lt_dlsym            (lt_dlhandle handle, const char *name);
const char *lt_dlerror         (void);
int        lt_dlclose          (lt_dlhandle handle);
int        lt_dlmakeresident   (lt_dlhandle handle);
lt_dlhandle lt_dlopenext       (const char *filename);
#endif

#if HAVE_DIRENT_H || WIN32==1    // win32 hack
# include <dirent.h>
# define NAMLEN(dirent) std::strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

using namespace amf;
using namespace gnash;
using namespace std;
using namespace cygnal;

static void usage (void);

// The user config for Cygnal is loaded and parsed here:
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();
	
LogFile& dbglogfile = LogFile::getDefaultInstance();

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

static OflaDemoTest oflaDemo;

extern "C" {
    
    // the standard API
    boost::shared_ptr<Handler::cygnal_init_t>
    oflaDemo_init_func(boost::shared_ptr<gnash::RTMPMsg> &msg)
    {
	GNASH_REPORT_FUNCTION;
        
        boost::shared_ptr<Handler::cygnal_init_t> init(new Handler::cygnal_init_t);
        if (msg) {
            oflaDemo.setNetConnection(msg);
        } else {
            log_error("No NetConnection message supplied to oflaDemo!");
        }

        init->version = "OflaDemo 0.1 (Gnash)";
        init->description = "streaming Video test for Cygnal.\n"
            "\tThis supplies the server side functionality required for\n"
            "\tCygnal to handle the Red5 OflaDemo test"; 
        return init;
    }

    size_t oflaDemo_read_func(boost::uint8_t *data, size_t size)
    {
// 	GNASH_REPORT_FUNCTION;
	
	size_t safe = 0;
	boost::shared_ptr<amf::Buffer> buf = oflaDemo.getResponse();
        if (!buf) {
            return -1;
        }
	if (size < buf->allocated()) {
	    safe = buf->allocated();
	} else {
	    safe = size;
	}
	std::copy(buf->begin(), buf->begin() + safe, data);
	
// 	log_network("%s", hexify(data, safe, true));

        if (buf) {
            return buf->allocated();
        } else {
            return 0;
        }
	    
//         GNASH_REPORT_RETURN;
    }

    size_t oflaDemo_write_func(boost::uint8_t *data, size_t size)
    {
// 	GNASH_REPORT_FUNCTION;

	boost::shared_ptr<amf::Buffer> buf = oflaDemo.getResponse();

        vector<boost::shared_ptr<amf::Element> > request =
	    oflaDemo.parseOflaDemoRequest(data, size);
        
        if (request.size() == 0) {
            // Send the packet to notify the client that the
            // NetConnection::connect() was sucessful. After the client
            // receives this, the handhsake is completed.
            boost::shared_ptr<amf::Buffer> error =
                oflaDemo.encodeResult(RTMPMsg::NC_CALL_FAILED);
            // This builds the full header,which is required as the first part
            // of the packet.
            boost::shared_ptr<amf::Buffer> head = oflaDemo.encodeHeader(0x3,
                                          RTMP::HEADER_12, error->allocated(),
                                          RTMP::INVOKE, RTMPMsg::FROM_SERVER);
            boost::scoped_ptr<amf::Buffer> response(new amf::Buffer(
                                   error->allocated() + head->allocated()));
            *response = head;
            *response += error;
            log_error("Couldn't send response to client!");
            
            return - 1;
        }

// 	log_network("%s", hexify(buf->reference(), buf->allocated(), true));

        if (buf) {
            return buf->allocated();
        } else {
            return 0;
        }
//         GNASH_REPORT_RETURN;
    }
    
} // end of extern C

int
main(int argc, char *argv[])
{
    int port = CGIBIN_PORT;
    bool done = false;
    
    dbglogfile.setLogFilename("oflaDemo-test.log");
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

    OflaDemoTest net;
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
        
        vector<boost::shared_ptr<amf::Element> > request = net.parseOflaDemoRequest(
            bufptr->reference(), bufptr->allocated());
        if (request[3]) {
            boost::shared_ptr<amf::Buffer> result = net.formatOflaDemoResponse(request[1]->to_number(), *request[3]);
            if (net.writeNet(netfd, *result)) {
                log_debug("Sent oflaDemo test response response to client.");
            }
        } else {
            log_error("Couldn't send oflaDemo test response to client!");
            done = true;
        }
    } while (!done);
}

demoService::demoService()
{
//    GNASH_REPORT_FUNCTION;
}

demoService::~demoService()
{
//    GNASH_REPORT_FUNCTION;
}


std::vector<std::string> &
demoService::getListOfAvailableFiles(const std::string &path)
{
//    GNASH_REPORT_FUNCTION;
    return getListOfAvailableFiles(path, ".flv");
}

std::vector<std::string> &
demoService::getListOfAvailableFiles(const std::string &path,
				    const std::string &type)
{
    GNASH_REPORT_FUNCTION;

    _path = path;		// store for later

    // If we don't have any files yet, look for some.
    if (_media.size() == 0) {
	    log_debug(_("Scanning directory \"%s\" for %s files"), path, type);
	    DIR *libdir = opendir(path.c_str());
	    
	    if (!libdir) {
		log_error(_("Can't open directory %s"), path);
		return _media;
	    }   
	    
	    struct dirent *entry;
	    
	    while ((entry = readdir(libdir)) != NULL) {
		// We only want media files that end with the suffix.
		std::string name(entry->d_name);
		
		// We don't want to see hidden files either.
		if (name.at(0) == '.') {
		    continue;
		}
		const std::string::size_type pos = name.find_last_of('.');
		if (pos == std::string::npos) {
		    continue;
		}
		
		const std::string suffix = name.substr(pos);
		name.erase(pos);

		// We only wat this type of file.
		if (suffix == type) {
		    log_debug(_("Gnash media file name: %s"), name);
		    _media.push_back(name);
		} else {
		    continue;
		}
	    }
	    
	    if (closedir(libdir) != 0) {
		return _media;
	    }
    }
    
    return _media;
}

OflaDemoTest::OflaDemoTest()
{
//    GNASH_REPORT_FUNCTION;
}

OflaDemoTest::~OflaDemoTest()
{
//    GNASH_REPORT_FUNCTION;
}

// Parse an OflaDemo Request message coming from the Red5 oflaDemo_test. This
// method should only be used for testing purposes.
vector<boost::shared_ptr<amf::Element > >
OflaDemoTest::parseOflaDemoRequest(boost::uint8_t *ptr, size_t size)
{
    GNASH_REPORT_FUNCTION;

    demoService demo;
    AMF amf;
    vector<boost::shared_ptr<amf::Element > > headers;

    boost::shared_ptr<amf::Element> el1 = amf.extractAMF(ptr, ptr+size);
    if (!el1) {
        log_error("No AMF data in message!");
        return headers;
    }
    
    string method = el1->to_string();    
    ptr += amf.totalsize();
    headers.push_back(el1);

    // The second element is a number
    boost::shared_ptr<amf::Element> el2 = amf.extractAMF(ptr, ptr+size);
    if (!el2) {
        log_error("No AMF data in message!");
        return headers;
    }
    ptr += amf.totalsize();
    headers.push_back(el2);

    if (method == "demoService.getListOfAvailableFLVs") {
        // Get the path from the NetConnection object we recieved from the
        // client at the end of the handshake process.
        boost::shared_ptr<amf::Element> version;
        boost::shared_ptr<amf::Element> tcurl;
        boost::shared_ptr<amf::Element> swfurl;
        
        boost::shared_ptr<gnash::RTMPMsg> msg = getNetConnection();
        if (msg) {
            version  = msg->findProperty("flashVer");
            if (version) {
                log_network("Flash player client version is: %s",
                            version->to_string());
            }
            tcurl  = msg->findProperty("tcUrl");
            swfurl  = msg->findProperty("swfUrl");
        } else {
            log_error("No NetConnection message!");
            return headers;
        }
        if (tcurl) {
            string docroot;
            URL url(tcurl->to_string());
            if (crcfile.getDocumentRoot().size() > 0) {
                docroot = crcfile.getDocumentRoot();
                log_debug (_("Document Root for media files is: %s"),
                           docroot);
            } else {
                docroot = "/var/www/html";
            }
            std::string key = docroot + "/";
            key += url.hostname() + url.path();
            demo.getListOfAvailableFiles(key);
        }
    } else {
        log_error("Unknown oflaDemp method \"%s\" to INVOKE!", el1->getName());
    }

    return headers;
}

// format a response to the 'oflaDemo' test used for testing Gnash. This
// is only used for testing by developers. The format appears to be
// a string '_result', followed by the number of the test, and then two
// NULL objects.
boost::shared_ptr<amf::Buffer>
OflaDemoTest::formatOflaDemoResponse(double num, amf::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<amf::Buffer> data = amf::AMF::encodeElement(el);
    if (data) {
	return formatOflaDemoResponse(num, data->reference(), data->allocated());
    } else {
	log_error("Couldn't encode element: %s", el.getName());
	el.dump();
    }

    return data;
}

boost::shared_ptr<amf::Buffer>
OflaDemoTest::formatOflaDemoResponse(double num, amf::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatOflaDemoResponse(num, data.reference(), data.allocated());
}

boost::shared_ptr<amf::Buffer>
OflaDemoTest::formatOflaDemoResponse(double num, boost::uint8_t *data, size_t size)
{
//    GNASH_REPORT_FUNCTION;

    string result = "_result";
    Element oflaDemo;
    oflaDemo.makeString(result);

    Element index;
    index.makeNumber(num);

    Element null;
    null.makeNull();

    boost::shared_ptr<amf::Buffer> encoflaDemo = oflaDemo.encode();
    boost::shared_ptr<amf::Buffer> encidx  = index.encode();   
    boost::shared_ptr<amf::Buffer> encnull  = null.encode();   

    boost::shared_ptr<amf::Buffer> buf(new amf::Buffer(encoflaDemo->size()
						       + encidx->size()
						       + encnull->size() + size));

    *buf = encoflaDemo;
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
