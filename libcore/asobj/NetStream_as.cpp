// NetStream.cpp:  ActionScript class for streaming audio/video, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "NetStream_as.h"

#include <functional>
#include <algorithm>
#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>

#include "RunResources.h"
#include "CharacterProxy.h"
#include "smart_ptr.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h"
#include "GnashException.h"
#include "NetConnection_as.h"
#include "VM.h"
#include "namedStrings.h"
#include "movie_root.h"
#include "GnashAlgorithm.h"
#include "VirtualClock.h" // for PlayHead
#include "MediaHandler.h"
#include "StreamProvider.h"
#include "sound_handler.h"
#include "AMFConverter.h"
#include "AMF.h"

// Define the following macro to have status notification handling debugged
//#define GNASH_DEBUG_STATUS

// Define the following macro to enable decoding debugging
//#define GNASH_DEBUG_DECODING

// Define the following macro to enable decoding of playhead activity
//#define GNASH_DEBUG_PLAYHEAD

namespace gnash {

namespace {

    as_value netstream_new(const fn_call& fn);
    as_value netstream_close(const fn_call& fn);
    as_value netstream_pause(const fn_call& fn);
    as_value netstream_play(const fn_call& fn);
    as_value netstream_seek(const fn_call& fn);
    as_value netstream_setbuffertime(const fn_call& fn);
    as_value netstream_time(const fn_call& fn);

    as_value netstream_attachAudio(const fn_call& fn);
    as_value netstream_attachVideo(const fn_call& fn);
    as_value netstream_publish(const fn_call& fn);
    as_value netstream_receiveAudio(const fn_call& fn);
    as_value netstream_receiveVideo(const fn_call& fn);
    as_value netstream_send(const fn_call& fn);

    void attachNetStreamInterface(as_object& o);

    /// Transform the volume by the requested amount
    //
    /// @param data     The data to transform
    /// @param size     The length of the array
    /// @param volume   The volume in percent.
    void adjustVolume(boost::int16_t* data, size_t size, int volume);

    // TODO: see where this can be done more centrally.
    void executeTag(const SimpleBuffer& _buffer, as_object& thisPtr);
}

/// Contruct a NetStream object.
//
/// The default size needed to begin playback (m_bufferTime) of media
/// is 100 milliseconds.
NetStream_as::NetStream_as(as_object* owner)
    :
    ActiveRelay(owner),
    _netCon(0),
    m_bufferTime(100), 
    m_newFrameReady(false),
    m_imageframe(),
    m_parser(NULL),
    inputPos(0),
    _invalidatedVideoCharacter(0),
    _decoding_state(DEC_NONE),
    _videoDecoder(0),
    _videoInfoKnown(false),
    _audioDecoder(0),
    _audioInfoKnown(false),

    // Playback clock starts in 'stop' mode
    _playbackClock(
            new InterruptableVirtualClock(getVM(*owner).getClock())),

    // Playhead starts at position 0 with a stopped source clock
    _playHead(_playbackClock.get()), 

