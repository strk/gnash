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

#include "buffer.h"
#include "network.h"
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

//    el.dump();
    el.clear();
    
    string str = "Hello World!";
    el.makeString("Hello World!");
    if ((el.getType() == Element::STRING) &&
        (el.to_string() == str)) {
        runtest.pass("Created string element");
    } else {
        runtest.fail("Created string element");
    }

    el.clear();
    bool sheet = true;
    el.makeBoolean(sheet);
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


// amf::Element::makeNumber(unsigned char*)
// amf::Element::makeNumber(std::string const&, double)
// amf::Element::makeNumber(double)
// amf::Element::makeObject(unsigned char*, unsigned int)
// amf::Element::makeObject(std::string const&)
// amf::Element::makeString(char const*, unsigned int)
// amf::Element::makeString(unsigned char*, unsigned int)
// amf::Element::makeString(std::string const&)
// amf::Element::makeString(std::string const&, std::string const&)
// amf::Element::getNameSize()
// amf::Element::makeBoolean(unsigned char*)
// amf::Element::makeBoolean(std::string const&, bool)
// amf::Element::makeBoolean(bool)
// amf::Element::to_reference()
// amf::Element::makeECMAArray(unsigned char*, unsigned int)
// amf::Element::makeMovieClip(unsigned char*, unsigned int)
// amf::Element::makeObjectEnd()
// amf::Element::makeRecordSet(unsigned char*, unsigned int)
// amf::Element::makeReference(unsigned char*, unsigned int)
// amf::Element::makeUndefined(std::string const&)
// amf::Element::makeUndefined()
// amf::Element::makeXMLObject(unsigned char*, unsigned int)
// amf::Element::makeLongString(unsigned char*, unsigned int)
// amf::Element::makeNullString()
// amf::Element::makeStrictArray(unsigned char*, unsigned int)
// amf::Element::makeTypedObject(unsigned char*, unsigned int)
// amf::Element::dump()
// amf::Element::init(std::string const&)
// amf::Element::init(std::string const&, std::string const&)
// amf::Element::init(std::string const&, bool)
// amf::Element::init(std::string const&, double)
// amf::Element::init(bool)
// amf::Element::init(bool, double, double, std::string const&)
// amf::Element::init(double)
// amf::Element::clear()
// amf::Element::getData()
// amf::Element::setName(unsigned char*, unsigned int)
// amf::Element::setName(std::string const&)
// amf::Element::to_bool()
// amf::Element::makeDate(unsigned char*)
// amf::Element::makeNull(std::string const&)
// amf::Element::makeNull()
// amf::Element::getLength()
// amf::Element::to_number()
// amf::Element::to_string()
// amf::Element::Element(unsigned char*)
// amf::Element::Element(std::string const&)
// amf::Element::Element(std::string const&, std::string const&)
// amf::Element::Element(std::string const&, bool)
// amf::Element::Element(bool)
// amf::Element::Element(bool, double, double, std::string const&)
// amf::Element::Element(double)
// amf::Element::Element()
// amf::Element::Element(unsigned char*)
// amf::Element::Element(std::string const&)
// amf::Element::Element(std::string const&, std::string const&)
// amf::Element::Element(std::string const&, bool)
// amf::Element::Element(bool)
// amf::Element::Element(bool, double, double, std::string const&)
// amf::Element::Element(double)
// amf::Element::Element()
// amf::Element::~Element()
// amf::Element::~Element()
// amf::Element::operator=(amf::Element&)
// amf::Element::operator==(bool)
// amf::Element::operator[](int)
