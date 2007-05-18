//   Copyright (C) 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// $Id: sound_handler_test.cpp,v 1.4 2007/05/18 10:28:17 martinwguy Exp $

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sound_handler_test.h"

#include "log.h"
#include <cmath>
#include <vector>

using namespace boost;

TEST_sound_handler::TEST_sound_handler()
	: 	soundsPlaying(0),
		muted(false),
		started_all(0),
		stopped_all(0)
{

}

TEST_sound_handler::~TEST_sound_handler()
{
	for (size_t i=0, e=m_sound_data.size(); i < e; ++i)
	{
		stop_sound(i);
		delete_sound(i);
	}
}


int	TEST_sound_handler::create_sound(
	void* data,
	int data_bytes,
	int sample_count,
	format_type format,
	int sample_rate,
	bool stereo)
// Called to create a sample.  We'll return a sample ID that
// can be use for playing it.
{

	sound_data *sounddata = new sound_data;
	if (!sounddata) {
		gnash::log_error("could not allocate memory for sounddata !\n");
		return -1;
	}

	sounddata->format = format;
	sounddata->data_size = data_bytes;
	sounddata->stereo = stereo;
	sounddata->sample_count = sample_count;
	sounddata->sample_rate = sample_rate;
	sounddata->volume = 100;
	sounddata->started = 0;
	sounddata->stopped = 0;

	mutex::scoped_lock lock(_mutex);

	sounddata->data = new uint8_t[data_bytes];
	if (!sounddata->data) {
		gnash::log_error("could not allocate space for data in soundhandler\n");
		return -1;
	}
	memcpy(sounddata->data, data, data_bytes);

	m_sound_data.push_back(sounddata);
	int sound_id = m_sound_data.size()-1;

	return sound_id;

}

// this gets called when a stream gets more data
long	TEST_sound_handler::fill_stream_data(void* data, int data_bytes, int /*sample_count*/, int handle_id)
{

	mutex::scoped_lock lock(_mutex);
	// @@ does a negative handle_id have any meaning ?
	//    should we change it to unsigned instead ?
	if (handle_id < 0 || (unsigned int) handle_id+1 > m_sound_data.size()) {
		return 1;
	}
	int start_size = 0;
	sound_data* sounddata = m_sound_data[handle_id];

	// Reallocate the required memory.
	uint8_t* tmp_data = new uint8_t[data_bytes + sounddata->data_size];
	memcpy(tmp_data, sounddata->data, sounddata->data_size);
	memcpy(tmp_data + sounddata->data_size, data, data_bytes);
	if (sounddata->data_size > 0) delete [] sounddata->data;
	sounddata->data = tmp_data;

	start_size = sounddata->data_size;
	sounddata->data_size += data_bytes;
	std::vector<active_sound*> asounds = sounddata->m_active_sounds;
	
	// If playback has already started, we also update the active sounds
	for(uint32_t i=0; i < asounds.size(); i++) {
		active_sound* sound = asounds[i];
		sound->set_data(sounddata->data);
		sound->data_size = sounddata->data_size;
	}

	return start_size;
}


void	TEST_sound_handler::play_sound(int sound_handle, int loop_count, int offset, long start_position, std::vector<sound_envelope>* envelopes)
// Play the index'd sample.
{
	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists, or if audio is muted
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size() || muted)
	{
		// Invalid handle or muted
		return;
	}

	sound_data* sounddata = m_sound_data[sound_handle];

	// If this is called from a streamsoundblocktag, we only start if this
	// sound isn't already playing. If a active_sound-struct is existing we
	// assume it is also playing.
	if (start_position > 0 && sounddata->m_active_sounds.size() > 0) {
		return;
	}

	// Make a "active_sound" for this sound which is later placed on the vector of instances of this sound being played
	active_sound* sound = new active_sound;

	// Copy data-info to the active_sound
	sound->data_size = sounddata->data_size;
	sound->set_data(sounddata->data);

	// Set the given options of the sound
	if (start_position < 0) sound->position = 0;
	else sound->position = start_position;

	if (offset < 0) sound->offset = 0;
	else sound->offset = (sounddata->stereo ? offset : offset*2); // offset is stored as stereo

	sound->envelopes = envelopes;
	sound->current_env = 0;
	sound->samples_played = 0;

	// Set number of loop we should do. -1 is infinte loop, 0 plays it once, 1 twice etc.
	sound->loop_count = loop_count;

	++soundsPlaying;
	sounddata->started++;
	started_all++;
	sounddata->m_active_sounds.push_back(sound);

}


