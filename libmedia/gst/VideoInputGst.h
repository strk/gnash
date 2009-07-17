// VideoInput.h: Video input processing using Gstreamer
// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_VIDEOINPUTGST_H
#define GNASH_VIDEOINPUTGST_H

#include <boost/cstdint.hpp> // for C99 int types
#include "VideoInput.h"
#include "gst/gst.h"

namespace gnash {
namespace media {
/// \namespace gst
///
/// This namespace is used for the Gstreamer implmentation of the VideoInput
/// class.
namespace gst{

/// \class FramerateFraction
///
/// Convience wrapper class which allows easily calculating fractions from
/// the information returned from probing hardware cameras for supported
/// framerates.
class FramerateFraction{
	public:
		/// \var FramerateFraction::numerator
		/// \brief contains a gint value for the numerator portion of a fraction.
		gint numerator;
		/// \var FramerateFraction::denominator
		/// \brief contains a gint value for the denominator portion of a fraction.
		gint denominator;
		
		/// \brief Constructor which sets the numerator and denominator fields upon construction.
		///
		/// @param num The integer numerator value to initialize the FramerateFraction class with.
		///
		/// @param denom The integer denominator value to initialzie the FramerateFraction class with.
		FramerateFraction(gint num, gint denom);
		
		/// Create a new empty FramerateFraction class.
		FramerateFraction();
};

/// \class GnashWebcam
///
/// The initial data structure used to store enumerated information about
/// attached hardware video input devices. This class is smaller in size
/// than the GnashWebcamPrivate class which is initialized once the user
/// specifies a hardware input device to use in the gnashrc file.
///
class GnashWebcam {
	public:
		/// \brief Accessor to retreive a the private _element variable
		///       from the GnashWebcam class which contains a pointer
		///       to the video source element.
		///
		/// @return GstElement* to the video source element
		GstElement* get_element_ptr() {return _element;};
		
		/// \brief Accessor to set the private _element variable from
		///       the GnashWebcam class.
		///
		/// @param element The GstElement pointer to the video source element.
		void set_element_ptr(GstElement* element) {_element = element;};
		
		/// \brief Accessor to get the private _dev_location variable from
		///       the GnashWebcam class.
		///
		/// @return The _dev_location private variable from GnashWebcam class.
		gchar* get_dev_location() {return _dev_location;};
		
		/// \brief Accessor to set the private _dev_location variable from
		///       the GnashWebcam class.
		///
		/// @param l A gchar* containing the physical location of the video
		///       input hardware device (e.g. on Linux typically would be set
		///       to '/dev/video0').
		void set_dev_location(gchar *l) {_dev_location = l;};
		
		/// \brief Accessor to return the private _gstreamer_src variable
		///       from the GnashWebcam class.
		///
		/// @return The _gstreamer_src variable from the GnashWebcam class.
		///        which should contain the type of the Gstreamer video source
		///        element (e.g. v4lsrc, v4l2src).
		gchar* get_gstreamer_src() {return _gstreamer_src;};
		
		/// \brief Accessor to set the private _gstreamer_src variable
		///       from the GnashWebcam class.
		///
		/// @param s A gchar* containing the type of the Gstreamer source
		///         element type (e.g. v4lsrc, v4l2src, etc)
		void set_gstreamer_src(gchar *s) {_gstreamer_src = s;};
		
		/// \brief Accessor to get the private _product_name variable
		///       from the GnashWebcam class.
		///
		/// @return A gchar* containing the video input's hardware name
		///       (e.g. Built-In Webcam or Microsoft LifeCam VX500).
		gchar* get_product_name() {return _product_name;};
		
		/// \brief Accessor to set the private _product_name variable
		///       from the GnashWebcam class.
		///
		/// @param n A gchar* to the hardware input device's hardware name
		///         (e.g. Built-In Webcam or Microsoft LifeCam VX500).
		void set_product_name(gchar *n) {_product_name = n;};
		
