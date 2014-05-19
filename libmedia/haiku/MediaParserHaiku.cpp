// MediaParserHaiku.cpp: Haiku media kit media parsers, for Gnash
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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
//

#include "MediaParserHaiku.h"
#include "GnashException.h"
#include "log.h"
#include "IOChannel.h"

#include "adipe.h"

//#define GNASH_ALLOW_VCODEC_ENV 1
// Set this to enable a special GNASH_DEFAULT_VCODEC environment variable, which
// is used as a default when the video codec can't be detected. This is a quick
// hack to make MJPEG HTTP videos work (which can't be detected as their MIME
// type is just "mixed/multipart"). Perhaps the codec will be configurable via
// ActionScript sometime. - Udo 

namespace gnash {
namespace media {
namespace haiku {

MediaParserHaiku::MediaParserHaiku(std::unique_ptr<IOChannel> stream)
    : MediaParser(stream)
{
    QQ(2);
}

MediaParserHaiku::~MediaParserHaiku()
{
    QQ(2);
}

bool
MediaParserHaiku::seek(boost::uint32_t&)
{
    QQ(2);
}

bool
MediaParserHaiku::parseNextChunk()
{
    QQ(2);
}

boost::uint64_t
MediaParserHaiku::getBytesLoaded() const
{
    QQ(2);
}


} // gnash.media.haiku namespace 
} // end of gnash::media namespace
} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
