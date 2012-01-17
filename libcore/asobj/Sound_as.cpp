// Sound_as.cpp:  ActionScript "Sound" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include "Sound_as.h"

#include <string>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

#include "RunResources.h"
#include "log.h"
#include "sound_handler.h"
#include "AudioDecoder.h"
#include "MediaHandler.h"
#include "sound_definition.h"
#include "movie_root.h"
#include "movie_definition.h"
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h" // for ActionException
#include "NativeFunction.h" // need builtin_function
#include "VM.h"
#include "namedStrings.h"
#include "StreamProvider.h"
#include "ObjectURI.h"
#include "Relay.h"
#include "Id3Info.h"

//#define GNASH_DEBUG_SOUND_AS 1

namespace gnash {

// Forward declarations
namespace {
    as_value sound_new(const fn_call& fn);
    as_value sound_attachsound(const fn_call& fn);
    as_value sound_getbytesloaded(const fn_call& fn);
    as_value sound_setPosition(const fn_call& fn);
    as_value sound_areSoundsInaccessible(const fn_call& fn);
    as_value sound_getbytestotal(const fn_call& fn);
    as_value sound_getpan(const fn_call& fn);
    as_value sound_setpan(const fn_call& fn);
    as_value sound_getDuration(const fn_call& fn);
    as_value sound_setDuration(const fn_call& fn);
    as_value sound_gettransform(const fn_call& fn);
    as_value sound_getPosition(const fn_call& fn);
    as_value sound_getvolume(const fn_call& fn);
    as_value sound_loadsound(const fn_call& fn);
    as_value sound_settransform(const fn_call& fn);
    as_value sound_setvolume(const fn_call& fn);
    as_value sound_start(const fn_call& fn);
    as_value sound_stop(const fn_call& fn);
    as_value checkPolicyFile_getset(const fn_call& fn);
    void attachSoundInterface(as_object& o);

    /// If there is Id3 data, create an id3 member and call the onID3 function.
    void handleId3Data(boost::optional<media::Id3Info> id3, as_object& sound);
}

/// A Sound object in ActionScript can control and play sound
//
/// Two types of sound are handled:
///
/// 1. external sounds, either loaded or streamed
/// 2. embedded sounds, referenced by library (export) symbol.
//
/// Sound objects also control volume, pan, and other properties for a target
/// movieclip.
//
/// Sound_as objects
//
/// 1. May be associated with a particular DisplayObject.
/// 2. May be associated with one or more playing sounds.
//
/// A Sound_as that is not associated with a particular DisplayObject controls
/// the sound properties of the whole Movie.
class Sound_as : public ActiveRelay
{

public:

    Sound_as(as_object* owner);
    
    ~Sound_as();
    
    /// Make this sound control the given DisplayObject
    //
    /// NOTE: 0 is accepted, to implement an "invalid"
    ///       controller type.
    ///
    void attachCharacter(DisplayObject* attachedChar);

    void attachSound(int si, const std::string& name);

    /// Get number of bytes loaded.
    //
    /// This only applies to external sounds. If unknown or not external, -1
    /// is returned.
    long getBytesLoaded();

    /// Get total number of bytes in the external sound being loaded
    //
    /// This only applies to external sounds. If unknown or not external, -1
    /// is returned.
    long getBytesTotal();

    /// Whether the Sound_as has any sound data
    bool active() const {
        return soundId >= 0 || isStreaming;
    }

    /// Get the pan setting of the attached DisplayObject.
    //
    /// If no object is attached, this retrieves settings for the whole Movie.
    void getPan();

    /// Get the sound transform of the attached DisplayObject.
    //
    /// If no object is attached, this retrieves settings for the whole Movie.
    void getTransform();

    /// Get volume from associated resource
    //
    /// @return true of volume was obtained, false
    ///         otherwise (for example if the associated
    ///         DisplayObject was unloaded).
    ///
    bool getVolume(int& volume);
    void setVolume(int volume);

    /// Load an external sound.
    //
    /// The Sound object is then associated with the external sound.
    void loadSound(const std::string& file, bool streaming);

    void setPan();

    void setTransform();

    void start(double secsStart, int loops);

    void stop(int si);

