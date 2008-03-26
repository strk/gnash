// cygnal.cpp:  GNU streaming Flash media server, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <signal.h>
#include <vector>
#include <sys/mman.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gettext.h>

extern "C"{
# include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#ifndef __GNUC__
 extern int optind, getopt(int, char *const *, const char *);
 extern char *optarg;
#endif
}

// classes internal to Gnash
#include "network.h"
#include "log.h"
#include "crc.h"
#include "rtmp.h"
#include "http.h"
#include "limits.h"
#include "netstats.h"
#include "statistics.h"
#include "stream.h"
#include "gmemory.h"
#include "arg_parser.h"

// classes internal to Cygnal
#include "buffer.h"
#include "handler.h"

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include <boost/date_time/gregorian/gregorian.hpp>
//#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/time_zone_base.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using gnash::log_debug;
using namespace std;
using namespace cygnal;
using namespace gnash;
using namespace amf;

static void usage();
static void version_and_copyright();
static void cntrlc_handler(int sig);

void connection_handler(Handler::thread_params_t *args);
void admin_handler(Handler::thread_params_t *args);

LogFile& dbglogfile = LogFile::getDefaultInstance();

// The rcfile is loaded and parsed here:
CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

static struct sigaction  act;

// The next few global variables have to be global because Boost
// threads don't take arguments. Since these are set in main() before
// any of the threads are started, and it's value should never change,
// it's safe to use these without a mutex, as all threads share the
// same read-only value.

// This is the default path to look in for files to be streamed.
const char *docroot;

// This is the number of times a thread loop continues, for debugging only
int thread_retries = 10;

// This is added to the default ports for testing so it doesn't
// conflict with apache on the same machine.
static int port_offset = 0;

// Keep a list of all active network connections
namespace cygnal {
  map<int, Handler *> handlers;
}

// Admin commands are small
const int ADMINPKTSIZE = 80;

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

   const Arg_parser::Option opts[] =
        {
        { 'h', "help",          Arg_parser::no  },
        { 'V', "version",       Arg_parser::no  },
        { 'p', "port-offset",   Arg_parser::yes },
        { 'v', "verbose",       Arg_parser::no  },
        { 'd', "dump",          Arg_parser::no  }
        };

    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() )	
    {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }

//    crcfile.loadFiles();
    
    // Set the log file name before trying to write to
    // it, or we might get two.
    dbglogfile.setLogFilename(crcfile.getDebugLog());
    
    if (crcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(crcfile.verbosityLevel());
    }    
    
    if (crcfile.getDocumentRoot().size() > 0) {
        docroot = crcfile.getDocumentRoot().c_str();
        log_debug (_("Document Root for media files is: %s"),
		       docroot);
    } else {
        docroot = "/var/www/html/software/tests/";
    }


    // Handle command line arguments
    for( int i = 0; i < parser.arguments(); ++i ) {
	const int code = parser.code(i);
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
	  case 'p':
	      port_offset = parser.argument<int>(i);
	      crcfile.setPortOffset(port_offset);
	      break;
	  case 'd':
	      crcfile.dump();
	      exit(0);
	      break;
	  default:
	      log_error (_("Extraneous argument: %s"), parser.argument(i).c_str());
        }
    }
    
    // Trap ^C (SIGINT) so we can kill all the threads
    act.sa_handler = cntrlc_handler;
    sigaction (SIGINT, &act, NULL);

//     struct thread_params rtmp_data;
//     struct thread_params ssl_data;
//     rtmp_data.port = port_offset + 1935;
//     boost::thread rtmp_port(boost::bind(&rtmp_thread, &rtmp_data));

#if 1
    // Admin handler
    Handler::thread_params_t admin_data;
    admin_data.port = gnash::ADMIN;
    boost::thread adminhandler(boost::bind(&admin_handler, &admin_data));
#endif

#if 1
    // Incomming connection handler
    Handler::thread_params_t conn_data;
    conn_data.port = port_offset + gnash::RTMPT;
    conn_data.filespec = docroot;
//    conn_data.handle = &handlers;
    boost::thread conn_handler(boost::bind(&connection_handler, &conn_data));
#endif
    
