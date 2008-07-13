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

#include <string>
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

// classes internal to Gnash
#include "network.h"
#include "rc.h"
#include "amf.h"
#include "log.h"
#include "rtmp.h"
#include "rtmp_msg.h"
#include "rtmp_client.h"
#include "http.h"
#include "limits.h"
#include "netstats.h"
#include "statistics.h"
#include "gmemory.h"
#include "arg_parser.h"

// classes internal to Cygnal
#include "buffer.h"
#include "handler.h"

std::vector<std::string> infiles;

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
static bool dump = false;

static struct sigaction  act;

// The next few global variables have to be global because Boost
// threads don't take arguments. Since these are set in main() before
// any of the threads are started, and it's value should never change,
// it's safe to use these without a mutex, as all threads share the
// same read-only value.

// end of globals

// These next two functions are borrowed from Libgloss, part of the GNU binutils,
// of which I am the primary author and copyright holder.
// convert an ascii hex digit to a number.
//      param is hex digit.
//      returns a decimal digit.
Network::byte_t
hex2digit (Network::byte_t digit)
{  
    if (digit == 0)
        return 0;
    
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    
    // shouldn't ever get this far
    return -1;
}

// Convert the hex array pointed to by buf into binary to be placed in mem
Buffer *
hex2mem(const char *str)
{
    size_t count = strlen(str);
    Network::byte_t ch = 0;
    Buffer *buf = new Buffer((count/3)+1);
    buf->clear();

    Network::byte_t *ptr = const_cast<Network::byte_t *>(reinterpret_cast<const Network::byte_t *>(str));
    
    for (size_t i=0; i<count; i++) {
        if (*ptr == ' ') {      // skip spaces.
            ptr++;
            continue;
        }
        ch = hex2digit(*ptr++) << 4;
        ch |= hex2digit(*ptr++);
        buf->append(ch);
    }
    return buf;
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
        { 'v', "verbose",       Arg_parser::no  },
        { 'd', "dump",          Arg_parser::no  },
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
    dbglogfile.setLogFilename(rcfile.getDebugLog());
    
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
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
// 	  default:
// 	      log_error (_("Extraneous argument: %s"), parser.argument(i).c_str());
        }
    }

    if (infiles.empty()) {
	cerr << _("Error: no input file was specified. Exiting.") << endl;
	usage();
	return EXIT_FAILURE;
    }

    string filespec;
    string protocol;
    string host;
    short port = 0;
    filespec = infiles.front();
    cout << "Will use \"" << filespec << "\" as the URL to fetch." << endl;
    
    // Trap ^C (SIGINT) so we can kill all the threads
    act.sa_handler = cntrlc_handler;
    sigaction (SIGINT, &act, NULL);

    Network net;

    // Take a standard URL apart

    // First extract the protocol
    string::size_type start, end, pos;
    pos = filespec.find(':', 0);
    if (pos != string::npos) {
        protocol = filespec.substr(0, pos);
        start = pos + 3;
    } else {
        protocol = "rtmp";
        start = 0;
    }

    // Then get the host name
    end = filespec.find('/', start); // get past the :// part
    if (end != string::npos) {
        host = filespec.substr(start, end-start);
    } else {
        host = "localhost";
    }

    start = end + 1;
    // Extract the port number if there is one
    pos = host.find(':', 0); // get past the / character
    if (pos != string::npos) {
        string portstr = host.substr(pos + 1, host.size()-pos);
        port = strtol(portstr.c_str(), NULL, 0);
        host = host.substr(0, pos);
    } else {
        if (protocol == "http") {
            port = RTMPT_PORT;
        }
        if (protocol == "rtmp") {
            port = RTMP_PORT;
        }
        if (protocol == "rtmpt") {
            port = RTMPT_PORT;
        }
    }

    if (end == string::npos) {
	cerr << "You need to supply a filename to grab" << endl;
	exit(1);
    }
    
    string filename = filespec.substr(end, filespec.size());
    if (netdebug) {
        cerr << "Protocol is " << protocol << endl;
        cerr << "Host is " << host << endl;
        cerr << "Port is " << port << endl;
        cerr << "File is " << filename << endl;
    }
    net.toggleDebug(netdebug);
        
    net.createClient(host, port);

    // Make a buffer to hold the handshake data.
    Buffer buf(1537);
    const char *handshake = "00 00 00 14 a1 00 00 00 00 96 00 8f 00 5c 00 0d 00 f2 00 9b 00 d8 00 b9 00 8e 00 e7 00 94 00 a5 00 6a 00 73 00 90 00 d1 00 86 00 3f 00 cc 00 3d 00 e2 00 4b 00 48 00 e9 00 7e 00 97 00 04 00 d5 00 5a 00 23 00 00 00 01 00 76 00 ef 00 3c 00 6d 00 d2 00 fb 00 b8 00 19 00 6e 00 47 00 74 00 05 00 4a 00 d3 00 70 00 31 00 66 00 9f 00 ac 00 9d 00 c2 00 ab 00 28 00 49 00 5e 00 f7 00 e4 00 35 00 3a 00 83 00 e0 00 61 00 56 00 4f 00 1c 00 cd 00 b2 00 5b 00 98 00 79 00 4e 00 a7 00 54 00 65 00 2a 00 33 00 50 00 91 00 46 00 ff 00 8c 00 fd 00 a2 00 0b 00 08 00 a9 00 3e 00 57 00 c4 00 95 00 1a 00 e3 00 c0 00 c1 00 36 00 af 00 fc 00 2d 00 92 00 bb 00 78 00 d9 00 2e 00 07 00 34 00 c5 00 0a 00 93 00 30 00 f1 00 26 00 5f 00 6c 00 5d 00 82 00 6b 00 e8 00 09 00 1e 00 b7 00 a4 00 f5 00 fa 00 43 00 a0 00 21 00 16 00 0f 00 dc 00 8d 00 72 00 1b 00 58 00 39 00 0e 00 67 00 14 00 25 00 ea 00 f3 00 10 00 51 00 06 00 bf 00 4c 00 bd 00 62 00 cb 00 c8 00 69 00 fe 00 17 00 84 00 55 00 da 00 a3 00 80 00 81 00 f6 00 6f 00 bc 00 ed 00 52 00 7b 00 38 00 99 00 ee 00 c7 00 f4 00 85 00 ca 00 53 00 f0 00 b1 00 e6 00 1f 00 2c 00 1d 00 42 00 2b 00 a8 00 c9 00 de 00 77 00 64 00 b5 00 ba 00 03 00 60 00 e1 00 d6 00 cf 00 9c 00 4d 00 32 00 db 00 18 00 f9 00 ce 00 27 00 d4 00 e5 00 aa 00 b3 00 d0 00 11 00 c6 00 7f 00 0c 00 7d 00 22 00 8b 00 88 00 29 00 be 00 d7 00 44 00 15 00 9a 00 63 00 40 00 41 00 b6 00 2f 00 7c 00 ad 00 12 00 3b 00 f8 00 59 00 ae 00 87 00 b4 00 45 00 8a 00 13 00 b0 00 71 00 a6 00 df 00 ec 00 dd 00 02 00 eb 00 68 00 89 00 9e 00 37 00 24 00 75 00 7a 00 c3 00 20 00 a1 00 96 00 8f 00 5c 00 0d 00 f2 00 9b 00 d8 00 b9 00 8e 00 e7 00 94 00 a5 00 6a 00 73 00 90 00 d1 00 86 00 3f 00 cc 00 3d 00 e2 00 4b 00 48 00 e9 00 7e 00 97 00 04 00 d5 00 5a 00 23 00 00 00 01 00 76 00 ef 00 3c 00 6d 00 d2 00 fb 00 b8 00 19 00 6e 00 47 00 74 00 05 00 4a 00 d3 00 70 00 31 00 66 00 9f 00 ac 00 9d 00 c2 00 ab 00 28 00 49 00 5e 00 f7 00 e4 00 35 00 3a 00 83 00 e0 00 61 00 56 00 4f 00 1c 00 cd 00 b2 00 5b 00 98 00 79 00 4e 00 a7 00 54 00 65 00 2a 00 33 00 50 00 91 00 46 00 ff 00 8c 00 fd 00 a2 00 0b 00 08 00 a9 00 3e 00 57 00 c4 00 95 00 1a 00 e3 00 c0 00 c1 00 36 00 af 00 fc 00 2d 00 92 00 bb 00 78 00 d9 00 2e 00 07 00 34 00 c5 00 0a 00 93 00 30 00 f1 00 26 00 5f 00 6c 00 5d 00 82 00 6b 00 e8 00 09 00 1e 00 b7 00 a4 00 f5 00 fa 00 43 00 a0 00 21 00 16 00 0f 00 dc 00 8d 00 72 00 1b 00 58 00 39 00 0e 00 67 00 14 00 25 00 ea 00 f3 00 10 00 51 00 06 00 bf 00 4c 00 bd 00 62 00 cb 00 c8 00 69 00 fe 00 17 00 84 00 55 00 da 00 a3 00 80 00 81 00 f6 00 6f 00 bc 00 ed 00 52 00 7b 00 38 00 99 00 ee 00 c7 00 f4 00 85 00 ca 00 53 00 f0 00 b1 00 e6 00 1f 00 2c 00 1d 00 42 00 2b 00 a8 00 c9 00 de 00 77 00 64 00 b5 00 ba 00 03 00 60 00 e1 00 d6 00 cf 00 9c 00 4d 00 32 00 db 00 18 00 f9 00 ce 00 27 00 d4 00 e5 00 aa 00 b3 00 d0 00 11 00 c6 00 7f 00 0c e5 7d 00 a2 00 0b 00 08 00 a9 00 3e 00 57 00 c4 00 95 00 1a 00 e3 00 c0 00 c1 00 36 00 af 00 fc 00 2d 00 92 00 bb 00 78 00 d9 00 2e 00 07 00 34 00 c5 00 0a 00 93 00 30 00 f1 00 26 00 5f 00 6c 00 5d 00 82 00 6b 00 e8 00 09 00 1e 00 b7 00 a4 00 f5 00 fa 00 43 00 a0 00 21 00 16 00 0f 00 dc 00 8d 00 72 00 1b 00 58 00 39 00 0e 00 67 00 14 00 25 00 ea 00 f3 00 10 00 51 00 06 00 bf 00 4c 00 bd 00 62 00 cb 00 c8 00 69 00 fe 00 17 00 84 00 55 00 da 00 a3 00 80 00 81 00 f6 00 6f 00 bc 00 ed 00 52 00 7b 00 38 00 99 00 ee 00 c7 00 f4 00 85 00 ca 00 53 00 f0 00 b1 00 e6 00 1f 00 2c 00 1d 00 42 00 2b 00 a8 00 c9 00 de 00 77 00 64 00 b5 00 ba 00 03 00 60 00 e1 00 d6 00 cf 00 9c 00 4d d00 32 00 db 00 18 00 f9 00 ce 00 27 00 d4 00 e5 00 aa 00 b3 00 d0 00 11 00 c6 00 7f 00 0c e5 7d 39 22 59 8b 3b 88 54 29 4f be a8 d7 29 44 02 15 00 9a 09 63 39 40 11 41 4f b6 a8 2f fd 7c c0 ad 3a 12 78 3b fb f8 ac 59 0a ae e0 87 0a b4 bd 45 39 8a 00 13 00 b0 00 71 00 a6 00 df 00 ec 00 dd 00 02 00 eb fb 68 a4 89 3a 9e 00 37 00 24 00 75 00 7a ac c3 0a 20 c8 a1 0a 96 74 8f ec 5c 0e 0d 00 f2 38 9b 3a d8 a0 b9 3a 8e 74 e7 ec 94 e5 a5 39 6a 00 73 00 90 00 d1 00 86 01 3f 00 cc 96 3d 00 e2 a8 4b 29 48 00 e9 fb 7e fd 97 4f 04 f8 d5 3b 5a 38 23 4f 00 01 01 00 76 c0 ef 3a 3c b8 6d 29 d2 bc fb 0a b8 d8 19 0a 6e e0 47 39 74 38 05 4f 4a bc d3 0a 70 ac 31 3a 66 07 9f 00 ac e5 9d 39 c2 e1 ab 3b 28 9e 49 4f 5e 88 f7 29 e4 0e 35 00 3a 09 83 39 e0 4e 61 4f 56 a8 4f fd 1c c0 cd 3a b2 78 5b fb 98 6c 79 0a 4e 71 a7 40 54 bd 65 39 2a a8 33 29 50 00 91 fb 46 0f ff 4f 8c 00 fd 0a a2 00 0b fb 08 a4 a9 3a 3e c0 57 3a c4 b8 95 29 1a bb e3 0a c0 88 c1 0a 36 ed af ab fc 50 2d 4f 92 38 bb 3a 78 d4 d9 3a 2e 99 07 00 34 a8 c5 29 0a 00 93 00 30 00 f1 00 26 01 5f 00 6c 23 5d 00 82 88 6b 29 e8 00 09 fb 1e 4a b7 4f a4 f8 f5 3b fa 68 43 4f a0 01 21 00 16 c0 0f 3a dc b8 8d 29 72 7c 1b 0a 58 30 39 2e 0e e0 67 39 14 13 25 00 ea 7c f3 0a 10 ac 51 3a 06 20 bf 40 4c 90 bd 00 62 01 cb 00 c8 00 69 00 fe f4 17 4f 84 20 55 4f da 90 a3 00 80 38 81 2e f6 e7 6f 41 bc 38 ed 2e 52 00 7b 00 38 94 99 00 ee f1 c7 02 f4 10 85 35 ca 90 53 00 f0 c0 b1 3a e6 10 1f 00 2c 90 1d 6a 42 03 2b 00 a8 61 c9 39 de 50 77 6a 64 00 b5 00 ba 38 03 00 60 40 e1 2e d6 48 cf fb 9c 00 4d 00 32 01 db 00 18 6c f9 fb ce 03 27 00 d4 90 e5 6a aa f4 b3 38 d0 00 11 00 c6 90 7f 6a 0c f4 7d 38 22 0b 8b 37 88 61 29 37 be c0 d7 6a 44 00 15 3d 9a 90 63 6a 40 ac 41 00 b6 9c 2f 0a 7c d8 ad 6a 12 7f 3b 00 f8 00 59 00 ae 90 87 6a b4 c0 45 6a 8a f8 13 0a b0 03 71 00 a6 00 df ca ec 00 dd 00 02 f4 eb 4f 68 90 89 03 9e c4 37 99 24 98 75 0a";
    Buffer *hand = hex2mem(handshake);

    // Make the NetConnection object
    Buffer *var;

    Element *connect = new Element;
    connect->makeString("connect");

    Element *connum = new Element;