    /// Get the duration of the sound.
    //
    /// This is only meaningful when the Sound_as object has an associated
    /// sound, that is after attachSound or loadSound has been called.
    size_t getDuration() const;

    /// Get the position within the sound.
    //
    /// This is only meaningful when the Sound_as object has an associated
    /// sound, that is after attachSound or loadSound has been called.
    size_t getPosition() const;

    std::string soundName;  

private:

    void markReachableResources() const;

    boost::scoped_ptr<CharacterProxy> _attachedCharacter;
    int soundId;
    bool externalSound;
    bool isStreaming;

    sound::sound_handler* _soundHandler;

    media::MediaHandler* _mediaHandler;

    boost::scoped_ptr<media::MediaParser> _mediaParser;

    boost::scoped_ptr<media::AudioDecoder> _audioDecoder;

    /// Number of milliseconds into the sound to start it
    //
    /// This is set by start()
    boost::uint64_t _startTime;

    boost::scoped_array<boost::uint8_t> _leftOverData;
    boost::uint8_t* _leftOverPtr;
    boost::uint32_t _leftOverSize;

    /// This is a sound_handler::aux_streamer_ptr type.
    static unsigned int getAudioWrapper(void *owner, boost::int16_t* samples,
            unsigned int nSamples, bool& etEOF);

    unsigned int getAudio(boost::int16_t* samples, unsigned int nSamples,
            bool& atEOF);

    /// The aux streamer for sound handler
    sound::InputStream* _inputStream;

    int remainingLoops;

    /// Query media parser for audio info, create decoder and attach aux streamer
    /// if found.
    ///
    /// @return  an InputStream* if audio found and aux streamer attached,
    ///          0 if no audio found.
    ///
    /// May throw a MediaException if audio was found but
    /// audio decoder could not be created
    /// 
    sound::InputStream* attachAuxStreamerIfNeeded();

    /// Register a timer for audio info probing
    void startProbeTimer();

    /// Unregister the probe timer
    void stopProbeTimer();

    virtual void update();

    /// Probe audio
    void probeAudio();

    bool _soundCompleted;

    boost::mutex _soundCompletedMutex;

    /// Thread-safe setter for _soundCompleted
    void markSoundCompleted(bool completed);

    bool _soundLoaded;

    // Does this sound have a live input stream?
    bool isAttached() const {
        return (_inputStream);
    }

};

Sound_as::Sound_as(as_object* owner) 
    :
    ActiveRelay(owner),
    _attachedCharacter(0),
    soundId(-1),
    externalSound(false),
    isStreaming(false),
    _soundHandler(getRunResources(*owner).soundHandler()),
    _mediaHandler(getRunResources(*owner).mediaHandler()),
    _startTime(0),
    _leftOverData(),
    _leftOverPtr(0),
    _leftOverSize(0),
    _inputStream(0),
    remainingLoops(0),
    _soundCompleted(false),
    _soundLoaded(false)
{
}

Sound_as::~Sound_as()
{
    // Just in case...
    if (_inputStream && _soundHandler) {
        _soundHandler->unplugInputStream(_inputStream);
        _inputStream=0;
    }

}

// extern (used by Global.cpp)
void
sound_class_init(as_object& where, const ObjectURI& uri)
{

    Global_as& gl = getGlobal(where);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(&sound_new, proto);
    attachSoundInterface(*proto);
    proto->set_member_flags(NSV::PROP_CONSTRUCTOR, PropFlags::readOnly);
    proto->set_member_flags(NSV::PROP_uuPROTOuu, PropFlags::readOnly, 0);

    where.init_member(uri, cl, as_object::DefaultFlags);
}

void
registerSoundNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(sound_getpan, 500, 0);
    vm.registerNative(sound_gettransform, 500, 1);
    vm.registerNative(sound_getvolume, 500, 2);
    vm.registerNative(sound_setpan, 500, 3);
    vm.registerNative(sound_settransform, 500, 4);
    vm.registerNative(sound_setvolume, 500, 5);
    vm.registerNative(sound_stop, 500, 6);
    vm.registerNative(sound_attachsound, 500, 7);
    vm.registerNative(sound_start, 500, 8);
    vm.registerNative(sound_getDuration, 500, 9);
    vm.registerNative(sound_setDuration, 500, 10);
    vm.registerNative(sound_getPosition, 500, 11);
    vm.registerNative(sound_setPosition, 500, 12);
    vm.registerNative(sound_loadsound, 500, 13);
    vm.registerNative(sound_getbytesloaded, 500, 14);
    vm.registerNative(sound_getbytestotal, 500, 15);
    vm.registerNative(sound_areSoundsInaccessible, 500, 16);
}

