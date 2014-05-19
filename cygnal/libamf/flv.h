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

#ifndef GNASH_FLV_H
#define GNASH_FLV_H

#include <vector>
#include <string>
#include <cstring>
#include <boost/cstdint.hpp>    // for boost::?int??_t
#include <boost/shared_ptr.hpp>

//#include "buffer.h"
#include "element.h"
#include "buffer.h"
#include "dsodefs.h" // DSOEXPORT

/// \namespace cygnal
///
/// This namespace is for all the AMF specific classes in libamf.
namespace cygnal
{

/// \brief The FLV header is always 9 bytes
const size_t FLV_HEADER_SIZE = 0x9;

/// \brief The maximum value that can be held in a 32 bit word.
const boost::uint32_t FLV_MAX_LENGTH = 0xffffff;

/// \class Flv
///	This class abstracts an FLV file into something usable by Gnash.
class DSOEXPORT Flv {
  public:
    /// \typedef previous_size_t
    ///		This is the size in bytes of the previous MetaTag.
    typedef boost::uint32_t previous_size_t;
    /// \enum Flv::flv_type_e
    ///		The two flv file types
    typedef enum {
        FLV_VIDEO = 0x1,
        FLV_AUDIO = 0x4
    } flv_type_e;
    /// \enum Flv::flv_tag_type_e
    ///		The three types of data blocks within an flv file.
    typedef enum {
        TAG_VIDEO = 0x8,
        TAG_AUDIO = 0x9,
        TAG_METADATA = 0x12
    } flv_tag_type_e;
    // The Audio Tag types
    
    /// \enum Flv::flv_sound_type_e.
    ///
    /// @remarks soundType (byte & 0x01) >> 0.
    typedef enum {
        AUDIO_MONO = 0x0,
        AUDIO_STEREO = 0x1
    } flv_sound_type_e;
    /// \enum Flv::flv_sound_size_e.
    ///
    /// @remarks soundSize (byte & 0x02) >> 1.
    typedef enum {
        AUDIO_8BIT = 0x0,
        AUDIO_16BIT = 0x1
    } flv_sound_size_e;
    /// \enum Flv::flv_sound_rate_e.
    ///
    /// @remarks soundRate (byte & 0x0c) >> 2.
    typedef enum {
        AUDIO_55KHZ = 0x0,
        AUDIO_11KHZ = 0x1,
        AUDIO_22KHZ = 0x2,
        AUDIO_44KHZ = 0x3
    } flv_sound_rate_e;
    /// \enum Flv::flv_sound_format_e.
    ///
    /// @remarks soundFormat (byte & 0xf0) >> 3.
    typedef enum {
        AUDIO_UNCOMPRESSED = 0x0,
        AUDIO_ADPCM = 0x1,
        AUDIO_MP3 = 0x2,
        AUDIO_NELLYMOSER_8KHZ = 0x5,
        AUDIO_NELLYMOSER = 0x6,
        // These next are only supported by Gnash
        AUDIO_VORBIS = 0x7
    } flv_sound_format_e;

    // Video Tag types
    
    /// \enum Flv::flv_video_codec_e.
    ///
    /// @remarks codecID (byte & 0x0f) >> 0.
    typedef enum {
	VIDEO_NONE = 0x0,
        VIDEO_H263 = 0x2,       // sorenson
        VIDEO_SCREEN = 0x3,
        VIDEO_VP6 = 0x4,
        VIDEO_VP6_ALPHA = 0x5,
        VIDEO_SCREEN2 = 0x6,
        VIDEO_THEORA = 0x7,
        VIDEO_DIRAC = 0x8,
        VIDEO_SPEEX = 0x9
    } flv_video_codec_e;
    
    /// \enum Flv::flv_video_frame_e.
    ///
    /// @remarks frameType (byte & 0x0f) >> 4
    typedef enum {
	NO_FRAME = 0x0,
        KEYFRAME = 0x1,
        INTERFRAME = 0x2,
        DISPOSABLE = 0x3
    } flv_video_frame_type_e;

    /// \struct Flv::flv_audio_t.
    typedef struct {
        flv_sound_type_e   type;
        flv_sound_size_e   size;
        flv_sound_rate_e   rate;
        flv_sound_format_e format;
    } flv_audio_t;
    /// \struct Flv::flv_video_t.
    typedef struct {
        flv_video_codec_e codecID;
        flv_video_frame_type_e type;
    } flv_video_t;

    /// \note Data structures used for the headers. These are binary compatible
    
    /// \struct Flv::flv_header_t.
    typedef struct {
        boost::uint8_t  sig[3];      // always "FLV"
        boost::uint8_t  version;     // version, always seems to be 1
        boost::uint8_t  type;        // Bitmask: 0x4 for audio, 0x1 for video
        boost::uint8_t  head_size[4];// size of header, always seems to be 9
    } flv_header_t;

