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
#include <csignal>
#include <vector>
#include <sys/mman.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gettext.h"

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
#include "rtmp_server.h"
#include "http.h"
#include "utility.h"
#include "limits.h"
#include "netstats.h"
#include "statistics.h"
//#include "stream.h"
#include "gmemory.h"
#include "diskstream.h"
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
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

//using gnash::log_debug;
using namespace std;
using namespace gnash;
using namespace cygnal;
using namespace amf;

// Keep a list of all active network connections
namespace gnash {
extern map<int, gnash::Handler *> handlers;
}

static void usage();
static void version_and_copyright();
static void cntrlc_handler(int sig);

void connection_handler(Handler::thread_params_t *args);
void admin_handler(Handler::thread_params_t *args);

LogFile& dbglogfile = LogFile::getDefaultInstance();

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

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

// Toggle the admin thread
static bool admin = false;

// Admin commands are small
const int ADMINPKTSIZE = 80;

// end of globals

// The rcfile is loaded and parsed here:
CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

// This mutex is used to signify when all the threads are done.
static boost::condition	alldone;
static boost::mutex	alldone_mutex;

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
        { 'd', "dump",          Arg_parser::no  },
        { 'n', "netdebug",      Arg_parser::no  },
        { 't', "testing",       Arg_parser::no  },
        { 'a', "admin",         Arg_parser::no  }
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
	  case 't':
	      crcfile.setTestingFlag(true);
	      break;
	  case 'a':
	      admin = true;
	      break;
	  case 'v':
	      dbglogfile.setVerbosity();
	      log_debug (_("Verbose output turned on"));
	      break;
	  case 'p':
	      port_offset = parser.argument<int>(i);
	      crcfile.setPortOffset(port_offset);
	      break;
	  case 'n':
	      netdebug = true;
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

    boost::mutex::scoped_lock lk(alldone_mutex);
    
//     struct thread_params rtmp_data;
//     struct thread_params ssl_data;
//     rtmp_data.port = port_offset + 1935;
//     boost::thread rtmp_port(boost::bind(&rtmp_thread, &rtmp_data));

    // Incomming connection handler for port 80, HTTP and
    // RTMPT. As port 80 requires root access, cygnal supports a
    // "port offset" for debugging and development of the
    // server. Since this port offset changes the constant to test
    // for which protocol, we pass the info to the start thread so
    // it knows which handler to invoke. 
    Handler::thread_params_t http_data;
    http_data.port = port_offset + gnash::RTMPT_PORT;
    http_data.netfd = 0;
    http_data.filespec = docroot;
    boost::thread http_thread(boost::bind(&connection_handler, &http_data));
    
    // Incomming connection handler for port 1935, RTMP. As RTMP
    // is not a priviledged port, we just open it without an offset.
    Handler::thread_params_t rtmp_data;
    rtmp_data.port = gnash::RTMP_PORT;
    rtmp_data.netfd = 0;
    rtmp_data.filespec = docroot;
    boost::thread rtmp_thread(boost::bind(&connection_handler, &rtmp_data));
    
    // Admin handler
    if (admin) {
	Handler::thread_params_t admin_data;
	admin_data.port = gnash::ADMIN_PORT;
	boost::thread admin_thread(boost::bind(&admin_handler, &admin_data));
	admin_thread.join();
    }

    // wait for the thread to finish
//     http_thread.join();
//     rtmp_thread.join();

    // Wait for all the threads to die
    alldone.wait(lk);
    
    log_debug (_("Cygnal done..."));
    
    return(0);
}

#if 0

static void
ssl_thread(struct thread_params *conndata)
{
    GNASH_REPORT_FUNCTION;
    int retries = 0;
    HTTP www;
    RTMPproto proto;
    struct thread_params loadfile;
    string filespec;
    int port = RTMPTS_PORT + port_offset;

    Statistics st;
    st.setFileType(NetStats::RTMPTS_PORT);
    
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

    // All threads should exit now.
    alldone.notify_all();
}
    
void
connection_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
    int fd = 0;
//    list<Handler *> *handlers = reinterpret_cast<list<Handler *> *>(args->handle);

    Network net;
    if (netdebug) {
	net.toggleDebug(true);
    }
    fd = net.createServer(args->port);
    log_debug("Starting Connection Handler for fd #%d, port %hd", fd, args->port);
    // FIXME: this runs forever, we probably want a cleaner way to test for the end of time.
    do {
	Handler *hand = new Handler;
	hand->setPort(args->port);
	if (netdebug) {
	    hand->toggleDebug(true);
	}
	args->netfd = hand->newConnection(true, fd);
	args->handle = hand;
 	log_debug("Adding handler: %x for fd #%d", (void *)hand, args->netfd);
	handlers[args->netfd] = hand;

	if (crcfile.getThreadingFlag()) {
	    hand->start(args);
	} else {
	    log_debug(_("Starting Handlers for port %d, tid %ld"),
		      args->port, get_thread_id());
	    
	    if (args->port == (port_offset + RTMPT_PORT)) {
		boost::thread handler(boost::bind(&http_handler, args));
	    }
	    if (args->port == RTMP_PORT) {
		boost::thread handler(boost::bind(&rtmp_handler, args));
	    }
	}
	log_debug("Restarting loop for next connection for port %d...", args->port);
    } while(1);

    // All threads should exit now.
    alldone.notify_all();

} // end of connection_handler

