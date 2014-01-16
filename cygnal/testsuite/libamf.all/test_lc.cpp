// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#ifdef HAVE_DEJAGNU_H

//#include <netinet/in.h>
#include <string>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#include "as_object.h"

#if !defined(HAVE_WINSOCK_H) && !defined(__riscos__) && !defined(__OS2__)
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#elif !defined(__riscos__) && !defined(__OS2__)
#include <windows.h>
#include <process.h>
#include <io.h>
#endif

extern "C"{
#include "GnashSystemIOHeaders.h"
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
#include "SharedMem.h"
#include "lcshm.h"

using namespace cygnal;
using namespace gnash;
using namespace std;

const int LC_HEADER_SIZE = 16;
const int MAX_LC_HEADER_SIZE = 40960;
const int LC_LISTENERS_START  = MAX_LC_HEADER_SIZE +  LC_HEADER_SIZE;

static void usage (void);

static TestState runtest;

void test_read();
void test_write();
void test_listen();
void test_data();
void load_data();

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
//    string filespec = "localhost:test_lc"; 
//    load_data();
//    test_read();
   system("ipcs");
    test_data();
   system("ipcs");
//    test_listen();
    
//     // Write a .sol file
//     filespec = "lc_name1";
//     test_write(filespec);
}

void
test_listen()
{

    LcShm lc;
    char *shmaddr;
    
    string con1 = "localhost:lc_reply";
    if (lc.connect("lc_reply")) {
        runtest.pass("LcShm::connect()");
    } else {
        runtest.fail("LcShm::connect()");
    }
    
    //
    shmaddr = reinterpret_cast<char*>(lc.begin());
    if (shmaddr == 0) {
        runtest.unresolved("LcShm::begin()");
        return;
    } else {
        runtest.pass("LcShm::begin()");
    }
    
    char *addr = shmaddr + LC_LISTENERS_START;
    memset(addr, 0, 1024);
    
    lc.close();

    // Now reconnect to the memory segment, and see if our name
    // is set.
    if (lc.connect(con1)) {
        runtest.pass("LcShm::connect()");
    } else {
        runtest.fail("LcShm::connect()");
    }
    
    if (strcmp(addr, "localhost:lc_reply") == 0) {
        runtest.pass("LcShm::addListener(lc_reply)");
    } else {
        runtest.fail("LcShm::addListener(lc_reply)");
    }
    
    string con2 = "localhost:lc_name1";
    if (lc.connect(con2)) {
        runtest.pass("LcShm::connect(lc_name1)");
    } else {
        runtest.fail("LcShm::connect(lc_name1)");
    }

    if (strcmp(addr, "localhost:lc_reply") == 0) {
        runtest.pass("LcShm::addListener(lc_reply)");
    } else {
        runtest.fail("LcShm::addListener(lc_reply)");
    }

    // Aftyer removing a listener, everything gets moved up
    // in the table. The last element gets zero'd out, so
    // we don't have duplicate entries.
    lc.removeListener(con1);
    if ((strcmp(addr, "localhost:lc_name1") == 0)
    && (addr[con1.size() + 1] == 0)
    && (addr[con1.size() + 2] == 0)
    && (addr[con1.size() + 3] == 0)) {
        runtest.pass("LcShm::removeListener(lc_reply)");
    } else {
        runtest.fail("LcShm::removeListener(lc_reply)");
    }
    
    Listener list(reinterpret_cast<uint8_t *>(shmaddr));
    vector<string>::const_iterator it;
    auto_ptr< vector<string> > listeners ( list.listListeners() );
    if (listeners->size() == 0) {
        cout << "Nobody is listening" << endl;
    } else {
        for (it=listeners->begin(); it!=listeners->end(); ++it) {
            string str = *it;
	    if ((str[0] != ':') || (dbglogfile.getVerbosity() > 0)) {
		cout << " Listeners: " << str << endl;
	    }
        }
    }
    
}

