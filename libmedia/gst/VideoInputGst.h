// VideoInput.h: Video input processing using Gstreamer
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include <vector>
#include <boost/cstdint.hpp> // for C99 int types
#include "VideoInput.h"
#include "gst/gst.h"

namespace gnash {
namespace media {
/// \namespace gst
///
/// This namespace is used for the Gstreamer implmentation of the VideoInput
/// class.
namespace gst {

class GnashWebcam;
class GnashWebcamPrivate;

/// \class FramerateFraction
///
/// Convience wrapper class which allows easily calculating fractions from
/// the information returned from probing hardware cameras for supported
/// framerates.
class FramerateFraction
{
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

/// \class WebcamVidFormat
///
/// Class used to hold enumerated information about usable video formats.
///
class WebcamVidFormat
{
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
    
    /// \var WebcamVidFormat::numFramerates
    /// \brief Contains a gint value representing the number of framerate
    ///       values supported by the format described in the mimetype var.
    gint   numFramerates;
    
    /// \var WebcamVidFormat::framerates
    /// \brief Pointer to a FramerateFraction class which simply holds a
    ///      temporary framerate variable while trying to determine the
    ///      highest possible supported framerate for the format described
    ///      in the mimetype var.
    FramerateFraction *framerates;
    
    /// \var WebcamVidFormat::highestFramerate
    /// \brief Holds the highest_frame supported by the format described
    ///      in the mimetype var.
    FramerateFraction highestFramerate;
    
    /// Constructor for the WebcamVidFormat class
    WebcamVidFormat();
};


/// \class VideoInputGst
///
/// This class is the main class that interacts with the other classes
/// defined in this header file. However, most of the significant information
/// is actually stored in a GnashWebcamPrivate class.
///
class VideoInputGst : public VideoInput
{
public:
 
    /// Constructor for the VideoInputGst class
    VideoInputGst();
    
    /// Destructor for the VideoInputGst class
    ~VideoInputGst();
    
    static void getNames(std::vector<std::string>& names);
    
    /// Return the current activity level of the webcam
    //
    /// @return     A double specifying the amount of motion currently
    ///             detected by the camera.
    double activityLevel () const { return _activityLevel; }
    
    /// The maximum available bandwidth for outgoing connections
    //
    /// TODO: see if this should really be here.
    size_t bandwidth() const { return _bandwidth; }
    
    void setBandwidth(size_t bandwidth) {
        _bandwidth = bandwidth;
    }

    /// The current frame rate of the webcam
    //
    /// @return     A double specifying the webcam's current FPS
    double currentFPS() const { return _currentFPS; }
    
    /// The maximum FPS rate of the webcam
    //
    /// @return     A double specifying the webcam's maximum FPS
    double fps() const { return _fps; }

    /// Return the height of the webcam's frame
    size_t height() const { return _height; }
    
    /// Return the width of the webcam's frame
    size_t width() const { return _width; }
    
    /// The index of the camera
    size_t index() const { return _index; }
    
    /// Request a native mode most closely matching the passed variables.
    //
    /// @param width            The required width
    /// @param height           The required height
    /// @param fps              The required frame rate
    /// @param favorArea        How to match the requested mode.
    void requestMode(size_t width, size_t height, double fps, bool favorArea);

    /// Set the amount of motion required before notifying the core
    void setMotionLevel(int m) { _motionLevel = m; }

    /// Return the current motionLevel setting
    int motionLevel() const { return _motionLevel; }
    
    /// Set time without motion in milliseconds before core is notified
    void setMotionTimeout(int m) { _motionTimeout = m; }

    /// Return the current motionTimeout setting.
    int motionTimeout() const { return _motionTimeout; }
    
    void mute(bool m) { _muted = m; }
    bool muted() const { return _muted; }
    
    /// Return the name of this webcam
    //
    /// @return     a string specifying the name of the webcam.
    const std::string& name() const { return _name; }

    /// Set the quality of the webcam
    void setQuality(int q) { _quality = q; }

    /// Return the current quality of the webcam
    int quality() const { return _quality; }
    
    /// \brief Function starts up the pipeline designed earlier in code
    ///      execution. This puts everything into motion.
    ///
    /// @return True if the pipeline was started correctly, false otherwise.
    bool play();
    
    /// \brief Function stops the pipeline designed earlier in code execution.
    ///
    /// @return True if the pipeline was stopped correctly, false otherwise.
    bool stop();
    

    /// Set this VideoInput's webcam to the device corresponding to an index.
    //
    ///       Now transfer the
    ///       important information from the GnashWebcam structure to the
    ///       GnashWebcamPrivate structure which is larger because it has
    ///       space to store Gstreamer pipeline, element and bin elements.
    ///       See definition of GnashWebcamPrivate for more info.
    ///
    /// @param dev_select The index of the camera the user wants to
    ///            select.
    /// @return    If the device index doesn't exist, return false
    bool setWebcam(size_t index);
    
