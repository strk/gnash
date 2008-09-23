// SoundFfmpeg.cpp:  Gnash sound output via FFMPEG library.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "SoundFfmpeg.h"
#include "log.h"
#include "sound_definition.h" // for sound_sample
#include "movie_definition.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "URL.h"
#include "StreamProvider.h"

#include <string>

namespace gnash {

SoundFfmpeg::SoundFfmpeg()
	: // REMEMBER TO ALWAYS INITIALIZE ALL MEMBERS !
	_mediaHandler(media::MediaHandler::get()),
	_startTime(0),
	_leftOverData(),
	_leftOverPtr(0),
	_leftOverSize(0),
	isAttached(false),
	remainingLoops(0)
{}


bool
SoundFfmpeg::getAudio(boost::uint8_t* stream, int len)
{
	//GNASH_REPORT_FUNCTION;

	while (len > 0)
	{
		if ( ! _leftOverData )
		{
			bool parsingComplete = _mediaParser->parsingCompleted(); // check *before* calling nextAudioFrame
			std::auto_ptr<media::EncodedAudioFrame> frame = _mediaParser->nextAudioFrame();
			if ( ! frame.get() )
			{
				// just wait some more if parsing isn't complete yet
				if ( ! parsingComplete )
				{
					//log_debug("Parsing not complete and no more audio frames in input, try again later");
					break;
				}

				// or detach and stop here...
				// (should really honour loopings if any)
				//if ( remainingLoops.. )
				//log_debug("Parsing complete and no more audio frames in input, detaching");
				return false; // will detach us (we should change isAttached, but need thread safety!)
			}

			// if we've been asked to start at a specific time, skip
			// any frame with earlier timestamp
			if ( frame->timestamp < _startTime ) continue;

			_leftOverData.reset( _audioDecoder->decode(*frame, _leftOverSize) );
			_leftOverPtr = _leftOverData.get();
			if ( ! _leftOverData )
			{
				log_error("No samples decoded from input of %d bytes", frame->dataSize);
				continue;
			}
		}

		int n = std::min<int>(_leftOverSize, len);
		memcpy(stream, _leftOverPtr, n);
		stream += n;
		_leftOverPtr += n;
		_leftOverSize -= n;
		len -= n;

		if (_leftOverSize == 0)
		{
			_leftOverData.reset();
			_leftOverPtr = 0;
		}

	}

	// drop any queued video frame
	while (_mediaParser->nextVideoFrame().get());

	return true;
}

// audio callback is running in sound handler thread
bool
SoundFfmpeg::getAudioWrapper(void* owner, boost::uint8_t* stream, int len)
{
	SoundFfmpeg* so = static_cast<SoundFfmpeg*>(owner);
	return so->getAudio(stream, len);
}

SoundFfmpeg::~SoundFfmpeg()
{
	//GNASH_REPORT_FUNCTION;

	if (isAttached && _soundHandler)
	{
		_soundHandler->detach_aux_streamer(this);
	}
}

void
SoundFfmpeg::loadSound(const std::string& file, bool streaming)
{
	if ( ! _mediaHandler || ! _soundHandler ) 
	{
		log_debug("No media or sound handlers, won't load any sound");
		return;
	}

	/// If we are already streaming stop doing so as we'll replace
	/// the media parser
	if ( isAttached )
	{
		_soundHandler->detach_aux_streamer(this);
		isAttached = false;
	}

	/// Delete any media parser being used (make sure we have detached!)
	_mediaParser.reset();

	/// Start at offset 0, in case a previous ::start() call
	/// changed that.
	_startTime=0;

	URL url(file, get_base_url());
	externalURL = url.str(); // what for ? bah!

	StreamProvider& streamProvider = StreamProvider::getDefaultInstance();
	std::auto_ptr<IOChannel> inputStream( streamProvider.getStream( externalURL ) );
	if ( ! inputStream.get() )
	{
		log_error( _("Gnash could not open this url: %s"), url );
		return;
	}

	externalSound = true;
	isStreaming = streaming;

	_mediaParser.reset( _mediaHandler->createMediaParser(inputStream).release() );
	if ( ! _mediaParser )
	{
		log_error(_("Unable to create parser for Sound input"));
		// not necessarely correct, the stream might have been found...
		return;
	}
	_mediaParser->setBufferTime(60000); // one minute buffer... should be fine

	media::AudioInfo* audioInfo =  _mediaParser->getAudioInfo();
	if (!audioInfo) {
		log_debug("No audio in Sound input");
		return;
	}

    try {
	    _audioDecoder.reset(_mediaHandler->createAudioDecoder(*audioInfo).release());
	}
	catch (MediaException& e) {
        assert(!_audioDecoder.get());
		log_error(_("Could not create audio decoder: %s"), e.what());
	}

	// start playing ASAP, a call to ::start will just change _startTime
	//log_debug("Attaching the aux streamer");
	_soundHandler->attach_aux_streamer(getAudioWrapper, (void*) this);
	isAttached = true;

}

void
SoundFfmpeg::start(int offset, int loops)
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, nothing to start...");
		return;
	}

	if (externalSound)
	{
		if ( ! _mediaParser )
		{
			log_error("No MediaParser initialized, can't start an external sound");
			return;
		}
		if ( ! _audioDecoder )
		{
			log_error("No AudioDecoder initialized, can't start an external sound");
			return;
		}

		if (offset > 0)
		{
			_startTime=offset*1000;
			boost::uint32_t seekms = boost::uint32_t(offset*1000);
			// TODO: boost::mutex::scoped_lock parserLock(_parserMutex);
			_mediaParser->seek(seekms); // well, we try...
		}

		// Save how many loops to do (not when streaming)
		if (! isStreaming && loops > 0)
		{
			remainingLoops = loops;
		}

		if ( ! isAttached ) 
		{
			_soundHandler->attach_aux_streamer(getAudioWrapper, (void*) this);
			isAttached = true;
		}
	}
	else
	{
	    	_soundHandler->play_sound(soundId, loops, offset, 0, NULL);
	}
}

