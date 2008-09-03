// rtmpget.cpp:  RTMP file downloader utility
// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#include <string>
#include <iostream>
#include <sstream>
#include <csignal>
#include <vector>
#include <sys/mman.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gettext.h>

// classes internal to Gnash
#include "gnash.h"
#include "network.h"
#include "log.h"
#include "http.h"
#include "limits.h"
#include "netstats.h"
#include "statistics.h"
#include "gmemory.h"
#include "arg_parser.h"
#include "amf.h"
#include "rtmp.h"
#include "rtmp_client.h"
#include "rtmp_msg.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "URL.h"

// classes internal to Cygnal
#include "buffer.h"
#include "handler.h"

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/time_zone_base.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using gnash::log_debug;
using namespace std;
using namespace gnash;
using namespace amf;

static void usage();
static void version_and_copyright();
static void cntrlc_handler(int sig);

void connection_handler(Handler::thread_params_t *args);
void admin_handler(Handler::thread_params_t *args);

LogFile& dbglogfile = LogFile::getDefaultInstance();

// The rcfile is loaded and parsed here:
RcInitFile& rcfile = RcInitFile::getDefaultInstance();

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

static struct sigaction  act;

std::vector<std::string> infiles;

// The next few global variables have to be global because Boost
// threads don't take arguments. Since these are set in main() before
// any of the threads are started, and it's value should never change,
// it's safe to use these without a mutex, as all threads share the
// same read-only value.

// end of globals

int
main(int argc, char *argv[])
{
    // Initialize national language support
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    
    // If no command line arguments have been supplied, do nothing but
    // print the  usage message.
    if (argc < 2) {
        usage();
        exit(0);
    }

   const Arg_parser::Option opts[] =
        {
        { 'h', "help",          Arg_parser::no  },
        { 'V', "version",       Arg_parser::no  },
        { 'p', "port-offset",   Arg_parser::yes },
        { 'v', "verbose",       Arg_parser::no  },
        { 'd', "dump",          Arg_parser::no  },
        { 'a', "app",           Arg_parser::yes  },
        { 'p', "path",          Arg_parser::yes  },
        { 'f', "filename",      Arg_parser::yes  },
        { 't', "tcurl",		Arg_parser::yes  },
        { 's', "swfurl",	Arg_parser::yes  },
        { 'u', "url",	        Arg_parser::yes  },
        { 'n', "netdebug",      Arg_parser::no  }
        };

    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() )	
    {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }

    // Set the log file name before trying to write to
    // it, or we might get two.
    dbglogfile.setLogFilename("rtmpget-dbg.log");
    
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
    }    

    RTMPClient client;    
    short port = 0;
    string protocol;		// the network protocol, rtmp or http
    string app;			// the application name
    string path;		// the path to the file on the server
    string query;		// any queries for the host
    string filename;		// the filename to play
    string tcUrl;		// the tcUrl field
    string swfUrl;		// the swfUrl field
    string pageUrl;		// the pageUrl field
    string hostname;		// the hostname of the server
	
    // Handle command line arguments
    for( int i = 0; i < parser.arguments(); ++i ) {
	const int code = parser.code(i);
	try {
	    switch( code ) {
	      case 'h':
		  version_and_copyright();
		  usage();
		  exit(0);
	      case 'V':
		  version_and_copyright();
		  exit(0);
	      case 'v':
		  dbglogfile.setVerbosity();
		  log_debug (_("Verbose output turned on"));
		  break;
	      case 'a':
		  app = parser.argument(i);
		  break;
	      case 'p':
		  path = parser.argument(i);
		  break;
	      case 't':
		  tcUrl = parser.argument(i);
		  break;
	      case 's':
		  swfUrl = parser.argument(i);
		  break;
	      case 'f':
		  filename = parser.argument(i);
		  break;
	      case 'n':
		  netdebug = true;
		  break;
	      case 'd':
		  rcfile.dump();
		  exit(0);
		  break;
	      case 0:
		  infiles.push_back(parser.argument(i));
		  break;
	      default:
		  log_error (_("Extraneous argument: %s"), parser.argument(i).c_str());
	    }
	}
	
	catch (Arg_parser::ArgParserException &e) {
	    cerr << _("Error parsing command line options: ") << e.what() << endl;
	    cerr << _("This is a Gnash bug.") << endl;
	}
    }
    
    if (infiles.empty()) {
        cerr << _("Error: no input file was specified. Exiting.") << endl;
        usage();
        return EXIT_FAILURE;
    }
    
    string url = infiles[0];
    string portstr;
    
    // Trap ^C (SIGINT) so we can kill all the threads
    act.sa_handler = cntrlc_handler;
    sigaction (SIGINT, &act, NULL);

    // Take a standard URL apart.
    string::size_type start = url.find(':', 0);
    if (start != string::npos) {
	protocol = url.substr(0, start);
	start += 3;		// skip past the "://" part after the protocol
    }
    string::size_type end = url.find('/', start);
    if (end != string::npos) {
	string::size_type pos = url.find(':', start);
	if (pos != string::npos) {
	    hostname = url.substr(start, pos - start);
	    portstr = url.substr(pos + 1, (end - pos) - 1);
	    port = strtol(portstr.c_str(), NULL, 0) & 0xffff;
	} else {
	    hostname = url.substr(start, end - start);
	    if ((protocol == "http") || (protocol == "rtmpt")) {
		port = RTMPT_PORT;
	    }
	    if (protocol == "rtmp") {
		port = RTMP_PORT;
	    }
	}
	start = end;
    }
    end = url.rfind('/');
    if (end != string::npos) {
	path = url.substr(start + 1, end - start - 1);
	start = end;
	filename = url.substr(end + 1);
    }

    start = path.find('?', 0);
    if (start != string::npos) {
	end = path.find('/', 0);
	query = path.substr(0, end);
	app = query;
	path = path.substr(end, path.size());
    } else {
	app = path;
    }

    if (tcUrl.empty()) {
	tcUrl = protocol + "://" + hostname;
	if (!portstr.empty()) {
	    tcUrl += ":" + portstr;
	}
	if (!query.empty()) {
	    tcUrl += "/" + query;
	} else {
	    tcUrl += "/" + path;
	}
    }
    
    if (app.empty()) {
	// Get the application name
	app = path;
	
	if (!query.empty()) {
	    app = path;
	    app += "?" + query;
	}
    }

    if (swfUrl.empty()) {
	swfUrl = "mediaplayer.swf";
    }
    if (pageUrl.empty()) {
	pageUrl = "http://gnashdev.org";
    }
    
    if (netdebug) {
	cerr << "URL is " << url << endl;
	cerr << "Protocol is " << protocol << endl;
	cerr << "Host is "  << hostname << endl;
	cerr << "Port is "  << port << endl;
	cerr << "Path is "  << path << endl;
	cerr << "Filename is "  << filename << endl;
	cerr << "App is "   << app << endl;
	cerr << "Query is " << query << endl;
	cerr << "tcUrl is " << tcUrl << endl;
	cerr << "swfUrl is " << swfUrl << endl;
	cerr << "pageUrl is " << pageUrl << endl;
    }

    client.toggleDebug(netdebug);
    if (client.createClient(hostname, port) == false) {
	log_error("Can't connect to RTMP server %s", hostname);
	exit(-1);
    }
    
    client.handShakeRequest();
    
    client.clientFinish();
    
    // Make a buffer to hold the handshake data.
    Buffer buf(1537);
    RTMP::rtmp_head_t *rthead = 0;
    int ret = 0;
    log_debug("Sending NetConnection Connect message,");
    Buffer *buf2 = client.encodeConnect(app.c_str(), swfUrl.c_str(), tcUrl.c_str(), 615, 124, 1, pageUrl.c_str());