    /// Call all functions necessary for initializing the camera.
    //
    /// For gstreamer this includes setting up bins.
    //
    /// Return false on failure of any initialization.
    /// TODO: better throw MediaException.
    bool init();
    
    /// \brief This function is important in the flow of the code. It looks
    ///       in the gnashrc file to see if you have a default camera defined
    ///       and selects that one (if it's defined), otherwise a videotestsrc
    ///       will be used for the remainder of the execution of the code.
    ///       Currently this code also calls other functions to make bins and
    ///       pipelines. It might be abstracted into separate functions later.
    /// @return The integer value respresenting the selected webcam from the
    ///   gnashrc file.
    int makeWebcamDeviceSelection();
    
    /// ==================================
    /// Functions that shouldn't be public.
    /// ==================================
    
    /// \brief Function links the videoSaveBin to the videoSaveQueue in the
    ///   main bin.
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///              created previously in a call to transferToPrivate()
    ///
    /// @return True if the link to the videoSaveQueue was successfully, false
    ///    otherwise.
    gboolean webcamMakeVideoSaveLink();

    /// \brief Function breaks link between the videoSaveBin and the videoSaveQueue
    ///   in the main bin.
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///             created previously in a call to transferToPrivate()
    /// @return True if the link was succesfully broken, false otherwise
    gboolean webcamBreakVideoSaveLink();

    /// \brief Function creates the save bin. For more information on pipeline
    ///       implementation and this function in general see the definition of
    ///       the _webcam_save_bin variable in the GnashWebcamPrivate structure
    ///       documentation.
    ///
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///             created previously in a call to transferToPrivate()
    ///
    /// @return True if everything went correctly (making elements, dropping
    ///        into bins and linking elements), false otherwis
    gboolean webcamCreateSaveBin();
    
    /// \brief Function links the video_display_bin to the video_display_queue
    ///       in the main bin.
    ///
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///             created previously in a call to transferToPrivate()
    ///
    /// @return True if the link to the video_display_queue was successful,
    ///        False otherwise.
    gboolean webcamMakeVideoDisplayLink();
    
    /// \brief Function breaks the link between the _videoDisplayBin and the
    ///    _videoDisplayQueue in the main bin
    ///
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///            created previously in a call to transferToPrivate()
    ///
    /// @return True if the link was successfully broken, false otherwise
    gboolean webcamBreakVideoDisplayLink();

private:

    
    /// \brief This function makes a temporary pipeline with the selected device
    ///       to determine its capabilities (Gstreamer calls these caps). This
    ///       information is saved in a GnashWebcamPrivate class and will be
    ///       used when enumerating additional information about the camera and
    ///       in creating the real pipeline to capture video.
    ///
    /// @param dev_select The integer value of the camera the user wants to select.
    ///                 This might be changed to the name of the camera, but it's
    ///                 currently an integer (if it changes, we need to change the
    ///                 gnashrc element). If this value is 0, you've selected a
    ///                 videotestsrc.
    ///
    /// @return Nothing. All pertantent information is stored in a GnashWebcam class.
    void getSelectedCaps(gint dev_select);
    
    /// \brief This function is called by get_selected caps to determine what
    ///       formats the input device can handle. It saves all the information
    ///       it finds into an array of WebcamVidFormat pointers. This information
    ///       will then be analyzed and actually added as a supported format in 
    ///       a call to addSupportedFormat.
    ///
    /// @param cam A pointer to the GnashWebcam structure you want to use as input.
    /// @param caps A pointer to the capabilities discovered in the getSelectedCaps() function.
    ///
    /// @return Nothing. All pertantent information is stored in a GnashWebcam class.
    void getSupportedFormats(GnashWebcam *cam, GstCaps *caps);
    
    /// \brief This function is called by getSupportedFormats when it has
    ///       determined that the format being analyzed can be used to capture
    ///       information from the hardware device.
    ///
    /// @param cam A pointer to the GnashWebcam structure you want to use as input.
    /// @param video_format A pointer to a WebcamVidFormat class that has had all
    ///                    variables initialize to their respective values.
    ///
    /// @return Nothing. All pertantent information is stored in a GnashWebcam class.
    void addSupportedFormat(GnashWebcam *cam, WebcamVidFormat *video_format,
        GstStructure *format_structure);
    
    /// \brief This function is called by addSupportedFormat. Since we have
    ///    found a format that will work with the input device, we now
    ///    need to figure out what framerate the camera can capture at
    ///    that corresponds with the format being analyzed.
    ///
    /// @param video_format A pointer to a WebcamVidFormat class that has
    ///    had all variables initialized to their respective values.
    /// @param structure A pointer to a structure initialized with the
    ///         capabilities of the selected input device.
    ///
    /// @return Nothing. All pertintent information is stored in a
    ///         WebcamVidFormat class.
    void getSupportedFramerates(WebcamVidFormat *video_format,
            GstStructure *structure);