		/// \var GnashWebcam::num_video_formats
		/// \brief Contains an integer value representing the number of
		///       video formats the camera supports (used for iteration
		///       purposes).
		gint   num_video_formats;
		
		/// \var GnashWebcam::video_formats
		/// \brief A GArray containing WebcamVidFormat data structures
		///       (see WebcamVidFormat class documentation for more info).
		GArray* video_formats;
		
		/// \var GnashWebcam::supported_resolutions
		/// \brief A hash table for easy lookup of resolutions the hardware
		///       camera supports.
		GHashTable* supported_resolutions;

		/// Constructor for the GnashWebcam class.
		GnashWebcam();
		
	private:
		/// \var GnashWebcam::_element
		/// \brief GstElement* which points to the video source
		///       element.
		GstElement* _element;
		
		/// \var GnashWebcam::_dev_location
		/// \brief Contains the physical location of the webcam device
		///      (e.g. on Linux typically would be set to /dev/video0).
		gchar* _dev_location;
		
		/// \var GnashWebcam::_gstreamer_src
		/// \brief Contains a gchar* which describes the gstreamer source
		///       type (e.g. v4lsrc or v4l2src).
		gchar* _gstreamer_src;
		
		/// \var GnashWebcam::_product_name
		/// \brief Contains a gchar* which describes the name of the hardware
		///      device (e.g. Built-In Webcam or Microsoft LifeCam VX500).
		gchar* _product_name;
};

/// \class WebcamVidFormat
///
/// Class used to hold enumerated information about usable video formats.
///
class WebcamVidFormat {
	public:
		/// \var WebcamVidFormat::mimetype
		/// \brief Contains a gchar* which describes the raw video input stream
		///       from the camera formated in a Gstreamer video format
		///       type (e.g. video/x-raw-rgb or video/x-raw-yuv).
		gchar *mimetype;
		
		/// \var WebcamVidFormat::width
		/// \brief Contains a gint value describing the width of the selected
		///       format.
		gint   width;
		
		/// \var WebcamVidFormat::height
		/// \brief Contains a gint value describing the height of the selected
		///       format.
		gint   height;
		
		/// \var WebcamVidFormat::num_framerates
		/// \brief Contains a gint value representing the number of framerate
		///       values supported by the format described in the mimetype var.
		gint   num_framerates;
		
		/// \var WebcamVidFormat::framerates
		/// \brief Pointer to a FramerateFraction class which simply holds a
		///      temporary framerate variable while trying to determine the
		///      highest possible supported framerate for the format described
		///      in the mimetype var.
		FramerateFraction *framerates;
		
		/// \var WebcamVidFormat::highest_framerate
		/// \brief Holds the highest_frame supported by the format described
		///      in the mimetype var.
		FramerateFraction highest_framerate;
		
		/// Constructor for the WebcamVidFormat class
		WebcamVidFormat();
};

/// Constructor for the WebcamVidFormat class. This constructor prepares
/// the data structure for data that will come in later. All gint values
/// are initialized to -1 to show that these values have never been set.
///
WebcamVidFormat::WebcamVidFormat() {
	width = -1;
	height = -1;
	num_framerates = -1;
	framerates = NULL;
}

/// Default constructor for the FramerateFraction class. This constructor prepares
/// the data structure for data that will come in later. All gint values
/// are initialized to -1 to show that these values have never been set.
FramerateFraction::FramerateFraction() {
	numerator = -1;
	denominator = -1;
}

/// Secondary constructor for the FramerateFraction class. This constructor
/// initialzes the structure with the numerator and denominator values passed
/// to the constructor.
FramerateFraction::FramerateFraction(gint num, gint denom) {
	numerator = num;
	denominator = denom;
}

/// Constructor for the GnashWebcam class. This constructor prepares the data
/// structure for data that will come in later. Also creates a blank hash table
/// and array.
GnashWebcam::GnashWebcam() {
	set_element_ptr(NULL);
	supported_resolutions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	video_formats = g_array_new (FALSE, FALSE, sizeof (WebcamVidFormat));
	num_video_formats = 0;
}

/// \class GnashWebcamPrivate
///
/// This class is initialized once a hardware video input device is chosen.
/// It is really the workhorse of VideoInputGst. It contains all the important
/// Gstreamer elements (element pointers, bins, pipelines, the GMainLoop, etc.)
///
class GnashWebcamPrivate
{
	public:	  
		/// Constructor for the GnashWebcamPrivate class.
		GnashWebcamPrivate();
		
