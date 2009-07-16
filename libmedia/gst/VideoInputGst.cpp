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
	}
	
	VideoInputGst::~VideoInputGst() {
		log_unimpl("Video Input destructor");
	}
	
	//populates video devices to a vector of GnashWebcam pointers
	//which contain important information about the hardware camera
	//inputs available on the machine
	void
	VideoInputGst::find_vid_devs() {
		_numdevs = 0;
		
		//find video test sources
		GstElement *element;
		element = gst_element_factory_make ("videotestsrc", "vidtestsrc");
		
		if (element == NULL) {
			log_error("%s: Could not create video test source. do you have gst-base-plugins installed?", __FUNCTION__);
			_vid_vect.push_back(NULL);
			_numdevs += 1;
		} else {
			_vid_vect.push_back(new GnashWebcam);
			_vid_vect[_numdevs]->set_element_ptr(element);
			_vid_vect[_numdevs]->set_gstreamer_src(g_strdup_printf("videotestsrc"));
			_vid_vect[_numdevs]->set_product_name(g_strdup_printf("videotest"));
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
				log_trace("No v4l video sources found...checking for other vid inputs\n");
			}
			else { 
				_vid_vect.push_back(new GnashWebcam);
				_vid_vect[_numdevs]->set_element_ptr(element);
				_vid_vect[_numdevs]->set_gstreamer_src(g_strdup_printf("v4lsrc"));
				_vid_vect[_numdevs]->set_product_name(dev_name);
				
				//set device location information (e.g. /dev/video0)
				gchar *location;
				g_object_get (element, "device", &location , NULL);
				_vid_vect[_numdevs]->set_dev_location(location);
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
				g_print("no v4l2 video sources found.\n");
			}
			else { 
				_vid_vect.push_back(new GnashWebcam);
				_vid_vect[_numdevs]->set_element_ptr(element);
				_vid_vect[_numdevs]->set_gstreamer_src(g_strdup_printf("v4l2src"));
				_vid_vect[_numdevs]->set_product_name(dev_name);
				
				//set device location information (e.g. /dev/video0)
				gchar *location;
				g_object_get (element, "device", &location , NULL);
				_vid_vect[_numdevs]->set_dev_location(location);
				_numdevs += 1;
			}
		}
		if (devarr) {
			g_value_array_free (devarr);
		}
	}
	
	//called by add_supported_format. finds the highest possible framerate
	//to record at (can be shaped down by a filter for performance)
	void
	VideoInputGst::find_highest_framerate(WebcamVidFormat *format)
	{
		gint framerate_numerator;
		gint framerate_denominator;
		gint i;

		//Select the highest framerate up to less than or equal to 30 Hz
		framerate_numerator   = 1;
		framerate_denominator = 1;
		for (i = 0; i < format->num_framerates; i++) {
			gfloat framerate = format->framerates[i].numerator / format->framerates[i].denominator;
			if (framerate > ((float) framerate_numerator / framerate_denominator)
			   && framerate <= 30) {
				framerate_numerator   = format->framerates[i].numerator;
				framerate_denominator = format->framerates[i].denominator;
			}
		}
		//set highest found above
		format->highest_framerate.numerator = framerate_numerator;
		format->highest_framerate.denominator = framerate_denominator;
	}
	
	//find the framerates at which the selected format can handle input
	void
	VideoInputGst::get_supported_framerates(WebcamVidFormat *video_format, GstStructure *structure)
	{
		const GValue *framerates;
		gint i, j;
		
		//note that framerates may contain one value, a list, or a range
		framerates = gst_structure_get_value (structure, "framerate");
		if (GST_VALUE_HOLDS_FRACTION (framerates)) {
			video_format->num_framerates = 1;
			video_format->framerates = g_new0 (FramerateFraction, video_format->num_framerates);
			video_format->framerates[0].numerator = gst_value_get_fraction_numerator (framerates);
			video_format->framerates[0].denominator = gst_value_get_fraction_denominator (framerates);
		}
		else if (GST_VALUE_HOLDS_LIST (framerates)) {
			video_format->num_framerates = gst_value_list_get_size (framerates);
			video_format->framerates = g_new0 (FramerateFraction, video_format->num_framerates);
			for (i = 0; i < video_format->num_framerates; i++) {
				const GValue *value;
				value = gst_value_list_get_value (framerates, i);
				video_format->framerates[i].numerator = gst_value_get_fraction_numerator (value);
				video_format->framerates[i].denominator = gst_value_get_fraction_denominator (value);
			}
		}
		else if (GST_VALUE_HOLDS_FRACTION_RANGE (framerates)) {
			gint numerator_min, denominator_min, numerator_max, denominator_max;
			const GValue *fraction_range_min;
			const GValue *fraction_range_max;

			fraction_range_min = gst_value_get_fraction_range_min (framerates);
			numerator_min      = gst_value_get_fraction_numerator (fraction_range_min);
			denominator_min    = gst_value_get_fraction_denominator (fraction_range_min);

			fraction_range_max = gst_value_get_fraction_range_max (framerates);
			numerator_max      = gst_value_get_fraction_numerator (fraction_range_max);
			denominator_max    = gst_value_get_fraction_denominator (fraction_range_max);
			g_print ("FractionRange: %d/%d - %d/%d\n", numerator_min, denominator_min, numerator_max, denominator_max);

			video_format->num_framerates = (numerator_max - numerator_min + 1) * (denominator_max - denominator_min + 1);
			video_format->framerates = g_new0 (FramerateFraction, video_format->num_framerates);
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
			g_critical ("GValue type %s, cannot be handled for framerates", G_VALUE_TYPE_NAME (framerates));
		}
	}
	
	//we found a supported framerate and want to add the information to 
	//the GnashWebcam structure
	void
	VideoInputGst::add_supported_format(GnashWebcam *cam, WebcamVidFormat *video_format,
		GstStructure *format_structure)
	{
		gint i;
		gchar *resolution;
		
		get_supported_framerates(video_format, format_structure);
		find_highest_framerate(video_format);
		
		resolution = g_strdup_printf ("%ix%i", video_format->width,
			  video_format->height);
		i = GPOINTER_TO_INT(g_hash_table_lookup (cam->supported_resolutions, resolution));
		
		//if i returns a value, maybe this resolution has been added previously?
		if(i) {
			WebcamVidFormat *curr_format = &g_array_index(cam->video_formats, WebcamVidFormat, i - 1);
			gfloat new_framerate = (float)(video_format->highest_framerate.numerator / 
				  video_format->highest_framerate.denominator);
			gfloat curr_framerate = (float)(curr_format->highest_framerate.numerator /
									  curr_format->highest_framerate.denominator);
			if (new_framerate > curr_framerate) {
				log_debug("higher framerate replacing existing format\n");
				*curr_format = *video_format;
			}
			
			g_free (resolution);
			
			return;
		}
		
		g_array_append_val (cam->video_formats, *video_format);
		g_hash_table_insert (cam->supported_resolutions, resolution,
			  GINT_TO_POINTER(cam->num_video_formats + 1));

		cam->num_video_formats++;
	}
	
	//pulls webcam device selection from gnashrc (will eventually tie into
	//gui)
	void
	VideoInputGst::make_webcamDevice_selection() {
		int dev_select;
		dev_select = rcfile.getWebcamDevice();
		if (dev_select == -1) {
			log_error("%s: No webcam selected in rc file, setting to videotestsource", __FUNCTION__);
			rcfile.setWebcamDevice(0);
		} else {
			log_trace("Camera %d specified in gnashrc file, using that one.\n", dev_select);
		}
		//now that a selection has been made, get capabilities of that device
		get_selected_caps(rcfile.getWebcamDevice());
		
		//now transfer gathered information over to the structure that will hold
		//pipelining information
		GnashWebcamPrivate *webcam = NULL;
		webcam = transfer_to_private(dev_select);
		if (webcam == NULL) {
			log_error("%s: GnashWebcamPrivate transfer didn't work as intended", __FUNCTION__);
		}
		
		//now create the main bin (also calls webcam_create_source_bin)
		gboolean result = false;
		result = webcam_create_main_bin(webcam);
		if (result != true) {
			log_error("%s: webcam_create_main_bin reported an error (returned false)", __FUNCTION__);
		} else {
			result = false;
		}
		
		//now create video display bin
		result = webcam_create_display_bin(webcam);
		if (result != true) {
			log_error("%s: webcam_create_display_bin reported an error (returned false)", __FUNCTION__);
		} else {
			result = false;
		}
		
		//try to link up the main and display bins
		result = webcam_make_video_display_link(webcam);
		if (result != true) {
			log_error("%s: webcam_make_video_display_link reported an error (returned false)", __FUNCTION__);
		} else {
			result = false;
		}
		
		//now create the save bin
		result = webcam_create_save_bin(webcam);
		if (result != true) {
			log_error("%s: webcam_create_save_bin reported an error (returned false)", __FUNCTION__);
		} else {
			result = false;
		}
		
		//start up the pipeline
		webcam_play(webcam);
	}

	//called after a device selection, this starts enumerating the device's
	//capabilities
	void
	VideoInputGst::get_selected_caps(gint dev_select)
	{
		GstElement *pipeline;
		gchar *command;
		GError *error = NULL;
		GstStateChangeReturn return_val;
		GstBus *bus;
		GstMessage *message;
		
		GnashWebcam *data_struct = _vid_vect[dev_select];
		GstElement *element;
		element = data_struct->get_element_ptr();
		
		//create tester pipeline to enumerate properties
		command = g_strdup_printf ("%s name=src device=%s ! fakesink",
			data_struct->get_gstreamer_src(), data_struct->get_dev_location());
		pipeline = gst_parse_launch(command, &error);
		if ((pipeline != NULL) && (error == NULL)) {
			//Wait at most 5 seconds for the pipeline to start playing
			gst_element_set_state (pipeline, GST_STATE_PLAYING);
			return_val = gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
			
			//errors on bus?
			bus = gst_element_get_bus (pipeline);
			message = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
			
			if (GST_IS_OBJECT(bus)){
				gst_object_unref (bus);
			} else {
				log_error("%s: Pipeline bus isn't an object for some reason", __FUNCTION__);
			}
			
			//if everything above worked properly, begin probing for values
			if ((return_val == GST_STATE_CHANGE_SUCCESS) && (message == NULL)) {
				GstElement *src;
				GstPad *pad;
				gchar *name;
				GstCaps *caps;
				
				gst_element_set_state(pipeline, GST_STATE_PAUSED);
				
				src = gst_bin_get_by_name(GST_BIN(pipeline), "src");
				
				//get the pad, find the capabilities for probing in supported vid formats
				pad  = gst_element_get_pad (src, "src");
				caps = gst_pad_get_caps (pad);
				if (GST_IS_OBJECT(pad)) {
					gst_object_unref (pad);
				} else {
					log_error("%s: Template pad isn't an object for some reason", __FUNCTION__);
				}
				
				get_supported_formats(data_struct, caps);
				
				gst_caps_unref (caps);
			}
			gst_element_set_state (pipeline, GST_STATE_NULL);
			if (GST_IS_OBJECT(pipeline)){
				gst_object_unref (pipeline);
			} else {
				log_error("%s: pipeline isn't an object for some reason", __FUNCTION__);
			}
		}
	   
		if (error) {
		  g_error_free (error);
		}
		g_free (command);
	}

	//probe the selected camera for the formats it supports
	void
	VideoInputGst::get_supported_formats(GnashWebcam *cam, GstCaps *caps) {
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

				  video_format.mimetype = g_strdup (gst_structure_get_name (structure));
				  gst_structure_get_int (structure, "width", &(video_format.width));
				  gst_structure_get_int (structure, "height", &(video_format.height));
				  add_supported_format(cam, &video_format, structure);
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

					video_format.mimetype = g_strdup (gst_structure_get_name (structure));
					video_format.width    = cur_width;
					video_format.height   = cur_height;
					add_supported_format(cam, &video_format, structure);
					cur_width  *= 2;
					cur_height *= 2;
				}

				cur_width  = max_width;
				cur_height = max_height;
				while (cur_width > min_width && cur_height > min_height) {
					WebcamVidFormat video_format;

					video_format.mimetype = g_strdup (gst_structure_get_name (structure));
					video_format.width    = cur_width;
					video_format.height   = cur_height;
					add_supported_format(cam, &video_format, structure);
					cur_width  /= 2;
					cur_height /= 2;
				}
			}
			else {
				log_error("%s: GValue type %s, cannot be handled for resolution width", __FUNCTION__, G_VALUE_TYPE_NAME (width));
			}
		}
	}

	//move the selected camera information to a more robust data structure
	//to store pipeline-ing information
	GnashWebcamPrivate*
	VideoInputGst::transfer_to_private(gint dev_select)
	{
		GnashWebcamPrivate *webcam = new GnashWebcamPrivate;
		if (webcam != NULL) {
			webcam->set_webcam_device(_vid_vect[dev_select]);
			webcam->set_device_name(_vid_vect[dev_select]->get_product_name());
			_global_webcam = webcam;
		} else {
			log_error("%s: was passed a NULL pointer (supposed to point to a GnashWebcamPrivate struct)", __FUNCTION__);
		}
		return webcam;
	}

	//create a bin containing the source and a connector ghostpad
	gboolean
	VideoInputGst::webcam_create_source_bin(GnashWebcamPrivate *webcam) {
		GError *error = NULL;
		gchar *command = NULL;
		
		if(webcam->_webcam_device == NULL) {
			log_trace("%s: looks like you don't have any webcams chosen, using videotestsrc", __FUNCTION__);
			webcam->_webcam_source_bin = gst_parse_bin_from_description (
				"videotestsrc name=video_source ! capsfilter name=capsfilter",
				TRUE, &error);
			log_debug("GstPipeline command: videotestsrc name=video_source ! capsfilter name=capsfilter");
		}
		else {
			WebcamVidFormat *format = NULL;
			gint i;
			gchar *resolution;
			
			resolution = g_strdup_printf("%ix%i", webcam->_x_resolution,
									  webcam->_y_resolution);
									  
			//use these resolutions determined above if the camera supports it
			if (webcam->_x_resolution != 0 && webcam->_y_resolution != 0) {
				
				i = GPOINTER_TO_INT(g_hash_table_lookup (webcam->_webcam_device->supported_resolutions, resolution));
				//the selected res is supported if i
				if (i) {
					format = &g_array_index (webcam->_webcam_device->video_formats,
							 WebcamVidFormat, i - 1);
				}
			}
			
			//if format didn't get set, something went wrong. try picking
			//the first supported format and a different supported resolution
			if (!format) {
				format = &g_array_index (webcam->_webcam_device->video_formats,
					 WebcamVidFormat, 0);
				for (i = 1; i < webcam->_webcam_device->num_video_formats; i++) {
					//debug
					//g_print("value from vid format is: %d \n", g_array_index(webcam->_webcam_device->video_formats, WebcamVidFormat, i).width);
					//g_print("value from format var is: %d \n", format->width);
					if (g_array_index (webcam->_webcam_device->video_formats,
							   WebcamVidFormat, i).width <= format->width){
						format = &g_array_index (webcam->_webcam_device->video_formats,
							 WebcamVidFormat, i);
					}
				}
			}
			
			webcam->_current_format = format;
			g_free(resolution);
			
			//if format isn't set, something is still going wrong, make generic
			//components and see if they work!
			if (format == NULL) {
				if (error != NULL) {
					g_error_free (error);
					error = NULL;
				}
				webcam->_webcam_source_bin = gst_parse_bin_from_description ("videotestsrc name=video_source",
																TRUE, &error);
				webcam->_video_source = gst_bin_get_by_name (GST_BIN (webcam->_webcam_source_bin), "video_source");
				
				//if there are still errors, something's up, return out of function
				if (error != NULL) {
					g_error_free (error);
					return false;
				}
				webcam->_capsfilter = gst_bin_get_by_name (GST_BIN (webcam->_webcam_source_bin),
											  "capsfilter");
				return true;
			}
			
			//execution here means we're good to make the pipeline
			else {
				command = g_strdup_printf (
				  "%s name=video_source device=%s ! capsfilter name=capsfilter caps=video/x-raw-rgb,width=%d,height=%d,framerate=%d/%d;video/x-raw-yuv,width=%d,height=%d,framerate=%d/%d",
				  webcam->_webcam_device->get_gstreamer_src(),
				  webcam->_webcam_device->get_dev_location(),
				  format->width,
				  format->height,
				  format->highest_framerate.numerator,
				  format->highest_framerate.denominator,
				  format->width,
				  format->height,
				  format->highest_framerate.numerator,
				  format->highest_framerate.denominator);
				
				//debug
				log_debug ("GstPipeline command is: %s\n", command);
				
				webcam->_webcam_source_bin = gst_parse_bin_from_description (command, TRUE, &error);
				if (webcam->_webcam_source_bin == NULL) {
					log_error ("%s: Creation of the webcam_source_bin failed", __FUNCTION__);
					g_print ("the error was %s\n", error->message);
					return false;
				}
				
				g_free(command);
				
				webcam->_video_source = gst_bin_get_by_name (GST_BIN (webcam->_webcam_source_bin), "video_source");
				webcam->_capsfilter = gst_bin_get_by_name (GST_BIN (webcam->_webcam_source_bin), "capsfilter");
				return true;
			}
		}
		return true;
	}
	
	//create a display bin that has ghostpads which allow display-to-screen
	//capabilities as well as save-to-file or buffer capabilities (both
	//implemented as bin ghostpads)
	gboolean
	VideoInputGst::webcam_create_main_bin(GnashWebcamPrivate *webcam) {
		GstElement *tee, *video_display_queue, *save_queue;
		gboolean ok;
		GstPad  *pad;
		
		//initialize a new GST pipeline
		webcam->_pipeline = gst_pipeline_new("pipeline");
		
		webcam->_webcam_main_bin = gst_bin_new ("webcam_main_bin");
		
		ok = webcam_create_source_bin(webcam);
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
		if ((video_display_queue = gst_element_factory_make("queue", "video_display_queue")) == NULL) {
			log_error("%s: problem creating video_display_queue element", __FUNCTION__);
			return false;
		}
		
		//add created elements to a bin
		gst_bin_add_many (GST_BIN (webcam->_webcam_main_bin), webcam->_webcam_source_bin,
						tee, save_queue, video_display_queue, NULL);
						
		ok = gst_element_link(webcam->_webcam_source_bin, tee);
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
		
		gst_bin_add (GST_BIN(webcam->_pipeline), webcam->_webcam_main_bin);
		
		//add ghostpad to save_queue (allows connections between bins)
		pad = gst_element_get_pad (save_queue, "src");
		if (pad == NULL) {
			log_error("%s: couldn't get save_queue_src_pad", __FUNCTION__);
			return false;
		}
		gst_element_add_pad (webcam->_webcam_main_bin, gst_ghost_pad_new ("save_queue_src", pad));
		gst_object_unref (GST_OBJECT (pad));
		
		//add ghostpad to video_display_queue
		pad = gst_element_get_pad (video_display_queue, "src");
		if (pad == NULL) {
			log_error("%s: couldn't get video_display_queue_pad", __FUNCTION__);
			return false;
		}
		gst_element_add_pad (webcam->_webcam_main_bin, gst_ghost_pad_new ("video_display_queue_src", pad));
		gst_object_unref (GST_OBJECT (pad));


		if (!ok) {
			log_error("%s: Unable to create main pipeline", __FUNCTION__);
			return false;
		}

		return true;
	}

	gboolean
	VideoInputGst::webcam_create_display_bin(GnashWebcamPrivate *webcam) {
		GstElement *video_scale, *video_sink;
		gboolean ok;
		GstPad  *pad;
		
		webcam->_video_display_bin = gst_bin_new("video_display_bin");
		
		if (webcam->_video_display_bin == NULL) {
			log_error("%s: something went wrong creating the new video_display_bin", __FUNCTION__);
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
		gst_bin_add_many (GST_BIN (webcam->_video_display_bin), video_scale, video_sink, NULL);
		
		//drop the display bin into the pipeline
		//gst_bin_add (GST_BIN (webcam->_webcam_main_bin), webcam->_video_display_bin);
		gst_bin_add (GST_BIN (webcam->_pipeline), webcam->_video_display_bin);
		
		ok = gst_element_link_many(video_scale, video_sink, NULL);
		if (ok != true) {
			log_error("%s: something went wrong in linking elements in video_display_bin", __FUNCTION__);
			return false;
		}
		
		//create ghostpad which can be used to connect this bin to the
		//video_display_queue src ghostpad
		pad = gst_element_get_pad (video_scale, "sink");
		gst_element_add_pad (webcam->_video_display_bin, gst_ghost_pad_new ("sink", pad));
		gst_object_unref (GST_OBJECT (pad));
		
		return true;
	}
	
	//make link between display_queue src ghostpad in main_bin and
	//the elements necessary to display video to screen (_video_display_bin)
	gboolean
	VideoInputGst::webcam_make_video_display_link(GnashWebcamPrivate *webcam) {
		gboolean ok;
		GstPad *video_display_queue_src, *video_display_bin_sink;
		
		video_display_queue_src = gst_element_get_pad(webcam->_webcam_main_bin, "video_display_queue_src");
		video_display_bin_sink = gst_element_get_pad(webcam->_video_display_bin, "sink");
		
		GstPadLinkReturn padreturn;
		padreturn = gst_pad_link(video_display_queue_src, video_display_bin_sink);
		
		if (padreturn == GST_PAD_LINK_OK) {
			return true;
		} else {
			log_error(_("something went wrong in the make_video_display_link function"));
			return false;
		}
	}
	
	//create a bin to take the video stream and dump it out to
	//an ogg file
	gboolean
	VideoInputGst::webcam_create_save_bin(GnashWebcamPrivate *webcam) {
		GstElement *video_save_csp, *video_save_rate, *video_save_scale, *video_enc;
		GstElement *mux;
		GstPad     *pad;
		gboolean    ok;
		
		webcam->_video_save_bin = gst_bin_new ("video_save_bin");
		
		if ((video_save_csp = gst_element_factory_make("ffmpegcolorspace", "video_save_csp")) == NULL) {
			log_error("%s: problem with creating video_save_csp element", __FUNCTION__);
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
		if ((webcam->_video_file_sink = gst_element_factory_make("filesink", "video_file_sink")) == NULL) {
			log_error("%s: problem with creating video_file_sink element", __FUNCTION__);
			return false;
		} else {
			g_object_set(webcam->_video_file_sink, "location", "vidoutput.ogg", NULL);
		}
		
		//add created elements to the video_save_bin in the datastructure
		gst_bin_add_many (GST_BIN (webcam->_video_save_bin), video_save_csp, 
				video_save_rate, video_save_scale, video_enc, mux, webcam->_video_file_sink,
				NULL);
					  
		//add ghostpad
		pad = gst_element_get_pad (video_save_csp, "sink");
		gst_element_add_pad (webcam->_video_save_bin, gst_ghost_pad_new ("sink", pad));
		gst_object_unref (GST_OBJECT (pad));
								
		ok = gst_element_link_many (video_save_csp, video_save_rate, video_save_scale, video_enc, mux, webcam->_video_file_sink, NULL);
		
		if (ok != true) {
			log_error("%s: there was some problem in linking!", __FUNCTION__);
		}
		
		//added starting here
		
		gst_bin_add (GST_BIN(webcam->_pipeline), webcam->_video_save_bin);
		
		//linking
		GstPad *video_save_queue_src, *video_save_sink;
		
		video_save_queue_src = gst_element_get_pad(webcam->_webcam_main_bin, "save_queue_src");
		video_save_sink = gst_element_get_pad(webcam->_video_save_bin, "sink");
		
		GstPadLinkReturn padreturn;
		padreturn = gst_pad_link(video_save_queue_src, video_save_sink);
		
		if (padreturn == GST_PAD_LINK_OK) {
			return true;
		} else {
			log_error("%s: something went wrong in the make_video_display_link function", __FUNCTION__);
			return false;
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
			log_trace ("End of stream\n");
			g_main_loop_quit (((class GnashWebcamPrivate *)data)->_loop);
			break;
		
		case GST_MESSAGE_ERROR: {
			gchar  *debug;
			GError *error;

			gst_message_parse_error (msg, &error, &debug);
			g_free (debug);

			g_printerr ("Error: %s\n", error->message);
			g_error_free (error);
			
			g_main_loop_quit (((class GnashWebcamPrivate *)data)->_loop);
			break;
		}
		default:
			break;
	  }

	  return TRUE;
	}
	
	//start the pipeline and run the g_main_loop
	void
	VideoInputGst::webcam_play(GnashWebcamPrivate *webcam) {
		GstStateChangeReturn state;
		GstBus *bus;
		GMainLoop *loop;
		gint ret;
		
		//setup bus to watch pipeline for messages
		bus = gst_pipeline_get_bus (GST_PIPELINE (webcam->_pipeline));
		ret = gst_bus_add_watch (bus, bus_call, webcam);
		gst_object_unref (bus);

		//declare clock variables to record time (mainly useful in debug)
		GstClockTime tfthen, tfnow;
		GstClockTimeDiff diff;
		
		tfthen = gst_util_get_timestamp ();
		state = gst_element_set_state (webcam->_pipeline, GST_STATE_PLAYING);
		
		if (state == GST_STATE_CHANGE_SUCCESS) {
			webcam->_pipeline_is_playing = true;
		}
		
		loop = webcam->_loop;
		log_trace("running (ctrl-c in terminal to quit).....\n");
		g_main_loop_run(loop);
		log_trace("main loop done...\n");
		tfnow = gst_util_get_timestamp ();
		diff = GST_CLOCK_DIFF (tfthen, tfnow);
		log_trace(("Execution ended after %" G_GUINT64_FORMAT " ns.\n"), diff);
	}
} //gst namespace
} //media namespace
} //gnash namespace
