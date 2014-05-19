// cygnal.cpp:  GNU streaming Flash media server, for Gnash.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#include <sys/stat.h>
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

#include "GnashSleep.h"
#include "revno.h"

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


// classes internal to Gnash
#include "network.h"
#include "log.h"
#include "crc.h"
#include "proc.h"
#include "rtmp.h"
#include "buffer.h"
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
#include "URL.h"
#include "rtmp_client.h"

// classes internal to Cygnal
#include "rtmp_server.h"
#include "http_server.h"

#include "handler.h"
#include "cache.h"
#include "cygnal.h"

#ifdef ENABLE_NLS
# include <locale>
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

#ifndef POLLRDHUP
#define POLLRDHUP 0
#endif

//using gnash::log_network;
using namespace std;
using namespace gnash;
using namespace cygnal;

static void usage();
static void version_and_copyright();
static void cntrlc_handler(int sig);
static void hup_handler(int sig);

void connection_handler(Network::thread_params_t *args);
void event_handler(Network::thread_params_t *args);
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
static string docroot;

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

// This is the global object for Cygnl
// The debug log used by all the gnash libraries.
static Cygnal& cyg = Cygnal::getDefaultInstance();

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

// The user config for Cygnal is loaded and parsed here:
static CRcInitFile& crcfile = CRcInitFile::getDefaultInstance();

// Cache support for responses and files.
static Cache& cache = Cache::getDefaultInstance();

// The list of active cgis being executed.
//static std::map<std::string, Proc> procs; // = proc::getDefaultInstance();

// This mutex is used to signify when all the threads are done.
static boost::condition	alldone;
static boost::mutex	alldone_mutex;

static boost::condition	noclients;
static boost::mutex	noclients_mutex;

const char *proto_str[] = {
    "NONE",
    "HTTP",
    "HTTPS",
    "RTMP",
    "RTMPT",
    "RTMPTS",
    "RTMPE",
    "RTMPS",
    "DTN"
};

static void
usage()
{
	cout << _("cygnal -- a streaming media server.") << endl
	<< endl
	<< _("Usage: cygnal [options...]") << endl
	<< _("  -h,  --help          Print this help and exit") << endl
	<< _("  -V,  --version       Print version information and exit") << endl
	<< _("  -v,  --verbose       Output verbose debug info") << endl
	<< _("  -s,  --singlethread  Disable Multi Threading") << endl
	<< _("  -n,  --netdebug      Turn on net debugging messages") << endl
	<< _("  -o   --only-port     Only use port for debugging") << endl
	<< _("  -p   --port-offset   Port offset for debugging") << endl
        << _("  -t,  --testing       Turn on special Gnash testing support") << endl
	<< _("  -a,  --admin         Enable the administration thread") << endl
	<< _("  -r,  --root          Document root for all files") << endl
	<< _("  -m,  --machine       Hostname for this machine") << endl
	<< endl;
}


Cygnal&
Cygnal::getDefaultInstance()
{
//     GNASH_REPORT_FUNCTION;
    static Cygnal o;
    return o;
}


Cygnal::~Cygnal()
{
//     GNASH_REPORT_FUNCTION;
}

bool
Cygnal::loadPeersFile()
{
    // GNASH_REPORT_FUNCTION;

    loadPeersFile("./peers.conf");

    loadPeersFile("/etc/peers.conf");

    // Check the users home directory    
#ifndef __amigaos4__
    char *home = std::getenv("HOME");
#else
    //on AmigaOS we have a GNASH: assign that point to program dir
    char *home = "/gnash";
#endif

    string homefile = home;
    homefile += "/peers.conf";

    return loadPeersFile(homefile);
}

