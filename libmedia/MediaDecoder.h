// MediaDecoder.h: Media decoding base class.
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

// $Id: MediaDecoder.h,v 1.7 2008/01/21 23:10:14 rsavoye Exp $

#ifndef __MEDIADECODER_H__
#define __MEDIADECODER_H__

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>
#include <vector> 

#include "MediaBuffer.h"
#include "MediaParser.h"
#include "FLVParser.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "log.h"

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

namespace gnash {
namespace media {

/// Status codes used for NetStream onStatus notifications
enum StatusCode {

	// Internal status, not a valid ActionScript value
	invalidStatus,

	/// NetStream.Buffer.Empty (level: status)
	bufferEmpty,

	/// NetStream.Buffer.Full (level: status)
	bufferFull,

	/// NetStream.Buffer.Flush (level: status)
	bufferFlush,

	/// NetStream.Play.Start (level: status)
	playStart,

	/// NetStream.Play.Stop  (level: status)
	playStop,

	/// NetStream.Seek.Notify  (level: status)
	seekNotify,

	/// NetStream.Play.StreamNotFound (level: error)
	streamNotFound,

	/// NetStream.Seek.InvalidTime (level: error)
	invalidTime
};

/// \brief
/// The MediaDecoder class decodes media data and puts it on a given buffer.
///
/// We need to be able to handle event stuff, so we store the events that
/// need to be handled, and then NetStream can ask for it on ::advance()
/// or whenever it is appropriate. Eventhandling is different from AS2 to AS3,
/// but in this first draft only AS2 will be supported.
///
class MediaDecoder
{

public:
	/// This is copied from the render and should be changed if the original is.
	enum videoOutputFormat
	{
		NONE,
		YUV,
		RGB
	};

	/// Internal error codes
	enum MediaDecoderErrorCode {
	
		/// All is fine
		noError,

		/// Stream/connection error
		streamError,

		/// Error while decoding
		decodingError,

		/// Error while parsing
		parsingError
	};

	/// \brief
	/// Create a MediaDecoder reading input from
	/// the given tu_file
	//
	/// @param stream
	/// 	tu_file to use for input.
	/// 	Ownership left to the caller.
	///
	/// @param buffer
	/// 	The buffer we will use.
	/// 	Ownership left to the caller.
	///
	MediaDecoder(boost::shared_ptr<tu_file> stream, MediaBuffer* buffer, boost::uint16_t swfVersion, int format)
		:
	_buffer(buffer),
	_stream(stream),
	_swfVersion(swfVersion),
	_videoFrameFormat(format),
	_parser(NULL),
	_lastConfirmedPosition(0),
	_streamSize(0),
	_error(noError),
	_isFLV(true),
	_inputPos(0),
	_audio(false),
	_video(false),
	_running(true),
	_audioDecoder(NULL),
	_videoDecoder(NULL),
	_decodeThread(NULL)
	{
	}

	/// Destroys the Decoder
#if !defined(sgi) || defined(__GNUC__)
	virtual ~MediaDecoder() {}
#endif
	/// Seeks to pos, returns the new position
	virtual boost::uint32_t seek(boost::uint32_t /*pos*/) { return 0;}

	virtual std::pair<boost::uint32_t, boost::uint32_t> getWidthAndHeight() { return std::pair<boost::uint32_t, boost::uint32_t>(0,0); }

	/// Returns the size of the file being loaded, in bytes
	boost::uint32_t getBytesTotal()
	{
		return _streamSize;
	}

	/// Returns the amount of bytes of the current file that has been loaded.
	boost::uint32_t getBytesLoaded() {
		return _lastConfirmedPosition;
	}

	/// Returns a vector with the waiting onStatus events (AS2)
	std::vector<StatusCode> getOnStatusEvents();

	/// Returns whether we got audio
	bool gotAudio() {
		return _audio;
	}

	/// Returns whether we got video
	bool gotVideo() {
		return _video;
	}

	/// Used to wake up the decoder when it is needed
	void wakeUp()
	{
		boost::mutex::scoped_lock lock(_monitor);
		_nothingToDo.notify_all();
	}

protected:

	/// Decodes data with the decoders that has been setup and pushes the data
	/// unto the buffer. Returns when decoding stops.
	void decodingLoop();

	/// Decodes a frame and push it unto the buffer
	bool decodeAndBufferFrame();

	// Used to decode a video frame and push it on the videoqueue
	void decodeVideo(MediaFrame* packet);

	// Used to decode a audio frame and push it on the audioqueue
	void decodeAudio(MediaFrame* packet);

	/// Push an event to the onStatus event queue (AS2)
	void pushOnStatus(StatusCode code) {
		boost::mutex::scoped_lock lock(_onStatusMutex);
		_onStatusQueue.push_back(code);	
	}

	/// used to wait for something to do
	void relax()
	{
		boost::mutex::scoped_lock lock(_monitor);
		_nothingToDo.wait(lock);
	}

	/// The media buffer
	MediaBuffer* _buffer;

	/// The stream we decode
	boost::shared_ptr<tu_file> _stream;

	/// Version of the SWF playing
	boost::uint16_t _swfVersion;
	
	/// The output format
	int _videoFrameFormat;

	/// The parser used
	std::auto_ptr<MediaParser> _parser;

	/// The last confirmed size of the downloaded file
	boost::uint32_t _lastConfirmedPosition;

	/// total size of the file being downloaded
	boost::uint32_t _streamSize;

	/// Is everything ok?
	MediaDecoderErrorCode _error;

	/// Waiting NetStream onStatus events (AS2)
	std::vector<StatusCode> _onStatusQueue;

	/// Mutex protecting _onStatusQueue
	boost::mutex _onStatusMutex;

	/// Are we decoding a FLV?
	bool _isFLV;

	/// The position in the inputfile, only used when not playing a FLV
	long _inputPos;

	/// Do we have audio ?
	bool _audio;

	/// Do we have video ?
	bool _video;

	/// Used to wait when there is nothing to do
	boost::condition _nothingToDo;
	boost::mutex _monitor;

	// Should the decode loop run or not
	volatile bool _running;
	
	// An audio decoder
	std::auto_ptr<AudioDecoder> _audioDecoder;

	// A video decoder
	std::auto_ptr<VideoDecoder> _videoDecoder;

	// The decoding thread
	boost::thread* _decodeThread;
};


} // gnash.media namespace 
} // namespace gnash

#endif // __MEDIADECODER_H__
