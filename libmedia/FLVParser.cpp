// FLVParser.cpp:  Flash Video file parser, for Gnash.
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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


#include "FLVParser.h"
#include "log.h"
#include "utility.h"
#include "GnashException.h"
#include "IOChannel.h"
#include "SimpleBuffer.h"
#include "GnashAlgorithm.h"

#include <string>
#include <iosfwd>

// Define the following macro the have seek() operations printed
//#define GNASH_DEBUG_SEEK 1

namespace gnash {
namespace media {


const size_t FLVParser::paddingBytes;
const boost::uint16_t FLVParser::FLVAudioTag::flv_audio_rates [] = 
    { 5500, 11000, 22050, 44100 };

FLVParser::FLVParser(std::auto_ptr<IOChannel> lt)
	:
	MediaParser(lt),
	_lastParsedPosition(0),
	_nextPosToIndex(0),
	_nextAudioFrame(0),
	_nextVideoFrame(0),
	_audio(false),
	_video(false),
	_cuePoints(),
	_indexingCompleted(false)
{
	if (!parseHeader()) {
		throw MediaException("FLVParser couldn't parse header from input");
    }

	startParserThread();
}

FLVParser::~FLVParser()
{
	stopParserThread();
}


// would be called by main thread
bool
FLVParser::seek(boost::uint32_t& time)
{

	boost::mutex::scoped_lock streamLock(_streamMutex);
	// we might obtain this lock while the parser is pushing the last
	// encoded frame on the queue, or while it is waiting on the wakeup
	// condition

	// Setting _seekRequest to true will make the parser thread
	// take care of cleaning up the buffers before going on with
	// parsing, thus fixing the case in which streamLock was obtained
	// while the parser was pushing to queue
	_seekRequest = true;

	if ( _cuePoints.empty() )
	{
		log_debug("No known cue points yet, can't seek");
		return false;
	}

	CuePointsMap::iterator it = _cuePoints.lower_bound(time);
	if ( it == _cuePoints.end() )
	{
		log_debug("No cue points greater or equal requested time %d", time);
		return false;
	}

	long lowerBoundPosition = it->second;
	log_debug("Seek requested to time %d triggered seek to cue point at "
            "position %d and time %d", time, it->second, it->first);
	time = it->first;
	_lastParsedPosition=lowerBoundPosition; 
	_parsingComplete=false; // or NetStream will send the Play.Stop event...


	// Finally, clear the buffers.
	// The call will also wake the parse up if it was sleeping.
	// WARNING: a race condition might be pending here:
	// If we handled to do all the seek work in the *small*
	// time that the parser runs w/out mutex locked (ie:
	// after it unlocked the stream mutex and before it locked
	// the queue mutex), it will still push an old encoded frame
	// to the queue; if the pushed frame alone makes it block
	// again (bufferFull) we'll have a problem.
	// Note though, that a single frame can't reach a bufferFull
	// condition, as it takes at least two for anything != 0.
	//
	clearBuffers();

	return true;
}

// would be called by parser thread
bool
FLVParser::parseNextChunk()
{
	bool indexOnly = bufferFull(); // won't lock, but our caller locked...
	return parseNextTag(indexOnly);
}

// would be called by parser thread
void
FLVParser::indexAudioTag(const FLVTag& tag, boost::uint32_t thisTagPos)
{
	if ( _videoInfo.get()) {
		// if we have video we let that drive cue points
		return;
	}

	// we can theoretically seek anywhere, but
	// let's just keep 5 seconds of distance
	CuePointsMap::iterator it = _cuePoints.lower_bound(tag.timestamp);
	if ( it == _cuePoints.end() || it->first - tag.timestamp >= 5000)
	{
		//log_debug("Added cue point at timestamp %d and position %d "
        //"(audio frame)", tag.timestamp, thisTagPos);
		_cuePoints[tag.timestamp] = thisTagPos; 
	}
}

void
FLVParser::indexVideoTag(const FLVTag& tag, const FLVVideoTag& videotag, boost::uint32_t thisTagPos)
{
	if ( videotag.frametype != FLV_VIDEO_KEYFRAME ) {
		return;
	}

	//log_debug("Added cue point at timestamp %d and position %d "
    //"(key video frame)", tag.timestamp, thisTagPos);
	_cuePoints[tag.timestamp] = thisTagPos;
}


std::auto_ptr<EncodedAudioFrame>
FLVParser::parseAudioTag(const FLVTag& flvtag, const FLVAudioTag& audiotag, boost::uint32_t thisTagPos)
{
	std::auto_ptr<EncodedAudioFrame> frame;

	if ( ! _audio ) {
		log_error(_("Unexpected audio tag found at offset %d FLV stream "
                    "advertising no audio in header. We'll warn only once for "
                    "each FLV, expecting any further audio tag."), thisTagPos);
		_audio = true; // TOCHECK: is this safe ?
	}

	bool header = false;
	boost::uint32_t bodyLength = flvtag.body_size;

	if (audiotag.codec == AUDIO_CODEC_AAC) {
		boost::uint8_t packettype = _stream->read_byte();
		header = (packettype == 0);
		--bodyLength;
	}

	frame = readAudioFrame(bodyLength-1, flvtag.timestamp);
	if ( ! frame.get() ) {
            log_error(_("could not read audio frame?"));
	}

	// If this is the first audioframe no info about the
	// audio format has been noted, so we do that now
	if (!_audioInfo.get()) {
		_audioInfo.reset(new AudioInfo(audiotag.codec, audiotag.samplerate,
                    audiotag.samplesize, audiotag.stereo, 0,
                    CODEC_TYPE_FLASH));

        if (header) {

            // The frame is 0-padded up to the end. It may be larger than
            // this if fewer bytes were read than requested, but it is
            // never smaller.
            const size_t bufSize = frame->dataSize + paddingBytes;

            boost::uint8_t* data = new boost::uint8_t[bufSize];

            std::copy(frame->data.get(), frame->data.get() + bufSize, data);

			_audioInfo->extra.reset( 
				new ExtraAudioInfoFlv(data, frame->dataSize)
			);

			// The FAAD decoder will reject us if we pass the header buffer.
			// It will receive the header via the extra audio info anyway.
			frame.reset();
		}
	}

	return frame;
}

std::auto_ptr<EncodedVideoFrame>
FLVParser::parseVideoTag(const FLVTag& flvtag, const FLVVideoTag& videotag, boost::uint32_t thisTagPos)
{
	if ( ! _video ) {
		log_error(_("Unexpected video tag found at offset %d of FLV stream "
                    "advertising no video in header. We'll warn only once per "
                    "FLV, expecting any further video tag."), thisTagPos);
		_video = true; // TOCHECK: is this safe ?
	}

	bool header = false;
	boost::uint32_t bodyLength = flvtag.body_size;

	switch(videotag.codec) {
		case VIDEO_CODEC_VP6:
		case VIDEO_CODEC_VP6A:
		{
			_stream->read_byte();
			--bodyLength;
			break;
		}
		case VIDEO_CODEC_H264:
		{
			boost::uint8_t packettype = _stream->read_byte();
			IF_VERBOSE_PARSE( log_debug(_("AVC packet type: %d"),
                        (unsigned)packettype) );

			header = (packettype == 0);

			// 24-bits value for composition time offset ignored for now.
			boost::uint8_t tmp[3];
			_stream->read(tmp, 3);
	
			bodyLength -= 4;
			break;
		}
		default:
			break;
	}

	std::auto_ptr<EncodedVideoFrame> frame = readVideoFrame(bodyLength-1,
            flvtag.timestamp);
	if ( ! frame.get() ) {
            log_error(_("could not read video frame?"));
	}

	// If this is the first videoframe no info about the
	// video format has been noted, so we do that now
	if ( ! _videoInfo.get() ) {
		_videoInfo.reset(new VideoInfo(videotag.codec, 0, 0, 0, 0,
                    CODEC_TYPE_FLASH));

		if (header) {
            // The frame is 0-padded up to the end. It may be larger than
            // this if fewer bytes were read than requested, but it is
            // never smaller.
            const size_t bufSize = frame->dataSize() + paddingBytes;

            boost::uint8_t* data = new boost::uint8_t[bufSize];

            std::copy(frame->data(), frame->data() + bufSize, data);
			_videoInfo->extra.reset( 
				new ExtraVideoInfoFlv(data, frame->dataSize())
			);

			// Don't bother emitting the header buffer.
			frame.reset();
		}
	}
	return frame;
}


// would be called by parser thread
bool
FLVParser::parseNextTag(bool index_only)
{
	// lock the stream while reading from it, so actionscript
	// won't mess with the parser on seek  or on getBytesLoaded
	boost::mutex::scoped_lock streamLock(_streamMutex);

	if ( index_only && _indexingCompleted ) return false; 
	if ( _parsingComplete ) return false;

	if ( _seekRequest )
	{
		clearBuffers();
		_seekRequest = false;
	}

	boost::uint64_t& position = index_only ? _nextPosToIndex : _lastParsedPosition;
	bool& completed = index_only ? _indexingCompleted : _parsingComplete;

	//log_debug("parseNextTag: _lastParsedPosition:%d, _nextPosToIndex:%d, index_only:%d", _lastParsedPosition, _nextPosToIndex, index_only);

	unsigned long thisTagPos = position;

	// Seek to next frame and skip the tag size 
	//log_debug("FLVParser::parseNextTag seeking to %d", thisTagPos+4);
	if (!_stream->seek(thisTagPos+4))
	{
            log_error(_("FLVParser::parseNextTag: can't seek to %d"),
                      thisTagPos+4);

		completed = true;
		return false;
	}
	//log_debug("FLVParser::parseNextTag seeked to %d", thisTagPos+4);

	// Read the tag info
	boost::uint8_t chunk[12];
	int actuallyRead = _stream->read(chunk, 12);
	if ( actuallyRead < 12 )
	{
		if ( actuallyRead )
                    log_error(_("FLVParser::parseNextTag: can't read tag info "
                                "(needed 12 bytes, only got %d)"), actuallyRead);
		// else { assert(_stream->eof(); } ?

		completed = true;

        // update bytes loaded
        boost::mutex::scoped_lock lock(_bytesLoadedMutex);
		_bytesLoaded = _stream->tell(); 
		return false;
	}

	FLVTag flvtag(chunk);

    // May be _lastParsedPosition OR _nextPosToIndex
    position += 15 + flvtag.body_size; 

	bool doIndex = (_lastParsedPosition+4 > _nextPosToIndex) || index_only;
	if ( _lastParsedPosition > _nextPosToIndex )
	{
		//log_debug("::parseNextTag setting _nextPosToIndex=%d", _lastParsedPosition+4);
		_nextPosToIndex = _lastParsedPosition;
	}

	if ( position > _bytesLoaded ) {
		boost::mutex::scoped_lock lock(_bytesLoadedMutex);
		_bytesLoaded = position;
	}

	// check for empty tag
	if (flvtag.body_size == 0) return true;

	if (flvtag.type == FLV_AUDIO_TAG)
	{
		FLVAudioTag audiotag(chunk[11]);

		if (doIndex) {
			indexAudioTag(flvtag, thisTagPos);
			if (index_only) {
				return true;
			}
		}


		std::auto_ptr<EncodedAudioFrame> frame = 
            parseAudioTag(flvtag, audiotag, thisTagPos);
		if (!frame.get()) {
			return false;
		}
		// Release the stream lock 
		// *before* pushing the frame as that 
		// might block us waiting for buffers flush
		// the _qMutex...
		// We've done using the stream for this tag parsing anyway
		streamLock.unlock();
		pushEncodedAudioFrame(frame);
	}
	else if (flvtag.type == FLV_VIDEO_TAG)
	{
		FLVVideoTag videotag(chunk[11]);

		if (doIndex) {
			indexVideoTag(flvtag, videotag, thisTagPos);
			if (index_only) {
				return true;
			}
		}

		std::auto_ptr<EncodedVideoFrame> frame = 
            parseVideoTag(flvtag, videotag, thisTagPos);
		if (!frame.get()) {
			return false;
		}

		// Release the stream lock 
		// *before* pushing the frame as that 
		// might block us waiting for buffers flush
		// the _qMutex...
		streamLock.unlock();
		pushEncodedVideoFrame(frame);

	}
	else if (flvtag.type == FLV_META_TAG)
	{
		if ( chunk[11] != 2 )
		{
			// ::processTags relies on the first AMF0 value being a string...
			log_unimpl(_("First byte of FLV_META_TAG is %d, expected "
                        "0x02 (STRING AMF0 type)"),
                    static_cast<int>(chunk[11]));
		}
		// Extract information from the meta tag
		std::auto_ptr<SimpleBuffer> metaTag(new SimpleBuffer(
                    flvtag.body_size-1));
		size_t actuallyRead = _stream->read(metaTag->data(),
                flvtag.body_size - 1);

        if ( actuallyRead < flvtag.body_size-1 )
		{
                    log_error(_("FLVParser::parseNextTag: can't read metaTag (%d) "
                                "body (needed %d bytes, only got %d)"),
				FLV_META_TAG, flvtag.body_size, actuallyRead);
			return false;
		}
		metaTag->resize(actuallyRead);

		boost::uint32_t terminus = getUInt24(metaTag->data() +
                actuallyRead - 3);

        if (terminus != 9) {
			log_error(_("Corrupt FLV: Meta tag unterminated!"));
		}

		boost::mutex::scoped_lock lock(_metaTagsMutex);
		_metaTags.insert(std::make_pair(flvtag.timestamp, metaTag.release()));
	}
	else
	{
		log_error(_("FLVParser::parseNextTag: unknown FLV tag type %d"),
                (int)chunk[0]);
		return false;
	}

	_stream->read(chunk, 4);
	boost::uint32_t prevtagsize = chunk[0] << 24 | chunk[1] << 16 |
        chunk[2] << 8 | chunk[3];
	if (prevtagsize != flvtag.body_size + 11) {
		log_error(_("Corrupt FLV: previous tag size record (%1%) unexpected "
                    "(actual size: %2%)"), prevtagsize, flvtag.body_size + 11);
	}

	return true;
}

// would be called by MAIN thread
bool
FLVParser::parseHeader()
{
	assert(_stream->tell() == static_cast<std::streampos>(0));

	// We only use 5 bytes of the header, because the last 4 bytes represent
    // an integer which is always 1.
	boost::uint8_t header[9];
	if ( _stream->read(header, 9) != 9 )
	{
            log_error(_("FLVParser::parseHeader: couldn't read 9 bytes of header"));
		return false;
	}

	_lastParsedPosition = _bytesLoaded = _nextPosToIndex = 9;

	if (!std::equal(header, header + 3, "FLV")) {
		return false;
	}

	const boost::uint8_t version = header[3];

	// Parse the audio+video bitmask
	_audio = header[4]&(1<<2);
	_video = header[4]&(1<<0);

	log_debug("Parsing FLV version %d, audio:%d, video:%d",
			(int)version, _audio, _video);

	return true;
}

inline boost::uint32_t
FLVParser::getUInt24(boost::uint8_t* in)
{
	// The bits are in big endian order
	return (in[0] << 16) | (in[1] << 8) | in[2];
}

boost::uint64_t
FLVParser::getBytesLoaded() const
{
	boost::mutex::scoped_lock lock(_bytesLoadedMutex);
	return _bytesLoaded;
}

// would be called by parser thread
/*private*/
std::auto_ptr<EncodedAudioFrame>
FLVParser::readAudioFrame(boost::uint32_t dataSize, boost::uint32_t timestamp)
{

	std::auto_ptr<EncodedAudioFrame> frame(new EncodedAudioFrame);
    
    const size_t bufSize = dataSize + paddingBytes;

    boost::uint8_t* data = new boost::uint8_t[bufSize];
	const size_t bytesRead = _stream->read(data, dataSize);

    std::fill(data + bytesRead, data + bufSize, 0);

	if (bytesRead < dataSize) {
            log_error(_("FLVParser::readAudioFrame: could only read %d/%d bytes"),
                bytesRead, dataSize);
	}

	frame->dataSize = bytesRead;
	frame->timestamp = timestamp;
	frame->data.reset(data);

	return frame;
}

// would be called by parser thread
/*private*/
std::auto_ptr<EncodedVideoFrame>
FLVParser::readVideoFrame(boost::uint32_t dataSize, boost::uint32_t timestamp)
{
	std::auto_ptr<EncodedVideoFrame> frame;

    const size_t bufSize = dataSize + paddingBytes;

	boost::uint8_t* data = new boost::uint8_t[bufSize];
	const size_t bytesRead = _stream->read(data, dataSize);

    std::fill(data + bytesRead, data + bufSize, 0);

	// We won't need frameNum, so will set to zero...
	// TODO: fix this ?
	// NOTE: ownership of 'data' is transferred here

	frame.reset(new EncodedVideoFrame(data, bytesRead, 0, timestamp));
	return frame;
}


void
FLVParser::fetchMetaTags(OrderedMetaTags& tags, boost::uint64_t ts)
{
	boost::mutex::scoped_lock lock(_metaTagsMutex);
	if (!_metaTags.empty()) {
        MetaTags::iterator it = _metaTags.upper_bound(ts);

        // Copy the first value into the return container.
        std::transform(_metaTags.begin(), it, std::back_inserter(tags),
                boost::bind(&MetaTags::value_type::second, _1));

        _metaTags.erase(_metaTags.begin(), it);
	}
}

} // end of gnash::media namespace
} // end of gnash namespace
