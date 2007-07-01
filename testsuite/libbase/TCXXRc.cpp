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
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "rc.h"

#ifdef HAVE_STDARG_H
#include <cstdarg>
#endif

#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <regex.h>
#include <cstdio>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <string>

#ifdef HAVE_DEJAGNU_H
#include "dejagnu.h"
#else
#include "check.h"
#endif

using namespace std;
using namespace gnash;

bool gofast = false;		// FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,
bool nodelay = false;           // FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,

#ifdef HAVE_LIBXML
extern int xml_fd;		// FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.
#endif // HAVE_LIBXML

TestState runtest;

int
main (int /*argc*/, char** /*argv*/) {
    RcInitFile rc;

    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();
    
    // Parse the test config file
    if (rc.parseFile("gnashrc")) {
        runtest.pass ("rc.parseFile()");
    } else {
        runtest.fail ("rc.parseFile()");
    }
    
    // By default, use a splash screen
    if (rc.useSplashScreen()) {
        runtest.pass ("useSplashScreen default");
    } else {
        runtest.fail ("useSplashScreen default");
    }

    // By default, limit access to the local host only
    if (rc.useLocalDomain()) {
        runtest.pass ("useLocalDomain default");
    } else {
        runtest.fail ("useLocalDomain default");
    }
    if (rc.useLocalHost()) {
        runtest.pass ("useLocalHost default");
    } else {
        runtest.fail ("useLocalHost default");
    }
    
    if (rc.useActionDump()) {
        runtest.fail ("useActionDump");
    } else {
        runtest.pass ("useActionDump");
    }

    if (rc.useParserDump()) {
        runtest.fail ("useParserDump");
    } else {
        runtest.pass ("useParserDump");
    }

    if (rc.useWriteLog()) {
        runtest.pass ("useWriteLog");
    } else {
        runtest.fail ("useWriteLog");
    }

    if (rc.useDebugger()) {
        runtest.fail ("useDebugger");
    } else {
        runtest.pass ("useDebugger");
    }

    if (rc.getTimerDelay() == 50) {
        runtest.pass ("getTimerDelay");
    } else {
        runtest.fail ("getTimerDelay");
    }

    if (rc.verbosityLevel() == 1) {
        runtest.pass ("verbosityLevel");
    } else {
        runtest.fail ("verbosityLevel");
    }

    if (rc.useSound() == 0) {
        runtest.pass ("useSound");
    } else {
        runtest.fail ("useSound");
    }

    if (rc.usePluginSound() == 0) {
        runtest.pass ("usePluginSound");
    } else {
        runtest.fail ("usePluginSound");
    }

    if (rc.enableExtensions() == 1) {
        runtest.pass ("enableExtensions");
    } else {
        runtest.fail ("enableExtensions");
    }

    if (rc.startStopped() == 1) {
        runtest.pass ("startStopped");
    } else {
        runtest.fail ("startStopped");
    }

    std::vector<std::string> whitelist = rc.getWhiteList();
    if (whitelist.size()) {
        if ((whitelist[0] == "www.doonesbury.com")
            && (whitelist[1] == "www.cnn.com")
            && (whitelist[2] == "www.9news.com")) {
            runtest.pass ("rc.getWhiteList()");
        } else {
            runtest.fail ("rc.getWhiteList()");
        }
        runtest.pass ("rc.getWhiteList() has elements");
    } else {
        runtest.fail ("rc.getWhiteList() doesn't has elements");        
    }
        
    std::vector<std::string> blacklist = rc.getBlackList();
    if (blacklist.size()) {
        if ((blacklist[0] == "www.doubleclick.com")
            && (blacklist[1] == "www.ebay.com")) {
            runtest.pass ("rc.getBlackList()");
        } else {
            runtest.fail ("rc.getBlackList()");
        }
        runtest.pass ("rc.getBlackList() has elements");
    } else {
        runtest.fail ("rc.getBlackList() doesn't has elements");        
    }
}

