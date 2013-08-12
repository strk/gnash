// Red5 server side support for the oflaDemo_test via RTMP
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#if !defined(_MSC_VER)
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <dirent.h>
# include <cerrno>
#else
#include <io.h>
#define dup _dup
#endif

// Gnash headers

#include "GnashFileUtilities.h"
#include "amf.h"
#include "arg_parser.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "URL.h"
#include "log.h"

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

    boost::shared_ptr<cygnal::Buffer> oflaDemo_read_func()
    {
// 	GNASH_REPORT_FUNCTION;
	
	boost::shared_ptr<cygnal::Buffer> buf = oflaDemo.getResponse();
// 	log_network("%s", hexify(data, safe, true));

        return buf;
	    
//         GNASH_REPORT_RETURN;
    }

    size_t oflaDemo_write_func(boost::uint8_t *data, size_t size)
    {
// 	GNASH_REPORT_FUNCTION;

	boost::shared_ptr<cygnal::Buffer> buf = oflaDemo.getResponse();

        vector<boost::shared_ptr<cygnal::Element> > request =
	    oflaDemo.parseOflaDemoRequest(data, size);
        
        if (request.empty()) {
            // Send the packet to notify the client that the
            // NetConnection::connect() was sucessful. After the client
            // receives this, the handhsake is completed.
            boost::shared_ptr<cygnal::Buffer> error =
                oflaDemo.encodeResult(RTMPMsg::NC_CALL_FAILED);
            // This builds the full header,which is required as the first part
            // of the packet.
            boost::shared_ptr<cygnal::Buffer> head = oflaDemo.encodeHeader(0x3,
                                          RTMP::HEADER_12, error->allocated(),
                                          RTMP::INVOKE, RTMPMsg::FROM_SERVER);
            boost::scoped_ptr<cygnal::Buffer> response(new cygnal::Buffer(
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
    int netfd = 0;
    
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
        boost::shared_ptr<cygnal::Buffer> bufptr(new cygnal::Buffer);
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
        
        vector<boost::shared_ptr<cygnal::Element> > request = net.parseOflaDemoRequest(
            bufptr->reference(), bufptr->allocated());
        if (request[3]) {
            boost::shared_ptr<cygnal::Buffer> result = net.formatOflaDemoResponse(request[1]->to_number(), *request[3]);
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

std::vector<boost::shared_ptr<demoService::filestats_t> > &
demoService::getListOfAvailableFiles(const std::string &path)
{
//    GNASH_REPORT_FUNCTION;
    return getListOfAvailableFiles(path, ".flv");
}

std::vector<boost::shared_ptr<demoService::filestats_t> > &
demoService::getListOfAvailableFiles(const std::string &path,
				    const std::string &type)
{
    GNASH_REPORT_FUNCTION;
    struct dirent **namelist;

    _path = path;		// store for later

    // If we don't have any files yet, look for some.
    if (_stats.empty()) {
        struct dirent *entry;
#ifndef HAVE_SCANDIR
        log_debug(_("Scanning directory \"%s\" for %s files"), path, type);
        DIR *libdir = opendir(path.c_str());
	
        if (!libdir) {
            log_error(_("Can't open directory %s"), path);
            return _stats;
        }
        while ((entry = readdir(libdir)) != NULL) {
#else
	// The Adobe media server and Red5 sort the directories
	// alphabetically, so we do too.
	int ret = scandir(path.c_str(), &namelist, 0, alphasort);
	for (int i=0; i<ret; ++i) {
	    entry = namelist[i];
#endif	
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
            
            // We only want this type of file.
            if (suffix == type) {
                log_debug(_("Gnash media file name: %s"), name);
                string filespec = path + "/";
                filespec += name;
                struct stat st;
                if (stat(filespec.c_str(), &st) == 0) {
                    boost::shared_ptr<demoService::filestats_t> stats(new filestats_t);
                    stats->name = name;
                    stringstream ss;
                        ss << st.st_size;
                        stats->size = ss.str();
                        // This originally used gmtime(), but then
                        // packet sniffing showed the Adobe player
                        // uses localtime.
                        struct tm *modified = localtime(&st.st_mtime);
                        // we know the max the date string will be is 24.
                        char modstr[24];
                        if (strftime(modstr, 24, "%d/%m/%y %H:%M:%S", modified)) {
                            stats->last = modstr;
                        }
                        _stats.push_back(stats);
                }
            } else {
                continue;
            }
        }
        
#ifndef HAVE_SCANDIR
        if (closedir(libdir) != 0) {
            return _stats;
        }
#endif
   }
    
    return _stats;
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
vector<boost::shared_ptr<cygnal::Element > >
OflaDemoTest::parseOflaDemoRequest(boost::uint8_t *ptr, size_t size)
{
    GNASH_REPORT_FUNCTION;

    demoService demo;
    cygnal::AMF amf;
    vector<boost::shared_ptr<cygnal::Element > > headers;

    boost::shared_ptr<cygnal::Element> el1 = amf.extractAMF(ptr, ptr+size);
    if (!el1) {
        log_error("No AMF data in message!");
        return headers;
    }

    string method = el1->to_string();    
    ptr += amf.totalsize();
    headers.push_back(el1);

    // The second element is a number
    boost::shared_ptr<cygnal::Element> el2 = amf.extractAMF(ptr, ptr+size);
    if (!el2) {
        log_error("No AMF data in message!");
        return headers;
    }
    ptr += amf.totalsize();
    headers.push_back(el2);

    if (method == "demoService.getListOfAvailableFLVs") {
        // Get the path from the NetConnection object we recieved from the
        // client at the end of the handshake process.
        boost::shared_ptr<cygnal::Element> version;
        boost::shared_ptr<cygnal::Element> tcurl;
        boost::shared_ptr<cygnal::Element> swfurl;
        
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
            std::vector<boost::shared_ptr<demoService::filestats_t> > &mediafiles = demo.getFileStats(); 
            // std::vector<demoService::filestats_t> mediafiles = demo.getListOfAvailableFiles(key); 
            std::vector<boost::shared_ptr<demoService::filestats_t> >::iterator it;
            // Make the top level object
            Element toparr;
            toparr.makeECMAArray();
            
            size_t total_size = 0;
            vector<boost::shared_ptr<cygnal::Buffer> > buffers;
            for (it=mediafiles.begin(); it<mediafiles.end(); ++it) {
                vector<boost::shared_ptr<cygnal::Element> > data;
                
                boost::shared_ptr<demoService::filestats_t> file = *it;
                boost::shared_ptr<cygnal::Element> obj(new cygnal::Element);
                obj->makeECMAArray();
                obj->setName(file->name);
                
                boost::shared_ptr<cygnal::Element> modified(new cygnal::Element);
                modified->makeString("lastModified", file->last);
                obj->addProperty(modified);

                boost::shared_ptr<cygnal::Element> name(new cygnal::Element);
                name->makeString("name", file->name);
                obj->addProperty(name);

                boost::shared_ptr<cygnal::Element> size(new cygnal::Element);
                size->makeString("size", file->size);
                obj->addProperty(size);

                data.push_back(obj);
                toparr.addProperty(obj);
            }
            
            boost::shared_ptr<cygnal::Buffer> topenc = toparr.encode();
            total_size += topenc->allocated();
            
            // Start with the method name for the INVOKE
            cygnal::Element method;
            method.makeString("_result");
            boost::shared_ptr<cygnal::Buffer> methodenc  = method.encode();
            total_size += methodenc->allocated();
            
            // Add the stream ID
            cygnal::Element sid;
            sid.makeNumber(2); // FIXME: needs a real value!
            boost::shared_ptr<cygnal::Buffer> sidenc  = sid.encode();
            total_size += sidenc->allocated();

            // There there is always a NULL object to start the data
            Element null;
            null.makeNull();
            boost::shared_ptr<cygnal::Buffer> encnull  = null.encode();
            total_size += encnull->allocated();

            boost::shared_ptr<cygnal::Buffer> result(new cygnal::Buffer(total_size+cygnal::AMF_HEADER_SIZE+RTMP_MAX_HEADER_SIZE+10));            
            _response.reset(new cygnal::Buffer(total_size+cygnal::AMF_HEADER_SIZE+RTMP_MAX_HEADER_SIZE+10));
#if 0
            boost::shared_ptr<cygnal::Buffer> head = encodeHeader(0x3,
			    RTMP::HEADER_8, total_size,
			    RTMP::INVOKE, RTMPMsg::FROM_SERVER);
            *result = head;
#endif
            *_response += methodenc;
            *_response += sidenc;
            *_response += encnull;
            *_response += topenc;

#if 0
            // Followed by all the encoded objects and properties
            vector<boost::shared_ptr<cygnal::Buffer> >::iterator rit;
            for (rit=buffers.begin(); rit<buffers.end(); ++rit) {
                boost::shared_ptr<cygnal::Buffer> buf = *rit;
                *_response += buf;
                std::vector<boost::shared_ptr<cygnal::Element> > data1;
            }
#endif
        }
    } else {
        log_error("Unknown oflaDemo method \"%s\" to INVOKE!", el1->getName());
    }

    return headers;
}

// format a response to the 'oflaDemo' test used for testing Gnash. This
// is only used for testing by developers. The format appears to be
// a string '_result', followed by the number of the test, and then two
// NULL objects.
boost::shared_ptr<cygnal::Buffer>
OflaDemoTest::formatOflaDemoResponse(double num, cygnal::Element &el)
{
//    GNASH_REPORT_FUNCTION;
    boost::shared_ptr<cygnal::Buffer> data = cygnal::AMF::encodeElement(el);
    if (data) {
	return formatOflaDemoResponse(num, data->reference(), data->allocated());
    } else {
	log_error("Couldn't encode element: %s", el.getName());
	el.dump();
    }

    return data;
}

boost::shared_ptr<cygnal::Buffer>
OflaDemoTest::formatOflaDemoResponse(double num, cygnal::Buffer &data)
{
//    GNASH_REPORT_FUNCTION;
    return formatOflaDemoResponse(num, data.reference(), data.allocated());
}

boost::shared_ptr<cygnal::Buffer>
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

    boost::shared_ptr<cygnal::Buffer> encoflaDemo = oflaDemo.encode();
    boost::shared_ptr<cygnal::Buffer> encidx  = index.encode();   
    boost::shared_ptr<cygnal::Buffer> encnull  = null.encode();   

    boost::shared_ptr<cygnal::Buffer> buf(new cygnal::Buffer(encoflaDemo->size()
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

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
