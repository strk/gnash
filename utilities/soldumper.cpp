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
#include <iostream>
#include <cstdarg>
#include <cstring>

#ifdef ENABLE_NLS
# include <clocale>
#endif

extern "C"{
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifndef __GNUC__
extern int optind, getopt(int, char *const *, const char *);
extern char *optarg;
#endif
}

#include "log.h"
#include "rc.h"
#include "amf.h"
#include "sol.h"

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

const char *SOLDUMPER_VERSION = "0.5";

/// \brief  Display the command line arguments
static void
usage(ostream &o)
{
    o << _("This program dumps the internal data of a .sol file")
         << endl;
    o << _("Usage: soldumper [h] filename") << endl;
    o << _("-h\tHelp") << endl;
    o << _("-f\tForce local directory access") << endl;
    o << _("-l\tList all .sol files in default dir") << endl;
}

int
main(int argc, char *argv[])
{
    unsigned int          i;
    int                   c;
    bool                  localdir = false;
    bool                  listdir = false;
    //int                   size  = 0;
    string                filespec, realname, tmpname;
    struct dirent         *entry;
    vector<const char *>  dirlist;
    
    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    // scan for the two main standard GNU options
    for (c = 0; c < argc; c++) {
      if (strcmp("--help", argv[c]) == 0) {
        usage(cout);
        exit(EXIT_SUCCESS);
      }
      if (strcmp("--version", argv[c]) == 0) {
        printf (_("Gnash soldumper version: %s, Gnash version: %s\n"),
		   SOLDUMPER_VERSION, VERSION);
        exit(EXIT_SUCCESS);
      }
    }
 
    /* This initializes the DBG_MSG macros */ 
    while ((c = getopt (argc, argv, "hvfl")) != -1) {
        switch (c) {
          case 'h':
            usage(cout);
	    exit(EXIT_SUCCESS);
            break;
            
	  case 'v':
              dbglogfile.setVerbosity();
	      cout << _("Verbose output turned on") << endl;
	      break;
              
	  case 'f':
	      cout << _("forcing local directory access only") << endl;
              localdir = true;
	      break;

	  case 'l':
	      cout << _("List .sol files in the default directory") << endl;
              listdir = true;
	      break;

          default:
            usage(cerr);
            break;
        }
    }
    
    
    // If no command line arguments have been supplied, do nothing but
    // print the  usage message.
    if (argc < 2) {
        usage(cerr);
        exit(EXIT_FAILURE);
    }

    // get the file name from the command line
    if (optind < argc) {
        filespec = argv[optind];
        cout << "Will use \"" << filespec << "\" for sol files location" << endl;
    }
    
    // List the .sol files in the default directory
    if (listdir) {
        const char *dirname;
        if ((localdir) || (filespec[0] == '/') || (filespec[0] == '.')) {
            if (filespec.size() == 0) {
                dirname = "./";
            } else {
                dirname = filespec.c_str();
            }
        } else {
            dirname = rcfile.getSOLSafeDir().c_str();
        }
        DIR *library_dir = nullptr;
        library_dir = opendir (dirname);
        if (library_dir != nullptr) {
            // By convention, the first two entries in each directory are
            // for . and .. (``dot'' and ``dot dot''), so we ignore those. The
            // next directory read will get a real file, if any exists.
            entry = readdir(library_dir);
            entry = readdir(library_dir);
        }
        if (library_dir != nullptr) {
            for (i=0; entry>static_cast<struct dirent *>(0); i++) {
                entry = readdir(library_dir);
                if (entry != nullptr) {
                    //string::size_type pos;
                    if (strstr(entry->d_name, ".sol")) {
                        cout << "Found SOL: " << entry->d_name << endl;
                    }
                }
            }
        }
        exit(EXIT_SUCCESS);
    }
    
    string newspec;
    if ((localdir) || (filespec[0] == '/') || (filespec[0] == '.')) {
        newspec = filespec;
    } else {
        newspec = rcfile.getSOLSafeDir();
	newspec += "/";
        newspec += filespec;
    }
    
    cygnal::SOL sol;
    
    if (sol.readFile(newspec)) {
        cout << "SOL file \"" << newspec << "\" read in" << endl;
    } else {
	cerr << "SOL file \"" << newspec << "\" does not exist!" << endl;
    }

    sol.dump();
}

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
