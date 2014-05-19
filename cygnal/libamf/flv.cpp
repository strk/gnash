// amf.cpp:  AMF (Action Message Format) rpc marshalling, for Gnash.
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
//

#include "GnashSystemNetHeaders.h"
#include "buffer.h"
#include "log.h"
#include "amf.h"
#include "amfutf8.h"
#include "utility.h"
#include "flv.h"

#include <boost/detail/endian.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <climits>
#include <boost/cstdint.hpp> 

using gnash::log_debug;
using gnash::log_error;

namespace cygnal
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
std::shared_ptr<cygnal::Buffer>
Flv::encodeHeader(boost::uint8_t type)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<cygnal::Buffer> buf(new Buffer(sizeof(Flv::flv_header_t)));
    buf->clear();
    
    boost::uint8_t version = 0x1;
    *buf = "FLV";
    *buf += version;

    *buf += type;

    boost::uint32_t size = htonl(0x9);
    buf->append((boost::uint8_t *)&size, sizeof(boost::uint32_t));

    return buf;
}

// Decode a Buffer into a header
std::shared_ptr<Flv::flv_header_t>
Flv::decodeHeader(boost::uint8_t *data)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<flv_header_t> header(new flv_header_t);
    memcpy(header.get(), data, sizeof(flv_header_t));
//    std::copy(buf->begin(), buf->begin() + sizeof(flv_header_t), header.get());

    // test the magic number
    if (memcmp(header->sig, "FLV", 3) != 0) {
	log_error(_("Bad magic number for FLV file!"));
	header.reset();
	return header;
    }

    // Make sure the version is legit, it should alwys be 1
    if (header->version != 0x1) {
	log_error(_("Bad version in FLV header! %d"), _header.version);
	header.reset();
	return header;
    }

    // Make sure the type is set correctly
    if (((header->type & Flv::FLV_AUDIO) && (header->type & Flv::FLV_VIDEO))
	|| (header->type & Flv::FLV_AUDIO) || (header->type & Flv::FLV_VIDEO)) {
    } else {
	log_error(_("Bad FLV file Type: %d"), header->type);
    }
    
    // Be lazy, as head_size is an array of 4 bytes, and not an integer in the data
    // structure. This is to get around possible padding done to the data structure
    // done by some compilers.
    boost::uint32_t size = *(reinterpret_cast<boost::uint32_t *>(header->head_size));
    // The header size is big endian
    swapBytes(header->head_size, sizeof(boost::uint32_t));
    
    // The header size is always 9, guess it could change some day in the far future, so
    // we should use it.
    if (ntohl(size) != 0x9) {
	log_error(_("Bad header size in FLV header! %d"), size);
	header.reset();
    }
    
    return header;
}

// Decode a MetaData object, which is after the header, but before all the tags
std::shared_ptr<cygnal::Element>
Flv::decodeMetaData(std::shared_ptr<cygnal::Buffer> buf)
{
    return decodeMetaData(buf->reference(), buf->size());
}

std::shared_ptr<cygnal::Element>
Flv::decodeMetaData(boost::uint8_t *buf, size_t size)
{
//    GNASH_REPORT_FUNCTION;
    AMF amf;
    boost::uint8_t *ptr = buf;
    boost::uint8_t *tooFar = ptr + size;
    
    // Extract the onMetaData object name
    // In disk files, I always see the 0x2 type field for
    // a string, but not always in streaming, at least according to
    // Gnash's libmedia/FLVParser code. Since this is always 
    if (*ptr == Element::STRING_AMF0) {
	ptr++;
    }
    
    boost::uint16_t length;
    length = ntohs((*(boost::uint16_t *)ptr) & 0xffff);
    if (length >= SANE_STR_SIZE) {
	log_error(_("%d bytes for a string is over the safe limit of %d"),
		  length, SANE_STR_SIZE);
    }
    ptr += sizeof(boost::uint16_t);
    std::string name(reinterpret_cast<const char *>(ptr), length);
    ptr += length;
    
    // Extract the properties for this metadata object.
    _metadata = amf.extractAMF(ptr, tooFar);
    if (_metadata.get()) {
    	ptr += amf.totalsize();
	    _metadata->setName(name.c_str(), length);
    }

    return _metadata;
}