		/// \brief Accessor to set the private _webcam_device variable in the
		///       GnashWebcamPrivate class.
		///
		/// @param d A pointer to a GnashWebcam class for the selected input device.
		void set_webcam_device(GnashWebcam *d) {_webcam_device = d;}
	  
		/// \brief Accessor to set the private _device_name variable in the
		///       GnashWebcamPrivate class.
		///
		/// @param n A gchar* describing the name of the hardware device
		///       (e.g. Built-In Webcam or Microsoft LifeCam VX500).
		void set_device_name(gchar *n) {_device_name = n;}
	  
	//FIXME: this should eventually be a private or protected data field  
	//protected:
		
		/// \var GnashWebcamPrivate::_pipeline
		///
		/// \brief A pointer to the main Gstreamer pipeline that all
		///      created elements and bins will be dropped into.
		GstElement *_pipeline;	

		/// \var GnashWebcamPrivate::_webcam_source_bin
		///
		/// A pointer to the Gstreamer source bin. This variable is set
		/// inside of the make_webcam_source_bin() function. The pipeline
		/// API of this source bin is written as follows:
		/// videosourcedevice ! capsfilter (ghostpad)
		GstElement *_webcam_source_bin;
		
		/// \var GnashWebcamPrivate::_webcam_main_bin
		///
		/// A pointer to the Gstreamer main bin. This variable is set
		/// inside of the make_webcam_main_bin() function. The pipeline
		/// API of the main bin is written as follows:
		/// tee ! save_queue (ghostpad)
		///
		/// tee ! display_queue (ghostpad)
		///
		/// This basically creates two queues where video stream data sits
		/// and can be attached (optionally) to a display_bin to show the
		/// video onscreen or to a save_bin to mux-out the stream and
		/// save to a file on disk.
		GstElement *_webcam_main_bin;
		
		/// \var GnashWebcamPrivate::_video_display_bin
		///
		/// A pointer to the Gstreamer display bin. This variable is set
		/// inside of the make_webcam_display_bin() function. The pipeline
		/// API of the video_display_bin is written as follows:
		///
		/// videoscale ! videosink
		///
		/// This bin is dropped into the webcam_main_bin, but by default
		/// the connection to display_queue is not made. This means that
		/// even though the video_display_bin is created, it is not linked
		/// and thus will not show video to the screen unless you call the
		/// webcam_make_video_display_link() function.
		GstElement *_video_display_bin;
		
		/// \var GnashWebcamPrivate::_video_save_bin
		///
		/// A pointer to the Gstreamer video_save_bin. This variable is set
		/// inside of the make_webcam_save_bin() function. The pipeline
		/// API of the video_save_bin is written as follows:
		///
		/// ffmpegcolorspace ! videorate ! videoscale ! theoraenc ! oggmux ! filesink
		///
		/// This bin is dropped into the webcam_main_bin and is linked automatically
		/// to the video_save_queue element in the webcam_main_bin
		/// Note: if you want to save the file in a different format, simply
		///   link up video scale to a different encoder and muxer.
		GstElement *_video_save_bin;
		
		/// \var GnashWebcamPrivate::_video_source
		/// \brief Contains a direct link to the src pad in the video source
		///       element. This is different from _webcam_source_bin in that
		///       it points to the video source element INSIDE the bin, not
		///       the source bin itself.
		GstElement *_video_source;
		
		/// \var GnashWebcamPrivate::_caps_filter
		/// \brief Contains a direct link to the src pad in the capsfilter
		///      element.
		GstElement *_capsfilter;
		
