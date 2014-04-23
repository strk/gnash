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
// 

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <vector>
#include <string>

#include "log.h"
#include "gmemory.h"

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

TestState runtest;

long *test_leak();
long *test_noleak();

using namespace std;
using namespace gnash;

const int INTARRAYSIZE = 10;

int
main (int /*argc*/, char** /*argv*/) {
    RcInitFile& rc = RcInitFile::getDefaultInstance();

    LogFile& dbglogfile = LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();

    int diff = 0;
    
    // Parse the test config file
    if (rc.parseFile("gnashrc")) {
        runtest.pass ("rc.parseFile()");
    } else {
        runtest.fail ("rc.parseFile()");
    }

// If we don't have support for mallinfo(), this code is useless
#if HAVE_MALLINFO
    Memory mem;

    mem.startStats();
    mem.addStats(__LINE__);             // take a sample
    mem.addStats(__LINE__);             // take a sample
    diff = mem.diffStats();
    if (mem.diffStats() == 0) {
        runtest.pass("No allocations yet");
    } else {
        runtest.fail("No allocations yet");
    }
    
    Memory m1;
    mem.addStats(__LINE__);             // take a sample
    diff = mem.diffStats();
//    cerr << "Memory::Memory: " << diff << endl;
    if ((diff >= 8) || (diff <= 16)) {
        runtest.pass("Memory::Memory");
    } else {
        runtest.fail("Memory::Memory");
    }
    
    if (mem.diffStamp() > 0) {
        runtest.pass("Memory::diffStamp()");
    } else {
        runtest.fail("Memory::diffStamp()");
    }

    if (mem.diffStats() > 0) {
        runtest.pass("Memory::diffStats()");
    } else {
        runtest.fail("Memory::diffStats()");
    }

    char *x = new char[120];
    mem.addStats(__LINE__);             // take a sample
    diff = mem.diffStats();
//    cerr << "Buffer allocation: " << diff << endl;
    if ((diff >= 104) && (diff <= 136)) {
        runtest.pass("Buffer allocation");
    } else {
        runtest.fail("Buffer allocation");
    }
    
    vector<string> sv;
    sv.push_back("Hello World");
    mem.addStats(__LINE__);             // take a sample
    diff = mem.diffStats();
//    cerr << "First string allocated: " << diff << endl;
    if ((diff >= 40) && (diff <= 48)) {
        runtest.pass("First string allocated");
    } else {
        runtest.fail("First string allocated");
    }

    sv.push_back("Aloha");
    delete[] x;
    mem.addStats(__LINE__);             // take a sample
    diff = mem.diffStats();
//    cerr << "Second string allocated: " << diff << endl;
    if ((diff >= -104) && (diff <= -96)) {
        runtest.pass("Second string allocated");
    } else {
        runtest.fail("Second string allocated");
    }

    sv.push_back("Guten Tag");
    mem.addStats(__LINE__);             // take a sample
    diff = mem.diffStats();
//    cerr << "Third string allocated: " << diff << endl;
    if ((diff >= 40) && (diff <= 48)){
        runtest.pass("Third string allocated");
    } else {
        runtest.fail("Third string allocated");
    }

    mem.startCheckpoint();
    test_leak();
    if (mem.endCheckpoint()) {
        runtest.fail("leak");
    } else {
        runtest.pass("leak");
    }    
    mem.addStats(__LINE__);             // take a sample
    if (mem.diffStats() == 32) {
        runtest.pass("test_leak");
    } else {
        runtest.fail("test_leak");
    }

    mem.startCheckpoint();
    test_noleak();
    mem.addStats(__LINE__);             // take a sample
    if (mem.endCheckpoint()) {
        runtest.pass("noleak");
    } else {
        runtest.fail("noleak");
    }
    diff = mem.diffStats();
    if ((diff >= 0) && (diff <= 8)) {
        runtest.pass("test_noleak");
    } else {
        runtest.fail("test_noleak");
    }
    
    mem.endStats();

//    mem.dump();

    mem.analyze();
#else
    runtest.untested("No support for mallinfo()");
#endif // end of HAVE_MALLINFO
    return 0;
}

// Allocate memory and forget to clean it up.
long *
test_leak()
{
    long *x = new long[INTARRAYSIZE];
    for (int i=0; i<INTARRAYSIZE; i++) {
        x[i] = i;
    }
    return x;
}

// Clean up after the other testcase
long *
test_noleak()
{
    long *x = test_leak();
    delete[] x;

    return 0;
}

