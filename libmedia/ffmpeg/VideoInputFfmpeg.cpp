// VideoInputFfmpeg.h: Video input processing using Ffmpeg
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

#include "VideoInputFfmpeg.h"

namespace gnash {
namespace media {
namespace ffmpeg {

VideoInputFfmpeg::VideoInputFfmpeg()
    :
    _activityLevel(-1.0),
    _bandwidth(16384),
    _currentFPS(0),
    _fps(15.0),
    _height(120),
    _width(160),
    _index(0),
    _motionLevel(50),
    _motionTimeout(2000),
    _muted(true),
    _quality(0)
{
}

void
VideoInputFfmpeg::requestMode(size_t width, size_t height, double fps,
        bool /*favorArea*/)
{
    // TODO: check what mode is available and set the best match.
    _width = width;
    _height = height;
    _fps = fps;
}


VideoInputFfmpeg::~VideoInputFfmpeg()
{
}

}
}
}