		/// \var GnashWebcamPrivate:_video_file_sink
		/// \brief Contains a direct link to the video_file_sink element
		GstElement *_video_file_sink;
		
		/// \var GnashWebcamPrivate::_video_enc
		/// \brief Contains a direct link to the video encoder element
		GstElement *_video_enc;
		
		/// \var GnashWebcamPrivate::_pipeline_is_playing
		/// \brief Boolean value which is changed based on whether or not
		///       the Gstreamer pipeline status is GST_STATE_PLAYING (true)
		///       or GST_STATE_NULL (false), GST_STATE_READY (false),
		///       GST_STATE_PAUSED (false).
		gboolean _pipeline_is_playing;

		/// \var GnashWebcamPrivate::_device_name
		/// \brief Contains a string with the hardware device name (transferred
		///       from GnashWebcam class
		gchar *_device_name;
		
		/// \var GnashWebcamPrivate::_webcam_device
		/// \brief Contains a pointer to the original GnashWebcam class
		///       that was created when enumerating and probing attached
		///       hardware.
		GnashWebcam *_webcam_device;
		
		/// \var GnashWebcamPrivate::_x_resolution
		/// \brief Contains the integer x_resolution variable (the first
		///       value when resolution is written as INTxINT)
		gint _x_resolution;
		
		/// \var GnashWebcamPrivate::_y_resolution
		/// \brief Contains the integer y_resolution variable (the second
		///       value when resolution is written as INTxINT)
		gint _y_resolution;
		
		/// \var GnashWebcamPrivate::_current_format
		/// \brief Contains a pointer to the WebcamVidFormat data structure
		///       selected to be used with this pipeline.
		WebcamVidFormat *_current_format;
	  
		/// \var GnashWebcamPrivate::_loop
		/// \brief Contains a pointer to a GMainLoop which runs to keep
		///       Gstreamer capturing information from the raw data
		///       stream coming in from the hardware input device.
		GMainLoop *_loop;
		
		/// \var GnashWebcamPrivate::_eos_timeout_id
		/// \brief This variable is not currently used, but will eventually
		///       be used as a timeout when networking encapsulation is being
		///       used.
		guint _eos_timeout_id;
};

/// Constructor that initializes all GnashWebcamPrivate variables to have
/// data dropped in later.
GnashWebcamPrivate::GnashWebcamPrivate() {
	_pipeline = NULL;
	_webcam_source_bin = NULL;
	_webcam_main_bin = NULL;
	_video_display_bin = NULL;
	_video_save_bin = NULL;
	_video_source = NULL;
	_capsfilter = NULL;
	_video_file_sink = NULL;
	_video_enc = NULL;
	
	_loop = g_main_loop_new(NULL, false);
	
	_pipeline_is_playing = false;
	
	//FIXME: the resolution here should be able to either
	//a. be determined by the user (or)
	//b. be determined by network latency/bandwidth availability
	_x_resolution = 320;
	_y_resolution = 240;
	_current_format = NULL;
	_eos_timeout_id = 0;
};

/// \class VideoInputGst
///
/// This class is the main class that interacts with the other classes
/// defined in this header file. However, most of the significant information
/// is actually stored in a GnashWebcamPrivate class.
///
class VideoInputGst : public VideoInput, public GnashWebcamPrivate {
public:
	/// Constructor for the VideoInputGst class
	VideoInputGst();
	
	/// Destructor for the VideoInputGst class
	~VideoInputGst();
	
	/// \brief This function interacts with the hardware on the machine
	///       to enumerate information about devices connected. Currently
	///       this function only looks for videotestsources (implemented
	///       in Gstreamer), video4linux and video4linux2 sources.
	/// @return Nothing. All pertantent information is now stored in a
	///      GnashWebcam class.
	void find_vid_devs();
	
	/// \brief This function is important in the flow of the code. It looks
	///       in the gnashrc file to see if you have a default camera defined
	///       and selects that one (if it's defined), otherwise a videotestsrc
	///       will be used for the remainder of the execution of the code.
	///       Currently this code also calls other functions to make bins and
	///       pipelines. It might be abstracted into separate functions later.
	/// @return Nothing.
	void make_webcamDevice_selection();
	
