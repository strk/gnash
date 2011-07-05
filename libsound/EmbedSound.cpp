// EmbedSound.cpp - embedded sound definition, for gnash
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "EmbedSound.h"

#include <vector>
#include <boost/cstdint.hpp>

#include "EmbedSoundInst.h" 
#include "SoundInfo.h"
#include "MediaHandler.h" 
#include "log.h"
#include "GnashException.h" 

namespace gnash {
namespace sound {

EmbedSound::EmbedSound(std::auto_ptr<SimpleBuffer> data,
        const media::SoundInfo& info, int nVolume)
    :
    soundinfo(info),
    volume(nVolume),
    _buf(data.release())
{
    if (!_buf.get()) _buf.reset(new SimpleBuffer());
}

void
EmbedSound::clearInstances()
{
    boost::mutex::scoped_lock lock(_soundInstancesMutex);
    _soundInstances.clear();
}

EmbedSound::Instances::iterator
EmbedSound::eraseActiveSound(Instances::iterator i)
{
    // Mutex intentionally NOT locked...
    return _soundInstances.erase(i);
}

std::auto_ptr<EmbedSoundInst>
EmbedSound::createInstance(media::MediaHandler& mh, unsigned int inPoint,
        unsigned int outPoint, const SoundEnvelopes* envelopes,
        unsigned int loopCount)
{
    std::auto_ptr<EmbedSoundInst> ret(
        new EmbedSoundInst(*this, mh, inPoint, outPoint, envelopes, loopCount));

    boost::mutex::scoped_lock lock(_soundInstancesMutex);

    // Push the sound onto the playing sounds container.
    _soundInstances.push_back(ret.get());

    return ret;
}

EmbedSound::~EmbedSound()
{
    clearInstances();
}

void
EmbedSound::eraseActiveSound(EmbedSoundInst* inst)
{
    boost::mutex::scoped_lock lock(_soundInstancesMutex);

    Instances::iterator it = std::find( _soundInstances.begin(),
            _soundInstances.end(), inst);

    if (it == _soundInstances.end()) {
        log_error("EmbedSound::eraseActiveSound: instance %p not found!", inst);
        return;
    }
    
    eraseActiveSound(it);
}

bool
EmbedSound::isPlaying() const
{
    boost::mutex::scoped_lock lock(_soundInstancesMutex);
    return !_soundInstances.empty();
}

size_t
EmbedSound::numPlayingInstances() const
{
    boost::mutex::scoped_lock lock(_soundInstancesMutex);
    return _soundInstances.size();
}

EmbedSoundInst*
EmbedSound::firstPlayingInstance() const
{
    boost::mutex::scoped_lock lock(_soundInstancesMutex);
    return _soundInstances.front();
}

void
EmbedSound::getPlayingInstances(std::vector<InputStream*>& to) const
{
    boost::mutex::scoped_lock lock(_soundInstancesMutex);
    for (Instances::const_iterator i=_soundInstances.begin(), e=_soundInstances.end();
            i!=e; ++i)
    {
        to.push_back(*i);
    }
}

} // gnash.sound namespace 
} // namespace gnash
