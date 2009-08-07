// AudioInput.cpp: Audio input base class source file.
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

#include "AudioInput.h"
#include "gnashconfig.h"

namespace gnash {
namespace media {
    
    //constructor
    AudioInput::AudioInput()
        :
        //actionscript default values
        _activityLevel(-1),
//Gstreamer uses different values for the gain parameter, thus this doesn't
//exactly match the AS livedocs, but when you get the value back it will be
//correct (see libcore/asobj/flash/Microphone_as.cpp:gain)
#ifdef USE_GST
        _gain(0),
#else
        _gain(50),
#endif
        _index(0),
        _muted(true),
//Again, gstreamer wants a different value for the _rate parameter (in hz) whereas
//AS wants the value in khz. Thus, the ifdefs
#ifdef USE_GST
        _rate(8000),
#else
        _rate(8),
#endif
        _silenceLevel(10),
        _silenceTimeout(2000), // in milliseconds
        _useEchoSuppression(false)
    {
    } 
    
} //media namespace
} //gnash namespace
