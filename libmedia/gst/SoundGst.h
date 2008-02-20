// 
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

#ifndef __GNASH_SOUND_GST_H
#define __GNASH_SOUND_GST_H

#include <gst/gst.h>
#include "sound_handler.h"
#include <boost/utility.hpp>
#include <boost/scoped_array.hpp>


namespace gnash {
namespace media {


/// This class is intended to control a single sound (corresponding to a single
/// SWF DefineSound or SoundStream set). Objects of this type will be managed
/// by the sound_handler.

class SoundGst : public boost::noncopyable
{
public:
  SoundGst(std::auto_ptr<SoundInfo>& sinfo);
  SoundGst(void* data, unsigned int data_bytes,
           std::auto_ptr<SoundInfo>& sinfo);
  ~SoundGst();
 

  long pushData(unsigned char* data, unsigned int data_bytes,
                unsigned int sample_count);
  void play(int loop_count, int secondOffset, long start,
            const std::vector<sound_handler::sound_envelope>* envelopes);
  void stop();
  
  unsigned int getVolume();
  void setVolume(unsigned int new_volume);
  
  void mute();
  void unmute();
  bool muted();
  
  
  unsigned int duration();
  unsigned int position();
  
  SoundInfo* getSoundInfo();

  static void message_received (GstBus * bus, GstMessage * message,
                                SoundGst* sound);
                                
  void poll();
private: // methods

  GstElement* gstFindDecoder(const GstCaps* caps, const gchar *name);
  
  bool gstBuildPipeline();
  
  GstCaps* getCaps();
  
  void handleMessage (GstMessage *message);

  bool needDecoder();
  
  void setLoopCount(int count);

private: // data fields

  /// A vector of data pointers. There may be only one pointer in this vector
  /// (in the case of a DefineSound), or several (in the case of a SoundStream)
  /// This class has ownership of the new[]-allocated pointers.
  std::vector<uint8_t*> _data_vec;
  
  std::auto_ptr<SoundInfo> _info;
  
  GstElement* _pipeline;
  GstElement* _volume;
  GstElement* _buffersrc;
  
  long _dataSize;
  
  int _loop_count;
};


} // namespace media
} // namespace gnash

#endif //__GNASH_SOUND_GST_H
