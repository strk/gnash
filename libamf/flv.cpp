// amf.cpp:  AMF (Action Message Format) rpc marshalling, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <vector>
#include <cmath>
#include <climits>

#include "buffer.h"
#include "log.h"
#include "amf.h"
#include "amfutf8.h"
#include "utility.h"
#include "flv.h"

//#include <boost/detail/endian.hpp>

#include <boost/cstdint.hpp> // for boost::?int??_t

using namespace std;
using namespace gnash;

namespace amf 
{


Flv::Flv() 
{
//    GNASH_REPORT_FUNCTION;
    memcpy(&_header.sig, "FLV", 3);
    _header.version = 1;
    _header.type = Flv::FLV_VIDEO | Flv::FLV_AUDIO;
//    _header.head_size = 9;
    
    memset(&_tag, 0, sizeof(flv_tag_t));
    _tag.type = Flv::TAG_METADATA;
    _tag.bodysize[0] = 0x0f;
    _tag.bodysize[1] = 0x30;
    _tag.bodysize[2] = 0x00;
}

Flv::~Flv()
{
//    GNASH_REPORT_FUNCTION;
}

// Encode the data into a Buffer
Buffer *
Flv::encodeHeader(Network::byte_t type)
{
//    GNASH_REPORT_FUNCTION;
    Buffer *buf = new Buffer(sizeof(Flv::flv_header_t));
    buf->clear();
    
    Network::byte_t version = 0x1;
    buf->copy("FLV");
    buf->append(version);

    buf->append(type);

    boost::uint32_t size = htonl(0x9);
    buf->append(size);

    return buf;
}

// Decode a Buffer into a header
Flv::flv_header_t *
Flv::decodeHeader(amf::Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    memcpy(&_header, buf->begin(), sizeof(Flv::flv_header_t));

    // test the magic number
    if (memcmp(_header.sig, "FLV", 3) != 0) {
	log_error("Bad magic number for FLV file!");
	return 0;
    }

    // Make sure the version is legit, it should alwys be 1
    if (_header.version != 0x1) {
	log_error("Bad version in FLV header! %d", _header.version);
		  return 0;
    }

    // Make sure the type is set correctly
    if (((_header.type & Flv::FLV_AUDIO) && (_header.type & Flv::FLV_VIDEO))
	|| (_header.type & Flv::FLV_AUDIO) || (_header.type & Flv::FLV_VIDEO)) {
    } else {
	    log_error("Bad FLV file Type: %d", _header.type);
    }
    
    // Be lazy, as head_size is an array of 4 bytes, and not an integer in the data
    // structure. This is to get around possible padding done to the data structure
    // done by some compilers.
    boost::uint32_t size = *(reinterpret_cast<boost::uint32_t *>(_header.head_size));

    // The header size is big endian
    size = ntohl(size);
    
    // The header size is always 9, guess it could change some day in the far future, so
    // we should use it.
    if (size != 0x9) {
	log_error("Bad header size in FLV header! %d", _header.head_size);
		  return 0;
    }
    
    return &_header;
}

// Decode a MetaData object, which is after the header, but before all the tags
amf::Element *
Flv::decodeMetaData(amf::Buffer *buf)
{
    return decodeMetaData(buf->reference(), buf->size());
}

amf::Element *
Flv::decodeMetaData(gnash::Network::byte_t *buf, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    AMF amf;
    Network::byte_t *ptr = buf;
    Network::byte_t *tooFar = ptr + size;
    char *name = 0;
    
    // Extract the onMetaData object name
    // In disk files, I always see the 0x2 type field for
    // a string, but not always in streaming, at least according to
    // Gnash's libmedia/FLVParser code. So if we see the begining
    // of "onMetaData", then just grab the length without the type
    // field.
    if ((*ptr == 0) && (*ptr+3 == 'o')) {
	boost::uint16_t length;
	length = ntohs((*(boost::uint16_t *)ptr) & 0xffff);
	name = new char(length+1);
	memset(name, 0, length+1);
	std::copy(name, name + length, ptr);
	ptr += length + AMF_HEADER_SIZE;
    } else {	
	Element *objname = amf.extractAMF(ptr, tooFar);
	if (objname == 0) {
	    log_error("Failed to get the onMetaData string");
	    return 0;
	}
	ptr += objname->getLength() + AMF_HEADER_SIZE;
	name = const_cast<char *>(objname->to_string());
    }    
    
    // Extract the properties for this metadata object.
    Element *el = amf.extractAMF(ptr, tooFar);
    ptr += amf.totalsize();

    if (name) {
        el->setName(name);
    }

    return el;
}

Flv::flv_audio_t *
Flv::decodeAudioData(gnash::Network::byte_t byte)
{
//    GNASH_REPORT_FUNCTION;
    flv_audio_t *audio = new flv_audio_t;
    memset(audio, 0, sizeof(flv_audio_t));

    // Get the sound type
    if (byte && Flv::AUDIO_STEREO) {
	audio->type = Flv::AUDIO_STEREO;
    } else if (!byte && Flv::AUDIO_STEREO) {
	audio->type = Flv::AUDIO_MONO;
    } else {
	log_error("Bad FLV Audio Sound Type: %x", byte + 0);
    }

    // Get the sound size
    if ((byte >> 1) && Flv::AUDIO_16BIT) {
	audio->size = Flv::AUDIO_16BIT;	
    } else if (!(byte >> 1) && Flv::AUDIO_16BIT) {
	audio->size = Flv::AUDIO_8BIT;	
    } else {
	log_error("Bad FLV Audio Sound size: %d", byte >> 1);
    }

    // Get the sound rate

    if ((byte >> 2) && Flv::AUDIO_11KHZ) {
	audio->rate = Flv::AUDIO_11KHZ;	
    } else if ((byte >> 2) & Flv::AUDIO_22KHZ) {
	audio->rate = Flv::AUDIO_22KHZ;	
    } else if ((byte >> 2) & Flv::AUDIO_44KHZ) {
	audio->rate = Flv::AUDIO_44KHZ;
    } else if ((byte >> 2) == 0) {
	audio->rate = Flv::AUDIO_55KHZ;
    } else {
	log_error("Bad FLV Audio Sound Rate: %d", byte >> 2);
    }

    // Get the sound format
    if ((byte >> 4) && Flv::AUDIO_ADPCM) {
	audio->format = Flv::AUDIO_ADPCM;	
    } else if ((byte >> 4) && Flv::AUDIO_MP3) {
	audio->format = Flv::AUDIO_MP3;
    } else if ((byte >> 4) && Flv::AUDIO_NELLYMOSER_8KHZ) {
	audio->format = Flv::AUDIO_NELLYMOSER_8KHZ;
    } else if ((byte >> 4) && Flv::AUDIO_NELLYMOSER) {
	audio->format = Flv::AUDIO_NELLYMOSER;
    } else if ((byte >> 4) && Flv::AUDIO_VORBIS) {
	audio->format = Flv::AUDIO_VORBIS;
    } else if (!(byte >> 4) && Flv::AUDIO_ADPCM) {
	audio->format = Flv::AUDIO_UNCOMPRESSED;
    } else {
	log_error("Bad FLV Audio Sound format: %d", byte >> 4);
    }
    
    return audio;
}

Flv::flv_video_t *
Flv::decodeVideoData(gnash::Network::byte_t byte)
{
//    GNASH_REPORT_FUNCTION;
    flv_video_t *video = new flv_video_t;
    memset(video, 0, sizeof(flv_video_t));

    // Get the codecID codecID
    if (byte && Flv::VIDEO_H263) {
	video->codecID = Flv::VIDEO_H263;
    } else if (byte && Flv::VIDEO_SCREEN) {
	video->codecID = Flv::VIDEO_SCREEN;
    } else if (byte && Flv::VIDEO_VP6) {
	video->codecID = Flv::VIDEO_VP6;
    } else if (byte && Flv::VIDEO_VP6_ALPHA) {
	video->codecID = Flv::VIDEO_VP6_ALPHA;
    } else if (byte && Flv::VIDEO_SCREEN2) {
	video->codecID = Flv::VIDEO_SCREEN2;
    } else if (byte && Flv::VIDEO_THEORA) {
	video->codecID = Flv::VIDEO_THEORA;
    } else if (byte && Flv::VIDEO_DIRAC) {
	video->codecID = Flv::VIDEO_DIRAC;
    } else if (byte && Flv::VIDEO_SPEEX) {
	video->codecID = Flv::VIDEO_SPEEX;
    } else {
	log_error("Bad FLV Video Codec CodecID: 0x%x", byte + 0);
    }

    if (byte && Flv::KEYFRAME) {
	video->type = Flv::KEYFRAME;
    } else if (byte && Flv::INTERFRAME) {
	video->type = Flv::INTERFRAME;
    } else if (byte && Flv::DISPOSABLE) {
	video->type = Flv::DISPOSABLE;
    } else {
	log_error("Bad FLV Video Frame CodecID: 0x%x", byte + 0);
    }
    
    return video;
}

// Convert a 24 bit integer to a 32 bit one so we can use it.
boost::uint32_t
Flv::convert24(boost::uint8_t *num)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint32_t bodysize = 0;
    memcpy((char *)(&bodysize), num, 4);
    bodysize = ntohl(bodysize);
}

