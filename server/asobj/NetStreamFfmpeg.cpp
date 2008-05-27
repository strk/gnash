// NetStreamFfmpeg.cpp:  Network streaming for FFMPEG video library, for Gnash.
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
//


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef USE_FFMPEG

#include "NetStreamFfmpeg.h"
#include "log.h"
#include "fn_call.h"
#include "NetStream.h"
#include "render.h"	
#include "movie_root.h"
#include "sound_handler.h"
#include "VideoDecoderFfmpeg.h"
#include "SystemClock.h"

#include "FLVParser.h" 

#include <boost/scoped_array.hpp>
#include <algorithm> // std::min


#if defined(_WIN32) || defined(WIN32)
# include <windows.h>	// for sleep()
# define usleep(x) Sleep(x/1000)
#else
# include "unistd.h" // for usleep()
#endif

/// Define this to add debugging prints for locking
//#define GNASH_DEBUG_THREADS

// Define the following macro to have status notification handling debugged
//#define GNASH_DEBUG_STATUS

// Define the following macro to have decoding activity  debugged
//#define GNASH_DEBUG_DECODING

namespace {

// Used to free data in the AVPackets we create our self
void avpacket_destruct(AVPacket* av)
{
	delete [] av->data;
}

} // anonymous namespace


namespace gnash {


NetStreamFfmpeg::NetStreamFfmpeg()
	:

	_decoding_state(DEC_NONE),

	m_video_index(-1),
	m_audio_index(-1),

	m_VCodecCtx(NULL),
	m_ACodecCtx(NULL),
	m_FormatCtx(NULL),
	m_Frame(NULL),

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	_parserThread(NULL),
	_parserThreadBarrier(2), // main and decoder threads
	_parserKillRequest(false),
#endif

	m_last_video_timestamp(0),
	m_last_audio_timestamp(0),

	_playbackClock(new InterruptableVirtualClock(new SystemClock)),
	_playHead(_playbackClock.get()), 

	m_unqueued_data(NULL),

	_decoderBuffer(0),
	_soundHandler(get_sound_handler())
{

	ByteIOCxt.buffer = NULL;
}

NetStreamFfmpeg::~NetStreamFfmpeg()
{
	if ( _decoderBuffer ) delete [] _decoderBuffer;

	close(); // close will also detach from sound handler

	delete m_imageframe;
}


void NetStreamFfmpeg::pause( PauseMode mode )
{
	log_debug("::pause(%d) called ", mode);
	switch ( mode )
	{
		case pauseModeToggle:
			if ( _playHead.getState() == PlayHead::PLAY_PAUSED) unpausePlayback();
			else pausePlayback();
			break;
		case pauseModePause:
			pausePlayback();
			break;
		case pauseModeUnPause:
			unpausePlayback();
			break;
		default:
			break;
	}

}

void NetStreamFfmpeg::close()
{
	GNASH_REPORT_FUNCTION;

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	killParserThread();
#endif

	// When closing gnash before playback is finished, the soundhandler 
	// seems to be removed before netstream is destroyed.
	if (_soundHandler)
	{
		_soundHandler->detach_aux_streamer(this);
	}

	if (m_Frame) av_free(m_Frame);
	m_Frame = NULL;

  if ( m_VCodecCtx ) {
    avcodec_close( m_VCodecCtx );
  }
  m_VCodecCtx = NULL;

  if ( m_ACodecCtx ) {
    avcodec_close( m_ACodecCtx );
  }
  m_ACodecCtx = NULL;

	if (m_FormatCtx)
	{
		m_FormatCtx->iformat->flags = AVFMT_NOFILE;
		av_close_input_file(m_FormatCtx);
		m_FormatCtx = NULL;
	}

	delete m_imageframe;
	m_imageframe = NULL;
	delete m_unqueued_data;
	m_unqueued_data = NULL;

	delete [] ByteIOCxt.buffer;

}

// ffmpeg callback function
int 
NetStreamFfmpeg::readPacket(void* opaque, boost::uint8_t* buf, int buf_size)
{

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(opaque);

	assert( ns->_inputStream.get() );
	tu_file& in = *(ns->_inputStream);

	size_t ret = in.read_bytes(static_cast<void*>(buf), buf_size);
	ns->inputPos += ret; // what for ??
	return ret;

}

// ffmpeg callback function
offset_t 
NetStreamFfmpeg::seekMedia(void *opaque, offset_t offset, int whence)
{

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(opaque);

	tu_file& in = *(ns->_inputStream);

	// Offset is absolute new position in the file
	if (whence == SEEK_SET)
	{	
		in.set_position(offset);
		ns->inputPos = offset; // what for ?!

	// New position is offset + old position
	}
	else if (whence == SEEK_CUR)
	{
		in.set_position(ns->inputPos + offset);
		ns->inputPos = ns->inputPos + offset; // what for ?!

	// New position is offset + end of file
	}
	else if (whence == SEEK_END)
	{
		// This is (most likely) a streamed file, so we can't seek to the end!
		// Instead we seek to 50.000 bytes... seems to work fine...
		in.set_position(50000);
		ns->inputPos = 50000; // what for ?!

	}

	return ns->inputPos; // ah, thats why ! :/
}

void
NetStreamFfmpeg::play(const std::string& c_url)
{
	// Is it already playing ?
	if ( m_parser.get() )
	{
		// TODO: check what to do in these cases
		log_error("FIXME: NetStream.play() called while already streaming");
		return;
	}

	// Does it have an associated NetConnection ?
	if ( ! _netCon )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("No NetConnection associated with this NetStream, won't play"));
		);
		return;
	}

	url = c_url;

	// Remove any "mp3:" prefix. Maybe should use this to mark as audio-only
	if (url.compare(0, 4, std::string("mp3:")) == 0)
	{
		url = url.substr(4);
	}

	// TODO: check what is this needed for, I'm not sure it would be needed..
	url = _netCon->validateURL(url);
	if (url.empty())
	{
		log_error("Couldn't load URL %s", c_url);
		return;
	}

	log_security( _("Connecting to movie: %s"), url );

	StreamProvider& streamProvider = StreamProvider::getDefaultInstance();
	_inputStream.reset( streamProvider.getStream( url ) );

	if ( ! _inputStream.get() )
	{
		log_error( _("Gnash could not open this url: %s"), url );
		setStatus(streamNotFound);
		return;
	}

	// We need to start playback
	if (!startPlayback())
	{
		log_error("NetStream.play(%s): failed starting playback", c_url);
		return;
	}

	// We need to restart the audio
	if (_soundHandler)
		_soundHandler->attach_aux_streamer(audio_streamer, this);

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	// This starts the parser thread
	_parserThread = new boost::thread(boost::bind(NetStreamFfmpeg::parseAllInput, this)); 
	_parserThreadBarrier.wait();
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

	return;
}

