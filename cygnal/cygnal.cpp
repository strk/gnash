// cygnal.cpp:  GNU streaming Flash media server, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
//#include "cvm.h"

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
#include "GnashSleep.h" // for usleep comptibility.

// classes internal to Cygnal
#include "buffer.h"
#include "handler.h"
#include "cache.h"
#include "gettext.h"
#include "cygnal.h"

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
#include <boost/thread/tss.hpp>

//using gnash::log_debug;
using namespace std;
using namespace gnash;
using namespace cygnal;
using namespace amf;

static void usage();
static void version_and_copyright();
static void cntrlc_handler(int sig);
static void hup_handler(int sig);

void connection_handler(Network::thread_params_t *args);
void dispatch_handler(Network::thread_params_t *args);
void admin_handler(Network::thread_params_t *args);

// Toggles very verbose debugging info from the network Network class
static bool netdebug = false;

struct sigaction  act1, act2;

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

// If set to a non zero value, this limits Cygnal to only one protocol
// at a time. This is for debugging only.
static int only_port = 0;

// These keep track of the number of active threads.
ThreadCounter tids;

map<int, Network *> networks;

// end of globals

static LogFile& dbglogfile = LogFile::getDefaultInstance();

// The rcfile is loaded and parsed here:
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

static Cache& cache = Cache::getDefaultInstance();

// This mutex is used to signify when all the threads are done.
static boost::condition	alldone;
static boost::mutex	alldone_mutex;

static boost::condition	noclients;
static boost::mutex	noclients_mutex;

static void
usage()
{
	cout << _("cygnal -- a streaming media server.") << endl
	<< endl
	<< _("Usage: cygnal [options...]") << endl
	<< _("  -h,  --help          Print this help and exit") << endl
	<< _("  -V,  --version       Print version information and exit") << endl
	<< _("  -v,  --verbose       Output verbose debug info") << endl
	<< _("  -m,  --multithread   Enable Multi Threading") << endl
	<< _("  -n,  --netdebug      Turn on net debugging messages") << endl
	<< _("  -o   --only-port     Only use port for debugging") << endl
	<< _("  -p   --port-offset   Port offset for debugging") << endl
        << _("  -t,  --testing       Turn on special Gnash testing support") << endl
	<< _("  -a,  --admin         Enable the administration thread") << endl
	<< _("  -r,  --root          Document root for all files") << endl
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
        { 'o', "only-port",     Arg_parser::yes },
        { 's', "singlethreaded", Arg_parser::no }
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
	  case 's':
	      crcfile.setThreadingFlag(false);
	      break;
	  case 'n':
	      netdebug = true;
	      break;
	  case 'o':	
	      only_port = parser.argument<int>(i);
	      break;
	  case 'd':
	      crcfile.dump();
	      exit(0);
	      break;
	  default:
	      log_error (_("Extraneous argument: %s"), parser.argument(i).c_str());
        }
    }

    // If a port is specified, we only want to run single threaded.
    if (only_port) {
	crcfile.setThreadingFlag(false);
    }
    
    // Trap ^C (SIGINT) so we can kill all the threads
    act1.sa_handler = cntrlc_handler;
    sigaction (SIGINT, &act1, NULL);
    act2.sa_handler = hup_handler;
    sigaction (SIGHUP, &act2, NULL);
//    sigaction (SIGPIPE, &act, NULL);

    boost::mutex::scoped_lock lk(alldone_mutex);
    
    // Admin handler    
    if (admin) {
	Network::thread_params_t admin_data;
	admin_data.port = gnash::ADMIN_PORT;
	boost::thread admin_thread(boost::bind(&admin_handler, &admin_data));
    }

