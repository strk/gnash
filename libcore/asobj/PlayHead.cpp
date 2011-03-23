// PlayHead.cpp: media playback controller 
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
//


#include "PlayHead.h"
#include "VirtualClock.h" // for use

#include <cassert>

namespace gnash {
 
PlayHead::PlayHead(VirtualClock* clockSource)
    :
    _position(0),
    _state(PLAY_PAUSED),
    _availableConsumers(0),
    _positionConsumers(0),
    _clockSource(clockSource)
{
    // NOTE: we construct in PAUSE mode so do not
    // really *need* to query _clockSource here.
    // We initialize it to an arbitrary value just
    // to be polite.
    _clockOffset = 0; // _clockSource->elapsed();
}

PlayHead::PlaybackStatus
PlayHead::setState(PlaybackStatus newState)
{
    if ( _state == newState ) return _state; // nothing to do

    if ( _state == PLAY_PAUSED )
    {
        assert(newState == PLAY_PLAYING);
        _state = PLAY_PLAYING;

        // if we go from PAUSED to PLAYING, reset
        // _clockOffset to yank current position
        // when querying clock source *now*
        boost::uint64_t now = _clockSource->elapsed();
        _clockOffset = ( now - _position );

        // check if we did the right thing
        // TODO: wrap this in PARANOIA_LEVEL > 1
        assert( now-_clockOffset == _position );

        return PLAY_PAUSED;
    }
    else
    {
        // TODO: wrap these in PARANOIA_LEVEL > 1 (or > 2)
        assert(_state == PLAY_PLAYING);
        assert(newState == PLAY_PAUSED);

        _state = PLAY_PAUSED;

        // When going from PLAYING to PAUSED
        // we do nothing with _clockOffset
        // as we'll update it when getting back to PLAYING
        return PLAY_PLAYING;
    }
}

PlayHead::PlaybackStatus
PlayHead::toggleState()
{
    if ( _state == PLAY_PAUSED ) return setState(PLAY_PLAYING);
    else return setState(PLAY_PAUSED);
}

void
PlayHead::advanceIfConsumed()
{
    if ( (_positionConsumers & _availableConsumers) != _availableConsumers)
    {
        // not all available consumers consumed current position,
        // won't advance
#if 0
        log_debug("PlayHead::advance(): "
            "not all consumers consumed current position, "
            "won't advance");
#endif
        return;
    }

    // Advance position
    boost::uint64_t now = _clockSource->elapsed();
    _position = now-_clockOffset;

    // Reset consumers state
    _positionConsumers = 0;
}

void
PlayHead::seekTo(boost::uint64_t position)
{
    boost::uint64_t now = _clockSource->elapsed();
    _position = position;

    _clockOffset = ( now - _position );
    assert( now-_clockOffset == _position ); // check if we did the right thing

    // Reset consumers state
    _positionConsumers = 0;
}

} // end of gnash namespace