bool
Cygnal::loadPeersFile(const std::string &filespec)
{
//     GNASH_REPORT_FUNCTION;

    struct stat stats;
    std::ifstream in;
    std::string line;
    string host;
    string portstr;
    string cgi;
    vector<string> supported;
    
    // Make sufre the file exists
    if (stat(filespec.c_str(), &stats) != 0) {
        return false;
    }

    in.open(filespec.c_str());
    
    if (!in) {
	log_error(_(": couldn't open file: "), filespec);
	return false;
    }

    // Read in each line and parse it
    size_t lineno = 0;
    while (std::getline(in, line)) {

        ++lineno;

        // Ignore comment and empty lines
        if (line.empty() || line[0] == '#') {
	    continue;
	}

        std::istringstream ss(line);
        
        // Get the first token
        if (! (ss >> host)) {
            // Empty line 
            continue;
        }
        
        // 'action' should never be empty, or (ss >> action) 
        // above would have failed

        if (host[0] == '#') {
	    continue; // discard comments
	}

        // Get second token
        if (!(ss >> portstr)) {
            // Do we need to warn here as well?
            continue;
        }

        while (ss >> cgi) {
	    supported.push_back(cgi);
            continue;
        }

	// Create a new peer item
	std::shared_ptr<peer_t> peer(new Cygnal::peer_t);
	peer->hostname = host;
	peer->port = strtol(portstr.c_str(), NULL, 0) & 0xffff;

	_peers.push_back(peer);
    }    

    return true;
}

void
Cygnal::probePeers()
{
//     GNASH_REPORT_FUNCTION;
    
    probePeers(_peers);
}

void
Cygnal::probePeers(peer_t &peer)
{    
//     GNASH_REPORT_FUNCTION;
    RTMPClient net;
    stringstream uri;

    uri << peer.hostname;
    
    vector<string>::iterator it;
    for (it = peer.supported.begin(); it <= peer.supported.end(); ++it) {
	string tmp = uri.str();
//	tmp += (*it);
// 	log_network("Constructed: %s/%s", uri.str(), *it);
	
	gnash::URL url(uri.str());
	if (!(peer.fd = net.connectToServer(uri.str()))) {
	    log_network(_("Couldn't connect to %s"), uri.str());
	    peer.connected = false;
	} else {
	    peer.connected = true;
// 	    peer.fd = net.getFileFd();
	}
    }
}

void
Cygnal::probePeers(std::vector<std::shared_ptr<peer_t> > &peers)
{
//     GNASH_REPORT_FUNCTION;

// 	createClient();
    std::vector<std::shared_ptr<Cygnal::peer_t> >::iterator it;
    for (it = peers.begin(); it != peers.end(); ++it) {
	std::shared_ptr<Cygnal::peer_t> peer = *it;
	probePeers(*peer);
	if (peer->connected) {
	    log_network(_("%s is active on fd #%d."), peer->hostname,
			peer->fd);
 	    _active_peers.push_back(*it);
	}
    }
}

void
Cygnal::removeHandler(const std::string &path)
{
//     GNASH_REPORT_FUNCTION;
    map<std::string, std::shared_ptr<Handler> >::iterator it;
    it = _handlers.find(path);
    if (it != _handlers.end()) {
	boost::mutex::scoped_lock lock(_mutex);
	_handlers.erase(it);
    }
}

std::shared_ptr<Handler>
Cygnal::findHandler(const std::string &path)
{
//     GNASH_REPORT_FUNCTION;
    map<std::string, std::shared_ptr<Handler> >::iterator it;
    std::shared_ptr<Handler> hand;
    it = _handlers.find(path);
    if (it != _handlers.end()) {
	hand = (*it).second;
    }

    return hand;
}

