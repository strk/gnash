// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <boost/intrusive_ptr.hpp>
#include <string>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/scoped_ptr.hpp>

#include "MediaParser.h"
#include "PlayHead.h" // for composition
#include "VideoDecoder.h" // for visibility of dtor
#include "AudioDecoder.h" // for visibility of dtor
#include "VirtualClock.h"
#include "Relay.h" // for ActiveRelay inheritance

// Forward declarations
namespace gnash {
    class CharacterProxy;
    class IOChannel;
    class NetConnection_as;
    class as_function;
    class DisplayObject;
    struct ObjectURI;
    namespace media {
        class MediaHandler;
    }
    namespace sound {
        class sound_handler;
        class InputStream;
    }
}

namespace gnash {

/// Buffered AudioStreamer
//
/// This class you create passing a sound handler, which will
/// be used to implement attach/detach and eventually throw away
/// buffers of sound when no sound handler is given.
///
/// Then you push samples to a buffer of it and can request attach/detach 
/// operations. When attached, the sound handler will fetch samples
/// from the buffer, in a thread-safe way.
///
class BufferedAudioStreamer {
public:

    /// @param handler
    ///     %Sound handler to use for attach/detach
    ///
    BufferedAudioStreamer(sound::sound_handler* handler);

    /// A buffer with a cursor state
    //
    /// @todo Make private, have ::push take a simpler
    ///       form (Buffer?)
    ///
    class CursoredBuffer
    {
    public:
        CursoredBuffer()
            :
            m_size(0),
            m_data(NULL),
            m_ptr(NULL)
        {}

        ~CursoredBuffer()
        {
            delete [] m_data;
        }

        /// Number of samples left in buffer starting from cursor
        boost::uint32_t m_size;

        /// Actual data
        //
        /// The data must be allocated with new []
        /// as will be delete []'d by the dtor
        boost::uint8_t* m_data;

        /// Cursor into the data
        boost::uint8_t* m_ptr;
    };

    typedef boost::ptr_deque<CursoredBuffer> AudioQueue;

    // Delete all samples in the audio queue.
    void cleanAudioQueue();

    sound::sound_handler* _soundHandler;

    /// This is where audio frames are pushed by ::advance
    /// and consumed by sound_handler callback (audio_streamer)
    AudioQueue _audioQueue;

    /// Number of bytes in the audio queue, protected by _audioQueueMutex
    size_t _audioQueueSize;

    /// The queue needs to be protected as sound_handler callback
    /// is invoked by a separate thread (dunno if it makes sense actually)
    boost::mutex _audioQueueMutex;

    // Id of an attached audio streamer, 0 if none
    sound::InputStream* _auxStreamer;

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

    /// Fetch samples from the audio queue
    unsigned int fetch(boost::int16_t* samples, unsigned int nSamples,
                    bool& eof);

    /// Fetch samples from the audio queue
    static unsigned int fetchWrapper(void* owner, boost::int16_t* samples,
                    unsigned int nSamples, bool& eof);

    /// Push a buffer to the audio queue
    //
    /// @param audio
    ///     Samples buffer, ownership transferred.
    ///
    /// @todo: take something simpler (SimpleBuffer?)
    ///
    void push(CursoredBuffer* audio);

};

// -----------------------------------------------------------------

/// NetStream_as ActionScript class
//
/// This class is responsible for handlign external
/// media files. Provides interfaces for playback control.
///
class NetStream_as : public ActiveRelay
{

public:

    enum PauseMode {
      pauseModeToggle = -1,
      pauseModePause = 0,
      pauseModeUnPause = 1  
    };

    NetStream_as(as_object* owner);

    ~NetStream_as();

    PlayHead::PlaybackStatus playbackState() const {
        return _playHead.getState();
    }

    /// Get the real height of the video in pixels if the decoder exists.
    //
    /// @return the height of the video in pixels or 0 if no decoder exists.
    ///         The width returned from the decoder may also vary, and will
    ///         be 0 until it knows the width.
    int videoHeight() const;

    /// Get the real width of the video in pixels if the decoder exists.
    //
    /// @return the width of the video in pixels or 0 if no decoder exists.
    ///         The width returned from the decoder may also vary, and will
    ///         be 0 until it knows the width.
    int videoWidth() const;

