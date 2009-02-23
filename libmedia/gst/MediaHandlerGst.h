// MediaHandlerGst.h: GST media handler, for Gnash
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#ifndef GNASH_MEDIAHANDLERGST_H
#define GNASH_MEDIAHANDLERGST_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "dsodefs.h" // DSOEXPORT
#include "MediaHandler.h" // for inheritance

#include <memory>

namespace gnash {
namespace media {

/// Gstreamer-based media handler module
namespace gst {

/// GST based MediaHandler
//
/// The module implements the MediaHandler factory as required
/// by Gnash core for a loadable media handler module.
///
/// It uses gstreamer: http://gstreamer.freedesktop.org/
///
/// Starting point is MediaHandlerGst.
/// 
class DSOEXPORT MediaHandlerGst : public MediaHandler
{
public:

	virtual std::auto_ptr<MediaParser> createMediaParser(std::auto_ptr<IOChannel> stream);

	virtual std::auto_ptr<VideoDecoder> createVideoDecoder(const VideoInfo& info);

	virtual std::auto_ptr<AudioDecoder> createAudioDecoder(const AudioInfo& info);
	
	virtual std::auto_ptr<VideoConverter> createVideoConverter(ImgBuf::Type4CC srcFormat,
	                                                       ImgBuf::Type4CC dstFormat);
};


} // gnash.media.gst namespace
} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAHANDLERGST_H__