extern "C" {
void
http_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
//    struct thread_params thread_data;
    string url, filespec, parameters;
    string::size_type pos;
    Handler *hand = reinterpret_cast<Handler *>(args->handle);
    HTTP www;
    www.setHandler(hand);

    log_debug(_("Starting HTTP Handler for fd #%d, tid %ld"),
	      args->netfd, get_thread_id());
    
    string docroot = args->filespec;
    
    log_debug("Starting to wait for data in net for fd #%d", args->netfd);

    // Wait for data, and when we get it, process it.
    do {
#ifdef THREADED_IO
	hand->wait();
	if (hand->timetodie()) {
	    log_debug("Not waiting no more, no more for more HTTP data for fd #%d...", args->netfd);
	    map<int, Handler *>::iterator hit = handlers.find(args->netfd);
	    if ((*hit).second) {
		log_debug("Removing handle %x for HTTP on fd #%d",
			  (void *)hand, args->netfd);
		handlers.erase(args->netfd);
		delete (*hit).second;
	    }
	    return;
	}
#endif
	
#ifdef USE_STATISTICS
	struct timespec start;
	clock_gettime (CLOCK_REALTIME, &start);
#endif
	
// 	conndata->statistics->setFileType(NetStats::RTMPT);
// 	conndata->statistics->startClock();
//	args->netfd = www.getFileFd();
//	www.recvMsg(5);
	www.recvMsg(args->netfd);
	
	if (!www.processGetRequest()) {
	    hand->die();	// tell all the threads for this connection to die
	    hand->notifyin();
	    log_debug("Net HTTP done for fd #%d...", args->netfd);
// 	    hand->closeNet(args->netfd);
	    return;
	}
	url = docroot;
	url += www.getURL();
	pos = url.find("?");
	filespec = url.substr(0, pos);
	parameters = url.substr(pos + 1, url.size());
	// Get the file size for the HTTP header
	
	if (www.getFileStats(filespec) == amf::AMF::FILETYPE_ERROR) {
	    www.formatErrorResponse(HTTP::NOT_FOUND);
	}
	// Send the reply
	www.formatGetReply(HTTP::LIFE_IS_GOOD);
//	cerr << "Size = " << www.getHeader().size() << "	" << www.getHeader() << endl;
	
	hand->Network::writeNet(args->netfd, (boost::uint8_t *)www.getHeader().c_str(), www.getHeader().size());
//	hand->writeNet(args->netfd, www.getHeader(), www.getHeader().size());
//	strcpy(thread_data.filespec, filespec.c_str());
//	thread_data.statistics = conndata->statistics;
	
	// Keep track of the network statistics
//	conndata->statistics->stopClock();
// 	log_debug (_("Bytes read: %d"), www.getBytesIn());
// 	log_debug (_("Bytes written: %d"), www.getBytesOut());
//	st.setBytes(www.getBytesIn() + www.getBytesOut());
//	conndata->statistics->addStats();

	if (filespec[filespec.size()-1] == '/') {
	    filespec += "index.html";
	}

// 	DiskStream filestream;
// 	filestream.open(filespec);
#if 0
	if (url != docroot) {
	    log_debug (_("File to load is: %s"), filespec.c_str());
	    log_debug (_("Parameters are: %s"), parameters.c_str());
	    struct stat st;
	    int filefd;
	    size_t ret;
#ifdef USE_STATISTICS
	    struct timespec start;
	    clock_gettime (CLOCK_REALTIME, &start);
#endif
	    if (stat(filespec.c_str(), &st) == 0) {
		filefd = ::open(filespec.c_str(), O_RDONLY);
		log_debug (_("File \"%s\" is %lld bytes in size, disk fd #%d"), filespec,
			   st.st_size, filefd);
		do {
		    boost::shared_ptr<amf::Buffer> buf(new amf::Buffer);
		    ret = read(filefd, buf->reference(), buf->size());
		    if (ret == 0) { // the file is done
			break;
		    }
		    if (ret != buf->size()) {
			buf->resize(ret);
			log_debug("Got last data block from disk file, size %d", buf->size());
		    }
		    log_debug("Read %d bytes from %s.", ret, filespec);
#if 0
		    hand->pushout(buf);
		    hand->notifyout();
#else
		    // Don't bother with the outgoing que
		    if (ret > 0) {
			ret = hand->writeNet(buf);
		    }
#endif
		} while(ret > 0);
		log_debug("Done transferring %s to net fd #%d",
			  filespec, args->netfd);
		::close(filefd); // close the disk file
		// See if this is a persistant connection
// 		if (!www.keepAlive()) {
// 		    log_debug("Keep-Alive is off", www.keepAlive());
// // 		    hand->closeConnection();
//  		}
#ifdef USE_STATISTICS
		struct timespec end;
		clock_gettime (CLOCK_REALTIME, &end);
		log_debug("Read %d bytes from \"%s\" in %f seconds",
			  st.st_size, filespec,
			  (float)((end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9)));
#endif
	    }

// 	    memset(args->filespec, 0, 256);
// 	    memcpy(->filespec, filespec.c_str(), filespec.size());
// 	    boost::thread sendthr(boost::bind(&stream_thread, args));
// 	    sendthr.join();
	}
#endif
	
#ifdef USE_STATISTICS
	struct timespec end;
	clock_gettime (CLOCK_REALTIME, &end);
	log_debug("Processing time for GET request was %f seconds",
		  (float)((end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec)/1e9)));
#endif
//	conndata->statistics->dump();
//    }
    } while(!hand->timetodie());
    
    log_debug("httphandler all done now finally...");
    
} // end of httphandler
    
} // end of extern C

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