//     const char *connumStr = "00 00 00 00 00 00 f0 3f";
//     Buffer *connumBuf = hex2mem(connumStr);
    connum->makeNumber(0x1);
    
    // Make the top level object
    Element obj;
    obj.makeObject();
    
    Element *app = new Element;
    app->makeString("app", "oflaDemo");
    obj.addProperty(app);
    
    Element *flashVer = new Element;
    flashVer->makeString("flashVer", "LNX 9,0,31,0");
    obj.addProperty(flashVer);
    
    Element *swfUrl = new Element;
    swfUrl->makeString("swfUrl", "http://192.168.1.70/software/gnash/tests/ofla_demo.swf");
    obj.addProperty(swfUrl);

    filespec = "rtmp://localhost/oflaDemo";
    Element *tcUrl = new Element;
    tcUrl->makeString("tcUrl", filespec);
    obj.addProperty(tcUrl);

    Element *fpad = new Element;
    fpad->makeBoolean("fpad", false);
    obj.addProperty(fpad);

    double dub = 615;
    Element *audioCodecs = new Element;
//     const char *audioCodecsStr = "00 00 00 00 00 38 83 40"; // 0x267 (615)
//     Buffer *audioCodecsBuf = hex2mem(audioCodecsStr);
//    audioCodecs->makeNumber("audioCodecs", audioCodecsBuf->reference());
    audioCodecs->makeNumber("audioCodecs", 615);
    obj.addProperty(audioCodecs);