/*private*/
void
Sound_as::startProbeTimer()
{
    getRoot(owner()).addAdvanceCallback(this);
}

/*private*/
void
Sound_as::stopProbeTimer()
{
#ifdef GNASH_DEBUG_SOUND_AS
    log_debug("stopProbeTimer called");
#endif
    getRoot(owner()).removeAdvanceCallback(this);
}

void
Sound_as::update()
{
    probeAudio();

    if (active()) {
        owner().set_member(NSV::PROP_DURATION, getDuration());
        owner().set_member(NSV::PROP_POSITION, getPosition());
    }
}

void
Sound_as::probeAudio()
{
    if ( ! externalSound ) {
        // Only probe for sound complete
        assert(_soundHandler);
        assert(!_soundCompleted);
        if (!_soundHandler->isSoundPlaying(soundId)) {
            stopProbeTimer();
            // dispatch onSoundComplete 
            callMethod(&owner(), NSV::PROP_ON_SOUND_COMPLETE);
        }
        return;
    }

    if (!_mediaParser) return; // nothing to do here w/out a media parser

    if ( ! _soundLoaded ) {
#ifdef GNASH_DEBUG_SOUND_AS
        log_debug("Probing audio for load");
#endif
        if (_mediaParser->parsingCompleted()) {

            _soundLoaded = true;

            if (!isStreaming) {
                stopProbeTimer(); // will be re-started on Sound.start()
            }
            bool success = _mediaParser->getAudioInfo() != 0;
            callMethod(&owner(), NSV::PROP_ON_LOAD, success);

            // TODO: check if this should be called anyway.
            if (success) handleId3Data(_mediaParser->getId3Info(), owner());
        }
        return; 
    }

    if (isAttached()) {
#ifdef GNASH_DEBUG_SOUND_AS
        log_debug("Probing audio for end");
#endif

        boost::mutex::scoped_lock lock(_soundCompletedMutex);
        if (_soundCompleted) {
            // when _soundCompleted is true we're NOT attached !
            // MediaParser may be still needed,
            // if this is a non-streaming sound
            if ( isStreaming ) {
                _mediaParser.reset(); // no use for this anymore...
            }
            _inputStream = 0;
            _soundCompleted = false;
            stopProbeTimer();

            // dispatch onSoundComplete 
            callMethod(&owner(), NSV::PROP_ON_SOUND_COMPLETE);
        }
    }
    else {
#ifdef GNASH_DEBUG_SOUND_AS
        log_debug("Probing audio for start");
#endif

        bool parsingCompleted = _mediaParser->parsingCompleted();
        try {
            log_debug("Attaching aux streamer");
            _inputStream = attachAuxStreamerIfNeeded();
        } 
        catch (const MediaException& e) {
            assert(!_inputStream);
            assert(!_audioDecoder.get());
            log_error(_("Could not create audio decoder: %s"), e.what());
            _mediaParser.reset(); // no use for this anymore...
            stopProbeTimer();
            return;
        }

        if ( ! _inputStream ) {
            if ( parsingCompleted ) {
                log_error(_("No audio in Sound input."));
                stopProbeTimer();
                _mediaParser.reset(); // no use for this anymore...
            } else {
                // keep probing
            }
        } else {
            // An audio decoder was constructed, good!
            assert(_audioDecoder.get());
        }
    }
}

void
Sound_as::markReachableResources() const
{
    if (_attachedCharacter) {
        _attachedCharacter->setReachable();
    }
}

void
Sound_as::markSoundCompleted(bool completed)
{
    boost::mutex::scoped_lock lock(_soundCompletedMutex);
    _soundCompleted=completed;
}