void
Cygnal::dump()
{
    std::vector<std::shared_ptr<Cygnal::peer_t> >::iterator it;
    for (it = _peers.begin(); it != _peers.end(); ++it) {
	cerr << "Remote Peer: " << (*it)->hostname
	     << ":" << (*it)->port << endl;
    }
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

    // This becomes the default hostname, which becomes
    // 127.0.0.1 or ::1 for the localhost. The --machine
    // otion can change this.
    std::string hostname = "localhost.localdomain";
    
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
            { 's', "singlethreaded", Arg_parser::no },
            { 'm', "machine",       Arg_parser::yes }
        };
    
    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() ) {
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
        docroot = crcfile.getDocumentRoot();
    } else {
        docroot = "/var/www/html/software/tests/";
	crcfile.setDocumentRoot(docroot);
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
	      exit(EXIT_SUCCESS);
	  case 'V':
	      version_and_copyright();
	      exit(EXIT_SUCCESS);
	  case 't':
	      crcfile.setTestingFlag(true);
	      break;
	  case 'a':
	      admin = true;
	      break;
	  case 'v':
	      dbglogfile.setVerbosity();
	      LOG_ONCE(log_network(_("Verbose output turned on")))
	      break;
	  case 'p':
	      port_offset = parser.argument<int>(i);
	      crcfile.setPortOffset(port_offset);
	      break;
	  case 'r':
	      docroot = parser.argument(i);
	      break;
	  case 's':
	      crcfile.setThreadingFlag(false);
	      break;
	  case 'n':
	      netdebug = true;
	      dbglogfile.setNetwork(true);
	      break;
	  case 'o':	
	      only_port = parser.argument<int>(i);
	      break;
	  case 'd':
	      crcfile.dump();
	      exit(EXIT_SUCCESS);
	      break;
	  case 'm':
	      hostname = parser.argument(i);
	      break;
	  default:
	      log_error(_("Extraneous argument: %s"), parser.argument(i).c_str());
        }
    }
    
    log_network(_("Document Root for media files is: %s"), docroot);
    crcfile.setDocumentRoot(docroot);
    
    // load the file of peers. A peer is another instance of Cygnal we
    // can use for distributed processing.
    cyg.loadPeersFile();
    cyg.probePeers();
    
//    cyg.dump();
    
    // Trap ^C (SIGINT) so we can kill all the threads
    act1.sa_handler = cntrlc_handler;
    sigaction (SIGINT, &act1, NULL);
    act2.sa_handler = hup_handler;
    sigaction (SIGHUP, &act2, NULL);
//    sigaction (SIGPIPE, &act, NULL);

    // Lock a mutex the main() waits in before exiting. This is
    // because all the actually processing is done by other threads.
    boost::mutex::scoped_lock lk(alldone_mutex);
    
    // Start the Admin handler. This allows one to connect to Cygnal
    // at port 1111 and dump statistics to the terminal for tuning
    // purposes.
    if (admin) {
	Network::thread_params_t admin_data;
	admin_data.port = gnash::ADMIN_PORT;
	boost::thread admin_thread(boost::bind(&admin_handler, &admin_data));
    }

//    Cvm cvm;
//    cvm.loadMovie("/tmp/out.swf");
    
    // If a only-port is specified, we only want to run single
    // threaded. As all the rest of the code checks the config value
    // setting, this overrides that in the memory, but doesn't change
    // the file itself. This feature is really only for debugging,
    // where it's easier to work with one protocol at a time.
    if (only_port) {
	crcfile.setThreadingFlag(false);
    }

    // Incomming connection handler for port 80, HTTP and
    // RTMPT. As port 80 requires root access, cygnal supports a
    // "port offset" for debugging and development of the
    // server. Since this port offset changes the constant to test
    // for which protocol, we pass the info to the start thread so
    // it knows which handler to invoke. 
    Network::thread_params_t *http_data = new Network::thread_params_t;
    if ((only_port == 0) || (only_port == gnash::HTTP_PORT)) {
	http_data->tid = 0;
	http_data->netfd = 0;
	http_data->filespec = docroot;
	http_data->protocol = Network::HTTP;
	http_data->port = port_offset + gnash::HTTP_PORT;
        http_data->hostname = hostname;
	if (crcfile.getThreadingFlag()) {
	    boost::thread http_thread(boost::bind(&connection_handler, http_data));
	} else {
	    connection_handler(http_data);
	}
    }
    
    // Incomming connection handler for port 1935, RTMPT and
    // RTMPTE. This supports the same port offset as the HTTP handler,
    // just to keep things consistent.
    Network::thread_params_t *rtmp_data = new Network::thread_params_t;
    if ((only_port == 0) || (only_port == gnash::RTMP_PORT)) {
	rtmp_data->tid = 0;
	rtmp_data->netfd = 0;
	rtmp_data->filespec = docroot;
	rtmp_data->protocol = Network::RTMP;
	rtmp_data->port = port_offset + gnash::RTMP_PORT;
        rtmp_data->hostname = hostname;
	if (crcfile.getThreadingFlag()) {
	    boost::thread rtmp_thread(boost::bind(&connection_handler, rtmp_data));
	} else {
	    connection_handler(rtmp_data);
	}
    }
    
    // Wait for all the threads to die.
    alldone.wait(lk);
    
    log_network(_("Cygnal done..."));

    // Delete the data we allowcated to pass to each connection_handler.
    delete rtmp_data;
    delete http_data;
    
    return(0);
}

