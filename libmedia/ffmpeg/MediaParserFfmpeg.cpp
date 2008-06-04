// MediaParserFfmpeg.cpp: FFMPG media parsers, for Gnash
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
//


#include "MediaParserFfmpeg.h"
#include "GnashException.h"
#include "log.h"

using namespace std;

//#define PADDING_BYTES 64
//#define READ_CHUNKS 64

namespace gnash {
namespace media {


int
MediaParserFfmpeg::readPacketWrapper(void* opaque, boost::uint8_t* buf, int buf_size)
{
	MediaParserFfmpeg* p = static_cast<MediaParserFfmpeg*>(opaque);
	return p->readPacket(buf, buf_size);
}

offset_t
MediaParserFfmpeg::seekMediaWrapper(void *opaque, offset_t offset, int whence)
{
	MediaParserFfmpeg* p = static_cast<MediaParserFfmpeg*>(opaque);
	return p->seekMedia(offset, whence);
}

AVInputFormat*
MediaParserFfmpeg::probeStream()
{
	boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[2048]);

	// Probe the file to detect the format
	AVProbeData probe_data;
	probe_data.filename = "";
	probe_data.buf = buffer.get();
	probe_data.buf_size = 2048;

	assert(_stream->get_position() == 0);
	size_t actuallyRead = _stream->read_bytes(probe_data.buf, probe_data.buf_size);
	_stream->set_position(0);

	if (actuallyRead < 1)
	{
 		log_error(_("Gnash could not read from movie url"));
 		return NULL;
	}

