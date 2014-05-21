//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "sound_handler.h"

#include <cstdint> // For C99 int types
#include <vector> 
#include <cmath> 

#include "EmbedSound.h" // for use
#include "InputStream.h" // for use
#include "EmbedSoundInst.h" // for upcasting to InputStream
#include "log.h" // for use
#include "StreamingSound.h"
#include "StreamingSoundData.h"
#include "SimpleBuffer.h"
#include "MediaHandler.h"

// Debug create_sound/delete_sound/playSound/stop_sound, loops
//#define GNASH_DEBUG_SOUNDS_MANAGEMENT

// Debug samples fetching
//#define GNASH_DEBUG_SAMPLES_FETCHING 1

namespace gnash {
namespace sound {

namespace {

unsigned int
silentStream(void*, std::int16_t* stream, unsigned int len, bool& atEOF)
{
    std::fill(stream, stream + len, 0);
    atEOF=false;
    return len;
}

template<typename T>
bool
validHandle(const T& container, int handle)
{
    return handle >= 0 && static_cast<size_t>(handle) < container.size();
}

/// Ensure that each buffer has appropriate padding for the decoder.
//
/// Note: all callers passing a SimpleBuffer should already do this,
/// so this is a paranoid check.
void
ensurePadding(SimpleBuffer& data, media::MediaHandler* m)
{
    const size_t padding = m ? m->getInputPaddingSize() : 0;
    if (data.capacity() - data.size() < padding) {
        log_error(_("Sound data creator didn't appropriately pad "
                    "buffer. We'll do so now, but will cost memory copies."));
        data.reserve(data.size() + padding);
    }
}

} // anonymous namespace

sound_handler::StreamBlockId
sound_handler::addSoundBlock(std::unique_ptr<SimpleBuffer> data,
        size_t sampleCount, int seekSamples, int handle)
{
    if (!validHandle(_streamingSounds, handle)) {
        log_error(_("Invalid (%d) handle passed to fill_stream_data, "
                    "doing nothing"), handle);
        return -1;
    }

    StreamingSoundData* sounddata = _streamingSounds[handle];
    if (!sounddata) {
        log_error(_("handle passed to fill_stream_data (%d) "
                    "was deleted"), handle);
        return -1;
    }

    assert(data.get());
    ensurePadding(*data, _mediaHandler);

    return sounddata->append(std::move(data), sampleCount, seekSamples);
}

void
sound_handler::delete_all_sounds()
{
    for (Sounds::iterator i = _sounds.begin(),
                          e = _sounds.end(); i != e; ++i)
    {
        EmbedSound* sdef = *i;

        // The sound may have been deleted already.
        if (!sdef) continue;

        stopEmbedSoundInstances(*sdef);
        assert(!sdef->numPlayingInstances());

        delete sdef; 
    }
    _sounds.clear();

    for (StreamingSounds::iterator i = _streamingSounds.begin(),
                          e = _streamingSounds.end(); i != e; ++i)
    {
        StreamingSoundData* sdef = *i;

        // Streaming sounds are never deleted.
        assert(sdef);

        stopEmbedSoundInstances(*sdef);
        assert(!sdef->numPlayingInstances());

        delete sdef; 
    }
    _streamingSounds.clear();

}

void
sound_handler::delete_sound(int handle)
{
    // Check if the sound exists
    if (!validHandle(_sounds, handle)) {
        log_error(_("Invalid (%d) handle passed to delete_sound, "
                    "doing nothing"), handle);
        return;
    }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("deleting sound :%d", handle);
#endif

    EmbedSound* def = _sounds[handle];
    if (!def) {
        log_error(_("handle passed to delete_sound (%d) "
                    "already deleted"), handle);
        return;
    }
    
    stopEmbedSoundInstances(*def);
    delete def;
    _sounds[handle] = nullptr;

}

void   
sound_handler::stop_all_sounds()
{
    for (Sounds::iterator i = _sounds.begin(),
                          e = _sounds.end(); i != e; ++i)
    {
        EmbedSound* sounddata = *i;
        if ( ! sounddata ) continue; // could have been deleted already
        stopEmbedSoundInstances(*sounddata);
    }

    for (StreamingSounds::iterator i = _streamingSounds.begin(),
                          e = _streamingSounds.end(); i != e; ++i)
    {
        StreamingSoundData* sounddata = *i;
        if (!sounddata) continue; 
        stopEmbedSoundInstances(*sounddata);
    }
}

int
sound_handler::get_volume(int handle) const
{
    if (validHandle(_sounds, handle)) return _sounds[handle]->volume; 

    // Invalid handle.
    return 0;
}

void   
sound_handler::set_volume(int handle, int volume)
{
    // Set volume for this sound.
    // Should this only apply to the active sounds?
    if (validHandle(_sounds, handle)) _sounds[handle]->volume = volume;
}

media::SoundInfo*
sound_handler::get_sound_info(int handle) const
{
    // Check if the sound exists.
    if (validHandle(_streamingSounds, handle)) {
        return &_streamingSounds[handle]->soundinfo;
    } 
    return nullptr;
}

void
sound_handler::stopStreamingSound(int handle)
{
    // Check if the sound exists.
    if (!validHandle(_streamingSounds, handle)) {
        log_debug("stop_sound(%d): invalid sound id", handle);
        return;
    }
    
    StreamingSoundData* sounddata = _streamingSounds[handle];
    assert(sounddata);

    stopEmbedSoundInstances(*sounddata);
}

void
sound_handler::stopEventSound(int handle)
{
    // Check if the sound exists.
    if (!validHandle(_sounds, handle)) {
        log_debug("stop_sound(%d): invalid sound id", handle);
        return;
    }
    
    EmbedSound* sounddata = _sounds[handle];
    if (!sounddata) {
        log_error(_("stop_sound(%d): sound was deleted"), handle);
        return;
    }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("stop_sound %d called", handle);
#endif

    stopEmbedSoundInstances(*sounddata);

}

void
sound_handler::stopAllEventSounds()
{
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("stopAllEventSounds called");
#endif

    for (Sounds::iterator i=_sounds.begin(), e=_sounds.end(); i != e; ++i)
    {
        EmbedSound* sounddata = *i;
        if (!sounddata) continue; // possible ?

        stopEmbedSoundInstances(*sounddata);
    }

}

void   
sound_handler::stopEmbedSoundInstances(StreamingSoundData& def)
{
    // Assert _mutex is locked ...
    typedef std::vector<InputStream*> InputStreamVect;
    InputStreamVect playing;
    def.getPlayingInstances(playing);

    // Now, for each playing InputStream, unplug it!
    // NOTE: could be optimized...
    for (InputStreamVect::iterator i=playing.begin(), e=playing.end();
            i!=e; ++i)
    {
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
        log_debug(" unplugging input stream %p from stopEmbedSoundInstances", *i);
#endif

        // Explicitly calling the base class implementation
        // is a (dirty?) way to avoid mutex-locking overrides
        // in subclasses causing deadlocks.
        sound_handler::unplugInputStream(*i);
    }

    def.clearInstances();

}

void   
sound_handler::stopEmbedSoundInstances(EmbedSound& def)
{
    // Assert _mutex is locked ...
    typedef std::vector<InputStream*> InputStreamVect;
    InputStreamVect playing;
    def.getPlayingInstances(playing);

    // Now, for each playing InputStream, unplug it!
    // NOTE: could be optimized...
    for (InputStreamVect::iterator i=playing.begin(), e=playing.end();
            i!=e; ++i)
    {
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
        log_debug(" unplugging input stream %p from stopEmbedSoundInstances", *i);
#endif

        // Explicitly calling the base class implementation
        // is a (dirty?) way to avoid mutex-locking overrides
        // in subclasses causing deadlocks.
        sound_handler::unplugInputStream(*i);
    }

    def.clearInstances();
}

void
sound_handler::unplugInputStream(InputStream* id)
{
    // WARNING: erasing would break any iteration in the set
    InputStreams::iterator it2=_inputStreams.find(id);
    if (it2 == _inputStreams.end()) {
        log_error(_("SDL_sound_handler::unplugInputStream: "
                    "Aux streamer %p not found. "),
                id);
        return; // we won't delete it, as it's likely deleted already
    }

    _inputStreams.erase(it2);

    // Increment number of sound stop request for the testing framework
    _soundsStopped++;

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("Unplugged InputStream %p", id);
#endif

    // Delete the InputStream (we own it..)
    delete id;
}

unsigned int
sound_handler::tell(int handle) const
{
    // Check if the sound exists.
    if (!validHandle(_sounds, handle)) return 0;

    const EmbedSound* sounddata = _sounds[handle];

    // If there is no active sounds, return 0
    if (!sounddata->isPlaying()) return 0;

    // We use the first active sound of this.
    const InputStream* asound = sounddata->firstPlayingInstance();

    // Return the playhead position in milliseconds
    unsigned int samplesPlayed = asound->samplesFetched();

    unsigned int ret = samplesPlayed / 44100 * 1000;
    ret += ((samplesPlayed % 44100) * 1000) / 44100;
    ret = ret / 2; // 2 channels 
    return ret;
}

unsigned int
sound_handler::get_duration(int handle) const
{
    // Check if the sound exists.
    if (!validHandle(_sounds, handle)) return 0;

    const EmbedSound* sounddata = _sounds[handle];

    const std::uint32_t sampleCount = sounddata->soundinfo.getSampleCount();
    const std::uint32_t sampleRate = sounddata->soundinfo.getSampleRate();

    // Return the sound duration in milliseconds
    if (sampleCount > 0 && sampleRate > 0) {
        // TODO: should we cache this in the EmbedSound object ?
        unsigned int ret = sampleCount / sampleRate * 1000;
        ret += ((sampleCount % sampleRate) * 1000) / sampleRate;
        return ret;
    } 
    return 0;
}

int
sound_handler::createStreamingSound(const media::SoundInfo& sinfo)
{
    std::unique_ptr<StreamingSoundData> sounddata(
            new StreamingSoundData(sinfo, 100));

    int sound_id = _streamingSounds.size();
    // the vector takes ownership
    _streamingSounds.push_back(sounddata.release());

    return sound_id;
}

int
sound_handler::create_sound(std::unique_ptr<SimpleBuffer> data,
                            const media::SoundInfo& sinfo)
{
    if (data.get()) {
        ensurePadding(*data, _mediaHandler);
    }
    else {
        log_debug("Event sound with no data!");
    }
    std::unique_ptr<EmbedSound> sounddata(new EmbedSound(std::move(data), sinfo, 100));

    int sound_id = _sounds.size();

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("create_sound: sound %d, format %s %s %dHz, %d samples (%d bytes)",
        sound_id, sounddata->soundinfo.getFormat(),
        sounddata->soundinfo.isStereo() ? "stereo" : "mono",
        sounddata->soundinfo.getSampleRate(),
        sounddata->soundinfo.getSampleCount(),
        sounddata->size());
#endif

    // the vector takes ownership
    _sounds.push_back(sounddata.release());

    return sound_id;

}

bool
sound_handler::isSoundPlaying(int handle) const
{
    if (!validHandle(_sounds, handle)) return false;

    EmbedSound& sounddata = *(_sounds[handle]);

    // When this is called from a StreamSoundBlockTag,
    // we only start if this sound isn't already playing.
    return sounddata.isPlaying();
}

void
sound_handler::playStream(int soundId, StreamBlockId blockId)
{
    StreamingSoundData& s = *_streamingSounds[soundId];
    if (s.isPlaying() || s.empty()) return;

    try {
        std::unique_ptr<InputStream> is(
                s.createInstance(*_mediaHandler, blockId));
        plugInputStream(std::move(is));
    }
    catch (const MediaException& e) {
        log_error(_("Could not start streaming sound: %s"), e.what());
    }
}

void
sound_handler::startSound(int handle, int loops, const SoundEnvelopes* env,
	               bool allowMultiple, unsigned int inPoint,
                   unsigned int outPoint)
{
    // Check if the sound exists
    if (!validHandle(_sounds, handle)) {
        log_error(_("Invalid (%d) sound_handle passed to startSound, "
                    "doing nothing"), handle);
        return;
    }

    // Handle delaySeek
    EmbedSound& sounddata = *(_sounds[handle]);
    const media::SoundInfo& sinfo = sounddata.soundinfo;

    const int swfDelaySeek = sinfo.getDelaySeek(); 
    if (swfDelaySeek) {
        // NOTE: differences between delaySeek and inPoint:
        //
        //      - Sample count semantic:
        //        inPoint uses output sample rate (44100 for one second)
        //        while delaySeek uses source sample rate
        //        (SoundInfo.getSampleRate() for one second)
        //
        //      - Loop-back semantic:
        //        An event sound always loops-back from inPoint with no gaps
        //        When delaySeek is specified it is still used as a start
        //        for the initial playback but when it comes to looping back
        //        it seems to play silence instead of samples for the amount
        //        skipped:
        //
        //               [ delaySeekTime ]
        //        loop1                  *****************
        //        loop2  ----------------*****************
        //        loop3  ----------------*****************
        //
        //               [ inPoint ]
        //        loop1             *****************
        //        loop2             *****************
        //        loop3             *****************
        //
        LOG_ONCE(log_unimpl("MP3 delaySeek"));
#if 0
        unsigned int outDelaySeek = swfToOutSamples(sinfo, swfDelaySeek);

        log_debug("inPoint(%d) + delaySeek(%d -> %d) == %d",
                  inPoint, swfDelaySeek, outDelaySeek,
                  inPoint+outDelaySeek);

        inPoint += outDelaySeek;
#endif
    }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("startSound %d called, SoundInfo format is %s",
            handle, sounddata.soundinfo.getFormat());
#endif

    // When this is called from a StreamSoundBlockTag,
    // we only start if this sound isn't already playing.
    if (!allowMultiple && sounddata.isPlaying()) {
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
        log_debug(" playSound: multiple instances not allowed, "
                  "and sound is already playing");
#endif
        return;
    }

    // Make sure sound actually got some data
    if (sounddata.empty()) {
        // @@ should this be a log_error ? or even an assert ?
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Trying to play sound with size 0"));
        );
        return;
    }

