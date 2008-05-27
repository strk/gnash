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


#include <string>
#include <iosfwd>
#include "FLVParser.h"
#include "amf.h"
#include "log.h"
#include "utility.h"

using namespace std;

#define PADDING_BYTES 64
#define READ_CHUNKS 64

// Define the following macro the have seek() operations printed
//#define GNASH_DEBUG_SEEK 1

namespace gnash {

static std::auto_ptr<FLVFrame>
makeVideoFrame(tu_file& in, const FLVVideoFrameInfo& frameInfo)
{
	std::auto_ptr<FLVFrame> frame ( new FLVFrame );

	frame->dataSize = frameInfo.dataSize;
	frame->timestamp = frameInfo.timestamp;
	frame->type = videoFrame;

	if ( in.set_position(frameInfo.dataPosition) )
	{
		log_error(_("Failed seeking to videoframe in FLV input"));
		frame.reset();
		return frame;
	}

	unsigned long int dataSize = frameInfo.dataSize;
	unsigned long int chunkSize = smallestMultipleContaining(READ_CHUNKS, dataSize+PADDING_BYTES);

	frame->data = new boost::uint8_t[chunkSize];
	size_t bytesread = in.read_bytes(frame->data, dataSize);

	unsigned long int padding = chunkSize-dataSize;
	assert(padding);
	memset(frame->data + bytesread, 0, padding);

	return frame;
}

static std::auto_ptr<FLVFrame>
makeAudioFrame(tu_file& in, const FLVAudioFrameInfo& frameInfo)
{
	std::auto_ptr<FLVFrame> frame ( new FLVFrame );
	frame->dataSize = frameInfo.dataSize; 
	frame->timestamp = frameInfo.timestamp;
	frame->type = audioFrame;


	if ( in.set_position(frameInfo.dataPosition) )
	{
		log_error(_("Failed seeking to audioframe in FLV input"));
		frame.reset();
		return frame;
	}

	unsigned long int dataSize = frameInfo.dataSize;
	unsigned long int chunkSize = smallestMultipleContaining(READ_CHUNKS, dataSize+PADDING_BYTES);

	frame->data = new boost::uint8_t[chunkSize];
	size_t bytesread = in.read_bytes(frame->data, dataSize);

	unsigned long int padding = chunkSize-dataSize;
	assert(padding);
	memset(frame->data + bytesread, 0, padding);

	return frame;
}

FLVParser::FLVParser(std::auto_ptr<tu_file> lt)
	:
	_lt(lt),
	_lastParsedPosition(0),
	_parsingComplete(false),
	_videoInfo(NULL),
	_audioInfo(NULL),
	_nextAudioFrame(0),
	_nextVideoFrame(0),
	_audio(false),
	_video(false)
{
}

FLVParser::~FLVParser()
{
	_videoFrames.clear();

	_audioFrames.clear();
}


boost::uint32_t
FLVParser::getBufferLength()
{
	if (_video) {
		size_t size = _videoFrames.size();
		if (size > 1 && size > _nextVideoFrame) {
			return _videoFrames.back()->timestamp; //  - _videoFrames[_nextVideoFrame]->timestamp;
		}
	}
	if (_audio) {
		size_t size = _audioFrames.size();
		if (size > 1 && size > _nextAudioFrame) {
			return _audioFrames.back()->timestamp; //  - _audioFrames[_nextAudioFrame]->timestamp;
		}
	}
	return 0;
}
boost::uint16_t
FLVParser::videoFrameRate()
{
	// Make sure that there are parsed some frames
	while(_videoFrames.size() < 2 && !_parsingComplete) {
		parseNextTag();
	}

	if (_videoFrames.size() < 2) return 0;

 	boost::uint32_t framedelay = _videoFrames[1]->timestamp - _videoFrames[0]->timestamp;

	return static_cast<boost::int16_t>(1000 / framedelay);
}


boost::uint32_t
FLVParser::videoFrameDelay()
{
	// If there are no video in this FLV return 0
	if (!_video && _lastParsedPosition > 0) return 0;

	// Make sure that there are parsed some frames
	while(_videoFrames.size() < 2 && !_parsingComplete) {
		parseNextTag();
	}

	// If there is no video data return 0
	if (_videoFrames.size() == 0 || !_video || _nextVideoFrame < 2) return 0;

	return _videoFrames[_nextVideoFrame-1]->timestamp - _videoFrames[_nextVideoFrame-2]->timestamp;
}

boost::uint32_t
FLVParser::audioFrameDelay()
{
	// If there are no audio in this FLV return 0
	if (!_audio && _lastParsedPosition > 0) return 0;

	// Make sure that there are parsed some frames
	while(_audioFrames.size() < 2 && !_parsingComplete) {
		parseNextTag();
	}

	// If there is no video data return 0
	if (_audioFrames.size() == 0 || !_audio || _nextAudioFrame < 2) return 0;

	return _audioFrames[_nextAudioFrame-1]->timestamp - _audioFrames[_nextAudioFrame-2]->timestamp;
}

FLVAudioFrameInfo*
FLVParser::peekNextAudioFrameInfo()
{
	// If there are no audio in this FLV return NULL
	if (!_audio && _lastParsedPosition > 0) return 0;

	// Make sure that there are parsed enough frames to return the need frame
	while(_audioFrames.size() <= _nextAudioFrame && !_parsingComplete) {
		if (!parseNextTag()) break;
	}

	// If the needed frame can't be parsed (EOF reached) return NULL
	if (_audioFrames.empty() || _audioFrames.size() <= _nextAudioFrame)
	{
		return 0;	
	}

	return _audioFrames[_nextAudioFrame];
}

FLVFrame*
FLVParser::nextAudioFrame()
{
	FLVAudioFrameInfo* frameInfo = peekNextAudioFrameInfo();
	if ( ! frameInfo ) return 0;

	std::auto_ptr<FLVFrame> frame = makeAudioFrame(*_lt, *frameInfo);
	if ( ! frame.get() )
	{
		log_error("Could not make audio frame %d", _nextAudioFrame);
		return 0;
	}

	_nextAudioFrame++;
	return frame.release(); // TODO: return by auto_ptr

}

FLVVideoFrameInfo*
 FLVParser::peekNextVideoFrameInfo()
{
	// If there are no video in this FLV return NULL
	if (!_video && _lastParsedPosition > 0)
	{
		//gnash::log_debug("no video, or lastParserPosition > 0");
		return 0;
	}

	// Make sure that there are parsed enough frames to return the need frame
	while(_videoFrames.size() <= static_cast<boost::uint32_t>(_nextVideoFrame) && !_parsingComplete)
	{
		if (!parseNextTag()) break;
	}

	// If the needed frame can't be parsed (EOF reached) return NULL
	if (_videoFrames.empty() || _videoFrames.size() <= _nextVideoFrame)
	{
		//gnash::log_debug("The needed frame (%d) can't be parsed (EOF reached)", _lastVideoFrame);
		return 0;
	}

	return _videoFrames[_nextVideoFrame];
}

FLVFrame* FLVParser::nextVideoFrame()
{
	FLVVideoFrameInfo* frameInfo = peekNextVideoFrameInfo();
	std::auto_ptr<FLVFrame> frame = makeVideoFrame(*_lt, *frameInfo);
	if ( ! frame.get() )
	{
		log_error("Could not make video frame %d", _nextVideoFrame);
		return 0;
	}

	_nextVideoFrame++;
	return frame.release(); // TODO: return by auto_ptr
}


boost::uint32_t
FLVParser::seekAudio(boost::uint32_t time)
{

	// If there is no audio data return NULL
	if (_audioFrames.empty()) return 0;

	// If there are no audio greater than the given time
	// the last audioframe is returned
	FLVAudioFrameInfo* lastFrame = _audioFrames.back();
	if (lastFrame->timestamp < time) {
		_nextAudioFrame = _audioFrames.size() - 1;
		return lastFrame->timestamp;
	}

	// We try to guess where in the vector the audioframe
	// with the correct timestamp is
	size_t numFrames = _audioFrames.size();
	double tpf = lastFrame->timestamp / numFrames; // time per frame
	size_t guess = size_t(time / tpf);

	// Here we test if the guess was ok, and adjust if needed.
	size_t bestFrame = utility::clamp<size_t>(guess, 0, _audioFrames.size()-1);

	// Here we test if the guess was ok, and adjust if needed.
	long diff = _audioFrames[bestFrame]->timestamp - time;
	if ( diff > 0 ) // our guess was too long
	{
		while ( bestFrame > 0 && _audioFrames[bestFrame-1]->timestamp > time ) --bestFrame;
	}
	else // our guess was too short
	{
		while ( bestFrame < _audioFrames.size()-1 && _audioFrames[bestFrame+1]->timestamp < time ) ++bestFrame;
	}

#ifdef GNASH_DEBUG_SEEK
	gnash::log_debug("Seek (audio): " SIZET_FMT "/" SIZET_FMT " (%u/%u)", bestFrame, numFrames, _audioFrames[bestFrame]->timestamp, time);
#endif
	_nextAudioFrame = bestFrame;
	return _audioFrames[bestFrame]->timestamp;

}


boost::uint32_t
FLVParser::seekVideo(boost::uint32_t time)
{
	if ( _videoFrames.empty() ) return 0;

	// If there are no videoframe greater than the given time
	// the last key videoframe is returned
	FLVVideoFrameInfo* lastFrame = _videoFrames.back();
	size_t numFrames = _videoFrames.size();
	if (lastFrame->timestamp < time)
	{
		size_t lastFrameNum = numFrames -1;
		while (! lastFrame->isKeyFrame() )
		{
			lastFrameNum--;
			lastFrame = _videoFrames[lastFrameNum];
		}

		_nextVideoFrame = lastFrameNum;
		return lastFrame->timestamp;

	}

	// We try to guess where in the vector the videoframe
	// with the correct timestamp is
	double tpf = lastFrame->timestamp / numFrames; // time per frame
	size_t guess = size_t(time / tpf);

	size_t bestFrame = utility::clamp<size_t>(guess, 0, _videoFrames.size()-1);

	// Here we test if the guess was ok, and adjust if needed.
	long diff = _videoFrames[bestFrame]->timestamp - time;
	if ( diff > 0 ) // our guess was too long
	{
		while ( bestFrame > 0 && _videoFrames[bestFrame-1]->timestamp > time ) --bestFrame;
	}
	else // our guess was too short
	{
		while ( bestFrame < _videoFrames.size()-1 && _videoFrames[bestFrame+1]->timestamp < time ) ++bestFrame;
	}

	// Find closest backward keyframe  
	size_t rewindKeyframe = bestFrame;
	while ( rewindKeyframe && ! _videoFrames[rewindKeyframe]->isKeyFrame() )
	{
		rewindKeyframe--;
	}

	// Find closest forward keyframe 
	size_t forwardKeyframe = bestFrame;
	size_t size = _videoFrames.size();
	while (size > forwardKeyframe+1 && ! _videoFrames[forwardKeyframe]->isKeyFrame() )
	{
		forwardKeyframe++;
	}

	// We can't ensure we were able to find a key frame *after* the best position
	// in that case we just use any previous keyframe instead..
	if ( ! _videoFrames[forwardKeyframe]->isKeyFrame() )
	{
		bestFrame = rewindKeyframe;
	}
	else
	{
		boost::int32_t forwardDiff = _videoFrames[forwardKeyframe]->timestamp - time;
		boost::int32_t rewindDiff = time - _videoFrames[rewindKeyframe]->timestamp;

		if (forwardDiff < rewindDiff) bestFrame = forwardKeyframe;
		else bestFrame = rewindKeyframe;
	}

#ifdef GNASH_DEBUG_SEEK
	gnash::log_debug("Seek (video): " SIZET_FMT "/" SIZET_FMT " (%u/%u)", bestFrame, numFrames, _videoFrames[bestFrame]->timestamp, time);
#endif

	_nextVideoFrame = bestFrame;
	assert( _videoFrames[bestFrame]->isKeyFrame() );
	return _videoFrames[bestFrame]->timestamp;
}



FLVVideoInfo* FLVParser::getVideoInfo()
{
	// If there are no video in this FLV return NULL
	if (!_video && _lastParsedPosition > 0) return NULL;

	// Make sure that there are parsed some video frames
	while( ! _parsingComplete && !_videoInfo.get() ) parseNextTag();

	return _videoInfo.get(); // may be null
}

FLVAudioInfo* FLVParser::getAudioInfo()
{
	// If there are no audio in this FLV return NULL
	if (!_audio && _lastParsedPosition > 0) return NULL;

	// Make sure that there are parsed some audio frames
	while (!_parsingComplete && ! _audioInfo.get() )
	{
		parseNextTag();
	}

	return _audioInfo.get(); // may be null
}

bool
FLVParser::isTimeLoaded(boost::uint32_t time)
{
	// Parse frames until the need time is found, or EOF
	while (!_parsingComplete) {
		if (!parseNextTag()) break;
		if ((_videoFrames.size() > 0 && _videoFrames.back()->timestamp >= time)
			|| (_audioFrames.size() > 0 && _audioFrames.back()->timestamp >= time)) {
			return true;
		}
	}

	if (_videoFrames.size() > 0 && _videoFrames.back()->timestamp >= time) {
		return true;
	}

	if (_audioFrames.size() > 0 && _audioFrames.back()->timestamp >= time) {
		return true;
	}

	return false;

}

boost::uint32_t
FLVParser::seek(boost::uint32_t time)
{
	GNASH_REPORT_FUNCTION;

	log_debug("FLVParser::seek(%d) ", time);

	if (time == 0) {
		if (_video) _nextVideoFrame = 0;
		if (_audio) _nextAudioFrame = 0;
	}

	// Video, if present, has more constraints
	// as to where we allow seeking (we only
	// allow seek to closest *key* frame).
	// So we first have video seeking tell us
	// what time is best for that, and next
	// we seek audio on that time

	if (_video)
	{
		time = seekVideo(time);
#ifdef GNASH_DEBUG_SEEK
		log_debug("  seekVideo -> %d", time);
#endif
	}

	if (_audio)
	{
		time = seekAudio(time);
#ifdef GNASH_DEBUG_SEEK
		log_debug("  seekAudio -> %d", time);
#endif
	}

	return time;
}

bool FLVParser::parseNextTag()
{
	if ( _parsingComplete ) return false;

	// Parse the header if not done already. If unsuccesfull return false.
	if (_lastParsedPosition == 0 && !parseHeader()) return false;

	// Seek to next frame and skip the size of the last tag
	if ( _lt->set_position(_lastParsedPosition+4) )
	{
		log_error("FLVParser::parseNextTag: can't seek to %d", _lastParsedPosition+4);
		_parsingComplete=true;
		return false;
	}

	// Read the tag info
	boost::uint8_t tag[12];
	int actuallyRead = _lt->read_bytes(tag, 12);
	if ( actuallyRead < 12 )
	{
		if ( actuallyRead )
			log_error("FLVParser::parseNextTag: can't read tag info (needed 12 bytes, only got %d)", actuallyRead);
		// else { assert(_lt->get_eof(); } ?
		_parsingComplete=true;
		return false;
	}

	// Extract length and timestamp
	boost::uint32_t bodyLength = getUInt24(&tag[1]);
	boost::uint32_t timestamp = getUInt24(&tag[4]);

	_lastParsedPosition += 15 + bodyLength;

	// check for empty tag
	if (bodyLength == 0) return true;

	if (tag[0] == AUDIO_TAG)
	{
		FLVAudioFrameInfo* frame = new FLVAudioFrameInfo;
		frame->dataSize = bodyLength - 1;
		frame->timestamp = timestamp;
		frame->dataPosition = _lt->get_position();
		_audioFrames.push_back(frame);

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

			_audioInfo.reset( new FLVAudioInfo((tag[11] & 0xf0) >> 4, samplerate, samplesize, (tag[11] & 0x01) >> 0, 0) );
		}


	}
	else if (tag[0] == VIDEO_TAG)
	{
		FLVVideoFrameInfo* frame = new FLVVideoFrameInfo;
		frame->dataSize = bodyLength - 1;
		frame->timestamp = timestamp;
		frame->dataPosition = _lt->get_position();
		frame->frameType = (tag[11] & 0xf0) >> 4;
		_videoFrames.push_back(frame);

		// If this is the first videoframe no info about the
		// video format has been noted, so we do that now
		if ( ! _videoInfo.get() )
		{
			boost::uint16_t codec = (tag[11] & 0x0f) >> 0;
			// Set standard guessed size...
			boost::uint16_t width = 320;
			boost::uint16_t height = 240;

			// Extract the video size from the videodata header
			if (codec == VIDEO_CODEC_H263) {
				if ( _lt->set_position(frame->dataPosition) )
				{
					log_error(" Couldn't seek to VideoTag data position");
					_parsingComplete=true;
					return false;
				}
				boost::uint8_t videohead[12];
				int actuallyRead = _lt->read_bytes(videohead, 12);
				if ( actuallyRead < 12 )
				{
		log_error("FLVParser::parseNextTag: can't read H263 video header (needed 12 bytes, only got %d)", actuallyRead);
		_parsingComplete=true;
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
			_videoInfo.reset( new FLVVideoInfo(codec, width, height, 0 /*frameRate*/, 0 /*duration*/) );
		}

	} else if (tag[0] == META_TAG) {
		LOG_ONCE( log_unimpl("FLV MetaTag parser") );
		// Extract information from the meta tag
		/*_lt->seek(_lastParsedPosition+16);
		char* metaTag = new char[bodyLength];
		size_t actuallyRead = _lt->read(metaTag, bodyLength);
		if ( actuallyRead < bodyLength )
		{
			log_error("FLVParser::parseNextTag: can't read metaTag (%d) body (needed %d bytes, only got %d)",
				META_TAG, bodyLength, actuallyRead);
			_parsingComplete=true;
			return false;
		}
		amf::AMF* amfParser = new amf::AMF();
		amfParser->parseAMF(metaTag);*/

	} else {
		log_error("Unknown FLV tag type %d", tag[0]);
		//_parsingComplete = true;
		//return false;
	}

	return true;
}

bool FLVParser::parseHeader()
{
	// seek to the begining of the file
	_lt->set_position(0); // seek back ? really ?

	// Read the header
	boost::uint8_t header[9];
	if ( _lt->read_bytes(header, 9) != 9 )
	{
		log_error("FLVParser::parseHeader: couldn't read 9 bytes of header");
		return false;
	}

	// Check if this is really a FLV file
	if (header[0] != 'F' || header[1] != 'L' || header[2] != 'V') return false;

	int version = header[3];

	// Parse the audio+video bitmask
	_audio = header[4]&(1<<2);
	_video = header[4]&(1<<0);

	log_debug("Parsing FLV version %d, audio:%d, video:%d", version, _audio, _video);

	_lastParsedPosition = 9;
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
	return _lastParsedPosition;
}

boost::uint64_t
FLVParser::getBytesTotal() const
{
	return _lt->get_size();
}

FLVFrame* FLVParser::nextMediaFrame()
{
	boost::uint32_t video_size = _videoFrames.size();
	boost::uint32_t audio_size = _audioFrames.size();

	if (audio_size <= _nextAudioFrame && video_size <= _nextVideoFrame)
	{

		// Parse a media frame if any left or if needed
		while(_videoFrames.size() <= _nextVideoFrame && _audioFrames.size() <= _nextAudioFrame && !_parsingComplete) {
			if (!parseNextTag()) break;
		}
	}

	// Find the next frame in the file
	bool audioReady = _audioFrames.size() > _nextAudioFrame;
	bool videoReady = _videoFrames.size() > _nextVideoFrame;
	bool useAudio = false;

	if (audioReady && videoReady) {
		useAudio = _audioFrames[_nextAudioFrame]->dataPosition < _videoFrames[_nextVideoFrame]->dataPosition;
	} else if (!audioReady && videoReady) {
		useAudio = false;
	} else if (audioReady && !videoReady) {
		useAudio = true;
	} else {
		// If no frames are next we have reached EOF
		return NULL;
	}

	// Find the next frame in the file a return it

	if (useAudio) {

		FLVAudioFrameInfo* frameInfo = _audioFrames[_nextAudioFrame];

		std::auto_ptr<FLVFrame> frame = makeAudioFrame(*_lt, *frameInfo);
		if ( ! frame.get() )
		{
			log_error("Could not make audio frame %d", _nextAudioFrame);
			return 0;
		}

		_nextAudioFrame++;
		return frame.release(); // TODO: return by auto_ptr

	} else {

		FLVVideoFrameInfo* frameInfo = _videoFrames[_nextVideoFrame];
		std::auto_ptr<FLVFrame> frame = makeVideoFrame(*_lt, *frameInfo);
		if ( ! frame.get() )
		{
			log_error("Could not make video frame %d", _nextVideoFrame);
			return 0;
		}

		_nextVideoFrame++;
		return frame.release(); // TODO: return by auto_ptr
	}


}

} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
