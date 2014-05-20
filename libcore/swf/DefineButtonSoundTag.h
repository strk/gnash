// DefineButtonSoundTag.h: sounds for Button DisplayObjects.
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "SoundInfoRecord.h" 
#include "SWF.h"

#include <vector>

namespace gnash {
    class SWFStream;
    class RunResources;
    class movie_definition;
    class sound_sample;
}

namespace gnash {
namespace SWF {

class DefineButtonSoundTag
{

public:

	struct ButtonSound
	{
		std::uint16_t soundID;
		sound_sample* sample;
		SoundInfoRecord soundInfo;

		ButtonSound()
			:
			soundID(0),
			sample(0)
		{}

	};

    typedef std::vector<ButtonSound> Sounds;

    static void loader(SWFStream& in, TagType tag, movie_definition& m,
		    const RunResources& r);

    const ButtonSound& getSound(Sounds::size_type index) const
    {
        // There should be exactly four sounds.
        assert (index < 4);
        return _sounds[index];
    }

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
