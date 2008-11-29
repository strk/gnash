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
# include "GnashSystemIOHeaders.h"
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#ifndef __GNUC__
 extern int optind, getopt(int, char *const *, const char *);
 extern char *optarg;
#endif
}

#include <boost/shared_ptr.hpp>

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
#include "GnashException.h"

// classes internal to Cygnal
#include "buffer.h"
#include "handler.h"
#include "cache.h"

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

static void usage();
static void version_and_copyright();
static void cntrlc_handler(int sig);

void connection_handler(Handler::thread_params_t *args);
void dispatch_handler(Handler::thread_params_t *args);
void admin_handler(Handler::thread_params_t *args);

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

static struct sigaction  act;

// The next few global variables have to be global because Boost
// threads don't take arguments. Since these are set in main() before
// any of the threads are started, and it's value should never change,
// it's safe to use these without a mutex, as all threads share the
// same read-only value.

// This is the default path to look in for files to be streamed.
const char *docroot = 0;

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

static LogFile& dbglogfile = LogFile::getDefaultInstance();

// The rcfile is loaded and parsed here:
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

static Cache& cache = Cache::getDefaultInstance();

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
	<< _("  -p   --port-offset   Port offset for debugging") << endl
	<< _("  -n,  --netdebug      Turn on net debugging messages") << endl
        << _("  -t,  --testing       Turn on special Gnash testing support") << endl
	<< _("  -a,  --admin         Enable the administration thread") << endl
	<< _("  -r,  --root          Document root for all files") << endl
	<< _("  -c,  --threads       Enable Threading") << endl
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
        { 'a', "admin",         Arg_parser::no  },
        { 'r', "root",          Arg_parser::yes },
        { 'm', "multithreaded", Arg_parser::no }
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
    if (crcfile.getPortOffset()) {
      port_offset = crcfile.getPortOffset();
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
	  case 'r':
	      docroot = parser.argument(i).c_str();
	      break;
	  case 'm':
	      crcfile.setThreadingFlag(true);
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
//    sigaction (SIGPIPE, &act, NULL);

    boost::mutex::scoped_lock lk(alldone_mutex);
    
//     struct thread_params rtmp_data;
//     struct thread_params ssl_data;
//     rtmp_data.port = port_offset + 1935;
//     boost::thread rtmp_port(boost::bind(&rtmp_thread, &rtmp_data));
    // Admin handler
    if (admin) {
	Handler::thread_params_t admin_data;
	admin_data.port = gnash::ADMIN_PORT;
	boost::thread admin_thread(boost::bind(&admin_handler, &admin_data));
//	admin_thread.join();
    }

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
    if (crcfile.getThreadingFlag()) {
      boost::thread http_thread(boost::bind(&connection_handler, &http_data));
    } else {
      connection_handler(&http_data);
    }
    
#if 0  
      // Incomming connection handler for port 1935, RTMP. As RTMP
      // is not a priviledged port, we just open it without an offset.
      Handler::thread_params_t rtmp_data;
      rtmp_data.port = port_offset + gnash::RTMP_PORT;
      rtmp_data.netfd = 0;
      rtmp_data.filespec = docroot;
      if (crcfile.getThreadingFlag()) {
	  boost::thread rtmp_thread(boost::bind(&connection_handler, &rtmp_data));
    } else {
      connection_handler(&rtmp_data);
    }
#endif
    
    // wait for the thread to finish
//     http_thread.join();
//     rtmp_thread.join();

      // Wait for all the threads to die
      alldone.wait(lk);
      
      log_debug (_("Cygnal done..."));
    
      return(0);
}

