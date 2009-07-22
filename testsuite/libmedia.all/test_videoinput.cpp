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
    if (webcam->_capsFilter == NULL) {
        runtest.fail("the _capsFilter reference wasn't created");
    } else {
        runtest.pass("the _capsFilter reference was created");
    }
    if (webcam->_currentFormat == NULL) {
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
        if ((gst_bin_get_by_name(GST_BIN(webcam->_pipeline),
            "video_display_bin")) == NULL) {
            runtest.fail("videoDisplayBin has an unexpected address");
        } else {
            runtest.pass("videoDisplayBin's address is set as expected");
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
    
    //end of setup tests
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