//    Buffer *buf2 = client.encodeConnect("video/2006/sekt/gate06/tablan_valentin", "mediaplayer.swf", "rtmp://velblod.videolectures.net/video/2006/sekt/gate06/tablan_valentin", 615, 124, 1, "http://gnashdev.org");
//    Buffer *buf2 = client.encodeConnect("oflaDemo", "http://192.168.1.70/software/gnash/tests/ofla_demo.swf", "rtmp://localhost/oflaDemo/stream", 615, 124, 1, "http://192.168.1.70/software/gnash/tests/index.html");
    buf2->resize(buf2->size() - 6); // FIXME: encodeConnect returns the wrong size for the buffer!
    size_t total_size = buf2->size();    
    RTMPMsg *msg1 = client.sendRecvMsg(0x3, RTMP::HEADER_12, total_size, RTMP::INVOKE, RTMPMsg::FROM_CLIENT, buf2);
    
    if (msg1) {
		msg1->dump();
		if (msg1->getStatus() ==  RTMPMsg::NC_CONNECT_SUCCESS) {
	    	log_debug("Sent NetConnection Connect message sucessfully");
		} else {
	    log_error("Couldn't send NetConnection Connect message,");
	//	    exit(-1);
		}
    }
    
    // make the createStream for ID 3 encoded object
    log_debug("Sending NetStream::createStream message,");
    Buffer *buf3 = client.encodeStream(0x2);
//    buf3->dump();
    total_size = buf3->size();
    RTMPMsg *msg2 = client.sendRecvMsg(0x3, RTMP::HEADER_12, total_size, RTMP::INVOKE, RTMPMsg::FROM_CLIENT, buf3);
    double streamID = 0.0;
    if (msg2) {
	msg2->dump();
	log_debug("Sent NetStream::createStream message successfully.");
	std::vector<amf::Element *> hell = msg2->getElements();
	if (hell.size() > 0) {
	    streamID = hell[0]->to_number();
	} else {
	    if (msg2->getMethodName() == "close") {	
		log_debug("Got close packet!!! Exiting...");
		exit(0);
	    }
	    streamID = 0.0;
	}
    } else {
	log_error("Couldn't send NetStream::createStream message,");
//	exit(-1);
    }
    int id = int(streamID);
    log_debug("Stream ID returned from createStream is: %d", id);
    
    // make the NetStream::play() operations for ID 2 encoded object
