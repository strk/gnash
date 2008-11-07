// SoundInfo.h: parse and store a SoundInfo record.
//
//   Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_SOUNDINFO_H
#define GNASH_SWF_SOUNDINFO_H

#include "sound_handler.h" // sound::SoundEnvelopes

namespace gnash {
    class SWFStream;
}

namespace gnash {
namespace SWF {

struct SoundInfo
{
    void read(SWFStream& in);

    bool m_no_multiple;
    bool m_stop_playback;
    bool m_has_envelope;
    bool m_has_loops;
    bool m_has_out_point;
    bool m_has_in_point;
    boost::uint32_t m_in_point;
    boost::uint32_t m_out_point;
    boost::uint16_t m_loop_count;
    sound::SoundEnvelopes m_envelopes;
};

}
}

#endif
