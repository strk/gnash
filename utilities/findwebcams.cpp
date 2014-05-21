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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gst/GlibDeprecated.h"

#include <vector>
#include <string>
#include <iostream>

#include "rc.h"
#include "gst/gst.h"
#include <gst/interfaces/propertyprobe.h>

namespace {
    //get rc file for webcam selection
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

class data {
    public:
        data();
        gchar* deviceName;
        gchar* deviceType;
        gint deviceNumber;
        gboolean duplicate;
};

data::data() {
    deviceName = nullptr;
    deviceType = nullptr;
    deviceNumber = -1;
    duplicate = false;
};

gint numDuplicates = 0;

size_t findVidDevs(std::vector<data*>& vidVect) {
    gint numdevs = 0;
    
    //vid test source
    GstElement *element;
    element = gst_element_factory_make ("videotestsrc", "vidtestsrc");
    
    if (element == nullptr) {
        vidVect.push_back(nullptr);
        numdevs += 1;
    } else {
        vidVect.push_back(new data);
        vidVect[numdevs]->deviceName = g_strdup_printf("videotestsrc");
        vidVect[numdevs]->deviceType = g_strdup_printf("videotestsrc");
        vidVect[numdevs]->deviceNumber = 0;
        numdevs += 1;
    }
    
    //video4linux source
    GstPropertyProbe *probe;
    GValueArray *devarr;
    element = nullptr;
    
    element = gst_element_factory_make ("v4lsrc", "v4lvidsrc");
    probe = GST_PROPERTY_PROBE (element);
    devarr = gst_property_probe_probe_and_get_values_name (probe, "device");
    for (size_t i = 0; devarr != nullptr && i < devarr->n_values; ++i) {
        GValue *val;
        gchar *dev_name = nullptr;
        
        val = g_value_array_get_nth (devarr, i);
        g_object_set (element, "device", g_value_get_string (val), NULL);
        gst_element_set_state (element, GST_STATE_PLAYING);
        g_object_get (element, "device-name", &dev_name, NULL);
        gst_element_set_state (element, GST_STATE_NULL);
        if (strcmp(dev_name, "null") == 0) {
            std::cout << "no v4l video sources found" << std::endl;
        }
        else { 
            vidVect.push_back(new data);
            vidVect[numdevs]->deviceType = g_strdup_printf("v4lsrc");
            vidVect[numdevs]->deviceName = dev_name;
            vidVect[numdevs]->deviceNumber = numdevs;
            numdevs += 1;
        }
    }
    if (devarr) {
        g_value_array_free (devarr);
    }
    
    
    //video4linux2 source
    probe = nullptr;
    element = nullptr;
    devarr = nullptr;
    
    element = gst_element_factory_make ("v4l2src", "v4l2vidsrc");
    probe = GST_PROPERTY_PROBE (element);
    devarr = gst_property_probe_probe_and_get_values_name (probe, "device");
    for (size_t i = 0; devarr != nullptr && i < devarr->n_values; ++i) {
        GValue *val;
        gchar *dev_name = nullptr;
        
        val = g_value_array_get_nth (devarr, i);
        g_object_set (element, "device", g_value_get_string (val), NULL);
        gst_element_set_state (element, GST_STATE_PLAYING);
        g_object_get (element, "device-name", &dev_name, NULL);
        gst_element_set_state (element, GST_STATE_NULL);
        if (strcmp(dev_name, "null") == 0) {
            std::cout << "no v4l2 video sources found." << std::endl;
        }
        else {
            vidVect.push_back(new data);
            vidVect[numdevs]->deviceType = g_strdup_printf("v4l2src");
            vidVect[numdevs]->deviceName = dev_name;
            vidVect[numdevs]->deviceNumber = numdevs;
            //mark duplicates (we like v4l2 sources more than v4l, so if
            //they're both detected, mark the v4l source as a duplicate)
            for (size_t g=1; g < (vidVect.size()-1); g++) {
                if (strcmp(vidVect[numdevs]->deviceName,
                        vidVect[g]->deviceName) == 0) {
                    vidVect[g]->duplicate = true;
                    numDuplicates += 1;
                }
            }
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
    gst_init(nullptr, nullptr);
    size_t numdevs = 0;
    std::vector<data*> vidVector;
    
    int fromrc = rcfile.getWebcamDevice();
    
    if (fromrc == -1) {
        std::cout << std::endl
                  << "Use this utility to set your desired default webcam device." << std::endl;
        numdevs = findVidDevs(vidVector);
        std::cout << std::endl
             << "INFO: these devices were ignored because they are supported by both" << std::endl
             << "video4linux and video4linux2:" << std::endl << std::endl;
        for (size_t i = 0; i < numdevs; ++i) {
            if (vidVector[i]->duplicate == true) {
                std::cout << "    " << vidVector[i]->deviceName
                     << " (" << vidVector[i]->deviceType << ")" << std::endl;
            }
        }
        std::cout << std::endl
             << "Gnash interacts with v4l2 sources better than v4l sources, thus the" << std::endl
             << "v4l sources will not be printed in the list below." << std::endl
             << std::endl
             << "Found " << (numdevs - numDuplicates) 
             << " video devices: " << std::endl << std::endl;
        gint counter = 0;
        for (size_t i = 0; i < numdevs; ++i)
        {
            if (i == 0 && (vidVector[i] != nullptr)) {
                std::cout << "    " << i
                     << ". Video Test Source (videotestsrc)" << std::endl;
                counter++;
            } else if (i == 0 && (vidVector[i] == nullptr)) {
                std::cout << "no test video device available";
            } else {
                if (vidVector[i]->duplicate != true) {
                    std::cout << "    " << counter << ". "
                         << vidVector[i]->deviceName
                         << " (" << vidVector[i]->deviceType << ")" << std::endl;
                    counter++;
                }
            }
        }
        //prompt user for device selection
        int dev_select = -1;
        std::string fromCin;
        do {
            dev_select = -1;
            std::cout << std::endl
                 << "Choose the device you would like to use (0-"
                 << (numdevs - numDuplicates - 1) << "): ";
            std::cin >> fromCin;
            if (fromCin.size() != 1) {
                dev_select = -1;
            } else if (fromCin[0] == '0') {
                dev_select = 0;
            } else {
                dev_select = atoi(fromCin.c_str());
            }
            if ((dev_select < 0) || (dev_select > ((int)numdevs - numDuplicates - 1))) {
                std::cout << "You must make a valid device selection" << std::endl;
            }
        } while ((dev_select < 0) || (dev_select > ((int)numdevs - numDuplicates - 1)));
        std::cout << std::endl
             << "To select this camera, add this line to your gnashrc file:" << std::endl
             << "set webcamDevice "
             << (vidVector[dev_select + numDuplicates]->deviceNumber) << std::endl;
    } else {
        numdevs = findVidDevs(vidVector);
        if ((size_t)fromrc <= (vidVector.size() - 1)) {
            std::cout << std::endl
                 << "The gnashrc file reports default webcam is set to:" << std::endl
                 << vidVector[fromrc]->deviceName
                 << " (" << vidVector[fromrc]->deviceType << ")" << std::endl
                 << "To change this setting, delete the 'set webcamDevice' line" << std::endl
                 << "from your gnashrc file and re-run this program." << std::endl << std::endl;
        } else {
          std::cout << std::endl
               << "You have an invalid webcam chosen in your gnashrc file." << std::endl
               << "Try reattaching the device or deleting the value from gnashrc" << std::endl
               << "and running this program again" << std::endl;
        }
    }
    return 0;
}
