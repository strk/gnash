// FLVParser.cpp:  Flash Video file parser, for Gnash.
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
//


#include "FLVParser.h"
#include "amf.h"
#include "element.h"
#include "flv.h"
#include "log.h"
#include "utility.h"
#include "GnashException.h"
#include "IOChannel.h"
#include "SimpleBuffer.h"

#include "as_object.h"
#include "array.h"
#include "element.h"
#include "VM.h"

#include <string>
#include <iosfwd>

using namespace std;

#define PADDING_BYTES 64
#define READ_CHUNKS 64

// The amount of bytes to scan an FLV for
// figuring availability of audio/video tags.
// This is needed because we can't trust the FLV
// header (youtube advertises audio w/out having it;
// strk tested that FLV with bogus flags are still
// supposed to show video).
//
// MediaParserFfmpeg.cpp uses 2048.
//
// TODO: let user tweak this ?
//
// See bug #24371 for pointers to FLV files needing so much.
//
#define PROBE_BYTES 8192

// Define the following macro the have seek() operations printed
//#define GNASH_DEBUG_SEEK 1

namespace gnash {
namespace media {

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
	if ( ! parseHeader() ) 
		throw GnashException("FLVParser couldn't parse header from input");
	startParserThread();
}

FLVParser::~FLVParser()
{
	for (MetaTags::iterator i=_metaTags.begin(), e=_metaTags.end(); i!=e; ++i)
	{
		delete *i;
	}
}


// would be called by main thread
bool
FLVParser::seek(boost::uint32_t& time)
{
	//GNASH_REPORT_FUNCTION;

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
	log_debug("Seek requested to time %d triggered seek to cue point at position %d and time %d", time, it->second, it->first);
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
	if ( indexOnly ) return indexNextTag();
	else return parseNextTag();
}

// would be called by parser thread
bool
FLVParser::indexNextTag()
{
	//GNASH_REPORT_FUNCTION;

	// lock the stream while reading from it, so actionscript
	// won't mess with the parser on seek  or on getBytesLoaded
	boost::mutex::scoped_lock streamLock(_streamMutex);

	if ( _indexingCompleted ) return false;

	unsigned long thisTagPos = _nextPosToIndex;

	if ( _stream->seek(thisTagPos+4) )
	{
		log_debug("FLVParser::indexNextTag failed seeking to %d: %s", thisTagPos+4);
		_indexingCompleted=true;
		//_parsingComplete=true; // better let ::parseNextTag set this
		return false;
	}

	// Read the tag info - NOTE: will block... (should we avoid the block here ?)
	boost::uint8_t tag[12];
	int actuallyRead = _stream->read(tag, 12);
	if ( actuallyRead < 12 )
	{
		if ( actuallyRead )
			log_error("FLVParser::indexNextTag: can't read tag info (needed 12 bytes, only got %d)", actuallyRead);
		// else { assert(_stream->eof(); } ?
		//log_debug("%d bytes read from input stream, when %d were requested", actuallyRead, 12);
		_indexingCompleted=true;
		//_parsingComplete=true; // better let ::parseNextTag set this

		// update bytes loaded
		boost::mutex::scoped_lock lock(_bytesLoadedMutex);
		_bytesLoaded = _stream->tell();

		return false;
	}

	// Extract length and timestamp
	boost::uint32_t bodyLength = getUInt24(&tag[1]);
	boost::uint32_t timestamp = getUInt24(&tag[4]);

	_nextPosToIndex += 15 + bodyLength;
	
	if ( _nextPosToIndex > _bytesLoaded ) {
		boost::mutex::scoped_lock lock(_bytesLoadedMutex);
		_bytesLoaded = _nextPosToIndex;
	}

	// check for empty tag
	if (bodyLength == 0)
	{
		log_debug("Empty tag, no index");
		return false;
	}

	if (tag[0] == FLV_AUDIO_TAG)
	{
		if ( ! _video) // if we have video we let that drive cue points
		{
			// we can theoretically seek anywhere, but
			// let's just keep 5 seconds of distance
			CuePointsMap::iterator it = _cuePoints.lower_bound(timestamp);
			if ( it == _cuePoints.end() || it->first - timestamp >= 5000)
			{
				//log_debug("Added cue point at timestamp %d and position %d (audio frame)", timestamp, thisTagPos);
				_cuePoints[timestamp] = thisTagPos; 
			}
		}
	}
	else if (tag[0] == FLV_VIDEO_TAG)
	{
		// 1:keyframe, 2:interlacedFrame, 3:disposableInterlacedFrame
		int frameType = (tag[11] & 0xf0) >> 4;
		
		bool isKeyFrame = (frameType == 1);
		if ( isKeyFrame )
		{
			//log_debug("Added cue point at timestamp %d and position %d (key video frame)", timestamp, thisTagPos);
			_cuePoints[timestamp] = thisTagPos;
		}
	}
	else
	{
		log_debug("FLVParser::indexNextTag: tag %d is neither audio nor video", (int)tag[0]);
	}

	return true;
}

// would be called by parser thread
bool FLVParser::parseNextTag()
{
	//GNASH_REPORT_FUNCTION;

	// lock the stream while reading from it, so actionscript
	// won't mess with the parser on seek  or on getBytesLoaded
	boost::mutex::scoped_lock streamLock(_streamMutex);

	if ( _parsingComplete ) return false;

	if ( _seekRequest )
	{
		clearBuffers();
		_seekRequest = false;
	}

	unsigned long thisTagPos = _lastParsedPosition;

	// Seek to next frame and skip the tag size 
	//log_debug("FLVParser::parseNextTag seeking to %d", thisTagPos+4);
	if ( _stream->seek(thisTagPos+4) )
	{
		log_error("FLVParser::parseNextTag: can't seek to %d", thisTagPos+4);
		_parsingComplete=true;
		return false;
	}
	//log_debug("FLVParser::parseNextTag seeked to %d", thisTagPos+4);

	// Read the tag info
	boost::uint8_t tag[12];
	int actuallyRead = _stream->read(tag, 12);
	if ( actuallyRead < 12 )
	{
		if ( actuallyRead )
			log_error("FLVParser::parseNextTag: can't read tag info (needed 12 bytes, only got %d)", actuallyRead);
		// else { assert(_stream->eof(); } ?
		_parsingComplete=true;
		return false;
	}

	// Extract length and timestamp
	boost::uint32_t bodyLength = getUInt24(&tag[1]);
	boost::uint32_t timestamp = getUInt24(&tag[4]);

	_lastParsedPosition += 15 + bodyLength;

	bool doIndex = _lastParsedPosition+4 > _nextPosToIndex;
	if ( doIndex )
	{
		//log_debug("::parseNextTag setting _nextPosToIndex=%d", _lastParsedPosition+4);
		_nextPosToIndex = _lastParsedPosition;
	}

	
	if ( _lastParsedPosition > _bytesLoaded ) {
		boost::mutex::scoped_lock lock(_bytesLoadedMutex);
		_bytesLoaded = _lastParsedPosition;
	}

	// check for empty tag
	if (bodyLength == 0) return true;

	if (tag[0] == FLV_AUDIO_TAG)
	{
		if ( ! _audio ) {
			log_error(_("Unexpected audio tag found at offset %d FLV stream advertising no audio in header. We'll warn only once for each FLV, expecting any further audio tag."), thisTagPos);
			_audio = true; // TOCHECK: is this safe ?
		}
		
		if ( doIndex && ! _video ) // if we have video we let that drive cue points
		{
			// we can theoretically seek anywhere, but
			// let's just keep 5 seconds of distance
			CuePointsMap::iterator it = _cuePoints.lower_bound(timestamp);
			if ( it == _cuePoints.end() || it->first - timestamp >= 5000)
			{
				log_debug("Added cue point at timestamp %d and position %d (audio frame)", timestamp, thisTagPos);
				_cuePoints[timestamp] = thisTagPos; 
			}
		}

		boost::uint8_t codec = (tag[11] & 0xf0) >> 4;

		bool header = false;

		if (codec == AUDIO_CODEC_AAC) {
			boost::uint8_t packettype = _stream->read_byte();
			header = (packettype == 0);
		}

		std::auto_ptr<EncodedAudioFrame> frame = readAudioFrame(bodyLength-1, timestamp);
		if ( ! frame.get() ) {
			log_error("could not read audio frame?");
			return true;
		}

		// If this is the first audioframe no info about the
		// audio format has been noted, so we do that now
		if ( !_audioInfo.get() )
		{
			int samplerate = (tag[11] & 0x0C) >> 2;
			if (samplerate == 0) samplerate = 5500;
			else if (samplerate == 1) samplerate = 11000;
			else if (samplerate == 2) samplerate = 22050;
			else if (samplerate == 3) samplerate = 44100;

			int samplesize = (tag[11] & 0x02) >> 1;
			if (samplesize == 0) samplesize = 1;
			else samplesize = 2;

			_audioInfo.reset( new AudioInfo(codec, samplerate, samplesize, (tag[11] & 0x01) >> 0, 0, FLASH) );
			if (header) {
				boost::uint8_t* newbuf = new boost::uint8_t[frame->dataSize];
				memcpy(newbuf, frame->data.get(), frame->dataSize);

				_audioInfo->extra.reset( 
					new ExtraAudioInfoFlv(newbuf, frame->dataSize)
				);

				// The FAAD decoder will reject us if we pass the header buffer.
				// It will receive the header via the extra audio info anyway.
				return true;
			}
		}

		// Release the stream lock 
		// *before* pushing the frame as that 
		// might block us waiting for buffers flush
		// the _qMutex...
		// We've done using the stream for this tag parsing anyway
		streamLock.unlock();
		pushEncodedAudioFrame(frame);
	}
	else if (tag[0] == FLV_VIDEO_TAG)
	{
		if ( ! _video ) {
			log_error(_("Unexpected video tag found at offset %d of FLV stream advertising no video in header. We'll warn only once per FLV, expecting any further video tag."), thisTagPos);
			_video = true; // TOCHECK: is this safe ?
		}
		// 1:keyframe, 2:interlacedFrame, 3:disposableInterlacedFrame
		int frameType = (tag[11] & 0xf0) >> 4;

		boost::uint16_t codec = (tag[11] & 0x0f) >> 0;

		if (codec == VIDEO_CODEC_VP6 || codec == VIDEO_CODEC_VP6A)
		{
			_stream->read_byte();
			--bodyLength;
		}

		bool header = false;

		if (codec == VIDEO_CODEC_H264) {
			boost::uint8_t packettype = _stream->read_byte();
			IF_VERBOSE_PARSE( log_debug(_("AVC packet type: %d"), (unsigned)packettype) );

			header = (packettype == 0);

			// 24-bits value for composition time offset ignored for now.
			boost::uint8_t tmp[3];
			_stream->read(tmp, 3);

			bodyLength -= 4;
		}
		
		if ( doIndex )
		{
			bool isKeyFrame = (frameType == 1);
			if ( isKeyFrame )
			{
				log_debug("Added cue point at timestamp %d and position %d (key video frame)", timestamp, thisTagPos);
				_cuePoints[timestamp] = thisTagPos;
			}
		}

		size_t dataPosition = _stream->tell();

		std::auto_ptr<EncodedVideoFrame> frame = readVideoFrame(bodyLength-1, timestamp);
		if ( ! frame.get() )
		{
			log_error("could not read video frame?");
			return true;
		}

		// If this is the first videoframe no info about the
		// video format has been noted, so we do that now
		if ( ! _videoInfo.get() )
		{
			_videoInfo.reset( new VideoInfo(codec, 0 /* width */, 0 /* height */, 0 /*frameRate*/, 0 /*duration*/, FLASH /*codec type*/) );

			if (header) {
				boost::uint8_t* newbuf = new boost::uint8_t[frame->dataSize()];
				memcpy(newbuf, frame->data(), frame->dataSize());

				_videoInfo->extra.reset( 
					new ExtraVideoInfoFlv(newbuf, frame->dataSize())
				);
				// Don't bother emitting the header buffer.
				return true;
			}
		}

		// Release the stream lock 
		// *before* pushing the frame as that 
		// might block us waiting for buffers flush
		// the _qMutex...
		streamLock.unlock();
		pushEncodedVideoFrame(frame);

	}
	else if (tag[0] == FLV_META_TAG)
	{
		if ( tag[11] != 2 )
		{
			// ::processTags relies on the first AMF0 value being a string...
			log_unimpl(_("First byte of FLV_META_TAG is %d, expected 0x02 (STRING AMF0 type)"),
				(int)tag[11]);
		}
		// Extract information from the meta tag
		std::auto_ptr<SimpleBuffer> metaTag(new SimpleBuffer(bodyLength-1));
		size_t actuallyRead = _stream->read(metaTag->data(), bodyLength-1);
		if ( actuallyRead < bodyLength-1 )
		{
			log_error("FLVParser::parseNextTag: can't read metaTag (%d) body (needed %d bytes, only got %d)",
				FLV_META_TAG, bodyLength, actuallyRead);
			return false;
		}
		metaTag->resize(actuallyRead);

		boost::mutex::scoped_lock lock(_metaTagsMutex);
		_metaTags.push_back(new MetaTag(timestamp, metaTag));
	}
	else
	{
		log_error("FLVParser::parseNextTag: unknown FLV tag type %d", (int)tag[0]);
		return false;
	}

	return true;
}

// would be called by MAIN thread
bool FLVParser::parseHeader()
{
	// seek to the begining of the file
	_stream->seek(0); // seek back ? really ?

	// Read the header
	boost::uint8_t header[9];
	if ( _stream->read(header, 9) != 9 )
	{
		log_error("FLVParser::parseHeader: couldn't read 9 bytes of header");
		return false;
	}

	_lastParsedPosition = _bytesLoaded = _nextPosToIndex = 9;

	// Check if this is really a FLV file
	if (header[0] != 'F' || header[1] != 'L' || header[2] != 'V') return false;

	int version = header[3];

	// Parse the audio+video bitmask
	_audio = header[4]&(1<<2);
	_video = header[4]&(1<<0);

	log_debug("Parsing FLV version %d, audio:%d, video:%d",
			version, _audio, _video);

	// Initialize audio/video info if any audio/video
	// tag was found within the first 'probeBytes' bytes
	// 
	const size_t probeBytes = PROBE_BYTES;
	while ( !_parsingComplete && _lastParsedPosition < probeBytes )
	{
		parseNextTag();

		// Early-out if we found both video and audio tags
		if ( _videoInfo.get() && _audioInfo.get() ) break;
	}

	// The audio+video bitmask advertised video but we 
	// found no video tag in the probe segment
	if ( _video && !_videoInfo.get() ) {
		log_error(_("Couldn't find any video frame in the first %d bytes "
			"of FLV advertising video in header"), probeBytes);
		_video = false;
	}

	// The audio+video bitmask advertised audio but we 
	// found no audio tag in the probe segment
	// (Youtube does this on some (maybe all) of their soundless clips.)
	if ( _audio && !_audioInfo.get() ) {
		log_error(_("Couldn't find any audio frame in the first %d bytes "
			"of FLV advertising audio in header"), probeBytes);
		_audio = false;
	}

	return true;
}

inline boost::uint32_t FLVParser::getUInt24(boost::uint8_t* in)
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
	//log_debug("Reading the %dth audio frame, with data size %d, from position %d", _audioFrames.size()+1, dataSize, _stream->tell());