//    Cvm cvm;
//    cvm.loadMovie("/tmp/out.swf");
    
    // Incomming connection handler for port 80, HTTP and
    // RTMPT. As port 80 requires root access, cygnal supports a
    // "port offset" for debugging and development of the
    // server. Since this port offset changes the constant to test
    // for which protocol, we pass the info to the start thread so
    // it knows which handler to invoke. 
    Network::thread_params_t *http_data = new Network::thread_params_t;
    if ((only_port == 0) || (only_port == gnash::RTMPT_PORT)) {
	http_data->port = port_offset + gnash::RTMPT_PORT;
	http_data->netfd = 0;
	http_data->filespec = docroot;
	if (crcfile.getThreadingFlag()) {
	    boost::thread http_thread(boost::bind(&connection_handler, http_data));
	} else {
	    connection_handler(http_data);
	}
    }
    
    
    Network::thread_params_t *rtmp_data = new Network::thread_params_t;
    // Incomming connection handler for port 1935, RTMP. As RTMP
    // is not a priviledged port, we just open it without an offset.
    if ((only_port == 0) || (only_port == gnash::RTMP_PORT)) {
	rtmp_data->port = port_offset + gnash::RTMP_PORT;
	rtmp_data->netfd = 0;
	rtmp_data->filespec = docroot;
	if (crcfile.getThreadingFlag()) {
	    boost::thread rtmp_thread(boost::bind(&connection_handler, rtmp_data));
	} else {
	    connection_handler(rtmp_data);
	}
    }
    
    // wait for the thread to finish
//     http_thread.join();
//     rtmp_thread.join();

      // Wait for all the threads to die
      alldone.wait(lk);
      
      log_debug (_("Cygnal done..."));
    
      delete rtmp_data;
      delete http_data;

      return(0);
}

// Trap Control-C (SIGINT) so we can cleanly exit
static void
cntrlc_handler (int sig)
{
    log_debug(_("Got a %d interrupt"), sig);
//    sigaction (SIGINT, &act, NULL);
    exit(-1);
}

// Trap SIGHUP so we can 
static void
hup_handler (int /* sig */)
{
    if (crcfile.getTestingFlag()) {
	cerr << "Testing, Testing, Testing..." << endl;
    }
	
}

static void
version_and_copyright()
{
    cout << "Cygnal " << VERSION << endl
        << endl
        << _("Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.\n"
        "Cygnal comes with NO WARRANTY, to the extent permitted by law.\n"
        "You may redistribute copies of Cygnal under the terms of the GNU General\n"
        "Public License V3. For more information, see the file named COPYING.\n")
    << endl;
}

// FIXME: this function could be tweaked for better performance
void
admin_handler(Network::thread_params_t *args)
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
connection_handler(Network::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;
    int fd = 0;
    Network net;
    bool done = false;
    static int tid = 0;
    
    if (netdebug) {
	net.toggleDebug(true);
    }
    // Start a server on this tcp/ip port.
    fd = net.createServer(args->port);
    if (fd <= 0) {
	log_error("Can't start Connection Handler for fd #%d, port %hd", fd, args->port);
	return;
    } else {
	log_debug("Starting Connection Handler for fd #%d, port %hd", fd, args->port);
    }

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
    log_debug("Spawn limit is: %d", spawn_limit);

//    Handler *hand = new Handler;

    args->handler = &net;
    boost::thread handler;
    
    // FIXME: this may run forever, we probably want a cleaner way to
    // test for the end of time.
    do {
	net.setPort(args->port);
	if (netdebug) {
	    net.toggleDebug(true);
	}

	// Rotate in a range of 0 to the limit.
	tid = (tid + 1) % (spawn_limit + 1);
	log_debug("thread ID %d for fd #%d", tid, fd);
	
	// Wait for a connection to this tcp/ip from a client. If set
	// to true, this will block until a request comes in. If set
	// to single threaded mode, this will only allow one client to
	// connect at a time. This is to make it easier to debug
	// things when you have a heavily threaded application.
	args->netfd = net.newConnection(true, fd);
	if (args->netfd <= 0) {
	    log_debug("No new network connections");
	    continue;
	}
	
	log_debug("New network connection for fd #%d", args->netfd);
    
	struct pollfd fds;
	fds.fd = args->netfd;
	fds.events = POLLIN | POLLRDHUP;
	if (crcfile.getThreadingFlag() == true) {
	    // Each dispatch thread gets it's own argument data and
	    // network connection data.
	    log_debug("Multi-threaded mode for server on fd #%d", fd);
	    Network::thread_params_t *targs = new Network::thread_params_t;
	    Network *tnet = 0;
	    targs->netfd = args->netfd;
	    // If we haven't spawned up to our max allowed, start a
	    // new dispatch thread to handle data.
	    if (networks[tid] == 0) {
		log_debug("Starting new dispatch thread for tid #%d", tid);
		tids.increment();
		tnet = new Network;
		tnet->setFileFd(args->netfd);
		targs->netfd = args->netfd;
		targs->handler = tnet;
		targs->filespec = docroot;
		targs->tid = tid;
	    } else {
		log_debug("Not starting new HTTP thread, spawned already for tid #%d", tid);
		tnet = networks[tid];
	    }
	    if (args->port == (port_offset + RTMPT_PORT)) {
		boost::bind(http_handler, targs);
		tnet->addPollFD(fds, http_handler);
	    } else if (args->port == (port_offset + RTMP_PORT)) {
		boost::bind(rtmp_handler, targs);
		tnet->addPollFD(fds, rtmp_handler);
	    }
	    if (networks[tid] == 0) {
		networks[tid] = tnet;
		boost::thread handler(boost::bind(&dispatch_handler, targs));
	    }
	} else {
	    // When in single threaded mode, just call the protocol
	    // handler directly. As this is primarily only used when
	    // debugging Cygnal itself, we don't want the extra
	    // overhead of the distpatch_handler.
	    log_debug("Single threaded mode for fd #%d", args->netfd);
	    if (args->port == (port_offset + RTMPT_PORT)) {
		http_handler(args);
	    } else if (args->port == (port_offset + RTMP_PORT)) {
		rtmp_handler(args);
	    }
	}
	
	log_debug("Number of active Threads is %d", tids.num_of_tids());
	
//	net.closeNet(args->netfd); 		// this shuts down this socket connection
	log_debug("Restarting loop for next connection for port %d...", args->port);
    } while(!done);
    
    // All threads should wake up now.
    alldone.notify_all();

} // end of connection_handler

