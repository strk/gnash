// SoundMad.cpp:  Play sounds using libmad (MP3 audio decoder), for Gnash.
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

#include "log.h"
#include "SoundMad.h"
#include "sound_definition.h" // for sound_sample
#include "movie_definition.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "utility.h" // for convert_raw_data

#include <string>

namespace gnash {

int 
SoundMad::readPacket(boost::uint8_t* buf, int buf_size)
{

	size_t ret = connection->read(static_cast<void*>(buf), buf_size);
	inputPos += ret;
	return ret;

}

int 
SoundMad::seekMedia(int offset, int whence){

	// Offset is absolute new position in the file
	if (whence == SEEK_SET) {
		connection->seek(offset);
		inputPos = offset;

	// New position is offset + old position
	} else if (whence == SEEK_CUR) {
		connection->seek(inputPos + offset);
		inputPos = inputPos + offset;

	// 	// New position is offset + end of file
	} else if (whence == SEEK_END) {
		// This is (most likely) a streamed file, so we can't seek to the end!
		// Instead we seek to 50.000 bytes... seems to work fine...
		connection->seek(50000);
		inputPos = 50000;
		
	}

	return inputPos;
}


void
SoundMad::setupDecoder(SoundMad* so)
{

	boost::intrusive_ptr<NetConnection> nc = so->connection;
	assert(nc);

	// Pass stuff from/to the NetConnection object.
	assert(so);
	if ( !nc->openConnection(so->externalURL) ) {
		log_error(_("Gnash could not open audio url: %s"), so->externalURL.c_str());
		delete so->lock;
		return;
	}

	so->inputPos = 0;

	// Init the mad decoder
	mad_stream_init(&so->stream);
	mad_frame_init(&so->frame);
	mad_synth_init(&so->synth);

	// Decode a single frame to decode the header

	// Fetch data from the file
	so->seekMedia(0, SEEK_SET);
	boost::uint8_t* buf = new boost::uint8_t[1024];
	int bufSize = so->readPacket(buf, 1024);

	// Setup the mad decoder
	mad_stream_buffer(&so->stream, buf, bufSize);

	int ret;

	int loops = 0;
	while(true) {

		ret = mad_frame_decode(&so->frame, &so->stream);
		loops++;
		
		// There is always some junk in front of the data, 
		// so we continue until we get past it.
		if (ret && so->stream.error == MAD_ERROR_LOSTSYNC) continue;
		
		// Error handling is done by relooping (max. 8 times) and just hooping that it will work...
		if (loops > 8) break;
		if (ret == -1 && so->stream.error != MAD_ERROR_BUFLEN && MAD_RECOVERABLE(so->stream.error)) {
			log_error(_("Recoverable error while decoding MP3, MAD error: %s"), mad_stream_errorstr (&so->stream));
			continue;
		}
		break;
	}
	so->bitrate = so->frame.header.bitrate;

	so->seekMedia(0, SEEK_SET);
	delete [] buf;

	// By deleting this lock we allow start() to start playback
	delete so->lock;
	return;
}

// audio callback is running in sound handler thread
bool SoundMad::getAudio(void* owner, boost::uint8_t* stream, int len)
{
	SoundMad* so = static_cast<SoundMad*>(owner);

	unsigned int pos = 0;

	// First use the data left over from last time
	if (so->leftOverSize > 0) {

		// If we have enough "leftover" data to fill the buffer,
		// we don't bother to decode some new.
		if (so->leftOverSize >= len) {
			memcpy(stream, so->leftOverData, len);
			int rest = so->leftOverSize - len;
			if (rest < 1) {
				delete[] so->leftOverData;
				so->leftOverSize = 0;
			} else {
				boost::uint8_t* buf = new boost::uint8_t[rest];
				memcpy(stream, so->leftOverData+len, rest);
				delete[] so->leftOverData;
				so->leftOverData = buf;
				so->leftOverSize -= len;
			}	
			return true;
		} else {
			memcpy(stream, so->leftOverData, so->leftOverSize);
			pos += so->leftOverSize;
			so->leftOverSize = 0;
			delete[] so->leftOverData;
		}
	}

	boost::uint8_t* buf = new boost::uint8_t[8192];
	int bufSize = so->readPacket(buf, 8192);
	int orgBufSize = bufSize;

	bool loop = true;
	boost::uint8_t* ptr = new boost::uint8_t[8192];

	bool ret = true;
	media::sound_handler* s = get_sound_handler();
	if (bufSize > 0) {
		if (s) {
			// temp raw buffer
			boost::uint8_t* tmp_raw_buffer;
			unsigned int tmp_raw_buffer_size;
			int outsize = 0;
			while (loop) {
			// Decode audio

				// Setup the mad decoder
				mad_stream_buffer(&so->stream, buf+(orgBufSize-bufSize), bufSize);

				int ret;
				const unsigned char* old_next_frame = so->stream.next_frame;
				int loops = 0;
				while(true) {

					ret = mad_frame_decode(&so->frame, &so->stream);
					loops++;
					
					// There is always some junk in front of the data, 
					// so we continue until we get past it.
					if (ret && so->stream.error == MAD_ERROR_LOSTSYNC) continue;
					
					// Error handling is done by relooping (max. 8 times) and just hooping that it will work...
					if (loops > 8) break;
					if (ret == -1 && so->stream.error != MAD_ERROR_BUFLEN && MAD_RECOVERABLE(so->stream.error)) {
						log_error(_("Recoverable error while decoding MP3, MAD error: %s"), mad_stream_errorstr (&so->stream));
						continue;
					}
					
					break;
				}

				if (ret == -1 && so->stream.error != MAD_ERROR_BUFLEN) {
					log_error(_("Unrecoverable error while decoding MP3, MAD error: %s"), mad_stream_errorstr (&so->stream));
					bufSize = 0;
					loop = false;
				} else if (ret == -1 && so->stream.error == MAD_ERROR_BUFLEN) {
					// the buffer is empty, no more to decode!
					bufSize = 0;
					loop = false;
				} else {
					bufSize -= so->stream.next_frame - old_next_frame;
				}

				mad_synth_frame (&so->synth, &so->frame);
				
				outsize = so->synth.pcm.length * ((so->frame.header.mode) ? 4 : 2);

				tmp_raw_buffer = new boost::uint8_t[outsize];
				int sample;
				
				boost::int16_t* dst = reinterpret_cast<boost::int16_t*>(tmp_raw_buffer);

				// transfer the decoded samples into the sound-struct, and do some
				// scaling while we're at it.
				for(int f = 0; f < so->synth.pcm.length; f++)
				{
					for (int e = 0; e < ((so->frame.header.mode) ? 2 : 1); e++){ // channels (stereo/mono)

						mad_fixed_t mad_sample = so->synth.pcm.samples[e][f];

						// round
						mad_sample += (1L << (MAD_F_FRACBITS - 16));

						// clip
						if (mad_sample >= MAD_F_ONE) mad_sample = MAD_F_ONE - 1;
						else if (mad_sample < -MAD_F_ONE) mad_sample = -MAD_F_ONE;

						// quantize
						sample = mad_sample >> (MAD_F_FRACBITS + 1 - 16);

						if ( sample != static_cast<boost::int16_t>(sample) ) sample = sample < 0 ? -32768 : 32767;

						*dst++ = sample;
					}
				}

				// If we need to convert samplerate or/and from mono to stereo...
				if (outsize > 0 && (so->frame.header.samplerate != 44100 || !so->frame.header.mode)) {

					boost::int16_t* adjusted_data = 0;
					int	adjusted_size = 0;
					int sample_count = outsize / ((so->frame.header.mode) ? 4 : 2);

					// Convert to needed samplerate
					convert_raw_data(&adjusted_data, &adjusted_size, tmp_raw_buffer, sample_count, 2, 
							so->frame.header.samplerate, so->frame.header.mode,
							44100, true/*stereo*/);

					// Hopefully this won't happen
					if (!adjusted_data) {
						log_error(_("Error in sound sample convertion"));
						continue;
					}

					// Move the new data to the sound-struct
					delete[] tmp_raw_buffer;
					tmp_raw_buffer = reinterpret_cast<boost::uint8_t*>(adjusted_data);
					tmp_raw_buffer_size = adjusted_size;

				} else {
					tmp_raw_buffer_size = outsize;
				}




				// Copy the data to buffer
				// If the decoded data isn't enough to fill the buffer, we put the decoded
				// data into the buffer, and continues decoding.
				if (tmp_raw_buffer_size <= len-pos) {
					memcpy(stream+pos, tmp_raw_buffer, tmp_raw_buffer_size);
					pos += tmp_raw_buffer_size;
				} else {
				// If we can fill the buffer, and still have "leftovers", we save them
				// and use them later.
					int rest = len-pos;
					so->leftOverSize = tmp_raw_buffer_size - rest;
					memcpy(stream+pos, tmp_raw_buffer, rest);
					so->leftOverData = new boost::uint8_t[so->leftOverSize];
					memcpy(so->leftOverData, (tmp_raw_buffer)+rest, so->leftOverSize);
					loop = false;
					pos += rest;
				}
				delete[] tmp_raw_buffer;
			} // while		 
		} // (s) 
	} else { // bufSize > 0
		// If we should loop we make sure we do.
		if (so->remainingLoops != 0) {
			so->remainingLoops--;

			// Seek to begining of file
			so->seekMedia(0, SEEK_SET);
		} else {
			 // Stops playback by returning false which makes the soundhandler
			 // detach this sound.
			 ret = false;
			 so->isAttached = false;
		}
	}
	so->seekMedia(-bufSize, SEEK_CUR);
	delete[] ptr;
	return ret;
}

SoundMad::~SoundMad() {
	if (leftOverData && leftOverSize) delete[] leftOverData;

	if (isAttached) {
		mad_synth_finish(&synth);
		mad_frame_finish(&frame);
		mad_stream_finish(&stream);

		media::sound_handler* s = get_sound_handler();
		if (s) {
			s->detach_aux_streamer(this);
		}
	}

}

void
SoundMad::loadSound(std::string file, bool streaming)
{
	leftOverData = NULL;
	leftOverSize = 0;
	remainingLoops = 0;

	if (connection) {
		log_error(_("This sound already has a connection?  (We try to handle this by overriding the old one...)"));
	}
	externalURL = file;

	connection = new NetConnection();

	externalSound = true;
	isStreaming = streaming;

	lock = new boost::mutex::scoped_lock(setupMutex);

	// To avoid blocking while connecting, we use a thread.
	setupThread = new boost::thread(boost::bind(SoundMad::setupDecoder, this));

}

void
SoundMad::start(int offset, int loops)
{
	boost::mutex::scoped_lock lock(setupMutex);

	if (externalSound) {
		seekMedia((bitrate*offset)/8, SEEK_SET);

		// Save how many loops to do
		if (loops > 0) {
			remainingLoops = loops;
		}
	}

	// Start sound
	media::sound_handler* s = get_sound_handler();
	if (s) {
		if (externalSound) {
			if (1)
			{
				s->attach_aux_streamer(getAudio, (void*) this);
				isAttached = true;
			} 
		} else {
	    	s->play_sound(soundId, loops, offset, 0, NULL);
	    }
	}
}

void
SoundMad::stop(int si)
{
	// stop the sound
	media::sound_handler* s = get_sound_handler();
	if (s != NULL)
	{
	    if (si < 0) {
	    	if (externalSound) {
	    		s->detach_aux_streamer(this);
	    	} else {
				s->stop_sound(soundId);
			}
		} else {
			s->stop_sound(si);
		}
	}
}

unsigned int
SoundMad::getDuration()
{
	// If this is a event sound get the info from the soundhandler
	if (!externalSound) {
		media::sound_handler* s = get_sound_handler();
		if (s) {		
	    	return (s->get_duration(soundId));
	    } else {
	    	return 0; // just in case
		}
	}

	// Return the duration of the file in milliseconds
/*	if (formatCtx && audioIndex) {
		return static_cast<unsigned int>(formatCtx->duration * 1000);
	} else {
		return 0;
	}*/
	return 0;
}

unsigned int
SoundMad::getPosition()
{

	// If this is a event sound get the info from the soundhandler
	if (!externalSound) {
		media::sound_handler* s = get_sound_handler();
		if (s) {
			return s->tell(soundId);	
	    } else {
	    	return 0; // just in case
		}
	}

	// Return the position in the file in milliseconds
	return inputPos/bitrate/8*1000;
}

} // end of gnash namespace