	std::auto_ptr<EncodedAudioFrame> frame ( new EncodedAudioFrame );
	frame->dataSize = dataSize;
	frame->timestamp = timestamp;

	unsigned long int chunkSize = smallestMultipleContaining(READ_CHUNKS, dataSize+PADDING_BYTES);

	frame->data.reset( new boost::uint8_t[chunkSize] );
	size_t bytesread = _stream->read(frame->data.get(), dataSize);
	if ( bytesread < dataSize )
	{
		log_error("FLVParser::readAudioFrame: could only read %d/%d bytes", bytesread, dataSize);
	}

	unsigned long int padding = chunkSize-dataSize;
	assert(padding);
	memset(frame->data.get() + bytesread, 0, padding);

	return frame;
}

// would be called by parser thread
/*private*/
std::auto_ptr<EncodedVideoFrame>
FLVParser::readVideoFrame(boost::uint32_t dataSize, boost::uint32_t timestamp)
{
	std::auto_ptr<EncodedVideoFrame> frame;

	unsigned long int chunkSize = smallestMultipleContaining(READ_CHUNKS, dataSize+PADDING_BYTES);

	boost::uint8_t* data = new boost::uint8_t[chunkSize];
	size_t bytesread = _stream->read(data, dataSize);

	unsigned long int padding = chunkSize-dataSize;
	assert(padding);
	memset(data + bytesread, 0, padding);

	// We won't need frameNum, so will set to zero...
	// TODO: fix this ?
	// NOTE: ownership of 'data' is transferred here

	frame.reset( new EncodedVideoFrame(data, dataSize, 0, timestamp) );
	return frame;
}