    _soundHandler(getRunResources(*owner).soundHandler()),
    _mediaHandler(getRunResources(*owner).mediaHandler()),
    _audioStreamer(_soundHandler),
    _statusCode(invalidStatus)
{
}

void
netstream_class_init(as_object& where, const ObjectURI& uri)
{
    // NetStream is genuinely a built-in class, but its constructor calls
    // several native functions. It also calls NetConnection.call.
    registerBuiltinClass(where, netstream_new, attachNetStreamInterface,
            0, uri);
}

void
registerNetStreamNative(as_object& global)
{
    VM& vm = getVM(global);

    vm.registerNative(netstream_close, 2101, 0);
    vm.registerNative(netstream_attachAudio, 2101, 1);
    vm.registerNative(netstream_attachVideo, 2101, 2);
    vm.registerNative(netstream_send, 2101, 3);
    vm.registerNative(netstream_setbuffertime, 2101, 4);

    // TODO:
    // ASnative(2101, 200): run in the constructor
    // ASnative(2101, 201) [OnCreate.prototype] onResult - inner function
    // ASnative(2101, 202) publish, play, receiveAudio, receiveVideo, pause,
    //                     seek

}

void
NetStream_as::processStatusNotifications()
{
    // TODO: check for System.onStatus too ! use a private
    // getStatusHandler() method for this.
    // Copy it to prevent threads changing it.
    StatusCode code = invalidStatus;

    {
        boost::mutex::scoped_lock lock(statusMutex);

        std::swap(code, _statusCode);
    }

    // Nothing to do if no more valid notifications.
    if (code == invalidStatus) return; 

    // Must be a new object every time.
    as_object* o = getStatusObject(code);

    callMethod(&owner(), NSV::PROP_ON_STATUS, o);
}

void
NetStream_as::setStatus(StatusCode status)
{
    // Get a lock to avoid messing with statuses while processing them
    boost::mutex::scoped_lock lock(statusMutex);
    _statusCode = status;
}

void
NetStream_as::setBufferTime(boost::uint32_t time)
{
    // The argument is in milliseconds,
    m_bufferTime = time;
    if ( m_parser.get() ) m_parser->setBufferTime(time);
}

long
NetStream_as::bufferLength()
{
    if (m_parser.get() == NULL) return 0;
    return m_parser->getBufferLength();
}

bool
NetStream_as::newFrameReady()
{
    if (m_newFrameReady) {
        m_newFrameReady = false;
        return true;
    }
    
    return false;
}

std::auto_ptr<image::GnashImage>
NetStream_as::get_video()
{
    boost::mutex::scoped_lock lock(image_mutex);

    return m_imageframe;    
}

void
NetStream_as::getStatusCodeInfo(StatusCode code, NetStreamStatus& info)
{
    switch (code)
    {
    
        case bufferEmpty:
            info.first = "NetStream.Buffer.Empty";
            info.second = "status";
            return;

        case bufferFull:
            info.first = "NetStream.Buffer.Full";
            info.second = "status";
            return;

        case bufferFlush:
            info.first = "NetStream.Buffer.Flush";
            info.second = "status";
            return;

        case playStart:
            info.first = "NetStream.Play.Start";
            info.second = "status";
            return;

        case playStop:
            info.first = "NetStream.Play.Stop";
            info.second = "status";
            return;

        case seekNotify:
            info.first = "NetStream.Seek.Notify";
            info.second = "status";
            return;

        case streamNotFound:
            info.first = "NetStream.Play.StreamNotFound";
            info.second = "error";
            return;

        case invalidTime:
            info.first = "NetStream.Seek.InvalidTime";
            info.second = "error";
            return;
        default:
            return;
    }
}

as_object* 
NetStream_as::getStatusObject(StatusCode code)
{
    // code, level
    NetStreamStatus info;
    getStatusCodeInfo(code, info);

    // Enumerable and deletable.
    const int flags = 0;

    as_object* o = createObject(getGlobal(owner()));
    o->init_member("code",  info.first, flags);
    o->init_member("level", info.second, flags);

    return o;
}

void
NetStream_as::setAudioController(DisplayObject* ch)
{
    _audioController.reset(new CharacterProxy(ch, getRoot(owner())));
}

void
NetStream_as::markReachableResources() const
{
    if (_netCon) _netCon->setReachable();
    if (_statusHandler) _statusHandler->setReachable();
    if (_audioController) _audioController->setReachable();
    if (_invalidatedVideoCharacter) _invalidatedVideoCharacter->setReachable();
}

void
NetStream_as::stopAdvanceTimer()
{
    getRoot(owner()).removeAdvanceCallback(this);
}

void
NetStream_as::startAdvanceTimer()
{
    getRoot(owner()).addAdvanceCallback(this);
}

NetStream_as::~NetStream_as()
{
    // TODO: have thi done by ~BufferedAudioQueue ?
    _audioStreamer.cleanAudioQueue();
    _audioStreamer.detachAuxStreamer();
}


void NetStream_as::pause(PauseMode mode)
{
    log_debug("::pause(%d) called ", mode);
    switch (mode) {

        case pauseModeToggle:
            if (_playHead.getState() == PlayHead::PLAY_PAUSED) {
                unpausePlayback();
            }
            else pausePlayback();
            break;
        case pauseModePause:
            pausePlayback();
            break;
        case pauseModeUnPause:
            unpausePlayback();
            break;
        default:
            break;
    }

}

void
NetStream_as::close()
{

    // Delete any samples in the audio queue.
    _audioStreamer.cleanAudioQueue();

    // When closing gnash before playback is finished, the soundhandler 
    // seems to be removed before netstream is destroyed.
    _audioStreamer.detachAuxStreamer();

    // Drop all information about decoders and parser
    _videoInfoKnown = false;
    _videoDecoder.reset();
    _audioInfoKnown = false;
    _audioDecoder.reset();
    m_parser.reset();

    m_imageframe.reset();

    stopAdvanceTimer();

}

void
NetStream_as::play(const std::string& c_url)
{

    // It doesn't matter if the NetStream object is already streaming; this
    // starts it again, possibly with a new URL.

    // Does it have an associated NetConnection ?
    if ( ! _netCon)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("No NetConnection associated with this NetStream, "
                "won't play"));
        );
        return;
    }

    if (!_netCon->isConnected()) {

        // This can happen when NetConnection is called with anything but
        // null.
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("NetConnection is not connected. Won't play."));
        );
        return;
    }

    url = c_url;

    // Remove any "mp3:" prefix. Maybe should use this to mark as audio-only
    if (url.compare(0, 4, std::string("mp3:")) == 0) {
        url = url.substr(4);
    }

    if (url.empty()) {
        log_error("Couldn't load URL %s", c_url);
        return;
    }

    // Reset any previously active playback
    close();

    log_security(_("Connecting to movie: %s"), url);

    _inputStream = _netCon->getStream(url); 

    // We need to start playback
    if (!startPlayback()) {
        log_error("NetStream.play(%s): failed starting playback", c_url);
        return;
    }

    // We need to restart the audio
    _audioStreamer.attachAuxStreamer();

    return;
}

void
NetStream_as::initVideoDecoder(const media::VideoInfo& info)
{
    // Caller should check these:
    assert (_mediaHandler); 
    assert (!_videoInfoKnown);
    assert (!_videoDecoder.get());

    _videoInfoKnown = true; 

    try {
        _videoDecoder = _mediaHandler->createVideoDecoder(info);
        assert ( _videoDecoder.get() ); 
        log_debug("NetStream_as::initVideoDecoder: hot-plugging "
                "video consumer");
        _playHead.setVideoConsumerAvailable();
    }
    catch (const MediaException& e) {
        log_error("NetStream: Could not create Video decoder: %s", e.what());

        // This is important enough to let the user know.
        movie_root& m = getRoot(owner());
        m.callInterface(HostMessage(HostMessage::NOTIFY_ERROR,
                std::string(e.what())));
    }

}


void
NetStream_as::initAudioDecoder(const media::AudioInfo& info)
{
    // Caller should check these
    assert ( _mediaHandler ); 
    assert ( !_audioInfoKnown ); 
    assert ( !_audioDecoder.get() );

    _audioInfoKnown = true; 

    try {
        _audioDecoder = _mediaHandler->createAudioDecoder(info);
        assert ( _audioDecoder.get() );
        log_debug("NetStream_as::initAudioDecoder: hot-plugging "
                "audio consumer");
        _playHead.setAudioConsumerAvailable();
    }
    catch (const MediaException& e) {
        const std::string& err = e.what();

        log_error("Could not create Audio decoder: %s", err);

        // This is important enough to let the user know.
        movie_root& m = getRoot(owner());
        m.callInterface(HostMessage(HostMessage::NOTIFY_ERROR, err));
    }

}


