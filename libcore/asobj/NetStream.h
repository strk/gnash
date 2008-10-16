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


#ifndef GNASH_NETSTREAM_H
#define GNASH_NETSTREAM_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "impl.h"
#include "video_stream_instance.h"
#include "NetConnection.h"
#include "MediaParser.h"
#include "as_function.h" // for visibility of destructor by intrusive_ptr

#include "VideoDecoder.h" // for visibility of dtor
#include "AudioDecoder.h" // for visibility of dtor

#include "VirtualClock.h"




#include <deque>
#include <boost/scoped_ptr.hpp>

// Forward declarations
namespace gnash {
	class CharacterProxy;
	class IOChannel;
	namespace media {
		class sound_handler;
		class MediaHandler;
	}
}

namespace gnash {

/// The playback controller
class PlayHead {

public:

	/// Flags for playback state
	enum PlaybackStatus {
		PLAY_PLAYING = 1,
		PLAY_PAUSED = 2
	};
	

	/// Initialize playhead given a VirtualCock to use
	/// as clock source.
	//
	/// The PlayHead will have initial state set to PLAYING
	///
	/// @param clockSource
	///	The VirtualClock to use as time source.
	///	Ownership left to caller (not necessarely a good thing).
	///
	PlayHead(VirtualClock* clockSource);

	/// Initialize playhead 
	//
	/// @param hasVideo
	/// 	Whether video consumer is available
	///
	/// @param hasAudio
	/// 	Whether video consumer is available
	///
	void init(bool hasVideo, bool hasAudio);

	/// Get current playhead position (milliseconds)
	boost::uint64_t getPosition() { return _position; }

	/// Get current playback state
	PlaybackStatus getState() { return _state; }

	/// Set playback state, returning old state
	PlaybackStatus setState(PlaybackStatus newState);

	/// Toggle playback state, returning old state
	PlaybackStatus toggleState();

	/// Return true if video of current position have been consumed
	bool isVideoConsumed() const
	{
		return (_positionConsumers & CONSUMER_VIDEO);
	}

	/// \brief
	/// Mark current position as being consumed by video consumer,
	/// advancing if needed
	void setVideoConsumed()
	{
		_positionConsumers |= CONSUMER_VIDEO;
		advanceIfConsumed();
	}

	/// Return true if audio of current position have been consumed
	bool isAudioConsumed() const
	{
		return (_positionConsumers & CONSUMER_AUDIO);
	}

	/// \brief
	/// Mark current position as being consumed by audio consumer,
	/// advancing if needed.
	void setAudioConsumed()
	{
		_positionConsumers |= CONSUMER_AUDIO;
		advanceIfConsumed();
	}

	/// Change current position to the given time.
	//
	/// Consume flag will be reset.
	///
	/// @param position
	///	Position timestamp (milliseconds)
	///
	/// POSTCONDITIONS:
	///	- isVideoConsumed() == false
	///	- isAudioConsumed() == false
	///	- getPosition() == position
	///
	void seekTo(boost::uint64_t position);

private:

	/// Advance position if all consumers consumed the current one
	//
	/// Clock source will be used to determine the amount
	/// of milliseconds to advance position to.
	///
	/// Consumer flags will be reset.
	///
	/// POSTCONDITIONS:
	///	- isVideoConsumed() == false
	///	- isAudioConsumed() == false
	///
	void advanceIfConsumed();
		
	/// Flags for consumers state
	enum ConsumerFlag {
		CONSUMER_VIDEO = 1,
		CONSUMER_AUDIO = 2
	};

	/// Current playhead position
	boost::uint64_t _position;

	/// Current playback state
	PlaybackStatus _state;

	/// Binary OR of consumers representing
	/// which consumers are active
	int _availableConsumers;

	/// Binary OR of consumers representing
	/// which consumers consumed current position
	int _positionConsumers;

	/// The clock source, externally owned
	VirtualClock* _clockSource;

	/// Offset to subtract from current clock source
	/// to get current position
	//
	/// The offset will be 
	boost::uint64_t _clockOffset; 

};





class raw_mediadata_t
{
public:
        DSOEXPORT raw_mediadata_t()
		:
	        m_stream_index(-1),
        	m_size(0),
        	m_data(NULL),
        	m_ptr(NULL),
        	m_pts(0)
	{}

