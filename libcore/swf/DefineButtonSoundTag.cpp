// DefineButtonSoundTag.cpp: sounds for Button DisplayObjects.
//
//   Copyright (C) 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "RunResources.h"
#include "DefineButtonSoundTag.h"
#include "movie_definition.h"
#include "SoundInfoRecord.h"
#include "SWFStream.h"
#include "DefineButtonTag.h"
#include "utility.h"

#include <vector>
namespace gnash {
namespace SWF {

DefineButtonSoundTag::DefineButtonSoundTag(SWFStream& in, movie_definition& m)
    :
    _sounds(4, ButtonSound())
{
    read(in, m);
}

void
DefineButtonSoundTag::loader(SWFStream& in, TagType tag, movie_definition& m,
        const RunResources& /*r*/)
{
    assert(tag == SWF::DEFINEBUTTONSOUND);

    in.ensureBytes(2);
    int id = in.read_u16();
    DefinitionTag* chdef = m.getDefinitionTag(id);
    if (!chdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("DEFINEBUTTONSOUND refers to an unknown "
                "DisplayObject def %d"), id);
        );
        return;
    }

    DefineButtonTag* button = dynamic_cast<DefineButtonTag*> (chdef);

    if (!button)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBUTTONSOUND refers to DisplayObject id "
                "%d, a %s (expected a button DisplayObject)"),
                id, typeName(*chdef));
        );
        return;
    }

    if (button->hasSound())
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Attempt to redefine button sound ignored"));
        );
        return;
    }

    std::auto_ptr<DefineButtonSoundTag> bs(
            new DefineButtonSoundTag(in, m));
    button->addSoundTag(bs);
}


void
DefineButtonSoundTag::read(SWFStream& in, movie_definition& m)
{

	for (Sounds::iterator i = _sounds.begin(), e = _sounds.end(); i != e; ++i)
	{
		ButtonSound& sound = *i;
		in.ensureBytes(2);
		sound.soundID = in.read_u16();
		if (sound.soundID)
		{
			sound.sample = m.get_sound_sample(sound.soundID);
			if (!sound.sample)
			{
				IF_VERBOSE_MALFORMED_SWF(
				log_swferror(_("sound tag not found, sound_id=%d, "
                        "button state #=%i"), sound.soundID);
				);
			}
			IF_VERBOSE_PARSE(
                log_parse("\tsound_id = %d", sound.soundID);
			);
			sound.soundInfo.read(in);
		}
	}
}


}
}