//     printf("FIXME1: %g\n", audioCodecs->to_number());
//     cerr << hexify(audioCodecs->getBuffer()->begin(), 8, false) << endl;
//    audioCodecs->dump();
    
    Element *videoCodecs = new Element;
//     const char *videoCodecsStr = "00 00 00 00 00 00 5f 40"; // 0x7c (124)
//     Buffer *videoCodecsBuf = hex2mem(videoCodecsStr);
    videoCodecs->makeNumber("videoCodecs", 124);
    obj.addProperty(videoCodecs);
//     printf("FIXME2: %g\n", videoCodecs->to_number());
//     cerr << hexify(videoCodecs->getBuffer()->begin(), 8, false) << endl;
//    videoCodecs->dump();

    Element *videoFunction = new Element;
//     const char *videoFunctionStr = "00 00 00 00 00 00 f0 3f"; // (1)
//     Buffer *videoFunctionBuf = hex2mem(videoFunctionStr);
//    videoFunction->makeNumber("videoFunction", videoFunctionBuf->reference());
    videoFunction->makeNumber("videoFunction", 0x1);
    obj.addProperty(videoFunction);
//     printf("FIXME3: %g\n", videoFunction->to_number());
//     cerr << hexify(videoFunction->getBuffer()->begin(), 8, false) << endl;

    Element *pageUrl = new Element;
    pageUrl->makeString("pageUrl", "http://x86-ubuntu/software/gnash/tests/");
    obj.addProperty(pageUrl);

    AMF amf_obj;
    RTMPClient rtmp;
    
    // DEBUG: The Header size is: 12
    // DEBUG: The AMF index is: 0x3
    // DEBUG: The AMF channel index is 3
    // DEBUG: The header size is 12
    // DEBUG: The mystery word is: 0
    // DEBUG: The body size is: 227
    // DEBUG: The type is: Invoke
    // DEBUG: The source/destination is: 0
    size_t total_size = 227;
    Buffer *out = rtmp.encodeHeader(0x3, RTMP::HEADER_12, total_size,
                                                 RTMP::INVOKE, RTMPMsg::FROM_CLIENT);
    cerr << hexify(out->begin(), 12, false) << endl;
    const char *rtmpStr = "03 00 00 04 00 01 1f 14 00 00 00 00";
    Buffer *rtmpBuf = hex2mem(rtmpStr);
    Buffer *conobj = connect->encode();
    Buffer *numobj = connum->encode();
    Buffer *encobj = obj.encode();    

    // All RTMP connections start with a 0x3
    *(hand->begin()) = 0x3;
    // Write the initial HELO packet to the server