/// Finds a decoder, allocates a context and initializes it.
//
/// @param codec_id the codec ID to find
/// @return the initialized context, or NULL on failure. The caller is 
///         responsible for deallocating!
static AVCodecContext*
initContext(enum CodecID codec_id)
{

	AVCodec* codec = avcodec_find_decoder(codec_id);
	if (!codec)
	{
		log_error(_("libavcodec couldn't find decoder"));
		return NULL;
	}

	AVCodecContext * context = avcodec_alloc_context();
	if (!context)
	{
		log_error(_("libavcodec couldn't allocate context"));
		return NULL;
	}

	int rv = avcodec_open(context, codec);
	if (rv < 0) 
	{
		avcodec_close(context);
		log_error(_("libavcodec failed to initialize codec"));
		return NULL;
	}

	return context;
}

/// Gets video info from the parser and initializes the codec.
//
/// @param parser the parser to use to get video information.
/// @return the initialized context, or NULL on failure. The caller
///         is responsible for deallocating this pointer.
static AVCodecContext* 
initFlvVideo(FLVParser& parser)
{
	// Get video info from the parser
	FLVVideoInfo* videoInfo = parser.getVideoInfo();
	if (!videoInfo)
	{
		return NULL;
	}

	enum CodecID codec_id;

	// Find the decoder and init the parser
	switch(videoInfo->codec)
	{
		case media::VIDEO_CODEC_H263:
			codec_id = CODEC_ID_FLV1;
			break;
#ifdef FFMPEG_VP6
		case media::VIDEO_CODEC_VP6:
			codec_id = CODEC_ID_VP6F;
			break;
#endif
		case media::VIDEO_CODEC_SCREENVIDEO:
			codec_id = CODEC_ID_FLASHSV;
			break;
		default:
			log_error(_("Unsupported video codec %d"), (int) videoInfo->codec);
			return NULL;
	}

	return initContext(codec_id);
}


/// Like initFlvVideo, but for audio.
static AVCodecContext*
initFlvAudio(FLVParser& parser)
{
	// Get audio info from the parser
	FLVAudioInfo* audioInfo =  parser.getAudioInfo();
	if (!audioInfo)
	{
		log_debug("No audio in FLV stream");
		return NULL;
	}

	enum CodecID codec_id;

	switch(audioInfo->codec)
	{
		case media::AUDIO_CODEC_RAW:
			codec_id = CODEC_ID_PCM_U16LE;
			break;
		case media::AUDIO_CODEC_ADPCM:
			codec_id = CODEC_ID_ADPCM_SWF;
			break;
		case media::AUDIO_CODEC_MP3:
			codec_id = CODEC_ID_MP3;
			break;
		default:
			log_error(_("Unsupported audio codec %d"), (int)audioInfo->codec);
			return NULL;
	}

	return initContext(codec_id);
}


/// Probe the stream and try to figure out what the format is.
//
/// @param ns the netstream to use for reading
/// @return a pointer to the AVInputFormat structure containing
///         information about the input format, or NULL.
static AVInputFormat*
probeStream(NetStreamFfmpeg* ns)
{
	boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[2048]);

	// Probe the file to detect the format
	AVProbeData probe_data;
	probe_data.filename = "";
	probe_data.buf = buffer.get();
	probe_data.buf_size = 2048;

	if (ns->readPacket(ns, probe_data.buf, probe_data.buf_size) < 1)
	{
 		log_error(_("Gnash could not read from movie url"));
 		return NULL;
	}

	return av_probe_input_format(&probe_data, 1);
}