bool
NetStream_as::startPlayback()
{
    // Make sure no old information is around
    assert(!_videoInfoKnown);
    assert(!_videoDecoder.get());
    assert(!_audioInfoKnown);
    assert(!_audioDecoder.get());
    // TODO: assert advance timer is not running either !


    // Register advance callback. This must be registered in order for
    // status notifications to be received (e.g. streamNotFound).
    startAdvanceTimer();

    if (!_inputStream.get()) {
        log_error(_("Gnash could not get stream '%s' from NetConnection"),
                url);
        setStatus(streamNotFound);
        return false;
    }

    assert(_inputStream->tell() == static_cast<std::streampos>(0));
    inputPos = 0;

    if (!_mediaHandler) {
        LOG_ONCE( log_error(_("No Media handler registered, can't "
            "parse NetStream input")) );
        return false;
    }
    m_parser = _mediaHandler->createMediaParser(_inputStream);
    assert(!_inputStream.get());

    if (!m_parser.get()) {
        log_error(_("Unable to create parser for NetStream input"));
        // not necessarily correct, the stream might have been found...
        setStatus(streamNotFound);
        return false;
    }

    m_parser->setBufferTime(m_bufferTime);

    // TODO:
    // We do NOT want to initialize decoders right after construction
    // of the MediaParser, but rather construct them when needed, which
    // is when we have something to decode.
    // Postponing this will allow us NOT to block while probing
    // for stream contents.

    decodingStatus(DEC_BUFFERING);

    // NOTE: might be running due to a previous playback in progress
    _playbackClock->pause();

    // NOTE: we set playhead position to 0 here but
    //       other code should take this as a sign
    //       that playHead should be advanced to first available
    //       timeframe timestamp instead.
#ifdef GNASH_DEBUG_PLAYHEAD
     log_debug("%p.startPlayback: playHead position reset to 0", this);
#endif
    _playHead.seekTo(0);
    _playHead.setState(PlayHead::PLAY_PLAYING);

#ifdef GNASH_DEBUG_STATUS
    log_debug("Setting playStart status");
#endif

    setStatus(playStart);

    return true;
}


std::auto_ptr<image::GnashImage> 
NetStream_as::getDecodedVideoFrame(boost::uint32_t ts)
{
    assert(_videoDecoder.get()); 

    std::auto_ptr<image::GnashImage> video;

    assert(m_parser.get());
    if (!m_parser.get()) {
        log_error("getDecodedVideoFrame: no parser available");
        return video; 
    }

    boost::uint64_t nextTimestamp;
    bool parsingComplete = m_parser->parsingCompleted();
    if (!m_parser->nextVideoFrameTimestamp(nextTimestamp)) {

#ifdef GNASH_DEBUG_DECODING
        log_debug("getDecodedVideoFrame(%d): "
            "no more video frames in input "
            "(nextVideoFrameTimestamp returned false, "
            "parsingComplete=%d)",
            ts, parsingComplete);
#endif 

        if (parsingComplete) {
            decodingStatus(DEC_STOPPED);
#ifdef GNASH_DEBUG_STATUS
            log_debug("getDecodedVideoFrame setting playStop status "
                    "(parsing complete and nextVideoFrameTimestamp() "
                    "returned false)");
#endif
            setStatus(playStop);
        }
        return video;
    }

    if (nextTimestamp > ts) {
#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.getDecodedVideoFrame(%d): next video frame is in "
                "the future (%d)", this, ts, nextTimestamp);
#endif 
        // next frame is in the future
        return video; 
    }

    // Loop until a good frame is found
    while (1) {
        video = decodeNextVideoFrame();
        if (!video.get()) {
            log_error("nextVideoFrameTimestamp returned true (%d), "
                "but decodeNextVideoFrame returned null, "
                "I don't think this should ever happen", nextTimestamp);
            break;
        }

        if (!m_parser->nextVideoFrameTimestamp(nextTimestamp)) {
            // the one we decoded was the last one
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.getDecodedVideoFrame(%d): last video frame decoded "
                "(should set playback status to STOP?)", this, ts);
#endif 
            break;
        }
        if (nextTimestamp > ts) {
            // the next one is in the future, we'll return this one.
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.getDecodedVideoFrame(%d): "
                "next video frame is in the future, "
                "we'll return this one",
                this, ts);
#endif 
            break; 
        }
    }

    return video;
}

std::auto_ptr<image::GnashImage> 
NetStream_as::decodeNextVideoFrame()
{
    std::auto_ptr<image::GnashImage> video;

    if (!m_parser.get()) {
        log_error("decodeNextVideoFrame: no parser available");
        return video; 
    }

    std::auto_ptr<media::EncodedVideoFrame> frame = m_parser->nextVideoFrame(); 
    if (!frame.get()) {
#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.decodeNextVideoFrame(): "
            "no more video frames in input",
            this);
#endif 
        return video;
    }

    assert(_videoDecoder.get()); 
    
    // everything we push, we'll pop too..
    assert(!_videoDecoder->peek()); 

    _videoDecoder->push(*frame);
    video = _videoDecoder->pop();
    if (!video.get()) {
        // TODO: tell more about the failure
        log_error(_("Error decoding encoded video frame in NetStream input"));
    }

    return video;
}

BufferedAudioStreamer::CursoredBuffer*
NetStream_as::decodeNextAudioFrame()
{
    assert (m_parser.get());

    std::auto_ptr<media::EncodedAudioFrame> frame = m_parser->nextAudioFrame(); 
    if (!frame.get()) {
#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.decodeNextAudioFrame: "
            "no more video frames in input",
            this);
#endif
        return 0;
    }

    // TODO: make the buffer cursored later ?
    BufferedAudioStreamer::CursoredBuffer* raw =
        new BufferedAudioStreamer::CursoredBuffer();
    raw->m_data = _audioDecoder->decode(*frame, raw->m_size);

    // TODO: let the sound_handler do this .. sounds cleaner
    if (_audioController) {
        DisplayObject* ch = _audioController->get();
        if (ch) {
            const int vol = ch->getWorldVolume();
            if (vol != 100) {
                // NOTE: adjust_volume assumes samples 
                // are 16 bits in size, and signed.
                // Size is still given in bytes..
                adjustVolume(reinterpret_cast<boost::int16_t*>(raw->m_data),
                        raw->m_size / 2, vol);
            }
        }
    }

#ifdef GNASH_DEBUG_DECODING
    log_debug("NetStream_as::decodeNextAudioFrame: "
        "%d bytes of encoded audio "
        "decoded to %d bytes",
        frame->dataSize,
        raw->m_size);
#endif 

    raw->m_ptr = raw->m_data;

    return raw;
}