std::shared_ptr<Flv::flv_audio_t>
Flv::decodeAudioData(boost::uint8_t byte)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<flv_audio_t> audio(new flv_audio_t);
//    memset(audio->reference(), 0, sizeof(flv_audio_t));

    // Get the sound type
    if (byte && Flv::AUDIO_STEREO) {
	audio->type = Flv::AUDIO_STEREO;
    } else if (!byte && Flv::AUDIO_STEREO) {
	audio->type = Flv::AUDIO_MONO;
    } else {
	log_error(_("Bad FLV Audio Sound Type: %x"), byte + 0);
    }

    // Get the sound size
    if ((byte >> 1) && Flv::AUDIO_16BIT) {
	audio->size = Flv::AUDIO_16BIT;	
    } else if (!(byte >> 1) && Flv::AUDIO_16BIT) {
	audio->size = Flv::AUDIO_8BIT;	
    } else {
	log_error(_("Bad FLV Audio Sound size: %d"), byte >> 1);
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
	log_error(_("Bad FLV Audio Sound Rate: %d"), byte >> 2);
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
	log_error(_("Bad FLV Audio Sound format: %d"), byte >> 4);
    }
    
    return audio;
}

std::shared_ptr<Flv::flv_video_t>
Flv::decodeVideoData(boost::uint8_t byte)
{
//    GNASH_REPORT_FUNCTION;
    std::shared_ptr<flv_video_t> video(new flv_video_t);
//    memset(video, 0, sizeof(flv_video_t));

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
	log_error(_("Bad FLV Video Codec CodecID: 0x%x"), byte + 0);
    }

    if (byte && Flv::KEYFRAME) {
	video->type = Flv::KEYFRAME;
    } else if (byte && Flv::INTERFRAME) {
	video->type = Flv::INTERFRAME;
    } else if (byte && Flv::DISPOSABLE) {
	video->type = Flv::DISPOSABLE;
    } else {
	log_error(_("Bad FLV Video Frame CodecID: 0x%x"), byte + 0);
    }
    
    return video;
}

// Convert a 24 bit integer to a 32 bit one so we can use it.
boost::uint32_t
Flv::convert24(boost::uint8_t *num)
{
//    GNASH_REPORT_FUNCTION;
    boost::uint32_t bodysize = 0;

#ifdef BOOST_BIG_ENDIAN
    bodysize = *(reinterpret_cast<boost::uint32_t *>(num)) >> 8;
#else
    bodysize = *(reinterpret_cast<boost::uint32_t *>(num)) << 8;
    bodysize = ntohl(bodysize);
#endif
    
    return bodysize;
}

// Decode the tag header
std::shared_ptr<Flv::flv_tag_t>
Flv::decodeTagHeader(boost::uint8_t *buf)
{
//    GNASH_REPORT_FUNCTION;
    flv_tag_t *data = reinterpret_cast<flv_tag_t *>(buf);
    std::shared_ptr<flv_tag_t> tag(new flv_tag_t);
    memcpy(tag.get(), data, sizeof(flv_tag_t));

//    std::copy(buf->begin(), buf->end(), tag);

    // These fields are all 24 bit, big endian integers
    swapBytes(tag->bodysize, 3);
    swapBytes(tag->timestamp, 3);
    swapBytes(tag->streamid, 3);

    return tag;
}

std::shared_ptr<cygnal::Element>
Flv::findProperty(const std::string &name)
{
    if (_properties.size() > 0) {
	std::vector<std::shared_ptr<cygnal::Element> >::iterator ait;
//	cerr << "# of Properties in object: " << _properties.size() << endl;
	for (ait = _properties.begin(); ait != _properties.end(); ++ait) {
	    std::shared_ptr<cygnal::Element> el = (*(ait));
	    if (el->getName() == name) {
		return el;
	    }
//	    el->dump();
	}
    }
    std::shared_ptr<cygnal::Element> el;
    return el;
}

void
Flv::dump()
{
//    GNASH_REPORT_FUNCTION;
    if (_properties.size() > 0) {
	std::vector<std::shared_ptr<cygnal::Element> >::iterator ait;
	std::cerr << "# of Properties in object: " << _properties.size()
	          << std::endl;
	for (ait = _properties.begin(); ait != _properties.end(); ++ait) {
	    std::shared_ptr<cygnal::Element> el = (*(ait));
            // an onMetaData packet of an FLV stream only contains number or
            // boolean bydefault
            if (el->getType() == Element::NUMBER_AMF0) {
                log_debug(_("FLV MetaData: %s: %s"), el->getName(), el->to_number());
            } else if (el->getType() == Element::BOOLEAN_AMF0) {
                log_debug(_("FLV MetaData: %s: %s"), el->getName(), (el->to_bool() ? "true" : "false") );
            } else {
                log_debug(_("FLV MetaData: %s: %s"), el->getName(), el->to_string());
            }
	}
    } else {
	std::cerr << "No properties" << std::endl;
    }
}

} // end of amf namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
