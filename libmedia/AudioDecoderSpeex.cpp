//   Copyright (C) 2008 Free Software Foundation, Inc.
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
#include "log.h"

#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>

namespace gnash {
namespace media {

AudioDecoderSpeex::AudioDecoderSpeex()
    : _speex_dec_state(speex_decoder_init(&speex_wb_mode)) 
{
    if (!_speex_dec_state) {
        throw MediaException(_("AudioDecoderSpeex: initialization failed."));
    }

    speex_bits_init(&_speex_bits);

    speex_decoder_ctl(_speex_dec_state, SPEEX_GET_FRAME_SIZE, &_speex_framesize);
}
AudioDecoderSpeex::~AudioDecoderSpeex()
{
    speex_bits_destroy(&_speex_bits);

    speex_decoder_destroy(_speex_dec_state);
}

struct DecodedFrame : boost::noncopyable
{
    DecodedFrame(boost::int16_t* newdata, size_t datasize)
    : data(newdata),
      size(datasize)
    {}

    boost::scoped_array<boost::int16_t> data;
    size_t size;
};

boost::uint8_t*
AudioDecoderSpeex::decode(const EncodedAudioFrame& input,
    boost::uint32_t& outputSize)
{
    speex_bits_read_from(&_speex_bits, reinterpret_cast<char*>(input.data.get()),
                         input.dataSize);

    std::vector<DecodedFrame*> decoded_frames;

    boost::uint32_t total_size = 0;

    while (speex_bits_remaining(&_speex_bits)) {

        boost::scoped_array<short> output( new short[_speex_framesize] );

        int rv = speex_decode_int(_speex_dec_state, &_speex_bits, output.get());
        if (rv != 0) {
            if (rv != -1) {
                log_error(_("Corrupt Speex stream!"));
            }

            break;
        }

        int conv_size = 0;
        boost::int16_t* conv_data = 0;

        Util::convert_raw_data(&conv_data, &conv_size, output.get(), 
            _speex_framesize /* sample count*/, 2 /* sample size */,
            16000, false /* stereo */, 44100 /* new rate */,
            true /* convert to stereo */);

        total_size += conv_size;

        decoded_frames.push_back(new DecodedFrame(conv_data, conv_size));
    }

    outputSize = total_size;

    // We have to jump through hoops because decode() requires as much
    // data to be returned as possible.
    boost::uint8_t* rv = new boost::uint8_t[total_size];
    boost::uint8_t* ptr = rv;

    for (std::vector<DecodedFrame*>::iterator it = decoded_frames.begin(),
         end = decoded_frames.end(); it != end; ++it) {
        DecodedFrame* frame = *it;

        memcpy(ptr, frame->data.get(), frame->size);

        ptr += frame->size;

        delete frame;
    }

    outputSize = total_size;

    return rv;
}

} // gnash.media namespace 
} // gnash namespace
