// RunInfo.h    Hold external and per-run resources for Gnash core.
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


#ifndef GNASH_RUN_INFO_H
#define GNASH_RUN_INFO_H

#include <string>
#include "StreamProvider.h"

namespace gnash {

// Forward declarations
namespace sound {
    class sound_handler;
}

/// Class to group together per-run and external resources for Gnash
//
/// This holds the following resources:
///     - sound::sound_handler
///     - StreamProvider
/// In addition, it stores the constant base URL for the run.
/// This must be kept alive for the entire duration of a run (presently
/// until the last SWFMovieDefinition has been destroyed).
/// @todo Check the lifetime and update documentation if it changes.
/// @todo   Add render_handler, MediaHandler.
class RunInfo
{
public:

    /// Constructs a RunInfo instance with an immutable base URL.
    //
    /// @param baseURL  The base URL for the run. This cannot be changed after
    ///                 construction.
    RunInfo(const std::string& baseURL)
        :
        _baseURL(baseURL),
        _streamProvider(StreamProvider::getDefaultInstance()),
        _soundHandler(0)
    {
    }

    /// Get the base URL for the run.
    //
    /// @return     The base URL set at construction.
    const std::string& baseURL() const { return _baseURL; }

    /// Get a StreamProvider instance.
    //
    /// This isn't optional. It must always be available, or nothing
    /// can be loaded.
    //
    /// @return     A StreamProvider (presently a global singleton).
    StreamProvider& streamProvider() const { return _streamProvider; }

    /// Set the sound::sound_handler.
    //
    /// @param s    A pointer to the sound::sound_handler for use
    ///             by Gnash core. This may also be NULL.
    //
    /// This is cached in various places, so changing it during a run will
    /// lead to unexpected behaviour.
    void setSoundHandler(sound::sound_handler* s) {
        _soundHandler = s;
    } 

    /// Get a pointer to a sound::sound_handler set by a hosting application.
    //
    /// @return     A pointer to a sound::sound_handler, or NULL if none
    ///             has yet been set.
    sound::sound_handler* soundHandler() const { return _soundHandler; }

private:

    std::string _baseURL;

    StreamProvider& _streamProvider;

    sound::sound_handler* _soundHandler;

};

}

#endif