        DSOEXPORT ~raw_mediadata_t()
	{
		delete [] m_data;
	}
	

        int m_stream_index;
        boost::uint32_t m_size;
        boost::uint8_t* m_data;
        boost::uint8_t* m_ptr;
        boost::uint32_t m_pts;  // presentation timestamp in millisec
};





/// NetStream ActionScript class
//
/// This class is responsible for handlign external
/// media files. Provides interfaces for playback control.
///
class NetStream : public as_object {

protected:
	
	/// Status codes used for notifications
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
	

	boost::intrusive_ptr<NetConnection> _netCon;

	boost::scoped_ptr<CharacterProxy> _audioController;

	/// Set stream status.
	//
	/// Valid statuses are:
	///
	/// Status level:
	///  - NetStream.Buffer.Empty
	///  - NetStream.Buffer.Full
	///  - NetStream.Buffer.Flush
	///  - NetStream.Play.Start
	///  - NetStream.Play.Stop 
	///  - NetStream.Seek.Notify 
	///
	/// Error level:
	///  - NetStream.Play.StreamNotFound
	///  - NetStream.Seek.InvalidTime
	///
	/// This method locks the statusMutex during operations
	///
	void setStatus(StatusCode code);

	/// \brief
	/// Call any onStatus event handler passing it
	/// any queued status change, see _statusQueue
	//
	/// Will NOT lock the statusMutex itself, rather it will
	/// iteratively call the popNextPendingStatusNotification()
	/// private method, which will take care of locking it.
	/// This is to make sure onStatus handler won't call methods
	/// possibly trying to obtain the lock again (::play, ::pause, ...)
	///
	void processStatusNotifications();
	
	
	void processNotify(const std::string& funcname, as_object* metadata_obj);

	// The size of the buffer in milliseconds
	boost::uint32_t m_bufferTime;

	// Are a new frame ready to be returned?
	volatile bool m_newFrameReady;

	// Mutex to insure we don't corrupt the image
	boost::mutex image_mutex;

	// The image/videoframe which is given to the renderer
	std::auto_ptr<image::ImageBase> m_imageframe;

	// The video URL
	std::string url;

	// The input media parser
	std::auto_ptr<media::MediaParser> m_parser;

	// Are we playing a FLV?
	bool m_isFLV;

	// The handler which is invoked on status change
	boost::intrusive_ptr<as_function> m_statusHandler;

	// The position in the inputfile, only used when not playing a FLV
	long inputPos;

#ifdef GNASH_USE_GC
	/// Mark all reachable resources of a NetStream, for the GC
	//
	/// Reachable resources are:
	///	- associated NetConnection object (_netCon)
	///	- character to invalidate on video updates (_invalidatedVideoCharacter)
	///	- onStatus event handler (m_statusHandler)
	///
	virtual void markReachableResources() const;
#endif // GNASH_USE_GC

	/// Unplug the advance timer callback
	void stopAdvanceTimer();

	/// Register the advance timer callback
	void startAdvanceTimer();

	/// The character to invalidate on video updates
	character* _invalidatedVideoCharacter;

public:


typedef std::deque<raw_mediadata_t*> AudioQueue;

	enum PauseMode {
	  pauseModeToggle = -1,
	  pauseModePause = 0,
	  pauseModeUnPause = 1	
	};

	NetStream();

	~NetStream();

	/// Closes the video session and frees all ressources used for decoding
	/// except the FLV-parser (this might not be correct).
	void close();

	/// Make audio controlled by given character
	void setAudioController(character* controller);
 
	/// Pauses/starts the playback of the media played by the current instance
	//
	/// @param mode
	///	Defines what mode to put the instance in.
	void pause(PauseMode mode);

	/// Starts the playback of the media
	//
	/// @param source
	///	Defines what file to play
	///
	void play(const std::string& source);

	/// Seek in the media played by the current instance
	//
	/// @param position
	///	Defines in seconds where to seek to
	///	TODO: take milliseconds !!
	///
	void seek(boost::uint32_t pos);

	/// Tells where the playhead currently is
	//
	/// @return The time in milliseconds of the current playhead position
	///
	boost::int32_t time();

