// NullSoundHandler - fake sound handler, for testing gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
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


#ifndef NULL_SOUND_HANDLER_H
#define NULL_SOUND_HANDLER_H

#include "sound_handler.h" // for inheritance
#include "dsodefs.h" // for DSOEXPORT

namespace gnash {

namespace media {
    class MediaHandler;
}

namespace sound {

/// Null sound_handler, for testing or manual fetching of samples
class DSOEXPORT NullSoundHandler : public sound_handler
{
public:

    sound_handler* _mixer;

    NullSoundHandler(media::MediaHandler* m, sound_handler* mixer=0)
        :
        sound_handler(m),
        _mixer(mixer)
    {}

    // If a _mixer was given, let it do the mixing!
    void mix(boost::int16_t* outSamples, boost::int16_t* inSamples,
                unsigned int nSamples, float volume)
    {
        if ( _mixer ) _mixer->mix(outSamples, inSamples, nSamples, volume);
        else {
            // cheating, just copy input to output, which in NO WAY
            // can be considered "mixing"
            std::copy(outSamples, outSamples+nSamples, inSamples);
        }
    }


};
	
} // gnash.sound namespace 
} // namespace gnash

#endif // NULL_SOUND_HANDLER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