    /// Closes the video session and frees all ressources used for decoding
    /// except the FLV-parser (this might not be correct).
    void close();

    /// Make audio controlled by given DisplayObject
    void setAudioController(DisplayObject* controller);
 
    /// Pauses/starts the playback of the media played by the current instance
    //
    /// @param mode
    /// Defines what mode to put the instance in.
    void pause(PauseMode mode);

    /// Starts the playback of the media
    //
    /// @param source
    /// Defines what file to play
    ///
    void play(const std::string& source);

    /// Seek in the media played by the current instance
    //
    /// @param pos
    ///     Defines in seconds where to seek to
    ///     @todo take milliseconds !!
    ///
    void seek(boost::uint32_t pos);

    /// Tells where the playhead currently is
    //
    /// @return The time in milliseconds of the current playhead position
    ///
    boost::int32_t time();

    /// Called at the SWF heart-beat. Used to process queued status messages
    /// and (re)start after a buffering pause. In NetStreamFfmpeg it is also
    /// used to find the next video frame to be shown, though this might
    /// change.
    void update();
    
    /// Returns the current framerate in frames per second.
    double getCurrentFPS()  { return 0; }

    /// Sets the NetConnection needed to access external files
    //
    /// @param nc
    ///     The NetConnection object to use for network access
    ///
    void setNetCon(NetConnection_as* nc) {
        _netCon = nc;
    }

    /// Return true if the NetStream has an associated NetConnection
    bool isConnected() const { return (_netCon); }

    /// Specifies the number of milliseconds to buffer before starting
    /// to display the stream.
    //
    /// @param time
    /// The time in milliseconds that should be buffered.
    ///
    void setBufferTime(boost::uint32_t time);

    /// Returns what the buffer time has been set to. (100 milliseconds
    /// is default)
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

    /// Returns the number of millisecond of the media file that is
    /// buffered and yet to be played
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
    /// @return a image containing the video frame, a NULL auto_ptr if
    /// none were ready
    ///
    std::auto_ptr<image::GnashImage> get_video();
    
    /// Register the DisplayObject to invalidate on video updates
    void setInvalidatedVideo(DisplayObject* ch)
    {
        _invalidatedVideoCharacter = ch;
    }

    virtual void markReachableResources() const;

    /// Callback used by sound_handler to get audio data
    //
    /// This is a sound_handler::aux_streamer_ptr type.
    ///
    /// It might be invoked by a separate thread (neither main,
    /// nor decoder thread).
    ///
    static unsigned int audio_streamer(void *udata, boost::int16_t* samples,
            unsigned int nSamples, bool& eof);

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

    NetConnection_as* _netCon;

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
    
    // The size of the buffer in milliseconds
    boost::uint32_t m_bufferTime;

    // Are a new frame ready to be returned?
    volatile bool m_newFrameReady;

    // Mutex to insure we don't corrupt the image
    boost::mutex image_mutex;

    // The image/videoframe which is given to the renderer
    std::auto_ptr<image::GnashImage> m_imageframe;

    // The video URL
    std::string url;

    // The input media parser
    std::auto_ptr<media::MediaParser> m_parser;

    // The position in the inputfile, only used when not playing a FLV
    long inputPos;

    /// Unplug the advance timer callback
    void stopAdvanceTimer();

    /// Register the advance timer callback
    void startAdvanceTimer();

    /// The DisplayObject to invalidate on video updates
    DisplayObject* _invalidatedVideoCharacter;

private:

    enum DecodingState {
        DEC_NONE,
        DEC_STOPPED,
        DEC_DECODING,
        DEC_BUFFERING
    };

    typedef std::pair<std::string, std::string> NetStreamStatus;

    /// Get 'status' (first) and 'level' (second) strings for given status code
    //
    /// Any invalid code, out of bound or explicitly invalid (invalidCode) 
    /// returns two empty strings.
    ///
    void getStatusCodeInfo(StatusCode code, NetStreamStatus& info);

    /// Return a newly allocated information object for the given status
    as_object* getStatusObject(StatusCode code);