void
dispatch_handler(Network::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;

//    Handler *hand = reinterpret_cast<Handler *>(args->handler);
    Network *net = reinterpret_cast<Network *>(args->handler);
//    Network net;
    int timeout = 5000;
    bool done = false;

    do {
	int limit = net->getPollFDSize();
	net->setTimeout(timeout);
	cerr << "LIMIT is: " << limit << endl;
	if (limit > 0) {
	    struct pollfd *fds = net->getPollFDPtr();
	    boost::shared_ptr< vector<struct pollfd> > hits;
	    try {
//                boost::shared_ptr< vector< int > > hits(net.waitForNetData(limit, fds));
                hits = net->waitForNetData(limit, fds);
		vector<struct pollfd>::iterator it;
		cerr << "Hits: " << hits->size() << endl;
		cerr << "Pollfds: " << net->getPollFDSize() << endl;
		for (it = hits->begin(); it != hits->end(); it++) {
		    // We got an error, which isn't always a crises, as some are normal
		    // if the client disconnects while we're talking to it.
		    if ((it->revents & POLLRDHUP) || (it->revents & POLLNVAL))  {
			log_debug("Revents has a POLLRDHUP or POLLNVAL set to %d for fd #%d",
				  it->revents, it->fd);
			net->erasePollFD(it->fd);
 			net->closeNet(it->fd);
//			continue;
			break;
		    } else {
			// We got some data, so process it
			log_debug("Got something on fd #%d, 0x%x", it->fd, it->revents);
			// Call the protocol handler for this network connection
			bool ret = net->getEntry(it->fd)(args);
			
//			log_debug("Handler returned %s", (ret) ? "true" : "false");
			// FIXME: we currently force a 'close connection' at the end
			// of sending a file, since apache does too. This pretty much
			// blows persistance,
//			if (ret) {
			    networks[args->tid] = 0;
			    net->closeNet(it->fd);
			    net->erasePollFD(it->fd);
//			}
		    }
		}
	    } catch (std::exception& e) {
		log_error("Network connection was dropped:  %s", e.what());
		vector<struct pollfd>::const_iterator it;
		if (hits) {
		    for (it = hits->begin(); it != hits->end(); it++) {
			log_debug("Need to disconnect fd #%d, it got an error.", (*it).fd);
		    }
		}
	    }
        } else {
	    log_debug("nothing to wait for...");
	    if (crcfile.getThreadingFlag()) {
		done = true;
	    }
        }
    } while (!done);
    tids.decrement();
    
} // end of dispatch_handler


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