//    net.writeNet(buf.begin(), 1);
    net.writeNet(hand->begin(), 1537);

    // Read the response
    Buffer buf1(3073);
    int ret = net.readNet(buf1.begin(), buf1.size());
    cerr << "read initial response: " << ret << endl;
    
    net.writeNet(buf1.begin() + 1537, 1536);

    // Write the response, which is the packet we just got duplicated twice.
//    ret = net.writeNet(buf1.begin(), 1536);
//    ret = net.writeNet(buf1.begin(), 1536);
    
    Buffer final(12);
    final.copy(rtmpBuf->reference(), 12);
// #if 0				// this works
//     const char *header = "03 00 00 01 00 01 23 14 00 00 00 00";
//     const char *body = "02 00 07 63 6f 6e 6e 65 63 74 00 3f f0 00 00 00 00 00 00 03 00 03 61 70 70 02 00 08 6f 66 6c 61 44 65 6d 6f 00 08 66 6c 61 73 68 56 65 72 02 00 0c 4c 4e 58 20 39 2c 30 2c 33 31 2c 30 00 06 73 77 66 55 72 6c 02 00 36 68 74 74 70 3a 2f 2f 31 39 32 2e 31 36 38 2e 31 2e 37 30 2f 73 6f 66 74 77 61 72 65 2f 67 6e 61 73 68 2f 74 65 73 74 73 2f 6f 66 6c 61 5f 64 65 6d 6f 2e 73 77 66 00 05 74 63 55 72 6c 02 00 19 72 74 6d 70 3a 2f 2f 6c 6f 63 61 6c 68 6f 73 74 2f 6f 66 6c 61 44 65 6d 6f 00 04 66 70 61 64 01 00 00 0b 61 75 64 69 6f 43 6f 64 65 63 73 00 40 83 38 00 00 00 00 00 00 0b 76 69 64 65 6f 43 6f 64 65 63 73 00 40 5f 00 00 00 00 00 00 00 0d 76 69 64 65 6f 46 75 6e 63 74 69 6f 6e 00 3f f0 00 00 00 00 00 00 00 07 70 61 67 65 55 72 6c 02 00 27 68 74 74 70 3a 2f 2f 78 38 36 2d 75 62 75 6e 74 75 2f 73 6f 66 74 77 61 72 65 2f 67 6e 61 73 68 2f 74 65 73 74 73 2f 00 00 09";
//     Buffer *header1 = hex2mem(header);
//     Buffer *body1 = hex2mem(body);
    
