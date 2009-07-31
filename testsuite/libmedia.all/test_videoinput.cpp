// 
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

static void test_client();

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main()
{   
    test_client();
    return 0;
}

static void test_client()
{
	//create a test class, call constructor
	gst::VideoInputGst vig;

    vig.findVidDevs();
	std::vector<GnashWebcam*> *vid_vect = vig.getVidVect();

	if (vid_vect->empty() == true) {
		runtest.fail("the video vector was not created by find_vid_devs");
	} else {
		runtest.pass("the video vector was created");
	}
	
	if (vid_vect->at(0) == NULL) {
		runtest.fail("the 0th vid_vect element is not the test source");
	} else {
		runtest.pass("the videotestsrc element was created");
	}
	
	if (vid_vect->at(0)->getElementPtr() == NULL) {
		runtest.fail("the videotestsrc didn't get assigned an element ptr");
	} else {
		runtest.pass("the videotestsrc was assigned an element ptr");
	}
	
	if (g_strcmp0(vid_vect->at(0)->getGstreamerSrc(), "videotestsrc") == 1) {
		runtest.fail("the zeroth element doesn't contain the right source info");
	} else {
		runtest.pass("the zeroth vid_vect element contains the right source info");
	}

	if (g_strcmp0(vid_vect->at(0)->getProductName(), "videotest") == 1) {
		runtest.fail("the zeroth element doesn't contain the right product name info");
	} else {
		runtest.pass("the zeroth vid_vect element contains the right product name info");
	} 
	
    int devselect;
	devselect = vig.makeWebcamDeviceSelection();
    
    GnashWebcamPrivate *webcam = NULL;
    webcam = vig.transferToPrivate(devselect);
    if (webcam == NULL) {
        runtest.fail("the transferToPrivate function didn't return anything");
    } else {
        runtest.pass("the transferToPrivate function returned a GnashWebcamPrivate ptr");
    }
	if (devselect == 0) {
        //videotestsrc tests
        if (g_strcmp0(webcam->_deviceName, "videotest") == 1) {
            runtest.fail("webcam doesn't have the right _deviceName value");
        } else {
            runtest.pass("webcam has the right _deviceName value");
        }
        if (webcam->_webcamDevice != vid_vect->at(0)){
            runtest.fail("_webcamDevice values isn't correct");
        } else {
            runtest.pass("_webcamDevice has the right address");
        }
    } else {
        //real camera source tests
        if (webcam->_deviceName == NULL) {
            runtest.fail("_deviceName isn't set in GnashWebcamPrivate class");
        } else {
            runtest.pass("_deviceName is set in GnashWebcamPrivate class");
        }
        if (webcam->_webcamDevice != vid_vect->at(devselect)) {
            runtest.fail("_webcamDevice isn't set in GnashWebcamPrivate class");
        } else {
            runtest.pass("_webcamDevice has the right address");
        }
    }
    
    gboolean result = false;
    result = vig.webcamCreateMainBin(webcam);
    if (result != true) {
        runtest.fail("the webcamCreateMainBin() function reported an error");
    } else {
        runtest.pass("the webcamCreateMainBin() function isn't reporting errors");
    }
    if (webcam->_pipeline == NULL) {
        runtest.fail("the main pipeline (webcam->_pipeline) wasn't initialized");
    } else {
        runtest.pass("the main pipeline (webcam->_pipeline) was initializied");
    }
    if (webcam->_webcamMainBin == NULL) {
        runtest.fail("the _webcamMainBin wasn't created");
    } else {
        runtest.pass("the _webcamMainBin was created");
        if ((gst_element_get_pad(webcam->_webcamMainBin, "save_queue_src")) == NULL) {
            runtest.fail("save_queue_src ghostpad wasn't created");
        } else {
            runtest.pass("save_queue_src ghostpad was created");
        }
        if ((gst_element_get_pad(webcam->_webcamMainBin, "video_display_queue_src")) == NULL) {
            runtest.fail("video_display_queue_src ghostpad wasn't created");
        } else {
            runtest.pass("video_display_queue_src ghostpad was created");
        }
        if ((gst_bin_get_by_name(GST_BIN(webcam->_pipeline),
            "webcam_main_bin")) == NULL) {
            runtest.fail("webcamMainBin has an unexpected address");
        } else {
            runtest.pass("webcamMainBin's address is set as expected");
        }
    }
    if (webcam->_webcamSourceBin == NULL) {
        runtest.fail("the _webcamSourceBin wasn't created");
    } else {
        runtest.pass("the _webcamSourceBin was created");
        if ((gst_bin_get_by_name(GST_BIN(webcam->_pipeline),
            "video_source")) == NULL) {
            runtest.fail("videoSourceBin has an unexpected address");
        } else {
            runtest.pass("videoSourceBin's address is set as expected");
        }
    }
    if (webcam->_videoSource == NULL) {
        runtest.fail("the _videoSource reference wasn't created");
    } else {
        runtest.pass("the _videoSource reference was created");
    }
    if (webcam->_capsFilter == NULL && (devselect !=0)) {
        runtest.fail("the _capsFilter reference wasn't created");
    } else {
        runtest.pass("the _capsFilter reference was created");
    }
    if (webcam->_currentFormat == NULL && (devselect != 0)) {
        runtest.fail("no format was set (_currentFormat == NULL!)");
    } else {
        runtest.pass("format is set");
    }
    
    result = false;
    result = vig.webcamCreateDisplayBin(webcam);
    if (result != true) {
        runtest.fail("webcamCreateDisplayBin() returned an error");
    } else {
        runtest.pass("webcamCreateDisplayBin() isn't reporting errors");
    }
    if (webcam->_videoDisplayBin == NULL) {
        runtest.fail("the _webcamDisplayBin wasn't created");
    } else {
        runtest.pass("the _webcamDisplayBin was created");
        if ((gst_element_get_pad(webcam->_videoDisplayBin, "sink")) == NULL) {
            runtest.fail("the sink ghostpad in _videoDisplayBin wasn't created");
        } else {
            runtest.pass("the _videoDisplayBin sink ghostpad was created");
        }
    }

    result = false;
    result = vig.webcamMakeVideoDisplayLink(webcam);
    if (result != true) {
        runtest.fail("making videosrc -> display link failed");
    } else {
        runtest.pass("making videosrc -> display link succeeded");
    }
    
    result = false;
    result = vig.webcamCreateSaveBin(webcam);
    if (result != true) {
        runtest.fail("webcamCreateSaveBin() reported an error");
    } else {
        runtest.pass("webcamCreateSaveBin() didn't report any errors");
    }
    result = false;
    result = vig.webcamMakeVideoSaveLink(webcam);
    if (result != true) {
        runtest.fail("webcamMakeVideoSaveLink() reported an error");
    } else {
        runtest.pass("webcamMakeVideoSaveLink() didn't report errors");
    }
    if (webcam->_videoSaveBin == NULL) {
        runtest.fail("webcam->_videoSaveBin reference isn't set");
    } else {
        runtest.pass("webcam->_videoSaveBin reference is set");
        if ((gst_bin_get_by_name(GST_BIN(webcam->_pipeline),
            "video_save_bin")) == NULL) {
            runtest.fail("videoSaveBin has an unexpected address");
        } else {
            runtest.pass("videoSaveBin's address is set as expected");
        }
        if ((gst_element_get_pad(webcam->_videoSaveBin, "sink")) == NULL) {
            runtest.fail("videoSaveBin's sink ghostpad wasn't created");
        } else {
            runtest.pass("videoSaveBin properly has a sink ghostpad");
        }
    }
    if (webcam->_videoFileSink == NULL) {
        runtest.fail("webcam->_videoFileSink reference isn't set");
    } else {
        runtest.pass("webcam->_videoFileSink reference is set");
    }
    
    //end of setup tests, now startup the webcamPipeline, run for a few seconds
    //and then make sure there is a file present after running
    result = vig.webcamPlay(webcam);
    if (result != true) {
        runtest.fail("webcamPlay() function reported an error");
    } else {
        runtest.pass("webcamPlay() function reported no errors");
    }
    
    g_print("        NOTE: the output window will close automatically\n");
    
    if (webcam->_pipelineIsPlaying != true) {
        runtest.fail("the _pipelineIsPlaying variable isn't being set");
    } else {
        runtest.pass("the _pipelineIsPlaying variable is properly set");
    }
    sleep(5);
    result = vig.webcamStop(webcam);
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
    
    result = vig.webcamBreakVideoDisplayLink(webcam);
    if (result != true) {
        runtest.fail("the webcamBreakVideoDisplayLink() function reported an error");
    } else {
        runtest.pass("the webcamBreakVideoDisplayLink() function reported no errors");
    }
    
    result = vig.webcamPlay(webcam);
    if (result != true) {
        runtest.fail("webcamPlay() reported errors after breaking display link");
    } else {
        runtest.pass("webcamPlay() still works after breaking display link");
    }
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = vig.webcamStop(webcam);
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
    
    result = vig.webcamBreakVideoSaveLink(webcam);
    if (result != true) {
        runtest.fail("breaking the videoSaveLink failed");
    } else {
        runtest.pass("breaking the videoSaveLink was successful");
    }
    
    result = vig.webcamMakeVideoDisplayLink(webcam);
    if (result != true) {
        runtest.fail("making videosrc -> display link failed");
    } else {
        runtest.pass("making videosrc -> display link succeeded");
    }

    result = vig.webcamPlay(webcam);
    if (result != true) {
        runtest.fail("webcamPlay() reported errors after relinking display");
    } else {
        runtest.pass("webcamPlay() still works after relinking display");
    }
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = vig.webcamStop(webcam);
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
    
    result = vig.webcamMakeVideoSaveLink(webcam);
    if (result != true) {
        runtest.fail("webcamMakeVideoSaveLink() reported an error");
    } else {
        runtest.pass("webcamMakeVideoSaveLink() reported no errors");
    }
    
    result = vig.webcamPlay(webcam);
    if (result != true) {
        runtest.fail("webcamPlay() reported errors");
    } else {
        runtest.pass("webcamPlay() reported no errors");
    }
    
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = vig.webcamStop(webcam);
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
    gst::VideoInputGst *video = new VideoInputGst;
    if (video == NULL) {
        runtest.fail("new VideoInputGst didn't work");
    } else {
        runtest.pass("new VideoInputGst returned a value");
    }
    
    //get global webcam reference for use below
    GnashWebcamPrivate *global;
    global = video->getGlobalWebcam();
    if (global == NULL) {
        runtest.fail("couldn't get a globalwebcam video reference");
    } else {
        runtest.pass("got a globalWebcam reference");
    }
    
    if (global->_pipeline == NULL) {
        runtest.fail("video->_globalWebcam->_pipeline isn't there");
    } else {
        runtest.pass("video->_globalWebcam->_pipeline is initialized");
    }
    if (global->_webcamSourceBin == NULL) {
        runtest.fail("webcamSourceBin isn't there");
    } else {
        runtest.pass("webcamSourceBin was made by the initializer");
    }
    if (global->_webcamMainBin == NULL) {
        runtest.fail("webcamMainBin isn't there");
    } else {
        runtest.pass("webcamMainBin was made by the initializer");
    }
    if (global->_videoDisplayBin == NULL) {
        runtest.fail("videoDisplayBin isn't there");
    } else {
        runtest.pass("videoDisplayBin was made by the initializer");
    }
    if (global->_videoSaveBin == NULL) {
        runtest.fail("videoSaveBin isn't there");
    } else {
        runtest.pass("videoSaveBin was made by the initializer");
    }
    
    result = video->webcamMakeVideoDisplayLink(global);
    if (result != true) {
        runtest.fail("webcamMakeVideoDisplayLink reported errors");
    } else {
        runtest.pass("webcamMakeVideoDisplayLink reported no errors");
    }
    
    result = video->webcamPlay(global);
    if (result != true) {
        runtest.fail("webcamPlay reported errors");
    } else {
        runtest.pass("webcamPlay reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = video->webcamStop(global);
    if (result != true) {
        runtest.fail("webcamStop reported errors");
    } else {
        runtest.pass("webcamStop reported no errors");
    }
    
    g_print("        NOTE: changing values (display window should be bigger)....\n");
    video->set_fps(30);
    video->set_width(800);
    video->set_height(600);
    result = video->webcamChangeSourceBin(global);
    if (result != true) {
        runtest.fail("webcamChangeSourceBin reported an error");
    } else {
        runtest.pass("webcamChangeSourceBin reported no errors");
    }
    
    result = video->webcamPlay(global);
    if (result != true) {
        runtest.fail("webcamPlay reported errors");
    } else {
        runtest.pass("webcamPlay reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = video->webcamStop(global);
    if (result != true) {
        runtest.fail("webcamStop reported errors");
    } else {
        runtest.pass("webcamStop reported no errors");
    }
    
    result = video->webcamMakeVideoSaveLink(global);
    if (result != true) {
        runtest.fail("webcamMakeVideoSaveLink reported errors");
    } else {
        runtest.pass("webcamMakeVideoSaveLink reported no errors");
    }
    
    result = video->webcamPlay(global);
    if (result != true) {
        runtest.fail("webcamPlay reported errors");
    } else {
        runtest.pass("webcamPlay reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = video->webcamStop(global);
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
    
    result = video->webcamBreakVideoDisplayLink(global);
    if (result != true) {
        runtest.fail("webcamBreakVideoDisplayLink reported errors");
    } else {
        runtest.pass("webcamBreakVideoDisplayLink reported no errors");
    }
    
    result = video->webcamPlay(global);
    if (result != true) {
        runtest.fail("webcamPlay reported errors");
    } else {
        runtest.pass("webcamPlay reported no errors");
    }
    
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = video->webcamStop(global);
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
    
    result = video->webcamMakeVideoDisplayLink(global);
    if (result != true) {
        runtest.fail("webcamMakeVideoDisplayLink failed after breaking the link");
    } else {
        runtest.pass("webcamMakeVideoDisplayLink reported no errors");
    }
    
    result = video->webcamBreakVideoSaveLink(global);
    if (result != true) {
        runtest.fail("webcamBreakVideoSaveLink function reported errors");
    } else {
        runtest.pass("webcamBreakVideoSaveLink function reported no errors");
    }
    
    result = video->webcamPlay(global);
    if (result != true) {
        runtest.fail("webcamPlay reported errors");
    } else {
        runtest.pass("webcamPlay reported no errors");
    }
    
    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = video->webcamStop(global);
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
    
    //pass bad values
    g_print("        NOTE: changing values to bad vals....\n");
    video->set_fps(200);
    video->set_width(8000);
    video->set_height(6000);
    result = video->webcamChangeSourceBin(global);
    if (result != true) {
        runtest.fail("webcamChangeSourceBin reported an error");
    } else {
        runtest.pass("webcamChangeSourceBin reported no errors");
    }
    
    result = video->webcamPlay(global);
    if (result != true) {
        runtest.fail("webcamPlay reported errors");
    } else {
        runtest.pass("webcamPlay reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = video->webcamStop(global);
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
    
    //reset to good vals
    g_print("        NOTE: changing back to legit vals....\n");
    video->set_fps(30);
    video->set_width(320);
    video->set_height(240);
    
    result = video->webcamMakeVideoSaveLink(global);
    if (result != true) {
        runtest.fail("webcamMakeVideoSaveLink reported an error");
    } else {
        runtest.pass("webcamMakeVideoSaveLink reported no errors");
    }
    result = video->webcamChangeSourceBin(global);
    if (result != true) {
        runtest.fail("webcamChangeSourceBin reported an error");
    } else {
        runtest.pass("webcamChangeSourceBin reported no errors");
    }
    
    result = video->webcamPlay(global);
    if (result != true) {
        runtest.fail("webcamPlay reported errors");
    } else {
        runtest.pass("webcamPlay reported no errors");
    }

    g_print("        NOTE: sleeping for 5 seconds here....\n");
    sleep(5);
    
    result = video->webcamStop(global);
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
