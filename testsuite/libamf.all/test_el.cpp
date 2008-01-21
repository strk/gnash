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

#ifdef HAVE_DEJAGNU_H

//#include <netinet/in.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "as_object.h"

extern "C"{
#include <unistd.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifndef __GNUC__
extern int optind, getopt(int, char *const *, const char *);
#endif
}
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <log.h>
#include <iostream>
#include <string>

#include "dejagnu.h"

#include "amf.h"
#include "element.h"

using namespace amf;
using namespace gnash;
using namespace std;

static void usage (void);

static TestState runtest;

bool test_read(std::string &filespec);
bool test_write(std::string &filespec);
bool test_sol(std::string &filespec);

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    int c;

    while ((c = getopt (argc, argv, "hdvsm:")) != -1) {
        switch (c) {
          case 'h':
            usage ();
            break;
            
          case 'v':
              dbglogfile.setVerbosity();
            break;
            
          default:
            usage ();
            break;
        }
    }

    Element el;
    if (el.getType() == Element::NOTYPE) {
        runtest.pass("Created empty element");
    } else {
        runtest.fail("Created empty element");
    }

    double dub = 54.3;
    el.init(dub);
    if ((el.getType() == Element::NUMBER) &&
        (el.to_number() == dub)) {
        runtest.pass("Created double element");
    } else {
        runtest.fail("Created double element");
    }

    el.dump();
    el.clear();
    
    string str = "Hello World!";
    el.init(str);
    if ((el.getType() == Element::STRING) &&
        (el.to_string() == str)) {
        runtest.pass("Created string element");
    } else {
        runtest.fail("Created string element");
    }

    el.clear();
    bool sheet = true;
    el.init(sheet);
    if ((el.getType() == Element::BOOLEAN) &&
        (el.to_bool() == sheet)) {
        runtest.pass("Created bool element");
    } else {
        runtest.fail("Created bool element");
    }

    el.clear();
    el.makeNull();
    if (el.getType() == Element::NULL_VALUE) {
        runtest.pass("Created NULL Value element");
    } else {
        runtest.fail("Created NULL Value element");
    }
    el.clear();
    el.makeUndefined();
    if (el.getType() == Element::UNDEFINED) {
        runtest.pass("Created Undefined element");
    } else {
        runtest.fail("Created Undefined element");
    }

}

static void
usage (void)
{
    cerr << "This program tests AMF Element support in the AMF library." << endl;
    cerr << "Usage: test_el [hv]" << endl;
    cerr << "-h\tHelp" << endl;
    cerr << "-v\tVerbose" << endl;
    exit (-1);
}

#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
  return 0;  
}

#endif
