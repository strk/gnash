// DefineButtonSoundTag.h: sounds for Button DisplayObjects.
//
//   Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_SWF_DEFINEBUTTONSOUNDTAG_H
#define GNASH_SWF_DEFINEBUTTONSOUNDTAG_H

#include "smart_ptr.h"
#include "swf.h"
#include "sound_definition.h" // For sound_sample. Ugh.
#include "SoundInfoRecord.h" 

#include <vector>

namespace gnash {
    class SWFStream;
    class RunInfo;
    class movie_definition;
}

namespace gnash {
namespace SWF {

class DefineButtonSoundTag
{

public:

	struct ButtonSound
	{
		boost::uint16_t soundID;
		sound_sample* sample;
		SoundInfoRecord soundInfo;

		ButtonSound()
			:
            soundID(0),
			sample(0)
		{}

#ifdef GNASH_USE_GC
		/// Mark all reachable resources (for GC)
		//
		/// Reachable resources are:
		///  - sound sample (sample)
		///
		void markReachableResources() const
        {
            if (sample) sample->setReachable();
        }
#endif // GNASH_USE_GC
	};

    typedef std::vector<ButtonSound> Sounds;

    static void loader(SWFStream& in, TagType tag, movie_definition& m,
		    const RunInfo& r);

    const ButtonSound& getSound(Sounds::size_type index) const
    {
        // There should be exactly four sounds.
        assert (index < 4);
        return _sounds[index];
    }
#ifdef GNASH_USE_GC
    /// Mark all reachable resources (for GC)
    //
    /// Reachable resources are:
    ///  - button sound infos (_sounds)
    ///
    void markReachableResources() const
    {
        for (Sounds::const_iterator i=_sounds.begin(), e = _sounds.end();
                i != e; ++i)
        {
            i->markReachableResources();
        }
    }
#endif // GNASH_USE_GC

private:

    /// Construct a DefineButtonSoundTag.
    //
    /// This can only be used from the loader() function.
    DefineButtonSoundTag(SWFStream& in, movie_definition& m);

    void read(SWFStream& in, movie_definition& m);

    Sounds _sounds;

};

}
}

#endif
