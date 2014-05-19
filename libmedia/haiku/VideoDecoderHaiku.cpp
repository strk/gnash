// VideoDecoderHaiku.cpp: Video decoding using Haiku media kit.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "VideoDecoderHaiku.h"
#include "MediaParserHaiku.h"

namespace gnash {
namespace media {
namespace haiku {

VideoDecoderHaiku::VideoDecoderHaiku(const VideoInfo& info)
    : _count(0)
{
    QQ(2);
}

int
VideoDecoderHaiku::width() const
{
    QQ(2);
    return 100;
    //return _width;
}

int
VideoDecoderHaiku::height() const
{
    QQ(2);
    return 100;
    //return _height;
}

VideoDecoderHaiku::~VideoDecoderHaiku()
{
    QQ(2);
}

void
VideoDecoderHaiku::push(const EncodedVideoFrame& frame)
{
    ++ _count;
    QQ(2);
}
  

std::unique_ptr<GnashImage>
VideoDecoderHaiku::pop()
{
    std::unique_ptr<GnashImage> ret;
    ret.reset(new ImageRGB(100, 100));
    boost::uint8_t *d =
        new boost::uint8_t[100*100*4];
    for (int i = 0; i < 100*100*4; ++i)
    {
        d[i] *= 1 - (boost::uint8_t) 2*(rand()%2);
        d[i] += (boost::uint8_t) rand()/64;
    }
    ret->update(d);
    delete [] d;
    -- _count;
    return ret;
}
  

bool
VideoDecoderHaiku::peek()
{
    QQ(2);
    return _count > 0;
}


} // namespace gnash::media::haiku
} // namespace gnash::media
} // namespace gnash
