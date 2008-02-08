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

#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>
#include <boost/mem_fn.hpp>
#include "log.h"
#include "vm/VM.h"
#include "builtin_function.h"
#include "gnash.h"

namespace gnash {
namespace media {

#define RETURN_IF_BAD_HANDLE(handle)                                          \
  if (handle < 0 || handle > int(_sounds.size()) - 1) {                       \
    return;                                                                   \
  }

#define RV_IF_BAD_HANDLE(handle, value)                                       \
  if (handle < 0 || handle > int(_sounds.size()) - 1) {                       \
    return value;                                                             \
  }


SoundHandlerGst::SoundHandlerGst()
  : _timer_id(0)
{
  gst_init(NULL, NULL);
  GNASH_REPORT_FUNCTION;
}


SoundHandlerGst::~SoundHandlerGst()
{
  std::for_each(_sounds.begin(), _sounds.end(), boost::checked_deleter<SoundGst>());
  
  _sounds.clear();
  
  if ( VM::initialized() ) VM::get().getRoot().clear_interval_timer(_timer_id);
}

void
SoundHandlerGst::poll_sounds()
{
  std::for_each(_sounds.begin(), _sounds.end(),
    boost::mem_fn(&SoundGst::poll));
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
  RV_IF_BAD_HANDLE(handle, 0);
  
  return _sounds[handle]->pushData(data, bytes, sample_count);
}

SoundInfo*
SoundHandlerGst::get_sound_info(int handle)
{
  RV_IF_BAD_HANDLE(handle, NULL);
  
  return _sounds[handle]->getSoundInfo();
}

void
SoundHandlerGst::play_sound(int handle, int loop_count, int offset,
           long start, const std::vector<sound_envelope>* envelopes)
{
  RETURN_IF_BAD_HANDLE(handle);
    
  start_timer();

  _sounds[handle]->play(loop_count, offset, start, envelopes);
  
  _soundsStarted++;
}

void
SoundHandlerGst::stop_all_sounds()
{
  std::for_each(_sounds.begin(), _sounds.end(), boost::mem_fn(&SoundGst::stop));
}


int
SoundHandlerGst::get_volume(int handle)
{
  RV_IF_BAD_HANDLE(handle, 0);

  return _sounds[handle]->getVolume();
}
	
void
SoundHandlerGst::set_volume(int handle, int volume)
{
  RETURN_IF_BAD_HANDLE(handle);
  
  _sounds[handle]->setVolume(volume);
}
		
void
SoundHandlerGst::stop_sound(int handle)
{
  RETURN_IF_BAD_HANDLE(handle);
  
  _sounds[handle]->stop();
  
  _soundsStopped++;
}
		
void
SoundHandlerGst::delete_sound(int handle)
{
  RETURN_IF_BAD_HANDLE(handle);
  
  std::vector<SoundGst*>::iterator it =
    std::find(_sounds.begin(), _sounds.end(), _sounds[handle]);
  
  SoundGst* sound = *it;
  
  _sounds.erase(it);
  
  delete sound;
}

unsigned int
SoundHandlerGst::get_duration(int handle)
{
  RV_IF_BAD_HANDLE(handle, 0);
  
  return _sounds[handle]->duration();

}

unsigned int
SoundHandlerGst::get_position(int handle)
{
  RV_IF_BAD_HANDLE(handle, 0);
  
  return _sounds[handle]->position();
}


void
SoundHandlerGst::reset()
{
  stop_all_sounds();
}

void
SoundHandlerGst::mute()
{
  std::for_each(_sounds.begin(), _sounds.end(),
                boost::mem_fn(&SoundGst::mute));
}

void
SoundHandlerGst::unmute()
{
  std::for_each(_sounds.begin(), _sounds.end(),
                boost::mem_fn(&SoundGst::unmute));
}


bool
SoundHandlerGst::is_muted()
{
  using namespace boost;

  std::vector<SoundGst*>::iterator it = std::find_if(_sounds.begin(),
    _sounds.end(), bind( std::logical_not<bool>(), 
                         bind(&SoundGst::muted, _1) ) );
    
  return (it == _sounds.end());
}

#undef RETURN_IF_BAD_HANDLE
#undef RV_IF_BAD_HANDLE

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




