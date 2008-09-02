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

#include <string>
#include <iosfwd>

using namespace std;

#define PADDING_BYTES 64
#define READ_CHUNKS 64

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
	// nothing to do here, all done in base class now
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
		if ( doIndex && ! _video ) // if we have video we let that drive cue points
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

		std::auto_ptr<EncodedAudioFrame> frame = readAudioFrame(bodyLength-1, timestamp);
		if ( ! frame.get() ) { log_error("could not read audio frame?"); }
		else
		{
			// Release the stream lock 
			// *before* pushing the frame as that 
			// might block us waiting for buffers flush
			// the _qMutex...
			// We've done using the stream for this tag parsing anyway
			streamLock.unlock();
			pushEncodedAudioFrame(frame);
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

			_audioInfo.reset( new AudioInfo((tag[11] & 0xf0) >> 4, samplerate, samplesize, (tag[11] & 0x01) >> 0, 0, FLASH) );
		}

	}
	else if (tag[0] == FLV_VIDEO_TAG)
	{
		// 1:keyframe, 2:interlacedFrame, 3:disposableInterlacedFrame
		int frameType = (tag[11] & 0xf0) >> 4;

		boost::uint16_t codec = (tag[11] & 0x0f) >> 0;

        if (codec == VIDEO_CODEC_VP6 || codec == VIDEO_CODEC_VP6A)
        {
            _stream->read_byte();
            --bodyLength;
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
			// Set standard guessed size...
			boost::uint16_t width = 320;
			boost::uint16_t height = 240;

			// Extract the video size from the videodata header
			if (codec == VIDEO_CODEC_H263) {

				// We're going to re-read some data here
				// (can likely avoid with a better cleanup)

				size_t bkpos = _stream->tell();
				if ( _stream->seek(dataPosition) ) {
					log_error(" Couldn't seek to VideoTag data position -- should never happen, as we just read that!");
					_parsingComplete=true;
					_indexingCompleted=true;
					return false;
				}
				boost::uint8_t videohead[12];

				int actuallyRead = _stream->read(videohead, 12);
				_stream->seek(bkpos); // rewind

				if ( actuallyRead < 12 )
				{
		log_error("FLVParser::parseNextTag: can't read H263 video header (needed 12 bytes, only got %d)", actuallyRead);
		_parsingComplete=true;
		_indexingCompleted=true;
		return false;
				}

				bool sizebit1 = (videohead[3] & 0x02);
				bool sizebit2 = (videohead[3] & 0x01);
				bool sizebit3 = (videohead[4] & 0x80);

				// First some predefined sizes
				if (!sizebit1 && sizebit2 && !sizebit3 ) {
					width = 352;
					height = 288;
				} else if (!sizebit1 && sizebit2 && sizebit3 ) {
					width = 176;
					height = 144;
				} else if (sizebit1 && !sizebit2 && !sizebit3 ) {
					width = 128;
					height = 96;
				} else if (sizebit1 && !sizebit2 && sizebit3 ) {
					width = 320;
					height = 240;
				} else if (sizebit1 && sizebit2 && !sizebit3 ) {
					width = 160;
					height = 120;

				// Then the custom sizes (1 byte - untested and ugly)
				} else if (!sizebit1 && !sizebit2 && !sizebit3 ) {
					width = (videohead[4] & 0x40) | (videohead[4] & 0x20) | (videohead[4] & 0x20) | (videohead[4] & 0x08) | (videohead[4] & 0x04) | (videohead[4] & 0x02) | (videohead[4] & 0x01) | (videohead[5] & 0x80);

					height = (videohead[5] & 0x40) | (videohead[5] & 0x20) | (videohead[5] & 0x20) | (videohead[5] & 0x08) | (videohead[5] & 0x04) | (videohead[5] & 0x02) | (videohead[5] & 0x01) | (videohead[6] & 0x80);

				// Then the custom sizes (2 byte - untested and ugly)
				} else if (!sizebit1 && !sizebit2 && sizebit3 ) {
					width = (videohead[4] & 0x40) | (videohead[4] & 0x20) | (videohead[4] & 0x20) | (videohead[4] & 0x08) | (videohead[4] & 0x04) | (videohead[4] & 0x02) | (videohead[4] & 0x01) | (videohead[5] & 0x80) | (videohead[5] & 0x40) | (videohead[5] & 0x20) | (videohead[5] & 0x20) | (videohead[5] & 0x08) | (videohead[5] & 0x04) | (videohead[5] & 0x02) | (videohead[5] & 0x01) | (videohead[6] & 0x80);

					height = (videohead[6] & 0x40) | (videohead[6] & 0x20) | (videohead[6] & 0x20) | (videohead[6] & 0x08) | (videohead[6] & 0x04) | (videohead[6] & 0x02) | (videohead[6] & 0x01) | (videohead[7] & 0x80) | (videohead[7] & 0x40) | (videohead[7] & 0x20) | (videohead[7] & 0x20) | (videohead[7] & 0x08) | (videohead[7] & 0x04) | (videohead[7] & 0x02) | (videohead[7] & 0x01) | (videohead[8] & 0x80);
				}

			}

			// Create the videoinfo
			_videoInfo.reset( new VideoInfo(codec, width, height, 0 /*frameRate*/, 0 /*duration*/, FLASH /*codec type*/) );
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
		// Extract information from the meta tag
		boost::scoped_array<unsigned char> metaTag ( new unsigned char[bodyLength] );
		size_t actuallyRead = _stream->read(metaTag.get(), bodyLength);
		if ( actuallyRead < bodyLength )
		{
			log_error("FLVParser::parseNextTag: can't read metaTag (%d) body (needed %d bytes, only got %d)",
				FLV_META_TAG, bodyLength, actuallyRead);
			return false;
		}
                amf::Flv flv;
                std::auto_ptr<amf::Element> el ( flv.decodeMetaData(metaTag.get(), actuallyRead) );
		log_unimpl("FLV MetaTag handling. Data: %s", *el);
                //el->dump();
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

	log_debug("Parsing FLV version %d, audio:%d, video:%d", version, _audio, _video);

	// Make sure we initialize audio/video info (if any)
	while ( !_parsingComplete && (_video && !_videoInfo.get()) || (_audio && !_audioInfo.get()) )
	{
		parseNextTag();
	}

	if ( _video && !_videoInfo.get() )
		log_error(" couldn't find any video frame in an FLV advertising video in header");

	if ( _audio && !_audioInfo.get() )
		log_error(" couldn't find any audio frame in an FLV advertising audio in header");

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

} // end of gnash::media namespace
} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
