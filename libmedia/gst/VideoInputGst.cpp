// VideoInputGst.cpp: Video input processing using Gstreamer.
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

#include "VideoInputGst.h"
#include "log.h"
#include "GstUtil.h"
#include "gst/gst.h"
#include <gst/interfaces/propertyprobe.h>
#include <vector>
#include "rc.h"
#include <cmath>


namespace {
    //get rc file for webcam selection
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

namespace gnash {
namespace media {
namespace gst {
    
    //initializes the Gstreamer interface
    VideoInputGst::VideoInputGst() {
        gst_init(NULL,NULL);
        
        int devSelection;
        findVidDevs();
        
        //enumerate names array for actionscript accessibility
        int i;
        for (i=0; i < _vidVect.size(); ++i) {
            _names.push_back(_vidVect[i]->getProductName());
        }
        
        devSelection = makeWebcamDeviceSelection();
        _devSelection = devSelection;
        //also set _index for actionscript accessibility
        _index = devSelection;
        
        transferToPrivate(devSelection);
        webcamCreateMainBin(_globalWebcam);
        webcamCreateDisplayBin(_globalWebcam);
        webcamCreateSaveBin(_globalWebcam);
    }
    
    VideoInputGst::~VideoInputGst() {
        log_unimpl("Video Input destructor");
    }
    
    //populates video devices to a vector of GnashWebcam pointers
    //which contain important information about the hardware camera
    //inputs available on the machine
    void
    VideoInputGst::findVidDevs() {
        _numdevs = 0;
        
        //find video test sources
        GstElement *element;
        element = gst_element_factory_make ("videotestsrc", "vidtestsrc");
        
        if (element == NULL) {
            log_error("%s: Could not create video test source.", __FUNCTION__);
            _vidVect.push_back(NULL);
            _numdevs += 1;
        } else {
            _vidVect.push_back(new GnashWebcam);
            _vidVect[_numdevs]->setElementPtr(element);
            _vidVect[_numdevs]->setGstreamerSrc(g_strdup_printf("videotestsrc"));
            _vidVect[_numdevs]->setProductName(g_strdup_printf("videotest"));
            _numdevs += 1;
        }
        
        //find v4l devices
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
                log_trace("No v4l video sources. Checking for other vid inputs");
            }
            else { 
                _vidVect.push_back(new GnashWebcam);
                _vidVect[_numdevs]->setElementPtr(element);
                _vidVect[_numdevs]->setGstreamerSrc(g_strdup_printf("v4lsrc"));
                _vidVect[_numdevs]->setProductName(dev_name);
                
                //set device location information (e.g. /dev/video0)
                gchar *location;
                g_object_get (element, "device", &location , NULL);
                _vidVect[_numdevs]->setDevLocation(location);
                _numdevs += 1;
            }
        }
        if (devarr) {
            g_value_array_free (devarr);
        }
        
