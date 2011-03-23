// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
#ifdef HAVE_GST_GST_H
#include "gst/gst.h"
#endif

#ifdef HAVE_GST_INTERFACES_PROBEPROBE_H
#include <gst/interfaces/propertyprobe.h>
#endif

#include <vector>
#include <string>
#include <iostream>

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

gint findAudioDevs(std::vector<data*>& audioVect) {
    gint numdevs = 0;
    
    //vid test source
    GstElement *element;
    element = gst_element_factory_make ("audiotestsrc", "audiotestsrc");
    
    if (element == NULL) {
        audioVect.push_back(NULL);
        numdevs += 1;
    } else {
        audioVect.push_back(new data);
        audioVect[numdevs]->deviceName = g_strdup_printf("audiotestsrc");
        audioVect[numdevs]->deviceType = g_strdup_printf("audiotestsrc");
        numdevs += 1;
    }
    
    //pulseaudio src
    GstPropertyProbe *probe;
    GValueArray *devarr;
    element = NULL;
    
    element = gst_element_factory_make ("pulsesrc", "pulsesrc");
    probe = GST_PROPERTY_PROBE (element);
    devarr = gst_property_probe_probe_and_get_values_name (probe, "device");
    for (size_t i = 0; devarr != NULL && i < devarr->n_values; ++i) {
        GValue *val;
        gchar *dev_name = NULL;
        
        val = g_value_array_get_nth (devarr, i);
        g_object_set (element, "device", g_value_get_string (val), NULL);
        gst_element_set_state (element, GST_STATE_PLAYING);
        g_object_get (element, "device-name", &dev_name, NULL);
        gst_element_set_state (element, GST_STATE_NULL);
        if (strcmp(dev_name, "null") == 0) {
            g_print("no pulse audio sources found\n");
        }
        else if ((strstr(dev_name, "Monitor") != NULL)) {
            g_print("ignoring monitor (audio output)");
        }
        else { 
            audioVect.push_back(new data);
            audioVect[numdevs]->deviceType = g_strdup_printf("pulsesrc");
            audioVect[numdevs]->deviceName = dev_name;
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
    std::vector<data*> audioVector;
    gint i;
    
    int fromrc = rcfile.getAudioInputDevice();
    
    if (fromrc == -1) {
        g_print("Use this utility to set your desired default microphone device.\n");
        numdevs = findAudioDevs(audioVector);
        g_print("\nFound %d audio input devices: \n\n", numdevs);
        for (i = 0; i < numdevs; ++i)
        {
            if (i == 0 && (audioVector[i] != 0)) {
                g_print("%d. device[%d] = Audio Test Source (audiotestsrc)\n", i, i);
            } else if (i == 0 && (audioVector[i] == 0)) {
                g_print("no test audio device available\n");
            } else {
                g_print("%d. device[%d] = %s (%s)\n", i, i, audioVector[i]->deviceName,
                        audioVector[i]->deviceType);
            }
        }
        //prompt user for device selection
        int dev_select = -1;
        std::string fromCin;
        do {
            dev_select = -1;
            g_print("\nChoose the device you would like to use (0-%d): ",
                (numdevs - 1));
            std::cin >> fromCin;
            if (fromCin.size() != 1) {
                dev_select = -1;
            } else if (fromCin[0] == '0') {
                dev_select = 0;
            } else {
                dev_select = atoi(fromCin.c_str());
            }
            if ((dev_select < 0) || (dev_select > (numdevs - 1))) {
                g_print("You must make a valid device selection\n");
            }
        } while ((dev_select < 0) || (dev_select > (numdevs - 1)));
        g_print("\nTo select this microphone, add this line to your gnashrc file:\n");
        g_print("set microphoneDevice %d\n", dev_select);
    } else {
        numdevs = findAudioDevs(audioVector);
        if ((size_t)fromrc < audioVector.size()) {
            g_print("\nThe gnashrc file reports default microphone is set to:\n");
            g_print("%s (%s)\n", audioVector[fromrc]->deviceName,
                audioVector[fromrc]->deviceType);
            g_print("To change this setting, delete the 'set microphoneDevice' line\n");
            g_print("from your gnashrc file and re-run this program.\n\n");
        } else {
            g_print("\nYou have an invalid microphone chosen in your gnashrc file.\n");
            g_print("Try reattaching the device or deleting the value from gnashrc\n");
            g_print("and running this program again\n");
        }
    }
    return 1;
}