	/// Called at the SWF framerate. Used to process queued status messages
	/// and (re)start after a buffering pause. In NetStreamFfmpeg it is also
	/// used to find the next video frame to be shown, though this might change.
	void advance();
	
	/// Returns the current framerate in frames per second.
	double getCurrentFPS()  { return 0; }
	

	/// Sets the NetConnection needed to access external files
	//
	/// @param netconnection
	///
	void setNetCon(boost::intrusive_ptr<NetConnection> nc)
	{
		_netCon = nc;
	}

	/// Return true if the NetStream has an associated NetConnection
	bool isConnected() const { return _netCon != 0; }

	/// Specifies the number of milliseconds to buffer before starting to display the stream.
	//
	/// @param time
	/// The time in milliseconds that should be buffered.
	///
	void setBufferTime(boost::uint32_t time);

	/// Returns what the buffer time has been set to. (100 miliseconds is default)
	//
	/// @return The size of the buffer in milliseconds.
	///
	boost::uint32_t bufferTime() { return m_bufferTime; }

	/// Returns the number of bytes of the media file that have been buffered.
	long bytesLoaded();

	/// Returns the total number of bytes (size) of the media file
	//
	/// @return the total number of bytes (size) of the media file
	///
	long bytesTotal();

	/// Returns the number of millisecond of the media file that is buffered and 
	/// yet to be played
	//
	/// @return Returns the number of millisecond of the media file that is 
	/// buffered and yet to be played
	///
	long bufferLength();

	/// Tells us if there is a new video frame ready
	//
	/// @return true if a frame is ready, false if not
	bool newFrameReady();

	/// Returns the video frame closest to current cursor. See time().
	//
	/// @return a image containing the video frame, a NULL auto_ptr if none were ready
	///
	std::auto_ptr<image::ImageBase> get_video();
	
	/// Register the character to invalidate on video updates
	void setInvalidatedVideo(character* ch)
	{
		_invalidatedVideoCharacter = ch;
	}



	/// Callback used by sound_handler to get audio data
	//
	/// This is a sound_handler::aux_streamer_ptr type.
	///
	/// It will be invoked by a separate thread (neither main, nor decoder thread).
	///
	static bool audio_streamer(void *udata, boost::uint8_t *stream, int len);





private:



	enum PlaybackState {
		PLAY_NONE,
		PLAY_STOPPED,
		PLAY_PLAYING,
		PLAY_PAUSED
	};

	enum DecodingState {
		DEC_NONE,
		DEC_STOPPED,
		DEC_DECODING,
		DEC_BUFFERING
	};

	/// Gets video info from the parser and initializes _videoDecoder
	//
	/// @param parser the parser to use to get video information.
	///
	void initVideoDecoder(media::MediaParser& parser);

	/// Gets audio info from the parser and initializes _audioDecoder
	//
	/// @param parser the parser to use to get audio information.
	///
	void initAudioDecoder(media::MediaParser& parser);

	DecodingState _decoding_state;

	// Mutex protecting _playback_state and _decoding_state
	// (not sure a single one is appropriate)
	boost::mutex _state_mutex;

	// Setups the playback
	bool startPlayback();

	// Pauses the playhead 
	//
	// Users:
	// 	- ::decodeFLVFrame() 
	// 	- ::pause() 
	// 	- ::play() 
	//
	void pausePlayback();

	// Resumes the playback 
	//
	// Users:
	// 	- ::av_streamer() 
	// 	- ::play() 
	// 	- ::startPlayback() 
	// 	- ::advance() 
	//
	void unpausePlayback();

	/// Update the image/videoframe to be returned by next get_video() call.
	//
	/// Uses by ::advance().
	///
	/// Note that get_video will be called by video_stream_instance::display, which
	/// is usually called right after video_stream_instance::advance, so  the result
	/// is that  refreshVideoFrame() is called right before get_video(). This is important
	/// to ensure timing is correct..
	///
	/// @param alsoIfPaused
	///	If true, video is consumed/refreshed even if playhead is paused.
	///	By default this is false, but will be used on ::seek (user-reguested)
	///
	void refreshVideoFrame(bool alsoIfPaused=false);

	/// Refill audio buffers, so to contain new frames since last run
	/// and up to current timestamp
	void refreshAudioBuffer();

	// Used to decode and push the next available (non-FLV) frame to the audio or video queue
	bool decodeMediaFrame();