void
test_data()
{
    LcShm lcs;
//  LcShm lcs=LcShm();
    char *shmaddr;

    const string con1 = "localhost:WeBuildTheseOOOOOOOOOOOOOOOOO";
    if (lcs.connect(con1)) {
        runtest.pass("LcShm::connect(localhost:lc_reply)");
    } else {
        runtest.fail("LcShm::connect(localhost:lc_reply)");
    }

    shmaddr = reinterpret_cast<char*>(lcs.begin());     // for gdb

    Element *el;
    vector<cygnal::Element *> els;

#if 0
    // Apparently this constructor no longer exists.

    el = new Element(true, 123.456, 987.654, "IAmReplyingNow");
//    el->dump();
    els.push_back(el);
    delete el;    
#endif
#if 0
    // 
    el = new Element(true);
    els.push_back(el);

    el = new Element(12.34);
    els.push_back(el);

    el = new Element(12.34);
    els.push_back(el);

    string str = "IAmReplyingNow";
    el = new Element(str);
    els.push_back(el);
#endif
    
    string str = "Volume Level 10 ";
    el = new Element(str);
    els.push_back(el);

    // Send the AMF objects
    const std::string localS="localhost";
  lcs.send(con1, localS, els);
//    system("ipcs");
//    system("dumpshm -i");
    sleep(3);
    delete el;
}

void
load_data()
{

    LcShm lc;
    char *shmaddr;

    string con1 = "lc_reply";
    if (!lc.connect(con1)) {
        if (errno == EACCES) {
            runtest.untested("Couldn't map input file, permission problems!!");
        } else {
            runtest.unresolved("LcShm::connect()");
        }
        exit(0);
    }

    shmaddr = reinterpret_cast<char*>(lc.begin());
//    if (memcmp(shmaddr, con1.c_str():
    // Since this is a test case, populate the memory with known good data
    string srcdir = SRCDIR;
    srcdir += "/segment.raw";
    int fd = ::open(srcdir.c_str(), O_RDONLY);
    void *dataptr = mmap(0, 64528, PROT_READ, MAP_SHARED, fd, 0);

    if (dataptr != (void*)-1) {
        memcpy(shmaddr, dataptr, 64528);
    } else {
        if (errno == EACCES) {
            runtest.unresolved("Couldn't map input file, permission problems!!");
        } else {
            runtest.unresolved("Couldn't map input file!");
            log_debug("Error was: %s", strerror(errno));
        }
        exit(0);
    }

    ::close(fd);
}

void
test_read()
{

    LcShm lc;
    char *shmaddr;

    string con1 = "lc_reply";
    if (lc.connect(con1)) {
        runtest.pass("LcShm::connect()");
    } else {
        runtest.fail("LcShm::connect()");
    }

    shmaddr = reinterpret_cast<char*>(lc.begin());
//    if (memcmp(shmaddr, con1.c_str():
    // Since this is a test case, populate the memory with known good data
    string srcdir = SRCDIR;
    srcdir += "/segment.raw";
    int fd = ::open(srcdir.c_str(), O_RDONLY);
    void *dataptr = mmap(0, 64528, PROT_READ, MAP_SHARED, fd, 0);
#if 1
    if (dataptr != (void*)-1) {
        memcpy(shmaddr, dataptr, 64528);
    } else {
        cerr << "ERROR: couldn't map input file!" << endl;
    }
#endif
    ::close(fd);
    
    Listener list(reinterpret_cast<uint8_t *>(shmaddr));
    vector<string>::const_iterator it;
    auto_ptr< vector<string> > listeners ( list.listListeners() );
    if (listeners->size() == 0) {
        cout << "Nobody is listening" << endl;
    } else {
        for (it=listeners->begin(); it!=listeners->end(); ++it) {
            string str = *it;
	    if ((str[0] != ':') || (dbglogfile.getVerbosity() > 0)) {
		cout << " Listeners: " << str << endl;
	    }
        }
    }

#if 0
    string str = "localhost:lc_name1";
    if (list.findListener(str)) {
        runtest.pass("LcShm::findListener()");
    } else {
        runtest.fail("LcShm::findListener()");
    }
#endif
    
//    list.addListener(filespec);
    listeners = list.listListeners(); // will delete former listener list
    if (listeners->empty()) {
        cout << "Nobody is listening" << endl;
    } else {
        for (it=listeners->begin(); it!=listeners->end(); ++it) {
            string str = *it;
	    if ((str[0] != ':') || (dbglogfile.getVerbosity() > 0)) {
		cout << " Listeners: " << str << endl;
	    }
        }
    }

//    boost::uint8_t *ptr = lc.parseHeader(reinterpret_cast<boost::uint8_t *>(shmaddr));
//    vector<cygnal::Element *> ellist = lc.parseBody(ptr);
//    cout << "# of AMF Elements in file: " << ellist.size() << endl;
//    lc.dump();
    lc.close();

    // cleanup
//    delete ptr;
}