	probe_data.buf_size = actuallyRead; // right ?
	return av_probe_input_format(&probe_data, 1);
}

boost::uint32_t
MediaParserFfmpeg::getBufferLength()
{
	log_unimpl("%s", __PRETTY_FUNCTION__);
	return 0;
}

bool
MediaParserFfmpeg::nextVideoFrameTimestamp(boost::uint64_t& ts)
{
	log_unimpl("%s", __PRETTY_FUNCTION__);
	return false;
}

std::auto_ptr<EncodedVideoFrame>
MediaParserFfmpeg::nextVideoFrame()
{
	std::auto_ptr<EncodedVideoFrame> ret;
	log_unimpl("%s", __PRETTY_FUNCTION__);
	return ret;
}

bool
MediaParserFfmpeg::nextAudioFrameTimestamp(boost::uint64_t& ts)
{
	log_unimpl("%s", __PRETTY_FUNCTION__);
	return false;
}

std::auto_ptr<EncodedAudioFrame>
MediaParserFfmpeg::nextAudioFrame()
{
	std::auto_ptr<EncodedAudioFrame> ret;

	log_unimpl("%s", __PRETTY_FUNCTION__);
	return ret;
}

VideoInfo*
MediaParserFfmpeg::getVideoInfo()
{
	log_unimpl("%s", __PRETTY_FUNCTION__);
	return 0;
}

AudioInfo*
MediaParserFfmpeg::getAudioInfo()
{
	log_unimpl("%s", __PRETTY_FUNCTION__);
	return 0;
}

boost::uint32_t
MediaParserFfmpeg::seek(boost::uint32_t pos)
{
	log_debug("MediaParserFfmpeg::seek(%d) TESTING", pos);

	AVStream* videostream = _formatCtx->streams[_videoIndex];
    	double timebase = static_cast<double>(videostream->time_base.num / videostream->time_base.den);
	long newpos = static_cast<long>(pos / timebase);
		
	if (av_seek_frame(_formatCtx, _videoIndex, newpos, 0) < 0)
	{
		log_error(_("%s: seeking failed"), __FUNCTION__);
		return 0;
	}

	AVPacket Packet;
	av_init_packet(&Packet);
	double newtime = 0;
	while (newtime == 0)
	{
		if (av_read_frame(_formatCtx, &Packet) < 0) 
		{
			log_error("Error in av_read_frame (while seeking)");
			av_seek_frame(_formatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
			av_free_packet( &Packet );
			return 0; // ??
		}

		newtime = timebase * (double)_formatCtx->streams[_videoIndex]->cur_dts;
	}

	av_free_packet( &Packet );
	av_seek_frame(_formatCtx, _videoIndex, newpos, 0);

	newtime = static_cast<boost::int32_t>(newtime / 1000.0);
	return newtime;
}

bool
MediaParserFfmpeg::parseNextChunk()
{
	log_unimpl("%s", __PRETTY_FUNCTION__);
	return false;
}

boost::uint64_t
MediaParserFfmpeg::getBytesLoaded() const
{
	//log_unimpl("%s", __PRETTY_FUNCTION__);
	return _stream->get_position();
}

MediaParserFfmpeg::MediaParserFfmpeg(std::auto_ptr<tu_file> stream)
	:
	MediaParser(stream),
	_inputFmt(0),
	_formatCtx(0),
	_videoIndex(-1),
	_videoStream(0),
	_audioIndex(-1),
	_audioStream(0)
{
	ByteIOCxt.buffer = NULL;

	_inputFmt = probeStream();
	if ( ! _inputFmt )
	{
		throw GnashException("MediaParserFfmpeg couldn't figure out input format");
	}

	_formatCtx = av_alloc_format_context();

	// Setup the filereader/seeker mechanism. 7th argument (NULL) is the writer function,
	// which isn't needed.
	init_put_byte(&ByteIOCxt,
		new boost::uint8_t[500000], // FIXME: happy leakage !
		500000, // ?
		0, // ?
		this, // opaque pointer to pass to the callbacks
		MediaParserFfmpeg::readPacketWrapper, // packet reader callback
		NULL, // writer callback
		MediaParserFfmpeg::seekMediaWrapper // seeker callback
		);

	ByteIOCxt.is_streamed = 1;

	// Open the stream. the 4th argument is the filename, which we ignore.
	if(av_open_input_stream(&_formatCtx, &ByteIOCxt, "", _inputFmt, NULL) < 0)
	{
		throw GnashException("MediaParserFfmpeg couldn't open input stream");
	}

	// Find audio and video stream
	for (unsigned int i = 0; i < (unsigned)_formatCtx->nb_streams; i++)
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

	log_unimpl("MediaParserFfmpeg");
}

MediaParserFfmpeg::~MediaParserFfmpeg()
{
	if ( _inputFmt )
	{
		// TODO: how to release an AVInputFormat ?
	}

	if ( _formatCtx )
	{
		// TODO: how to release a AVFormatContext 
		av_close_input_file(_formatCtx);
	}
}

int 
MediaParserFfmpeg::readPacket(boost::uint8_t* buf, int buf_size)
{

	assert( _stream.get() );
	tu_file& in = *_stream;

	size_t ret = in.read_bytes(static_cast<void*>(buf), buf_size);

	// Update _lastParsedPosition
	boost::uint64_t curPos = in.get_position();
	if ( curPos > _lastParsedPosition ) _lastParsedPosition = curPos;

	return ret;

}

offset_t 
MediaParserFfmpeg::seekMedia(offset_t offset, int whence)
{

	assert(_stream.get());
	tu_file& in = *(_stream);

	// Offset is absolute new position in the file
	if (whence == SEEK_SET)
	{	
		in.set_position(offset);
		// New position is offset + old position
	}
	else if (whence == SEEK_CUR)
	{
		in.set_position(in.get_position() + offset);
		// New position is offset + end of file
	}
	else if (whence == SEEK_END)
	{
		// This is (most likely) a streamed file, so we can't seek to the end!
		// Instead we seek to 50.000 bytes... seems to work fine...
		in.set_position(50000);

	}

	return in.get_position(); 
}


} // end of gnash::media namespace
} // end of gnash namespace

#undef PADDING_BYTES
#undef READ_CHUNKS 
