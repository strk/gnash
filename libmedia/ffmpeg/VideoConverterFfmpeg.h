//
//     Copyright (C) 2008, 2009 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA    02110-1301    USA


#ifndef GNASH_VIDEOCONVERTERFFMPEG_H
#define GNASH_VIDEOCONVERTERFFMPEG_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ffmpegHeaders.h"
#include "VideoConverter.h"

#include "log.h"

#if HAVE_SWSCALE_H
extern "C" {
#include <libswscale/swscale.h>
}
#endif


namespace gnash {
namespace media {
namespace ffmpeg {

class SwsContextWrapper;

#ifdef HAVE_SWSCALE_H
/// A wrapper round an SwsContext that ensures it's
/// freed on destruction.
class SwsContextWrapper
{
public:

    SwsContextWrapper(SwsContext* context)
        :
        _context(context)
    {}

    ~SwsContextWrapper()
    {
         sws_freeContext(_context);
    }
    
    SwsContext* getContext() const { return _context; }

private:
    SwsContext* _context;

};
#endif

class VideoConverterFfmpeg : public VideoConverter {

public:
    VideoConverterFfmpeg(ImgBuf::Type4CC srcFormat, ImgBuf::Type4CC dstFormat);
    
    ~VideoConverterFfmpeg();

    std::auto_ptr<ImgBuf> convert(const ImgBuf& src);
  
private:

#if HAVE_SWSCALE_H
    std::auto_ptr<SwsContextWrapper> _swsContext;
#endif
};

}
}
}

#endif // GNASH_VIDEOCONVERTERFFMPEG_H