    /// \struct Flv::flv_tag_t.
    typedef struct {
        boost::uint8_t  type;         // the type. audio, video, or meta
        boost::uint8_t  bodysize[3];  // body size (tag size - sizeof(flv_tag_t))
        boost::uint8_t  timestamp[3]; // timestamp in milliseconds
        boost::uint8_t  extended;     // extended timestamp
        boost::uint8_t  streamid[3];  // always 0
    } flv_tag_t;
    
    Flv();
    ~Flv();

    /// \brief Encode the file header into a Buffer
    ///
    /// @param type The data type for the header
    ///
    /// @return a smart pointer to a Buffer containing the data in big endian format.
    std::shared_ptr<cygnal::Buffer> encodeHeader(boost::uint8_t type);
    
    /// \brief Decode a Buffer into a header
    ///
    /// @param buf a smart pointer to a Buffer containing the data.
    ///
    /// @return a smart pointer to data structure that contains the data.
    std::shared_ptr<flv_header_t> decodeHeader(std::shared_ptr<cygnal::Buffer> buf) { return decodeHeader(buf->reference()); };
    std::shared_ptr<flv_header_t> decodeHeader(boost::uint8_t *data);

    /// \brief Decode a MetaData object.
    ///		This is after the header, but before all the other tags usually
    ///
    /// @param buf a smart pointer to a Buffer containing the data.
    ///
    /// @return a smart pointer to an Element that contains the data.
    std::shared_ptr<cygnal::Element> decodeMetaData(std::shared_ptr<cygnal::Buffer> buf);

    /// \brief Decode a MetaData object.
    ///		This is after the header, but before all the other tags usually
    ///
    /// @param data The data to serialize into big endian format
    /// 
    /// @param size The size of the data in bytes
    ///
    /// @return a smart pointer to an Element that contains the data.
    std::shared_ptr<cygnal::Element> decodeMetaData(boost::uint8_t *data, size_t size);

    /// \brief Decode an Audio object.
    ///
    /// @param flags The data to deserialize.
    /// 
    /// @return a smart pointer to an audio data structure that contains the data.
    std::shared_ptr<flv_audio_t> decodeAudioData(boost::uint8_t flags);

    /// \brief Decode an Video object.
    ///
    /// @param flags The data to deserialize.
    /// 
    /// @return a smart pointer to an video data structure that contains the data.
    std::shared_ptr<flv_video_t> decodeVideoData(boost::uint8_t flags);
    
    /// \brief Decode an MetaData object.
    ///
    /// @param flags The data to deserialize.
    /// 
    /// @return a smart pointer to an video data structure that contains the data.
    std::shared_ptr<flv_tag_t> decodeTagHeader(std::shared_ptr<cygnal::Buffer> &buf) { return decodeTagHeader(buf->reference()); };
    std::shared_ptr<flv_tag_t> decodeTagHeader(boost::uint8_t *data);

    /// \brief Find the named property for this Object.
    ///
    /// @param name An ASCII string that is the name of the property to
    ///		search for.
    ///
    /// @return A smart pointer to the Element for this property.    
    std::shared_ptr<cygnal::Element> findProperty(const std::string &name);

    /// \brief Set all the properties from an array of Element classes.
    ///
    /// @param array
    ///
    /// @return nothing
    void setProperties(std::vector<std::shared_ptr<cygnal::Element> > array)
			{ _properties = array; };

    /// \brief Convert a 24 bit integer to a 32 bit one so we can use it.
    ///
    /// @return An unsigned 32 bit integer
    boost::uint32_t convert24(boost::uint8_t *);
    
    /// \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump();
    
  private:
    /// \var Flv::_header
    ///		A stored copy of the Flv file header.
    flv_header_t                _header;
    
//    boost::uint32_t             _previous_tag_size;
    /// \var Flv::_tag
    ///		A stored copy of the main onMetaTag data. Althought
    ///		there may be more than one MetaTag in an FLV, I've
    ///		actually never seen more than one. This is safe, as
    ///		this primarily for convienince.
    flv_tag_t                   _tag;

    /// \var Flv::_properties
    ///		The array of properties for this Flv file, which is
    ///		populated by the data from the first onMetaTag block.
    std::vector<std::shared_ptr<cygnal::Element> > _properties;

    /// \var _metadata
    ///         The data contained in the first onMetaData tag from
    ///         the FLV file.
    std::shared_ptr<cygnal::Element> _metadata;
    
}; // end of class definition


} // end of amf namespace

// end of _FLV_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
