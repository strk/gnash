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

// $Id: embedVideoDecoderGst.h,v 1.5 2007/07/01 10:54:08 bjacques Exp $

#ifndef __EMBEDVIDEODECODERGST_H__
#define __EMBEDVIDEODECODERGST_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef SOUND_GST

#include "embedVideoDecoder.h"
#include <gst/gst.h>
#include "image.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>

namespace gnash {

class embedVideoDecoderGst : public embedVideoDecoder {
public:
	embedVideoDecoderGst();
	
	~embedVideoDecoderGst();

	void createDecoder(
		int width,
		int height,
		int deblocking,
		bool smoothing,
		int format,
		int outputFormat);

	// gnash calls this when it wants you to decode the given videoframe
	std::auto_ptr<image::image_base> decodeFrame(uint8_t* data, int size);

	// Callback functions used to handle input and output
	static void callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_output (GstElement* /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);

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
	boost::mutex::scoped_lock *input_lock;
	boost::mutex::scoped_lock *output_lock;

	/// Info from the video tag header. Might be usefull...
	uint32_t width;
	uint32_t height;
	int deblocking;
	bool smoothing;
	int format;
	int outputFormat;

	/// Input data and size for current frame
	uint8_t* frame;
	int frameSize;
	
	/// Last decoded frame
	std::auto_ptr<image::image_base> decodedFrame;

	/// If we should stop this will be true
	volatile bool stop;
};

} // end of gnash namespace

#endif // SOUND_GST

#endif //  __EMBEDVIDEODECODERGST_H__
