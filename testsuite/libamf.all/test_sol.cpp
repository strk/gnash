// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
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
#include "sol.h"

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

    // Read a premade .sol file
    string filespec = SRCDIR;
    filespec += "/settings.sol";    
    test_read(filespec);

    // Write a .sol file
    filespec = "test.sol";
    test_write(filespec);
//    test_read(filespec);
    
//    test_sol();
}

bool
test_read(std::string &filespec)
{
    GNASH_REPORT_FUNCTION;
    struct stat st;

    if (stat(filespec.c_str(), &st) == 0) {
        SOL sol;
        char *buf;
        
        buf = new char[st.st_size + 1];

        sol.readFile(filespec);
        vector<amf::Element *> els = sol.getElements();

        if (els.size() > 1) {
            string str = els[2]->to_string();
            
            // Make sure multiple elements of varying datatypes are checked for.
            if ((els[0]->getName() == "gain") &&
                (els[2]->getName() == "defaultmicrophone") &&
                (str == "/dev/input/mic") &&
                (els[5]->getName() == "defaultalways") &&
                (els[9]->getName() == "trustedPaths")) {
                runtest.pass("Read SOL File");
            } else {
                runtest.fail("Read SOL file");
            }
        } else {
            runtest.fail("Read SOL file");
        }
        sol.dump();
    }
}

bool
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
    amf::Element *el = new amf::Element;
    el->init("gain", dub);
//    amf_obj.createElement(&el, "gain", dub);
    sol.addObj(el);
    if ((el->getName() == "gain") &&
        (el->getType() == Element::NUMBER) &&
        (memcmp(el->getData(), &dub, AMF_NUMBER_SIZE) == 0) &&
        (*((double *)el->getData()) == dub) &&
        (el->getLength() == AMF_NUMBER_SIZE)) {
        runtest.pass("gain set");
    } else {
        runtest.fail("gain set");
    }

    uint8_t *foo;
    char *ptr;
#if 0
    foo = amf_obj.encodeVariable(el); 
    ptr = (char *)amf_obj.extractVariable(&newel, foo);
    if ((el.getName() == newel.getName()) &&
        (el.getLength() == newel.getLength()) &&
        (newel.getType() == Element::NUMBER) &&
        (*((double *)newel.getData()) == dub)) {
        runtest.pass("gain number encoded/extracted");
    } else {
        runtest.fail("gain number encoded/extracted");
    }
#endif
    el = new amf::Element("echosuppression", false);
    sol.addObj(el);
    if ((el->getName() == "echosuppression") &&
        (el->getType() == Element::BOOLEAN) &&
        (*el->getData() == 0) &&
        (el->getLength() == 1)) {
        runtest.pass("echosupression set");
    } else {
        runtest.fail("echosupression set");
    }
    
//     foo = amf_obj.encodeVariable(el); 
//     ptr = (char *)amf_obj.extractVariable(&newel, reinterpret_cast<uint8_t *>(foo));
//     if ((el->getName() == newel.getName()) &&
//         (el->getType() == Element::BOOLEAN) &&
//         (el->getLength() == newel.getLength()) &&
//         (memcmp(el->getData(), newel.getData(), el->getLength()) == 0)) {
//         runtest.pass("echosupression bool(false) encoded/extracted");
//     } else {
//         runtest.fail("echosupression bool(false) encoded/extracted");
//     }
    
    string name = "defaultmicrophone";
    string data = "/dev/input/mic";
    el = new amf::Element("defaultmicrophone", data);
    sol.addObj(el);
    if ((el->getName() == name) &&
        (el->getType() == Element::STRING) &&
        (memcmp(el->getData(), data.c_str(), el->getLength()) == 0) &&
        (el->getLength() == data.size())) {
        runtest.pass("defaultmicrophone set");
    } else {
        runtest.fail("defaultmicrophone set");
    }

    data = "";
    el = new amf::Element("defaultcamera", data);
    el->init("defaultcamera", data);
    sol.addObj(el);
    if ((el->getName() == "defaultcamera") &&
        (el->getType() == Element::STRING) &&
        (*el->getData() == 0) &&
        (el->getLength() == 0)) {
        runtest.pass("defaultcamera set");
    } else {
        runtest.fail("defaultcamea set");
    }

    dub = 100.0;
    el = new amf::Element;