    /// \brief This function checks to see if the current format selected for the
    ///     webcam supports the framerate passed in as the second argument
    /// @param webcam A pointer to the selected GnashWebcamPrivate structure to
    ///     check for the supported framerate value
    /// @param fps An integer value to check for support
    /// @return True if the framerate is supported, false otherwise
    gboolean checkForSupportedFramerate (GnashWebcamPrivate *webcam, int fps);

    /// \brief This function runs through the list of framerates determined by
    ///       getSupportedFramerates() and finds the highest supported framerate
    ///       less than 30fps.
    ///
    /// @param format A pointer to the chosen format structure.
    ///
    /// @return Nothing. All pertantent information is stored in the structure
    ///        passed in (a WebcamVidFormat class).
    void findHighestFramerate(WebcamVidFormat *format);
    
    /// \brief Function creates the source bin. For more information on pipeline
    ///      implementation and this function in general see the definition of
    ///      the _webcamSourceBin variable in the GnashWebcamPrivate structure
    ///      documentation.
    ///
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///             created previously in a call to transferToPrivate()
    ///
    /// @return True if everything went correctly (making elements, dropping
    ///        into bins and linking elements), false otherwise.
    gboolean webcamCreateSourceBin();
    
    /// \brief Function is called when changes have been made to certain variables
    ///      that effect the video source's capabilities (specifically resolution
    ///      and fps values)
    /// @return True if the changes to the source's capabilities happened succesfully
    ///      false otherwise.
    gboolean webcamChangeSourceBin();
    
    /// \brief Function creates the main bin. For more information on pipeline
    ///       implementation and this function in general see the definition of
    ///       the _webcamMainBin variable in the GnashWebcamPrivate structure
    ///       documentation.
    ///
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///             created previously in a call to transferToPrivate()
    ///
    /// @return True if everything went correctly (making elements, dropping
    ///        into bins and linking elements), false otherwise.
    gboolean webcamCreateMainBin();
    
    /// \brief Function creates the display bin. For more information on pipeline
    ///       implementation and this function in general see the definition of
    ///       the _videoDisplayBin variable in the GnashWebcamPrivate structure
    ///       documentation.
    ///
    /// @param webcam A pointer to the GnashWebcamPrivate webcam structure
    ///             created previously in a call to transferToPrivate()
    ///
    /// @return True if everything went correctly (making elements, dropping
    ///        into bins and linking elements), false otherwise.
    gboolean webcamCreateDisplayBin();
    
    
    
    /// \brief Accessor to return a pointer to the global GnashWebcamPrivate
    ///    variable
    /// @return A pointer to the global GnashWebcamPrivate pointer
    GnashWebcamPrivate* getGlobalWebcam() {return _globalWebcam;}
    
    /// \brief This function interacts with the hardware on the machine
    ///       to enumerate information about devices connected. Currently
    ///       this function only looks for videotestsources (implemented
    ///       in Gstreamer), video4linux and video4linux2 sources.
    //
    /// @return Nothing. All pertintent information is stored to the passed
    ///                  vector. Note: elements can also be null.
    static void findVidDevs(std::vector<GnashWebcam*>& cams);
    
    /// \var VideoInputGst::_vidVect
    /// \brief A vector containing pointers to GnashWebcam classes.
    std::vector<GnashWebcam*> _vidVect;
    
    /// \var VideoInputGst::_devSelection
    /// \brief An integer value representing the original GnashWebcam data struct
    ///     _vidVect
    int _devSelection;
    
    /// \var VideoInputGst::_numdevs
    /// \brief An integer value containing the number of devices attached
    ///       to the machine.
    gint _numdevs;
    
    /// \var VideoInputGst::_globalWebcam
    /// \brief Convienient pointer to the selected device's GnashWebcamPrivate
    ///       class structure.
    GnashWebcamPrivate *_globalWebcam;

    /// TODO: see which of these need to be retrieved from the camera,
    /// which of them should be stored like this, and which should
    /// be stored in the Camera_as relay object.

    /// The currently detected activity level. This should be queried from 
    /// the camera.
    double _activityLevel;

    /// The available bandwidth. This probably shouldn't be dealt with by
    /// the camera class. But maybe it should.
    size_t _bandwidth;

    /// The current FPS of the camera. This should be queried from the
    /// camera.
    double _currentFPS;

    /// The maximum FPS allowed.
    double _fps;

    /// The height of the frame. This should probably be retrieved from
    /// the camera
    size_t _height;

    /// The width of the frame. This should probably be retrieved from
    /// the camera
    size_t _width;

    /// The index of this Webcam
    size_t _index;

    /// The motion level required to trigger a notification to the core
    int _motionLevel;

    /// The length of inactivity required to trigger a notification to the core.
    int _motionTimeout;

    /// Whether access to the camera is allowed. This depends on the rcfile
    /// setting
    bool _muted;

    /// The name of this camera.
    std::string _name;

    /// The current quality setting.
    int _quality;

};




} //gst namespace
} // gnash.media namespace 
} // gnash namespace

#endif 