bool
NetStreamFfmpeg::startPlayback()
{
	assert(_inputStream.get());
	assert(_inputStream->get_position() == 0);

	inputPos = 0;

	// Check if the file is a FLV, in which case we use our own parser
	char head[4] = {0, 0, 0, 0};
	if (_inputStream->read_bytes(head, 3) < 3)
	{
		log_error(_("Could not read 3 bytes from NetStream input"));
		// not really correct, the stream was found, just wasn't what we expected..
		setStatus(streamNotFound);
		return false;
	}


	_inputStream->set_position(0);
	if (std::string(head) == "FLV")
	{
		m_isFLV = true;
		assert ( !m_parser.get() );

		m_parser.reset( new FLVParser(_inputStream) ); 
		assert(! _inputStream.get() ); // TODO: when ownership will be transferred...

		if (! m_parser.get() )
		{
			log_error(_("Gnash could not open FLV movie: %s"), url.c_str());
			// not really correct, the stream was found, just wasn't what we expected..
			setStatus(streamNotFound);
			return false;
		}

		// Init the avdecoder-decoder
		avcodec_init();
		avcodec_register_all();

		m_VCodecCtx = initFlvVideo(*m_parser);
		if (!m_VCodecCtx)
		{
			log_error(_("Failed to initialize FLV video codec"));
			return false;
		}

		m_ACodecCtx = initFlvAudio(*m_parser);
		if (!m_ACodecCtx)
		{
			// There might simply be no audio, no problem...
			//log_error(_("Failed to initialize FLV audio codec"));
			//return false;
		}

		// We just define the indexes here, they're not really used when
		// the file format is FLV
		m_video_index = 0;
		m_audio_index = 1;

		// Allocate a frame to store the decoded frame in
		m_Frame = avcodec_alloc_frame();
	}
	else
	{

		// This registers all available file formats and codecs 
		// with the library so they will be used automatically when
		// a file with the corresponding format/codec is opened
		// XXX should we call avcodec_init() first?
		av_register_all();

		AVInputFormat* inputFmt = probeStream(this);
		if (!inputFmt)
		{
			log_error(_("Couldn't determine stream input format from URL %s"), url.c_str());
			return false;
		}

		// After the format probe, reset to the beginning of the file.
		// TODO: have this done by probeStream !
		//       (actually, have the whole thing done by MediaParser)
		_inputStream->set_position(0);

		// Setup the filereader/seeker mechanism. 7th argument (NULL) is the writer function,
		// which isn't needed.
		init_put_byte(&ByteIOCxt, new boost::uint8_t[500000], 500000, 0, this, NetStreamFfmpeg::readPacket, NULL, NetStreamFfmpeg::seekMedia);
		ByteIOCxt.is_streamed = 1;

		m_FormatCtx = av_alloc_format_context();

		// Open the stream. the 4th argument is the filename, which we ignore.
		if(av_open_input_stream(&m_FormatCtx, &ByteIOCxt, "", inputFmt, NULL) < 0)
		{
			log_error(_("Couldn't open file '%s' for decoding"), url.c_str());
			setStatus(streamNotFound);
			return false;
		}

		// Next, we need to retrieve information about the streams contained in the file
		// This fills the streams field of the AVFormatContext with valid information
		int ret = av_find_stream_info(m_FormatCtx);
		if (ret < 0)
		{
			log_error(_("Couldn't find stream information from '%s', error code: %d"), url.c_str(), ret);
			return false;
		}

	//	m_FormatCtx->pb.eof_reached = 0;
	//	av_read_play(m_FormatCtx);

		// Find the first video & audio stream
		m_video_index = -1;
		m_audio_index = -1;
		//assert(m_FormatCtx->nb_streams >= 0); useless assert. 
		for (unsigned int i = 0; i < (unsigned)m_FormatCtx->nb_streams; i++)
		{
			AVCodecContext* enc = m_FormatCtx->streams[i]->codec; 

			switch (enc->codec_type)
			{
				case CODEC_TYPE_AUDIO:
					if (m_audio_index < 0)
					{
						m_audio_index = i;
						m_audio_stream = m_FormatCtx->streams[i];
					}
					break;

				case CODEC_TYPE_VIDEO:
					if (m_video_index < 0)
					{
						m_video_index = i;
						m_video_stream = m_FormatCtx->streams[i];
					}
					break;
				default:
					break;
			}
		}

		if (m_video_index < 0)
		{
			log_error(_("Didn't find a video stream from '%s'"), url.c_str());
			return false;
		}

		// Get a pointer to the codec context for the video stream
		m_VCodecCtx = m_FormatCtx->streams[m_video_index]->codec;

		// Find the decoder for the video stream
		AVCodec* pCodec = avcodec_find_decoder(m_VCodecCtx->codec_id);
		if (pCodec == NULL)
		{
			m_VCodecCtx = NULL;
			log_error(_("Video decoder %d not found"), 
				m_VCodecCtx->codec_id);
			return false;
		}

		// Open codec
		if (avcodec_open(m_VCodecCtx, pCodec) < 0)
		{
			log_error(_("Could not open codec %d"),
				m_VCodecCtx->codec_id);
		}

		// Allocate a frame to store the decoded frame in
		m_Frame = avcodec_alloc_frame();

		if ( m_audio_index >= 0 && _soundHandler )
		{
			// Get a pointer to the audio codec context for the video stream
			m_ACodecCtx = m_FormatCtx->streams[m_audio_index]->codec;

			// Find the decoder for the audio stream
			AVCodec* pACodec = avcodec_find_decoder(m_ACodecCtx->codec_id);
			if (pACodec == NULL)
			{
				log_error(_("No available audio decoder %d to process MPEG file: '%s'"), 
					m_ACodecCtx->codec_id, url.c_str());
				return false;
			}
		
			// Open codec
			if (avcodec_open(m_ACodecCtx, pACodec) < 0)
			{
				log_error(_("Could not open audio codec %d for %s"),
					m_ACodecCtx->codec_id, url.c_str());
				return false;
			}

		}
	}

	//_playHead.init(m_VCodecCtx!=0, false); // second arg should be m_ACodecCtx!=0, but we're testing video only for now
	_playHead.init(m_VCodecCtx!=0, m_ACodecCtx!=0);
	_playHead.setState(PlayHead::PLAY_PLAYING);

	decodingStatus(DEC_BUFFERING);

#ifdef GNASH_DEBUG_STATUS
	log_debug("Setting playStart status");
#endif // GNASH_DEBUG_STATUS
	setStatus(playStart);

	return true;
}


/// Copy RGB data from a source raw_mediadata_t to a destination image::rgb.
/// @param dst the destination image::rgb, which must already be initialized
///            with a buffer of size of at least src.m_size.
/// @param src the source raw_mediadata_t to copy data from. The m_size member
///            of this structure must be initialized.
/// @param width the width, in bytes, of a row of video data.
static void
rgbcopy(image::rgb* dst, media::raw_mediadata_t* src, int width)
{
  assert( src->m_size <= static_cast<boost::uint32_t>(dst->width() * dst->height() * 3) ); 

  boost::uint8_t* dstptr = dst->data();

  boost::uint8_t* srcptr = src->m_data;
  boost::uint8_t* srcend = src->m_data + src->m_size;

  while (srcptr < srcend) {
    memcpy(dstptr, srcptr, width);
    dstptr += dst->pitch();
    srcptr += width;
  }
}

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
// to be run in parser thread
void NetStreamFfmpeg::parseAllInput(NetStreamFfmpeg* ns)
{
	//GNASH_REPORT_FUNCTION;

	ns->_parserThreadBarrier.wait();

	// Parse in a thread...
	while ( 1 )
	{
		// this one will lock _parserKillRequestMutex
		if ( ns->parserThreadKillRequested() ) break;

		{
			boost::mutex::scoped_lock lock(ns->_parserMutex);
			if ( ns->m_parser->parsingCompleted() ) break;
			ns->m_parser->parseNextTag();
		}

		usleep(10); // task switch (after lock was released!)
	}
}

void
NetStreamFfmpeg::killParserThread()
{
	GNASH_REPORT_FUNCTION;

	{
		boost::mutex::scoped_lock lock(_parserKillRequestMutex);
		_parserKillRequest = true;
	}

	// might as well be never started
	if ( _parserThread )
	{
		_parserThread->join();
	}

	delete _parserThread;
	_parserThread = NULL;
}

bool
NetStreamFfmpeg::parserThreadKillRequested()
{
	boost::mutex::scoped_lock lock(_parserKillRequestMutex);
	return _parserKillRequest;
}

#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

