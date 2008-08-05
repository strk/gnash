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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <dirent.h>
#include <iostream>
#include <stdarg.h>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>

#ifdef ENABLE_NLS
# include <locale>
#endif

#include "log.h"
#include "rc.h"
#include "amf.h"
#include "flv.h"
#include "buffer.h"
#include "arg_parser.h"

using namespace amf;
using namespace std;
using namespace gnash;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

static void usage ();

int
main(int argc, char *argv[])
{
    bool dump = false;          // dump the FLV data
    vector<string> infiles;
    
    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    std::setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'd', "dump",          Arg_parser::no  },
        };
    
    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() ) {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }
    
    for( int i = 0; i < parser.arguments(); ++i ) {
        const int code = parser.code(i);
        try {
            switch( code ) {
              case 'h':
                  usage ();
                  exit(EXIT_SUCCESS);
              case 'v':
                  dbglogfile.setVerbosity();
                  log_debug(_("Verbose output turned on"));
                  break;
              case 'd':
                  dump = true;
                  break;
	      case 0:
		  infiles.push_back(parser.argument(i));
		  break;
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

    // Get the filename from the command line
    string filespec = infiles[0];

    Flv flv; 
    struct stat st;
    Network::byte_t *buf = 0;
    Network::byte_t *ptr = 0;
    Flv::flv_header_t head;
    Flv::previous_size_t   previous = 0;
    Flv::flv_tag_t         tag;
    
    // Make sure it's an SOL file
    if (stat(filespec.c_str(), &st) == 0) {
	try {
            // Open the binary file
	    ifstream ifs(filespec.c_str(), ios::binary);

            // Read just the initial 9 byte header
	    ifs.read(reinterpret_cast<char *>(&head), sizeof(Flv::flv_header_t));
            cerr << "sizeof(Flv::flv_header_t): " << sizeof(Flv::flv_header_t) << endl;
            
            // Does the magic number match ?
            if (memcmp(head.sig, "FLV", 3) == 0) {
                log_debug("%s is an FLV video file.", filespec);
            } else {
                log_error("%s is not an FLV video file!", filespec);
            }
            log_debug("FLV Version: %d (should always be 1)", int(head.version));
            if ((head.type & Flv::FLV_VIDEO) && (head.type & Flv::FLV_AUDIO)) {
                log_debug("FLV File type: Video and Audio");         
            } else if (head.type && Flv::FLV_VIDEO) {
                log_debug("FLV File type: Video");
            } else if (head.type && Flv::FLV_AUDIO) {
                log_debug("FLV File type: Audio");
            }
            boost::uint32_t head_size;
            memcpy(&head_size, head.head_size, 4);
            head_size = ntohl(head_size);
            log_debug("FLV Header size: %d (should always be 9)", head_size);
            
            // Extract all the Tags
            size_t total = st.st_size - head_size;
            while (total) {
                ifs.read(reinterpret_cast<char *>(&previous), sizeof(Flv::previous_size_t));
                previous = ntohl(previous);
                total -= sizeof(Flv::previous_size_t);
                log_debug("FLV Previous Tag Size was: %d", previous);
                ifs.read(reinterpret_cast<char *>(&tag), sizeof(Flv::flv_tag_t));
                total -= sizeof(Flv::previous_size_t);
                boost::uint32_t bodysize = 0;
                memcpy((char *)(&bodysize) + 1, tag.bodysize, 3);
//                swapBytes(&bodysize, 3);
                bodysize = ntohl(bodysize);
                total -= bodysize;
                switch (tag.type) {
                  case Flv::TAG_AUDIO:
                      log_debug("FLV Tag type is: Audio");
                      break;
                  case Flv::TAG_VIDEO: 
                      log_debug("FLV Tag type is: Video");
                      break;
                  case Flv::TAG_METADATA:
                  {
                      AMF amf;
                      log_debug("FLV Tag type is: MetaData");
                      Buffer *meta = new Buffer(500);
                      Network::byte_t *ptr = meta->reference();
                      meta->clear();
                      ifs.read(reinterpret_cast<char *>(meta->reference()), 500);
                      Network::byte_t* tooFar = ptr + meta->size();
                      // Extract the onMetaData object name
                      Element *el1 = amf.extractAMF(ptr, tooFar);
                      size_t seek = ifs.tellg();
                      ptr += amf.totalsize();
                      // Extract the properties for this metadata object.
                      Element *el2 = amf.extractAMF(ptr, tooFar);
                      ptr += amf.totalsize();
//                      ifs.seekg (seek + 424 + 9, ios::beg);
                      meta->clear();
                      ifs.read(reinterpret_cast<char *>(meta->reference()), 500);
                      seek = ifs.tellg();
                      continue;
                  }
                  default:
                      log_error("FLV Tag type out of range! was %d, should be 0x8, 0x9, or 0x12.",
                                bodysize);
                      break;
                }
                if (bodysize < FLV_MAX_LENGTH) {
                    boost::uint8_t *buf = new boost::uint8_t[bodysize];
                    ifs.read(reinterpret_cast<char *>(&buf), bodysize);
                    total -= bodysize;
                    log_debug("FLV Body size is: %d", bodysize);
                } else {
                    log_error("Bad bodysize of 0x%x.", bodysize);
                }
            };
            
        } catch (std::exception& e) {
	    log_error("Reading  %s: %s", filespec, e.what());
	    return false;
	}
    }
}

/// \brief  Display the command line arguments
static void
usage ()
{
    cerr << _("This program dumps the internal data of an FLV video file")
         << endl;
    cerr << _("Usage: flvdumper [h] filename") << endl;
    cerr << _("-h\tHelp") << endl;
    exit (-1);
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
