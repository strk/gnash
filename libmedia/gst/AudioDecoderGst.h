// AudioDecoderGst.h: Audio decoding using Gstreamer.
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

// $Id: AudioDecoderGst.h,v 1.1 2007/11/30 00:13:01 tgc Exp $

#ifndef __AUDIODECODERGST_H__
#define __AUDIODECODERGST_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "AudioDecoder.h"

#include <gst/gst.h>
#include "image.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp> 
#include <boost/thread/mutex.hpp>

namespace gnash {
namespace media {

/// Video decoding using Gstreamer.
class AudioDecoderGst : public AudioDecoder {
	
public:
	AudioDecoderGst();
	~AudioDecoderGst();

	bool setup(AudioInfo* info);

	uint8_t* decode(uint8_t* /*input*/, uint32_t /*inputSize*/, uint32_t& /*outputSize*/, uint32_t& /*decodedData*/, bool /*parse*/);

	static void callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_output (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
private:

	// gstreamer pipeline objects

	/// the main bin containing the elements
	GstElement* _pipeline;

	/// Gstreamer objects
	GstElement* _input;
	GstElement* _inputcaps;
	GstElement* _outputcaps;
	GstElement* _output;
	GstElement* _decoder;
	GstElement* _resampler;
	GstElement* _converter;

	/// mutexes and locks used to handle input and output.
	boost::mutex input_mutex;
	boost::mutex output_mutex;
	boost::mutex::scoped_lock* input_lock;
	boost::mutex::scoped_lock* output_lock;

	/// Info from the video tag header. Might be usefull...
	bool _stereo;
	uint32_t _sampleRate;
	audioCodecType _format;

	/// If we should stop this will be true
	volatile bool _stop;

	uint32_t _undecodedDataSize;
	uint8_t* _undecodedData;

	uint32_t _decodedDataSize;
	uint8_t* _decodedData;

};

} // media namespace
} // gnash namespace

#endif // __AUDIODECODERGST_H__

