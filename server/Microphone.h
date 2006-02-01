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

#ifndef __MICROPHONE_H__
#define __MICROPHONE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "log.h"

namespace gnash {
  
class Microphone {
public:
    Microphone();
    ~Microphone();
   void get();
   void setGain();
   void setRate();
   void setSilenceLevel();
   void setUseEchoSuppression();
private:
    bool _activityLevel;
    bool _gain;
    bool _index;
    bool _muted;
    bool _name;
    bool _names;
    bool _onActivity;
    bool _onStatus;
    bool _rate;
    bool _silenceLevel;
    bool _silenceTimeOut;
    bool _useEchoSuppression;
};

struct microphone_as_object : public as_object
{
    Microphone obj;
};

void microphone_new(const fn_call& fn);
void microphone_get(const fn_call& fn);
void microphone_setgain(const fn_call& fn);
void microphone_setrate(const fn_call& fn);
void microphone_setsilencelevel(const fn_call& fn);
void microphone_setuseechosuppression(const fn_call& fn);

} // end of gnash namespace

// __MICROPHONE_H__
#endif