//    el = new amf::Element("defaultklimit", dub);
    el->init("defaultklimit", dub);
    sol.addObj(el);
    if ((el->getName() == "defaultklimit") &&
        (el->getType() == Element::NUMBER) &&
        (memcmp(el->getData(), &dub, AMF_NUMBER_SIZE) == 0) &&
        (*((double *)el->getData()) == dub) &&
        (el->getLength() == AMF_NUMBER_SIZE)) {
        runtest.pass("defaultklimit set");
    } else {
        runtest.fail("defaultklimit set");
    }

    el = new amf::Element("defaultalways", false);
    sol.addObj(el);
    if ((el->getName() == "defaultalways") &&
        (el->getType() == Element::BOOLEAN) &&
        (*el->getData() == 0) &&
        (el->getLength() == 1)) {
        runtest.pass("defaultalways set");
    } else {
        runtest.fail("defaultalways set");
    }

    el = new amf::Element("crossdomainAllow", true);
    sol.addObj(el);
    if ((el->getName() == "crossdomainAllow") &&
        (el->getType() == Element::BOOLEAN) &&
        (*el->getData() == 1) &&
        (el->getLength() == 1)) {
        runtest.pass("crossdomainAllow set");
    } else {
        runtest.fail("crossdomainAllow set");
    }

    el = new amf::Element("crossdomainAlways", true);
    sol.addObj(el);
    if ((el->getName() == "crossdomainAlways") &&
        (el->getType() == Element::BOOLEAN) &&
        (*el->getData() == 1) &&
        (el->getLength() == 1)) {
        runtest.pass("crossdomainAlways set");
    } else {
        runtest.fail("crossdomainAlways set");
    }

    el = new amf::Element("allowThirdPartyLSOAccess", true);
    sol.addObj(el);
    if ((el->getName() == "allowThirdPartyLSOAccess") &&
        (el->getType() == Element::BOOLEAN) &&
        (*el->getData() == 1) &&
        (el->getLength() == 1)) {
        runtest.pass("allowThirdPartyLSOAccess set");
    } else {
        runtest.fail("allowThirdPartyLSOAccess set");
    }

#if 0
    // FIXME: Why does GCC keep linking this to the bool
    // version instead ?
    boost::intrusive_ptr<gnash::as_object> as;
    amf_obj.createElement(&el, "trustedPaths", &as);
    if ((el->getName() == "trustedPaths") &&
        (el->getType() == Element::OBJECT)) {
        runtest.xpass("trustedPaths set");
    } else {
        runtest.xfail("trustedPaths set");
        // force the type so the binary output stays correct.
        // As this builds a null object, we get away with it,
        // and it helps debugging to have the hexdumps of the
        // .sol files match the originals.
        el->getType() = Element::OBJECT;        
        el->getLength() = 0;
    }
    sol.addObj(el);
#endif
    
    el = new amf::Element("localSecPath", data);
    sol.addObj(el);
    if ((el->getName() == "localSecPath") &&
        (el->getType() == Element::STRING) &&
        (*el->getData() == 0) &&
        (el->getLength() == 0)) {
        runtest.pass("localSecPath set");
    } else {
        runtest.fail("localSecPath set");
    }

    // Grabbed from GDB when reading this huge value
    dub = 1.8379389592608646e-304;
    swapBytes(&dub, 8);
    
    el = new amf::Element;
    el->init("localSecPathTime", dub);
//    el = new amf::Element("localSecPathTime", dub);
    sol.addObj(el);
    if ((el->getName() == "localSecPathTime") &&
        (el->getType() == Element::NUMBER) &&
        (memcmp(el->getData(), &dub, AMF_NUMBER_SIZE) == 0) &&
        (*((double *)el->getData()) == dub) &&
        (el->getLength() == AMF_NUMBER_SIZE)) {
        runtest.pass("localSecPathTime set");
    } else {
        runtest.fail("localSecPathTime set");
    }
    sol.dump();
    // now write the data to disk
    sol.writeFile(filespec, "settings");
}

#if 0
// Test SOL files. These are shared Objects which are basically an AMF object with
// a header. These .sol files are used for transferring data, so we want to make
// sure they actually work. All numeric data is stored in big endian format.
bool
test_sol(std::string &filespec)
{
    struct stat st;

    if (stat(filespec.c_str(), &st) == 0) {
        AMF amf_obj;
        int fd, ret;
        uint8_t *buf;
        
        buf = new uint8_t[st.st_size + 1];
        
        memset(buf, 0, st.st_size+1);
        fd = open(filespec.c_str(), O_RDONLY);
        ret = read(fd, buf, st.st_size);
        close(fd);
        
        Element *el = amf_obj.extractElement(buf);
        boost::uint8_t *num = &reinterpret_cast<boost::uint8_t *>(el);
        
        if ((num[6] == -16) && (num[7] == 0x3f)) {
            runtest.pass("Extracted Number SOL object");
        } else {
            runtest.fail("Extracted Number SOL object");
        }
        
        void *out = el->encodeNumber(num);
        
        if (memcmp(out, buf, 9) == 0) {
            runtest.pass("Encoded SOL Number");
        } else {
            runtest.fail("Encoded SOL Number");
        }
        return true;
    }
 
    runtest.untested("testfile not found");
    return false;
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
