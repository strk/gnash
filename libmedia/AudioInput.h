// AudioInput.h: Audio input base class
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

#ifndef GNASH_AUDIOINPUT_H
#define GNASH_AUDIOINPUT_H

#include <cstdint> // for C99 int types
#include <string>
#include "dsodefs.h" //DSOEXPORT

namespace gnash {
namespace media {

/// A class representing a single AudioInput device.
//
/// This interface has almost everything needed for control of the input
/// device, but currently no data-fetching functions. These should be
/// implemented only when the requirements of AS have been investigated!
class AudioInput
{
    
public:

    DSOEXPORT AudioInput() {}

    virtual ~AudioInput() {}
    
    virtual void setActivityLevel(double a) = 0;

    virtual double activityLevel() const = 0;
    
    virtual void setGain(double g) = 0;

    virtual double gain() const = 0;
    
    virtual void setIndex(int i) = 0;

    virtual int index() const = 0;
    
    virtual bool muted() = 0;
    
    virtual void setName(std::string name) = 0;

    virtual const std::string& name() const = 0;
    
    virtual void setRate(int r) = 0;

    virtual int rate() const = 0;
    
    virtual void setSilenceLevel(double s) = 0;
    
    virtual double silenceLevel() const = 0;
    
    virtual void setSilenceTimeout(int s) = 0;
    
    virtual int silenceTimeout() const = 0;
    
    virtual void setUseEchoSuppression(bool e) = 0;

    virtual bool useEchoSuppression() const = 0;
    
};

    
} // gnash.media namespace 
} // gnash namespace

#endif 