// Trap Control-C so we can cleanly exit
static void
cntrlc_handler (int sig)
{
    log_debug(_("Got a %d interrupt"), sig);
//    sigaction (SIGINT, &act, NULL);
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
    int retries = 100;
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
	sleep(1);
	do {
	    Network::byte_t data[ADMINPKTSIZE+1];
	    memset(data, 0, ADMINPKTSIZE+1);
	    const char *ptr = reinterpret_cast<const char *>(data);
	    ret = net.readNet(data, ADMINPKTSIZE, 100);
	    if (ret < 0) {
		log_debug("no more admin data, exiting...\n");
		if ((ret == 0) && cmd != Handler::POLL) {
		    break;
		}
	    } else {
		// force the case to make comparisons easier. Only compare enough characters to
		// till each command is unique.
		std::transform(ptr, ptr + ret, data, (int(*)(int)) toupper);
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
	      {
#ifdef USE_STATS_CACHE
//		  cache.dump();
		  string results = cache.stats(false);
		  if (results.size()) {
		      net.writeNet(results);
		      results.clear();
		  }
#endif
#if 0
		  response << handlers.size() << " handlers are currently active.";
 		  for (hit = handlers.begin(); hit != handlers.end(); hit++) {
		      int fd = hit->first;
 		      Handler *hand = hit->second;
		      response << fd << ","
			       << hand->insize()
			       << "," << hand->outsize()
			       << "\r\n";
		      net.writeNet(response);
		      index++;
		  }
#endif
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
	} while (ret > 0);
        log_debug("admin_handler: Done...!\n");
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
    bool done = false;
    static int tid = 0;
    Network net;
    if (netdebug) {
	net.toggleDebug(true);
    }
    // Start a server on this tcp/ip port.
    fd = net.createServer(args->port);
    log_debug("Starting Connection Handler for fd #%d, port %hd", fd, args->port);

    // Get the number of cpus in this system. For multicore
    // systems we'll get better load balancing if we keep all the
    // cpus busy. So a pool of threrads is started for each cpu,
    // the default being just one. Each thread is reponsible for
    // handling part of the total active file descriptors.
#ifdef HAVE_SYSCONF
    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    log_debug("This system has %d cpus.", ncpus);
#endif	
    size_t nfds = crcfile.getFDThread();
    
    log_debug("This system is configured for %d file descriptors to be watched by each thread.", nfds);
    
    // cap the number of threads
    int cpu = 0;
    cpu = (cpu % ncpus);
    
    // Get the next thread ID to hand off handling this file
    // descriptor to. If the limit for threads per cpu hasn't been
    // set or is set to 0, assume one thread per processor by
    // default. There won't even be threads for each cpu if
    // threading has been disabled in the cygnal config file.
    int spawn_limit = 0;
    if (nfds == 0) {
	spawn_limit = ncpus;
    } else {
	spawn_limit = ncpus * nfds;
    }

    // Rotate in a range of 0 to the limit.
    tid = (tid + 1) % (spawn_limit + 1);
    log_debug("thread ID %d for fd #%d", tid, args->netfd);
	
    Handler *hand = new Handler;
    args->handler = hand;
    if (crcfile.getThreadingFlag()) {
      log_debug("Multi-threaded mode for server on fd #%d", fd);
//      log_debug("Starting handler: %x for fd #%d", (void *)hand, args->netfd);
      boost::thread handler(boost::bind(&dispatch_handler, args));
    }
    
    // FIXME: this runs forever, we probably want a cleaner way to
    // test for the end of time.
    do {
	net.setPort(args->port);
	if (netdebug) {
	    net.toggleDebug(true);
	}
	// Wait for a connection to this tcp/ip from a client. If set
	// to true, this will block until a request comes in. If set
	// to single threaded mode, this will only allow one client to
	// connect at a time. This is to make it easier to debug
	// things when you have a heavily threadd application.
	args->netfd = net.newConnection(true, fd);
	if (args->netfd <= 0) {
	    log_debug("No new network connections");
	    continue;
	}
	
	log_debug("New network connection for fd #%d", args->netfd);
    
	struct pollfd fds;
	fds.fd = args->netfd;
	fds.events = POLLIN |POLLRDHUP;
	if (args->port == (port_offset + RTMPT_PORT)) {
	    hand->addPollFD(fds, http_handler);
	}
//  	if (args->port == RTMP_PORT) {
//  	    hand->addPollFD(fds, rtmp_handler);
//  	}
	// if supporting multiple threads
	if (crcfile.getThreadingFlag()) {
	    hand->notify();
	} else {
	  log_debug("Single threaded mode for fd #%d", args->netfd);
	  dispatch_handler(args);
#if 0
	  if (args->port == (port_offset + RTMPT_PORT)) {
	    boost::thread handler(boost::bind(&http_handler, args));
	  }
	  if (args->port == (port_offset + RTMP_PORT)) {
	    boost::thread handler(boost::bind(&rtmp_handler, args));
	  }
	} else {		// single threaded
#endif
	}
//	net.closeNet(args->netfd); 		// this shuts down this socket connection
	log_debug("Restarting loop for next connection for port %d...", args->port);
    } while(!done);
    
    // All threads should wake up now.
    alldone.notify_all();

} // end of connection_handler

void
dispatch_handler(Handler::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;

    Handler *hand = reinterpret_cast<Handler *>(args->handler);
    Network net;
    int timeout = 5000;

    while(!hand->timetodie()) {    
	int limit = hand->getPollFDSize();
	net.setTimeout(timeout);
	cerr << "LIMIT is: " << limit << endl;
	if (limit > 0) {
	    struct pollfd *fds = hand->getPollFDPtr();
	    boost::shared_ptr< vector<struct pollfd> > hits;
	    try {
//                boost::shared_ptr< vector< int > > hits(net.waitForNetData(limit, fds));
                hits = net.waitForNetData(limit, fds);
		vector<struct pollfd>::iterator it;
		cerr << "Hits: " << hits->size() << endl;
		cerr << "Pollfds: " << hand->getPollFDSize() << endl;
		for (it = hits->begin(); it != hits->end(); it++) {
		    if ((it->revents & POLLRDHUP) || (it->revents & POLLNVAL))  {
			log_debug("Revents has a POLLRDHUP or POLLNVAL set to %d for fd #%d",
				  it->revents, it->fd);
// 			hand->erasePollFD(it);
 			net.closeNet(it->fd);
//			continue;
		    }
		    log_debug("Got something on fd #%d, 0x%x", it->fd, it->revents);
		    hand->getEntry(it->fd)(args);
// 		    if (!crcfile.getThreadingFlag()) {
// 			hand->die();
// 		    }
		    hand->erasePollFD(it->fd);
		    net.closeNet(it->fd);
		}
	    } catch (std::exception& e) {
		log_error("Network connection was dropped:  %s", e.what());
		vector<struct pollfd>::const_iterator it;
		if (hits) {
		    for (it = hits->begin(); it != hits->end(); it++) {
			log_debug("Need to disconnect fd #%d, it got an error.", (*it).fd);
//		    hand->erasePollFD(it);
// 		    net.closeNet(it->fd);
		    }
		}
	    }
        } else {
	    log_debug("nothing to wait for...");
	    if (crcfile.getThreadingFlag()) {
		hand->wait();
		log_debug("Got new network file descriptor to watch");
	    } else {
		return;
	    }
        }
    }
} // end of dispatch_handler
	

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