	/// \brief This function makes a temporary pipeline with the selected device
	///       to determine its capabilities (Gstreamer calls these caps). This
	///       information is saved in a GnashWebcamPrivate class and will be
	///       used when enumerating additional information about the camera and
	///       in creating the real pipeline to capture video.
	///
	/// @param dev_select The integer value of the camera the user wants to select.
	///                 This might be changed to the name of the camera, but it's
	///                 currently an integer (if it changes, we need to change the
	///					gnashrc element). If this value is 0, you've selected a
	///                 videotestsrc.
	///
	/// @return Nothing. All pertantent information is stored in a GnashWebcam class.
	void get_selected_caps(gint dev_select);
	
	/// \brief This function is called by get_selected caps to determine what
	///       formats the input device can handle. It saves all the information
	///       it finds into an array of WebcamVidFormat pointers. This information
	///       will then be analyzed and actually added as a supported format in 
	///       a call to add_supported_format.
	///
	/// @param cam A pointer to the GnashWebcam structure you want to use as input.
	/// @param caps A pointer to the capabilities discovered in the get_selected_caps() function.
	///
	/// @return Nothing. All pertantent information is stored in a GnashWebcam class.
	void get_supported_formats(GnashWebcam *cam, GstCaps *caps);
	
	/// \brief This function is called by get_supported_formats when it has
	///       determined that the format being analyzed can be used to capture
	///       information from the hardware device.
	///
	/// @param cam A pointer to the GnashWebcam structure you want to use as input.
	/// @param video_format A pointer to a WebcamVidFormat class that has had all
	///                    variables initialize to their respective values.
	///
	/// @return Nothing. All pertantent information is stored in a GnashWebcam class.
	void add_supported_format(GnashWebcam *cam, WebcamVidFormat *video_format,
		GstStructure *format_structure);
	
	/// \brief This function is called by add_supported_format. Since we have found
	///       a format that will work with the input device, we now need to figure
	///       out what framerate the camera can capture at that corresponds with the
	///       format being analyzed.
	///
	/// @param video_format A pointer to a WebcamVidFormat class taht has had all
	///                    variables initialized to their respective values.
	/// @param structure A pointer to a structure initialized with the capabilities
	///                    of the selected input device.
	///
	/// @return Nothing. All pertantent information is stored in a WebcamVidFormat class.
	void get_supported_framerates(WebcamVidFormat *video_format, GstStructure *structure);

	/// \brief This function runs through the list of framerates determined by
	///       get_supported_framerates() and finds the highest supported framerate
	///       less than 30fps.
	///
	/// @param format A pointer to the chosen format structure.
	///
	/// @return Nothing. All pertantent information is stored in the structure
	///        passed in (a WebcamVidFormat class).
	void find_highest_framerate(WebcamVidFormat *format);
	
	/// \brief Function is called when all the information has been enumerated
	///       that can be stored in the GnashWebcam structure. Now transfer the
	///       important information from the GnashWebcam structure to the
	///       GnashWebcamPrivate structure which is larger because it has
	///       space to store Gstreamer pipeline, element and bin elements.
	///       See definition of GnashWebcamPrivate for more info.
	///
	/// @param dev_select The integer value of the camera the user wants to select.
	///                 This might be changed to the name of the camera, but it's
	///                 currently an integer (if it changes, we need to change the
	///					gnashrc element). If this value is 0, you've selected a
	///                 videotestsrc.
	///
	/// @return A pointer to the newly created GnashWebcamPrivate structure.
	GnashWebcamPrivate* transfer_to_private(gint dev_select);
	
	/// \brief Function creates the source bin. For more information on pipeline
	///      implementation and this function in general see the definition of
	///      the _webcam_source_bin variable in the GnashWebcamPrivate structure
	///      documentation.
	///
	/// @param webcam A pointer to the GnashWebcamPrivate webcam structure
	///             created previously in a call to transfer_to_private()
	///
	/// @return True if everything went correctly (making elements, dropping
	///        into bins and linking elements), false otherwise.
	gboolean webcam_create_source_bin(GnashWebcamPrivate *webcam);
	