void
NetStream_as::seek(boost::uint32_t posSeconds)
{
    GNASH_REPORT_FUNCTION;

    // We'll mess with the input here
    if ( ! m_parser.get() )
    {
        log_debug("NetStream_as::seek(%d): no parser, no party", posSeconds);
        return;
    }

    // Don't ask me why, but NetStream_as::seek() takes seconds...
    boost::uint32_t pos = posSeconds*1000;

    // We'll pause the clock source and mark decoders as buffering.
    // In this way, next advance won't find the source time to 
    // be a lot of time behind and chances to get audio buffer
    // overruns will reduce.
    // ::advance will resume the playbackClock if DEC_BUFFERING...
    //
    _playbackClock->pause();

    // Seek to new position
    boost::uint32_t newpos = pos;
    if ( ! m_parser->seek(newpos) )
    {
#ifdef GNASH_DEBUG_STATUS
        log_debug("Setting invalidTime status");
#endif
        setStatus(invalidTime);
        // we won't be *BUFFERING*, so resume now
        _playbackClock->resume(); 
        return;
    }
    log_debug("m_parser->seek(%d) returned %d", pos, newpos);

        // cleanup audio queue, so won't be consumed while seeking
    _audioStreamer.cleanAudioQueue();
    
    // 'newpos' will always be on a keyframe (supposedly)
#ifdef GNASH_DEBUG_PLAYHEAD
     log_debug("%p.seek: playHead position set to %d", this, newpos);
#endif
    _playHead.seekTo(newpos);
    decodingStatus(DEC_BUFFERING); 
    
    refreshVideoFrame(true);
}

void
NetStream_as::parseNextChunk()
{
    // If we parse too much we might block
    // the main thread, if we parse too few
    // we'll get bufferEmpty often.
    // I guess 2 chunks (frames) would be fine..
    //
    m_parser->parseNextChunk();
    m_parser->parseNextChunk();
}

void
NetStream_as::refreshAudioBuffer()
{
    assert (m_parser.get());

#ifdef GNASH_DEBUG_DECODING
    // bufferLength() would lock the mutex (which we already hold),
    // so this is to avoid that.
    boost::uint32_t parserTime = m_parser->getBufferLength();
    boost::uint32_t playHeadTime = time();
    boost::uint32_t bufferLen = 
        parserTime > playHeadTime ? parserTime-playHeadTime : 0;
#endif

    if (_playHead.getState() == PlayHead::PLAY_PAUSED) {
#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.refreshAudioBuffer: doing nothing as playhead "
                "is paused - bufferLength=%d/%d", this, bufferLength(),
                m_bufferTime);
#endif 
        return;
    }

    if (_playHead.isAudioConsumed()) {
#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.refreshAudioBuffer: doing nothing "
            "as current position was already decoded - "
            "bufferLength=%d/%d",
            this, bufferLen, m_bufferTime);
#endif
        return;
    }

    // Calculate the current time
    boost::uint64_t curPos = _playHead.getPosition();

#ifdef GNASH_DEBUG_DECODING
    log_debug("%p.refreshAudioBuffer: currentPosition=%d, playHeadState=%d, bufferLength=%d, bufferTime=%d",
        this, curPos, _playHead.getState(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

    // TODO: here we should fetch all frames up to the one with
    // timestamp >= curPos and push them into the buffer to be 
    // consumed by audio_streamer
    pushDecodedAudioFrames(curPos);
}

void
NetStream_as::pushDecodedAudioFrames(boost::uint32_t ts)
{
    assert(m_parser.get());

    if (!_audioDecoder.get()) {

        // There are 3 possible reasons for _audioDecoder to not be here:
        //
        // 1: The stream does contain audio but we were unable to find
        //    an appropriate decoder for it
        //
        // 2: The stream does contain audio but we didn't try to construct
        //    a decoder for it yet.
        //
        // 3: The stream does not contain audio yet

        if (_audioInfoKnown) {
            // case 1: we saw the audio info already,
            //         but couldn't construct a decoder

            // TODO: shouldn't we still flush any existing Audio frame
            //       in the encoded queue ?

            return;
        }

        media::AudioInfo* audioInfo = m_parser->getAudioInfo();
        if (!audioInfo) {
            // case 3: no audio found yet
            return;
        }

        // case 2: here comes the audio !

        // try to create an AudioDecoder!
        initAudioDecoder(*audioInfo);

        // Don't go ahead if audio decoder construction failed
        if (!_audioDecoder.get()) {
            // TODO: we should still flush any existing Audio frame
            //       in the encoded queue...
            //       (or rely on next call)

            return; 
        }

    }

    bool consumed = false;

    boost::uint64_t nextTimestamp;
    while (1) {

        // FIXME: use services of BufferedAudioStreamer for this
        boost::mutex::scoped_lock lock(_audioStreamer._audioQueueMutex);

        // The sound_handler mixer will pull decoded
        // audio frames off the _audioQueue whenever 
        // new audio has to be played.
        // This is done based on the output frequency,
        // currently hard-coded to be 44100 samples per second.
        //
        // Our job here would be to provide that much data.
        // We're in an ::advance loop, so must provide enough
        // data for the mixer to fetch till next advance.
        // Assuming we know the ::advance() frame rate (which we don't
        // yet) the computation would be something along these lines:
        //
        //    44100/1 == samplesPerAdvance/secsPerAdvance
        //    samplesPerAdvance = secsPerAdvance*(44100/1)
        //
        // For example, at 12FPS we have:
        //
        //   secsPerAdvance = 1/12 = .083333
        //   samplesPerAdvance = .08333*44100 =~ 3675
        //
        // Now, to know how many samples are on the queue
        // we need to know the size in bytes of each sample.
        // If I'm not wrong this is again hard-coded to 2 bytes,
        // so we'd have:
        //
        //   bytesPerAdvance = samplesPerAdvance / sampleSize
        //   bytesPerAdvance = 3675 / 2 =~ 1837
        //
        // Finally we'll need to find number of bytes in the
        // queue to really tell how many there are (don't think
        // it's a fixed size for each element).
        //
        // For now we use the hard-coded value of 20, arbitrarely
        // assuming there is an average of 184 samples per frame.
        //
        // - If we push too few samples, we'll hear silence gaps (underrun)
        // - If we push too many samples the audio mixer consumer
        //   won't be able to consume all before our next filling
        //   iteration (overrun)
        //
        // For *underrun* conditions we kind of have an handling, that is
        // sending the BufferEmpty event and closing the time tap (this is
        // done by ::advance directly).
        //
        // For *overrun* conditions we currently don't have any handling.
        // One possibility could be closing the time tap till we've done
        // consuming the queue.
        //
        //

        float swfFPS = 25; // TODO: get this host app (gnash -d affects this)
        double msecsPerAdvance = 10000/swfFPS;

        const unsigned int bufferLimit = 20;
        unsigned int bufferSize = _audioStreamer._audioQueue.size();
        if (bufferSize > bufferLimit) {

            // we won't buffer more then 'bufferLimit' frames in the queue
            // to avoid ending up with a huge queue which will take some
            // time before being consumed by audio mixer, but still marked
            // as "consumed". Keeping decoded frames buffer low would also
            // reduce memory use.
            //
            // The alternative would be always decode on demand from the
            // audio consumer thread, but would introduce a lot of thread-safety
            // issues: playhead would need protection, input would need
            // protection.
            //
//#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.pushDecodedAudioFrames(%d) : buffer overrun (%d/%d).",
                this, ts, bufferSize, bufferLimit);
//#endif 

            // we may want to pause the playbackClock here...
            _playbackClock->pause();

            return;
        }
        
        // no need to keep the audio queue locked while decoding.
        lock.unlock();

        bool parsingComplete = m_parser->parsingCompleted();
        if (!m_parser->nextAudioFrameTimestamp(nextTimestamp)) {
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.pushDecodedAudioFrames(%d): "
                "no more audio frames in input "
                "(nextAudioFrameTimestamp returned false, parsingComplete=%d)",
                this, ts, parsingComplete);
#endif 

            if (parsingComplete) {
                consumed = true;
                decodingStatus(DEC_STOPPED);
#ifdef GNASH_DEBUG_STATUS
                log_debug("pushDecodedAudioFrames setting playStop status "
                        "(parsing complete and nextAudioFrameTimestamp "
                        "returned false)");
#endif
                setStatus(playStop);
            }

            break;
        }

        if (nextTimestamp > ts) {
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.pushDecodedAudioFrames(%d): "
                "next audio frame is in the future (%d)",
                this, ts, nextTimestamp);
#endif 
            consumed = true;

            // next frame is in the future
            if (nextTimestamp > ts+msecsPerAdvance) break; 
        }

        BufferedAudioStreamer::CursoredBuffer* audio = decodeNextAudioFrame();
        if (!audio) {
            // Well, it *could* happen, why not ?
            log_error("nextAudioFrameTimestamp returned true (%d), "
                "but decodeNextAudioFrame returned null, "
                "I don't think this should ever happen", nextTimestamp);
            break;
        }

        if (!audio->m_size) {
            // Don't bother pushing an empty frame
            // to the audio Queue...
            log_debug("pushDecodedAudioFrames(%d): Decoded audio frame "
                    "contains no samples");
            delete audio;
            continue;
        }

