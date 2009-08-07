// VideoInput.cpp: Video input base class source file.
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

#include "VideoInput.h"

namespace gnash {
namespace media {
    
    //constructor
    VideoInput::VideoInput() {
        //initialize variables
        _activityLevel = -1.0;
        _bandwidth = 16384;
        _currentFPS = 0;
        _fps = 15.0;
        _height = 120;
        _index = 0;
        _keyFrameInterval = 15;
        _loopback = false;
        _motionLevel = 50;
        _motionTimeout = 2000;  //millisecs
        _muted = true;  //security (false = allow, true = decline)
        _quality = 0;
        _width = 160;
    } 
    
} //media namespace
} //gnash namespace