void
test_write()
{
#if 0
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
        (memcmp(el.data, &dub, AMF0_NUMBER_SIZE) == 0) &&
        (*((double *)el.data) == dub) &&
        (el.length == AMF0_NUMBER_SIZE)) {
        runtest.pass("gain set");
    } else {
        runtest.fail("gain set");
    }

    boost::uint8_t *foo = amf_obj.encodeVariable(el); 
    char *ptr = (char *)amf_obj.extractVariable(&newel, foo);
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
        (memcmp(el.data, &dub, AMF0_NUMBER_SIZE) == 0) &&
        (*((double *)el.data) == dub) &&
        (el.length == AMF0_NUMBER_SIZE)) {
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
        (memcmp(el.data, &dub, AMF0_NUMBER_SIZE) == 0) &&
        (*((double *)el.data) == dub) &&
        (el.length == AMF0_NUMBER_SIZE)) {
        runtest.pass("localSecPathTime set");
    } else {
        runtest.fail("localSecPathTime set");
    }

//    sol.dump();
    // now write the data to disk
    sol.writeFile(filespec, "settings");
#endif
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

int main(int /*argc*/, char **/* argv[]*/)
{
  // nop
  return 0;  
}

#endif

#if 0 // { // what are these Listeners ?

    Listener::setBaseAddress(Shm::begin());
        
    string str1 = "HelloWorld";
    addListener(str1);
    str1 = "GutenTag";
    addListener(str1);
    str1 = "Aloha";
    addListener(str1);
    
    vector<string>::const_iterator it;
    auto_ptr< vector<string> > listeners ( listListeners() );
    if (listeners->empty()) {
        log_debug("Nobody is listening");
    } else {
        log_debug("There are %d", listeners->size());
        for (it=listeners->begin(); it!=listeners->end(); it++) {
            string str = *it;
            log_debug("Listeners: %s", str.c_str());
        }
    }

    str1 = "HelloWorld";
    removeListener(str1);
    listeners.reset( listListeners() );
    log_debug("There are %d", listeners->size());
    for (it=listeners->begin(); it != listeners->end(); it++) {
        string str = *it;
        log_debug("Listeners: %s", str.c_str());
    }
    
#endif // }

// FIXME: more tests! Here's all the methods:
//
// gnash::LcShm::parseHeader(unsigned char*)
// gnash::LcShm::formatHeader(std::string const&, std::string const&, bool)
// gnash::LcShm::dump()
// gnash::LcShm::send(std::string const&, std::string const&, std::vector<cygnal::Element*, std::allocator<cygnal::Element*> >&)
// gnash::LcShm::close()
// gnash::LcShm::connect(std::string const&)
// gnash::LcShm::connect(int)
// gnash::LcShm::parseBody(unsigned char*)
// gnash::LcShm::LcShm(unsigned char*)
// gnash::LcShm::LcShm(int)
// gnash::LcShm::LcShm()
// gnash::LcShm::LcShm(unsigned char*)
// gnash::LcShm::LcShm(int)
// gnash::LcShm::LcShm()
// gnash::LcShm::~LcShm()
// gnash::LcShm::~LcShm()
// gnash::Listener::addListener(std::string const&)
// gnash::Listener::findListener(std::string const&)
// gnash::Listener::listListeners()
// gnash::Listener::removeListener(std::string const&)
// gnash::Listener::Listener(unsigned char*)
// gnash::Listener::Listener()
// gnash::Listener::Listener(unsigned char*)
// gnash::Listener::Listener()
// gnash::Listener::~Listener()
// gnash::Listener::~Listener()
