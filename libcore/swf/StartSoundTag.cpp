// StartSoundTag.cpp:  for Gnash.
//
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "StartSoundTag.h"
#include "SWFStream.h"
#include "movie_root.h"
#include "movie_definition.h"
#include "log.h" // for log_parse
#include "sound_definition.h" // for sound_sample
#include "RunResources.h"
#include "SoundInfoRecord.h"
#include "MovieClip.h"

#include <limits>

namespace gnash {
namespace SWF {


void
StartSoundTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& r)
{
    assert(tag == STARTSOUND); 

    sound::sound_handler* handler = r.soundHandler();

    in.ensureBytes(2); // sound_id

    int sound_id = in.read_u16();

    sound_sample* sam = m.get_sound_sample(sound_id);
    if (!sam) {
        // invalid id... nothing to do
        IF_VERBOSE_MALFORMED_SWF(
            // if there's no sound_handler we might have simply skipped
            // the definition of sound sample...
            if (handler) {
              log_swferror(_("start_sound_loader: sound_id %d is not "
                      "defined"), sound_id);
            }
        );
        return;
    }

    IF_VERBOSE_PARSE(
         log_parse(_("StartSound: id=%d"), sound_id);
    );

    // NOTE: sound_id is the SWF-defined id,
    //       sam->m_sound_handler_id is the sound_handler-provided id
    
    in.align(); // necessary?

    boost::intrusive_ptr<ControlTag> sst(
        new StartSoundTag(in, sam->m_sound_handler_id));

    m.addControlTag(sst);
}

void
StartSoundTag::executeActions(MovieClip* m, DisplayList& /* dlist */) const
{

    sound::sound_handler* handler = 
        getRunResources(*getObject(m)).soundHandler();

    if (handler) {
        if (_soundInfo.stopPlayback) {
            //log_debug("Execute StartSoundTag with 'stop playback' flag on");
            handler->stopEventSound(m_handler_id);
        }
        else {

            const sound::SoundEnvelopes* env = 
                _soundInfo.envelopes.empty() ? 0 : &_soundInfo.envelopes;

            handler->startSound(m_handler_id,
                    _soundInfo.loopCount,
                    env, // envelopes
                    !_soundInfo.noMultiple, // allow multiple instances ?
                    _soundInfo.inPoint,
                    _soundInfo.outPoint
                );
        }
    }
}

void
StartSound2Tag::loader(SWFStream& in, TagType tag, movie_definition& /*m*/,
        const RunResources& /*r*/)
{
    assert(tag == STARTSOUND2);

    std::string className;
    in.read_string(className);
    log_unimpl(_("STARTSOUND2 tag not parsed and not used"));

    // We should probably then use StartSoundTag to parse the
    // tag, but we don't know which sound_handler_id to use.

    IF_VERBOSE_PARSE(
        log_parse("StartSound2 tag: SoundClassName %s", className);
    );
    in.skip_to_tag_end();
}


} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
