// MediaHandler.h: Base class for media handlers
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


#ifndef GNASH_MEDIAHANDLER_H
#define GNASH_MEDIAHANDLER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "MediaParser.h" // for videoCodecType and audioCodecType enums
#include "dsodefs.h" // DSOEXPORT

#include <memory>

// Forward declarations
namespace gnash {
	class IOChannel;
	namespace media {
		class VideoDecoder;
		class AudioDecoder;
		class AudioInfo;
		class VideoInfo;
	}
}

namespace gnash {
namespace media {

/// The MediaHandler class acts as a factory to provide parser and decoders
class DSOEXPORT MediaHandler
{
public:

	virtual ~MediaHandler() {}

	/// Return currently registered MediaHandler, possibly null.
	static MediaHandler* get()
	{
		return _handler.get();
	}

	/// Register a MediaHandler to use
	static void set(std::auto_ptr<MediaHandler> mh)
	{
		_handler = mh;
	}

	/// Return an appropriate MediaParser for given input
	//
	/// @param stream
	///	Input stream, ownership transferred
	///
	/// @return 0 if no parser could be created for the input
	///
	/// NOTE: the default implementation returns an FLVParser for FLV input
	///       or 0 for others.
	///
	virtual std::auto_ptr<MediaParser> createMediaParser(std::auto_ptr<IOChannel> stream);

	/// Create a VideoDecoder for decoding what's specified in the VideoInfo
	//
	/// @param info VideoInfo class with all the info needed to decode
	///             the sound correctly.
    /// @return     Will always return a valid VideoDecoder or throw a
    ///             gnash::MediaException if a fatal error occurs.
	virtual std::auto_ptr<VideoDecoder> createVideoDecoder(VideoInfo& info)=0;

	/// Create an AudioDecoder for decoding what's specified in the AudioInfo
	//
	/// @param info AudioInfo class with all the info needed to decode
	///             the sound correctly.
    /// @return     Will always return a valid AudioDecoder or throw a
    ///             gnash::MediaException if a fatal error occurs.
	virtual std::auto_ptr<AudioDecoder> createAudioDecoder(AudioInfo& info)=0;

protected:

	/// Return true if input stream is an FLV
	bool isFLV(IOChannel& stream);

	MediaHandler() {}

private:

	static std::auto_ptr<MediaHandler> _handler;
};


} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAHANDLER_H__
