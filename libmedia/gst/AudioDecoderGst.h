// AudioDecoderGst.h: Audio decoding using Gstreamer.
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


#ifndef GNASH_AUDIODECODERGST_H
#define GNASH_AUDIODECODERGST_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
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
	AudioDecoderGst(AudioInfo& info);

	~AudioDecoderGst();

	boost::uint8_t* decode(boost::uint8_t* /*input*/, boost::uint32_t /*inputSize*/, boost::uint32_t& /*outputSize*/, boost::uint32_t& /*decodedData*/, bool /*parse*/);

	static void callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
	static void callback_output (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data);
private:

	void setup(AudioInfo& info);

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
	boost::uint32_t _sampleRate;
	audioCodecType _format;

	/// If we should stop this will be true
	volatile bool _stop;

	boost::uint32_t _undecodedDataSize;
	boost::uint8_t* _undecodedData;

	boost::uint32_t _decodedDataSize;
	boost::uint8_t* _decodedData;

};

} // media namespace
} // gnash namespace

#endif // __AUDIODECODERGST_H__