#ifdef GNASH_DEBUG_DECODING
        // this one we might avoid :) -- a less intrusive logging could
        // be take note about how many things we're pushing over
        log_debug("pushDecodedAudioFrames(%d) pushing %dth frame with "
                "timestamp %d", ts, _audioStreamer._audioQueue.size()+1,
                nextTimestamp); 
#endif

        _audioStreamer.push(audio);

    }

    // If we consumed audio of current position, feel free to advance
    // if needed, resuming playbackClock too...
    if (consumed) {
        // resume the playback clock, assuming the
        // only reason for it to be paused is we
        // put in pause mode due to buffer overrun
        // (ie: the sound handler is slow at consuming
        // the audio data).
#ifdef GNASH_DEBUG_DECODING
        log_debug("resuming playback clock on audio consume");
#endif 
        assert(decodingStatus()!=DEC_BUFFERING);
        _playbackClock->resume();

        _playHead.setAudioConsumed();
    }

}


void
NetStream_as::refreshVideoFrame(bool alsoIfPaused)
{
    assert (m_parser.get());

    if (!_videoDecoder.get()) {
        // There are 3 possible reasons for _videoDecoder to not be here:
        //
        // 1: The stream does contain video but we were unable to find
        //    an appropriate decoder for it
        //
        // 2: The stream does contain video but we didn't try to construct
        //    a decoder for it yet.
        //
        // 3: The stream does not contain video yet
        //

        if (_videoInfoKnown) {
            // case 1: we saw the video info already,
            //         but couldn't construct a decoder

            // TODO: shouldn't we still flush any existing Video frame
            //       in the encoded queue ?

            return;
        }

        media::VideoInfo* videoInfo = m_parser->getVideoInfo();
        if (!videoInfo) {
            // case 3: no video found yet
            return;
        }

        // case 2: here comes the video !

        // Try to initialize the video decoder 
        initVideoDecoder(*videoInfo);

        // Don't go ahead if video decoder construction failed
        if (!_videoDecoder.get()) {
            // TODO: we should still flush any existing Video frame
            //       in the encoded queue...
            //       (or rely on next call)
            return; 
        }

    }

#ifdef GNASH_DEBUG_DECODING
    boost::uint32_t bufferLen = bufferLength();
#endif

    if ( ! alsoIfPaused && _playHead.getState() == PlayHead::PLAY_PAUSED )
    {
#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.refreshVideoFrame: doing nothing as playhead is paused - "
            "bufferLength=%d, bufferTime=%d",
            this, bufferLen, m_bufferTime);
#endif 
        return;
    }

    if ( _playHead.isVideoConsumed() ) 
    {
#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.refreshVideoFrame: doing nothing "
            "as current position was already decoded - "
            "bufferLength=%d, bufferTime=%d",
            this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
        return;
    }

    // Calculate the current time
    boost::uint64_t curPos = _playHead.getPosition();

#ifdef GNASH_DEBUG_DECODING
    log_debug("%p.refreshVideoFrame: currentPosition=%d, playHeadState=%d, "
            "bufferLength=%d, bufferTime=%d",
            this, curPos, _playHead.getState(), bufferLen, m_bufferTime);
#endif 

    // Get next decoded video frame from parser, will have the lowest timestamp
    std::auto_ptr<image::GnashImage> video = getDecodedVideoFrame(curPos);

    // to be decoded or we're out of data
    if (!video.get())
    {
        if ( decodingStatus() == DEC_STOPPED )
        {
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.refreshVideoFrame(): "
                "no more video frames to decode "
                "(DEC_STOPPED, null from getDecodedVideoFrame)",
                this);
#endif
        }
        else
        {
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.refreshVideoFrame(): "
                "last video frame was good enough "
                "for current position",
                this);
