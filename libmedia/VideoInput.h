// VideoInput.h: Video input base class.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_VIDEOINPUT_H
#define GNASH_VIDEOINPUT_H

#include <boost/cstdint.hpp> // for C99 int types
#include <string>

#include "dsodefs.h" //DSOEXPORT

namespace gnash {
namespace media {

/// This is the interface for video input devices.
//
/// Each VideoInput should represent exactly one webcam (or similar device).
//
/// The interface for querying the camera is provisionally done, but needs
/// more testing of how it actually works. Most of the values are faked. 
//
/// TODO: separate the process of finding cameras from this class.
///       It could be implemented as a static method. The available cameras
///       and all created VideoInput objects should be stored in a
///       MediaHandler, mapped by an index for retrieval by ActionScript.
//
/// TODO: design a useful interface for starting, stopping and attaching
///       the video data. VideoInputGst has some functionality here, but it
///       is not generic enough, relying on too many gst-specific
///       implementation details.
class VideoInput {

public:

    DSOEXPORT VideoInput() {}

    // virtual classes need a virtual destructor !
    virtual ~VideoInput() {}
       
    /// Return the current activity level of the webcam
    //
    /// @return     A double specifying the amount of motion currently
    ///             detected by the camera.
    virtual double activityLevel() const = 0;
    
    /// The maximum available bandwidth for outgoing connections
    //
    /// TODO: see if this should really be here.
    virtual size_t bandwidth() const = 0;
    
    /// Set the bandwidth for outgoing connections.
    virtual void setBandwidth(size_t bandwidth) = 0;

    /// The current frame rate of the webcam
    //
    /// @return     A double specifying the webcam's current FPS
    virtual double currentFPS() const = 0;
    
    /// The maximum FPS rate of the webcam
    //
    /// @return     A double specifying the webcam's maximum FPS
    virtual double fps() const = 0;

    /// Return the height of the webcam's frame
    virtual size_t height() const = 0;
    
    /// Return the width of the webcam's frame
    virtual size_t width() const = 0;
    
    /// The index of the camera
    virtual size_t index() const = 0;
    
    /// Request a native mode most closely matching the passed variables.
    //
    /// @param width            The required width
    /// @param height           The required height
    /// @param fps              The required frame rate
    /// @param favorArea        How to match the requested mode.
    virtual void requestMode(size_t width, size_t height, double fps,
            bool favorArea) = 0;

    /// Set the amount of motion required before notifying the core
    virtual void setMotionLevel(int m) = 0;

    /// Return the current motionLevel setting
    virtual int motionLevel() const = 0;
    
    /// Set time without motion in milliseconds before core is notified
    virtual void setMotionTimeout(int m) = 0;

    /// Return the current motionTimeout setting.
    virtual int motionTimeout() const = 0;
    
    virtual void mute(bool m) = 0;
    virtual bool muted() const = 0;
    
    /// Return the name of this webcam
    //
    /// @return     a string specifying the name of the webcam.
    virtual const std::string& name() const = 0;

    /// Set the quality of the webcam
    virtual void setQuality(int q) = 0;

    /// Return the current quality of the webcam
    virtual int quality() const = 0;

};

    
} // media namespace 
} // gnash namespace

#endif 
