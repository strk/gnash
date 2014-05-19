// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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
#include <boost/shared_ptr.hpp>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string>

#include "as_object.h"
#include "dejagnu.h"
#include "log.h"
#include "amf.h"
#include "buffer.h"
#include "network.h"
#include "element.h"
#include "sol.h"
#include "arg_parser.h"
#include "gmemory.h"

using namespace cygnal;
using namespace gnash;
using namespace std;

static void usage (void);

// Enable the display of memory allocation and timing data
static bool memdebug = false;

// We use the Memory profiling class to check the malloc buffers
// in the kernel to make sure the allocations and frees happen
// the way we expect them too. There is no real other way to tell.
#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
Memory *mem = 0;
#endif

static TestState runtest;

static void test_read(std::string &filespec);
//static void test_write(std::string &filespec);
bool test_sol(std::string &filespec);

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    const Arg_parser::Option opts[] =
        {
            { 'h', "help",          Arg_parser::no  },
            { 'v', "verbose",       Arg_parser::no  },
            { 'w', "write",         Arg_parser::no  },
// Unless you have support for memory debugging turned on, and
// you have support for the Linux mallinfo() system call,
// this option is totally useless. This doesn't really matter
// as the memory testing is primarily used only during
// debugging or development.
            { 'm', "memstats",      Arg_parser::no  },
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
              case 'm':
                    log_debug(_("Enabling memory statistics"));
                    memdebug = true;
                    break;
	    }
        }
        
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }

#if defined(HAVE_MALLINFO) && defined(USE_STATS_MEMORY)
    if (memdebug) {
        mem = new Memory;
        mem->startStats();
    }
#endif
    
    // Read a premade .sol file
    string filespec = SRCDIR;
    filespec += "/settings.sol";    
    test_read(filespec);

    filespec = SRCDIR;
    filespec += "/testout.sol";    
    // Write a .sol file
    
//    test_write(filespec);
    
//    test_sol();
}

void
test_read(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;
    struct stat st;

    std::shared_ptr<Buffer> hex1(new Buffer("00 bf 00 00 01 28 54 43 53 4f 00 04 00 00 00 00 00 08 73 65 74 74 69 6e 67 73 00 00 00 00 00 04 67 61 69 6e 00 40 49 00 00 00 00 00 00 00 00 0f 65 63 68 6f 73 75 70 70 72 65 73 73 69 6f 6e 01 00 00 00 11 64 65 66 61 75 6c 74 6d 69 63 72 6f 70 68 6f 6e 65 02 00 0e 2f 64 65 76 2f 69 6e 70 75 74 2f 6d 69 63 00 00 0d 64 65 66 61 75 6c 74 63 61 6d 65 72 61 02 00 00 00 00 0d 64 65 66 61 75 6c 74 6b 6c 69 6d 69 74 00 40 59 00 00 00 00 00 00 00 00 0d 64 65 66 61 75 6c 74 61 6c 77 61 79 73 01 00 00 00 10 63 72 6f 73 73 64 6f 6d 61 69 6e 41 6c 6c 6f 77 01 01 00 00 11 63 72 6f 73 73 64 6f 6d 61 69 6e 41 6c 77 61 79 73 01 01 00 00 18 61 6c 6c 6f 77 54 68 69 72 64 50 61 72 74 79 4c 53 4f 41 63 63 65 73 73 01 01 00 00 0c 74 72 75 73 74 65 64 50 61 74 68 73 03 00 00 09 00 00 0c 6c 6f 63 61 6c 53 65 63 50 61 74 68 02 00 00 00 00 10 6c 6f 63 61 6c 53 65 63 50 61 74 68 54 69 6d 65 00 42 71 6d 14 10 22 e0 00 00"));

    if (stat(filespec.c_str(), &st) == 0) {
        SOL sol;
        sol.readFile(filespec);
        vector<std::shared_ptr<cygnal::Element> > els = sol.getElements();

        if (els.size() > 1) {
            string str = els[2]->to_string();

            // Make sure multiple elements of varying datatypes are checked for.
            if ((strcmp(els[0]->getName(), "gain") == 0) &&
                (strcmp(els[2]->getName(), "defaultmicrophone") == 0) &&
                (str == "/dev/input/mic") &&
                (strcmp(els[5]->getName(), "defaultalways") == 0) &&
                 (strcmp(els[9]->getName(), "trustedPaths") == 0)) {
                runtest.pass("Read SOL File");
            } else {
                runtest.fail("Read SOL file");
            }
        } else {
            runtest.fail("Read SOL file");
        }
//        sol.dump();
    }
}