void
FLVParser::processTags(boost::uint64_t ts, as_object* thisPtr, VM& vm)
{
	boost::mutex::scoped_lock lock(_metaTagsMutex);
	while (!_metaTags.empty())
	{
		if ( _metaTags.front()->timestamp() > ts ) break;

		std::auto_ptr<MetaTag> tag ( _metaTags.front() );
		_metaTags.pop_front();
		tag->execute(thisPtr, vm);
		
	}
}

void
FLVParser::MetaTag::execute(as_object* thisPtr, VM& vm)
{
	boost::uint8_t* ptr = _buffer->data();
	boost::uint8_t* endptr = ptr+_buffer->size();

	//log_debug("FLV meta: %s", hexify(ptr, 32, 0));
	//log_debug("FLV meta: %s", hexify(ptr, 32, 1));

	if ( ptr + 2 > endptr ) {
		log_error("Premature end of AMF in FLV metatag");
		return;
	}
	boost::uint16_t length = ntohs((*(boost::uint16_t *)ptr) & 0xffff);
	ptr+=2;

	if ( ptr + length > endptr ) {
		log_error("Premature end of AMF in FLV metatag");
		return;
	}
	std::string funcName((char*)ptr, length); // TODO: check for OOB !
	ptr += length;

	log_debug("funcName: %s", funcName);

	string_table& st = vm.getStringTable();
	string_table::key funcKey = st.find(funcName);

	as_value arg;
    std::vector<as_object*> objRefs;
	if ( ! arg.readAMF0(ptr, endptr, -1, objRefs, vm) )
	{
		log_error("Could not convert FLV metatag to as_value, but will try passing it anyway. It's an %s", arg);
		//return;
	}

    log_debug("Calling %s(%s)", funcName, arg);
	thisPtr->callMethod(funcKey, arg);
}

} // end of gnash::media namespace
} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