void
Sound_as::attachCharacter(DisplayObject* attachTo) 
{
    _attachedCharacter.reset(new CharacterProxy(attachTo, getRoot(owner())));
}

void
Sound_as::attachSound(int si, const std::string& name)
{
    soundId = si;
    soundName = name;
    
    owner().set_member(NSV::PROP_DURATION, getDuration());
    owner().set_member(NSV::PROP_POSITION, getPosition());

}

long
Sound_as::getBytesLoaded()
{
    if ( _mediaParser ) {
        return _mediaParser->getBytesLoaded();
    }
    
    return -1;
}

long
Sound_as::getBytesTotal()
{
    if ( _mediaParser ) {
        return _mediaParser->getBytesTotal();
    }
    
    return -1;
}

void
Sound_as::getPan()
{
    LOG_ONCE(log_unimpl(__FUNCTION__));
}

void
Sound_as::getTransform()
{
    LOG_ONCE(log_unimpl(__FUNCTION__));
}

bool
Sound_as::getVolume(int& volume)
{
    // TODO: check what takes precedence in case we
    //       have both an attached DisplayObject *and*
    //       some other sound...
    //
    if ( _attachedCharacter ) {
        //log_debug("Sound has an attached DisplayObject");
        DisplayObject* ch = _attachedCharacter->get();
        if (! ch) {
            log_debug("Character attached to Sound was unloaded and "
                        "couldn't rebind");
            return false;
        }
        volume = ch->getVolume();
        return true;
    }

    // If we're not attached to a DisplayObject we'll need to query
    // sound_handler for volume. If we have no sound handler, we
    // can't do much, so we'll return false
    if (!_soundHandler) {
        log_debug("We have no sound handler here...");
        return false;
    }

    // Now, we may be controlling a specific sound or
    // the final output as a whole.
    // If soundId is -1 we're controlling as a whole
    //
    if (soundId == -1) {
        volume = _soundHandler->getFinalVolume();
    } else {
        volume = _soundHandler->get_volume(soundId);
    }

    return true;
}

void
Sound_as::loadSound(const std::string& file, bool streaming)
{
    if (!_mediaHandler || !_soundHandler) {
        log_debug("No media or sound handlers, won't load any sound");
        return;
    }

    /// If we are already streaming stop doing so as we'll replace
    /// the media parser
    if (_inputStream) {
        _soundHandler->unplugInputStream(_inputStream);
        _inputStream = 0;
    }
    
    /// Mark sound as not being loaded
    // TODO: should we check for _soundLoaded == true?
    _soundLoaded = false;

    /// Delete any media parser being used (make sure we have detached!)
    _mediaParser.reset();

    /// Start at offset 0, in case a previous ::start() call
    /// changed that.
    _startTime = 0;

    const RunResources& rr = getRunResources(owner());
    URL url(file, rr.streamProvider().baseURL());

    const RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    const StreamProvider& streamProvider = rr.streamProvider();
    std::auto_ptr<IOChannel> inputStream(streamProvider.getStream(url,
                rcfile.saveStreamingMedia()));

    if (!inputStream.get()) {
        log_error(_("Gnash could not open this URL: %s"), url );
        // dispatch onLoad (false)
        callMethod(&owner(), NSV::PROP_ON_LOAD, false);
        return;
    }

    externalSound = true;
    isStreaming = streaming;

    _mediaParser.reset(_mediaHandler->createMediaParser(inputStream).release());
    if (!_mediaParser) {
        log_error(_("Unable to create parser for Sound at %s"), url);
        // not necessarely correct, the stream might have been found...
        // dispatch onLoad (false)
        callMethod(&owner(), NSV::PROP_ON_LOAD, false);
        return;
    }

    // TODO: use global _soundbuftime
    _mediaParser->setBufferTime(60000); // one minute buffer... should be fine

    startProbeTimer();

    owner().set_member(NSV::PROP_DURATION, getDuration());
    owner().set_member(NSV::PROP_POSITION, getPosition());
}

