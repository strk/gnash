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

#include "SoundHandlerGst.h"

#include "log.h"
#include "vm/VM.h"
#include "builtin_function.h"
#include "timers.h"
#include "as_value.h"

#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>
#include <boost/mem_fn.hpp>

namespace gnash {
namespace media {

  
using boost::bind;


SoundHandlerGst::SoundHandlerGst()
  : _timer_id(0)
{
  gst_init(NULL, NULL);
}

SoundHandlerGst::~SoundHandlerGst()
{
  boost::mutex::scoped_lock lock(_sounds_mutex); // Just in case...

  std::for_each(_sounds.begin(), _sounds.end(), boost::checked_deleter<SoundGst>());
  
  _sounds.clear();
  
  if ( VM::isInitialized() ) VM::get().getRoot().clear_interval_timer(_timer_id);
}

void
SoundHandlerGst::start_timer()
{
  if (_timer_id) {
    // Timer is already running.
    return;
  }
  
  boost::intrusive_ptr<builtin_function> poller =
    new builtin_function(&SoundHandlerGst::poll_cb);
    
      
  boost::intrusive_ptr<as_object> obj(new as_object);
  
  std::auto_ptr<Timer> timer (new Timer());

  timer->setInterval(*poller, 50, obj);
  _timer_id = VM::get().getRoot().add_interval_timer(timer, true);
}


int
SoundHandlerGst::create_sound(void* data, unsigned int data_bytes,
                              std::auto_ptr<SoundInfo> sinfo)
{
  boost::mutex::scoped_lock lock(_sounds_mutex);

  if (!data) {
    _sounds.push_back(new SoundGst(sinfo));
  } else {
    assert(data_bytes);

    _sounds.push_back(new SoundGst(data, data_bytes, sinfo));
  }

  // This is our "unique identifier" or "handle". 
  return _sounds.size()-1;
}

long
SoundHandlerGst::fill_stream_data(unsigned char* data, unsigned int bytes,
                                  unsigned int sample_count, int handle)
{
  return ts_call(handle,
                 bind(&SoundGst::pushData, _1, data, bytes, sample_count), 0);
}

SoundInfo*
SoundHandlerGst::get_sound_info(int handle)
{
  return ts_call(handle, bind(&SoundGst::getSoundInfo, _1),
                 static_cast<SoundInfo*>(NULL));
}

void
SoundHandlerGst::play_sound(int handle, int loop_count, int offset,
           long start, const std::vector<sound_envelope>* envelopes)
{
  ts_call(handle,
          bind(&SoundGst::play, _1, loop_count, offset, start, envelopes));
  
  start_timer();
  
  _soundsStarted++;
}

int
SoundHandlerGst::get_volume(int handle)
{
  return ts_call(handle, bind(&SoundGst::getVolume, _1), 0);
}
	
void
SoundHandlerGst::set_volume(int handle, int volume)
{
  ts_call(handle, bind(&SoundGst::setVolume, _1, volume));  
}
		
void
SoundHandlerGst::stop_sound(int handle)
{
  ts_call(handle, bind(&SoundGst::stop, _1));
  
  _soundsStopped++;
}
		
void
SoundHandlerGst::delete_sound(int handle)
{
  boost::mutex::scoped_lock lock(_sounds_mutex);

  if (handle < 0 || handle > int(_sounds.size()) - 1) {
    return;
  }
  
  std::vector<SoundGst*>::iterator it =
    std::find(_sounds.begin(), _sounds.end(), _sounds[handle]);
  
  SoundGst* sound = *it;
  
  _sounds.erase(it);
  
  delete sound;
}

unsigned int
SoundHandlerGst::get_duration(int handle)
{

  return ts_call(handle, bind(&SoundGst::duration, _1), 0);
}

unsigned int
SoundHandlerGst::tell(int handle)
{
  return ts_call(handle, bind(&SoundGst::position, _1), 0);
}


void
SoundHandlerGst::reset()
{
  stop_all_sounds();
}

void
SoundHandlerGst::mute()
{
  ts_foreach(boost::mem_fn(&SoundGst::mute));
}

void
SoundHandlerGst::unmute()
{
  ts_foreach(boost::mem_fn(&SoundGst::unmute));
}


bool
SoundHandlerGst::is_muted()
{
  boost::mutex::scoped_lock lock(_sounds_mutex);

  std::vector<SoundGst*>::iterator it = std::find_if(_sounds.begin(),
    _sounds.end(), bind( std::logical_not<bool>(), 
                         bind(&SoundGst::muted, _1) ) );
    
  return (it == _sounds.end());
}

void
SoundHandlerGst::poll_sounds()
{
  ts_foreach(boost::mem_fn(&SoundGst::poll));
}

void
SoundHandlerGst::stop_all_sounds()
{
  ts_foreach(boost::mem_fn(&SoundGst::stop));
}

/* static */ as_value
SoundHandlerGst::poll_cb(const fn_call& /*fn*/)
{
  sound_handler* handler = get_sound_handler();
  SoundHandlerGst* handler_gst = dynamic_cast<SoundHandlerGst*>(handler);
  assert(handler_gst);
  
  handler_gst->poll_sounds();
  
  return as_value();
}



sound_handler*	create_sound_handler_gst()
// Factory.
{
  GNASH_REPORT_FUNCTION;
	return new SoundHandlerGst;
}


} // namespace media
} // namespace gnash