	/// \brief Function creates the main bin. For more information on pipeline
	///       implementation and this function in general see the definition of
	///       the _webcam_main_bin variable in the GnashWebcamPrivate structure
	///       documentation.
	///
	/// @param webcam A pointer to the GnashWebcamPrivate webcam structure
	///             created previously in a call to transfer_to_private()
	///
	/// @return True if everything went correctly (making elements, dropping
	///        into bins and linking elements), false otherwise.
	gboolean webcam_create_main_bin(GnashWebcamPrivate *webcam);
	
	/// \brief Function creates the display bin. For more information on pipeline
	///       implementation and this function in general see the definition of
	///       the _video_display_bin variable in the GnashWebcamPrivate structure
	///       documentation.
	///
	/// @param webcam A pointer to the GnashWebcamPrivate webcam structure
	///             created previously in a call to transfer_to_private()
	///
	/// @return True if everything went correctly (making elements, dropping
	///        into bins and linking elements), false otherwise.
	gboolean webcam_create_display_bin(GnashWebcamPrivate *webcam);
	
	/// \brief Function links the video_display_bin to the video_display_queue
	///       in the main bin.
	///
	/// @param webcam A pointer to the GnashWebcamPrivate webcam structure
	///             created previously in a call to transfer_to_private()
	///
	/// @return True if the link to the video_display_queue was successful,
	///        False otherwise.
	gboolean webcam_make_video_display_link(GnashWebcamPrivate *webcam);

	/// \brief Function creates the save bin. For more information on pipeline
	///       implementation and this function in general see the definition of
	///       the _webcam_save_bin variable in the GnashWebcamPrivate structure
	///       documentation.
	///
	/// @param webcam A pointer to the GnashWebcamPrivate webcam structure
	///             created previously in a call to transfer_to_private()
	///
	/// @return True if everything went correctly (making elements, dropping
	///        into bins and linking elements), false otherwis
	gboolean webcam_create_save_bin(GnashWebcamPrivate *webcam);
	
	/// \brief Function starts up the pipeline designed earlier in code
	///      execution. This puts everything into motion.
	///
	/// @param webcam A pointer to the GnashWebcamPrivate webcam structure
	///             created previously in a call to transfer_to_private()
	///
	void webcam_play(GnashWebcamPrivate *webcam);
	
	/// \brief Accessor which returns the vid_vect private variable in the
	///       VideoInputGst class.
	///
	/// @return A pointer to a vector of GnashWebcam pointers.
	std::vector<GnashWebcam*>* get_vid_vect() {return &_vid_vect;}
	
	/// \brief Accessor which sets the number of devices in the vid_vect
	///
	/// @param i The integer value representing the number of devices attached
	///         to the machine.
	void set_numdevs(int i) {_numdevs = i;}
	
	/// \brief Accessor which returns the number of video devices attached
	///      to the machine (useful in accessing the vid_vect vector).
	///
	/// @return The _numdev variable in the VideoInputGst class.
	int get_numdevs() {return _numdevs;}
	
	/// \brief Accessor which increments the number of video devices
	///       attached to the machine.
	void increment_numdevs() {_numdevs += 1;}
	
private:
	/// \var VideoInputGst::_vid_vect
	/// \brief A vector containing pointers to GnashWebcam classes.
	std::vector<GnashWebcam*> _vid_vect;
	
	/// \var VideoInputGst::_numdevs
	/// \brief An integer value containing the number of devices attached
	///       to the machine.
	gint _numdevs;
	
	/// \var VideoInputGst::_global_webcam
	/// \brief Convienient pointer to the selected device's GnashWebcamPrivate
	///       class structure.
	GnashWebcamPrivate *_global_webcam;
};

} //gst namespace
} // gnash.media namespace 
} // gnash namespace

#endif // __VIDEOINPUT_H__
