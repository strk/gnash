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

#ifndef __NETSTREAM_H__
#define __NETSTREAM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "impl.h"
#include "log.h"

namespace gnash {
  
class NetStream {
public:
    NetStream();
    ~NetStream();
   void close();
   void pause();
   void play();
   void seek();
   void setBufferTime();
private:
    bool _bufferLength;
    bool _bufferTime;
    bool _bytesLoaded;
    bool _bytesTotal;
    bool _currentFps;
    bool _onStatus;
    bool _time;
};

struct netstream_as_object : public as_object
{
    NetStream obj;
};

void netstream_new(const fn_call& fn);
void netstream_close(const fn_call& fn);
void netstream_pause(const fn_call& fn);
void netstream_play(const fn_call& fn);
void netstream_seek(const fn_call& fn);
void netstream_setbuffertime(const fn_call& fn);

} // end of gnash namespace

// __NETSTREAM_H__
#endif

