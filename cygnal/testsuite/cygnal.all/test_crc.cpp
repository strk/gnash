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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_STDARG_H
#include <cstdarg>
#endif

#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include "GnashSystemIOHeaders.h"
#endif

#include <regex.h>
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <string>
#include "log.h"
#include "crc.h"

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

using namespace std;
using namespace gnash;
using namespace cygnal;

TestState runtest;
LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main (int /*argc*/, char** /*argv*/) {
    CRcInitFile& crc = CRcInitFile::getDefaultInstance();

    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();
    
    // Parse the test config file
    if (crc.parseFile("cygnalrc")) {
        runtest.pass ("crc.parseFile()");
    } else {
        runtest.fail ("crc.parseFile()");
    }
    
    if (crc.useActionDump()) {
        runtest.fail ("useActionDump");
    } else {
        runtest.pass ("useActionDump");
    }

    if (crc.useParserDump()) {
        runtest.fail ("useParserDump");
    } else {
        runtest.pass ("useParserDump");
    }
    
    if (crc.verbosityLevel() == 11) {
        runtest.pass ("verbosityLevel");
    } else {
        runtest.fail ("verbosityLevel");
    }
    
    if (crc.getDebugLog() == "/tmp/cygnal-dbg.log") {
        runtest.pass ("getDebugLog");
    } else {
        runtest.fail ("getDebugLog");
    }

    if (crc.getPortOffset() == 4000) {
        runtest.pass ("getPortOffset");
    } else {
        runtest.fail ("getPortOffset");
    }

    if (crc.getAdminFlag() == false) {
        runtest.pass ("getAdminFlag");
    } else {
        runtest.fail ("getAdminFlag");
    }

    if (crc.getTestingFlag() == true) {
        runtest.pass ("getTestingFlag");
    } else {
        runtest.fail ("getTestingFlag");
    }

    if (crc.getThreadingFlag() == true) {
        runtest.pass ("getThreadingFlag");
    } else {
        runtest.fail ("getThreadingFlag");
    }

    if (crc.getFDThread() == 10) {
        runtest.pass ("getFDThread");
    } else {
        runtest.fail ("getFDThread");
    }

    crc.dump();
}