// audio callback is running in sound handler thread
bool NetStreamFfmpeg::audio_streamer(void *owner, boost::uint8_t *stream, int len)
{
	//GNASH_REPORT_FUNCTION;

	NetStreamFfmpeg* ns = static_cast<NetStreamFfmpeg*>(owner);

	boost::mutex::scoped_lock lock(ns->_audioQueueMutex);

#if 0
	log_debug("audio_streamer called, audioQueue size: %d, "
		"requested %d bytes of fill-up",
		ns->_audioQueue.size(), len);
#endif


	while (len > 0)
	{

		if ( ns->_audioQueue.empty() )
		{
			break;
		}

    		media::raw_mediadata_t* samples = ns->_audioQueue.front();

		int n = std::min<int>(samples->m_size, len);
		memcpy(stream, samples->m_ptr, n);
		stream += n;
		samples->m_ptr += n;
		samples->m_size -= n;
		len -= n;

		if (samples->m_size == 0)
		{
			delete samples;
			ns->_audioQueue.pop_front();
		}

	}

	return true;
}

media::raw_mediadata_t*
NetStreamFfmpeg::getDecodedVideoFrame(boost::uint32_t ts)
{
	assert(m_parser.get());
	if ( ! m_parser.get() )
	{
		log_error("getDecodedVideoFrame: no parser available");
		return 0; // no parser, no party
	}

	FLVVideoFrameInfo* info = m_parser->peekNextVideoFrameInfo();
	if ( ! info )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("getDecodedVideoFrame(%d): no more video frames in input (peekNextVideoFrameInfo returned false)");
#endif // GNASH_DEBUG_DECODING
		decodingStatus(DEC_STOPPED);
		return 0;
	}

	if ( info->timestamp > ts )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.getDecodedVideoFrame(%d): next video frame is in the future (%d)",
			this, ts, info->timestamp);
#endif // GNASH_DEBUG_DECODING
		return 0; // next frame is in the future
	}

	// Loop until a good frame is found
    	media::raw_mediadata_t* video = 0;
	while ( 1 )
	{
    		video = decodeNextVideoFrame();
		if ( ! video )
		{
			log_error("peekNextVideoFrameInfo returned some info, "
				"but decodeNextVideoFrame returned null, "
				"I don't think this should ever happen");
			break;
		}

		FLVVideoFrameInfo* info = m_parser->peekNextVideoFrameInfo();
		if ( ! info )
		{
			// the one we decoded was the last one
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.getDecodedVideoFrame(%d): last video frame decoded "
				"(should set playback status to STOP?)", this, ts);
#endif // GNASH_DEBUG_DECODING
			break;
		}
		if ( info->timestamp > ts )
		{
			// the next one is in the future, we'll return this one.
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.getDecodedVideoFrame(%d): "
				"next video frame is in the future, "
				"we'll return this one",
				this, ts);
#endif // GNASH_DEBUG_DECODING
			break; // the one we decoded
		}
	}

	return video;
}

media::raw_mediadata_t*
NetStreamFfmpeg::decodeNextVideoFrame()
{
	if ( ! m_parser.get() )
	{
		log_error("decodeNextVideoFrame: no parser available");
		return 0; // no parser, no party
	}

	FLVFrame* frame = m_parser->nextVideoFrame(); 
	if (frame == NULL)
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.decodeNextVideoFrame(): "
			"no more video frames in input",
			this);
#endif // GNASH_DEBUG_DECODING
		return 0;
	}
	assert (frame->type == videoFrame);

  	AVPacket packet;

  	packet.destruct = avpacket_destruct; // needed ?
  	packet.size = frame->dataSize;
  	packet.data = frame->data;
  	// FIXME: is this the right value for packet.dts?
  	packet.pts = packet.dts = static_cast<boost::int64_t>(frame->timestamp);
	assert (frame->type == videoFrame);
	packet.stream_index = 0;

	return decodeVideo(&packet);
}

media::raw_mediadata_t*
NetStreamFfmpeg::decodeNextAudioFrame()
{
	assert ( m_parser.get() );

	FLVFrame* frame = m_parser->nextAudioFrame(); 
	if (frame == NULL)
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.decodeNextAudioFrame: "
			"no more video frames in input",
			this);
#endif // GNASH_DEBUG_DECODING
		return 0;
	}
	assert (frame->type == audioFrame);

  	AVPacket packet;

  	packet.destruct = avpacket_destruct;
  	packet.size = frame->dataSize;
  	packet.data = frame->data;
  	// FIXME: is this the right value for packet.dts?
  	packet.pts = packet.dts = static_cast<boost::int64_t>(frame->timestamp);
	assert(frame->type == audioFrame);
	packet.stream_index = 1;

	return decodeAudio(&packet);
}

bool
NetStreamFfmpeg::decodeFLVFrame()
{
#if 1
	abort();
	return false;
#else
	FLVFrame* frame = m_parser->nextMediaFrame(); // we don't care which one, do we ?

	if (frame == NULL)
	{
		//assert ( _netCon->loadCompleted() );
		//assert ( m_parser->parsingCompleted() );
		decodingStatus(DEC_STOPPED);
		return true;
	}

  	AVPacket packet;

  	packet.destruct = avpacket_destruct;
  	packet.size = frame->dataSize;
  	packet.data = frame->data;
  	// FIXME: is this the right value for packet.dts?
  	packet.pts = packet.dts = static_cast<boost::int64_t>(frame->timestamp);

	if (frame->type == videoFrame)
	{
    		packet.stream_index = 0;
		media::raw_mediadata_t* video = decodeVideo(&packet);
		assert (m_isFLV);
		if (video)
		{
			// NOTE: Caller is assumed to have locked _qMutex already
			if ( ! m_qvideo.push(video) )
			{
				log_error("Video queue full !");
			}
		}
	}
	else
	{
		assert(frame->type == audioFrame);
    		packet.stream_index = 1;
		media::raw_mediadata_t* audio = decodeAudio(&packet);
		if ( audio )
		{
			if ( ! m_qaudio.push(audio) )
			{
				log_error("Audio queue full!");
			}
		}
	}

	return true;
#endif
}


