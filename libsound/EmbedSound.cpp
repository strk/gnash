// EmbedSound.cpp - embedded sound definition, for gnash
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#include <cstdint>

#include "EmbedSoundInst.h" 
#include "SoundInfo.h"
#include "MediaHandler.h" 
#include "log.h"
#include "GnashException.h" 

namespace gnash {
namespace sound {

EmbedSound::EmbedSound(std::unique_ptr<SimpleBuffer> data,
        media::SoundInfo info, int nVolume)
    :
    soundinfo(std::move(info)),
    volume(nVolume),
    _buf(data.release())
{
    if (!_buf.get()) _buf.reset(new SimpleBuffer());
}

void
EmbedSound::clearInstances()
{
    std::lock_guard<std::mutex> lock(_soundInstancesMutex);
    _soundInstances.clear();
}

EmbedSound::Instances::iterator
EmbedSound::eraseActiveSound(Instances::iterator i)
{
    // Mutex intentionally NOT locked...
    return _soundInstances.erase(i);
}

std::unique_ptr<EmbedSoundInst>
EmbedSound::createInstance(media::MediaHandler& mh, unsigned int inPoint,
        unsigned int outPoint, const SoundEnvelopes* envelopes,
        int loopCount)
{
    std::unique_ptr<EmbedSoundInst> ret(
        new EmbedSoundInst(*this, mh, inPoint, outPoint, envelopes, loopCount));

    std::lock_guard<std::mutex> lock(_soundInstancesMutex);

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
    std::lock_guard<std::mutex> lock(_soundInstancesMutex);

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
    std::lock_guard<std::mutex> lock(_soundInstancesMutex);
    return !_soundInstances.empty();
}

size_t
EmbedSound::numPlayingInstances() const
{
    std::lock_guard<std::mutex> lock(_soundInstancesMutex);
    return _soundInstances.size();
}

EmbedSoundInst*
EmbedSound::firstPlayingInstance() const
{
    std::lock_guard<std::mutex> lock(_soundInstancesMutex);
    return _soundInstances.front();
}

void
EmbedSound::getPlayingInstances(std::vector<InputStream*>& to) const
{
    std::lock_guard<std::mutex> lock(_soundInstancesMutex);
    for (InputStream* stream : _soundInstances)
    {
        to.push_back(stream);
    }
}

} // gnash.sound namespace 
} // namespace gnash
