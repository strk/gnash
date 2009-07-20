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

#include <boost/shared_ptr.hpp>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
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

static void usage (void);

static TestState runtest;

static string infile;

static void test_client();

LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
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
	
	//vig.make_webcamDevice_selection();
	
	//cerr << "placeholder" << endl;
    
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
