// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

//#include "buffer.h"
#include "element.h"
#include "dsodefs.h" // DSOEXPORT

namespace amf
{

// The FLV header is always 9 bytes
const size_t FLV_HEADER_SIZE = 0x9;
const boost::uint32_t FLV_MAX_LENGTH = 0xffffff;

class DSOEXPORT Flv {
  public:
    // There is a previous tag size
    typedef boost::uint32_t previous_size_t;
    typedef enum {
        FLV_VIDEO = 0x1,
        FLV_AUDIO = 0x4
    } flv_type_e;
    typedef enum {
        TAG_VIDEO = 0x8,
        TAG_AUDIO = 0x9,
        TAG_METADATA = 0x12
    } flv_tag_type_e;
    // Audio Tag types
    // soundType (byte & 0x01) >> 0
    typedef enum {
        AUDIO_MONO = 0x0,
        AUDIO_STEREO = 0x1
    } flv_sound_type_e;
    // soundSize (byte & 0x02) >> 1
    typedef enum {
        AUDIO_8BIT = 0x0,
        AUDIO_16BIT = 0x1
    } flv_sound_size_e;
    // soundRate (byte & 0x0c) >> 2
    typedef enum {
        AUDIO_55KHZ = 0x0,
        AUDIO_11KHZ = 0x1,
        AUDIO_22KHZ = 0x2,
        AUDIO_44KHZ = 0x3,
    } flv_sound_rate_e;
    // soundFormat (byte & 0xf0) >> 3
    typedef enum {
        AUDIO_UNCOMPRESSED = 0x0,
        AUDIO_ADPCM = 0x1,
        AUDIO_MP3 = 0x2,
        AUDIO_NELLYMOSER_8KHZ = 0x5,
        AUDIO_NELLYMOSER = 0x6,
        // These next are only supported by Gnash
        AUDIO_VORBIS = 0x7,
    } flv_sound_format_e;
    // Video Tag types
    // codecID (byte & 0x0f) >> 0
    typedef enum {
	VIDEO_NONE = 0x0,
        VIDEO_H263 = 0x2,       // sorenson
        VIDEO_SCREEN = 0x3,
        VIDEO_VP6 = 0x4,
        VIDEO_VP6_ALPHA = 0x5,
        VIDEO_SCREEN2 = 0x6,
        VIDEO_THEORA = 0x7,
        VIDEO_DIRAC = 0x8,
        VIDEO_SPEEX = 0x9,
    } flv_video_codec_e;
    // frameType (byte & 0x0f) >> 4
    typedef enum {
	NO_FRAME = 0x0,
        KEYFRAME = 0x1,
        INTERFRAME = 0x2,
        DISPOSABLE = 0x3
    } flv_video_frame_type_e;
    typedef struct {
        flv_sound_type_e   type;
        flv_sound_size_e   size;
        flv_sound_rate_e   rate;
        flv_sound_format_e format;
    } flv_audio_t;
    typedef struct {
        flv_video_codec_e codecID;
        flv_video_frame_type_e type;
    } flv_video_t;
    // Data structures used for the headers. These are binary compatible
    typedef struct {
        boost::uint8_t  sig[3];      // always "FLV"
        boost::uint8_t  version;     // version, always seems to be 1
        boost::uint8_t  type;        // Bitmask: 0x4 for audio, 0x1 for video
        boost::uint8_t  head_size[4];// size of header, always seems to be 9
    } flv_header_t;
    typedef struct {
        boost::uint8_t  type;         // the type. audio, video, or meta
        boost::uint8_t  bodysize[3];  // body size (tag size - sizeof(flv_tag_t))
        boost::uint8_t  timestamp[3]; // timestamp in milliseconds
        boost::uint8_t  extended;     // extended timestamp
        boost::uint8_t  streamid[3];  // always 0
    } flv_tag_t;
    Flv();
    ~Flv();

    // Encode the data into a Buffer
    amf::Buffer *encodeHeader(gnash::Network::byte_t type);
    // Decode a Buffer into a header
    flv_header_t *decodeHeader(amf::Buffer *buf);

    // Decode a MetaData object, which is after the header, but before all the tags
    amf::Element *decodeMetaData(amf::Buffer *buf);
    amf::Element *decodeMetaData(gnash::Network::byte_t *buf, size_t size);
    flv_audio_t *decodeAudioData(gnash::Network::byte_t flags);
    flv_video_t *decodeVideoData(gnash::Network::byte_t flags);
    
    // Decode the tag header
    flv_tag_t *decodeTagHeader(amf::Buffer *buf);
    
    amf::Element *findProperty(const std::string &name);
    void setProperties(std::vector<amf::Element *> x) { _properties = x; };

    // Convert a 24 bit integer to a 32 bit one so we can use it.
    boost::uint32_t convert24(boost::uint8_t *);
    
    void dump();
  private:
    flv_header_t                _header;
//    boost::uint32_t             _previous_tag_size;
    flv_tag_t                   _tag;
    std::vector<amf::Element *> _properties;
}; // end of class definition


} // end of amf namespace

// end of _FLV_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