        //find v4l2 devices
        probe = NULL;
        devarr = NULL;
        element = NULL;
        
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
                log_trace("no v4l2 video sources found.");
            }
            else { 
                _vidVect.push_back(new GnashWebcam);
                _vidVect[_numdevs]->setElementPtr(element);
                _vidVect[_numdevs]->setGstreamerSrc(g_strdup_printf("v4l2src"));
                _vidVect[_numdevs]->setProductName(dev_name);
                
                //set device location information (e.g. /dev/video0)
                gchar *location;
                g_object_get (element, "device", &location , NULL);
                _vidVect[_numdevs]->setDevLocation(location);
                _numdevs += 1;
            }
        }
        if (devarr) {
            g_value_array_free (devarr);
        }
    }
    
    
    //called by addSupportedFormat. finds the highest possible framerate
    //to record at (can be shaped down by a filter for performance)
    void
    VideoInputGst::findHighestFramerate(WebcamVidFormat *format)
    {
        gint framerate_numerator;
        gint framerate_denominator;
        gint i;

        //Select the highest framerate up to less than or equal to 30 Hz
        framerate_numerator   = 1;
        framerate_denominator = 1;
        for (i = 0; i < format->numFramerates; i++) {
            gfloat framerate = format->framerates[i].numerator / 
                format->framerates[i].denominator;
            if (framerate > ((float) framerate_numerator / framerate_denominator)
               && framerate <= 30) {
                framerate_numerator   = format->framerates[i].numerator;
                framerate_denominator = format->framerates[i].denominator;
            }
        }
        //set highest found above
        format->highestFramerate.numerator = framerate_numerator;
        format->highestFramerate.denominator = framerate_denominator;
    } 
    
    //find the framerates at which the selected format can handle input
    void
    VideoInputGst::getSupportedFramerates 
      (WebcamVidFormat *video_format, GstStructure *structure)
    {
        const GValue *framerates;
        gint i, j;
        
        //note that framerates may contain one value, a list, or a range
        framerates = gst_structure_get_value (structure, "framerate");
        if (GST_VALUE_HOLDS_FRACTION (framerates)) {
            video_format->numFramerates = 1;
            video_format->framerates =
                g_new0 (FramerateFraction, video_format->numFramerates);
            video_format->framerates[0].numerator =
                gst_value_get_fraction_numerator (framerates);
            video_format->framerates[0].denominator =
                gst_value_get_fraction_denominator (framerates);
        }
        else if (GST_VALUE_HOLDS_LIST (framerates)) {
            video_format->numFramerates = gst_value_list_get_size (framerates);
            video_format->framerates =
                g_new0 (FramerateFraction, video_format->numFramerates);
            for (i = 0; i < video_format->numFramerates; i++) {
                const GValue *value;
                value = gst_value_list_get_value (framerates, i);
                video_format->framerates[i].numerator =
                    gst_value_get_fraction_numerator (value);
                video_format->framerates[i].denominator =
                    gst_value_get_fraction_denominator (value);
            }
        }
        else if (GST_VALUE_HOLDS_FRACTION_RANGE (framerates)) {
            gint numerator_min, denominator_min, numerator_max, denominator_max;
            const GValue *fraction_range_min;
            const GValue *fraction_range_max;

            fraction_range_min =
                gst_value_get_fraction_range_min (framerates);
            numerator_min =
                gst_value_get_fraction_numerator (fraction_range_min);
            denominator_min =
                gst_value_get_fraction_denominator (fraction_range_min);

            fraction_range_max = gst_value_get_fraction_range_max (framerates);
            numerator_max =
                gst_value_get_fraction_numerator (fraction_range_max);
            denominator_max = 
                gst_value_get_fraction_denominator (fraction_range_max);
            log_trace ("FractionRange: %d/%d - %d/%d",
                numerator_min, denominator_min, numerator_max, denominator_max);

            video_format->numFramerates =
                (numerator_max - numerator_min + 1) * 
                (denominator_max - denominator_min + 1);
            video_format->framerates =
                g_new0 (FramerateFraction, video_format->numFramerates);
            int k = 0;
            for (i = numerator_min; i <= numerator_max; i++) {
                for (j = denominator_min; j <= denominator_max; j++) {
                    video_format->framerates[k].numerator   = i;
                    video_format->framerates[k].denominator = j;
                    k++;
                }
            }
        }
        else {
            g_critical ("GValue type %s, cannot be handled for framerates",
                G_VALUE_TYPE_NAME (framerates));
        }
    }
    
    //we found a supported framerate and want to add the information to 
    //the GnashWebcam structure
    void
    VideoInputGst::addSupportedFormat(GnashWebcam *cam, WebcamVidFormat *video_format,
        GstStructure *format_structure)
    {
        gint i;
        gchar *resolution;
        
        getSupportedFramerates(video_format, format_structure);
        findHighestFramerate(video_format);
        
        resolution = g_strdup_printf ("%ix%i", video_format->width,
              video_format->height);
        i = GPOINTER_TO_INT(g_hash_table_lookup (cam->supportedResolutions, resolution));
        
        //if i returns a value, maybe this resolution has been added previously?
        if(i) {
            WebcamVidFormat *curr_format =
                &g_array_index(cam->videoFormats, WebcamVidFormat, i - 1);
            gfloat new_framerate = (float)(video_format->highestFramerate.numerator / 
                  video_format->highestFramerate.denominator);
            gfloat curr_framerate = (float)(curr_format->highestFramerate.numerator /
                                      curr_format->highestFramerate.denominator);
            if (new_framerate > curr_framerate) {
                log_debug("higher framerate replacing existing format");
                *curr_format = *video_format;
            }
            
            g_free (resolution);
            
            return;
        }
        
        g_array_append_val (cam->videoFormats, *video_format);
        g_hash_table_insert (cam->supportedResolutions, resolution,
              GINT_TO_POINTER(cam->numVideoFormats + 1));

        cam->numVideoFormats++;
    }
    
    //pulls webcam device selection from gnashrc (will eventually tie into
    //gui)
    int
    VideoInputGst::makeWebcamDeviceSelection() {
        int dev_select;
        dev_select = rcfile.getWebcamDevice();
        if (dev_select == -1) {
            log_trace("%s: No webcam selected in rc file, setting to videotestsource",
                __FUNCTION__);
            rcfile.setWebcamDevice(0);
            dev_select = rcfile.getWebcamDevice();
        } else {
            log_trace("Camera %d specified in gnashrc file, using that one.",
                dev_select);
        }
        //make sure that the device selected is actually valid
        if ((rcfile.getWebcamDevice() > (_vidVect.size() - 1)) || rcfile.getWebcamDevice() < 0) {
            log_error("You have an invalid camera selected. Please check your gnashrc file");
            exit(EXIT_FAILURE);
        } else {
            
            //set _name value for actionscript
            _name = _vidVect[dev_select]->getProductName();
            
            //now that a selection has been made, get capabilities of that device
            getSelectedCaps(rcfile.getWebcamDevice());
            return rcfile.getWebcamDevice();
        }
    }

    //called after a device selection, this starts enumerating the device's
    //capabilities
    void
    VideoInputGst::getSelectedCaps(gint dev_select)
    {
        GstElement *pipeline;
        gchar *command;
        GError *error = NULL;
        GstStateChangeReturn return_val;
        GstBus *bus;
        GstMessage *message;
        
        GnashWebcam *data_struct = _vidVect[dev_select];
        GstElement *element;
        element = data_struct->getElementPtr();
        
        //create tester pipeline to enumerate properties
        if (dev_select == 0) {
            command = g_strdup_printf ("%s name=src ! fakesink",
                data_struct->getGstreamerSrc());
        } else if ((dev_select > (_vidVect.size() - 1)) || dev_select < 0) {
            log_error("%s: Passed an invalid argument (not a valid dev_select value)",
                __FUNCTION__);
            exit(EXIT_FAILURE);
        } else {
            command = g_strdup_printf ("%s name=src device=%s ! fakesink",
                data_struct->getGstreamerSrc(), data_struct->getDevLocation());
        }
        pipeline = gst_parse_launch(command, &error);
        if ((pipeline != NULL) && (error == NULL)) {
            //Wait at most 5 seconds for the pipeline to start playing
            gst_element_set_state (pipeline, GST_STATE_PLAYING);
            return_val = 
                gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
            
            //errors on bus?
            bus = gst_element_get_bus (pipeline);
            message = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
            
            if (GST_IS_OBJECT(bus)){
                gst_object_unref (bus);
            } else {
                log_error("%s: Pipeline bus isn't an object for some reason",
                    __FUNCTION__);
            }
            
            //if everything above worked properly, begin probing for values
            if ((return_val == GST_STATE_CHANGE_SUCCESS) && (message == NULL)) {
                GstElement *src;
                GstPad *pad;
                gchar *name;
                GstCaps *caps;
                
                gst_element_set_state(pipeline, GST_STATE_PAUSED);
                
                src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
                
                //get the pad, find the capabilities for probing in supported formats
                pad  = gst_element_get_pad (src, "src");
                caps = gst_pad_get_caps (pad);
                if (GST_IS_OBJECT(pad)) {
                    gst_object_unref (pad);
                } else {
                    log_error("%s: Template pad isn't an object for some reason",
                        __FUNCTION__);
                }
                if (dev_select != 0) {
                    getSupportedFormats(data_struct, caps);
                }
                
                gst_caps_unref (caps);
            }
            gst_element_set_state (pipeline, GST_STATE_NULL);
            if (GST_IS_OBJECT(pipeline)){
                gst_object_unref (pipeline);
            } else {
                log_error("%s: pipeline isn't an object for some reason",
                    __FUNCTION__);
            }
        }
       
        if (error) {
          g_error_free (error);
        }
        g_free (command);
    }

    //probe the selected camera for the formats it supports
    void
    VideoInputGst::getSupportedFormats(GnashWebcam *cam, GstCaps *caps) {
        gint i;
        gint num_structs;
        
        num_structs = gst_caps_get_size (caps);
        
        for (i=0; i < num_structs; i++) {
            GstStructure *structure;
            const GValue *width, *height;
            
            //this structure is used to probe the source for information
            structure = gst_caps_get_structure (caps, i);
            
            //we just want to enumerate raw formats to keep things consistent
            //so if the strcuture we're currently looking at isn't either of
            //the standard raw formats, keep iterating through the loop
            if (!gst_structure_has_name (structure, "video/x-raw-yuv") &&
                !gst_structure_has_name (structure, "video/x-raw-rgb")) 
            {
              continue;
            }
            
            width  = gst_structure_get_value (structure, "width");
            height = gst_structure_get_value (structure, "height");
            
            if (G_VALUE_HOLDS_INT (width)) {
                  WebcamVidFormat video_format;

                  video_format.mimetype = 
                    g_strdup (gst_structure_get_name (structure));
                  gst_structure_get_int (structure, "width", &(video_format.width));
                  gst_structure_get_int (structure, "height", &(video_format.height));
                  addSupportedFormat(cam, &video_format, structure);
            }
            else if (GST_VALUE_HOLDS_INT_RANGE (width)) {
                int min_width, max_width, min_height, max_height;
                int cur_width, cur_height;

                min_width  = gst_value_get_int_range_min (width);
                max_width  = gst_value_get_int_range_max (width);
                min_height = gst_value_get_int_range_min (height);
                max_height = gst_value_get_int_range_max (height);

                cur_width  = min_width;
                cur_height = min_height;
                while (cur_width <= max_width && cur_height <= max_height) {
                    WebcamVidFormat video_format;

                    video_format.mimetype =
                        g_strdup (gst_structure_get_name (structure));
                    video_format.width    = cur_width;
                    video_format.height   = cur_height;
                    addSupportedFormat(cam, &video_format, structure);
                    cur_width  *= 2;
                    cur_height *= 2;
                }

                cur_width  = max_width;
                cur_height = max_height;
                while (cur_width > min_width && cur_height > min_height) {
                    WebcamVidFormat video_format;

                    video_format.mimetype = 
                        g_strdup (gst_structure_get_name (structure));
                    video_format.width    = cur_width;
                    video_format.height   = cur_height;
                    addSupportedFormat(cam, &video_format, structure);
                    cur_width  /= 2;
                    cur_height /= 2;
                }
            }
            else {
                log_error("%s: type %s, cannot be handled for resolution width",
                    __FUNCTION__, G_VALUE_TYPE_NAME (width));
            }
        }
    }

    //move the selected camera information to a more robust data structure
    //to store pipeline-ing information
    GnashWebcamPrivate*
    VideoInputGst::transferToPrivate(gint dev_select)
    {
        if ((dev_select > (_vidVect.size() - 1)) || dev_select < 0) {
            log_error("%s: Passed an invalid argument (bad dev_select value)",
                __FUNCTION__);
                exit(EXIT_FAILURE);
        }
        GnashWebcamPrivate *webcam = new GnashWebcamPrivate;
        if (webcam != NULL) {
            webcam->setWebcamDevice(_vidVect[dev_select]);
            webcam->setDeviceName(_vidVect[dev_select]->getProductName());
            _globalWebcam = webcam;
        } else {
            log_error("%s: was passed a NULL pointer", __FUNCTION__);
        }
        return webcam;
    }

    //create a bin containing the source and a connector ghostpad
    gboolean
    VideoInputGst::webcamCreateSourceBin(GnashWebcamPrivate *webcam) {
        GError *error = NULL;
        gchar *command = NULL;
        
        if(webcam->_webcamDevice == NULL) {
            log_trace("%s: You don't have any webcams chosen, using videotestsrc",
                __FUNCTION__);
            webcam->_webcamSourceBin = gst_parse_bin_from_description (
                "videotestsrc name=video_source ! capsfilter name=capsfilter",
                TRUE, &error);
            log_debug("Command: videotestsrc name=video_source ! \
                capsfilter name=capsfilter");
        }
        else {
            WebcamVidFormat *format = NULL;
            gint i;
            gchar *resolution;
            
            resolution = g_strdup_printf("%ix%i", _width, _height);
                                      
            //use these resolutions determined above if the camera supports it
            if (_width != 0 && _height != 0) {
                
                i = GPOINTER_TO_INT(g_hash_table_lookup
                    (webcam->_webcamDevice->supportedResolutions, resolution));
                //the selected res is supported if i
                if (i) {
                    format = &g_array_index (webcam->_webcamDevice->videoFormats,
                             WebcamVidFormat, i - 1);
                }
            }
            
            //if format didn't get set, something went wrong. try picking
            //the first supported format and a different supported resolution
            if (!format) {
                format = &g_array_index (webcam->_webcamDevice->videoFormats,
                     WebcamVidFormat, 0);
                for (i = 1; i < webcam->_webcamDevice->numVideoFormats; i++) {
                    if (g_array_index (webcam->_webcamDevice->videoFormats,
                               WebcamVidFormat, i).width <= format->width){
                        format = &g_array_index (webcam->_webcamDevice->videoFormats,
                             WebcamVidFormat, i);
                    }
                }
            }
            
            webcam->_currentFormat = format;
            g_free(resolution);
            
            //if format isn't set, something is still going wrong, make generic
            //components and see if they work!
            if (format == NULL) {
                if (error != NULL) {
                    g_error_free (error);
                    error = NULL;
                }
                webcam->_webcamSourceBin = 
                    gst_parse_bin_from_description ("videotestsrc name=video_source",
                    TRUE, &error);
                webcam->_videoSource = 
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "video_source");
                
                //if there are still errors, something's up, return out of function
                if (error != NULL) {
                    g_error_free (error);
                    return false;
                }
                webcam->_capsFilter = 
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "capsfilter");
                return true;
            }
            
            //execution here means we're good to make the pipeline
            else {
                //can't reduce this to 80 line limit without causing problems
                command = g_strdup_printf (
                  "%s name=video_source device=%s ! capsfilter name=capsfilter caps=video/x-raw-rgb,width=%d,height=%d,framerate=%d/%d;video/x-raw-yuv,width=%d,height=%d,framerate=%d/%d",
                  webcam->_webcamDevice->getGstreamerSrc(),
                  webcam->_webcamDevice->getDevLocation(),
                  format->width,
                  format->height,
                  format->highestFramerate.numerator,
                  format->highestFramerate.denominator,
                  format->width,
                  format->height,
                  format->highestFramerate.numerator,
                  format->highestFramerate.denominator);
                
                //debug
                log_debug("GstPipeline command is: %s", command);
                
                webcam->_webcamSourceBin =
                    gst_parse_bin_from_description (command, TRUE, &error);
                if (webcam->_webcamSourceBin == NULL) {
                    log_error ("%s: Creation of the webcam_source_bin failed",
                        __FUNCTION__);
                    log_error ("the error was %s", error->message);
                    return false;
                }
                
                //set _currentFps value for actionscript
                _currentFPS = (format->highestFramerate.numerator / 
                            format->highestFramerate.denominator);
                
                g_free(command);
                
                webcam->_videoSource = 
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "video_source");
                webcam->_capsFilter =
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "capsfilter");
                return true;
            }
        }
        return true;
    }
    
    gboolean
    VideoInputGst::checkForSupportedFramerate (GnashWebcamPrivate *webcam, int fps) {
        gint fNum, fDenom, i, val;
        
        for (i = 0; i < webcam->_currentFormat->numFramerates; ++i) {
            val = std::ceil(webcam->_currentFormat->framerates[i].numerator /
                       webcam->_currentFormat->framerates[i].denominator);
            if (val == fps) {
                return true;
            } else {
                continue;
            }
        }
        return false;
    }
    
    gboolean
    VideoInputGst::webcamChangeSourceBin(GnashWebcamPrivate *webcam) {
        GError *error = NULL;
        gchar *command = NULL;
        
        if(webcam->_pipelineIsPlaying == true) {
            webcamStop(webcam);
        }

        //delete the old source bin
        gst_bin_remove(GST_BIN(webcam->_webcamMainBin), webcam->_webcamSourceBin);
        webcam->_webcamSourceBin = NULL;
        
        if(webcam->_webcamDevice == NULL) {
            log_trace("%s: You don't have any webcams chosen, using videotestsrc",
                __FUNCTION__);
            webcam->_webcamSourceBin = gst_parse_bin_from_description (
                "videotestsrc name=video_source ! capsfilter name=capsfilter",
                TRUE, &error);
            log_debug("Command: videotestsrc name=video_source ! \
                capsfilter name=capsfilter");
        }
        else {
            WebcamVidFormat *format = NULL;
            gint i;
            gchar *resolution;
            
            resolution = g_strdup_printf("%ix%i", _width, _height);
                                      
            //use these resolutions determined above if the camera supports it
            if (_width != 0 && _height != 0) {
                
                i = GPOINTER_TO_INT(g_hash_table_lookup
                    (webcam->_webcamDevice->supportedResolutions, resolution));
                //the selected res is supported if i
                if (i) {
                    format = &g_array_index (webcam->_webcamDevice->videoFormats,
                             WebcamVidFormat, i - 1);
                }
            }
            
            //if format didn't get set, something went wrong. try picking
            //the first supported format and a different supported resolution
            if (!format) {
                log_error("%s: the resolution you chose isn't supported, picking \
                    a supported value", __FUNCTION__);
                format = &g_array_index (webcam->_webcamDevice->videoFormats,
                     WebcamVidFormat, 0);
                for (i = 1; i < webcam->_webcamDevice->numVideoFormats; i++) {
                    if (g_array_index (webcam->_webcamDevice->videoFormats,
                               WebcamVidFormat, i).width <= format->width){
                        format = &g_array_index (webcam->_webcamDevice->videoFormats,
                             WebcamVidFormat, i);
                    }
                }
            }
            
            //check here to make sure the fps value is supported (only valid for
            //non test sources)
            if (! g_strcmp0(webcam->_webcamDevice->getGstreamerSrc(), "videotestsrc") == 0) {
                int newFps = _fps;
                if (checkForSupportedFramerate(webcam, newFps)) {
                    log_debug("checkforsupportedfr returned true");
                    format->highestFramerate.numerator = newFps;
                    format->highestFramerate.denominator = 1;
                } else {
                    log_debug("checkforsupportedfr returned false");
                    
                    //currently chooses the ActionScript default of 15 fps in case
                    //you pass in an unsupported framerate value
                    format->highestFramerate.numerator = 15;
                    format->highestFramerate.denominator = 1;
                }
            }
            webcam->_currentFormat = format;
            g_free(resolution);
            
            //if format isn't set, something is still going wrong, make generic
            //components and see if they work!
            if (format == NULL) {
                if (error != NULL) {
                    g_error_free (error);
                    error = NULL;
                }
                webcam->_webcamSourceBin = 
                    gst_parse_bin_from_description ("videotestsrc name=video_source",
                    TRUE, &error);
                webcam->_videoSource = 
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "video_source");
                
                //if there are still errors, something's up, return out of function
                if (error != NULL) {
                    g_error_free (error);
                    return false;
                }
                webcam->_capsFilter = 
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "capsfilter");
                return true;
            }
            
            //execution here means we're good to make the pipeline
            else {
                //can't reduce this to 80 line limit without causing problems
                command = g_strdup_printf (
                  "%s name=video_source device=%s ! capsfilter name=capsfilter caps=video/x-raw-rgb,width=%d,height=%d,framerate=%d/%d;video/x-raw-yuv,width=%d,height=%d,framerate=%d/%d",
                  webcam->_webcamDevice->getGstreamerSrc(),
                  webcam->_webcamDevice->getDevLocation(),
                  format->width,
                  format->height,
                  format->highestFramerate.numerator,
                  format->highestFramerate.denominator,
                  format->width,
                  format->height,
                  format->highestFramerate.numerator,
                  format->highestFramerate.denominator);
                
                //debug
                log_debug ("GstPipeline command is: %s", command);
                
                webcam->_webcamSourceBin =
                    gst_parse_bin_from_description (command, TRUE, &error);
                if (webcam->_webcamSourceBin == NULL) {
                    log_error ("%s: Creation of the webcam_source_bin failed",
                        __FUNCTION__);
                    log_error ("the error was %s", error->message);
                    return false;
                }
                
                g_free(command);
                
                //set _currentFps for actionscript
                _currentFPS = (format->highestFramerate.numerator /
                            format->highestFramerate.denominator);
                
                webcam->_videoSource = 
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "video_source");
                webcam->_capsFilter =
                    gst_bin_get_by_name (GST_BIN (webcam->_webcamSourceBin),
                    "capsfilter");
                
                //drop the new source bin back into the main bin
                gboolean result;
                result = gst_bin_add(GST_BIN(webcam->_webcamMainBin),
                    webcam->_webcamSourceBin);
                if (result != true) {
                    log_error("%s: couldn't drop the sourcebin back into the main bin",
                        __FUNCTION__);
                    return false;
                } else {
                    //get the tee from main bin
                    GstElement *tee = gst_bin_get_by_name(GST_BIN(webcam->_webcamMainBin),
                        "tee");
                    result = gst_element_link(webcam->_webcamSourceBin, tee);
                    if (result != true) {
                        log_error("%s: couldn't link up sourcebin and tee", __FUNCTION__);
                        return false;
                    } else {
                        return true;
                    }
                }
            }
        }
        return true;
    }
    
    //create a display bin that has ghostpads which allow display-to-screen
    //capabilities as well as save-to-file or buffer capabilities (both
    //implemented as bin ghostpads)
    gboolean
    VideoInputGst::webcamCreateMainBin(GnashWebcamPrivate *webcam) {
        GstElement *tee, *video_display_queue, *save_queue;
        gboolean ok;
        GstPad  *pad;
        
        //initialize a new GST pipeline
        webcam->_pipeline = gst_pipeline_new("pipeline");
        
        webcam->_webcamMainBin = gst_bin_new ("webcam_main_bin");
        
        ok = webcamCreateSourceBin(webcam);
        if (ok != true) {
            log_error("%s: problem creating source bin", __FUNCTION__);
            return false;
        }
        
        if ((tee = gst_element_factory_make ("tee", "tee")) == NULL) {
            log_error("%s: problem creating tee element", __FUNCTION__);
            return false;
        }
        if ((save_queue = gst_element_factory_make("queue", "save_queue")) == NULL) {
            log_error("%s: problem creating save_queue element", __FUNCTION__);
            return false;
        }
        if ((video_display_queue = 
            gst_element_factory_make("queue", "video_display_queue")) == NULL) {
            log_error("%s: problem creating video_display_queue element", __FUNCTION__);
            return false;
        }
        
        //add created elements to a bin
        gst_bin_add_many (GST_BIN (webcam->_webcamMainBin), webcam->_webcamSourceBin,
                        tee, save_queue, video_display_queue, NULL);
                        
        ok = gst_element_link(webcam->_webcamSourceBin, tee);
        if (ok != true) {
            log_error("%s: couldn't link webcam_source_bin and tee", __FUNCTION__);
            return false;
        }
        
        ok &= gst_element_link_many (tee, save_queue, NULL);
        if (ok != true) {
            log_error("%s: couldn't link tee and save_queue", __FUNCTION__);
            return false;
        }
        
        ok &= gst_element_link_many (tee, video_display_queue, NULL);
        if (ok != true) {
            log_error("%s: couldn't link tee and video_display_queue", __FUNCTION__);
            return false;
        }
        
        gst_bin_add (GST_BIN(webcam->_pipeline), webcam->_webcamMainBin);
        
        //add ghostpad to save_queue (allows connections between bins)
        pad = gst_element_get_pad (save_queue, "src");
        if (pad == NULL) {
            log_error("%s: couldn't get save_queue_src_pad", __FUNCTION__);
            return false;
        }
        gst_element_add_pad (webcam->_webcamMainBin,
            gst_ghost_pad_new ("save_queue_src", pad));
        gst_object_unref (GST_OBJECT (pad));
        
        //add ghostpad to video_display_queue
        pad = gst_element_get_pad (video_display_queue, "src");
        if (pad == NULL) {
            log_error("%s: couldn't get video_display_queue_pad", __FUNCTION__);
            return false;
        }
        gst_element_add_pad (webcam->_webcamMainBin,
            gst_ghost_pad_new ("video_display_queue_src", pad));
        gst_object_unref (GST_OBJECT (pad));


        if (!ok) {
            log_error("%s: Unable to create main pipeline", __FUNCTION__);
            return false;
        }
        return true;
    }

    gboolean
    VideoInputGst::webcamCreateDisplayBin(GnashWebcamPrivate *webcam) {
        GstElement *video_scale, *video_sink;
        gboolean ok;
        GstPad  *pad;
        
        webcam->_videoDisplayBin = gst_bin_new("video_display_bin");
        
        if (webcam->_videoDisplayBin == NULL) {
            log_error("%s: something went wrong creating the new video_display_bin",
                __FUNCTION__);
            return false;
        }
        
        if ((video_scale = gst_element_factory_make("videoscale", "video_scale")) == NULL) {
            log_error("%s: problem creating video_scale element", __FUNCTION__);
            return false;
        }
        else {
            //set bilinear scaling
            g_object_set (video_scale, "method", 1, NULL);
        }
        
        if ((video_sink = gst_element_factory_make("autovideosink", "video_sink")) == NULL) {
            log_error("%s: problem creating the video_sink element", __FUNCTION__);
            return false;
        }
        
        //add created elements to a bin
        gst_bin_add_many (GST_BIN (webcam->_videoDisplayBin), video_scale, video_sink, NULL);
        
        ok = gst_element_link_many(video_scale, video_sink, NULL);
        if (ok != true) {
            log_error("%s: something went wrong in linking elements in video_display_bin",
                __FUNCTION__);
            return false;
        }
        
        //create ghostpad which can be used to connect this bin to the
        //video_display_queue src ghostpad
        pad = gst_element_get_pad (video_scale, "sink");
        gst_element_add_pad (webcam->_videoDisplayBin, gst_ghost_pad_new ("sink", pad));
        gst_object_unref (GST_OBJECT (pad));
        
        return true;
    }
    
    //make link between display_queue src ghostpad in main_bin and
    //the elements necessary to display video to screen (_videoDisplayBin)
    gboolean
    VideoInputGst::webcamMakeVideoDisplayLink(GnashWebcamPrivate *webcam) {
        if (gst_bin_get_by_name(GST_BIN(webcam->_pipeline), "video_display_bin") == NULL) {
            gst_object_ref(webcam->_videoDisplayBin);
            gst_bin_add (GST_BIN(webcam->_pipeline), webcam->_videoDisplayBin);
        }
        
        gboolean ok;
        GstPad *video_display_queue_src, *video_display_bin_sink;
        
        video_display_queue_src = gst_element_get_pad(webcam->_webcamMainBin,
            "video_display_queue_src");
        video_display_bin_sink = gst_element_get_pad(webcam->_videoDisplayBin,
            "sink");
        
        GstPadLinkReturn padreturn;
        padreturn = gst_pad_link(video_display_queue_src, video_display_bin_sink);
        
        if (padreturn == GST_PAD_LINK_OK) {
            return true;
        } else {
            log_error("something went wrong in the make_video_display_link function");
            return false;
        }
    }
    
    //break the link that displays the webcam video to the screen
    gboolean
    VideoInputGst::webcamBreakVideoDisplayLink(GnashWebcamPrivate *webcam) {
        if (webcam->_pipelineIsPlaying == true) {
            GstStateChangeReturn state;
            state = gst_element_set_state(webcam->_pipeline, GST_STATE_NULL);
            if (state != GST_STATE_CHANGE_FAILURE) {
                webcam->_pipelineIsPlaying = false;
            } else {
                return false;
            }
        }
        
        gboolean ok;
        GstPad *videoDisplayQueueSrc, *videoDisplayBinSink;
        
        videoDisplayQueueSrc = gst_element_get_pad(webcam->_webcamMainBin,
            "video_display_queue_src");
        videoDisplayBinSink = gst_element_get_pad(webcam->_videoDisplayBin,
            "sink");
        
        ok = gst_pad_unlink(videoDisplayQueueSrc, videoDisplayBinSink);
        
        if (ok != true) {
            log_error("%s: the unlinking of the pads failed", __FUNCTION__);
            return false;
        } else {
            return true;
        }
    }
    
    //make link to saveQueue in main bin
    gboolean
    VideoInputGst::webcamMakeVideoSaveLink(GnashWebcamPrivate *webcam) {
        if (gst_bin_get_by_name(GST_BIN(webcam->_pipeline), "video_save_bin") == NULL) {
            gst_object_ref(webcam->_videoSaveBin);
            gst_bin_add(GST_BIN(webcam->_pipeline), webcam->_videoSaveBin);
        }

        //linking
        GstPad *video_save_queue_src, *video_save_sink;
        
        video_save_queue_src = gst_element_get_pad(webcam->_webcamMainBin, "save_queue_src");
        video_save_sink = gst_element_get_pad(webcam->_videoSaveBin, "sink");
        
        GstPadLinkReturn padreturn;
        padreturn = gst_pad_link(video_save_queue_src, video_save_sink);
        
        if (padreturn == GST_PAD_LINK_OK) {
            return true;
        } else {
            log_error("%s: something went wrong in the make_video_display_link function",
                __FUNCTION__);
            return false;
        }
    }
    
    //break link to saveQueue in main bin
    gboolean
    VideoInputGst::webcamBreakVideoSaveLink(GnashWebcamPrivate *webcam) {
        if (webcam->_pipelineIsPlaying == true) {
            GstStateChangeReturn state;
            state = gst_element_set_state(webcam->_pipeline, GST_STATE_NULL);
            if (state != GST_STATE_CHANGE_FAILURE) {
                webcam->_pipelineIsPlaying = false;
            } else {
                return false;
            }
        }
        gboolean ok;
        GstPad *videoSaveQueueSrc, *videoSaveSink;
        GstStateChangeReturn state;
        videoSaveQueueSrc = gst_element_get_pad(webcam->_webcamMainBin,
            "save_queue_src");
        videoSaveSink = gst_element_get_pad(webcam->_videoSaveBin, "sink");
        
        ok = gst_pad_unlink(videoSaveQueueSrc, videoSaveSink);
        if (ok != true) {
            log_error("%s: unlink failed", __FUNCTION__);
            return false;
        } else {
            state = gst_element_set_state(webcam->_videoSaveBin, GST_STATE_NULL);
            if (state != GST_STATE_CHANGE_FAILURE) {
                ok = gst_bin_remove(GST_BIN(webcam->_pipeline), webcam->_videoSaveBin);
                if (ok != true) {
                    log_error("%s: couldn't remove saveBin from pipeline", __FUNCTION__);
                    return false;
                } else {
                    return true;
                }
            } else {
                log_error("%s: videoSaveBin state change failed", __FUNCTION__);
                return false;
            }
        }
    }
    
    //create a bin to take the video stream and dump it out to
    //an ogg file
    gboolean
    VideoInputGst::webcamCreateSaveBin(GnashWebcamPrivate *webcam) {
        GstElement *video_save_csp, *video_save_rate, *video_save_scale, *video_enc;
        GstElement *mux;
        GstPad     *pad;
        gboolean    ok;
        
        webcam->_videoSaveBin = gst_bin_new ("video_save_bin");
        
        if ((video_save_csp =
            gst_element_factory_make("ffmpegcolorspace", "video_save_csp"))
                == NULL) {
            log_error("%s: problem with creating video_save_csp element",
                __FUNCTION__);
            return false;
        }
        if ((video_enc = gst_element_factory_make("theoraenc", "video_enc")) == NULL) {
            log_error("%s: problem with creating video_enc element", __FUNCTION__);
            return false;
        } else {
            g_object_set (video_enc, "keyframe-force", 1, NULL);
        }
        
        if ((video_save_rate = gst_element_factory_make("videorate", "video_save_rate")) == NULL) {
            log_error("%s: problem with creating video_save_rate element", __FUNCTION__);
            return false;
        }
        if ((video_save_scale = gst_element_factory_make("videoscale", "video_save_scale")) == NULL) {
            log_error("%s: problem with creating video_save_scale element", __FUNCTION__);
            return false;
        } else {
            //Use bilinear scaling
            g_object_set (video_save_scale, "method", 1, NULL);
        }
        if ((mux = gst_element_factory_make("oggmux", "mux")) == NULL) {
            log_error("%s: problem with creating mux element", __FUNCTION__);
            return false;
        }
        if ((webcam->_videoFileSink = gst_element_factory_make("filesink", "video_file_sink")) == NULL) {
            log_error("%s: problem with creating video_file_sink element", __FUNCTION__);
            return false;
        } else {
            g_object_set(webcam->_videoFileSink, "location", "vidoutput.ogg", NULL);
        }
        
        //add created elements to the video_save_bin in the datastructure
        gst_bin_add_many (GST_BIN (webcam->_videoSaveBin), video_save_csp, 
                video_save_rate, video_save_scale, video_enc, mux, webcam->_videoFileSink,
                NULL);

        //add ghostpad
        pad = gst_element_get_pad (video_save_csp, "sink");
        gst_element_add_pad (webcam->_videoSaveBin, gst_ghost_pad_new ("sink", pad));
        gst_object_unref (GST_OBJECT (pad));
                                
        ok = gst_element_link_many (video_save_csp, video_save_rate,
            video_save_scale, video_enc, mux, webcam->_videoFileSink, NULL);
        
        if (ok != true) {
            log_error("%s: there was some problem in linking!", __FUNCTION__);
        }
        return true;
    }
    
    //to handle messages while the main capture loop is running
    gboolean
    bus_call (GstBus     *bus,
              GstMessage *msg,
              gpointer data)
    {
      switch (GST_MESSAGE_TYPE (msg)) {

        case GST_MESSAGE_EOS:
            log_trace ("End of stream");
            break;
        
        case GST_MESSAGE_ERROR: {
            gchar  *debug;
            GError *error;

            gst_message_parse_error (msg, &error, &debug);
            g_free (debug);

            log_error ("Error: %s", error->message);
            g_error_free (error);
            
            break;
        }
        default:
            break;
      }

      return TRUE;
    }
    
    //start the pipeline and run the g_main_loop
    gboolean
    VideoInputGst::webcamPlay(GnashWebcamPrivate *webcam) {
        GstStateChangeReturn state;
        GstBus *bus;
        GMainLoop *loop;
        gint ret;
            //setup bus to watch pipeline for messages
            bus = gst_pipeline_get_bus (GST_PIPELINE (webcam->_pipeline));
            ret = gst_bus_add_watch (bus, bus_call, webcam);
            gst_object_unref (bus);
            
            state = gst_element_set_state (webcam->_pipeline, GST_STATE_PLAYING);
            
            if (state != GST_STATE_CHANGE_FAILURE) {
                webcam->_pipelineIsPlaying = true;
                return true;
            } else {
                return false;
            }
    }
    
    gboolean
    VideoInputGst::webcamStop(GnashWebcamPrivate *webcam) {
        GstStateChangeReturn state;
        
        state = gst_element_set_state (webcam->_pipeline, GST_STATE_NULL);
        if (state != GST_STATE_CHANGE_FAILURE) {
            webcam->_pipelineIsPlaying = FALSE;
            return true;
        } else {
            return false;
        }
    }
} //gst namespace
} //media namespace
} //gnash namespace
