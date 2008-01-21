// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: NetStreamGst.h,v 1.30 2008/01/21 07:07:28 bjacques Exp $ */

#ifndef __NETSTREAMGST_H__
#define __NETSTREAMGST_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef SOUND_GST

#include <boost/bind.hpp> 
#include "impl.h"
#include "video_stream_instance.h"
#include <gst/gst.h>
#include "image.h"
#include "NetStream.h" // for inheritance


namespace gnash {
  
class NetStreamGst: public NetStream {
public:
  NetStreamGst();
  ~NetStreamGst();

  void close();

  void pause(PauseMode mode);

  void play(const std::string& url);

  void seek(boost::uint32_t pos);
	
  void advance();
	
  boost::int32_t time();
	  
  double getCurrentFPS();
  
  long bytesLoaded();

  long bytesTotal();
	
  static void video_data_cb(GstElement* /*c*/, GstBuffer *buffer, GstPad* pad,
                           gpointer user_data);
  
  static void
  decodebin_newpad_cb(GstElement* decodebin, GstPad* pad, gboolean last,
                      gpointer user_data);
                      
  static void queue_underrun_cb(GstElement *queue, gpointer  user_data);
  
  static void queue_running_cb(GstElement *queue, gpointer  user_data);


private:
  void handleMessage (GstMessage *message);

  GstElement* _pipeline;
  GstElement* _dataqueue;
  GstElement* _downloader;
  GstPad*     _videopad;
  GstPad*     _audiopad;
  gint64      _duration;
};

} // gnash namespace

#endif // SOUND_GST

#endif //  __NETSTREAMGST_H__
