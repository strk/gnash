// 
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>

#include "dejagnu.h"
#include "log.h"

#include "gst/VideoInputGst.h"
#include <vector>


using namespace gnash;
using namespace media;
using namespace gst;
using namespace std;

static TestState runtest;

static string infile;

void test_client();

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main()
{   
    test_client();
    return 0;
}

void test_client()
{

    std::vector<std::string> names;
    VideoInputGst::getNames(names);

	if (names.empty()) {
		runtest.fail("the video vector was not created by find_vid_devs");
	} else {
		runtest.pass("the video vector was created");
	}
	

	if (names.at(0) != "videotest") {
		runtest.fail("the zeroth element doesn't contain the right product name info");
	} else {
		runtest.pass("the zeroth vid_vect element contains the right product name info");
	} 
	
    VideoInputGst vig;

	int devselect = vig.makeWebcamDeviceSelection();
    
    bool ret = vig.setWebcam(devselect);
    if (!ret) {
        runtest.fail("the transferToPrivate function didn't return anything");
    } else {
        runtest.pass("the transferToPrivate function returned a GnashWebcamPrivate ptr");
    }
	if (devselect == 0) {
        if (vig.name() != "videotest") {
            runtest.fail("webcam doesn't have the right _deviceName value");
        } else {
            runtest.pass("webcam has the right _deviceName value");
        }
    } else {
        //real camera source tests
        if (vig.name().empty()) {
            runtest.fail("_deviceName isn't set in GnashWebcamPrivate class");
        } else {
            runtest.pass("_deviceName is set in GnashWebcamPrivate class");
        }
    }
    
    bool result = vig.init();
    if (result != true) {
        runtest.fail("Webcam inititalization");
    } else {
        runtest.pass("Webcam initialization was okay");
    }
    
    result = false;
    result = vig.webcamCreateSaveBin();
    if (result != true) {
        runtest.fail("webcamCreateSaveBin() reported an error");
    } else {
        runtest.pass("webcamCreateSaveBin() didn't report any errors");
    }
    result = false;
    result = vig.webcamMakeVideoSaveLink();
    if (result != true) {
        runtest.fail("webcamMakeVideoSaveLink() reported an error");
    } else {
        runtest.pass("webcamMakeVideoSaveLink() didn't report errors");
    }
    
    //end of setup tests, now startup the webcamPipeline, run for a few seconds
    //and then make sure there is a file present after running
    result = vig.play();
    if (result != true) {
        runtest.fail("play() function reported an error");
    } else {
        runtest.pass("play() function reported no errors");
    }
    
    g_print("        NOTE: the output window will close automatically\n");
    
    sleep(2);
    result = vig.stop();
    if (result != true) {
        runtest.fail("webcamStop() function reported an error");
    } else {
        runtest.pass("webcamStop() function reported no errors");
    }

    struct stat st;
    std::string file = "./vidoutput.ogg";
    
    if (stat(file.c_str(), &st) == 0) {
        runtest.pass("vidoutput.ogg file is in testsuite/libmedia.all");
        if (st.st_blocks == 0) {
            runtest.fail("the output file is there, but there's no information in it!");
        } else {
            runtest.pass("the output file has data in it");
        }
    } else {
        runtest.fail("there's no output video file in testsuite/libmedia.all");
    }
    
    //delete the old vidoutput.ogg file
    if (unlink(file.c_str()) == 0) {
        g_print("        NOTE: deleting output file...\n");
    }
    
    result = vig.webcamBreakVideoDisplayLink();
    if (result != true) {
        runtest.fail("the webcamBreakVideoDisplayLink() function reported an error");
    } else {
        runtest.pass("the webcamBreakVideoDisplayLink() function reported no errors");
    }
    
    result = vig.play();
    if (result != true) {
        runtest.fail("play() reported errors after breaking display link");
    } else {
        runtest.pass("play() still works after breaking display link");
    }
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = vig.stop();
    if (result != true) {
        runtest.fail("webcamStop() reported errors after breaking display link");
    } else {
        runtest.pass("webcamStop() reported success after breaking display link");
    }
    
    if (stat(file.c_str(), &st) == 0) {
        runtest.pass("the a new vidoput.ogg file was created");
    } else {
        runtest.fail("there's no new vidoutput.ogg file!");
    }
    
    //delete the old vidoutput.ogg file
    if (unlink(file.c_str()) == 0) {
        g_print("        NOTE: deleting output file...\n");
    }
    
    result = vig.webcamBreakVideoSaveLink();
    if (result != true) {
        runtest.fail("breaking the videoSaveLink failed");
    } else {
        runtest.pass("breaking the videoSaveLink was successful");
    }
    
    result = vig.webcamMakeVideoDisplayLink();
    if (result != true) {
        runtest.fail("making videosrc -> display link failed");
    } else {
        runtest.pass("making videosrc -> display link succeeded");
    }

    result = vig.play();
    if (result != true) {
        runtest.fail("play() reported errors after relinking display");
    } else {
        runtest.pass("play() still works after relinking display");
    }
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = vig.stop();
    if (result != true) {
        runtest.fail("webcamStop() reported errors after breaking display link");
    } else {
        runtest.pass("webcamStop() reported success after breaking display link");
    }

    if (stat(file.c_str(), &st) == 0) {
        runtest.fail("a vidoutput.ogg file was created, and it shouldn't be");
    } else {
        runtest.pass("no vidoutput.ogg file wasn't created");
    }
    
    result = vig.webcamMakeVideoSaveLink();
    if (result != true) {
        runtest.fail("webcamMakeVideoSaveLink() reported an error");
    } else {
        runtest.pass("webcamMakeVideoSaveLink() reported no errors");
    }
    
    result = vig.play();
    if (result != true) {
        runtest.fail("play() reported errors");
    } else {
        runtest.pass("play() reported no errors");
    }
    
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = vig.stop();
    if (result != true) {
        runtest.fail("webcamStop() reported errors after breaking display link");
    } else {
        runtest.pass("webcamStop() reported success after breaking display link");
    }
    
    if (stat(file.c_str(), &st) == 0) {
        runtest.pass("the a new vidoput.ogg file was created");
    } else {
        runtest.fail("there's no new vidoutput.ogg file!");
    }
    //delete the old vidoutput.ogg file
    if (unlink(file.c_str()) == 0) {
        g_print("        NOTE: deleting output file...\n");
    }
    
    //end unit tests
    
    //tests more similar to execution flow
    gst::VideoInputGst* video = new VideoInputGst;
    if (video == NULL) {
        runtest.fail("new VideoInputGst didn't work");
    } else { 
        runtest.pass("new VideoInputGst returned a value");
    }
	
    //get global webcam reference for use below
    result = video->webcamMakeVideoDisplayLink();
    if (result != true) {
        runtest.fail("webcamMakeVideoDisplayLink reported errors");
    } else {
        runtest.pass("webcamMakeVideoDisplayLink reported no errors");
    }
    
    result = video->play();
    if (result != true) {
        runtest.fail("play reported errors");
    } else {
        runtest.pass("play reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = video->stop();
    if (result != true) {
        runtest.fail("webcamStop reported errors");
    } else {
        runtest.pass("webcamStop reported no errors");
    }
    
    g_print("        NOTE: changing values (display window should be bigger)....\n");

    video->requestMode(800, 600, 30, true);
    
    result = video->play();
    if (result != true) {
        runtest.fail("play reported errors");
    } else {
        runtest.pass("play reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = video->stop();
    if (result != true) {
        runtest.fail("webcamStop reported errors");
    } else {
        runtest.pass("webcamStop reported no errors");
    }
    
    result = video->webcamMakeVideoSaveLink();
    if (result != true) {
        runtest.fail("webcamMakeVideoSaveLink reported errors");
    } else {
        runtest.pass("webcamMakeVideoSaveLink reported no errors");
    }
    
    result = video->play();
    if (result != true) {
        runtest.fail("play reported errors");
    } else {
        runtest.pass("play reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = video->stop();
    if (result != true) {
        runtest.fail("webcamStop reported errors");
    } else {
        runtest.pass("webcamStop reported no errors");
    }
    if (stat(file.c_str(), &st) == 0) {
        runtest.pass("the a new vidoput.ogg file was created");
    } else {
        runtest.fail("there's no new vidoutput.ogg file!");
    }
    //delete the old vidoutput.ogg file
    if (unlink(file.c_str()) == 0) {
        g_print("        NOTE: deleting output file...\n");
    }
    
    result = video->webcamBreakVideoDisplayLink();
    if (result != true) {
        runtest.fail("webcamBreakVideoDisplayLink reported errors");
    } else {
        runtest.pass("webcamBreakVideoDisplayLink reported no errors");
    }
    
    result = video->play();
    if (result != true) {
        runtest.fail("play reported errors");
    } else {
        runtest.pass("play reported no errors");
    }
    
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = video->stop();
    if (result != true) {
        runtest.fail("webcamStop reported errors");
    } else {
        runtest.pass("webcamStop reported no errors");
    }
    if (stat(file.c_str(), &st) == 0) {
        runtest.pass("the a new vidoput.ogg file was created");
    } else {
        runtest.fail("there's no new vidoutput.ogg file!");
    }

    //delete the old vidoutput.ogg file
    if (unlink(file.c_str()) == 0) {
        g_print("        NOTE: deleting output file...\n");
    }
    
    result = video->webcamMakeVideoDisplayLink();
    if (result != true) {
        runtest.fail("webcamMakeVideoDisplayLink failed after breaking the link");
    } else {
        runtest.pass("webcamMakeVideoDisplayLink reported no errors");
    }
    
    result = video->webcamBreakVideoSaveLink();
    if (result != true) {
        runtest.fail("webcamBreakVideoSaveLink function reported errors");
    } else {
        runtest.pass("webcamBreakVideoSaveLink function reported no errors");
    }
    
    result = video->play();
    if (result != true) {
        runtest.fail("play reported errors");
    } else {
        runtest.pass("play reported no errors");
    }
    
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(2);
    
    result = video->stop();
    if (result != true) {
        runtest.fail("webcamStop reported errors");
    } else {
        runtest.pass("webcamStop reported no errors");
    }
    
    if (stat(file.c_str(), &st) == 0) {
        runtest.fail("a vidoutput.ogg file was created, and it shouldn't be");
    } else {
        runtest.pass("no vidoutput.ogg file wasn't created");
    }

}


#else

int
main(int /*argc*/, char /* *argv[]*/)
{
  // nop
    cerr << "This program needs to have DejaGnu installed!" << endl;
    return 0;  
}

#endif
