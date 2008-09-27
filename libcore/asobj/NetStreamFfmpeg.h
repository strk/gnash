// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifndef GNASH_NETSTREAMFFMPEG_H
#define GNASH_NETSTREAMFFMPEG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef USE_FFMPEG

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "impl.h" // what for ? drop ?
#include "VideoDecoder.h" // for visibility of dtor
#include "AudioDecoder.h" // for visibility of dtor

#include "image.h"
#include "StreamProvider.h"	
#include "NetStream.h" // for inheritance
#include "VirtualClock.h"

// TODO: drop ffmpeg-specific stuff
#include "ffmpegNetStreamUtil.h"


#include <queue>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/barrier.hpp>

#include <memory>
#include <cassert>

// Forward declarations
namespace gnash {
	class IOChannel;
	namespace media {
		class sound_handler;
		class MediaHandler;
	}
}

namespace gnash {
  

class NetStreamFfmpeg: public NetStream {
public:
private:

};


} // gnash namespace


#endif // USE_FFMPEG

#endif //  __NETSTREAMFFMPEG_H__
