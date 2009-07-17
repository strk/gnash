// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "rc.h"
#include "gst/gst.h"
#include <gst/interfaces/propertyprobe.h>

#include <vector>

namespace {
    //get rc file for webcam selection
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

class data {
    public:
        data();
        gchar* deviceName;
        gchar* deviceType;
};

data::data() {
    deviceName = NULL;
    deviceType = NULL;
};

gint findVidDevs(std::vector<data*>& vidVect) {
    gint numdevs = 0;
    
    //vid test source
    GstElement *element;
    element = gst_element_factory_make ("videotestsrc", "vidtestsrc");
    
    if (element == NULL) {
        vidVect.push_back(NULL);
        numdevs += 1;
    } else {
        vidVect.push_back(new data);
        vidVect[numdevs]->deviceName = g_strdup_printf("videotestsrc");
        vidVect[numdevs]->deviceType = g_strdup_printf("videotestsrc");
        numdevs += 1;
    }
    
    //video4linux source
    GstPropertyProbe *probe;
    GValueArray *devarr;
    element = NULL;
    gint i;
    
    element = gst_element_factory_make ("v4lsrc", "v4lvidsrc");
    probe = GST_PROPERTY_PROBE (element);
    devarr = gst_property_probe_probe_and_get_values_name (probe, "device");
    for (i = 0; devarr != NULL && i < devarr->n_values; ++i) {
        GValue *val;
        gchar *dev_name = NULL;
        
        val = g_value_array_get_nth (devarr, i);
        g_object_set (element, "device", g_value_get_string (val), NULL);
        gst_element_set_state (element, GST_STATE_PLAYING);
        g_object_get (element, "device-name", &dev_name, NULL);
        gst_element_set_state (element, GST_STATE_NULL);
        if (dev_name == "null") {
            g_print("no v4l video sources found\n");
        }
        else { 
            vidVect.push_back(new data);
            vidVect[numdevs]->deviceType = g_strdup_printf("v4lsrc");
            vidVect[numdevs]->deviceName = dev_name;
            numdevs += 1;
        }
    }
    if (devarr) {
        g_value_array_free (devarr);
    }
    
    
    //video4linux2 source
    probe = NULL;
    element = NULL;
    devarr = NULL;
    
    element = gst_element_factory_make ("v4l2src", "v4l2vidsrc");
    probe = GST_PROPERTY_PROBE (element);
    devarr = gst_property_probe_probe_and_get_values_name (probe, "device");
    for (i = 0; devarr != NULL && i < devarr->n_values; ++i) {
        GValue *val;
        gchar *dev_name = NULL;
        
        val = g_value_array_get_nth (devarr, i);
        g_object_set (element, "device", g_value_get_string (val), NULL);
        gst_element_set_state (element, GST_STATE_PLAYING);
        g_object_get (element, "device-name", &dev_name, NULL);
        gst_element_set_state (element, GST_STATE_NULL);
        if (dev_name == "null") {
            g_print("no v4l2 video sources found.\n");
        }
        else { 
            vidVect.push_back(new data);
            vidVect[numdevs]->deviceType = g_strdup_printf("v4l2src");
            vidVect[numdevs]->deviceName = dev_name;
            
            numdevs += 1;
        }
    }
    if (devarr) {
        g_value_array_free (devarr);
    }
    
    return numdevs;
}

int main () {
    //initialize gstreamer to probe for devs
    gst_init(NULL, NULL);
    gint numdevs = 0;
    std::vector<data*> vidVector;
    gint i;
    
    int fromrc = rcfile.getWebcamDevice();
    
    if (fromrc == -1) {
        g_print("Use this utility to set your desired default webcam device.\n");
        numdevs = findVidDevs(vidVector);
        g_print("\nFound %d video devices: \n\n", numdevs);
        for (i = 0; i < numdevs; ++i)
        {
            if (i == 0 && (vidVector[i] != 0)) {
                g_print("%d. device[%d] = Video Test Source (videotestsrc)\n", i, i);
            } else if (i == 0 && (vidVector[i] == 0)) {
                g_print("no test video device available");
            } else {
                g_print("%d. device[%d] = %s (%s)\n", i, i, vidVector[i]->deviceName,
                        vidVector[i]->deviceType);
            }
        }
        //prompt user for device selection
        int dev_select = -1;
        do {
            dev_select = -1;
            g_print("\nChoose the device you would like to use (0-%d): ",
                (numdevs - 1));
            std::cin >> dev_select;
            if ((dev_select < 0) || (dev_select > (numdevs - 1))) {
                g_print("You must make a valid device selection\n");
            }
        } while ((dev_select < 0) || (dev_select > (numdevs - 1)));
        g_print("\nTo select this camera, add this line to your gnashrc file:\n");
        g_print("set webcamDevice %d\n", dev_select);
    } else {
        numdevs = findVidDevs(vidVector);
        g_print("\nThe gnashrc file reports default webcam is set to:\n");
        g_print("%s (%s)\n", vidVector[fromrc]->deviceName,
            vidVector[fromrc]->deviceType);
        g_print("To change this setting, delete the 'set webcamDevice' line\n");
        g_print("from your gnashrc file and re-run this program.\n\n");
    }
    return 1;
}