//    log_debug("Sending NetStream play message,");
    Buffer *buf4 = client.encodeStreamOp(0, RTMP::STREAM_PLAY, false, filename.c_str());
//    Buffer *buf4 = client.encodeStreamOp(0, RTMP::STREAM_PLAY, false, "gate06_tablan_bcueu_01");
//     log_debug("TRACE: buf4: %s", hexify(buf4->reference(), buf4->size(), true));
    total_size = buf4->size();
    RTMPMsg *msg3 = client.sendRecvMsg(0x8, RTMP::HEADER_12, total_size, RTMP::INVOKE, RTMPMsg::FROM_CLIENT, buf4);
    if (msg3) {
        msg3->dump();
        if (msg3->getStatus() ==  RTMPMsg::NS_PLAY_START) {
	    log_debug("Sent NetStream::play message sucessfully.");
	} else {
	    log_error("Couldn't send NetStream::play message,");
//	    exit(-1);
	}
    }

    int loop = 20;
    do {
	Buffer *msgs = client.recvMsg(1);	// use a 1 second timeout
	if (msgs == 0) {
	    log_error("Never got any data!");
	    exit(-1);
	}
	RTMP::queues_t *que = client.split(msgs);
	if (que == 0) {
	    log_error("Never got any messages!");
	    exit(-1);
	}

#if 0
	deque<CQue *>::iterator it;
	for (it = que->begin(); it != que->end(); it++) {
	    CQue *q = *(it);
	    q->dump();
	}
#endif
	while (que->size()) {
	    cerr << "QUE SIZE: " << que->size() << endl;
	    Buffer *ptr = que->front()->pop();
	    if ((ptr->size() >= 0) && (ptr->size() <= 0xffff)) {
		que->pop_front();	// delete the item from the queue
		RTMP::rtmp_head_t *rthead = client.decodeHeader(ptr);
		msg2 = client.decodeMsgBody(ptr);
		if (msg2 == 0) {
//		    log_error("Couldn't process the RTMP message!");
		    continue;
		}
	    } else {
		log_error("Buffer size (%d) out of range at %d", ptr->size(), __LINE__);
		break;
	    }
	}
    } while(loop--);

//     std::vector<amf::Element *> hell = msg2->getElements();
//     std::vector<amf::Element *> props = hell[0]->getProperties();

//     cerr << "HELL Elements: " << hell.size() << endl;
//     cerr << "HELL Properties: " << props.size() << endl;

// //     cerr << props[0]->getName() << endl;
// //     cerr << props[0]->to_string() << endl;
//     cerr << props[0]->getName() << endl;
// //    cerr << props[0]->to_number() << endl;
//     cerr << props[1]->getName() << ": " << props[1]->to_string() << endl;
//     cerr << props[2]->getName() << ": " << props[3]->to_string() << endl;
//     cerr << props[3]->getName() << ": " << props[3]->to_string() << endl;

//     Element *eell = hell[0]->findProperty("level");
//     if (eell) {
// 	eell->dump();
//     }
//     *eell = hell[0]->findProperty("code");
//     if (eell) {
// 	eell->dump();
//     }

#if 0
    // Write the packet to disk so we can anaylze it with other tools
    int fd = open("outbuf.raw",O_WRONLY|O_CREAT, S_IRWXU);
    if (fd < 0) {
        perror("open");
    }
    cout << "Writing packet to disk: \"outbuf.raw\"" << endl;
//     write(fd, out, 12);
//     write(fd, outbuf.begin(), amf_obj.totalsize());
    write(fd, buf2->reference(), buf2->size());
    write(fd, buf3->reference(), buf3->size());
    close(fd);
#endif    
}

// Trap Control-C so we can cleanly exit
static void
cntrlc_handler (int /*sig*/)
{
    log_debug(_("Got an interrupt"));

    exit(-1);
}

static void
version_and_copyright()
{
    cout << "rtmpget " << VERSION << endl
        << endl
        << _("Copyright (C) 2008 Free Software Foundation, Inc.\n"
        "Cygnal comes with NO WARRANTY, to the extent permitted by law.\n"
        "You may redistribute copies of Cygnal under the terms of the GNU General\n"
        "Public License.  For more information, see the file named COPYING.\n")
    << endl;
}


static void
usage()
{
	cout << _("rtmpget -- a file downloaded that uses RTMP.") << endl
	<< endl
	<< _("Usage: rtmpget     [options...]") << endl
	<< _("  -h,  --help          Print this help and exit") << endl
	<< _("  -V,  --version       Print version information and exit") << endl
	<< _("  -v,  --verbose       Output verbose debug info") << endl
	<< _("  -n,  --netdebug      Verbose networking debug info") << endl
	<< _("  -d,  --dump          display init file to terminal") << endl
	<< endl;
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
