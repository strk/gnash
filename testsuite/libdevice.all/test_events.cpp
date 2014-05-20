// 
//   Copyright (C) 2010, 2011, 2012 Free Software Foundation, Inc
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

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include <regex.h>
#include <unistd.h>
#include <signal.h>

#include <boost/assign/list_of.hpp>

#include "log.h"
#include "dejagnu.h"

#include "GnashDevice.h"
#include "InputDevice.h"

TestState runtest;

using namespace gnash;
using namespace std;

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

// Trap a Sig Alarm, so this test doesn't hang forever if there
// is no input.
static void
alarm_handler (int sig)
{
    cerr << endl << "Ending test beacuse of no input. This is normal if" << endl
         << "running this in an automated fashion, ie... \"make check\"" << endl
         << "This is an interactive test, not a unit or regression test" << endl;
    exit(0);
}

int
main(int argc, char *argv[])
{
    dbglogfile.setVerbosity();
    
    struct sigaction act;
    act.sa_handler = alarm_handler;
    sigaction (SIGALRM, &act, NULL);

    bool loop = false;
    std::vector<std::shared_ptr<InputDevice> > inputs
        = InputDevice::scanForDevices();
    cerr << "Found " << inputs.size() << " input devices" << endl;
    if (inputs.empty()) {
        runtest.fail("InputDevice::scanForDevices()");
    } else {
        runtest.pass("InputDevice::scanForDevices()");
        loop = true;
    }    
    
    std::vector<std::shared_ptr<InputDevice> >::iterator it;
    
    // check input devices
    for (it = inputs.begin(); it != inputs.end(); ++it) {
        std::shared_ptr<InputDevice> id = *it;
        cerr << "Found " << id->id() << " device" << endl;
        if (id->init()) {
            runtest.pass("InputDevice::init()");
            id->setScreenSize(1024, 768);        
        } else {
            runtest.fail("InputDevice::init()()");
        }
    }

    cerr << "Starting inactivity timeout to 10 seconds..." << endl;
    alarm(10);
    
    // This loops endlessly at the frame rate
    while (loop) {  
        std::vector<std::shared_ptr<InputDevice> >::iterator it;
        // // check input devices
        for (it = inputs.begin(); it != inputs.end(); ++it) {
            std::shared_ptr<InputDevice> id = *it;
            if (id->check()) {
                // FIXME: process the input data
                std::shared_ptr<InputDevice::input_data_t> ie = id->popData();
#if 0
                if (ie) {
                    cerr << "Got data: " << ie->pressed;
                    cerr << ", " << ie->key << ", " << ie->modifier;
                    cerr << ", " << ie->x << ", " << ie->y << endl;
                    // Range check and convert the position
                    std::unique_ptr<int[]> coords =
                        MouseDevice::convertCoordinates(ie->x, ie->y, 1024, 768);
                    cerr << "X = " << coords[0] << endl;
                    cerr << "Y = " << coords[1] << endl;
                }
#endif
            } else {
                std::cerr << ".";
            }
        }
        
        // wait the "heartbeat" interval. The default mouse update rate is
        // only 100 samples/sec. so why rush...
        usleep(1000000);
    }
    
    std::cerr << std::endl;
    return 0;
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
