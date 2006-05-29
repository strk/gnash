// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "rc.h"

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <regex.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <string>

#include "dejagnu.h"

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
main (int argc, char **argv) {
    RcInitFile rc;

    dbglogfile.setVerbosity();
    
    // By default, use a splash screen
    if (rc.useSplashScreen()) {
        runtest.pass ("useSplashScreen default");
    } else {
        runtest.fail ("useSplashScreen default");
    }

    // By default, limit access to the local host only
    if (rc.useLocalDomain()) {
        runtest.fail ("useLocalDomain default");
    } else {
        runtest.pass ("useLocalDomain default");
    }
    if (rc.useLocalHost()) {
        runtest.pass ("useLocalHost default");
    } else {
        runtest.fail ("useLocalHost default");
    }

    // Parse the test config file
    if (rc.parseFile("gnashrc")) {
        runtest.pass ("rc.parseFile()");
    } else {
        runtest.fail ("rc.parseFile()");
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
        
//     for (size_t i = 0; i < whitelist.size(); i++) {
//         dbglogfile << whitelist[i] << endl;
//     }
        
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
//     for (size_t i = 0; i < blacklist.size(); i++) {
//         dbglogfile << blacklist[i] << endl;
//     }
    
}