//     size_t totalsize = 0;
//     net.writeNet(header1->begin(), 12);
//     totalsize = 0;
//     while (totalsize <= body1->size()) {
// 	size_t len = (RTMP_VIDEO_PACKET_SIZE < totalsize) ?( body1->size() - totalsize) : RTMP_VIDEO_PACKET_SIZE;
// 	net.writeNet(body1->begin() + totalsize, len);
// 	if (netdebug) {
// 	    cerr << hexify(body1->begin() + totalsize, len, false) << endl;
// 	    cerr << hexify(body1->begin() + totalsize, len, true) << endl;
// 	}
// 	Network::byte_t mark = 0xc3;
// 	totalsize += 128;
// 	if (len == RTMP_VIDEO_PACKET_SIZE) {
// 	    net.writeNet(&mark, 1);
// 	}
//     }
// #else
//     const char *header = "03 00 00 01 00 01 23 14 00 00 00 00";
//     Buffer *header1 = hex2mem(header);
    Buffer *body1 = rtmp.encodeConnect("oflaDemo", "http://192.168.1.70/software/gnash/tests/ofla_demo.swf",
		       "rtmp://localhost/oflaDemo", 615, 124, 1,
		       "http://x86-ubuntu/software/gnash/tests/");
//     final.append(conobj);
//     final.append(numobj);
//     final.append(encobj);
    Buffer *header1 = rtmp.encodeHeader(0x3, RTMP::HEADER_12, body1->size() - 28,
					RTMP::INVOKE, RTMPMsg::FROM_CLIENT);
    size_t totalsize = 0;
    net.writeNet(header1->begin(), 12);
    totalsize = 0;
    while (totalsize <= body1->size()) {
	size_t len = (RTMP_VIDEO_PACKET_SIZE < totalsize) ?( body1->size() - totalsize) : RTMP_VIDEO_PACKET_SIZE;
	net.writeNet(body1->begin() + totalsize, len);
	if (netdebug) {
	    cerr << hexify(body1->begin() + totalsize, len, true) << endl;
	}
	Network::byte_t mark = 0xc3;
	totalsize += 128;
	if (len == RTMP_VIDEO_PACKET_SIZE) {
	    net.writeNet(&mark, 1);
	}
    }