sound::InputStream*
Sound_as::attachAuxStreamerIfNeeded()
{
    media::AudioInfo* audioInfo =  _mediaParser->getAudioInfo();
    if (!audioInfo) return 0;

    // the following may throw an exception
    _audioDecoder.reset(_mediaHandler->createAudioDecoder(*audioInfo).release());

    // start playing ASAP, a call to ::start will just change _startTime
#ifdef GNASH_DEBUG_SOUND_AS
    log_debug("Attaching the aux streamer");
#endif
    return _soundHandler->attach_aux_streamer(getAudioWrapper, (void*) this);
}

void
Sound_as::setPan()
{
    LOG_ONCE(log_unimpl(__FUNCTION__));
}

void
Sound_as::setTransform()
{
    LOG_ONCE(log_unimpl(__FUNCTION__));
}

void
Sound_as::setVolume(int volume)
{
    // TODO: check what takes precedence in case we
    //       have both an attached DisplayObject *and*
    //       some other sound...
    //
    if ( _attachedCharacter ) {
        DisplayObject* ch = _attachedCharacter->get();
        if ( ! ch ) {
            log_debug("Character attached to Sound was unloaded and "
                      "couldn't rebind");
            return;
        }
        ch->setVolume(volume);
        return;
    }

    // If we're not attached to a DisplayObject we'll need to use
    // sound_handler for volume. If we have no sound handler, we
    // can't do much, so we'll just return
    if (!_soundHandler) {
        return;
    }

    // Now, we may be controlling a specific sound or
    // the final output as a whole.
    // If soundId is -1 we're controlling as a whole
    //
    if ( soundId == -1 ) {
        _soundHandler->setFinalVolume(volume);
    } else {
        _soundHandler->set_volume(soundId, volume);
    }
}

void
Sound_as::start(double secOff, int loops)
{
    if ( ! _soundHandler ) {
        log_error(_("No sound handler, nothing to start..."));
        return;
    }

    if (externalSound) {
        if ( ! _mediaParser ) {
            log_error(_("No MediaParser initialized, can't start an external sound"));
            return;
        }

        if (isStreaming) {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Sound.start() has no effect on a streaming Sound"));
            );
            return;
        }

        // Always seek as we might be called during or after some playing...
        {
            _startTime = secOff * 1000;
            boost::uint32_t seekms = boost::uint32_t(secOff * 1000);
            // TODO: boost::mutex::scoped_lock parserLock(_parserMutex);
            bool seeked = _mediaParser->seek(seekms); // well, we try...
            log_debug("Seeked MediaParser to %d, returned: %d", seekms, seeked);
        }


        // Save how many loops to do (not when streaming)
        if (loops > 0) {
            remainingLoops = loops;
        }

        startProbeTimer();

    } else {
        unsigned int inPoint = 0;

        if ( secOff > 0 ) {
            inPoint = (secOff*44100);
        }

        log_debug("Sound.start: secOff:%d", secOff);

        _soundHandler->startSound(
                    soundId,
                    loops,
                    0, // envelopes
                    true, // allow multiple instances (checked)
                    inPoint
                    );

        startProbeTimer(); // to dispatch onSoundComplete
    }
}

void
Sound_as::stop(int si)
{
    if ( ! _soundHandler ) {
        log_error(_("No sound handler, nothing to stop..."));
        return;
    }

    // stop the sound
    if (si < 0) {
        if (externalSound) {
            if ( _inputStream ) {
                _soundHandler->unplugInputStream(_inputStream);
                _inputStream=0;
            }
        } else {
            if ( ! _attachedCharacter ) {
                // See https://savannah.gnu.org/bugs/index.php?33888
                _soundHandler->stopAllEventSounds();
            } else {
                _soundHandler->stopEventSound(soundId);
            }
        }
    } else {
        _soundHandler->stopEventSound(si);
    }
}

size_t
Sound_as::getDuration() const
{
    if ( ! _soundHandler ) {
        log_error(_("No sound handler, can't check duration..."));
        return 0;
    }

    // If this is a event sound get the info from the soundhandler
    if (!externalSound) {
        return _soundHandler->get_duration(soundId);
    }

    // If we have a media parser (we'd do for an externalSound)
    // try fetching duration from it
    if ( _mediaParser ) {
        media::AudioInfo* info = _mediaParser->getAudioInfo();
        if ( info ) {
            return info->duration;
        }
    }

    return 0;
}

size_t
Sound_as::getPosition() const
{
    if (!_soundHandler) {
        log_error(_("No sound handler, can't check position (we're "
                    "likely not playing anyway)..."));
        return 0;
    }

    // If this is a event sound get the info from the soundhandler
    if (!externalSound) {
        return _soundHandler->tell(soundId);
    }

    if (_mediaParser) {
        boost::uint64_t ts;
        if ( _mediaParser->nextAudioFrameTimestamp(ts) ) {
            return ts;
        }
    }

    return 0;

}


unsigned int
Sound_as::getAudio(boost::int16_t* samples, unsigned int nSamples, bool& atEOF)
{
    boost::uint8_t* stream = reinterpret_cast<boost::uint8_t*>(samples);
    int len = nSamples*2;

    //GNASH_REPORT_FUNCTION;

    while (len) {
        if ( ! _leftOverData ) {
            bool parsingComplete = _mediaParser->parsingCompleted(); // check *before* calling nextAudioFrame
            std::auto_ptr<media::EncodedAudioFrame> frame = _mediaParser->nextAudioFrame();
            if ( ! frame.get() ) {
                // just wait some more if parsing isn't complete yet
                if ( ! parsingComplete ) {
                    //log_debug("Parsing not complete and no more audio frames in input, try again later");
                    break;
                }

                // or detach and stop here...
                // (should really honour loopings if any,
                // but that should be only done for non-streaming sound!)
                //log_debug("Parsing complete and no more audio frames in input, detaching");

                markSoundCompleted(true);

                // Setting atEOF to true will detach us.
                // We should change _inputStream, but need thread safety!
                // So on probeAudio, if _soundCompleted is set
                // we'll consider ourselves detached already and set
                // _inputStream to zero
                atEOF=true;
                return nSamples-(len/2);
            }

            // if we've been asked to start at a specific time, skip
            // any frame with earlier timestamp
            if ( frame->timestamp < _startTime ) {
                //log_debug("This audio frame timestamp (%d) < requested start time (%d)", frame->timestamp, _startTime);
                continue;
            }

            _leftOverData.reset( _audioDecoder->decode(*frame, _leftOverSize) );
            _leftOverPtr = _leftOverData.get();
            if ( ! _leftOverData ) {
                log_error(_("No samples decoded from input of %d bytes"),
                          frame->dataSize);
                continue;
            }

            //log_debug(" decoded %d bytes of audio", _leftOverSize);
        }

        assert( !(_leftOverSize%2) );

        int n = std::min<int>(_leftOverSize, len);
        //log_debug(" consuming %d bytes of decoded audio", n);

        std::copy(_leftOverPtr, _leftOverPtr+n, stream);

        stream += n;
        _leftOverPtr += n;
        _leftOverSize -= n;
        len -= n;

        if (_leftOverSize == 0) {
            _leftOverData.reset();
            _leftOverPtr = 0;
        }

    }

    // drop any queued video frame
    while (_mediaParser->nextVideoFrame().get()) {};

    atEOF=false;
    return nSamples-(len/2);
}

// audio callback is running in sound handler thread
unsigned int
Sound_as::getAudioWrapper(void* owner, boost::int16_t* samples,
        unsigned int nSamples, bool& atEOF)
{
    Sound_as* so = static_cast<Sound_as*>(owner);
    return so->getAudio(samples, nSamples, atEOF);
}


