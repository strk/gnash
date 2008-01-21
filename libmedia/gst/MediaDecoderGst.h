// MediaDecoderGst.h: Media decoding using Gstreamer
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

// $Id: MediaDecoderGst.h,v 1.3 2008/01/21 23:10:15 rsavoye Exp $

#ifndef __MEDIADECODERGST_H__
#define __MEDIADECODERGST_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "MediaDecoder.h"
#include "MediaParser.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"

#include "image.h"

namespace gnash {
namespace media {

///  Media decoding using Gstreamer
class MediaDecoderGst: public MediaDecoder {
public:
	MediaDecoderGst(boost::shared_ptr<tu_file> stream, MediaBuffer* buffer, boost::uint16_t swfVersion, int format);
	~MediaDecoderGst();

	/// Seeks to pos
	boost::uint32_t seek(boost::uint32_t pos);

	std::pair<boost::uint32_t, boost::uint32_t> getWidthAndHeight();

private:
	/// Sets up the parser
	bool setupParser();

	/// The decoding thread. Sets up the decoder, and decodes.
	static void decodeThread(MediaDecoderGst* decoder);

	/// Sets up the decoder and parser
	bool setupDecoding();
};

} // namespace media
} // namespace gnash
#endif // __MEDIADECODERGST_H__