    try {
        // Make an InputStream for this sound and plug it into  
        // the set of InputStream channels
        std::unique_ptr<InputStream> sound(
                sounddata.createInstance(*_mediaHandler, inPoint, outPoint,
                    env, loops));
        plugInputStream(std::move(sound));
    }
    catch (const MediaException& e) {
        log_error(_("Could not start event sound: %s"), e.what());
    }

}

void
sound_handler::plugInputStream(std::unique_ptr<InputStream> newStreamer)
{
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    InputStream* newStream = newStreamer.get(); // for debugging
#endif

    if (!_inputStreams.insert(newStreamer.release()).second) {
        // this should never happen !
        log_error(_("_inputStreams container still has a pointer "
                    "to deleted InputStream %p!"), newStreamer.get());
        // FIXME: replace the old element with the new one !
        abort();
    }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("Plugged InputStream %p", newStream);
#endif

    // Increment number of sound start request for the testing framework
    ++_soundsStarted;
}

void
sound_handler::unplugAllInputStreams()
{
    for (InputStreams::iterator it=_inputStreams.begin(),
                                itE=_inputStreams.end();
            it != itE; ++it)
    {
        delete *it;
    }
    _inputStreams.clear();
}

void
sound_handler::fetchSamples(std::int16_t* to, unsigned int nSamples)
{
    if (isPaused()) return; // should we write wav file anyway ?

    float finalVolumeFact = getFinalVolume()/100.0;

    std::fill(to, to + nSamples, 0);

    // call NetStream or Sound audio callbacks
    if (!_inputStreams.empty()) {

        // A buffer to fetch InputStream samples into
        std::unique_ptr<std::int16_t[]> buf(new std::int16_t[nSamples]);

#ifdef GNASH_DEBUG_SAMPLES_FETCHING 
        log_debug("Fetching %d samples from each of %d input streams", nSamples, _inputStreams.size());
#endif

        // Loop through the aux streamers sounds
        for (InputStreams::iterator it=_inputStreams.begin(),
                                    end=_inputStreams.end();
                                    it != end; ++it)
        {
            InputStream* is = *it;

            unsigned int wrote = is->fetchSamples(buf.get(), nSamples);
            if (wrote < nSamples) {
                // fill what wasn't written
                std::fill(buf.get()+wrote, buf.get()+nSamples, 0);
            }

#if GNASH_DEBUG_SAMPLES_FETCHING > 1
            log_debug("  fetched %d/%d samples from input stream %p"
                    " (%d samples fetchehd in total)",
                    wrote, nSamples, is, is->samplesFetched());
#endif

            mix(to, buf.get(), nSamples, finalVolumeFact);
        }

        unplugCompletedInputStreams();
    }

    // TODO: move this to base class !
    if (_wavWriter.get()) {
        _wavWriter->pushSamples(to, nSamples);

        // now, mute all audio
        std::fill(to, to+nSamples, 0);
    }

    // Now, after having "consumed" all sounds, blank out
    // the buffer if muted..
    if (is_muted()) {
        std::fill(to, to+nSamples, 0);
    }
}

void
sound_handler::setAudioDump(const std::string& wavefile)
{
    bool wasDumping = (_wavWriter.get() != nullptr);

    if (!wavefile.empty()) {
        _wavWriter.reset(new WAVWriter(wavefile));
    }

    // TODO: just avoid pausing instead ...
    if (!wasDumping) {
        // add a silent stream to the audio pool so that our
        // output file is homogenous;  we actually want silent
        // wave data when no sounds are playing on the stage
        attach_aux_streamer(silentStream, (void*) this);
    }
}

bool
sound_handler::streamingSound() const
{
    if (_inputStreams.empty()) return false;

    for (StreamingSounds::const_iterator it = _streamingSounds.begin(), 
            e = _streamingSounds.end(); it != e; ++it) {
        if ((*it)->isPlaying()) return true;
    }
    return false;
}

int 
sound_handler::getStreamBlock(int handle) const
{
    if (!validHandle(_streamingSounds, handle)) return -1;
    if (!_streamingSounds[handle]->isPlaying()) return -1;
    InputStream* i = _streamingSounds[handle]->firstPlayingInstance();
    if (!i) return -1;
    return static_cast<StreamingSound*>(i)->currentBlock();
}

void
sound_handler::unplugCompletedInputStreams()
{

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
    log_debug("Scanning %d input streams for completion", _inputStreams.size());
#endif

    for (InputStreams::iterator it = _inputStreams.begin(),
            end = _inputStreams.end(); it != end;) {

        InputStream* is = *it;

        // On EOF, detach
        if (is->eof()) {
            // InputStream EOF, detach
            InputStreams::iterator it2 = it;
            ++it2; // before we erase it
            InputStreams::size_type erased = _inputStreams.erase(is);
            if ( erased != 1 ) {
                log_error(_("Expected 1 InputStream element, found %d"), erased);
                abort();
            }
            it = it2;

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
            log_debug(" Input stream %p reached EOF, unplugging", is);
#endif

            // WARNING! deleting the InputStream here means
            //          a lot of things will happen from a
            //          separate thread. Instead, if we
            //          extend sound_handler interface to
            //          have an unplugCompletedInputStreams
            //          we may call it at heart-beating intervals
            //          and drop any threading paranoia!
            delete is;

            // Increment number of sound stop request for the testing framework
            ++_soundsStopped;
        }
        else ++it;
    }
}

bool
sound_handler::hasInputStreams() const
{
    return !_inputStreams.empty();
}

bool
sound_handler::is_muted() const
{
    // TODO: lock a mutex ?
    return _muted;
}

void
sound_handler::mute()
{
    // TODO: lock a mutex ?
    _muted=true;
}

void
sound_handler::unmute()
{
    // TODO: lock a mutex ?
    _muted=false;
}

void
sound_handler::reset()
{
    // Do not delete sounds on reset or there'd be nothing to play
    // on restart. For a new SWF, we need a new sound_handler.
    sound_handler::stop_all_sounds();
}

InputStream*
sound_handler::attach_aux_streamer(aux_streamer_ptr ptr, void* owner)
{
    assert(owner);
    assert(ptr);

    std::unique_ptr<InputStream> newStreamer ( new AuxStream(ptr, owner) );

    InputStream* ret = newStreamer.get();

    plugInputStream(std::move(newStreamer));

    return ret;
}

sound_handler::~sound_handler()
{
    delete_all_sounds();
    unplugAllInputStreams();
}

} // gnash.sound namespace 
} // namespace gnash
