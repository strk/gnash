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

/*  $Id: NetStream.h,v 1.17 2007/01/18 22:53:21 strk Exp $ */

#ifndef __NETSTREAM_H__
#define __NETSTREAM_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <queue>
#include <pthread.h>
#include "impl.h"
#include "video_stream_instance.h"
#include "NetStreamFfmpeg.h"
#include "NetStreamGst.h"

namespace gnash {
  
class netstream_as_object;

class NetStreamBase {
public:
	NetStreamBase(){}
	~NetStreamBase(){}
	void close(){}
	void pause(int /*mode*/){}
	int play(const char* /*source*/){ log_error("FFMPEG or Gstreamer is needed to play video"); return 0; }
	void seek(unsigned int /*pos*/){}
	void setBufferTime(unsigned int /*pos*/){}
	void set_status(const char* /*code*/){}
	void setNetCon(as_object* /*nc*/) {}
	image::image_base* get_video(){ return NULL; }

	inline void set_parent(netstream_as_object* /*ns*/)
	{
	}

	inline bool playing()
	{
		return false;
	}

};

#ifdef SOUND_GST
class NetStream : public NetStreamGst {
#elif defined(USE_FFMPEG)
class NetStream : public NetStreamFfmpeg {
#else
class NetStream : public NetStreamBase {
#endif
public:

};

class netstream_as_object : public as_object
{
	public:
	
	netstream_as_object()
	{
		obj.set_parent(this);
	}

	~netstream_as_object()
	{
	}

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