//#endif

#if 1
    // Write the packet to disk so we can anaylze it with other tools
    int fd = open("outbuf.raw",O_WRONLY|O_CREAT, S_IRWXU);
    if (fd == -1) {
        perror("open");
    }
    cout << "Writing packet to disk: \"outbuf.raw\"" << endl;
    write(fd, header1->begin(), 12);
     write(fd, body1->begin(), body1->size());
//     write(fd, numobj->begin(), numobj->size());    
//     write(fd, encobj->begin(), encobj->size());    
//    write(fd, final.begin(), final.size());    
    close(fd);
#endif

    // This first packet appears to be a Ping packet to the client
    // from the server. 04 is a Ping message type, 06 is for the client.
    // This is used to set the timestamp so the client and server are
    // synchronized.
    // 02 00 00 00 00 00 06 04 00 00 00 00 00 00 00 00 00 00
    Buffer buf2;
    cerr  << "Waiting for data stream..." << endl;
    ret = net.readNet(buf2.begin(), buf2.size());
    if (ret) {
	cerr << hexify(buf2.begin(), ret, true) << endl;
	cerr << hexify(buf2.begin(), ret, false) << endl;
    } else {
	cerr << "Got no data back from server" << endl;
    }

    // This next packet is the timestamp in milliseconds.
    // c2 00 06 ce 75 ba 00
    ret = net.readNet(buf2.begin(), buf2.size());
    if (ret) {
	cerr << hexify(buf2.begin(), ret, true) << endl;
	cerr << hexify(buf2.begin(), ret, false) << endl;
    } else {
	cerr << "Got no data back from server" << endl;
    }

    ret = net.readNet(buf2.begin(), buf2.size());
    Element *ell = amf_obj.extractAMF(buf2.begin()+12, buf2.begin() + ret - 12);
    if (ell) {
	ell->dump();
    }
    if (ret) {
	cerr << hexify(buf2.begin(), ret, true) << endl;
	cerr << hexify(buf2.begin(), ret, false) << endl;
    } else {
	cerr << "Got no data back from server" << endl;
    }

    ret = net.readNet(buf2.begin(), buf2.size());
    if (ret) {
	cerr << hexify(buf2.begin(), ret, true) << endl;
	cerr << hexify(buf2.begin(), ret, false) << endl;
    } else {
	cerr << "Got no data back from server" << endl;
    }
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

// local Propertys:
// mode: C++
// indent-tabs-mode: t
// End:
