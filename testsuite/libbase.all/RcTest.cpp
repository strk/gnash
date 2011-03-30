// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "log.h"
#include "rc.h"

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

TestState runtest;

int
main (int /*argc*/, char** /*argv*/) {
    RcInitFile& rc = RcInitFile::getDefaultInstance();

    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    dbglogfile.setVerbosity();

    // Default rendering quality is driven by SWF
    if (rc.qualityLevel() == -1) {
        runtest.pass ("rc.qualityLevel() == -1");
    } else {
        runtest.fail ("rc.qualityLevel() != -1");
    }
    
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

    if (rc.getFlashVersionString() == "GSH 9,0,99,0") {
        runtest.pass ("getFlashVersionString");
    } else {
        runtest.fail ("getFlashVersionString");
    }

    if (rc.getDebugLog() == "/tmp/gnash-dbg.log") {
        runtest.pass ("getDebugLog");
    } else {
        runtest.fail ("getDebugLog");
    }

    if (rc.startStopped() == 1) {
        runtest.pass ("startStopped");
    } else {
        runtest.fail ("startStopped");
    }

    if (rc.getStreamsTimeout() == 1.5) {
        runtest.pass ("streamsTimeout");
    } else {
        runtest.fail ("streamsTimeout");
    }

    if (rc.insecureSSL()) {
        runtest.pass ("insecureSSL");
    } else {
        runtest.fail ("insecureSSL");
    }

    if (rc.getSOLSafeDir().size() > 0) {
        runtest.pass ("getSOLSafeDir");
    } else {
        runtest.fail ("getSOLSafeDir");
    }

    // Parsed gnashrc sets qualityLevel to 0 (low)
    if (rc.qualityLevel() == 0) {
        runtest.pass ("rc.qualityLevel() == 0");
    } else {
        runtest.fail ("rc.qualityLevel() != 0");
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
        runtest.fail ("rc.getWhiteList() doesn't have elements");        
    }

    if (rc.getLCShmKey() == 0xdd3adabd) {
        runtest.pass ("rc.getLCShmKey() == 0xabcd1234");
    } else {
        runtest.fail ("rc.getLCShmKey() != 0xabcd1234");
    }
        
    if (rc.getRootCert() == "testrootcert.pem") {
        runtest.pass ("rc.getRootCert() == testrootcert.pem");
    } else {
        runtest.fail ("rc.getRootCert() != testrootcert.pem");
    }

    if (rc.getCertFile() == "testclient.pem") {
        runtest.pass ("rc.getCertFile() == testclient.pem");
    } else {
        runtest.fail ("rc.getCertFile() != testclient.pem");
    }

    if (rc.getCertDir() == "/test/etc/pki/tls/") {
        runtest.pass ("rc.getCertDir() == /test/etc/pki/tls/");
    } else {
        runtest.fail ("rc.getCertDir() != /test/etc/pki/tls/");
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
        runtest.fail ("rc.getBlackList() doesn't have elements");        
    }

    const std::vector<std::string>& localSandbox = rc.getLocalSandboxPath();
    if (localSandbox.size() != 1) {
        runtest.fail ("rc.getLocalSandboxPath() doesn't have 1 element after set");
    } else {
        if ( localSandbox[0] == "/tmp/p1" )
        {
            runtest.pass ("set localSandbox");
        }
        else
        {
            runtest.fail ("rc.getLocalSandboxPath() doesn't have the correct first element after set");
        }
    }

    if (rc.getURLOpenerFormat() == "lynx %u") {
        runtest.pass ("getURLOpenerFormat");
    } else {
        runtest.fail ("getURLOpenerFormat");
    }

    if (rc.getSOLSafeDir() == "/tmp/SharedObjects") {
        runtest.pass ("getSOLSafeDir");
    } else {
        runtest.fail ("getSOLSafeDir");
    }

    if (rc.getSOLReadOnly() == true) {
        runtest.pass ("getSOLReadOnly");
    } else {
        runtest.fail ("getSOLReadOnly");
    }
    
    if (rc.ignoreShowMenu() == false) {
        runtest.pass ("ignoreShowMenu");
    } else {
        runtest.fail ("ignoreShowMenu");
    }

    if ( rc.getRenderer().empty() ) {
        runtest.pass ("getRenderer gives empty string");
    } else {
        runtest.fail ("getRenderer gives " + rc.getRenderer() );
    }

    if ( rc.getMediaHandler().empty() ) {
        runtest.pass ("getMediaHandler gives empty string");
    } else {
        runtest.fail ("getMediaHandler gives " + rc.getMediaHandler() );
    }

    if ( rc.getScriptsTimeout() == 15 ) {
        runtest.pass ("getScriptsTimeout gives 15");
    } else {
        runtest.fail ("getScriptsTimeout doesn't gives 15");
    }

    if ( rc.getScriptsRecursionLimit() == 256 ) {
        runtest.pass ("getScriptsRecursionLimit gives 256");
    } else {
        runtest.fail ("getScriptsRecursionLimit doesn't gives 256");
    }


    // Parse a second file
    if (rc.parseFile("gnashrc-local")) {

	// Test whether blacklist in gnashrc-local is appended
        std::vector<std::string> blacklist = rc.getBlackList();
        if (blacklist.size()) {
            if ((blacklist[2] == "www.gnashdev.org")
                && (blacklist[3] == "www.wikipedia.de")) {
                runtest.pass ("rc.getBlackList(): append");
            } else {
                runtest.fail ("rc.getBlackList(): append");
            }
            runtest.pass ("rc.getBlackList(): has appended elements");
        } else {
            runtest.fail ("rc.getBlackList(): doesn't appended elements");        
        }

        // Test local override of previous whitelist 
        std::vector<std::string> whitelist = rc.getWhiteList();
        if (whitelist.size()) {
            runtest.fail ("rc.getWhiteList(): local override failed");
        } else {
            runtest.pass ("rc.getWhiteList(): local override succeeded");
        }

        // Test local override of previous local sandbox
        const std::vector<std::string>& localSandbox = rc.getLocalSandboxPath();
        if (localSandbox.empty()) {
            runtest.fail ("rc.getLocalSandboxPath() doesn't have elements after append");        
        } else {
            if ( localSandbox.back() == "/tmp/gnash" )
            {
                runtest.pass ("append localSandbox");
            }
            else
            {
                runtest.fail ("rc.getLocalSandboxPath() doesn't have the correct last element after append");
            }
        }

        if ( rc.getRenderer() == std::string("fakeRenderer") ) {
            runtest.pass ("getRenderer gives " + rc.getRenderer() );
        } else {
            runtest.fail ("getRenderer gives " + rc.getRenderer() );
        }

        if ( rc.getMediaHandler() == std::string("fakeMediaHandler") ) {
            runtest.pass ("getMediaHandler gives " + rc.getMediaHandler() );
        } else {
            runtest.fail ("getMediaHandler gives " + rc.getMediaHandler() );
        }

        if ( rc.getScriptsTimeout() == 2 ) {
            runtest.pass ("getScriptsTimeout gives 2");
        } else {
            runtest.fail ("getScriptsTimeout doesn't gives 2");
        }

        if ( rc.getScriptsRecursionLimit() == 32 ) {
            runtest.pass ("getScriptsRecursionLimit gives 32");
        } else {
            runtest.fail ("getScriptsRecursionLimit doesn't gives 32");
        }

    }
}

