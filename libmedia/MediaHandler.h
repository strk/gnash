// MediaHandler.h: Base class for media handlers
// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

#include "MediaParser.h" // for videoCodecType and audioCodecType enums
#include "dsodefs.h" // DSOEXPORT
#include "VideoConverter.h"

#include <vector>
#include <memory>
#include <map>
#include <string>
#include <boost/scoped_ptr.hpp>

// Forward declarations
namespace gnash {
    class IOChannel;
    namespace media {
        class VideoDecoder;
        class AudioDecoder;
        class AudioInfo;
        class VideoInfo;
        class VideoInput;
        class AudioInput;
        class MediaHandler;
    }
}

namespace gnash {

/// Gnash %media handling subsystem (libmedia)
//
/// The core Gnash lib will delegate any parsing decoding and encoding
/// of %media files to the %media subsystem.
///
/// The subsystem's entry point is a MediaHandler instance, which acts
/// as a factory for parsers, decoders and encoders.
///
/// @todo fix http://wiki.gnashdev.org/wiki/index.php/Libmedia, is obsoleted
namespace media {

class DSOEXPORT MediaFactory
{
public:
    typedef MediaHandler*(*CreateHandler)();
    typedef std::map<std::string, CreateHandler> Handlers;

    /// Get the MediaFactory singleton.
    static MediaFactory& get() {
        if (!_factory.get()) _factory.reset(new MediaFactory());
        return *_factory;
    }

    /// Return a MediaHandler identified by a name.
    //
    /// @param name     The name of the handler to return. An empty string
    ///                 will return the first available handler. If the
    ///                 string is not empty and no match is found, a null
    ///                 pointer will be returned.
    MediaHandler* get(const std::string& name);

    /// Register a MediaHandler with a particular name.
    //
    /// @param name     The name to register the MediaHandler under. Duplicated
    ///                 names will replace previous handlers!
    /// @param r        A pointer to a function that will return the 
    ///                 MediaHandler when called.
    void registerHandler(const std::string& name, CreateHandler r);

private:
    static boost::scoped_ptr<MediaFactory> _factory;
    Handlers _handlers;
};

/// The MediaHandler class acts as a factory to provide parser and decoders
class DSOEXPORT MediaHandler
{
public:

    virtual ~MediaHandler() {}

    /// Return an appropriate MediaParser for given input
    //
    /// @param stream
    ///    Input stream, ownership transferred
    ///
    /// @return 0 if no parser could be created for the input
    ///
    /// NOTE: the default implementation returns an FLVParser for FLV input
    ///       or 0 for others.
    ///
    virtual std::auto_ptr<MediaParser>
        createMediaParser(std::auto_ptr<IOChannel> stream);

    /// Create a VideoDecoder for decoding what's specified in the VideoInfo
    //
    /// @param info VideoInfo class with all the info needed to decode
    ///             the sound correctly.
    /// @return     Will always return a valid VideoDecoder or throw a
    ///             gnash::MediaException if a fatal error occurs.
    virtual std::auto_ptr<VideoDecoder>
        createVideoDecoder(const VideoInfo& info)=0;

    /// Create an AudioDecoder for decoding what's specified in the AudioInfo
    //
    /// @param info AudioInfo class with all the info needed to decode
    ///             the sound correctly.
    /// @return     Will always return a valid AudioDecoder or throw a
    ///             gnash::MediaException if a fatal error occurs.
    virtual std::auto_ptr<AudioDecoder>
        createAudioDecoder(const AudioInfo& info)=0;

    /// Create an VideoConverter for converting between color spaces.
    //
    /// @param srcFormat The source image color space
    /// @param dstFormat The destination image color space
    ///
    /// @return A valid VideoConverter or a NULL auto_ptr if a fatal error
    ///         occurs.
    virtual std::auto_ptr<VideoConverter>
        createVideoConverter(ImgBuf::Type4CC srcFormat,
                ImgBuf::Type4CC dstFormat)=0;

    /// Return a VideoInput
    //
    /// This is always owned by the MediaHandler, but will remain alive
    /// as long as it is referenced by a Camera object.
    //
    /// @param index    The index of the VideoInput to return.
    /// @return         A Video Input corresponding to the specified index
    ///                 or null if it is not available. 
    virtual VideoInput* getVideoInput(size_t index) = 0;

    virtual AudioInput* getAudioInput(size_t index) = 0;

    /// Return a list of available cameras.
    //
    /// This is re-generated every time the function is called.
    virtual void cameraNames(std::vector<std::string>& names) const = 0;

    /// Return the number of bytes padding needed for input buffers
    //
    /// Bitstream readers are optimized to read several bytes at a time,
    /// and this should be used to allocate a large enough input buffer.
    virtual size_t getInputPaddingSize() const { return 0; }

protected:

    /// Create an AudioDecoder for FLASH codecs 
    //
    /// This method is attempted as a fallback in case
    /// a mediahandler-specific audio decoder couldn't be created
    /// for a FLASH codec.
    /// 
    /// @throws a MediaException if it can't create a decoder
    ///
    /// @param info
    ///     Informations about the audio. It is *required*
    ///     for info.type to be media::FLASH (caller should check
    ///     that before calling this).
    ///
    std::auto_ptr<AudioDecoder> createFlashAudioDecoder(const AudioInfo& info);

    /// Return true if input stream is an FLV
    //
    /// If this cannot read the necessary 3 bytes, it throws an IOException.
    bool isFLV(IOChannel& stream) throw (IOException);

protected:

    MediaHandler() {}

};


} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIAHANDLER_H__