media::raw_mediadata_t* 
NetStreamFfmpeg::decodeAudio( AVPacket* packet )
{
	if (!m_ACodecCtx) return 0;

	int frame_size;
	//static const unsigned int bufsize = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;
	static const unsigned int bufsize = AVCODEC_MAX_AUDIO_FRAME_SIZE;

	if ( ! _decoderBuffer ) _decoderBuffer = new boost::uint8_t[bufsize];

	boost::uint8_t* ptr = _decoderBuffer;

#ifdef FFMPEG_AUDIO2
	frame_size = bufsize; // TODO: is it safe not initializing this ifndef FFMPEG_AUDIO2 ?
	if (avcodec_decode_audio2(m_ACodecCtx, (boost::int16_t*) ptr, &frame_size, packet->data, packet->size) >= 0)
#else
	if (avcodec_decode_audio(m_ACodecCtx, (boost::int16_t*) ptr, &frame_size, packet->data, packet->size) >= 0)
#endif
	{

		bool stereo = m_ACodecCtx->channels > 1 ? true : false;
		int samples = stereo ? frame_size >> 2 : frame_size >> 1;
		
		if (_resampler.init(m_ACodecCtx))
		{
			// Resampling is needed.
			
			// Compute new size based on frame_size and
			// resampling configuration
			double resampleFactor = (44100.0/m_ACodecCtx->sample_rate) * (2.0/m_ACodecCtx->channels);
			int resampledFrameSize = int(ceil(frame_size*resampleFactor));

			// Allocate just the required amount of bytes
			boost::uint8_t* output = new boost::uint8_t[resampledFrameSize];
			
			samples = _resampler.resample(reinterpret_cast<boost::int16_t*>(ptr), 
							 reinterpret_cast<boost::int16_t*>(output), 
							 samples);

			if (resampledFrameSize < samples*2*2)
			{
				log_error(" --- Computation of resampled frame size (%d) < then the one based on samples (%d)",
					resampledFrameSize, samples*2*2);

				log_debug(" input frame size: %d", frame_size);
				log_debug(" input sample rate: %d", m_ACodecCtx->sample_rate);
				log_debug(" input channels: %d", m_ACodecCtx->channels);
				log_debug(" input samples: %d", samples);

				log_debug(" output sample rate (assuming): %d", 44100);
				log_debug(" output channels (assuming): %d", 2);
				log_debug(" output samples: %d", samples);

				abort(); // the call to resample() likely corrupted memory...
			}

			frame_size = samples*2*2;

			// ownership of memory pointed-to by 'ptr' will be
			// transferred below
			ptr = reinterpret_cast<boost::uint8_t*>(output);

			// we'll reuse _decoderBuffer 
		}
		else
		{
			// ownership of memory pointed-to by 'ptr' will be
			// transferred below, so we reset _decoderBuffer here.
			// Doing so, next time we'll need to decode we'll create
			// a new buffer
			_decoderBuffer=0;
		}
		
    		media::raw_mediadata_t* raw = new media::raw_mediadata_t();
		
		raw->m_data = ptr; // ownership of memory pointed by 'ptr' transferred here
		raw->m_ptr = raw->m_data;
		raw->m_size = frame_size;
		raw->m_stream_index = m_audio_index;

		// set presentation timestamp
		if (packet->dts != static_cast<signed long>(AV_NOPTS_VALUE))
		{
			if (!m_isFLV) raw->m_pts = static_cast<boost::uint32_t>(as_double(m_audio_stream->time_base) * packet->dts * 1000.0);
			else raw->m_pts = static_cast<boost::uint32_t>((as_double(m_ACodecCtx->time_base) * packet->dts) * 1000.0);
		}

		if (raw->m_pts != 0)
		{	
			// update audio clock with pts, if present
			m_last_audio_timestamp = raw->m_pts;
		}
		else
		{
			raw->m_pts = m_last_audio_timestamp;
		}

		// update video clock for next frame
		boost::uint32_t frame_delay;
		if (!m_isFLV)
		{
			frame_delay = static_cast<boost::uint32_t>((as_double(m_audio_stream->time_base) * packet->dts) * 1000.0);
		}
		else
		{
			frame_delay = m_parser->audioFrameDelay();
		}

		m_last_audio_timestamp += frame_delay;

		return raw;
	}
	return 0;
}


media::raw_mediadata_t* 
NetStreamFfmpeg::decodeVideo(AVPacket* packet)
{
	if (!m_VCodecCtx) return NULL;
	if (!m_Frame) return NULL;

	int got = 0;
	avcodec_decode_video(m_VCodecCtx, m_Frame, &got, packet->data, packet->size);
	if (!got) return NULL;

	// This tmpImage is really only used to compute proper size of the video data...
	// stupid isn't it ?
	std::auto_ptr<image::image_base> tmpImage;
	if (m_videoFrameFormat == render::YUV)
	{
		tmpImage.reset( new image::yuv(m_VCodecCtx->width, m_VCodecCtx->height) );
	}
	else if (m_videoFrameFormat == render::RGB)
	{
		tmpImage.reset( new image::rgb(m_VCodecCtx->width, m_VCodecCtx->height) );
	}

	AVPicture rgbpicture;

	if (m_videoFrameFormat == render::NONE)
	{
		// NullGui?
		return NULL;

	}
	else if (m_videoFrameFormat == render::YUV && m_VCodecCtx->pix_fmt != PIX_FMT_YUV420P)
	{
		assert( 0 );	// TODO
		//img_convert((AVPicture*) pFrameYUV, PIX_FMT_YUV420P, (AVPicture*) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
		// Don't use depreceted img_convert, use sws_scale

	}
	else if (m_videoFrameFormat == render::RGB && m_VCodecCtx->pix_fmt != PIX_FMT_RGB24)
	{
		rgbpicture = media::VideoDecoderFfmpeg::convertRGB24(m_VCodecCtx, *m_Frame);
		if (!rgbpicture.data[0])
		{
			return NULL;
		}
	}

	media::raw_mediadata_t* video = new media::raw_mediadata_t();

	video->m_data = new boost::uint8_t[tmpImage->size()];
	video->m_ptr = video->m_data;
	video->m_stream_index = m_video_index;
	video->m_pts = 0;

	// set presentation timestamp
	if (packet->dts != static_cast<signed long>(AV_NOPTS_VALUE))
	{
		if (!m_isFLV)	video->m_pts = static_cast<boost::uint32_t>((as_double(m_video_stream->time_base) * packet->dts) * 1000.0);
		else video->m_pts = static_cast<boost::uint32_t>((as_double(m_VCodecCtx->time_base) * packet->dts) * 1000.0);
	}

	if (video->m_pts != 0)
	{	
		// update video clock with pts, if present
		m_last_video_timestamp = video->m_pts;
	}
	else
	{
		video->m_pts = m_last_video_timestamp;
	}

	// update video clock for next frame
	boost::uint32_t frame_delay;
	if (!m_isFLV) frame_delay = static_cast<boost::uint32_t>(as_double(m_video_stream->codec->time_base) * 1000.0);
	else frame_delay = m_parser->videoFrameDelay();

	// for MPEG2, the frame can be repeated, so we update the clock accordingly
	frame_delay += static_cast<boost::uint32_t>(m_Frame->repeat_pict * (frame_delay * 0.5) * 1000.0);

	m_last_video_timestamp += frame_delay;

	if (m_videoFrameFormat == render::YUV)
	{
		image::yuv* yuvframe = static_cast<image::yuv*>(tmpImage.get());
		unsigned int copied = 0;
		boost::uint8_t* ptr = video->m_data;
		for (int i = 0; i < 3 ; i++)
		{
			int shift = (i == 0 ? 0 : 1);
			boost::uint8_t* yuv_factor = m_Frame->data[i];
			int h = m_VCodecCtx->height >> shift;
			int w = m_VCodecCtx->width >> shift;
			for (int j = 0; j < h; j++)
			{
				copied += w;
				assert(copied <= yuvframe->size());
				memcpy(ptr, yuv_factor, w);
				yuv_factor += m_Frame->linesize[i];
				ptr += w;
			}
		}
		video->m_size = copied;
	}
	else if (m_videoFrameFormat == render::RGB)
	{
		AVPicture* src;
		if (m_VCodecCtx->pix_fmt != PIX_FMT_RGB24)
		{
			src = &rgbpicture;
		} else
		{
			src = (AVPicture*) m_Frame;
		}
	
		boost::uint8_t* srcptr = src->data[0];		  
		boost::uint8_t* srcend = srcptr + rgbpicture.linesize[0] * m_VCodecCtx->height;
		boost::uint8_t* dstptr = video->m_data;
		unsigned int srcwidth = m_VCodecCtx->width * 3;

		video->m_size = 0;

		while (srcptr < srcend) {
			memcpy(dstptr, srcptr, srcwidth);
			srcptr += src->linesize[0];
			dstptr += srcwidth;
			video->m_size += srcwidth;
		}
		
		if (m_VCodecCtx->pix_fmt != PIX_FMT_RGB24) {
			delete [] rgbpicture.data[0];
		}

	}

	return video;
}