#endif 
            // There no video but decoder is still running
            // not much to do here except wait for next call
            //assert(decodingStatus() == DEC_BUFFERING);
        }

    }
    else
    {
        m_imageframe = video; // ownership transferred
        assert(!video.get());
        // A frame is ready for pickup
        if ( _invalidatedVideoCharacter )
        {
            _invalidatedVideoCharacter->set_invalidated();

            // NOTE: setting the newFrameReady flag this is not needed anymore,
            // we don't realy on newFrameReady() call anyore to invalidate
            // the video DisplayObject
        }
    }

    // We consumed video of current position, feel free to advance if needed
    _playHead.setVideoConsumed();


}

int
NetStream_as::videoHeight() const
{
    if (!_videoDecoder.get()) return 0;
    return _videoDecoder->height();
}

int
NetStream_as::videoWidth() const
{
    if (!_videoDecoder.get()) return 0;
    return _videoDecoder->width();
}


void
NetStream_as::update()
{
    // Check if there are any new status messages, and if we should
    // pass them to a event handler
    processStatusNotifications();

    // Nothing to do if we don't have a parser.
    if (!m_parser.get()) {
        return;
    }

    if ( decodingStatus() == DEC_STOPPED )
    {
        //log_debug("NetStream_as::advance: dec stopped...");
        // nothing to do if we're stopped...
        return;
    }

    bool parsingComplete = m_parser->parsingCompleted();
#ifndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
    if ( ! parsingComplete ) parseNextChunk();
#endif

    size_t bufferLen = bufferLength();

    // Check decoding status 
    if ( decodingStatus() == DEC_DECODING && bufferLen == 0 )
    {
        if (!parsingComplete)
        {
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.advance: buffer empty while decoding,"
                " setting buffer to buffering and pausing playback clock",
                this);
#endif 
#ifdef GNASH_DEBUG_STATUS
            log_debug("Setting bufferEmpty status");
#endif
            setStatus(bufferEmpty);
            decodingStatus(DEC_BUFFERING);
            _playbackClock->pause();
        }
        else
        {
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.advance : bufferLength=%d, parsing completed",
                this, bufferLen);
#endif
            // set playStop ? (will be done later for now)
        }
    }

    if ( decodingStatus() == DEC_BUFFERING )
    {
        if ( bufferLen < m_bufferTime && ! parsingComplete )
        {
#ifdef GNASH_DEBUG_DECODING
            log_debug("%p.advance: buffering"
                " - position=%d, buffer=%d/%d",
                this, _playHead.getPosition(), bufferLen, m_bufferTime);
#endif

            // The very first video frame we want to provide
            // as soon as possible (if not paused),
            // reguardless bufferLength...
            if (!m_imageframe.get() && 
                    _playHead.getState() != PlayHead::PLAY_PAUSED)
            {
                //log_debug("refreshing video frame for the first time");
                refreshVideoFrame(true);
            }

            return;
        }

#ifdef GNASH_DEBUG_DECODING
        log_debug("%p.advance: buffer full (or parsing completed), "
                "resuming playback clock - position=%d, buffer=%d/%d",
                this, _playHead.getPosition(), bufferLen, m_bufferTime);
#endif

        setStatus(bufferFull);
        decodingStatus(DEC_DECODING);
        _playbackClock->resume();
    }

    // If playhead position needs to be updated
    // is set to Set playhead to first available frame, if any
    // TODO: use another flag to signify 'initialization-needed'
    boost::uint64_t curPosition = _playHead.getPosition();
    if ( curPosition == 0 )
    {
        boost::uint64_t firstFrameTimestamp;
        if ( m_parser->nextFrameTimestamp(firstFrameTimestamp) )
        {
             _playHead.seekTo(firstFrameTimestamp);
#ifdef GNASH_DEBUG_PLAYHEAD
            log_debug("%p.advance: playHead position set to timestamp of first frame: %d", this, firstFrameTimestamp);
#endif
        }
#ifdef GNASH_DEBUG_PLAYHEAD
        else
        {
            log_debug("%p.advance: playHead position is 0 and parser still doesn't have a frame to set it to", this);
        }
#endif
    }

    // Find video frame with the most suited timestamp in the video queue,
    // and put it in the output image frame.
    refreshVideoFrame();

    // Refill audio buffer to consume all samples
    // up to current playhead
    refreshAudioBuffer();

    // Advance PlayHead position if current one was consumed
    // by all available consumers
    _playHead.advanceIfConsumed();

    // As of bug #26687 we discovered that 
    // an FLV containing only audio with consecutive
    // frames performing a jump of more then an hour
    // result in a jump-forward of the playhead (NetStream.time)
    // w/out waiting for the whole time gap to elapse
    //
    // We'll then perform the jump with this conditions:
    //  1: there are no video frames yet
    //  2: the audio buffer is empty, to avoid buffer overrun conditions
    //  3: input audio frames exist with a timestamp in the future
    //
    if ( ! m_parser->getVideoInfo() ) 
    {
        // FIXME: use services of BufferedAudioStreamer for this
        boost::mutex::scoped_lock lock(_audioStreamer._audioQueueMutex);
        bool emptyAudioQueue = _audioStreamer._audioQueue.empty();
        lock.unlock();

        if ( emptyAudioQueue )
        {
            boost::uint64_t nextTimestamp;
            if ( m_parser->nextAudioFrameTimestamp(nextTimestamp) )
            {
                log_debug("Moving NetStream playhead "
                          "from timestamp %d to timestamp %d "
                          "as there are no video frames yet, "
                          "audio buffer is empty and next audio "
                          "frame timestamp is there (see bug #26687)",
                          _playHead.getPosition(), nextTimestamp);
                _playHead.seekTo(nextTimestamp);
            }
        }
    }

    media::MediaParser::OrderedMetaTags tags;

    m_parser->fetchMetaTags(tags, _playHead.getPosition());

    if (tags.empty()) return;

    for (media::MediaParser::OrderedMetaTags::iterator i = tags.begin(),
            e = tags.end(); i != e; ++i) {
        executeTag(**i, owner());
    }
}

