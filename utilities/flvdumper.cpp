// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2014
//   Free Software Foundation, Inc
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

#include "GnashFileUtilities.h"
#include "GnashSystemNetHeaders.h"

#include <dirent.h>
#include <iostream>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>

#ifdef ENABLE_NLS
# include <clocale>
#endif

#include "log.h"
#include "rc.h"
#include "amf.h"
#include "flv.h"
#include "buffer.h"
#include "arg_parser.h"

using namespace cygnal;
using namespace std;
using namespace gnash;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

#ifdef BOOST_NO_EXCEPTIONS
namespace boost
{

	void throw_exception(std::exception const & e)
	{
		std::abort();
	}
}
#endif

static void usage ();

static const char *codec_strs[] = {
    "None",
    "None",
    "H263",
    "Screen",
    "VP6",
    "VP6_Alpha",
    "Screen2",
    "Theora",
    "Dirac",
    "Speex"
};

static const char *format_strs[] = {
    "Uncompressed",
    "ADPCM",
    "MP3",
    "Unknown",
    "Unknown",
    "Nellymoser_8KHZ",
    "Nellymoser",
    // These next are only supported by Gnash
    "Vorbis"
};

static const char *frame_strs[] = {
    "NO_frame",
    "Keyframe",
    "Interframe",
    "Disposable"
};

static const char *size_strs[] = {
    "8Bit",
    "16Bit"
};

static const char *type_strs[] = {
    "Mono",
    "Stereo"
};

static const char *rate_strs[] = {
    "55Khz",
    "11Khz",
    "22Khz",
    "44Khz",
};

int
main(int argc, char *argv[])
{
    bool dump = false;          // dump the FLV data
    bool all = false;		// dump all the tags too
    bool meta = true;		// dump all Meta tags only
    vector<string> infiles;
    
    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'd', "dump",          Arg_parser::no  },
            { 'a', "all",           Arg_parser::no  },
            { 'm', "meta",           Arg_parser::no  },
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
              case 'a':
                  all = true;
                  break;
              case 'm':
                  meta = true;
                  break;
	      case 0:
		  infiles.push_back(parser.argument(i));
		  break;
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash flvdumper bug.") << endl;
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

//    std::shared_ptr<Flv::flv_header_t> head;
    Flv::previous_size_t   previous = 0;
    std::shared_ptr<Flv::flv_tag_t> tag;
    
    // Make sure it's an FLV file
    if (stat(filespec.c_str(), &st) == 0) {
	try {
            // Open the binary file
	    ifstream ifs(filespec.c_str(), ios::binary);
	    std::shared_ptr<cygnal::Buffer> buf(new Buffer);
            // Read just the initial 9 byte header
	    ifs.read(reinterpret_cast<char *>(buf->reference()), sizeof(Flv::flv_header_t));
	    log_debug("header is: %s",  hexify(buf->reference(), 9, false));
	    std::shared_ptr<Flv::flv_header_t> head = flv.decodeHeader(buf);
	    if (head == 0) {
		log_error("Couldn't decode the header! %s",  hexify(buf->reference(), 9, false));
		exit(EXIT_FAILURE);
	    }
	    if ((head->type & Flv::FLV_VIDEO) && (head->type & Flv::FLV_AUDIO)) {
                cout <<"FLV File type: Video and Audio" << endl;
            } else if (head->type && Flv::FLV_VIDEO) {
		cout << "FLV File type: Video" << endl;
            } else if (head->type && Flv::FLV_AUDIO) {
		cout <<"FLV File type: Audio" << endl;
	    }
	    
	    cout << "FLV Version: " << int(head->version) << " (should always be 1)" << endl;
	    boost::uint32_t headsize = flv.convert24(head->head_size);
	    if (all) {
		cout << "FLV Header size: " << headsize << " (should always be 9)" << endl;
	    }
            // Extract all the Tags
            //size_t total = st.st_size - sizeof(Flv::flv_header_t);
             while (!ifs.eof()) {
		 ifs.read(reinterpret_cast<char *>(&previous), sizeof(Flv::previous_size_t));
		 if (ifs.gcount() != sizeof(Flv::previous_size_t)) {
		     log_error("Couldn't read the entire header");
		 }
		 
		 previous = ntohl(previous);
		 //total -= sizeof(Flv::previous_size_t);
		 if (all) {   
		     cout << "FLV Previous Tag Size was: " << previous << endl;
		 }
		 ifs.read(reinterpret_cast<char *>(buf->reference()), sizeof(Flv::flv_tag_t));
		 if (ifs.gcount() != sizeof(Flv::flv_tag_t)) {
		     log_error("Couldn't read the entire tag");
		 }
		 tag  = flv.decodeTagHeader(buf);
		 if (dump) {
		     flv.dump();
		 }
		 //total -= sizeof(Flv::previous_size_t);
		 size_t bodysize = flv.convert24(tag->bodysize);
		 if (bodysize == 0) {
		     cerr << "FLV Tag size is zero, skipping reading packet body " << bodysize << endl;
		     continue;
		 } else {
		     if (all) {   
			 cout << "FLV Tag size is: " << bodysize +  sizeof(Flv::previous_size_t) << endl;
		     }
		 }
		 buf->resize(bodysize);
		 ifs.read(reinterpret_cast<char *>(buf->reference()), bodysize);
// 		 if (ifs.gcount() != bodysize) {
// 		     log_error("Couldn't read the entire body");
// 		 }
		 //total -= bodysize;
		 switch (tag->type) {
		   case Flv::TAG_AUDIO:
		   {
		       if (all) {
			   cerr << "FLV Tag type is: Audio" << endl;
 			   std::shared_ptr<Flv::flv_audio_t> data = flv.decodeAudioData(*(buf->reference() + sizeof(Flv::flv_tag_t)));
 			   cout << "\tSound Type is: "   << type_strs[data->type] << endl;
 			   cout << "\tSound Size is: "   << size_strs[data->size] << endl;
 			   cout << "\tSound Rate is: "   << rate_strs[data->rate] << endl;
 			   cout << "\tSound Format is: " << format_strs[data->format] << endl;
		       }
		       break;
		   }
		   case Flv::TAG_VIDEO:
		   {
		       if (all) {
			   cout << "FLV Tag type is: Video" << endl;
 			   std::shared_ptr<Flv::flv_video_t> data = flv.decodeVideoData(*(buf->reference() + sizeof(Flv::flv_tag_t)));
 			   cout << "\tCodec ID is: "   << codec_strs[data->codecID] << endl;
 			   cout << "\tFrame Type is: " << frame_strs[data->type] << endl;
		       }
		       break;
		   }
		   case Flv::TAG_METADATA:
 		       if (meta || all) {
 			   cout << "FLV Tag type is: MetaData" << endl;
 		       }
		       std::shared_ptr<cygnal::Element> metadata = flv.decodeMetaData(buf->reference(), bodysize);
		       if (meta && metadata) {
			   metadata->dump();
		       }
		       continue;
		 };		 
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
    cerr << _("Usage: flvdumper [-h] [-m] [-a] filename") << endl;
    cerr << _("-h\tHelp") << endl;
    cerr << _("-m\tPrint only Meta tags (default)") << endl;
    cerr << _("-a\tPrint all tags.") << endl;
    exit (-1);
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
