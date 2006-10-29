// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"

namespace gnash {
  
class Camera {
public:
    Camera();
    ~Camera();
   void get();
   void setMode();
   void setMotionLevel();
   void setQuality();
private:
    bool _activityLevel;
    bool _bandwidth;
    bool _currentFps;
    bool _fps;
    bool _height;
    bool _index;
    bool _motionLevel;
    bool _motionTimeOut;
    bool _muted;
    bool _name;
    bool _names;
    bool _onActivity;
    bool _onStatus;
    bool _quality;
    bool _width;
};

class camera_as_object : public as_object
{
public:
    Camera obj;
};

void camera_new(const fn_call& fn);
void camera_get(const fn_call& fn);
void camera_setmode(const fn_call& fn);
void camera_setmotionlevel(const fn_call& fn);
void camera_setquality(const fn_call& fn);

} // end of gnash namespace

// __CAMERA_H__
#endif

