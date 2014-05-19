//
//   Copyright (C) 2008, 2009, 2010, 2011, 2012
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

#include "VideoConverterHaiku.h"
#include "MediaParser.h"
#include "log.h"
#include <cassert>

#include "adipe.h"


namespace gnash {
namespace media {
namespace haiku {

/// Base class for video image space conversion.

VideoConverterHaiku::VideoConverterHaiku(ImgBuf::Type4CC srcFormat, ImgBuf::Type4CC dstFormat)
    : VideoConverter(srcFormat, dstFormat)
{
    QQ(2);
}

VideoConverterHaiku::~VideoConverterHaiku()
{
    QQ(2);
}

std::unique_ptr<ImgBuf>
VideoConverterHaiku::convert(const ImgBuf& src)
{
    QQ(2);
}

} // gnash.media.haiku namespace
} // gnash.media namespace 
} // gnash namespace