	/// Decode next video frame fetching it MediaParser cursor
	//
	/// @return 0 on EOF or error, a decoded video otherwise
	///
	std::auto_ptr<image::ImageBase> decodeNextVideoFrame();

	/// Decode next audio frame fetching it MediaParser cursor
	//
	/// @return 0 on EOF or error, a decoded audio frame otherwise
	///
	raw_mediadata_t* decodeNextAudioFrame();

	/// \brief
	/// Decode input audio frames with timestamp <= ts
	/// and push them to the output audio queue
	void pushDecodedAudioFrames(boost::uint32_t ts);

	/// Decode input frames up to the one with timestamp <= ts.
	//
	/// Decoding starts from "next" element in the parser cursor.
	///
	/// Return 0 if:
	///	1. there's no parser active.
	///	2. parser cursor is already on last frame.
	///	3. next element in cursor has timestamp > tx
	///	4. there was an error decoding
	///
	std::auto_ptr<image::ImageBase> getDecodedVideoFrame(boost::uint32_t ts);

	DecodingState decodingStatus(DecodingState newstate = DEC_NONE);

	/// Video decoder
	std::auto_ptr<media::VideoDecoder> _videoDecoder;

	/// Audio decoder
	std::auto_ptr<media::AudioDecoder> _audioDecoder;

	/// Virtual clock used as playback clock source
	std::auto_ptr<InterruptableVirtualClock> _playbackClock;

	/// Playback control device 
	PlayHead _playHead;

	// Current sound handler
	media::sound_handler* _soundHandler;

	// Current media handler
	media::MediaHandler* _mediaHandler;

	/// Parse a chunk of input
	/// Currently blocks, ideally should parse as much
	/// as possible w/out blocking
	void parseNextChunk();

	/// Input stream
	//
	/// This should just be a temporary variable, transferred
	/// to MediaParser constructor.
	///
	std::auto_ptr<IOChannel> _inputStream;

	/// This is where audio frames are pushed by ::advance
	/// and consumed by sound_handler callback (audio_streamer)
	AudioQueue _audioQueue;

	/// Number of bytes in the audio queue, protected by _audioQueueMutex
	size_t _audioQueueSize;

	/// The queue needs to be protected as sound_handler callback
	/// is invoked by a separate thread (dunno if it makes sense actually)
	boost::mutex _audioQueueMutex;

	// Whether or not the aux streamer is attached
	bool _auxStreamerAttached;

	/// Attach the aux streamer.
	//
	/// On success, _auxStreamerAttached will be set to true.
	/// Won't attach again if already attached.
	///
	void attachAuxStreamer();

	/// Detach the aux streamer
	//
	/// _auxStreamerAttached will be set to true.
	/// Won't detach if not attached.
	///
	void detachAuxStreamer();



	/// Pop next queued status notification from the queue
	//
	/// Lock the statusMutex during operations
	///
	/// @return The status code to notify, or invalidStatus when
	///	    the queue is empty
	///
	StatusCode popNextPendingStatusNotification();

	/// Clear status notification queue
	//
	/// Lock the statusMutex during operations
	///
	void clearStatusQueue();

	// TODO: change to a container with fast pop_front()
	typedef std::deque<StatusCode> StatusQueue;

	/// List of status messages to be processed
	StatusQueue _statusQueue;

	/// Mutex protecting _statusQueue
	boost::mutex statusMutex;

	/// Last status code (to avoid consecutively notifying the same event)
	StatusCode _lastStatus;

	/// Get 'status' (first) and 'level' (second) strings for given status code
	//
	/// The two members of the pair are ensured to be not-NULL
	/// Any invalid code, out of bound or explicitly invalid (invalidCode) 
	/// returns two empty C strings.
	///
	std::pair<const char*, const char*> getStatusCodeInfo(StatusCode code);

	/// Return a newly allocated information object for the given status
	boost::intrusive_ptr<as_object> getStatusObject(StatusCode code);

	/// hack for using a Timer to drive ::advance calls
	static as_value advanceWrapper(const fn_call& fn);

	/// Identifier of the advance timer
	unsigned int _advanceTimer;

};


// Initialize the global NetStream class
void netstream_class_init(as_object& global);

} // end of gnash namespace

// __NETSTREAM_H__
#endif

