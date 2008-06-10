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

#ifndef SOUND_HANDLER_GST_H
#define SOUND_HANDLER_GST_H

#include "SoundGst.h"
#include <boost/thread/mutex.hpp>
#include <algorithm>

// Forward declarations
namespace gnash {
	class as_value;
	class fn_call;
}

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

  unsigned int tell(int sound_handle);
  
  void start_timer();  
  
  static as_value poll_cb(const fn_call& fn);

private:
  
  /// Thread safe version of for_each which is applied to the _sounds vector.
  ///
  /// @param functor Function object of which operator() will be called; akin
  ///                the third argument of std::for_each.
  template <typename T>
  void
  ts_foreach(T functor)
  {
    boost::mutex::scoped_lock lock(_sounds_mutex);

    std::for_each(_sounds.begin(), _sounds.end(), functor);
  }
  
  /// Calls operator() on a function object which takes a SoundGst pointer
  /// as its sole argument. The pointer is taken from the _sounds object,
  /// as indicated by the first argument to this template method. The access
  /// to _sounds is thread safe.
  /// 
  /// @param handle the _sounds index to _sounds with which the SoundGst
  ///               pointer is to be retrieved.
  /// @param functor the function object to call operator(SoundGst*) on, so that
  ///        the expression functor(SoundGst*) is valid.  
  template<typename T>
  void ts_call(int handle, T functor)
  {
    boost::mutex::scoped_lock lock(_sounds_mutex);

    if (handle < 0 || handle > int(_sounds.size()) - 1) {
      return;
    }
    
    functor(_sounds[handle]);
  }
  
  /// This member is like the previous template member, but returns a value
  /// indicated by its third argument, *if the handle indicated in the first
  /// argument is invalid*. Otherwise, it returns functor(SoundGst*). Of
  /// course, the return value of functor() must be equal to the type of 
  /// bad_handle_rv (the third argument).
  ///
  /// @param bad_handle_rv the value to return if the passed handle is invalid.
  /// @return functor(SoundGst*).  
  template<typename T, typename R>
  R ts_call(int handle, T functor, R bad_handle_rv)
  {
    boost::mutex::scoped_lock lock(_sounds_mutex);

    if (handle < 0 || handle > int(_sounds.size()) - 1) {
      return bad_handle_rv;
    }
    
    return functor(_sounds[handle]);
  }





private:

  /// Mutex for access to the _sounds object.
  boost::mutex _sounds_mutex;
  
  std::vector<SoundGst*> _sounds;
  
  unsigned int _timer_id;

}; // SoundHandlerGst

} // namespace media
} // namespace gnash


#endif // __SOUND_HANDLER_GST_H