// Trap Control-C (SIGINT) so we can cleanly exit
static void
cntrlc_handler (int sig)
{
    log_network(_("Got a %d interrupt"), sig);
//    sigaction (SIGINT, &act, NULL);
    exit(EXIT_FAILURE);
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
    cout << "Cygnal: " << BRANCH_NICK << "_" << BRANCH_REVNO << endl
        << endl
        << _("Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.\n"
        "Cygnal comes with NO WARRANTY, to the extent permitted by law.\n"
        "You may redistribute copies of Cygnal under the terms of the GNU General\n"
        "Public License V3 or later. For more information, see the file named COPYING.\n")
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
    
    Network net;
    Handler::admin_cmd_e cmd = Handler::POLL;
    net.createServer(args->hostname, args->port);
    while (retries > 0) {
	log_network(_("Starting Admin Handler for port %d"), args->port);

	if (net.newConnection(true) <= 0) {
            return;
        }
        
	log_network(_("Got an incoming Admin request"));
	sleep(1);
	do {
	    Network::byte_t data[ADMINPKTSIZE+1];
	    memset(data, 0, ADMINPKTSIZE+1);
	    const char *ptr = reinterpret_cast<const char *>(data);
	    ret = net.readNet(data, ADMINPKTSIZE, 100);
	    if (ret < 0) {
		log_network(_("no more admin data, exiting...\n"));
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
		  }
#endif
	      }
	      break;
	      case Handler::POLL:
#ifdef USE_STATS_QUEUE
		  response << handlers.size() << " handlers are currently active." << "\r\n";
 		  for (hit = handlers.begin(); hit != handlers.end(); ++hit) {
		      int fd = hit->first;
 		      Handler *hand = hit->second;
		      struct timespec now;
		      clock_gettime (CLOCK_REALTIME, &now);
		      // Incoming que stats
 		      CQue::que_stats_t *stats = hand->statsin();
		      float diff = static_cast<float>(((now.tv_sec -
		      stats->start.tv_sec) + ((now.tv_nsec -
		      stats->start.tv_nsec)/1e9)));
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
		  }
#endif
		  break;
	      case Handler::INTERVAL:
		  net.writeNet("set interval\n");
		  break;
	      default:
		  break;
	    };
	} while (ret > 0);
        log_network(_("admin_handler: Done...!\n"));
	net.closeNet();		// this shuts down this socket connection
    }
    net.closeConnection();		// this shuts down the server on this connection

    // All threads should exit now.
    alldone.notify_all();
}

