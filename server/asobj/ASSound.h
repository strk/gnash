// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

#ifndef __ASSOUND_H__
#define __ASSOUND_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "log.h"
#include "as_object.h" // for inheritance 

namespace gnash {

// Forward declarations
struct fn_call;
  
class Sound {
public:
    Sound();
    ~Sound();
   void attachSound();
   void getBytesLoaded();
   void getBytesTotal();
   void getPan();
   void getTransform();
   void getVolume();
   void loadSound();
   void setPan();
   void setTransform();
   void setVolume();
   void start();
   void stop();
private:
    bool _duration;
    bool _id3;
    bool _onID3;
    bool _onLoad;
    bool _onomplete;
    bool _position;
};

class sound_as_object : public as_object
{
public:
    //Sound obj;
    tu_string sound;
    int sound_id;
};

void sound_new(const fn_call& fn);
void sound_attachsound(const fn_call& fn);
void sound_getbytesloaded(const fn_call& fn);
void sound_getbytestotal(const fn_call& fn);
void sound_getpan(const fn_call& fn);
void sound_gettransform(const fn_call& fn);
void sound_getvolume(const fn_call& fn);
void sound_loadsound(const fn_call& fn);
void sound_setpan(const fn_call& fn);
void sound_settransform(const fn_call& fn);
void sound_setvolume(const fn_call& fn);
void sound_start(const fn_call& fn);
void sound_stop(const fn_call& fn);

} // end of gnash namespace

// __ASSOUND_H__
#endif

