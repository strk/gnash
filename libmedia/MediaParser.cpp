// MediaParser.cpp:  Media file parser, for Gnash.
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


#include "MediaParser.h"
#include "log.h"

namespace gnash {
namespace media {

MediaParser::MediaParser(std::auto_ptr<IOChannel> stream)
	:
	_isAudioMp3(false),
	_isAudioNellymoser(false),
	_stream(stream),
	//_stream(new LoadThread),
	_parsingComplete(false)
{
	//_stream->setStream(stream);
}

boost::uint64_t
MediaParser::getBufferLength() const
{
	bool hasVideo = _videoInfo.get();
	bool hasAudio = _audioInfo.get();

	//log_debug("MediaParser::getBufferLength: %d video %d audio frames", _videoFrames.size(), _audioFrames.size());

	if ( hasVideo && hasAudio )
	{
		return std::min(audioBufferLength(), videoBufferLength());
	}
	else if ( hasVideo )
	{
		return videoBufferLength();
	}
	else if ( hasAudio )
	{
		return audioBufferLength();
	}
	else return 0;
}

boost::uint64_t
MediaParser::videoBufferLength() const
{
	if (_videoFrames.empty()) return 0;
	//log_debug("videoBufferLength: first video frame has timestamp %d", _videoFrames.front()->timestamp());
	return _videoFrames.back()->timestamp() - _videoFrames.front()->timestamp(); 
}

boost::uint64_t
MediaParser::audioBufferLength() const
{
	if (_audioFrames.empty()) return 0;
	//log_debug("audioBufferLength: first audio frame has timestamp %d", _audioFrames.front()->timestamp);
	return _audioFrames.back()->timestamp - _audioFrames.front()->timestamp; 
}

const EncodedVideoFrame*
MediaParser::peekNextVideoFrame() const
{
	if (_videoFrames.empty())
	{
		log_debug("MediaParser::peekNextVideoFrame: no more video frames here...");
		return 0;
	}
	return _videoFrames.front();
}

bool
MediaParser::nextVideoFrameTimestamp(boost::uint64_t& ts) const
{
	const EncodedVideoFrame* ef = peekNextVideoFrame();
	if ( ! ef ) return false;
	ts = ef->timestamp();
	return true;
}

std::auto_ptr<EncodedVideoFrame>
MediaParser::nextVideoFrame()
{
	std::auto_ptr<EncodedVideoFrame> ret;
	if (_videoFrames.empty()) return ret;
	ret.reset(_videoFrames.front());
	_videoFrames.pop_front();
	return ret;
}

std::auto_ptr<EncodedAudioFrame>
MediaParser::nextAudioFrame()
{
	std::auto_ptr<EncodedAudioFrame> ret;
	if (_audioFrames.empty()) return ret;
	ret.reset(_audioFrames.front());
	_audioFrames.pop_front();
	return ret;
}

bool
MediaParser::nextAudioFrameTimestamp(boost::uint64_t& ts) const
{
	const EncodedAudioFrame* ef = peekNextAudioFrame();
	if ( ! ef ) return false;
	ts = ef->timestamp;
	return true;
}

const EncodedAudioFrame*
MediaParser::peekNextAudioFrame() const
{
	if (!_audioInfo.get() || _audioFrames.empty()) return 0;
	return _audioFrames.front();
}

MediaParser::~MediaParser()
{
	for (VideoFrames::iterator i=_videoFrames.begin(),
		e=_videoFrames.end(); i!=e; ++i)
	{
		delete (*i);
	}

	for (AudioFrames::iterator i=_audioFrames.begin(),
		e=_audioFrames.end(); i!=e; ++i)
	{
		delete (*i);
	}
}

} // end of gnash::media namespace
} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