void
SoundFfmpeg::stop(int si)
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, nothing to stop...");
		return;
	}

	// stop the sound
	if (si < 0)
	{
	    	if (externalSound)
		{
	    		_soundHandler->detach_aux_streamer(this);
	    	}
		else
		{
			_soundHandler->stop_sound(soundId);
		}
	}
	else
	{
		_soundHandler->stop_sound(si);
	}
}

unsigned int
SoundFfmpeg::getDuration()
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, can't check duration...");
		return 0;
	}

	// If this is a event sound get the info from the soundhandler
	if (!externalSound)
	{
		return _soundHandler->get_duration(soundId);
	}

	// If we have a media parser (we'd do for an externalSound)
	// try fetching duration from it
	if ( _mediaParser )
	{
		media::AudioInfo* info = _mediaParser->getAudioInfo();
		if ( info )
		{
			return info->duration;
		}
	}

	return 0;
}

unsigned int
SoundFfmpeg::getPosition()
{
	if ( ! _soundHandler )
	{
		log_error("No sound handler, can't check position (we're likely not playing anyway)...");
		return 0;
	}

	// If this is a event sound get the info from the soundhandler
	if (!externalSound)
	{
		return _soundHandler->tell(soundId);
	}

	if ( _mediaParser )
	{
		boost::uint64_t ts;
		if ( _mediaParser->nextAudioFrameTimestamp(ts) )
		{
			return ts;
		}
	}

	return 0;

}

long
SoundFfmpeg::getBytesLoaded()
{
	if ( _mediaParser ) return _mediaParser->getBytesLoaded();
	return 0;
}

long
SoundFfmpeg::getBytesTotal()
{
	if ( _mediaParser ) return _mediaParser->getBytesTotal();
	return -1;
}




} // end of gnash namespace
