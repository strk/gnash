//
//   Copyright (C) 2008, 2009, 2010, 2011. 2012 Free Software Foundation, Inc.
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

#include "AudioDecoderSpeex.h"
#include "AudioResampler.h"
#include "GnashException.h" // for MediaException
#include "MediaParser.h" // for EncodedAudioFrame
#include "log.h"

#include <functional>
#include <boost/checked_delete.hpp>
#include <boost/utility.hpp> // noncopyable
#include <cstdint> // For C99 int types

#ifdef RESAMPLING_SPEEX
# include <boost/rational.hpp>
#endif


namespace gnash {
namespace media {

AudioDecoderSpeex::AudioDecoderSpeex()
    : _speex_dec_state(speex_decoder_init(&speex_wb_mode)) 
{
    if (!_speex_dec_state) {
        throw MediaException(_("AudioDecoderSpeex: state initialization failed."));
    }

    speex_bits_init(&_speex_bits);

    speex_decoder_ctl(_speex_dec_state, SPEEX_GET_FRAME_SIZE, &_speex_framesize);

#ifdef RESAMPLING_SPEEX
    int err = 0;
    _resampler = speex_resampler_init(1, 16000, 44100,
        SPEEX_RESAMPLER_QUALITY_DEFAULT, &err);

    if (err != RESAMPLER_ERR_SUCCESS) {
        throw MediaException(_("AudioDecoderSpeex: initialization failed."));
    }

    spx_uint32_t num = 0, den = 0;

    speex_resampler_get_ratio (_resampler, &num, &den);
    assert(num && den);

    boost::rational<std::uint32_t> numsamples(den, num);

    numsamples *= _speex_framesize * 2 /* convert to stereo */;

    _target_frame_size = boost::rational_cast<std::uint32_t>(numsamples);
#endif
}
AudioDecoderSpeex::~AudioDecoderSpeex()
{
    speex_bits_destroy(&_speex_bits);

    speex_decoder_destroy(_speex_dec_state);

#ifdef RESAMPLING_SPEEX
    speex_resampler_destroy(_resampler);
#endif
}

struct DecodedFrame : boost::noncopyable
{
    DecodedFrame(std::int16_t* newdata, size_t datasize)
    : data(newdata),
      size(datasize)
    {}

    std::unique_ptr<std::int16_t[]> data;
    size_t size;
};

std::uint8_t*
AudioDecoderSpeex::decode(const EncodedAudioFrame& input,
    std::uint32_t& outputSize)
{
    speex_bits_read_from(&_speex_bits, reinterpret_cast<char*>(input.data.get()),
                         input.dataSize);

    std::vector<DecodedFrame*> decoded_frames;

    std::uint32_t total_size = 0;

    while (speex_bits_remaining(&_speex_bits)) {

        std::unique_ptr<short[]> output( new short[_speex_framesize] );

        int rv = speex_decode_int(_speex_dec_state, &_speex_bits, output.get());
        if (rv != 0) {
            if (rv != -1) {
                log_error(_("Corrupt Speex stream!"));
            }

            break;
        }

        
        std::int16_t* conv_data = nullptr;

#ifdef RESAMPLING_SPEEX
		spx_uint32_t conv_size = 0;
        conv_data = new std::int16_t[_target_frame_size];
        memset(conv_data, 0, _target_frame_size * 2);

        spx_uint32_t in_size = _speex_framesize;

        // Our input format is mono and we want to expand to stereo. Speex
        // won't do this for us, but we can ask it to skip a sample after
        // writing one, so all we have to do is duplicate the samples.
        speex_resampler_set_output_stride(_resampler, 2);
        conv_size = _target_frame_size; // Assuming this hould be samples.

        int err = speex_resampler_process_int(_resampler, 0 /* mono */, output.get(), &in_size, conv_data, &conv_size);
        if (err != RESAMPLER_ERR_SUCCESS) {
            log_error(_("Failed to resample Speex frame."));
            delete [] conv_data;
            continue;
        }

        // The returned size is the number of *mono* samples returned.
        conv_size *= 2;

        // Now, duplicate all the samples so we get a stereo sound.
        for (std::uint32_t i = 0; i < conv_size; i += 2) {
            conv_data[i+1] = conv_data[i];
        }

        // Our interface requires returning the audio size in bytes.
        conv_size *= sizeof(std::int16_t);
#else
        int outsize = 0;
        AudioResampler::convert_raw_data(&conv_data, &outsize, output.get(), 
            _speex_framesize /* sample count*/, 2 /* sample size */,
            16000, false /* stereo */, 44100 /* new rate */,
            true /* convert to stereo */);
        std::uint32_t conv_size = outsize;
#endif
        total_size += conv_size;

        decoded_frames.push_back(new DecodedFrame(conv_data, conv_size));
    }

    outputSize = total_size;

    // We have to jump through hoops because decode() requires as much
    // data to be returned as possible.
    std::uint8_t* rv = new std::uint8_t[total_size];
    std::uint8_t* ptr = rv;

    for (DecodedFrame* frame : decoded_frames) {

        memcpy(ptr, frame->data.get(), frame->size);

        ptr += frame->size;

        delete frame;
    }

    outputSize = total_size;

    return rv;
}

} // gnash.media namespace 
} // gnash namespace