bool NetStreamFfmpeg::decodeMediaFrame()
{
	return false;

#if 0 // Only FLV for now (non-FLV should be threated the same as FLV, using a MediaParser in place of the FLVParser)

	if (m_unqueued_data)
	{
		if (m_unqueued_data->m_stream_index == m_audio_index)
		{
			if (_soundHandler)
			{
				m_unqueued_data = m_qaudio.push(m_unqueued_data) ? NULL : m_unqueued_data;
			}
		}
		else if (m_unqueued_data->m_stream_index == m_video_index)
		{
			m_unqueued_data = m_qvideo.push(m_unqueued_data) ? NULL : m_unqueued_data;
		}
		else
		{
			log_error(_("read_frame: not audio & video stream"));
		}
		return true;
	}

  	AVPacket packet;
	
  	int rc = av_read_frame(m_FormatCtx, &packet);

	if (rc >= 0)
	{
		if (packet.stream_index == m_audio_index && _soundHandler)
		{
			media::raw_mediadata_t* audio = decodeAudio(&packet);
      			if (!audio)
			{
				log_error(_("Problems decoding audio frame"));
				return false;
			}
			m_unqueued_data = m_qaudio.push(audio) ? NULL : audio;
		}
		else
		if (packet.stream_index == m_video_index)
		{
			media::raw_mediadata_t* video = decodeVideo(&packet);
      			if (!video)
			{
				log_error(_("Problems decoding video frame"));
				return false;
			}
			m_unqueued_data = m_qvideo.push(video) ? NULL : video;
		}
		av_free_packet(&packet);
	}
	else
	{
		log_error(_("Problems decoding frame"));
		return false;
	}

	return true;
#endif
}