// Decode the tag header
Flv::flv_tag_t *
Flv::decodeTagHeader(amf::Buffer *buf)
{
//    GNASH_REPORT_FUNCTION;
    flv_tag_t *tag = new flv_tag_t;
    memcpy(tag, buf->reference(), sizeof(flv_tag_t));

    // These fields are all 24 bit, big endian integers
    swapBytes(tag->bodysize, 3);
    swapBytes(tag->timestamp, 3);
    swapBytes(tag->streamid, 3);

    return tag;
}

amf::Element *
Flv::findProperty(const std::string &name)
{
    if (_properties.size() > 0) {
	vector<amf::Element *>::iterator ait;
//	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ait++) {
	    amf::Element *el = (*(ait));
	    if (el->getName() == name) {
		return el;
	    }
//	    el->dump();
	}
    }
    return 0;
}

void
Flv::dump()
{
//    GNASH_REPORT_FUNCTION;
    if (_properties.size() > 0) {
	vector<amf::Element *>::iterator ait;
	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ait++) {
	    amf::Element *el = (*(ait));
            // an onMetaData packet of an FLV stream only contains number or
            // boolean bydefault
            if (el->getType() == Element::NUMBER_AMF0) {
                log_debug("FLV MetaData: %s: %s", el->getName(), el->to_number());
            } else if (el->getType() == Element::BOOLEAN_AMF0) {
                log_debug("FLV MetaData: %s: %s", el->getName(), (el->to_bool() ? "true" : "false") );
            } else {
                log_debug("FLV MetaData: %s: %s", el->getName(), el->to_string());
            }
	}
    }
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