    /// Initialize video decoder and (if successful) PlayHead consumer 
    //
    /// @param info Video codec information
    ///
    void initVideoDecoder(const media::VideoInfo& info);

    /// Initialize audio decoder and (if successful) a PlayHead consumer 
    //
    /// @param info Audio codec information
    ///
    void initAudioDecoder(const media::AudioInfo& parser);

    // Setups the playback
    bool startPlayback();

    // Pauses the playhead 
    //
    // Users:
    //  - ::decodeFLVFrame() 
    //  - ::pause() 
    //  - ::play() 
    //
    void pausePlayback();

    // Resumes the playback 
    //
    // Users:
    //  - ::av_streamer() 
    //  - ::play() 
    //  - ::startPlayback() 
    //  - ::advance() 
    //
    void unpausePlayback();

    /// Update the image/videoframe to be returned by next get_video() call.
    //
    /// Used by update().
    ///
    /// Note that get_video will be called by Video::display(), which
    /// is usually called right after Video::advance(), so the result
    /// is that refreshVideoFrame() is called right before
    /// get_video(). This is important
    /// to ensure timing is correct..
    ///
    /// @param alsoIfPaused
    /// If true, video is consumed/refreshed even if playhead is paused.
    /// By default this is false, but will be used on ::seek (user-reguested)
    ///
    void refreshVideoFrame(bool alsoIfPaused = false);

    /// Refill audio buffers, so to contain new frames since last run
    /// and up to current timestamp
    void refreshAudioBuffer();

    /// Decode next video frame fetching it MediaParser cursor
    //
    /// @return 0 on EOF or error, a decoded video otherwise
    ///
    std::auto_ptr<image::GnashImage> decodeNextVideoFrame();

    /// Decode next audio frame fetching it MediaParser cursor
    //
    /// @return 0 on EOF or error, a decoded audio frame otherwise
    ///
    BufferedAudioStreamer::CursoredBuffer* decodeNextAudioFrame();

    /// \brief
    /// Decode input audio frames with timestamp <= ts
    /// and push them to the output audio queue
    void pushDecodedAudioFrames(boost::uint32_t ts);

    /// Decode input frames up to the one with timestamp <= ts.
    //
    /// Decoding starts from "next" element in the parser cursor.
    ///
    /// Return 0 if:
    /// 1. there's no parser active.
    /// 2. parser cursor is already on last frame.
    /// 3. next element in cursor has timestamp > tx
    /// 4. there was an error decoding
    ///
    std::auto_ptr<image::GnashImage> getDecodedVideoFrame(boost::uint32_t ts);

    DecodingState decodingStatus(DecodingState newstate = DEC_NONE);

    /// Parse a chunk of input
    /// Currently blocks, ideally should parse as much
    /// as possible w/out blocking
    void parseNextChunk();

    DecodingState _decoding_state;

    // Mutex protecting _playback_state and _decoding_state
    // (not sure a single one is appropriate)
    boost::mutex _state_mutex;
    
    /// Video decoder
    std::auto_ptr<media::VideoDecoder> _videoDecoder;

    /// True if video info are known
    bool _videoInfoKnown;

    /// Audio decoder
    std::auto_ptr<media::AudioDecoder> _audioDecoder;

    /// True if an audio info are known
    bool _audioInfoKnown;

    /// Virtual clock used as playback clock source
    boost::scoped_ptr<InterruptableVirtualClock> _playbackClock;

    /// Playback control device 
    PlayHead _playHead;

    // Current sound handler
    sound::sound_handler* _soundHandler;

    // Current media handler
    media::MediaHandler* _mediaHandler;

    /// Input stream
    //
    /// This should just be a temporary variable, transferred
    /// to MediaParser constructor.
    ///
    std::auto_ptr<IOChannel> _inputStream;

    /// The buffered audio streamer
    BufferedAudioStreamer _audioStreamer;

    /// List of status messages to be processed
    StatusCode _statusCode;

    /// Mutex protecting _statusQueue
    boost::mutex statusMutex;

};

void netstream_class_init(as_object& global, const ObjectURI& uri);

void registerNetStreamNative(as_object& global);

} // gnash namespace

#endif

