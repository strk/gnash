// 
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include <boost/shared_ptr.hpp>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>

#include "flash/system/System_as.h"
#include "as_object.h"
#include "dejagnu.h"
#include "log.h"
#include "element.h"
#include "arg_parser.h"

using namespace gnash;
using namespace std;

static void usage (void);

static TestState runtest;

static void test_client();
static string url;

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
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
              case 0:
                  url = parser.argument(i);
				  log_debug(_("URL for testing the allowDomain function is: %s"), url);
                  break;
            }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }
    
    test_client();
}

static void test_client()
{
	string domain1("www.google.com");
	string domain2("www.youtube.com");
	string domain3("cnn.com");
	string domain4("92.123.68.89");

	addAllowDataAccess( domain1 );

	vector<string> vec = getAllowDataAccess();

	string added = vec[0];
	if( added == domain1 ) {
		runtest.pass("addAllowDataAccess correctly added 'www.google.com'");
	} else {
		runtest.fail("addAllowDataAccess did not correctly add 'www.google.com'");
	}
	if( (int) vec.size() == 1 ) {
		runtest.pass("_allowDataAccess vector contains 1 item");
	} else {
		runtest.fail("_allowDataAccess vector does not contain 1 item");
	}

	addAllowDataAccess( domain2 );
	vec = getAllowDataAccess();
	added = vec[1];
	if( added == domain2 ) {
		runtest.pass("addAllowDataAccess correctly added 'www.youtube.com'");
	} else {
		runtest.fail("addAllowDataAccess did not correctly add 'www.youtube.com'");
	}
	if( (int) vec.size()  == 2 ) {
		runtest.pass("_allowDataAccess vector contains 2 items");
	} else {
		runtest.fail("_allowDataAccess vector does not contain 2 items");
	}

	addAllowDataAccess( domain3 );
	vec = getAllowDataAccess();
	added = vec[2];
	if( added == domain3 ) {
		runtest.pass("'cnn.com' was correctly added to the vector");
	} else {
		runtest.fail("'cnn.com' was not correctly added to the vector");
	}
	if( (int)vec.size() == 3 ) {
		runtest.pass("_allowDataAccess vector contains 3 items");
	} else {
		runtest.fail("_allowDataAccess vector does not contain 3 items");
	}

	addAllowDataAccess( domain4 );
	vec = getAllowDataAccess();
	added = vec[3];
	if( added == domain4 ) {
		runtest.pass("'92.123.68.89' was correctly added to the vector");
	} else {
		runtest.fail("'92.123.68.89' was not correctly added to the vector");
	}
	if( (int)vec.size() == 4 ) {
		runtest.pass("_allowDataAccess vector now contains 4 items");
	} else {
		runtest.fail("_allowDataAccess vector does not contain 4 items");
	}

}

static void
usage (void)
{
    cerr << "This program tests SSL support in the libnet library." << endl;
    cerr << "Usage: test_ssl [hvsocpkwar]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    exit (-1);
}

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
    cerr << "This program needs to have DejaGnu installed!" << endl;
    return 0;  
}

#endif
