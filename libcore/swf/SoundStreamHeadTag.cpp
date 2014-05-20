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

#include "SoundStreamHeadTag.h"

#include <cstdint>
#include <memory>

#include "SWF.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "RunResources.h"
#include "sound_handler.h"
#include "GnashAlgorithm.h"
#include "SoundInfo.h"
#include "utility.h"

namespace gnash {
namespace SWF {

// Load a SoundStreamHead(2) tag.
void
SoundStreamHeadTag::loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
#ifdef USE_SOUND
    
    // 18 || 45
    assert(tag == SWF::SOUNDSTREAMHEAD || tag == SWF::SOUNDSTREAMHEAD2);

    sound::sound_handler* handler = r.soundHandler();

    // If we don't have a sound_handler registered stop here
    if (!handler) return;

    // 1 byte for playback info, 1 for SWFStream info, 2 for sample count
    in.ensureBytes(4);

    // These are all unused by current implementation
    int reserved = in.read_uint(4); UNUSED(reserved);

    const std::uint32_t samplerates[] = { 5512, 11025, 22050, 44100 };

    std::uint8_t pbSoundRate = in.read_uint(2);
    if (pbSoundRate >= arraySize(samplerates)) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("SOUNDSTREAMHEAD: playback sound rate %d (expected "
                "0 to %d)", +pbSoundRate, arraySize(samplerates));
        );
        pbSoundRate = 0;
    }
    const std::uint32_t playbackSoundRate = samplerates[pbSoundRate];
    const bool playbackSound16bit = in.read_bit();
    const bool playbackSoundStereo = in.read_bit();

    // These are the used ones
    media::audioCodecType format =
        static_cast<media::audioCodecType>(in.read_uint(4)); // TODO: check input !
    std::uint8_t stSoundRate = in.read_uint(2);

    if (stSoundRate >= arraySize(samplerates)) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("SOUNDSTREAMHEAD: stream sample rate %d (expected "
                "0 to %u)"), +stSoundRate, arraySize(samplerates));
        );
        stSoundRate=0;
    }
    const std::uint32_t streamSoundRate = samplerates[stSoundRate];
    const bool streamSound16bit = in.read_bit(); 
    const bool streamSoundStereo = in.read_bit(); 

    if (playbackSoundRate != streamSoundRate) {
        LOG_ONCE(log_unimpl(_("Different stream/playback sound rate (%d/%d). "
                "This seems common in SWF files, so we'll warn only once."),
                streamSoundRate, playbackSoundRate)
        );
    }

    if (playbackSound16bit != streamSound16bit) {
        LOG_ONCE(log_unimpl(_("Different stream/playback sample size (%d/%d). "
            "This seems common in SWF files, so we'll warn only once."),
            streamSound16bit ? 16 : 32, playbackSound16bit ? 16 : 32 )
        );
    }
    if (playbackSoundStereo != streamSoundStereo) {
        LOG_ONCE(log_unimpl(_("Different stream/playback channels (%s/%s). "
            "This seems common in SWF files, so we'll warn only once."),
            streamSoundStereo ? "stereo" : "mono",
            playbackSoundStereo ? "stereo" : "mono")
        );
    }

    // checks if this is a new streams header or just one in the row
    if (format == 0 && streamSoundRate == 0 &&
            !streamSound16bit && !streamSoundStereo) {
        return;
    }

    // 2 bytes here
    const std::uint16_t sampleCount = in.read_u16();

    if (!sampleCount) {
        // this seems common too, we'd need to reproduce with a custom
        // testcase to really tell if it's a problem or not...
        IF_VERBOSE_MALFORMED_SWF(
            LOG_ONCE(log_swferror(_("No samples advertised for sound stream, "
                        "pretty common so will warn only once")));
        );
    }

    std::int16_t latency = 0;
    if (format == media::AUDIO_CODEC_MP3) {
        try {
            in.ensureBytes(2);
            latency = in.read_s16(); 
        }
        catch (const ParserException&) {
            // See https://savannah.gnu.org/bugs/?21729 for an example 
            // triggering this.
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror("MP3 sound stream lacks a 'latency' field");
            );
        }
    }

    // Check if we did read everything in this tag...
    unsigned long curPos = in.tell(), endTag=in.get_tag_end_position();
    if (curPos < endTag) {
        log_unimpl("SOUNDSTREAMHEAD contains %d unparsed bytes", endTag-curPos);
    }

    IF_VERBOSE_PARSE(
        log_parse(_("sound stream head: format=%s, rate=%d, 16=%d, "
            "stereo=%d, ct=%d, latency=%d"), format, streamSoundRate,
            +streamSound16bit, +streamSoundStereo, +sampleCount, +latency);
    );

    // Store all the data in a SoundInfo object
    const media::SoundInfo sinfo(format, streamSoundStereo,
                streamSoundRate, sampleCount, streamSound16bit, latency);

    // Stores the sounddata in the soundhandler, and the ID returned
    // can be used to starting, stopping and deleting that sound
    const int handler_id = handler->createStreamingSound(sinfo);

    m.set_loading_sound_stream_id(handler_id);
#endif  // USE_SOUND
}

} // namespace SWF
} // namespace gnash
