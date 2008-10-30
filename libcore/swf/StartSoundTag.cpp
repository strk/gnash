// StartSoundTag.cpp:  for Gnash.
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

#include "StartSoundTag.h"
#include "SWFStream.h"
#include "movie_definition.h"
#include "log.h" // for log_parse
#include "sound_definition.h" // for sound_sample
#include "RunInfo.h"
#include "MovieClip.h"

namespace gnash {
namespace SWF {

/* public static */
void
StartSoundTag::loader(SWFStream& in, tag_type tag, movie_definition& m,
        const RunInfo& r)
{
    assert(tag == SWF::STARTSOUND); // 15 

    sound::sound_handler* handler = r.soundHandler();

    in.ensureBytes(2); // sound_id

    int sound_id = in.read_u16();

    sound_sample* sam = m.get_sound_sample(sound_id);
    if ( ! sam ) // invalid id... nothing to do
    {
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

    // NOTE: sound_id is the SWF-defined id,
    //       sam->m_sound_handler_id is the sound_handler-provided id
    //
    std::auto_ptr<StartSoundTag> sst(
            new StartSoundTag(sam->m_sound_handler_id));
    sst->read(in);

    IF_VERBOSE_PARSE (
         log_parse(_("StartSound: id=%d, stop = %d, loop ct = %d"),
              sound_id, int(sst->m_stop_playback), sst->m_loop_count);
    );

    m.addControlTag(static_cast<std::auto_ptr<ControlTag> >(sst));
}

/* private */
void
StartSoundTag::read(SWFStream& in)
{
    in.align();
    in.ensureBytes(1); // header

    int flags = in.read_u8();
    // first two bits are reserved.
    m_stop_playback     = flags & (1 << 5); 
    bool  no_multiple   = flags & (1 << 4); 
    bool  has_envelope  = flags & (1 << 3); 
    bool  has_loops     = flags & (1 << 2); 
    bool  has_out_point = flags & (1 << 1); 
    bool  has_in_point  = flags & (1 << 0); 

    if (no_multiple)
    {
        LOG_ONCE( log_unimpl("syncNoMultiple flag in StartSoundTag") );
    }

    boost::uint32_t in_point = 0;
    boost::uint32_t out_point = 0;

    if ( in_point ) log_unimpl(_("StartSoundTag with in point"));
    if ( out_point ) log_unimpl(_("StartSoundTag with out point"));

    in.ensureBytes(has_in_point*4 + has_out_point*4 + has_loops*2);

    if (has_in_point)
    {
        in_point = in.read_u32();
    }
    if (has_out_point)
    {
        out_point = in.read_u32();
    }
    if (has_loops)
    {
        m_loop_count = in.read_u16();
    }

    if (has_envelope)
    {
        in.ensureBytes(1);
        int nPoints = in.read_u8();

        m_envelopes.resize(nPoints);
        in.ensureBytes(8*nPoints);
        for (int i=0; i < nPoints; i++)
        {
            m_envelopes[i].m_mark44 = in.read_u32();
            m_envelopes[i].m_level0 = in.read_u16();
            m_envelopes[i].m_level1 = in.read_u16();
        }
    }

}

void
StartSoundTag::execute(MovieClip* m, DisplayList& /* dlist */) const
{
    //GNASH_REPORT_FUNCTION;

    sound::sound_handler* handler = 
        m->getVM().getRoot().runInfo().soundHandler();

    if (handler)
    {
        if (m_stop_playback)
        {
            //log_debug("Execute StartSoundTag with 'stop playback' flag on");
            handler->stop_sound(m_handler_id);
        }
        else
        {
            //log_debug("Execute StartSoundTag with 'stop playback' flag OFF");
            handler->play_sound(m_handler_id, m_loop_count, 0, 0,
                    (m_envelopes.empty() ? NULL : &m_envelopes));
        }
    }
}


} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
