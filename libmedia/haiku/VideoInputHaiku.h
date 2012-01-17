// VideoInputHaiku.h: Video input processing using Haiku media kit
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifndef GNASH_VIDEOINPUTHAIKU_H
#define GNASH_VIDEOINPUTHAIKU_H

#include <vector>
#include <boost/cstdint.hpp> // for C99 int types
#include "VideoInput.h"

namespace gnash {
namespace media {
namespace haiku {

class VideoInputHaiku : public VideoInput
{
public:

    /// Constructor for the VideoInputHaiku class
    //
    /// TODO: most of these properties need not be stored, but should rather
    /// be queried from the input device.
    VideoInputHaiku();
    
    /// Destructor for the VideoInputGst class
    virtual ~VideoInputHaiku();
    
    static void getNames(std::vector<std::string>& /*names*/) {}
    
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
    
private:

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


} // gnash.media.haiku namespace
} // gnash.media namespace 
} // gnash namespace

#endif 