void	TEST_sound_handler::stop_sound(int sound_handle)
{
	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || (unsigned int) sound_handle >= m_sound_data.size())
	{
		// Invalid handle.
	} else {
	
		sound_data* sounddata = m_sound_data[sound_handle];
	
		for (int32_t i = (int32_t) sounddata->m_active_sounds.size()-1; i >-1; i--) {

			// Stop sound, remove it from the active list (mp3)
			if (sounddata->format == 2) {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
				soundsPlaying--;

			// Stop sound, remove it from the active list (adpcm/native16)
			} else {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
				soundsPlaying--;
			}
		}
		sounddata->stopped++;
		stopped_all++;

	}

}


void	TEST_sound_handler::delete_sound(int sound_handle)
// this gets called when it's done with a sample.
{
	mutex::scoped_lock lock(_mutex);

	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		delete[] m_sound_data[sound_handle]->data;
	}

}

// This will stop all sounds playing. Will cause problems if the soundhandler is made static
// and supplys sound_handling for many SWF's, since it will stop all sounds with no regard
// for what sounds is associated with what SWF.
void	TEST_sound_handler::stop_all_sounds()
{
	mutex::scoped_lock lock(_mutex);

	int32_t num_sounds = (int32_t) m_sound_data.size()-1;
	for (int32_t j = num_sounds; j > -1; j--) {//Optimized
		sound_data* sounddata = m_sound_data[j];
		int32_t num_active_sounds = (int32_t) sounddata->m_active_sounds.size()-1;
		for (int32_t i = num_active_sounds; i > -1; i--) {

			// Stop sound, remove it from the active list (mp3)
			if (sounddata->format == 2) {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
				soundsPlaying--;

			// Stop sound, remove it from the active list (adpcm/native16)
			} else {
				sounddata->m_active_sounds.erase(sounddata->m_active_sounds.begin() + i);
				soundsPlaying--;
			}
		}
	}
}


//	returns the sound volume level as an integer from 0 to 100,
//	where 0 is off and 100 is full volume. The default setting is 100.
int	TEST_sound_handler::get_volume(int sound_handle) {

	mutex::scoped_lock lock(_mutex);

	int ret;
	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		ret = m_sound_data[sound_handle]->volume;
	} else {
		ret = 0; // Invalid handle
	}
	return ret;
}


//	A number from 0 to 100 representing a volume level.
//	100 is full volume and 0 is no volume. The default setting is 100.
void	TEST_sound_handler::set_volume(int sound_handle, int volume) {

	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size())
	{
		// Invalid handle.
	} else {

		// Set volume for this sound. Should this only apply to the active sounds?
		m_sound_data[sound_handle]->volume = volume;
	}


}
	
void TEST_sound_handler::get_info(int sound_handle, int* format, bool* stereo) {

	mutex::scoped_lock lock(_mutex);

	// Check if the sound exists.
	if (sound_handle >= 0 && static_cast<unsigned int>(sound_handle) < m_sound_data.size())
	{
		*format = m_sound_data[sound_handle]->format;
		*stereo = m_sound_data[sound_handle]->stereo;
	}

}

// gnash calls this to mute audio
void TEST_sound_handler::mute() {
	stop_all_sounds();
	muted = true;
}

// gnash calls this to unmute audio
void TEST_sound_handler::unmute() {
	muted = false;
}

bool TEST_sound_handler::is_muted()
{
	return muted;
}

void	TEST_sound_handler::attach_aux_streamer(aux_streamer_ptr ptr, void* owner)
{
	assert(owner);
	assert(ptr);

	aux_streamer_ptr p;
	if (m_aux_streamer.get(owner, &p))
	{
		// Already in the hash.
		return;
	}
	m_aux_streamer[owner] = ptr;

	soundsPlaying++;
}

void	TEST_sound_handler::detach_aux_streamer(void* owner)
{
	soundsPlaying--;
	m_aux_streamer.erase(owner);
}

int TEST_sound_handler::test_times_stopped_all() {
	return stopped_all;
}

int TEST_sound_handler::test_times_started_all() {
	return started_all;
}

int TEST_sound_handler::test_times_stopped(int sound_handle) {
	// Check if the sound exists.
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size())
	{
		// Invalid handle.
		return -1;
	} else {

		// Set volume for this sound. Should this only apply to the active sounds?
		return m_sound_data[sound_handle]->stopped;
	}

}

int TEST_sound_handler::test_times_started(int sound_handle) {
	// Check if the sound exists.
	if (sound_handle < 0 || static_cast<unsigned int>(sound_handle) >= m_sound_data.size())
	{
		// Invalid handle.
		return -1;
	} else {

		// Set volume for this sound. Should this only apply to the active sounds?
		return m_sound_data[sound_handle]->started;
	}

}

gnash::sound_handler*	gnash::create_sound_handler_test()
// Factory.
{
	return new TEST_sound_handler;
}

void active_sound::set_data(uint8_t* idata) {
	data = idata;
}

// Local Variables:
// mode: C++
// End:

