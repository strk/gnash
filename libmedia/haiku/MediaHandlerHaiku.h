// MediaHandlerHaiku.h: Haiku media kit media handler, for Gnash
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

#ifndef GNASH_MEDIAHANDLERHAIKU_H
#define GNASH_MEDIAHANDLERHAIKU_H

#include "MediaHandler.h" // for inheritance

#include <vector>
#include <memory>

namespace gnash {
namespace media {

/// Haiku media kit based media handler module
//
/// The module implements the MediaHandler factory as required
/// by Gnash core for a loadable media handler module.
///
/// It uses the Haiku media kit
///
/// Starting point is MediaHandlerHaiku.
/// 
namespace haiku {

/// Haiku based MediaHandler
class MediaHandlerHaiku : public MediaHandler
{
public:

    virtual std::string description() const {
        return "Haiku Media Handler";
    }

	virtual std::unique_ptr<MediaParser>
        createMediaParser(std::shared_ptr<IOChannel> stream);

	virtual std::unique_ptr<VideoDecoder>
        createVideoDecoder(const VideoInfo& info);
	
	virtual std::unique_ptr<VideoConverter>
		createVideoConverter(ImgBuf::Type4CC srcFormat,
                ImgBuf::Type4CC dstFormat);

	virtual std::unique_ptr<AudioDecoder>
        createAudioDecoder(const AudioInfo& info);

//    virtual size_t getInputPaddingSize() const;
    
    virtual VideoInput* getVideoInput(size_t index);
    
    virtual AudioInput* getAudioInput(size_t index);

    virtual void cameraNames(std::vector<std::string>& names) const;

};


} // gnash.media.haiku namespace 
} // gnash.media namespace 
} // namespace gnash

#endif 
