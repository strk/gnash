//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#ifndef __SOUND_HANDLER_GST_H
#define __SOUND_HANDLER_GST_H

#include "SoundGst.h"
#include "timers.h"
#include "as_value.h"

namespace gnash {
namespace media {

class SoundHandlerGst : public sound_handler
{
public:
  SoundHandlerGst();
  ~SoundHandlerGst();

  int create_sound(void* data, unsigned int data_bytes,
                   std::auto_ptr<SoundInfo> sinfo);

  long fill_stream_data(unsigned char* data, unsigned int data_bytes,
                        unsigned int sample_count, int handle_id);

  SoundInfo* get_sound_info(int sound_handle);

  void play_sound(int sound_handle, int loop_count, int secondOffset,
                  long start, const std::vector<sound_envelope>* envelopes);

  void stop_all_sounds();

  int get_volume(int sound_handle);
	
  void set_volume(int sound_handle, int volume);
		
  void stop_sound(int sound_handle);
		
  void delete_sound(int sound_handle);
  
  void poll_sounds();

  void reset();
		
  void mute();

  void unmute();

  bool is_muted();

  unsigned int get_duration(int sound_handle);

  unsigned int get_position(int sound_handle);
  
  void start_timer();
  
  static as_value poll_cb(const fn_call& fn);

private:
  std::vector<SoundGst*> _sounds;
  unsigned int _timer_id;  
}; // SoundHandlerGst

} // namespace media
} // namespace gnash


#endif // __SOUND_HANDLER_GST_H

