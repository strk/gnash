// VideoDecoderHaiku.h: Video decoding using Haiku media kit
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

#ifndef GNASH_VIDEODECODERHAIKU_H
#define GNASH_VIDEODECODERHAIKU_H

#include "GnashImage.h"
#include "log.h"
#include "VideoDecoder.h"
#include "dsodefs.h"
#include "MediaParser.h" // for videoCodecType enum

#include "adipe.h"



namespace gnash {
namespace media {
namespace haiku {


/// Haiku media kit based VideoDecoder
class DSOEXPORT VideoDecoderHaiku : public VideoDecoder
{
public:
    //VideoDecoderGst(videoCodecType codec_type, int width, int height,
    //                const boost::uint8_t* extradata, size_t extradatasize);
    //VideoDecoderGst(GstCaps* caps);
    VideoDecoderHaiku(const VideoInfo& info);
    ~VideoDecoderHaiku();

    void push(const EncodedVideoFrame& buffer);

    std::auto_ptr<GnashImage> pop();
  
    bool peek();

    /// Get the width of the video
    //
    /// @return The width of the video in pixels or 0 if unknown.
    int width() const;

    /// Get the height of the video
    //
    /// @return The height of the video in pixels or 0 if unknown.
    int height() const;

private:
    int _count;
};


} // gnash.media.haiku namespace
} // namespace media
} // namespace gnash
#endif // __VIDEODECODERHAIKU_H__
