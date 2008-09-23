// AudioDecoderGst.cpp: Audio decoding using Gstreamer.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef SOUND_GST

#include "AudioDecoderGst.h"

namespace gnash {
namespace media {

AudioDecoderGst::AudioDecoderGst(AudioInfo& info) :
	_pipeline(NULL),
	_input(NULL),
	_inputcaps(NULL),
	_outputcaps(NULL),
	_output(NULL),
	_decoder(NULL),
	_resampler(NULL),
	_converter(NULL),
	_stop(false),
	_undecodedDataSize(0),
	_undecodedData(NULL),
	_decodedDataSize(0),
	_decodedData(0)
{
    setup(info);
}

AudioDecoderGst::~AudioDecoderGst()
{

	if (_pipeline) {
		_stop = true;
		delete input_lock;
		gst_element_set_state (GST_ELEMENT (_pipeline), GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (_pipeline));
	}
}

void AudioDecoderGst::setup(AudioInfo& info)
{
	if (info.type != FLASH || info.codec != AUDIO_CODEC_MP3)
	{
	    throw MediaException("AudioDecoderGst: cannot handle this codec!");
	}

	// init GStreamer
	gst_init (NULL, NULL);

	// setup the pipeline
	_pipeline = gst_pipeline_new (NULL);

	// Setup the pipeline elements

	// setup fake source
	_input = gst_element_factory_make ("fakesrc", NULL);
	g_object_set (G_OBJECT (_input),	"sizetype", 3, /*"can-activate-pull", FALSE,*/ "signal-handoffs", TRUE, NULL);

	// Setup the callback
	g_signal_connect (_input, "handoff", G_CALLBACK (AudioDecoderGst::callback_handoff), this);

	_decoder = gst_element_factory_make ("mad", NULL);
	if (_decoder == NULL) {
		_decoder = gst_element_factory_make ("flump3dec", NULL);
		if (_decoder != NULL && !gst_default_registry_check_feature_version("flump3dec", 0, 10, 4))
		{
			static bool warned=false;
			if ( ! warned ) 
			{
				// I keep getting these messages even if I hear sound... too much paranoia ?
				log_debug(_("This version of fluendos mp3 plugin does not support flash streaming sounds, please upgrade to version 0.10.4 or higher"));
				warned=true;
			}
		}
	}
	// Check if the element was correctly created
	if (!_decoder) {
		throw MediaException (_("A gstreamer mp3-decoder element could not "
				"be created. You probably need to install a mp3-decoder plugin"
				" like gstreamer0.10-mad or gstreamer0.10-fluendo-mp3."));
	}

	GstCaps *caps = NULL;

	// Set the info about the stream so that gstreamer knows what it is.
	_inputcaps = gst_element_factory_make ("capsfilter", NULL);
	caps = gst_caps_new_simple ("audio/mpeg",
		"mpegversion", G_TYPE_INT, 1,
		"layer", G_TYPE_INT, 3,
		"rate", G_TYPE_INT, _sampleRate,
		"channels", G_TYPE_INT, _stereo ? 2 : 1, NULL);
	g_object_set (G_OBJECT (_inputcaps), "caps", caps, NULL);
	gst_caps_unref (caps);

	// Set the info about the stream so that gstreamer knows what the output should be.
	_outputcaps = gst_element_factory_make ("capsfilter", NULL);
	caps = gst_caps_new_simple ("audio/x-raw-int",
		"rate", G_TYPE_INT, 44100,
		"channels", G_TYPE_INT, 2,
		"width", G_TYPE_INT, 16,
		//"depth", G_TYPE_INT, 16,
		//"signed", G_TYPE_INT, ?,
		NULL);
	g_object_set (G_OBJECT (_outputcaps), "caps", caps, NULL);
	gst_caps_unref (caps);

	// setup the audiosink with callback
	_output = gst_element_factory_make ("fakesink", NULL);
	g_object_set (G_OBJECT (_output), "signal-handoffs", TRUE, NULL);
	g_signal_connect (_output, "handoff", G_CALLBACK (AudioDecoderGst::callback_output), this);


	// Put the elemets in the pipeline and link them
	gst_bin_add_many (GST_BIN (_pipeline), _input, _inputcaps, _decoder, _resampler, _converter, _outputcaps, _output, NULL);

	// link the elements
	gst_element_link_many(_input, _inputcaps, _decoder, _resampler, _converter, _outputcaps, _output, NULL);

	// This make callback_handoff wait for data
	input_lock = new boost::mutex::scoped_lock(input_mutex);

	// This make decodeFrame wait for data
	output_lock = new boost::mutex::scoped_lock(output_mutex);

	// Start "playing"
	gst_element_set_state (GST_ELEMENT (_pipeline), GST_STATE_PLAYING);

}

boost::uint8_t* AudioDecoderGst::decode(boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize, boost::uint32_t& decodedData, bool /*parse*/)
{
	// If there is nothing to decode in the new data we return NULL
	if (input == NULL || inputSize == 0 || !_decoder)
	{
		outputSize = 0;
		decodedData = 0;
		return NULL;
	}

	_undecodedData = input;
	_undecodedDataSize = inputSize;

	delete input_lock;
printf("waiting for decoded data\n");
	output_lock = new boost::mutex::scoped_lock(output_mutex);
printf("decoded data arrived\n");

	decodedData = inputSize;
	outputSize = _decodedDataSize;
	return _decodedData;
}

// The callback function which refills the buffer with data
void
AudioDecoderGst::callback_handoff (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	AudioDecoderGst* decoder = static_cast<AudioDecoderGst*>(user_data);

	if (decoder->_stop) return;

	decoder->input_lock = new boost::mutex::scoped_lock(decoder->input_mutex);

	GST_BUFFER_SIZE(buffer) = decoder->_undecodedDataSize;

	GST_BUFFER_DATA(buffer) = decoder->_undecodedData;
}

// The callback function which passes the decoded audio data
void
AudioDecoderGst::callback_output (GstElement * /*c*/, GstBuffer *buffer, GstPad* /*pad*/, gpointer user_data)
{
	AudioDecoderGst* decoder = static_cast<AudioDecoderGst*>(user_data);

	if (decoder->_stop) return;

	decoder->_decodedDataSize = GST_BUFFER_SIZE(buffer);

	decoder->_decodedData = GST_BUFFER_DATA(buffer);

	delete decoder->output_lock;

}

} // end of media namespace
} // end of gnash namespace

#endif // SOUND_GST

