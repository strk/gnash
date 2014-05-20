// PlayHead.h: media playback controller 
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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


#ifndef GNASH_PLAYHEAD_H
#define GNASH_PLAYHEAD_H

#include <cstdint> // For C99 int types

// Forward declarations
namespace gnash {
    class VirtualClock;
}

namespace gnash {

/// The playback controller
class PlayHead {

public:

    /// Flags for playback state
    enum PlaybackStatus {
        PLAY_PLAYING = 1,
        PLAY_PAUSED = 2
    };

    /// Initialize playhead given a VirtualCock to use
    /// as clock source.
    //
    /// The PlayHead will have initial state set to PAUSED
    /// and initial position set to 0.
    ///
    /// @param clockSource
    /// The VirtualClock to use as time source.
    /// Ownership left to caller (not necessarely a good thing).
    ///
    PlayHead(VirtualClock* clockSource);

    /// Set a video consumer as available
    //
    /// This should be completely fine to do during
    /// PlayHead lifetime.
    ///
    void setVideoConsumerAvailable()
    {
        _availableConsumers |= CONSUMER_VIDEO;
    }

    /// Set an audio consumer as available
    //
    /// This should be completely fine to do during
    /// PlayHead lifetime.
    ///
    void setAudioConsumerAvailable()
    {
        _availableConsumers |= CONSUMER_AUDIO;
    }

    /// Get current playhead position (milliseconds)
    std::uint64_t getPosition() const { return _position; }

    /// Get current playback state
    PlaybackStatus getState() const { return _state; }

    /// Set playback state, returning old state
    PlaybackStatus setState(PlaybackStatus newState);

    /// Toggle playback state, returning old state
    PlaybackStatus toggleState();

    /// Return true if video of current position have been consumed
    bool isVideoConsumed() const
    {
        return (_positionConsumers & CONSUMER_VIDEO);
    }

    /// Mark current position as being consumed by video consumer
    void setVideoConsumed()
    {
        _positionConsumers |= CONSUMER_VIDEO;
    }

    /// Return true if audio of current position have been consumed
    bool isAudioConsumed() const
    {
        return (_positionConsumers & CONSUMER_AUDIO);
    }

    /// Mark current position as being consumed by audio consumer
    void setAudioConsumed()
    {
        _positionConsumers |= CONSUMER_AUDIO;
    }

    /// Change current position to the given time.
    //
    /// Consume flag will be reset.
    ///
    /// @param position
    /// Position timestamp (milliseconds)
    ///
    /// POSTCONDITIONS:
    /// - isVideoConsumed() == false
    /// - isAudioConsumed() == false
    /// - getPosition() == position
    ///
    void seekTo(std::uint64_t position);

    /// Advance position if all available consumers consumed the current one
    //
    /// Clock source will be used to determine the amount
    /// of milliseconds to advance position to.
    ///
    /// Consumer flags will be reset.
    ///
    /// POSTCONDITIONS:
    /// - isVideoConsumed() == false
    /// - isAudioConsumed() == false
    ///
    void advanceIfConsumed();
        

private:

    /// Flags for consumers state
    enum ConsumerFlag {
        CONSUMER_VIDEO = 1,
        CONSUMER_AUDIO = 2
    };

    /// Current playhead position
    std::uint64_t _position;

    /// Current playback state
    PlaybackStatus _state;

    /// Binary OR of consumers representing
    /// which consumers are active
    int _availableConsumers;

    /// Binary OR of consumers representing
    /// which consumers consumed current position
    int _positionConsumers;

    /// The clock source, externally owned
    VirtualClock* _clockSource;

    /// Offset to subtract from current clock source
    /// to get current position
    //
    /// The offset will be 
    std::uint64_t _clockOffset;

};

} // end of gnash namespace

// __PLAYHEAD_H__
#endif