boost::int32_t
NetStream_as::time()
{
    return _playHead.getPosition();
}

void
NetStream_as::pausePlayback()
{
    GNASH_REPORT_FUNCTION;

    PlayHead::PlaybackStatus oldStatus = 
        _playHead.setState(PlayHead::PLAY_PAUSED);

    // Disconnect the soundhandler if we were playing before
    if ( oldStatus == PlayHead::PLAY_PLAYING )
    {
        _audioStreamer.detachAuxStreamer();
    }
}

void
NetStream_as::unpausePlayback()
{

    PlayHead::PlaybackStatus oldStatus = 
        _playHead.setState(PlayHead::PLAY_PLAYING);

    // Re-connect to the soundhandler if we were paused before
    if ( oldStatus == PlayHead::PLAY_PAUSED )
    {
        _audioStreamer.attachAuxStreamer();
    }
}


long
NetStream_as::bytesLoaded ()
{
    if ( ! m_parser.get() ) {
//        log_debug("bytesLoaded: no parser, no party");
        return 0;
    }

    return m_parser->getBytesLoaded();
}

long
NetStream_as::bytesTotal ()
{
    if ( ! m_parser.get() ) {
//        log_debug("bytesTotal: no parser, no party");
        return 0;
    }

    return m_parser->getBytesTotal();
}

NetStream_as::DecodingState
NetStream_as::decodingStatus(DecodingState newstate)
{
    boost::mutex::scoped_lock lock(_state_mutex);

    if (newstate != DEC_NONE) {
        _decoding_state = newstate;
    }

    return _decoding_state;
}

//------- BufferedAudioStreamer (move in his own file)

void
BufferedAudioStreamer::attachAuxStreamer()
{
    if ( ! _soundHandler ) return;
    if ( _auxStreamer )
    {
        log_debug("attachAuxStreamer called while already attached");
        // Let's detach first..
        _soundHandler->unplugInputStream(_auxStreamer);
        _auxStreamer=0;
    }

    try {
        _auxStreamer = _soundHandler->attach_aux_streamer(
                BufferedAudioStreamer::fetchWrapper, (void*)this);
    }
    catch (SoundException& e) {
        log_error("Could not attach NetStream aux streamer to sound handler: "
                "%s", e.what());
    }
}

void
BufferedAudioStreamer::detachAuxStreamer()
{
    if ( ! _soundHandler ) return;
    if ( !_auxStreamer )
    {
        log_debug("detachAuxStreamer called while not attached");
        return;
    }
    _soundHandler->unplugInputStream(_auxStreamer);
    _auxStreamer = 0;
}

// audio callback, possibly running in a separate thread
unsigned int
BufferedAudioStreamer::fetchWrapper(void *owner, boost::int16_t* samples,
        unsigned int nSamples, bool& eof)
{
    BufferedAudioStreamer* streamer =
        static_cast<BufferedAudioStreamer*>(owner);

    return streamer->fetch(samples, nSamples, eof);
}

BufferedAudioStreamer::BufferedAudioStreamer(sound::sound_handler* handler)
    :
    _soundHandler(handler),
    _audioQueue(),
    _audioQueueSize(0),
    _auxStreamer(0)
{
}

unsigned int
BufferedAudioStreamer::fetch(boost::int16_t* samples, unsigned int nSamples, bool& eof)
{
    //GNASH_REPORT_FUNCTION;

    boost::uint8_t* stream = reinterpret_cast<boost::uint8_t*>(samples);
    int len = nSamples*2;

    boost::mutex::scoped_lock lock(_audioQueueMutex);

#if 0
    log_debug("audio_streamer called, audioQueue size: %d, "
        "requested %d bytes of fill-up",
        _audioQueue.size(), len);
#endif


    while (len)
    {
        if ( _audioQueue.empty() )
        {
            break;
        }

        CursoredBuffer* samples = _audioQueue.front();

        assert( ! (samples->m_size%2) ); 
        int n = std::min<int>(samples->m_size, len);
        std::copy(samples->m_ptr, samples->m_ptr+n, stream);

        stream += n;
        samples->m_ptr += n;
        samples->m_size -= n;
        len -= n;

        if (samples->m_size == 0)
        {
            delete samples;
            _audioQueue.pop_front();
        }

        _audioQueueSize -= n; // we consumed 'n' bytes here 

    }

    assert( ! (len%2) ); 

    // currently never signalling EOF
    eof=false;
    return nSamples-(len/2);
}

void
BufferedAudioStreamer::push(CursoredBuffer* audio)
{
    boost::mutex::scoped_lock lock(_audioQueueMutex);

    if ( _auxStreamer )
    {
        _audioQueue.push_back(audio);
        _audioQueueSize += audio->m_size;
    }
    else 
    {
        // Don't bother pushing audio to the queue,
        // as nobody would consume it...
        delete audio;
    }
}

void
BufferedAudioStreamer::cleanAudioQueue()
{
    boost::mutex::scoped_lock lock(_audioQueueMutex);

    deleteChecked(_audioQueue.begin(), _audioQueue.end());

    _audioQueue.clear();
}

namespace {

as_value
netstream_new(const fn_call& fn)
{

    as_object* obj = fn.this_ptr;

    NetStream_as* ns = new NetStream_as(obj);

    if (fn.nargs) {

        NetConnection_as* nc;
        if (isNativeType(toObject(fn.arg(0), getVM(fn)), nc)) {
            ns->setNetCon(nc);
        }
        else {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("First argument "
                    "to NetStream constructor "
                    "doesn't cast to a NetConnection (%s)"),
                    fn.arg(0));
            );
        }
    }
    obj->setRelay(ns);


    return as_value();

}

as_value
netstream_close(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    ns->close();
    return as_value();
}

as_value
netstream_pause(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    
    // mode: -1 ==> toogle, 0==> pause, 1==> play
    NetStream_as::PauseMode mode = NetStream_as::pauseModeToggle;
    if (fn.nargs > 0) {
        mode = toBool(fn.arg(0), getVM(fn)) ? NetStream_as::pauseModePause :
                                              NetStream_as::pauseModeUnPause;
    }
    
    // Toggle pause mode
    ns->pause(mode); 
    return as_value();
}