namespace {

void
attachSoundInterface(as_object& o)
{

    int flags = PropFlags::dontEnum | 
                PropFlags::dontDelete | 
                PropFlags::readOnly;

    VM& vm = getVM(o);
    o.init_member("getPan", vm.getNative(500, 0), flags);
    o.init_member("getTransform", vm.getNative(500, 1), flags);
    o.init_member("getVolume", vm.getNative(500, 2), flags);
    o.init_member("setPan", vm.getNative(500, 3), flags);
    o.init_member("setTransform", vm.getNative(500, 4), flags);
    o.init_member("setVolume", vm.getNative(500, 5), flags);
    o.init_member("stop", vm.getNative(500, 6), flags);
    o.init_member("attachSound", vm.getNative(500, 7), flags);
    o.init_member("start", vm.getNative(500, 8), flags);

    int flagsn6 = flags | PropFlags::onlySWF6Up;

    o.init_member("getDuration", vm.getNative(500, 9), flagsn6);
    o.init_member("setDuration", vm.getNative(500, 10), flagsn6);
    o.init_member("getPosition", vm.getNative(500, 11), flagsn6); 
    o.init_member("setPosition", vm.getNative(500, 12), flagsn6);
    o.init_member("loadSound", vm.getNative(500, 13), flagsn6);
    o.init_member("getBytesLoaded", vm.getNative(500, 14), flagsn6); 
    o.init_member("getBytesTotal", vm.getNative(500, 15), flagsn6);

    int flagsn9 = PropFlags::dontEnum | 
                  PropFlags::dontDelete | 
                  PropFlags::readOnly | 
                  PropFlags::onlySWF9Up;

    o.init_member("areSoundsInaccessible", vm.getNative(500, 16), flagsn9);

    int fl_hp = PropFlags::dontEnum | PropFlags::dontDelete;

    o.init_property("checkPolicyFile", &checkPolicyFile_getset, 
            &checkPolicyFile_getset, fl_hp);
}


as_value
sound_new(const fn_call& fn)
{
    as_object* so = ensure<ValidThis>(fn);
    Sound_as* s(new Sound_as(so));
    so->setRelay(s);

    if (fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            if (fn.nargs > 1) {
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("new Sound(%d) : args after first one ignored"),
                    ss.str());
            }
        );

        const as_value& arg0 = fn.arg(0);

        if (!arg0.is_null() && !arg0.is_undefined()) {

            as_object* obj = toObject(arg0, getVM(fn));
            DisplayObject* ch = get<DisplayObject>(obj);
            IF_VERBOSE_ASCODING_ERRORS(
                if (!ch) {
                    std::stringstream ss; fn.dump_args(ss);
                    log_aserror(_("new Sound(%s) : first argument isn't null "
                        "or undefined, and isn't a DisplayObject. "
                                  "We'll take as an invalid DisplayObject ref."),
                        ss.str());
                }
            );

            s->attachCharacter(ch);
        }
    }
       
    return as_value();
}

as_value
sound_start(const fn_call& fn)
{
    IF_VERBOSE_ACTION (
    log_action(_("-- start sound"));
    )
    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);
    int loop = 0;
    double secondOffset = 0;

    if (fn.nargs > 0) {
        secondOffset = toNumber(fn.arg(0), getVM(fn));

        if (fn.nargs > 1) {
            loop = (int) toNumber(fn.arg(1), getVM(fn)) - 1;

            // -1 means infinite playing of sound
            // sanity check
            loop = loop < 0 ? -1 : loop;
        }
    }
    so->start(secondOffset, loop);
    return as_value();
}

as_value
sound_stop(const fn_call& fn)
{
    IF_VERBOSE_ACTION (
    log_action(_("-- stop sound "));
    )
    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);

    int si = -1;

    if (fn.nargs > 0) {
        const std::string& name = fn.arg(0).to_string();

        // check the import.
        const movie_definition* def = fn.callerDef;
        assert(def);

        const boost::uint16_t id = def->exportID(name);
        if (!id) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("No such export '%s'"),
                    name);
                );
            return as_value();
        }

        sound_sample* ss = def->get_sound_sample(id);
        if (!ss) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("Export '%s' is not a sound"), name);
                );
            return as_value();
        }

        si = ss->m_sound_handler_id;
    }
    
    so->stop(si);
    return as_value();
}

as_value
sound_attachsound(const fn_call& fn)
{
    IF_VERBOSE_ACTION(
        log_action(_("-- attach sound"));
    )

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("attach sound needs one argument"));
            );
        return as_value();
    }

    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);

    const std::string& name = fn.arg(0).to_string();
    if (name.empty()) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("attachSound needs a non-empty string"));
        );
        return as_value();
    }

    // check the import.
    // NOTE: we should be checking in the SWF containing the calling code
    // (see 'winter bell' from orisinal morning sunshine for a testcase)
    const movie_definition* def = fn.callerDef;
    assert(def);


    const boost::uint16_t id = def->exportID(name);
    if (!id) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("No such export '%s'"),
                name);
            );
        return as_value();
    }

    sound_sample* ss = def->get_sound_sample(id);
    if (!ss) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Export '%s'is not a sound"), name);
            );
        return as_value();
    }

    const int si = ss->m_sound_handler_id;

    // sanity check
    assert(si >= 0);
    so->attachSound(si, name);

    return as_value();
}

