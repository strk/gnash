// FLVParser.h:  Flash Video file format parser, for Gnash.
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


// Information about the FLV format can be found at http://osflash.org/flv

#ifndef GNASH_FLVPARSER_H
#define GNASH_FLVPARSER_H

#include "dsodefs.h"
#include "MediaParser.h" // for inheritance

#include <memory>
#include <map>

#include <boost/thread/mutex.hpp>

namespace gnash {
namespace media {

/// Extra video info found in some FLV embedded streams
//
/// This is basically composed by an header for the audio stream
///
class ExtraVideoInfoFlv : public VideoInfo::ExtraInfo
{
public:

    /// Construct an ExtraVideoInfoFlv
    //
    /// @param extradata
    ///     Video header data. Ownership transferred.
    ///
    /// @param datasize
    ///     Video header data size
    ///
    /// @todo take a SimpleBuffer by unique_ptr
    ///
    ExtraVideoInfoFlv(std::uint8_t* extradata, size_t datasize)
        :
        data(extradata),
        size(datasize)
    {
    }

    /// Video stream header
    std::unique_ptr<std::uint8_t[]> data;

    /// Video stream header size
    size_t size;
};

/// Extra audoi info found in some FLV embedded streams
//
/// This is basically composed by an header for the audio stream
///
class ExtraAudioInfoFlv : public AudioInfo::ExtraInfo
{
public:

    /// Construct an ExtraAudioInfoFlv
    //
    /// @param extradata
    ///     Audio header data. Ownership transferred.
    ///
    /// @param datasize
    ///     Audio header data size
    ///
    /// @todo take a SimpleBuffer by unique_ptr
    ///
    ExtraAudioInfoFlv(std::uint8_t* extradata, size_t datasize)
        :
        data(extradata),
        size(datasize)
    {
    }

    /// Audio stream header
    std::unique_ptr<std::uint8_t[]> data;

    /// Audio stream header size
    size_t size;
};

/// The FLVParser class parses FLV streams
class DSOEXPORT FLVParser : public MediaParser
{

public:

    /// The size of padding for all buffers that might be read by FFMPEG.
    //
    /// This possibly also applies to other media handlers (gst).
    /// Ideally this would be taken from the current Handler, but we don't
    /// know about it here.
    static const size_t paddingBytes = 8;

	/// \brief
	/// Create an FLV parser reading input from
	/// the given IOChannel
	//
	/// @param lt
	/// 	IOChannel to use for input.
	/// 	Ownership transferred.
	///
	FLVParser(std::unique_ptr<IOChannel> lt);

	/// Kills the parser...
	~FLVParser();

	// see dox in MediaParser.h
	virtual bool seek(std::uint32_t&);

	// see dox in MediaParser.h
	virtual bool parseNextChunk();

	// see dox in MediaParser.h
	std::uint64_t getBytesLoaded() const;

	// see dox in MediaParser.h
	bool indexingCompleted() const
	{
		return _indexingCompleted;
	}

    /// Retrieve any parsed metadata tags up to a specified timestamp.
    //
    /// This copies pointers to a SimpleBuffer of AMF data from _metaTags,
    /// then removes those pointers from the MetaTags map. Any metadata later
    /// than the timestamp is kept until fetchMetaTags is called again (or 
    /// the dtor is called).
    //
    /// @param ts   The latest timestamp to retrieve metadata for.
    /// @param tags This is filled with shared pointers to metatags in
    ///             timestamp order. Ownership of the data is shared. It
    ///             is destroyed automatically along with the last owner.
    //
    virtual void fetchMetaTags(OrderedMetaTags& tags, std::uint64_t ts);

private:

	enum tagType
	{
		FLV_AUDIO_TAG = 0x08,
		FLV_VIDEO_TAG = 0x09,
		FLV_META_TAG = 0x12
	};

	struct FLVTag : public boost::noncopyable
	{
		FLVTag(std::uint8_t* stream)
		    :
            type(stream[0]),
            body_size(getUInt24(stream+1)),
            timestamp(getUInt24(stream+4) | (stream[7] << 24) )
		{}

		/// Equals tagType
		std::uint8_t type;
		std::uint32_t body_size;
		std::uint32_t timestamp;
	};

	struct FLVAudioTag : public boost::noncopyable
	{
		FLVAudioTag(const std::uint8_t& byte)
		    :
            codec( (byte & 0xf0) >> 4 ),
		    samplerate( flv_audio_rates[(byte & 0x0C) >> 2] ),
		    samplesize( 1 + ((byte & 0x02) >> 1)),
		    stereo( (byte & 0x01) )
		{
		}

		/// Equals audioCodecType
		std::uint8_t codec;

		std::uint16_t samplerate;

		/// Size of each sample, in bytes
		std::uint8_t samplesize;

		bool stereo;

    private:
	
        static const std::uint16_t flv_audio_rates[];
	
    };

	enum frameType
	{
		FLV_VIDEO_KEYFRAME = 1,
		FLV_VIDEO_INTERLACED = 2,
		FLV_VIDEO_DISPOSABLE = 3
	};

	struct FLVVideoTag : public boost::noncopyable
	{
		FLVVideoTag(const std::uint8_t& byte)
            :
            frametype( (byte & 0xf0) >> 4 ),
		    codec( byte & 0x0f )
		{}

		/// Equals frameType
		std::uint8_t frametype;
		/// Equals videoCodecType
		std::uint8_t codec;
	};

	/// Parses next tag from the file
	//
	/// Returns true if something was parsed, false otherwise.
	/// Sets _parsingComplete=true on end of file.
	///
	bool parseNextTag(bool index_only);

	std::unique_ptr<EncodedAudioFrame> parseAudioTag(const FLVTag& flvtag,
            const FLVAudioTag& audiotag, std::uint32_t thisTagPos);
	
    std::unique_ptr<EncodedVideoFrame> parseVideoTag(const FLVTag& flvtag,
            const FLVVideoTag& videotag, std::uint32_t thisTagPos);

	void indexAudioTag(const FLVTag& tag, std::uint32_t thisTagPos);
	
    void indexVideoTag(const FLVTag& tag, const FLVVideoTag& videotag,
            std::uint32_t thisTagPos);

	/// Parses the header of the file
	bool parseHeader();

	/// Reads three bytes in FLV (big endian) byte order.
	/// @param in Pointer to read 3 bytes from.
	/// @return 24-bit integer.
	static std::uint32_t getUInt24(std::uint8_t* in);

	/// The position where the parsing should continue from.
	/// Will be reset on seek, and will be protected by the _streamMutex
	std::uint64_t _lastParsedPosition;

	/// Position of next tag to index
	std::uint64_t _nextPosToIndex;

	/// Audio stream is present
	bool _audio;

	/// Audio stream is present
	bool _video;

	std::unique_ptr<EncodedAudioFrame>
        readAudioFrame(std::uint32_t dataSize, std::uint32_t timestamp);

	std::unique_ptr<EncodedVideoFrame>
        readVideoFrame(std::uint32_t dataSize, std::uint32_t timestamp);

	/// Position in input stream for each cue point
	/// first: timestamp
	/// second: position in input stream
	typedef std::map<std::uint64_t, long> CuePointsMap;
	CuePointsMap _cuePoints;

	bool _indexingCompleted;

    MetaTags _metaTags;

    boost::mutex _metaTagsMutex;
};

} // end of gnash::media namespace
} // end of gnash namespace

#endif 
