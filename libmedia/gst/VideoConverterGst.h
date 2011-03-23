//   Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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


#ifndef GNASH_VIDEOCONVERTERGST_H
#define GNASH_VIDEOCONVERTERGST_H

#include "VideoConverter.h"
#include "swfdec_codec_gst.h"

#include <gst/gst.h>

namespace gnash {
namespace media {
namespace gst {

/// Base class for video image space conversion with gst

class VideoConverterGst : public VideoConverter {

public:
    VideoConverterGst(ImgBuf::Type4CC srcFormat, ImgBuf::Type4CC dstFormat);
    
    ~VideoConverterGst();
  
    std::auto_ptr<ImgBuf> convert(const ImgBuf& src);
    
    bool init(const ImgBuf& src);
  
private:
    SwfdecGstDecoder _decoder;
};

} // gnash.media.gst namespace
} // gnash.media namespace 
} // gnash namespace

#endif // __VIDEOCONVERTERGST_H__