as_value
netstream_play(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("NetStream_as play needs args"));
        );
        return as_value();
    }

    if (!ns->isConnected()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("NetStream.play(%s): stream is not connected"),
                fn.arg(0));
        );
        return as_value();
    }

    ns->play(fn.arg(0).to_string());

    return as_value();
}

as_value
netstream_seek(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    boost::uint32_t time = 0;
    if (fn.nargs > 0) {
        time = static_cast<boost::uint32_t>(toNumber(fn.arg(0), getVM(fn)));
    }
    ns->seek(time);

    return as_value();
}

as_value
netstream_setbuffertime(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);

    // TODO: should we do anything if given no args ?
    //       are we sure setting bufferTime to 0 is what we have to do ?
    double time = 0;
    if (fn.nargs > 0) {
        time = toNumber(fn.arg(0), getVM(fn));
    }

    // TODO: don't allow a limit < 100 

    ns->setBufferTime(boost::uint32_t(time * 1000));

    return as_value();
}

as_value
netstream_attachAudio(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    UNUSED(ns);

    LOG_ONCE(log_unimpl("NetStream.attachAudio"));

    return as_value();
}

as_value
netstream_attachVideo(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    UNUSED(ns);

    LOG_ONCE(log_unimpl("NetStream.attachVideo"));

    return as_value();
}

as_value
netstream_publish(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    UNUSED(ns);

    LOG_ONCE(log_unimpl("NetStream.publish"));

    return as_value();
}

as_value
netstream_receiveAudio(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    UNUSED(ns);

    LOG_ONCE(log_unimpl("NetStream.receiveAudio"));

    return as_value();
}

as_value
netstream_receiveVideo(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    UNUSED(ns);

    LOG_ONCE(log_unimpl("NetStream.receiveVideo"));

    return as_value();
}

as_value
netstream_send(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    UNUSED(ns);

    LOG_ONCE(log_unimpl("NetStream.send"));

    return as_value();
}

// Both a getter and a (do-nothing) setter for time
as_value
netstream_time(const fn_call& fn)
{
    //GNASH_REPORT_FUNCTION;

    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);

    assert(fn.nargs == 0); // we're a getter
    return as_value(double(ns->time()/1000.0));
}

// Both a getter and a (do-nothing) setter for bytesLoaded
as_value
netstream_bytesloaded(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    if (!ns->isConnected()) {
        return as_value();
    }
    long ret = ns->bytesLoaded();
    return as_value(ret);
}

// Both a getter and a (do-nothing) setter for bytesTotal
as_value
netstream_bytestotal(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    if (!ns->isConnected()) {
        return as_value();
    }
    long ret = ns->bytesTotal();
    return as_value(ret);
}

// Both a getter and a (do-nothing) setter for currentFPS
as_value
netstream_currentFPS(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    if (!ns->isConnected()) {
        return as_value();
    }

    double fps = ns->getCurrentFPS();

    return as_value(fps);
}

// read-only property bufferLength: amount of time buffered before playback
as_value
netstream_bufferLength(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);

    // NetStream_as::bufferLength returns milliseconds, we want
    // to return *fractional* seconds.
    double ret = ns->bufferLength()/1000.0;
    return as_value(ret);
}

// Both a getter and a (do-nothing) setter for bufferTime
as_value
netstream_bufferTime(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    // We return bufferTime in seconds
    double ret = ns->bufferTime() / 1000.0;
    return as_value(ret);
}

// Both a getter and a (do-nothing) setter for liveDelay
as_value
netstream_liveDelay(const fn_call& fn)
{
    NetStream_as* ns = ensure<ThisIsNative<NetStream_as> >(fn);
    UNUSED(ns);

    LOG_ONCE(log_unimpl("NetStream.liveDelay getter/setter"));

    if (fn.nargs == 0) {
        return as_value();
    }
    return as_value();
}

void
attachNetStreamInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    VM& vm = getVM(o);
    
    o.init_member("close", vm.getNative(2101, 0));
    o.init_member("pause", gl.createFunction(netstream_pause));
    o.init_member("play", gl.createFunction(netstream_play));
    o.init_member("seek", gl.createFunction(netstream_seek));
    o.init_member("setBufferTime", vm.getNative(2101, 4));
    o.init_member("attachAudio", vm.getNative(2101, 1));
    o.init_member("attachVideo", vm.getNative(2101, 2));
    o.init_member("publish", gl.createFunction(netstream_publish));
    o.init_member("receiveAudio", gl.createFunction(netstream_receiveAudio));
    o.init_member("receiveVideo", gl.createFunction(netstream_receiveVideo));
    o.init_member("send", vm.getNative(2101, 3));

    // Properties
    // TODO: attach to each instance rather then to the class ? check it ..

    o.init_readonly_property("time", &netstream_time);
    o.init_readonly_property("bytesLoaded", &netstream_bytesloaded);
    o.init_readonly_property("bytesTotal", &netstream_bytestotal);
    o.init_readonly_property("currentFps", &netstream_currentFPS);
    o.init_readonly_property("bufferLength", &netstream_bufferLength);
    o.init_readonly_property("bufferTime", &netstream_bufferTime);
    o.init_readonly_property("liveDelay", &netstream_liveDelay);

}

void
executeTag(const SimpleBuffer& _buffer, as_object& thisPtr)
{
	const boost::uint8_t* ptr = _buffer.data();
	const boost::uint8_t* endptr = ptr + _buffer.size();

    std::string funcName;

    try {
        funcName = amf::readString(ptr, endptr);
    }
    catch (const amf::AMFException&) {
        log_error("Invalid AMF data in FLV tag");
        return;
    }

	VM& vm = getVM(thisPtr);
	const ObjectURI& funcKey = getURI(vm, funcName);

    amf::Reader rd(ptr, endptr, getGlobal(thisPtr));

	as_value arg;
	if (!rd(arg)) {
		log_error("Could not convert FLV metatag to as_value, passing "
                "undefined");
	}

	log_debug("Calling %s(%s)", funcName, arg);
	callMethod(&thisPtr, funcKey, arg);
}

// AS-volume adjustment
void
adjustVolume(boost::int16_t* data, size_t size, int volume)
{
    std::transform(data, data + size, data,
            boost::bind(std::multiplies<double>(), volume / 100.0, _1));
}

} // anonymous namespace
} // gnash namespace