#if 0
    int retries = 10;
    // Run forever
    while (retries > 0) {
	Handler::thread_params_t http_data;
	http_data.netfd = 0;
	http_data.port = port_offset + gnash::RTMPT;
	http_data.filespec = docroot;
	Handler *hand = new Handler;
	http_data.handle = &hand;
	cerr << "Adding handler: " << (void *)hand << endl;
	handlers.push_back(hand);
	hand->createServer(http_data.port);
	hand->start(&http_data);
	cerr << "Removing handler: " << (void *)hand << endl;
	handlers.remove(hand);
	delete hand;
    }
#endif
    
    // wait for the thread to finish
//    adminhandler.join();    
    conn_handler.join();    
    log_debug (_("All done I think..."));
    
    return(0);
}

#if 0
static void
rtmp_thread(struct thread_params *conndata)
{
    GNASH_REPORT_FUNCTION;
    int retries = 0;
    RTMPproto proto;
    
    Statistics st;
    st.setFileType(NetStats::RTMP);

    log_debug("Param port is: %d", conndata->port);
    
    proto.createServer(RTMP);
    while (retries++ < thread_retries) {
	log_debug(_("%s: Thread for RTMP port looping..."), __PRETTY_FUNCTION__);
	proto.newConnection(true);
	st.startClock();
	proto.handShakeWait();
	proto.handShakeResponse();
	proto.serverFinish();
	
	// Keep track of the network statistics
	st.stopClock();
 	log_debug (_("Bytes read: %d"), proto.getBytesIn());
 	log_debug (_("Bytes written: %d"), proto.getBytesOut());
	st.setBytes(proto.getBytesIn() + proto.getBytesOut());
	st.addStats();
	proto.resetBytesIn();
	proto.resetBytesOut();	

	st.dump();
    }    
}

static void
ssl_thread(struct thread_params *conndata)
{
    GNASH_REPORT_FUNCTION;
    int retries = 0;
    HTTP www;
    RTMPproto proto;
    struct thread_params loadfile;
    string filespec;
    int port = RTMPTS + port_offset;

    Statistics st;
    st.setFileType(NetStats::RTMPTS);    
    
    www.createServer(port);
    
    log_debug("Param port is: %d", conndata->port);
    while (retries++ < thread_retries) {
	log_debug (_("%s: Thread for port %d looping..."), __PRETTY_FUNCTION__, port);
	www.newConnection(true);
	loadfile.netfd = www.getFileFd();
	strcpy(loadfile.filespec, "Hello World");
	boost::thread sendthr(boost::bind(&stream_thread, &loadfile));
	sendthr.join();
    }
}
#endif

#if 0
static void
stream_thread(struct thread_params *params)
{
    GNASH_REPORT_FUNCTION;
    
    //struct stat stats;
    //struct thread_params loadfile;
    
    log_debug ("%s: %s", __PRETTY_FUNCTION__, params->filespec);
    
    Stream str;
    str.open(params->filespec, params->netfd);
    str.play();
//    ::close(params->netfd);
}
#endif

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
    cout << "Cygnal " << VERSION << endl
        << endl
        << _("Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.\n"
        "Cygnal comes with NO WARRANTY, to the extent permitted by law.\n"
        "You may redistribute copies of Cygnal under the terms of the GNU General\n"
        "Public License.  For more information, see the file named COPYING.\n")
    << endl;
}


static void
usage()
{
	cout << _("cygnal -- a streaming media server.") << endl
	<< endl
	<< _("Usage: cygnal [options...]") << endl
	<< _("  -h,  --help          Print this help and exit") << endl
	<< _("  -V,  --version       Print version information and exit") << endl
	<< _("  -v,  --verbose       Output verbose debug info") << endl
	<< _("  -p   --port-offset   RTMPT port offset") << endl
	<< endl;
}