void
NetStreamFfmpeg::seek(boost::uint32_t posSeconds)
{
	GNASH_REPORT_FUNCTION;

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

	// We'll mess with the input here
	if ( ! m_parser.get() )
	{
		log_debug("NetStreamFfmpeg::seek(%d): no parser, no party", posSeconds);
		return;
	}

	// Don't ask me why, but NetStream::seek() takes seconds...
	boost::uint32_t pos = posSeconds*1000;

	long newpos = 0;
	double timebase = 0;

	// We'll pause the clock source and mark decoders as buffering.
	// In this way, next advance won't find the source time to 
	// be a lot of time behind and chances to get audio buffer
	// overruns will reduce.
	// ::advance will resume the playbackClock if DEC_BUFFERING...
	//
	_playbackClock->pause();
	decodingStatus(DEC_BUFFERING); 

	// Seek to new position
	if (m_isFLV)
	{
		newpos = m_parser->seek(pos);
		log_debug("m_parser->seek(%d) returned %d", pos, newpos);
	}
	else if (m_FormatCtx)
	{
		AVStream* videostream = m_FormatCtx->streams[m_video_index];
    		timebase = static_cast<double>(videostream->time_base.num / videostream->time_base.den);
		newpos = static_cast<long>(pos / timebase);
		
		if (av_seek_frame(m_FormatCtx, m_video_index, newpos, 0) < 0)
		{
			log_error(_("%s: seeking failed"), __FUNCTION__);
			return;
		}
	}
	else
	{
		// TODO: should we log_debug ??
		return;
	}

	// This is kindof hackish and ugly :-(
	if (newpos == 0)
	{
		m_last_video_timestamp = 0;
		m_last_audio_timestamp = 0;
	}
	else if (m_isFLV)
	{
		if (m_ACodecCtx) m_last_audio_timestamp = newpos;
		if (m_VCodecCtx) m_last_video_timestamp = newpos;
	}
	else
	{
    		AVPacket Packet;
    		av_init_packet(&Packet);
		double newtime = 0;
		while (newtime == 0)
		{
      			if (av_read_frame(m_FormatCtx, &Packet) < 0) 
			{
				av_seek_frame(m_FormatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
				av_free_packet( &Packet );
				return;
			}

			newtime = timebase * (double)m_FormatCtx->streams[m_video_index]->cur_dts;
		}

    		av_free_packet( &Packet );

		av_seek_frame(m_FormatCtx, m_video_index, newpos, 0);
		newpos = static_cast<boost::int32_t>(newtime / 1000.0);

		m_last_audio_timestamp = newpos;
		m_last_video_timestamp = newpos;
	}

	{ // cleanup audio queue, so won't be consumed while seeking
		boost::mutex::scoped_lock lock(_audioQueueMutex);
		for (AudioQueue::iterator i=_audioQueue.begin(), e=_audioQueue.end();
				i!=e; ++i)
		{
			delete (*i);
		}
		_audioQueue.clear();
	}
	
	// 'newpos' will always be on a keyframe (supposedly)
	_playHead.seekTo(newpos);
	_qFillerResume.notify_all(); // wake it decoder is sleeping
	
	refreshVideoFrame(true);
}

void
NetStreamFfmpeg::parseNextChunk()
{
	// TODO: parse as much as possible w/out blocking
	//       (will always block currently..)
	const int tagsPerChunk = 2;
	for (int i=0; i<tagsPerChunk; ++i)
		m_parser->parseNextTag();
}

void
NetStreamFfmpeg::refreshAudioBuffer()
{
	assert ( m_parser.get() );

#ifdef GNASH_DEBUG_DECODING
	// bufferLength() would lock the mutex (which we already hold),
	// so this is to avoid that.
	boost::uint32_t parserTime = m_parser->getBufferLength();
	boost::uint32_t playHeadTime = time();
	boost::uint32_t bufferLen = parserTime > playHeadTime ? parserTime-playHeadTime : 0;
#endif

	if ( _playHead.getState() == PlayHead::PLAY_PAUSED )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshAudioBuffer: doing nothing as playhead is paused - "
			"bufferLength=%d, bufferTime=%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	if ( _playHead.isAudioConsumed() ) 
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshAudioBuffer: doing nothing "
			"as current position was already decoded - "
			"bufferLength=%d, bufferTime=%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	// Calculate the current time
	boost::uint64_t curPos = _playHead.getPosition();

#ifdef GNASH_DEBUG_DECODING
	log_debug("%p.refreshAudioBuffer: currentPosition=%d, playHeadState=%d, bufferLength=%d, bufferTime=%d",
		this, curPos, _playHead.getState(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

	// TODO: here we should fetch all frames up to the one with timestamp >= curPos
	//       and push them into the buffer to be consumed by audio_streamer
	pushDecodedAudioFrames(curPos);
}

void
NetStreamFfmpeg::pushDecodedAudioFrames(boost::uint32_t ts)
{
	assert(m_parser.get());

	bool consumed = false;

	while ( 1 )
	{
		FLVAudioFrameInfo* info = m_parser->peekNextAudioFrameInfo();
		if ( ! info )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.pushDecodedAudioFrames(%d): "
				"no more audio frames in input "
				"(peekNextAudioFrameInfo returned false)",
				this, ts);
#endif // GNASH_DEBUG_DECODING
			consumed = true;
			decodingStatus(DEC_STOPPED);
#ifdef GNASH_DEBUG_STATUS
			log_debug("Setting playStop status");
#endif
			setStatus(playStop);
			break;
		}

		if ( info->timestamp > ts )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.pushDecodedAudioFrames(%d): "
				"next audio frame is in the future (%d)",
				this, ts, info->timestamp);
#endif // GNASH_DEBUG_DECODING
			consumed = true;
			break; // next frame is in the future
		}

		boost::mutex::scoped_lock lock(_audioQueueMutex);

		static const int bufferLimit = 20;
		if ( _audioQueue.size() > bufferLimit )
		{
			// we won't buffer more then 'bufferLimit' frames in the queue
			// to avoid ending up with a huge queue which will take some
			// time before being consumed by audio mixer, but still marked
			// as "consumed". Keeping decoded frames buffer low would also
			// reduce memory use.
			//
			// The alternative would be always decode on demand from the
			// audio consumer thread, but would introduce a lot of thread-safety
			// issues: playhead would need protection, input would need protection.
			//
//#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.pushDecodedAudioFrames(%d) : queue size over limit (%d), "
				"audio won't be consumed (buffer overrun?)",
				this, ts, bufferLimit);
//#endif // GNASH_DEBUG_DECODING
			return;
		}

		media::raw_mediadata_t* audio = decodeNextAudioFrame();
		if ( ! audio )
		{
			log_error("peekNextAudioFrameInfo returned some info, "
				"but decodeNextAudioFrame returned null, "
				"I don't think this should ever happen");
			break;
		}

#ifdef GNASH_DEBUG_DECODING
		// this one we might avoid :) -- a less intrusive logging could
		// be take note about how many things we're pushing over
		log_debug("pushDecodedAudioFrames(%d) pushing frame with timestamp %d", ts, info->timestamp); 
#endif
		_audioQueue.push_back(audio);
	}

	// If we consumed audio of current position, feel free to advance if needed
	if ( consumed ) _playHead.setAudioConsumed();
}


void
NetStreamFfmpeg::refreshVideoFrame(bool alsoIfPaused)
{
	assert ( m_parser.get() );

#ifdef GNASH_DEBUG_DECODING
	// bufferLength() would lock the mutex (which we already hold),
	// so this is to avoid that.
	boost::uint32_t parserTime = m_parser->getBufferLength();
	boost::uint32_t playHeadTime = time();
	boost::uint32_t bufferLen = parserTime > playHeadTime ? parserTime-playHeadTime : 0;
#endif

	if ( ! alsoIfPaused && _playHead.getState() == PlayHead::PLAY_PAUSED )
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshVideoFrame: doing nothing as playhead is paused - "
			"bufferLength=%d, bufferTime=%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	if ( _playHead.isVideoConsumed() ) 
	{
#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.refreshVideoFrame: doing nothing "
			"as current position was already decoded - "
			"bufferLength=%d, bufferTime=%d",
			this, bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
		return;
	}

	// Calculate the current time
	boost::uint64_t curPos = _playHead.getPosition();

#ifdef GNASH_DEBUG_DECODING
	log_debug("%p.refreshVideoFrame: currentPosition=%d, playHeadState=%d, bufferLength=%d, bufferTime=%d",
		this, curPos, _playHead.getState(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

	// Get next decoded video frame from parser, will have the lowest timestamp
    	media::raw_mediadata_t* video = getDecodedVideoFrame(curPos);

	// to be decoded or we're out of data
	if (!video)
	{
		if ( decodingStatus() == DEC_STOPPED )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.refreshVideoFrame(): "
				"no more video frames to decode, "
				"sending STOP event",
				this);
#endif // GNASH_DEBUG_DECODING
#ifdef GNASH_DEBUG_STATUS
			log_debug("Setting playStop status");
#endif
			setStatus(playStop);
		}
		else
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.refreshVideoFrame(): "
				"last video frame was good enough "
				"for current position",
				this);
#endif // GNASH_DEBUG_DECODING
			// There no video but decoder is still running
			// not much to do here except wait for next call
			//assert(decodingStatus() == DEC_BUFFERING);
		}

	}
	else
	{

		if (m_videoFrameFormat == render::YUV)
		{
			if ( ! m_imageframe ) m_imageframe  = new image::yuv(m_VCodecCtx->width, m_VCodecCtx->height);
			// XXX m_imageframe might be a byte aligned buffer, while video is not!
			static_cast<image::yuv*>(m_imageframe)->update(video->m_data);
		}
		else if (m_videoFrameFormat == render::RGB)
		{
			if ( ! m_imageframe ) m_imageframe  = new image::rgb(m_VCodecCtx->width, m_VCodecCtx->height);
			image::rgb* imgframe = static_cast<image::rgb*>(m_imageframe);
			rgbcopy(imgframe, video, m_VCodecCtx->width * 3);
		}

		// Delete the frame from the queue
		delete video;

		// A frame is ready for pickup
		m_newFrameReady = true;
	}

	// We consumed video of current position, feel free to advance if needed
	_playHead.setVideoConsumed();


}