as_value
sound_getbytesloaded(const fn_call& fn)
{
    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);
    long loaded = so->getBytesLoaded();
    if (loaded < 0) return as_value();
    return as_value(loaded);
}

as_value
sound_getbytestotal(const fn_call& fn)
{
    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);
    long total = so->getBytesTotal();
    if (total < 0) return as_value();
    return as_value(total);
}

as_value
sound_getpan(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("Sound.getPan()")));
    return as_value();
}

as_value
sound_getDuration(const fn_call& fn)
{
    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);
    if (!so->active()) return as_value();
    return as_value(so->getDuration());
}

as_value
sound_setDuration(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("Sound.setDuration()")));
    return as_value();
}

as_value
sound_getPosition(const fn_call& fn)
{
    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);
    if (!so->active()) return as_value();
    return as_value(so->getPosition());
}

as_value
sound_setPosition(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("Sound.setPosition()")));
    return as_value();
}

as_value
sound_gettransform(const fn_call& /*fn*/)
{
    LOG_ONCE( log_unimpl(_("Sound.getTransform()")));
    return as_value();
}

as_value
sound_getvolume(const fn_call& fn)
{

    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);

    if ( fn.nargs ) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Sound.getVolume(%s) : arguments ignored"));
        );
    }

    int volume;
    if (so->getVolume(volume)) return as_value(volume);
    return as_value();
}

as_value
sound_loadsound(const fn_call& fn)
{
    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Sound.loadSound() needs at least 1 argument"));
            );
        return as_value();      
    }

    std::string url = fn.arg(0).to_string();

    bool streaming = false;
    if ( fn.nargs > 1 ) {
        streaming = toBool(fn.arg(1), getVM(fn));

        IF_VERBOSE_ASCODING_ERRORS(
        if ( fn.nargs > 2 )
        {
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("Sound.loadSound(%s): arguments after first 2 "
                    "discarded"), ss.str());
        }
        );
    }

    so->loadSound(url, streaming);

    return as_value();
}

as_value
sound_setpan(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("Sound.setPan()")));
    return as_value();
}

as_value
sound_settransform(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("Sound.setTransform()")));
    return as_value();
}

as_value
sound_setvolume(const fn_call& fn)
{
    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("set volume of sound needs one argument"));
        );
        return as_value();
    }

    Sound_as* so = ensure<ThisIsNative<Sound_as> >(fn);
    int volume = (int) toNumber(fn.arg(0), getVM(fn));

    so->setVolume(volume);
    return as_value();
}

as_value
checkPolicyFile_getset(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(_("Sound.checkPolicyFile")));
    return as_value();
}

as_value
sound_areSoundsInaccessible(const fn_call& /*fn*/)
{
    // TODO: I guess this would have to do with permissions (crossdomain stuff)
    // more then capability.
    // See http://www.actionscript.org/forums/showthread.php3?t=160028
    // 
    // naive test shows this always being undefined..
    //
    LOG_ONCE(log_unimpl(_("Sound.areSoundsInaccessible()")));
    return as_value();
}

void
handleId3Data(boost::optional<media::Id3Info> id3, as_object& sound)
{
    if (!id3) return;
    VM& vm = getVM(sound);

    as_object* o = new as_object(getGlobal(sound));

    // TODO: others.
    if (id3->album) o->set_member(getURI(vm, "album"), *id3->album);
    if (id3->year) o->set_member(getURI(vm, "year"), *id3->year);

    // Add Sound.id3 member
    const ObjectURI& id3prop = getURI(vm, "id3");
    sound.set_member(id3prop, o);

    // Notify onID3 function.
    const ObjectURI& onID3 = getURI(vm, "onID3");
    callMethod(&sound, onID3);
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

