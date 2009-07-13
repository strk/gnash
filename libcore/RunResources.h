// RunResources.h    Hold external and per-run resources for Gnash core.
// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "TagLoadersTable.h"
#include "StreamProvider.h"
#include "Renderer.h"
#include "sound_handler.h"

#include <string>
#include <boost/shared_ptr.hpp>

namespace gnash {

/// Class to group together per-run and external resources for Gnash
//
/// This holds the following resources:
///     - sound::sound_handler
///     - StreamProvider
/// In addition, it stores the constant base URL for the run.
/// This must be kept alive for the entire duration of a run (presently
/// until the last SWFMovieDefinition has been destroyed).
/// @todo Check the lifetime and update documentation if it changes.
/// @todo   Add MediaHandler.
class RunResources
{
public:

    /// Constructs a RunResources instance with an immutable base URL.
    //
    /// @param baseURL  The base URL for the run. This cannot be changed after
    ///                 construction.
    RunResources(const std::string& baseURL)
        :
        _baseURL(baseURL)
    {
    }

    /// Get the base URL for the run.
    //
    /// @return     The base URL set at construction.
    const std::string& baseURL() const { return _baseURL; }

    /// Set the StreamProvider.
    //
    /// This can probably be changed during a run without ill effects.
    void setStreamProvider(boost::shared_ptr<StreamProvider> sp)
    {
        _streamProvider = sp;
    }

    /// Get a StreamProvider instance.
    //
    /// This isn't optional. It must always be available, or nothing
    /// can be loaded.
    //
    /// @return     A StreamProvider (presently a global singleton).
    const StreamProvider& streamProvider() const {
        assert (_streamProvider.get());
        return *_streamProvider;
    }

    /// Set the sound::sound_handler.
    //
    /// @param s    A pointer to the sound::sound_handler for use
    ///             by Gnash core. This may also be NULL.
    //
    /// This is cached in various places, so changing it during a run will
    /// lead to unexpected behaviour.
    void setSoundHandler(boost::shared_ptr<sound::sound_handler> s) {
        _soundHandler = s;
    } 

    /// Get a pointer to a sound::sound_handler set by a hosting application.
    //
    /// @return     A pointer to a sound::sound_handler, or NULL if none
    ///             has yet been set.
    sound::sound_handler* soundHandler() const {
        return _soundHandler.get();
    }

    void setRenderer(boost::shared_ptr<Renderer> r) {
        _renderer = r;
    }

    Renderer* renderer() const {
        return _renderer.get();
    }

    /// Set the loader functions for SWF parsing.
    //
    /// This must be present before parsing.
    /// It is a pointer to const so that the same table can be shared between
    /// simultaneous runs if desired.
    void setTagLoaders(boost::shared_ptr<const SWF::TagLoadersTable> loaders) {
        _tagLoaders = loaders;
    }

    /// Get the loader function table for parsing a SWF.
    const SWF::TagLoadersTable& tagLoaders() const {
        assert(_tagLoaders.get());
        return *_tagLoaders;
    }

private:

    const std::string _baseURL;

    boost::shared_ptr<StreamProvider> _streamProvider;

    boost::shared_ptr<sound::sound_handler> _soundHandler;

    boost::shared_ptr<Renderer> _renderer;

    boost::shared_ptr<const SWF::TagLoadersTable> _tagLoaders;

};

}

#endif
