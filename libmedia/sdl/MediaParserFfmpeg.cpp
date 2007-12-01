// MediaParserFfmpeg.cpp: Media parser using ffmpeg
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

// $Id: MediaParserFfmpeg.cpp,v 1.7 2007/12/01 12:58:41 bwy Exp $

#include "MediaParserFfmpeg.h"
#include "log.h"
#include <boost/scoped_array.hpp>

namespace gnash {
namespace media {

MediaParserFfmpeg::MediaParserFfmpeg(boost::shared_ptr<tu_file> stream)
	:
	MediaParser(stream),
	_videoIndex(-1),
	_audioIndex(-1),

	_videoCodecCtx(NULL),
	_audioCodecCtx(NULL),
	_formatCtx(NULL),
	_frame(NULL),

	_lastVideoTimestamp(0),
	_lastAudioTimestamp(0),

	_inputPos(0),
	_maxInputPos(0)
{
}

MediaParserFfmpeg::~MediaParserFfmpeg()
{

}


/// Probe the stream and try to figure out what the format is.
//
/// @param stream the tu_file to use for reading
/// @return a pointer to the AVInputFormat structure containing
///         information about the input format, or NULL.
static AVInputFormat*
probeStream(tu_file* stream)
{
	boost::scoped_array<uint8_t> buffer(new uint8_t[4096]);

	// Probe the file to detect the format
	AVProbeData probe_data;
	probe_data.filename = "";
	probe_data.buf = buffer.get();
	probe_data.buf_size = 4096;

	// Get probe data, making sure the necessary data is available
	if (stream->read_bytes(probe_data.buf, probe_data.buf_size)
				< probe_data.buf_size)
	{
	 	log_error(_("Stream too short to determine input format"));
 		return NULL;
	}

	return av_probe_input_format(&probe_data, 1);
}

bool MediaParserFfmpeg::setupParser()
{

	// This registers all available file formats and codecs 
	// with the library so they will be used automatically when
	// a file with the corresponding format/codec is opened
	// XXX should we call avcodec_init() first?
	av_register_all();

	AVInputFormat* inputFmt = probeStream(_stream.get());
	if (!inputFmt) {
		log_error(_("Couldn't determine stream input format"));
		//pushOnStatus(streamNotFound);
		return false;
	}

	// After the format probe, reset to the beginning of the file.
	_stream->set_position(0);

	// Setup the filereader/seeker mechanism. 7th argument (NULL) is the writer function,
	// which isn't needed.
	init_put_byte(&_byteIOCxt, new uint8_t[500000], 500000, 0, this, MediaParserFfmpeg::readPacket, NULL, MediaParserFfmpeg::seekMedia);
	_byteIOCxt.is_streamed = 1;

	_formatCtx = av_alloc_format_context();

	// Open the stream. the 4th argument is the filename, which we ignore.
	if(av_open_input_stream(&_formatCtx, &_byteIOCxt, "", inputFmt, NULL) < 0){
		log_error(_("Couldn't open stream for decoding"));
		//pushOnStatus(streamNotFound);
		return false;
	}

	// Next, we need to retrieve information about the streams contained in the file
	// This fills the streams field of the AVFormatContext with valid information
	int ret = av_find_stream_info(_formatCtx);
	if (ret < 0)
	{
		log_error(_("Couldn't find stream information, error code: %d"), ret);
		//pushOnStatus(streamNotFound);
		return false;
	}

//	_formatCtx->pb.eof_reached = 0;
//	av_read_play(_formatCtx);

	// Find the first video & audio stream
	_videoIndex = -1;
	_audioIndex = -1;
	//assert(_formatCtx->nb_streams >= 0); useless assert. 
	for (unsigned int i = 0; i <  static_cast<unsigned int>(_formatCtx->nb_streams); i++)
	{
		AVCodecContext* enc = _formatCtx->streams[i]->codec; 

		switch (enc->codec_type)
		{
			case CODEC_TYPE_AUDIO:
				if (_audioIndex < 0)
				{
					_audioIndex = i;
					_audioStream = _formatCtx->streams[i];
				}
				break;

			case CODEC_TYPE_VIDEO:
				if (_videoIndex < 0)
				{
					_videoIndex = i;
					_videoStream = _formatCtx->streams[i];
				}
				break;
			default:
				break;
		}
	}

	if (_videoIndex >= 0)
	{

		// Get a pointer to the codec context for the video stream
		_videoCodecCtx = _formatCtx->streams[_videoIndex]->codec;

/*		// Find the decoder for the video stream
		AVCodec* pCodec = avcodec_find_decoder(_videoCodecCtx->codec_id);
		if (pCodec == NULL)
		{
			_videoCodecCtx = NULL;
			log_error(_("Video decoder %d not found"), _videoCodecCtx->codec_id);
			return false;
		}

		// Open codec
		if (avcodec_open(_videoCodecCtx, pCodec) < 0)
		{
			log_error(_("Could not open codec %d"), _videoCodecCtx->codec_id);
			return false;
		}

		// Allocate a frame to store the decoded frame in
		_frame = avcodec_alloc_frame();*/
		
	}

	if (_audioIndex >= 0) {
		// Get a pointer to the audio codec context for the video stream
		_audioCodecCtx = _formatCtx->streams[_audioIndex]->codec;

/*		// Find the decoder for the audio stream
		AVCodec* pACodec = avcodec_find_decoder(_audioCodecCtx->codec_id);
	    if(pACodec == NULL)
		{
			log_error(_("No available audio decoder %d to process stream"), _audioCodecCtx->codec_id);
			return false;
		}
        
		// Open codec
		if (avcodec_open(_audioCodecCtx, pACodec) < 0)
		{
			log_error(_("Could not open audio codec %d for stream"),_audioCodecCtx->codec_id);
			return false;
		}*/

	}
	return true;
}	

MediaFrame* MediaParserFfmpeg::parseMediaFrame()
{
	AVPacket packet;
	int rc = av_read_frame(_formatCtx, &packet);
	
	if (rc >= 0)
	{
		MediaFrame* ret = new MediaFrame;
		ret->dataSize = packet.size;
		
		// "The input buffer must be FF_INPUT_BUFFER_PADDING_SIZE
		// larger than the actual read bytes because some optimized bitstream
		// readers read 32 or 64 bits at once and could read over the end."
		ret->data = new uint8_t[packet.size + FF_INPUT_BUFFER_PADDING_SIZE];
		
		memcpy(ret->data, packet.data, packet.size);
		
		// "The end of the input buffer should be set to 0 to ensure
		// that no overreading happens for damaged MPEG streams."
		memset(ret->data + packet.size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

		if (packet.stream_index == _audioIndex)
		{
			ret->tag = AUDIO_TAG;

			// set presentation timestamp
			if (packet.dts != static_cast<signed long>(AV_NOPTS_VALUE))
			{
				ret->timestamp = static_cast<uint64_t>(as_double(_audioStream->time_base) * packet.dts * 1000.0);
			}

			if (ret->timestamp != 0)
			{	
				// update audio clock with pts, if present
				_lastAudioTimestamp = ret->timestamp;
			} else {
				ret->timestamp = _lastAudioTimestamp;
			}

			// update video clock for next frame
			uint32_t frame_delay;
			frame_delay = static_cast<uint32_t>((as_double(_audioStream->time_base) * packet.dts) * 1000.0);

			_lastAudioTimestamp += frame_delay;

		} else if (packet.stream_index == _videoIndex) {
			ret->tag = VIDEO_TAG;

			ret->timestamp = 0;

			// set presentation timestamp
			if (packet.dts != static_cast<signed long>(AV_NOPTS_VALUE))
			{
				ret->timestamp = static_cast<uint32_t>((as_double(_videoStream->time_base) * packet.dts) * 1000.0);
			}

			if (ret->timestamp != 0)
			{	
				// update video clock with pts, if present
				_lastVideoTimestamp = ret->timestamp;
			} else {
				ret->timestamp = _lastVideoTimestamp;
			}

			// update video clock for next frame
			uint32_t frame_delay;
			frame_delay = static_cast<uint32_t>(as_double(_videoStream->codec->time_base) * 1000.0);

			// for MPEG2, the frame can be repeated, so we update the clock accordingly
			//frame_delay += static_cast<uint32_t>(_frame->repeat_pict * (frame_delay * 0.5) * 1000.0);

			_lastVideoTimestamp += frame_delay;

		} else {
			delete ret;
			return NULL;
		}
		av_free_packet(&packet);
		return ret;
	} else {
		return NULL;
	}

}

uint32_t MediaParserFfmpeg::seek(uint32_t pos)
{
	long newpos = 0;
	double timebase = 0;
	uint32_t ret = 0;

	AVStream* videostream = _formatCtx->streams[_videoIndex];
	timebase = static_cast<double>(videostream->time_base.num / videostream->time_base.den);
	newpos = static_cast<long>(pos / timebase);

	if (av_seek_frame(_formatCtx, _videoIndex, newpos, 0) < 0) {
		log_error(_("%s: seeking failed"), __FUNCTION__);
		return 0;
	}
	// We have to do parse a new frame to get
	// the new position. This is kindof hackish and ugly :-(
	AVPacket Packet;
	av_init_packet(&Packet);
	double newtime = 0;
	while (newtime == 0) {
		if ( av_read_frame(_formatCtx, &Packet) < 0) {
			av_seek_frame(_formatCtx, -1, 0,AVSEEK_FLAG_BACKWARD);
			av_free_packet(&Packet);
			return 0;
		}

		newtime = timebase * static_cast<double>(_formatCtx->streams[_videoIndex]->cur_dts);
	}

	av_free_packet(&Packet);
	av_seek_frame(_formatCtx, _videoIndex, newpos, 0);
	uint32_t newtime_ms = static_cast<int32_t>(newtime / 1000.0);

	_lastAudioTimestamp = newtime_ms;
	_lastVideoTimestamp = newtime_ms;
	ret = newtime_ms;

	return ret;
}

std::auto_ptr<VideoInfo> 
MediaParserFfmpeg::getVideoInfo() 
{
	if (!_videoCodecCtx || !_videoStream) return std::auto_ptr<VideoInfo>(NULL);

	std::auto_ptr<VideoInfo> ret (new VideoInfo(_videoCodecCtx->codec_id,
												_videoCodecCtx->width, 
												_videoCodecCtx->height,
												static_cast<int16_t>(as_double(_videoStream->r_frame_rate)), // Is this correct? What do we use the framerate for?
												_videoStream->duration,
												FFMPEG));
	ret->videoCodecCtx = _videoCodecCtx;
	return ret;
}

std::auto_ptr<AudioInfo> 
MediaParserFfmpeg::getAudioInfo() 
{
	if (!_audioCodecCtx || !_audioStream) return std::auto_ptr<AudioInfo>(NULL);

	if (_audioCodecCtx->codec_id == CODEC_ID_MP3) _isAudioMp3 = true;

	return std::auto_ptr<AudioInfo>(new AudioInfo(_audioCodecCtx->codec_id,
												_audioCodecCtx->sample_rate,
												_audioCodecCtx->sample_fmt + 1, // see definition of SampleFormat in avcodec.h
												_audioCodecCtx->channels > 1 ? true : false,
												_audioStream->duration,
												FFMPEG));
}


// ffmpeg callback function
int 
MediaParserFfmpeg::readPacket(void* opaque, uint8_t* buf, int buf_size)
{

	MediaParserFfmpeg* decoder = static_cast<MediaParserFfmpeg*>(opaque);
	
	size_t ret = decoder->_stream->read_bytes(static_cast<void*>(buf), buf_size);
	decoder->_inputPos += ret;

	if (decoder->_inputPos > decoder->_maxInputPos) decoder->_maxInputPos = decoder->_inputPos;

	return ret;

}

// ffmpeg callback function
offset_t 
MediaParserFfmpeg::seekMedia(void *opaque, offset_t offset, int whence){

	MediaParserFfmpeg* decoder = static_cast<MediaParserFfmpeg*>(opaque);

	// Offset is absolute new position in the file
	if (whence == SEEK_SET) {
		if (decoder->_stream->set_position(offset) != 0) {
			decoder->_inputPos = decoder->_stream->get_position();
		} else {
			decoder->_inputPos = offset;
		}

	// New position is offset + old position
	} else if (whence == SEEK_CUR) {
		if (decoder->_stream->set_position(decoder->_inputPos + offset) != 0) {
			decoder->_inputPos = decoder->_stream->get_position();
		} else {
			decoder->_inputPos += offset;
		}

	// 	// New position is offset + end of file
	} else if (whence == SEEK_END) {
		// This is (most likely) a file being downloaded, so we can't seek to the end
		// without causing big delays! Instead we seek to 50.000 bytes... seems to work fine...
		if (decoder->_stream->set_position(50000) != 0) {
			decoder->_inputPos = decoder->_stream->get_position();
		} else {
			decoder->_inputPos = 50000;
		}
		
	}

	if (decoder->_inputPos > decoder->_maxInputPos) decoder->_maxInputPos = decoder->_inputPos;

	return decoder->_inputPos;
}

} // gnash.media namespace 
} // namespace gnash