// A connection handler is started for each port the server needs to
// wait on for incoming connections. When it gets an incoming
// connection, it reads the first packet to get the resource name, and
// then starts the event handler thread if it's a newly requested
// resource, otherwise it loads a copy of the cached resource.
void
connection_handler(Network::thread_params_t *args)
{
    // GNASH_REPORT_FUNCTION;
    int fd = 0;
    Network net;
    bool done = false;
    static int tid = 0;
    
    if (netdebug) {
	net.toggleDebug(true);
    }
    // Start a server on this tcp/ip port.
    fd = net.createServer(args->hostname, args->port);
    if (fd <= 0) {
	log_error(_("Can't start %s Connection Handler for fd #%d, port %hd"),
		  proto_str[args->protocol], fd, args->port);
	return;
    } else {
	log_network(_("Starting %s Connection Handler for fd #%d, port %hd"),
		    proto_str[args->protocol], fd, args->port);
    }

    // Get the number of cpus in this system. For multicore
    // systems we'll get better load balancing if we keep all the
    // cpus busy. So a pool of threads is started for each cpu,
    // the default being just one. Each thread is reponsible for
    // handling part of the total active file descriptors.
#ifdef HAVE_SYSCONF
    long ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    LOG_ONCE(log_network(_("This system has %d cpus."), ncpus));
#endif	
    size_t nfds = crcfile.getFDThread();
    
//     log_network("This system is configured for %d file descriptors to be watched by each thread.", nfds);
    
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
    
    // FIXME: this may run forever, we probably want a cleaner way to
    // test for the end of time.
    do {
	net.setPort(args->port);
	if (netdebug) {
	    net.toggleDebug(true);
	}

	// Rotate in a range of 0 to the limit.
	tid = (tid + 1) % (spawn_limit + 1);
	// log_network("%s handler: thread ID #%d, fd #%d", proto_str[args->protocol], tid, fd);
	
	// Wait for a connection to this tcp/ip from a client. If set
	// to true, this will block until a request comes in. If set
	// to single threaded mode, this will only allow one client to
	// connect at a time. This is to make it easier to debug
	// things when you have a heavily threaded application.
	args->netfd = net.newConnection(true, fd);
	if (args->netfd <= 0) {
	    log_network(_("No new %s network connections"),
                        proto_str[args->protocol]);
	    return;
	} else {
	    log_network(_("*** New %s network connection for thread ID #%d, fd #%d ***"),
			proto_str[args->protocol], tid, args->netfd);
	}

	//
	// Setup HTTP handler
	//
	if (args->protocol == Network::HTTP) {
	    Network::thread_params_t *hargs = new Network::thread_params_t;
	    // std::copy(args, args+sizeof(Network::thread_params_t), &hargs);
	    hargs->protocol = args->protocol;
	    hargs->netfd = args->netfd;
#if 0
	    std::shared_ptr<Handler> hand = cyg.findHandler(path);
	    HTTPServer *http = new HTTPServer;
	    hargs.entry = http;
	    http->setDocRoot(crcfile.getDocumentRoot());
	    std::shared_ptr<cygnal::Buffer> buf(http->peekChunk());
	    http->processHeaderFields(*buf);
	    string hostname, path;
	    string::size_type pos = http->getField("host").find(":", 0);
	    if (pos != string::npos) {
		hostname += http->getField("host").substr(0, pos);
	    } else {
		hostname += "localhost.localdomain";
	    }
	    path = http->getFilespec();
	    string key = hostname + path;
#endif
	    string key;
	    Handler *hand = 0;
	    if (!hand) {
		hand = new Handler;
		hand->addClient(args->netfd, Network::HTTP);
		int retries = 10;
		cygnal::Buffer *buf = 0;
		do {
		    buf = hand->parseFirstRequest(args->netfd, Network::HTTP);
		    if (!buf) {
			retries--;
			continue;
		    } else {
			break;
		    }
		} while (retries);
		string &key = hand->getKey(args->netfd);
		log_network(_("Creating new %s Handler for %s using fd #%d"),
			    proto_str[hargs->protocol], key, hargs->netfd);
		hargs->handler = hand;
		hargs->buffer = buf;
		hargs->filespec = key;
		// cyg.addHandler(key, hand);
		
		// If in multi-threaded mode (the default), start a thread
		// with a connection_handler for each port we're interested
		// in. Each port of could have a different protocol.
		boost::bind(event_handler, hargs);
		if (crcfile.getThreadingFlag() == true) {
		    boost::thread event_thread(boost::bind(&event_handler, hargs));
		} else {
		    event_handler(hargs);
		    // We're done, close this network connection
		}
	    } else {
		log_network(_("Reusing %s Handler for %s using fd #%d"),
			    proto_str[hargs->protocol], key, hargs->netfd);
		hand->addClient(args->netfd, Network::HTTP);
	    }
	    // delete http;
	} // end of if HTTP
	
	//
	// Setup RTMP handler
	//
	if (args->protocol == Network::RTMP) {
	    Network::thread_params_t *rargs = new Network::thread_params_t;
	    rargs->protocol = args->protocol;
	    rargs->netfd = args->netfd;
	    RTMPServer *rtmp = new RTMPServer;
	    std::shared_ptr<cygnal::Element> tcurl =
		rtmp->processClientHandShake(args->netfd);
	    if (!tcurl) {
// 		    log_error("Couldn't read the tcUrl variable!");
		rtmp->closeNet(args->netfd);
		return;
	    }
	    URL url(tcurl->to_string());
	    string key = url.hostname() + url.path();
	    std::shared_ptr<Handler> hand = cyg.findHandler(url.path());
	    if (!hand) {
		log_network(_("Creating new %s Handler for: %s for fd %#d"),
			    proto_str[args->protocol], key, args->netfd);
		hand.reset(new Handler);
		cyg.addHandler(key, hand);
		rargs->entry = rtmp;
		hand->setNetConnection(rtmp->getNetConnection());
		std::vector<std::shared_ptr<Cygnal::peer_t> >::iterator it;
		std::vector<std::shared_ptr<Cygnal::peer_t> > active = cyg.getActive();
		for (it = active.begin(); it < active.end(); ++it) {
		    Cygnal::peer_t *peer = (*it).get();
		    hand->addRemote(peer->fd);
		}
		hand->addClient(args->netfd, Network::RTMP);
		rargs->handler = reinterpret_cast<void *>(hand.get());
		args->filespec = key;
		args->entry = rtmp;
		
		string cgiroot;
		char *env = std::getenv("CYGNAL_PLUGINS");
		if (env != 0) {
		    cgiroot = env;
		}
		if (crcfile.getCgiRoot().size() > 0) {
		    cgiroot += ":" + crcfile.getCgiRoot();
		    log_network(_("Cygnal Plugin paths are: %s"), cgiroot);
		} else {
		    cgiroot = PLUGINSDIR;
		}
		hand->scanDir(cgiroot);
		std::shared_ptr<Handler::cygnal_init_t> init =
		    hand->initModule(url.path());
		
		// this is where the real work gets done.
		if (init) {
		    // If in multi-threaded mode (the default), start a thread
		    // with a connection_handler for each port we're interested
		    // in. Each port of course has a different protocol.
		    if (crcfile.getThreadingFlag() == true) {
			boost::thread event_thread(boost::bind(&event_handler, args));
		    } else {
			event_handler(args);
			// We're done, close this network connection
			net.closeNet(args->netfd);
		    }
		} else {
		    log_error(_("Couldn't load plugin for %s"), key); 
		}
		
		// // We're done, close this network connection
		// if (crcfile.getThreadingFlag() == true) {
		//     net.closeNet(args->netfd);
		// }
	    }
	    // delete rtmp;
	} // end of if RTMP	
	
	log_network(_("Number of active Threads is %d"), tids.num_of_tids());
	
//	net.closeNet(args->netfd); 		// this shuts down this socket connection
	log_network(_("Restarting loop for next connection for port %d..."),
                    args->port);
    } while(!done);

    // All threads should wake up now.
    alldone.notify_all();
    
} // end of connection_handler

