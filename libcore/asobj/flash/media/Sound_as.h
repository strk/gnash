// Sound_as.h:  ActionScript 3 "Sound" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ3_SOUND_H
#define GNASH_ASOBJ3_SOUND_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "smart_ptr.h" //GNASH_USE_GC
#include "as_object.h"

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread/mutex.hpp>

namespace gnash {

// Forward declarations
class CharacterProxy;
	namespace sound {
		class sound_handler;
		class InputStream;
	}
	namespace media {
		class MediaHandler;
		class MediaParser;
		class AudioDecoder;
	}
	
	class fn_call;
	class as_object;
class ObjectURI;
	class Sound_as : public as_object
	{

	public:
		Sound_as();
		
		~Sound_as();
		
		static void init(as_object& global, const ObjectURI& uri);
		
	/// Make this sound control the given DisplayObject
    //
    /// NOTE: 0 is accepted, to implement an "invalid"
    ///       controller type.
    ///
    void attachCharacter(DisplayObject* attachedChar);

    void attachSound(int si, const std::string& name);

    /// Get number of bytes loaded from the external sound (if any)
    long getBytesLoaded();

    /// Get total number of bytes in the external sound being loaded
    //
    /// @return -1 if unknown
    ///
    long getBytesTotal();

    void getPan();
    void getTransform();

    /// Get volume from associated resource
    //
    /// @return true of volume was obtained, false
    ///         otherwise (for example if the associated
    ///         DisplayObject was unloaded).
    ///
    bool getVolume(int& volume);
    void setVolume(int volume);

    void loadSound(const std::string& file, bool streaming);
    void setPan();
    void setTransform();
    void start(double secsStart, int loops);
    void stop(int si);
    unsigned int getDuration();
    unsigned int getPosition();

    std::string soundName;  

private:

#ifdef GNASH_USE_GC
    /// Mark all reachable resources of a Sound, for the GC
    //
    /// Reachable resources are:
    /// - attached DisplayObject object (attachedCharacter)
    ///
    void markReachableResources() const;
#endif // GNASH_USE_GC

    bool _duration;
    bool _id3;
    bool _onID3;
    bool _onLoad;
    bool _onComplete;
    bool _position;

    boost::scoped_ptr<CharacterProxy> _attachedCharacter;
    int soundId;
    bool externalSound;
    std::string externalURL;
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

    virtual void advanceState();

    /// Probe audio
    void probeAudio();

    int _probeTimer;

    bool _soundCompleted;

    boost::mutex _soundCompletedMutex;

    /// Thread-safe setter for _soundCompleted
    void markSoundCompleted(bool completed);

    /// Is this sound attached to the soundhandler?
    bool isAttached() const {
        return _inputStream!=0;
    }
	};

} // gnash namespace

// GNASH_ASOBJ3_SOUND_H
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

