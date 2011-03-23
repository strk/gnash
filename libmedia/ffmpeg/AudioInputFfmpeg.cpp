// AudioInputFfmpeg.cpp: Audio input base class source file.
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

#include "AudioInputFfmpeg.h"
#include "gnashconfig.h"

namespace gnash {
namespace media {
namespace ffmpeg {
 
AudioInputFfmpeg::AudioInputFfmpeg()
    :
    _activityLevel(-1),
    _gain(50),
    _index(0),
    _muted(true),
    _rate(8),
    _silenceLevel(10),
    _silenceTimeout(2000), 
    _useEchoSuppression(false)
{
} 
    
void
AudioInputFfmpeg::setRate(int r)
{
    // Yes, this isn't pretty, but it is only designed for the 
    // testsuite to continue passing.
    if (r >= 44) {
        _rate = 44;
        return;
    }
    static const int rates[] = { 5, 8, 11, 16, 22, 44 };
    const int* rate = rates;
    while (*rate < r) ++rate;
    _rate = *rate;
}

} // gnash.media.ffmpeg namespace
} // gnash.media namespace
} // gnash namespace