void
NetStreamFfmpeg::advance()
{
	// Check if there are any new status messages, and if we should
	// pass them to a event handler
	processStatusNotifications();

#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	// stop parser thread while advancing
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

	// Nothing to do if we don't have a parser
	if ( ! m_parser.get() ) return;

#ifndef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	// Parse some input no matter what
	parseNextChunk(); 
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

	// bufferLength() would lock the mutex (which we already hold),
	// so this is to avoid that.
	boost::uint32_t parserTime = m_parser->getBufferLength();
	boost::uint32_t playHeadTime = time();
	boost::uint32_t bufferLen = parserTime > playHeadTime ? parserTime-playHeadTime : 0;


	// Check decoding status 
	if ( decodingStatus() == DEC_DECODING && bufferLen == 0 )
	{
		if ( ! m_parser->parsingCompleted() )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.advance: buffer empty while decoding,"
				" setting buffer to buffering and pausing playback clock",
				this);
#endif // GNASH_DEBUG_DECODING
#ifdef GNASH_DEBUG_STATUS
			log_debug("Setting bufferEmpty status");
#endif
			setStatus(bufferEmpty);
			decodingStatus(DEC_BUFFERING);
			_playbackClock->pause();
		}
		else
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.advance : bufferLength=%d, parsing completed",
 				this, bufferLen);
#endif // GNASH_DEBUG_DECODING
			// set playStop ? (will be done later for now)
		}
	}

	if ( decodingStatus() == DEC_BUFFERING )
	{
		if ( bufferLen < m_bufferTime && ! m_parser->parsingCompleted() )
		{
#ifdef GNASH_DEBUG_DECODING
			log_debug("%p.advance: buffering"
				" - position=%d, buffer=%d/%d",
				this, _playHead.getPosition(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING
			return;
		}

#ifdef GNASH_DEBUG_DECODING
		log_debug("%p.advance: buffer full (or parsing completed), resuming playback clock"
			" - position=%d, buffer=%d/%d",
			this, _playHead.getPosition(), bufferLen, m_bufferTime);
#endif // GNASH_DEBUG_DECODING

		setStatus(bufferFull);
		decodingStatus(DEC_DECODING);
		_playbackClock->resume();
	}

	// Find video frame with the most suited timestamp in the video queue,
	// and put it in the output image frame.
	refreshVideoFrame();

	// Refill audio buffer to consume all samples
	// up to current playhead
	refreshAudioBuffer();
}

boost::int32_t
NetStreamFfmpeg::time()
{
	return _playHead.getPosition();
}

void NetStreamFfmpeg::pausePlayback()
{
	GNASH_REPORT_FUNCTION;

	PlayHead::PlaybackStatus oldStatus = _playHead.setState(PlayHead::PLAY_PAUSED);

	// Disconnect the soundhandler if we were playing before
	if ( oldStatus == PlayHead::PLAY_PLAYING && _soundHandler )
	{
		_soundHandler->detach_aux_streamer((void*)this);
	}
}

void NetStreamFfmpeg::unpausePlayback()
{
	GNASH_REPORT_FUNCTION;

	PlayHead::PlaybackStatus oldStatus = _playHead.setState(PlayHead::PLAY_PLAYING);

	// Re-connect to the soundhandler if we were paused before
	if ( oldStatus == PlayHead::PLAY_PAUSED && _soundHandler )
	{
		_soundHandler->attach_aux_streamer(audio_streamer, (void*) this);
	}
}


long
NetStreamFfmpeg::bytesLoaded ()
{
  	if ( ! m_parser.get() )
	{
		log_debug("bytesLoaded: no parser, no party");
		return 0;
  	}

	return m_parser->getBytesLoaded();
}

long
NetStreamFfmpeg::bufferLength ()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

  	if ( ! m_parser.get() )
	{
		log_debug("bytesTotal: no parser, no party");
		return 0;
  	}

	boost::uint32_t maxTimeInBuffer = m_parser->getBufferLength();
	boost::uint64_t curPos = _playHead.getPosition();

	if ( maxTimeInBuffer < curPos ) return 0;
	return maxTimeInBuffer-curPos;
}

long
NetStreamFfmpeg::bytesTotal ()
{
#ifdef LOAD_MEDIA_IN_A_SEPARATE_THREAD
	boost::mutex::scoped_lock lock(_parserMutex);
#endif // LOAD_MEDIA_IN_A_SEPARATE_THREAD

  	if ( ! m_parser.get() )
	{
		log_debug("bytesTotal: no parser, no party");
		return 0;
  	}

	return m_parser->getBytesTotal();
}

NetStreamFfmpeg::DecodingState
NetStreamFfmpeg::decodingStatus(DecodingState newstate)
{
	boost::mutex::scoped_lock lock(_state_mutex);

	if (newstate != DEC_NONE) {
		_decoding_state = newstate;
	}

	return _decoding_state;
}

} // gnash namespcae

#endif // USE_FFMPEG