void
event_handler(Network::thread_params_t *args)
{
    GNASH_REPORT_FUNCTION;

    Network::thread_params_t largs;
    // std::copy(args, args+sizeof(Network::thread_params_t), &largs);    
    Handler *hand = reinterpret_cast<Handler *>(args->handler);

    largs.protocol = args->protocol;
    largs.netfd = args->netfd;
    largs.port = args->port;
    largs.buffer = args->buffer;
    largs.entry = args->entry;
    largs.filespec = args->filespec;

    Network net;
    int timeout = 30;
    int retries = 0;
    bool done = false;

    fd_set hits;
    FD_ZERO(&hits);
    FD_SET(args->netfd, &hits);

    tids.increment();
    
    // We need to calculate the highest numbered file descriptor
    // for select. We may want to do this elsewhere, as it could
    // be a performance hit as the number of file descriptors gets
    // larger.
    log_debug("Handler has %d clients attached, %d threads",
	      hand->getClients().size(), tids.num_of_tids());
    
    int max = 0;
    for (size_t i = 0; i<hand->getClients().size(); i++) {
	log_debug("Handler client[%d] is: %d", i, hand->getClient(i));
	if (hand->getClient(i) >= max) {
	    max = hand->getClient(i);
	    // hand->dump();
	}
    }

    do {
	
	// If we have active disk streams, send those packets first.
	// 0 is a reserved stream, so we start with 1, as the reserved
	// stream isn't one we care about here.
	if (hand->getActiveDiskStreams()) {
	    log_network(_("%d active disk streams"),
			hand->getActiveDiskStreams());
	    // hand->dump();
	}
#if 0
	std::shared_ptr<DiskStream> filestream(cache.findFile(args->filespec));
	if (filestream) {
	    filestream->dump();
	}
// #else
//      	cache.dump();
#endif
	//hand->dump();
	std::shared_ptr<DiskStream> ds;
	for (int i=1; i <= hand->getActiveDiskStreams(); i++) {
	    ds = hand->getDiskStream(i);
	    if (ds) {
   		//ds->dump();
		// Only play the next chunk of the file.
//log_network("Sending following chunk of %s", ds->getFilespec());
		if (ds->play(i, false)) {
		    if (ds->getState() == DiskStream::CLOSED) {
			net.closeNet(args->netfd);
			hand->removeClient(args->netfd);
			done = true;
		    }
		} else {
		    // something went wrong, the stream failed
		    net.closeNet(args->netfd);
		    hand->removeClient(args->netfd);
		    done = true;
		}
	    }
	}
    
	// See if we have any data waiting behind any of the file
	// descriptors.
	for (int i=0; i <= max + 1; i++) {
	    if (FD_ISSET(i, &hits)) {
		FD_CLR(i, &hits);
		log_network(_("Got a hit for fd #%d, protocol %s"), i,
			    proto_str[hand->getProtocol(i)]);
		switch (hand->getProtocol(i)) {
		  case Network::NONE:
		      log_error(_("No protocol specified!"));
		      break;
		  case Network::HTTP:
		  {
		      largs.netfd = i;
		      // largs.filespec = fullpath;
		      std::shared_ptr<HTTPServer> &http = hand->getHTTPHandler(i);
		      if (!http->http_handler(hand, args->netfd, args->buffer)) {
			  log_network(_("Done with HTTP connection for fd #%d, CGI %s"), i, args->filespec);
			  net.closeNet(args->netfd);
			  hand->removeClient(args->netfd);
			  done = true;
		      } else {
			  log_network(_("Not Done with HTTP connection for fd #%d, it's a persistent connection."), i);
			  
		      }
		      continue;
		  }
		  case Network::RTMP:
		      args->netfd = i;
		      // args->filespec = path;
		      if (!rtmp_handler(args)) {
			  log_network(_("Done with RTMP connection for fd #%d, CGI "), i, args->filespec);
			  done = true;
		      }
		      break;
		  case Network::RTMPT:
		  {
		      net.setTimeout(timeout);
		      args->netfd = i;
		      std::shared_ptr<HTTPServer> &http = hand->getHTTPHandler(i);
		      // args->filespec = path;
		      if (!http->http_handler(hand, args->netfd, args->buffer)) {
			  log_network(_("Done with HTTP connection for fd #%d, CGI %s"), i, largs.filespec);
			  return;
		      }		      
		      break;
		  }
		  case Network::RTMPTS:
		  {
		      args->netfd = i;
		      // args->filespec = path;
		      std::shared_ptr<HTTPServer> &http = hand->getHTTPHandler(i);
		      if (!http->http_handler(hand, args->netfd, args->buffer)) {
			  log_network(_("Done with HTTP connection for fd #%d, CGI %s"), i, args->filespec);
			  return;
		      }		      
		      break;
		  }
		  case Network::RTMPE:
		      break;
		  case Network::RTMPS:
		      break;
		  case Network::DTN:
		      break;
		  default:
		      log_error(_("Unsupported network protocol for fd #%d, %d"),
				largs.netfd, hand->getProtocol(i));
		      done = true;
		      break;
		}
//		delete args->buffer;
	    }
	}

	// // Clear the current message so next time we read new data
	// args->buffer->clear();
	// largs.buffer->clear();
	
	// Wait for something from one of the file descriptors. This timeout
	// is the time between sending packets to the client when there is
	// no client input, which effects the streaming speed of big files.
	net.setTimeout(5);
	hits = net.waitForNetData(hand->getClients());
	if (FD_ISSET(0, &hits)) {
	    FD_CLR(0, &hits);
	    log_network(_("Got no hits, %d retries"), retries);
	    // net.closeNet(args->netfd);
	    // hand->removeClient(args->netfd);
	    // done = true;
	}
	retries++;
#if 0
	if (retries >= 10) {
	    net.closeNet(args->netfd);
	    hand->removeClient(args->netfd);
	    done = true;
	}
#endif
    } while (!done);

    tids.decrement();
	
} // end of event_handler

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
