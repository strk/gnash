// AudioDecoderFfmpeg.cpp: Audio decoding using the FFmpeg library.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#include "AudioDecoderFfmpeg.h"

#include <cmath> // for std::ceil
#include <algorithm> // for std::copy, std::max

#include "MediaParserFfmpeg.h" // for ExtraAudioInfoFfmpeg
#include "FLVParser.h"
#include "SoundInfo.h"
#include "MediaParser.h" // for AudioInfo

//#define GNASH_DEBUG_AUDIO_DECODING

namespace gnash {
namespace media {
namespace ffmpeg {
    
AudioDecoderFfmpeg::AudioDecoderFfmpeg(const AudioInfo& info)
    :
    _audioCodec(nullptr),
    _audioCodecCtx(nullptr),
    _parser(nullptr),
    _needsParsing(false)
{
    setup(info);

    if (info.type == CODEC_TYPE_CUSTOM) {
        log_debug(_("AudioDecoderFfmpeg: initialized FFmpeg codec %d (%s)"),
            _audioCodec->id, _audioCodec->name);
    } else {
        log_debug(_("AudioDecoderFfmpeg: initialized FFmpeg codec %d (%s) "
                    "for flash codec %d (%s)"),
            _audioCodec->id, _audioCodec->name,
            info.codec, (audioCodecType)info.codec);
    }
}

AudioDecoderFfmpeg::AudioDecoderFfmpeg(SoundInfo& info)
    :
    _audioCodec(nullptr),
    _audioCodecCtx(nullptr),
    _parser(nullptr)
{
    setup(info);

      log_debug(_("AudioDecoderFfmpeg: initialized FFmpeg codec %s (%d)"),
        _audioCodec->name, _audioCodec->id);
}

AudioDecoderFfmpeg::~AudioDecoderFfmpeg()
{
    if (_audioCodecCtx)
    {
        avcodec_close(_audioCodecCtx);
        av_free(_audioCodecCtx);
    }
    if (_parser) av_parser_close(_parser);
}

void AudioDecoderFfmpeg::setup(SoundInfo& info)
{
    avcodec_register_all();// change this to only register need codec?

    enum CODECID codec_id;

    switch(info.getFormat()) {
        case AUDIO_CODEC_RAW:
            codec_id = AV_CODEC_ID_PCM_U16LE;
            break;
        case AUDIO_CODEC_ADPCM:
            codec_id = AV_CODEC_ID_ADPCM_SWF;
            break;
        case AUDIO_CODEC_MP3:
            codec_id = AV_CODEC_ID_MP3;
            _needsParsing=true;
            break;
        case AUDIO_CODEC_AAC:
            codec_id = AV_CODEC_ID_AAC;
            _needsParsing=true;
            break;
        default:
            boost::format err = boost::format(
                _("Unsupported audio codec %d")) %
                static_cast<int>(info.getFormat());
            throw MediaException(err.str());
    }

    _audioCodec = avcodec_find_decoder(codec_id);
    if (!_audioCodec) {
        audioCodecType codec = info.getFormat();
        boost::format err = boost::format(
            _("libavcodec could not find a decoder for codec %d (%s)")) %
            static_cast<int>(codec) % codec;
        throw MediaException(err.str());
    }

    if ( _needsParsing )
    {
        // Init the parser
        _parser = av_parser_init(codec_id);
        if (!_parser) {    
        throw MediaException(_("AudioDecoderFfmpeg can't initialize "
                    "MP3 parser"));
        }
    }

    _audioCodecCtx = avcodec_alloc_context3(_audioCodec);
    if (!_audioCodecCtx) {
        throw MediaException(_("libavcodec couldn't allocate context"));
    }

    int ret = avcodec_open2(_audioCodecCtx, _audioCodec, nullptr);
    if (ret < 0) {
        av_free(_audioCodecCtx);
        _audioCodecCtx=nullptr;
        boost::format err = boost::format(
            _("AudioDecoderFfmpeg: avcodec_open failed to initialize "
            "FFmpeg codec %s (%d)")) % _audioCodec->name % (int)codec_id;
        throw MediaException(err.str());
    }

    log_debug(_("AudioDecoder: initialized FFMPEG codec %s (%d)"), 
                _audioCodec->name, (int)codec_id);

    /// @todo do this only if !_needsParsing ?
    switch (_audioCodecCtx->codec->id)
    {
            case AV_CODEC_ID_MP3:
                break;

            case AV_CODEC_ID_PCM_U16LE:
                _audioCodecCtx->channels = (info.isStereo() ? 2 : 1);
                _audioCodecCtx->sample_rate = info.getSampleRate();
                _audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16; // ?! arbitrary ?
                _audioCodecCtx->frame_size = 1; 
                break;

            default:
                _audioCodecCtx->channels = (info.isStereo() ? 2 : 1);
                _audioCodecCtx->sample_rate = info.getSampleRate();
                _audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16; // ?! arbitrary ?
                break;
    }
}

void AudioDecoderFfmpeg::setup(const AudioInfo& info)
{
    // Init the avdecoder-decoder
    avcodec_register_all();// change this to only register need codec?

    enum CODECID codec_id = AV_CODEC_ID_NONE;

    if (info.type == CODEC_TYPE_CUSTOM)
    {
        codec_id = static_cast<CODECID>(info.codec);
    }
    else if (info.type == CODEC_TYPE_FLASH)
    {

        switch(info.codec)
        {
            case AUDIO_CODEC_UNCOMPRESSED:
            case AUDIO_CODEC_RAW:
                if (info.sampleSize == 2) {
                    codec_id = AV_CODEC_ID_PCM_S16LE;
                } else {
                    codec_id = AV_CODEC_ID_PCM_S8;
                }
                break;

            case AUDIO_CODEC_ADPCM:
                codec_id = AV_CODEC_ID_ADPCM_SWF;
                break;

            case AUDIO_CODEC_MP3:
                codec_id = AV_CODEC_ID_MP3;
                break;

            case AUDIO_CODEC_AAC:
                codec_id = AV_CODEC_ID_AAC;
                break;

#ifdef FFMPEG_NELLYMOSER
            // NOTE: bjacques found this failing in decodeFrame
            //       (but probably not Ffmpeg's fault, he said)
            //       I'd like to take a look at the testcase --strk
            case AUDIO_CODEC_NELLYMOSER:
                codec_id = AV_CODEC_ID_NELLYMOSER;
                break;
#endif

            default:
                boost::format err = boost::format(
                    _("AudioDecoderFfmpeg: unsupported flash audio "
                        "codec %d (%s)")) %
                        info.codec % (audioCodecType)info.codec;
                throw MediaException(err.str());
        }
    }
    else
    {
        boost::format err = boost::format(
            _("AudioDecoderFfmpeg: unknown codec type %d "
            "(should never happen)")) % info.type;
        throw MediaException(err.str());
    }

    _audioCodec = avcodec_find_decoder(codec_id);
    if (!_audioCodec)
    {
        if (info.type == CODEC_TYPE_FLASH) {
            boost::format err = boost::format(
                _("AudioDecoderFfmpeg: libavcodec could not find a decoder "
                    "for codec %d (%s)")) %
                    info.codec % static_cast<audioCodecType>(info.codec);
            throw MediaException(err.str());
        } else {
            boost::format err = boost::format(
                _("AudioDecoderFfmpeg: libavcodec could not find a decoder "
                    "for ffmpeg codec id %s")) % codec_id;
            throw MediaException(err.str());
        }
    }

    _parser = av_parser_init(codec_id);
    _needsParsing = (_parser != nullptr);

    // Create an audioCodecCtx from the ffmpeg parser if exists/possible
    _audioCodecCtx = avcodec_alloc_context3(_audioCodec);
    if (!_audioCodecCtx) {
        throw MediaException(_("AudioDecoderFfmpeg: libavcodec couldn't "
                    "allocate context"));
    }

    if ( info.extra.get() )
    {
        if (dynamic_cast<ExtraAudioInfoFfmpeg*>(info.extra.get())) {
            const ExtraAudioInfoFfmpeg& ei = 
                static_cast<ExtraAudioInfoFfmpeg&>(*info.extra);
            _audioCodecCtx->extradata = ei.data;
            _audioCodecCtx->extradata_size = ei.dataSize;
        } else if (dynamic_cast<ExtraAudioInfoFlv*>(info.extra.get())) {
            ExtraAudioInfoFlv* extra = 
                static_cast<ExtraAudioInfoFlv*>(info.extra.get());
            _audioCodecCtx->extradata = extra->data.get();
            _audioCodecCtx->extradata_size = extra->size;
        }
    }

    // Setup known configurations for the audio codec context
    // NOTE: this is done before calling avcodec_open, as that might update
    //       some of the variables
    switch (codec_id)
    {
            case AV_CODEC_ID_MP3:
                break;

            case AV_CODEC_ID_PCM_S8:
                // Either FFMPEG or the parser are getting this wrong.
                _audioCodecCtx->sample_rate = info.sampleRate / 2;
                _audioCodecCtx->channels = (info.stereo ? 2 : 1);
                break;
            case AV_CODEC_ID_PCM_S16LE:
                _audioCodecCtx->channels = (info.stereo ? 2 : 1);
                _audioCodecCtx->sample_rate = info.sampleRate;
                break;

            default:
                _audioCodecCtx->channels = (info.stereo ? 2 : 1);
                _audioCodecCtx->sample_rate = info.sampleRate;
                // was commented out (why?):
                _audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
                break;
    }


#ifdef GNASH_DEBUG_AUDIO_DECODING
    log_debug("  Opening codec");
#endif // GNASH_DEBUG_AUDIO_DECODING
    int ret = avcodec_open2(_audioCodecCtx, _audioCodec, nullptr);
    if (ret < 0) {
        //avcodec_close(_audioCodecCtx);
        av_free(_audioCodecCtx);
        _audioCodecCtx = nullptr;

        boost::format err = boost::format(
            _("AudioDecoderFfmpeg: avcodec_open failed to initialize "
            "FFmpeg codec %s (%d)")) % _audioCodec->name % (int)codec_id;
        throw MediaException(err.str());
    }

}

std::uint8_t*
AudioDecoderFfmpeg::decode(const std::uint8_t* input,
        std::uint32_t inputSize, std::uint32_t&
        outputSize, std::uint32_t& decodedBytes)
{
    //GNASH_REPORT_FUNCTION;

    size_t retCapacity = MAX_AUDIO_FRAME_SIZE;
    std::uint8_t* retBuf = new std::uint8_t[retCapacity];
    int retBufSize = 0;

#ifdef GNASH_DEBUG_AUDIO_DECODING
    log_debug("  Parsing loop starts, input is %d bytes, retCapacity is %d "
            "bytes", inputSize, retCapacity);
#endif
    decodedBytes = 0; // nothing decoded yet
    while (decodedBytes < inputSize)
    {
        const std::uint8_t* frame=nullptr; // parsed frame (pointer into input)
        int framesize; // parsed frame size

        int consumed = parseInput(input+decodedBytes,
                             inputSize-decodedBytes,
                             &frame, &framesize);
        if (consumed < 0)
        {
            log_error(_("av_parser_parse returned %d. "
                "Upgrading ffmpeg/libavcodec might fix this issue."),
                consumed);
            // Setting data position to data size will get the sound removed
            // from the active sound list later on.
            decodedBytes = inputSize;
            break;
        }

#ifdef GNASH_DEBUG_AUDIO_DECODING
        log_debug("   parsed frame is %d bytes (consumed +%d = %d/%d)",
                framesize, consumed, decodedBytes+consumed, inputSize);
#endif

#if GNASH_PARANOIA_LEVEL > 1
        if ( frame )
        {
            // the returned frame pointer is inside the input buffer
            assert(frame == input+decodedBytes);
            // the returned frame size is within the input size
            assert(framesize <= inputSize);
        }
#endif

        // all good so far, keep going..
        // (we might do this immediately, as we'll override decodedBytes
        // on error anyway)
        decodedBytes += consumed;

        if ( ! framesize )
        {
            // See https://savannah.gnu.org/bugs/?25456
            // for a live testcase
            log_debug("AudioDecoderFfmpeg: "
                      "could not find a complete frame in "
                      "the last %d bytes of a %d bytes block"
                        " (nothing should be lost)",
                      consumed, inputSize);
            break; 
        }


        // Now, decode the frame. We use the ::decodeFrame specialized function
        // here so resampling is done appropriately
        std::uint32_t outSize = 0;
        std::unique_ptr<std::uint8_t[]> outBuf(
                decodeFrame(frame, framesize, outSize));

        if (!outBuf)
        {
            // Setting data position to data size will get the sound removed
            // from the active sound list later on.
            decodedBytes = inputSize;
            break;
        }

#ifdef GNASH_DEBUG_AUDIO_DECODING
        log_debug("   decoded frame is %d bytes, would grow return "
                "buffer size to %d bytes", outSize,
                retBufSize+static_cast<unsigned int>(outSize));
#endif 

        //
        // Now append this data to the buffer we're going to return
        //

        // if the new data doesn't fit, reallocate the output
        // TODO: can use the Buffer class here.. if we return it too...
        if ( retBufSize+(unsigned)outSize > retCapacity )
        {
#ifdef GNASH_DEBUG_AUDIO_DECODING
            log_debug("    output buffer won't be able to hold %d bytes, "
                    "capacity is only %d bytes",
                    retBufSize+(unsigned)outSize, retCapacity);
#endif 

            std::uint8_t* tmp = retBuf;
            retCapacity = std::max(retBufSize+static_cast<size_t>(outSize),
                    retCapacity * 2);

#ifdef GNASH_DEBUG_AUDIO_DECODING
            log_debug("    reallocating it to hold up to %d bytes",
                    retCapacity);
#endif // GNASH_DEBUG_AUDIO_DECODING

            retBuf = new std::uint8_t[retCapacity];
            if ( retBufSize ) std::copy(tmp, tmp+retBufSize, retBuf);
            delete [] tmp;
        }
        std::copy(outBuf.get(), outBuf.get()+outSize, retBuf+retBufSize);
        retBufSize += static_cast<unsigned int>(outSize);
    }

    
    outputSize = retBufSize;
    return retBuf;

}

std::uint8_t*
AudioDecoderFfmpeg::decode(const EncodedAudioFrame& ef,
        std::uint32_t& outputSize)
{
    return decodeFrame(ef.data.get(), ef.dataSize, outputSize);
}

std::uint8_t*
AudioDecoderFfmpeg::decodeFrame(const std::uint8_t* input,
        std::uint32_t inputSize, std::uint32_t& outputSize)
{
    assert(inputSize);

    outputSize = 0;

#ifdef GNASH_DEBUG_AUDIO_DECODING
    log_debug("AudioDecoderFfmpeg: about to decode %d bytes; "
        "ctx->channels:%d, ctx->frame_size:%d",
        inputSize, _audioCodecCtx->channels, _audioCodecCtx->frame_size);
#endif

    // older ffmpeg versions didn't accept a const input..
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = const_cast<uint8_t*>(input);
    pkt.size = inputSize;
    std::unique_ptr<AVFrame, decltype(av_free)*> frm ( FRAMEALLOC(), av_free );
    if (!frm.get()) {
        log_error(_("failed to allocate frame."));
        return nullptr;
    }

    int got_frm = 0;
    int bytesRead = avcodec_decode_audio4(_audioCodecCtx, frm.get(), &got_frm, &pkt);

    // FIXME: "[Some] decoders ... decode the first frame and the return value
    // would be less than the packet size. In this case, avcodec_decode_audio4
    // has to be called again with an AVPacket containing the remaining data in
    // order to decode the second frame, etc... Even if no frames are returned,
    // the packet needs to be fed to the decoder with remaining data until it
    // is completely consumed or an error occurs."

    if (bytesRead < 0) {
        log_error(_("avcodec_decode_audio4() returned error code %d."), bytesRead);
        return nullptr;
    }
    if (!got_frm) {
        // FIXME: "Note that this field being set to zero does not mean that an
        // error has occurred. For decoders with CODEC_CAP_DELAY set, no given
        // decode call is guaranteed to produce a frame."
        log_error(_("avcodec_decode_audio4() didn't produce a frame."));
        return nullptr;
    }

#ifdef GNASH_DEBUG_AUDIO_DECODING
    const char* fmtname = av_get_sample_fmt_name(_audioCodecCtx->sample_fmt);
    log_debug(" decodeFrame | frm->nb_samples: %d | &got_frm: %d | "
        "returned %d | inputSize: %d",
        frm->nb_samples, got_frm, tmp, inputSize);
#endif
    bool resamplingNeeded = _resampler.init(_audioCodecCtx);

    int plane_size = 0;
    int frameSize = av_samples_get_buffer_size(&plane_size, 2 /* channels */,
        frm->nb_samples, AV_SAMPLE_FMT_S16, 0 /* default alignment */);

    // TODO: make this a private member, to reuse (see NetStreamFfmpeg in 0.8.3)
    std::unique_ptr<uint8_t[]> output(new std::uint8_t[frameSize]);
    outputSize = frameSize;
    if (!resamplingNeeded) {
        memcpy(output.get(), frm->extended_data[0], frameSize);
    } else {
        std::uint8_t* outPtr = output.get();
        int outSamples = _resampler.resample(frm->extended_data, // input
            plane_size, // input
            frm->nb_samples, // input
            &outPtr);
    }

    return output.release();
}

int
AudioDecoderFfmpeg::parseInput(const std::uint8_t* input,
        std::uint32_t inputSize,
        std::uint8_t const ** outFrame, int* outFrameSize)
{
    if ( _needsParsing )
    {
        return av_parser_parse2(_parser, _audioCodecCtx,
                    // as of 2008-10-28 SVN, ffmpeg doesn't
                    // accept a pointer to pointer to const..
                    const_cast<std::uint8_t**>(outFrame),
                    outFrameSize,
                    input, inputSize,
                    0, 0, AV_NOPTS_VALUE); // pts, dts, pos
    }
    else
    {
        // democratic value for a chunk to decode...
        // @todo this might be constrained by codec id, check that !

        // NOTE: AVCODEC_MAX_AUDIO_FRAME_SIZE (192000, deprecated replaced with
        //       MAX_AUDIO_FRAME_SIZE) resulted bigger
        //       than avcodec_decode_audio could handle, resulting
        //       in eventSoundTest1.swf regression.
        //static const unsigned int maxFrameSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;

        // NOTE: 1024 resulted too few
        //       to properly decode (or resample?) raw audio
        //       thus resulting noisy (bugs #21177 and #22284)
        //static const unsigned int maxFrameSize = 1024;

        // NOTE: 96000 was found to be the max returned
        //       by avcodec_decode_audio when passed anything
        //       bigger than that. Works fine with all of
        //       eventSoundTest1.swf, bug #21177 and bug #22284
        //
        static const unsigned int maxFrameSize = 96000;

        int frameSize = inputSize < maxFrameSize ? inputSize : maxFrameSize;

        // we assume the input is just a set of frames
        // and we'll consume all
        *outFrame = input; // frame always start on input
        *outFrameSize = frameSize;
        int parsed = frameSize;
        return parsed;
    }
}


} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // gnash namespace