// FIXME: this function could be tweaked for better performance
void
admin_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
    int retries = 10;
    int ret;

    map<int, Handler *>::iterator hit;
    stringstream response;
    int index = 0;
    
    Network net;
    Handler::admin_cmd_e cmd = Handler::POLL;
    net.createServer(args->port);
    while (retries > 0) {
	log_debug(_("Starting Admin Handler for port %d"), args->port);
	net.newConnection(true);
	log_debug(_("Got an incoming Admin request"));
	do {
	    Network::byte_t data[ADMINPKTSIZE+1];
	    memset(data, 0, ADMINPKTSIZE+1);
	    const char *ptr = reinterpret_cast<const char *>(data);
	    ret = net.readNet(data, ADMINPKTSIZE, 3);
	    // force the case to make comparisons easier. Only compare enough characters to
	    // till each command is unique.
	    std::transform(ptr, ptr + ret, data, (int(*)(int)) toupper);
	    if (ret == 0) {
		net.writeNet("no more admin data, exiting...\n");
		if ((ret == 0) && cmd != Handler::POLL) {
//		    retries = 0;
		    ret = -1;
		    break;
		}
	    } else {
		if (strncmp(ptr, "QUIT", 4) == 0) { 
		    cmd = Handler::QUIT;
		} else if (strncmp(ptr, "STATUS", 5) == 0) {
		    cmd = Handler::STATUS;
		} else if (strncmp(ptr, "HELP", 2) == 0) {
		    cmd = Handler::HELP;
		    net.writeNet("commands: help, status, poll, interval, statistics, quit.\n");
		} else if (strncmp(ptr, "POLL", 2) == 0) {
		    cmd = Handler::POLL;
		} else if (strncmp(ptr, "INTERVAL", 2) == 0) {
		    cmd = Handler::INTERVAL;
		}
	    }
	    switch (cmd) {
		// close this connection
	      case Handler::QUIT:
		  ret = -1;
		  break;
	      case Handler::STATUS:
		  response << handlers.size() << " handlers are currently active.";
 		  for (hit = handlers.begin(); hit != handlers.end(); hit++) {
		      int fd = hit->first;
 		      Handler *hand = hit->second;
		      response << fd << ","
			       << hand->insize()
			       << "," << hand->outsize()
			       << "\r\n";
		      net.writeNet(response.str());
		      index++;
		  }
		  index = 0;
		  break;
	      case Handler::POLL:
#ifdef USE_STATS_QUEUE
		  index = 0;
		  response << handlers.size() << " handlers are currently active." << "\r\n";
 		  for (hit = handlers.begin(); hit != handlers.end(); hit++) {
		      int fd = hit->first;
 		      Handler *hand = hit->second;
		      struct timespec now;
		      clock_gettime (CLOCK_REALTIME, &now);
		      // Incoming que stats
 		      CQue::que_stats_t *stats = hand->statsin();
		      float diff = (float)((now.tv_sec - stats->start.tv_sec) + ((now.tv_nsec - stats->start.tv_nsec)/1e9));
		      response << fd
			       << "," << stats->totalbytes
			       << "," << diff
			       << "," << stats->totalin
			       << "," << stats->totalout;
		      // Outgoing que stats
 		      stats = hand->statsout();
 		      response << "," <<stats->totalbytes
			       << "," << stats->totalin
			       << "," << stats->totalout
			       << "\r\n";
 		      net.writeNet(response.str());
		      index++;
		  }
		  index = 0;
#endif
		  break;
	      case Handler::INTERVAL:
		  net.writeNet("set interval\n");
		  break;
	      default:
		  break;
	    };
	} while (ret >= 0);
	net.writeNet("admin_handler: Done...!\n");
	net.closeNet();		// this shuts down this socket connection
    }
    net.closeConnection();		// this shuts down the server on this connection
}
    
void
connection_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
    int fd = 0;
//    list<Handler *> *handlers = reinterpret_cast<list<Handler *> *>(args->handle);

    Network net;
//	net.toggleDebug(true);
    fd = net.createServer(args->port);
    // Run forever
    do {
	Handler *hand = new Handler;
	hand->toggleDebug(true); // FIXME: too verbose
	args->netfd = hand->newConnection(true, fd);
	args->handle = hand;
 	log_debug("Adding handler: %x for fd #%d",
		  (void *)hand, args->netfd);
#if 0
	map<int, Handler *>::iterator hit = handlers.find(args->netfd);
	if ((*hit).second) {
	    log_debug("Removing handle %x for fd #%d: ",
		      (void *)hand), args->netfd;
	    handlers.erase(args->netfd);
	}
#endif
	handlers[args->netfd] = hand;
	hand->start(args);

//  	handlers.remove(hand);
// 	delete hand;
	log_debug("Restarting loop...");
    } while(1);
} // end of connection_handler

//} // end of cygnal namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
