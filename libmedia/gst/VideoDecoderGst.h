// VideoDecoderGst.h: Video decoding using Gstreamer.
// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

// $Id: VideoDecoderGst.h,v 1.10 2007/12/12 10:23:06 zoulunkai Exp $

#ifndef __VIDEODECODERGST_H__
#define __VIDEODECODERGST_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "VideoDecoder.h"

#include <gst/gst.h>
#include "image.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>

namespace gnash {
namespace media {

/// Video decoding using Gstreamer.
class VideoDecoderGst : public VideoDecoder {
	
public:
	VideoDecoderGst();
	~VideoDecoderGst();

	bool setup(VideoInfo* info);

	bool setup(
		int /*width*/,
		int /*height*/,
		int /*deblocking*/,
		bool /*smoothing*/,
		videoCodecType /*format*/,
		int /*outputFormat*/);

	//boost::uint8_t* decode(boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize);

	std::auto_ptr<image::image_base> decodeToImage(boost::uint8_t* /*input*/, boost::uint32_t /*inputSize*/);

	static void callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_output (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
private:

	// gstreamer pipeline objects

	/// the main bin containing the elements
	GstElement *pipeline;

	/// Gstreamer objects
	GstElement *input;
	GstElement *inputcaps;
	GstElement *videocaps;
	GstElement *output;
	GstElement *decoder;
	GstElement *colorspace;

	/// mutexes and locks used to handle input and output.
	boost::mutex input_mutex;
	boost::mutex output_mutex;
	boost::mutex::scoped_lock* input_lock;
	boost::mutex::scoped_lock* output_lock;

	/// Info from the video tag header. Might be usefull...
	boost::uint32_t width;
	boost::uint32_t height;
	int deblocking;
	bool smoothing;
	videoCodecType format;
	int outputFormat;

	/// Input data and size for current frame
	boost::uint8_t* frame;
	int frameSize;
	
	/// Last decoded frame
	std::auto_ptr<image::image_base> decodedFrame;

	/// If we should stop this will be true
	volatile bool stop;

};
	
} // gnash.media namespace 
} // gnash namespace

#endif // __VIDEODECODERGST_H__
