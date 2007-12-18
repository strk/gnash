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
    struct stat st;

    if (stat(filespec.c_str(), &st) == 0) {
        SOL sol;
        char *buf;
        
        buf = new char[st.st_size + 1];

        sol.readFile(filespec);
        vector<AMF::amf_element_t> els = sol.getElements();

        if (els.size() > 1) {
            string str = reinterpret_cast<const char *>(els[2].data);
            
            // Make sure multiple elements of varying datatypes are checked for.
            if ((els[0].name == "gain") &&
                (els[2].name == "defaultmicrophone") &&
                (str == "/dev/input/mic") &&
                (els[5].name == "defaultalways") &&
                (els[9].name == "trustedPaths")) {
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
    SOL sol;
    AMF amf_obj;
    
//    char *data = const_cast<char *>("/dev/input/mic");
//    el.data = reinterpret_cast<uint8_t *>(data);
    AMF::amf_element_t el;
    AMF::amf_element_t newel;

    double dub = 50.0;
    amf_obj.createElement(&el, "gain", dub);
    sol.addObj(el);
    if ((el.name == "gain") &&
        (el.type == AMF::NUMBER) &&
        (memcmp(el.data, &dub, AMF_NUMBER_SIZE) == 0) &&
        (*((double *)el.data) == dub) &&
        (el.length == AMF_NUMBER_SIZE)) {
        runtest.pass("gain set");
    } else {
        runtest.fail("gain set");
    }

    uint8_t *foo = amf_obj.encodeVariable(el); 
    char *ptr = (char *)amf_obj.extractVariable(&newel, reinterpret_cast<uint8_t *>(foo));
    if ((el.name == newel.name) &&
        (el.length == newel.length) &&
        (newel.type == AMF::NUMBER) &&
        (memcmp(el.data, newel.data, el.length) == 0)) {
        runtest.pass("gain number encoded/extracted");
    } else {
        runtest.fail("gain number encoded/extracted");
    }
    
    
    amf_obj.createElement(&el, "echosuppression", false);
    sol.addObj(el);
    if ((el.name == "echosuppression") &&
        (el.type == AMF::BOOLEAN) &&
        (*el.data == 0) &&
        (el.length == 1)) {
        runtest.pass("echosupression set");
    } else {
        runtest.fail("echosupression set");
    }
    
    foo = amf_obj.encodeVariable(el); 
    ptr = (char *)amf_obj.extractVariable(&newel, reinterpret_cast<uint8_t *>(foo));
    if ((el.name == newel.name) &&
        (el.type == AMF::BOOLEAN) &&
        (el.length == newel.length) &&
        (memcmp(el.data, newel.data, el.length) == 0)) {
        runtest.pass("echosupression bool(false) encoded/extracted");
    } else {
        runtest.fail("echosupression bool(false) encoded/extracted");
    }
    

    string name = "defaultmicrophone";
    string data = "/dev/input/mic";
    amf_obj.createElement(&el, name, data);
    sol.addObj(el);
    if ((el.name == name) &&
        (el.type == AMF::STRING) &&
        (memcmp(el.data, data.c_str(), el.length) == 0) &&
        (el.length == data.size())) {
        runtest.pass("defaultmicrophone set");
    } else {
        runtest.fail("defaultmicrophone set");
    }

    amf_obj.createElement(&el, "defaultcamera", "");
    sol.addObj(el);
    if ((el.name == "defaultcamera") &&
        (el.type == AMF::STRING) &&
        (*el.data == 0) &&
        (el.length == 0)) {
        runtest.pass("defaultcamera set");
    } else {
        runtest.fail("defaultcamea set");
    }

    dub = 100.0;
    amf_obj.createElement(&el, "defaultklimit", dub);
    sol.addObj(el);
    if ((el.name == "defaultklimit") &&
        (el.type == AMF::NUMBER) &&
        (memcmp(el.data, &dub, AMF_NUMBER_SIZE) == 0) &&
        (*((double *)el.data) == dub) &&
        (el.length == AMF_NUMBER_SIZE)) {
        runtest.pass("defaultklimit set");
    } else {
        runtest.fail("defaultklimit set");
    }

    amf_obj.createElement(&el, "defaultalways", false);
    sol.addObj(el);
    if ((el.name == "defaultalways") &&
        (el.type == AMF::BOOLEAN) &&
        (*el.data == 0) &&
        (el.length == 1)) {
        runtest.pass("defaultalways set");
    } else {
        runtest.fail("defaultalways set");
    }

    amf_obj.createElement(&el, "crossdomainAllow", true);
    sol.addObj(el);
    if ((el.name == "crossdomainAllow") &&
        (el.type == AMF::BOOLEAN) &&
        (*el.data == 1) &&
        (el.length == 1)) {
        runtest.pass("crossdomainAllow set");
    } else {
        runtest.fail("crossdomainAllow set");
    }

    amf_obj.createElement(&el, "crossdomainAlways", true);
    sol.addObj(el);
    if ((el.name == "crossdomainAlways") &&
        (el.type == AMF::BOOLEAN) &&
        (*el.data == 1) &&
        (el.length == 1)) {
        runtest.pass("crossdomainAlways set");
    } else {
        runtest.fail("crossdomainAlways set");
    }

    amf_obj.createElement(&el, "allowThirdPartyLSOAccess", true);
    sol.addObj(el);
    if ((el.name == "allowThirdPartyLSOAccess") &&
        (el.type == AMF::BOOLEAN) &&
        (*el.data == 1) &&
        (el.length == 1)) {
        runtest.pass("allowThirdPartyLSOAccess set");
    } else {
        runtest.fail("allowThirdPartyLSOAccess set");
    }

    // FIXME: Why does GCC keep linking this to the bool
    // version instead ?
    boost::intrusive_ptr<gnash::as_object> as;
    amf_obj.createElement(&el, "trustedPaths", &as);
    if ((el.name == "trustedPaths") &&
        (el.type == AMF::OBJECT)) {
        runtest.pass("trustedPaths set");
    } else {
        runtest.fail("trustedPaths set");
        // force the type so the binary output stays correct.
        // As this builds a null object, we get away with it,
        // and it helps debugging to have the hexdumps of the
        // .sol files match the originals.
        el.type = AMF::OBJECT;        
        el.length = 0;
    }
    sol.addObj(el);

    amf_obj.createElement(&el, "localSecPath", "");
    sol.addObj(el);
    if ((el.name == "localSecPath") &&
        (el.type == AMF::STRING) &&
        (*el.data == 0) &&
        (el.length == 0)) {
        runtest.pass("localSecPath set");
    } else {
        runtest.fail("localSecPath set");
    }

    // Grabbed from GDB when reading this huge value
    dub = 1.8379389592608646e-304;
    swapBytes(&dub, 8);
    
    amf_obj.createElement(&el, "localSecPathTime", dub);
    sol.addObj(el);
    if ((el.name == "localSecPathTime") &&
        (el.type == AMF::NUMBER) &&
        (memcmp(el.data, &dub, AMF_NUMBER_SIZE) == 0) &&
        (*((double *)el.data) == dub) &&
        (el.length == AMF_NUMBER_SIZE)) {
        runtest.pass("localSecPathTime set");
    } else {
        runtest.fail("localSecPathTime set");
    }

    sol.dump();
    // now write the data to disk
    sol.writeFile(filespec, "settings");
}

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
        amfnum_t *num;
        
        buf = new uint8_t[st.st_size + 1];
        
        memset(buf, 0, st.st_size+1);
        fd = open(filespec.c_str(), O_RDONLY);
        ret = read(fd, buf, st.st_size);
        close(fd);
        
        num = amf_obj.extractNumber(buf);
        
        if ((((char *)num)[6] == -16) && (((char *)num)[7] == 0x3f)) {
            runtest.pass("Extracted Number SOL object");
        } else {
            runtest.fail("Extracted Number SOL object");
        }
        
        void *out = amf_obj.encodeNumber(*num);
        
        if (memcmp(out, buf, 9) == 0) {
            runtest.pass("Encoded SOL Number");
        } else {
            runtest.fail("Encoded SOL Number");
        }
        delete num;
        return true;
    }
 
    runtest.untested("testfile not found");
    return false;
}

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