#if 0
void
test_write(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;
    
    SOL sol;
    AMF amf_obj;
    
//    char *data = const_cast<char *>("/dev/input/mic");
//    el.getData() = reinterpret_cast<uint8_t *>(data);
    amf::Element newel;

    double dub = 50.0;

//    amf::Element *el = new amf::Element("gain", dub);
    amf::Element *el = new amf::Element("gain", dub);
//    amf_obj.createElement(&el, "gain", dub);
    sol.addObj(el);

    if ((strcmp(el->getName(), "gain") == 0) &&
        (el->getType() == Element::NUMBER_AMF0) &&
        (el->to_number() == dub) &&
        (el->getLength() == AMF0_NUMBER_SIZE)) {
        runtest.pass("gain set");
    } else {
        runtest.fail("gain set");
    }
    
    el = new amf::Element("echosuppression", false);
    sol.addObj(el);
    if ((strcmp(el->getName(), "echosuppression") == 0) &&
        (el->getType() == Element::BOOLEAN_AMF0) &&
        (*el->getData() == 0) &&
        (el->getLength() == 1)) {
        runtest.pass("echosupression set");
    } else {
        runtest.fail("echosupression set");
    }
    
    string name = "defaultmicrophone";
    string data = "/dev/input/mic";
    el = new amf::Element("defaultmicrophone", data);
    sol.addObj(el);
    if ((el->getName() == name) &&
        (el->getType() == Element::STRING_AMF0) &&
        (memcmp(el->getData(), data.c_str(), el->getLength()) == 0) &&
        (el->getLength() == data.size())) {
        runtest.pass("defaultmicrophone set");
    } else {
        runtest.fail("defaultmicrophone set");
    }

    data = "";
    el = new amf::Element("defaultcamera", data);
    sol.addObj(el);
    if ((strcmp(el->getName(), "defaultcamera") == 0) &&
        (el->getType() == Element::STRING_AMF0) &&
        (el->getLength() == 0)) {
        runtest.pass("defaultcamera set");
    } else {
        runtest.fail("defaultcamera set");
    }
    
    dub = 100.0;
    el = new amf::Element("defaultklimit", dub);
//    el = new amf::Element("defaultklimit", dub);
    sol.addObj(el);
    if ((strcmp(el->getName(), "defaultklimit") == 0) &&
        (el->getType() == Element::NUMBER_AMF0) &&
        (el->to_number() == dub) &&
        (el->getLength() == AMF0_NUMBER_SIZE)) {
        runtest.pass("defaultklimit set");
    } else {
        runtest.fail("defaultklimit set");
    }
    
    el = new amf::Element("defaultalways", false);
    sol.addObj(el);
    if ((strcmp(el->getName(), "defaultalways") == 0) &&
        (el->getType() == Element::BOOLEAN_AMF0) &&
        (el->to_bool() == false) &&
        (el->getLength() == 1)) {
        runtest.pass("defaultalways set");
    } else {
        runtest.fail("defaultalways set");
    }

    el = new amf::Element("crossdomainAllow", true);
    sol.addObj(el);
    if ((strcmp(el->getName(), "crossdomainAllow") == 0) &&
        (el->getType() == Element::BOOLEAN_AMF0) &&
        (el->to_bool() == true) &&
        (el->getLength() == 1)) {
        runtest.pass("crossdomainAllow set");
    } else {
        runtest.fail("crossdomainAllow set");
    }

    el = new amf::Element("crossdomainAlways", true);
    sol.addObj(el);
    if ((strcmp(el->getName(), "crossdomainAlways") == 0) &&
        (el->getType() == Element::BOOLEAN_AMF0) &&
        (*el->getData() == 1) &&
        (el->getLength() == 1)) {
        runtest.pass("crossdomainAlways set");
    } else {
        runtest.fail("crossdomainAlways set");
    }

    el = new amf::Element("allowThirdPartyLSOAccess", true);
    sol.addObj(el);
    if ((strcmp(el->getName(), "allowThirdPartyLSOAccess") ==0) &&
        (el->getType() == Element::BOOLEAN_AMF0) &&
        (*el->getData() == 1) &&
        (el->getLength() == 1)) {
        runtest.pass("allowThirdPartyLSOAccess set");
    } else {
        runtest.fail("allowThirdPartyLSOAccess set");
    }
    
    el = new amf::Element("localSecPath", data);
    sol.addObj(el);
    if ((strcmp(el->getName(), "localSecPath") == 0) &&
        (el->getType() == Element::STRING_AMF0) &&
        (el->getLength() == 0)) {
        runtest.pass("localSecPath set");
    } else {
        runtest.fail("localSecPath set");
    }

    // Grabbed from GDB when reading this huge value
    dub = 1.8379389592608646e-304;
    swapBytes(&dub, 8);
    
    el = new amf::Element("localSecPathTime", dub);
    sol.addObj(el);
    if ((strcmp(el->getName(), "localSecPathTime") ==0) &&
        (el->getType() == Element::NUMBER_AMF0) &&
        (memcmp(el->getData(), &dub, AMF0_NUMBER_SIZE) == 0) &&
        (*((double *)el->getData()) == dub) &&
        (el->getLength() == AMF0_NUMBER_SIZE)) {
        runtest.pass("localSecPathTime set");
    } else {
        runtest.fail("localSecPathTime set");
    }
    sol.dump();
    // now write the data to disk
    sol.writeFile(filespec, "settings");
}
#endif

static void
usage (void)
{
    cerr << "This program tests SOL support in the AMF library." << endl;
    cerr << "Usage: test_sol [hv]" << endl;
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

// amf::SOL::formatHeader(std::string const&)
// amf::SOL::formatHeader(std::string const&, int)
// amf::SOL::formatHeader(std::vector<unsigned char, std::allocator<unsigned char> > const&)
// amf::SOL::extractHeader(std::string const&)
// amf::SOL::extractHeader(std::vector<unsigned char, std::allocator<unsigned char> > const&)
// amf::SOL::dump()
// amf::SOL::addObj(amf::Element*)
// amf::SOL::readFile(std::string&)
// amf::SOL::writeFile(std::string const&, std::string const&)
// amf::SOL::SOL()
// amf::SOL::SOL()
// amf::SOL::~SOL()
// amf::SOL::~SOL()
