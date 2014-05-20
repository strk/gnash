// AudioInputHaiku.h: Audio input with Haiku media kit
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

#ifndef GNASH_AUDIOINPUT_HAIKU_H
#define GNASH_AUDIOINPUT_HAIKU_H

#include "dsodefs.h" //DSOEXPORT
#include "AudioInput.h"

#include <cstdint> // for C99 int types
#include <string>

namespace gnash {
namespace media {
namespace haiku {

/// A class representing a single AudioInput device.
class AudioInputHaiku : public AudioInput
{
    
public:

    DSOEXPORT AudioInputHaiku();

    virtual ~AudioInputHaiku() {}
    
    //setters and getters
    virtual void setActivityLevel(double a) {
        _activityLevel = a;
    }

    virtual double activityLevel() const {
        return _activityLevel;
    }
    
    virtual void setGain(double g) {
        _gain = g;
    }

    virtual double gain() const {
        return _gain;
    }
    
    virtual void setIndex(int i) {
        _index = i;
    }

    virtual int index() const {
        return _index; 
    }
    
    virtual bool muted() {
        return _muted;
    }
    
    virtual void setName(std::string name) {
        _name = name;
    }

    virtual const std::string& name() const { return _name; }
    
    virtual void setRate(int r);

    virtual int rate() const {
        return _rate;
    }
    
    virtual void setSilenceLevel(double s) {
        _silenceLevel = s;
    }
    
    virtual double silenceLevel() const {
        return _silenceLevel;
    }
    
    virtual void setSilenceTimeout(int s) {
        _silenceTimeout = s;
    }
    
    virtual int silenceTimeout() const {
        return _silenceTimeout;
    }
    
    virtual void setUseEchoSuppression(bool e) {
        _useEchoSuppression = e;
    }

    virtual bool useEchoSuppression() const {
        return _useEchoSuppression;
    }
    
private:

    double _activityLevel;
    double _gain;
    int _index;
    bool _muted;
    std::string _name;
    int _rate;
    double _silenceLevel;
    int _silenceTimeout;
    bool _useEchoSuppression;
};

    
} // gnash.media.haiku namespace
} // gnash.media namespace 
} // gnash namespace

#endif 
