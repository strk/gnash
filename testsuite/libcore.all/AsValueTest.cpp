// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_DEJAGNU_H

#include "VM.h"
#include "DummyMovieDefinition.h"
#include "ManualClock.h"
#include "movie_definition.h"
#include "dejagnu.h"
#include "as_value.h"
#include "StreamProvider.h"
#include "as_object.h"
#include "arg_parser.h"
#include "Global_as.h"
#include "GnashNumeric.h"
#include "movie_root.h"
#include "RunResources.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <iostream>
#include <string>

#include "check.h"

using namespace gnash;
using namespace std;

static void usage (void);

static void test_isnan();
static void test_conversion();

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();
RcInitFile& rcfile = RcInitFile::getDefaultInstance();

TRYMAIN(_runtest);
int
trymain(int argc, char *argv[])
{    const Arg_parser::Option opts[] =
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
                    // This happens once per 'v' flag 
                    log_debug(_("Verbose output turned on"));
                    break;
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }

    // Initialize gnash lib
    
    RunResources runResources;

    const URL url("");
    runResources.setStreamProvider(
            std::shared_ptr<StreamProvider>(new StreamProvider(url, url)));

    // Create a bogus movie with swf version 7 support
    movie_definition* md = new DummyMovieDefinition(runResources, 7);

    ManualClock clock;

    movie_root stage(clock, runResources);

    MovieClip::MovieVariables v;
    stage.init(md, v);

    // run the tests
    test_isnan();
    test_conversion();
   
    return 0;
}

void
test_bool(as_value boolval)
{
    if (boolval.is_bool()) {
        runtest.pass("as_value(bool)");
    } else {
        runtest.fail("as_value(bool)");
    }
}

void
test_int(as_value val)
{
    if (val.is_number()) {
        runtest.pass("as_value(int)");
    } else {
        runtest.fail("as_value(int)");
    }
}

void
test_string(as_value val)
{
    if (val.is_string()) {
        runtest.pass("as_value(string)");
    } else {
        runtest.fail("as_value(string)");
    }
}


typedef enum {
    ONE = 0,
    TWO = 1,
    THREE = 2
} enumbers;

void
test_conversion()
{
    test_bool(true);
    test_bool(false);
    test_int(5);
    test_int(1);
    test_int(double(0));
    test_int(0.0);

    test_int(THREE);
    test_int(ONE);
    test_int(TWO);

    test_string(std::string("lar"));

    test_string("lar");

    
}


void
test_isnan()
{
	float num = 0;

	if(!isNaN(num)) {
            runtest.pass("isNaN(0)");
        } else {
            runtest.fail("isNaN(0)");
        }
        
	num /= 9999999;

	if(!isNaN(num)) {
            runtest.pass("isNaN(9999999)");
        } else {
            runtest.fail("isNaN(9999999)");
        }
	if(isFinite(num)) {
            runtest.pass("isFinite(9999999)");
        } else {
            runtest.fail("isFinite(9999999)");
        }

	num = std::numeric_limits<float>::quiet_NaN();

	if(isNaN(num)) {
            runtest.pass("isNaN(quiet_NaN)");
        } else {
            runtest.fail("isNaN(quiet_NaN)");
        }
	if(!isFinite(num)) {
            runtest.pass("isFinite(quiet_NaN)");
        } else {
            runtest.fail("isFinite(quiet_NaN)");
        }

	num = std::numeric_limits<float>::infinity();
	
	if(!isNaN(num)) {
            runtest.pass("isNaN(infinity)");
        } else {
            runtest.fail("isNaN(infinity)");
        }
	if(!isFinite(num)) {
            runtest.pass("isFinite(infinity)");
        } else {
            runtest.fail("isFinite(infinity)");
        }

	num = 1.0 / 0.0;

	if(!isNaN(num)) {
            runtest.pass("isNaN(1.0/0.0)");
        } else {
            runtest.fail("isNaN(1.0/0.0)");
        }
	if(!isFinite(num)) {
            runtest.pass("isFinite(1.0/0.0)");
        } else {
            runtest.fail("isFinite(1.0/0.0)");
        }

	int intgr = num;

	num = intgr;

	if(!isNaN(num)) {
            runtest.pass("isNaN(int)");
        } else {
            runtest.fail("isNaN(int)");
        }
	if(isFinite(num)) {
            runtest.pass("isFinite(int)");
        } else {
            runtest.fail("isFinite(int)");
        }
}

static void
usage (void)
{
    cerr << "This program tests the as_value class." << endl
         << endl
         << _("Usage: AsValue [options...]") << endl
         << _("  -h,  --help          Print this help and exit") << endl
         << _("  -v,  --verbose       Output verbose debug info") << endl
         << endl;
}

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
  return 0;  
}

#endif

