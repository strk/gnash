// RunResources.h    Hold external and per-run resources for Gnash core.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011. 2012
//   Free Software Foundation, Inc.
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
#include <boost/shared_ptr.hpp>
#include "StreamProvider.h"
#include "Renderer.h"
#include "sound_handler.h"
#include "MediaHandler.h"
#include "TagLoadersTable.h"

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
class RunResources
{
public:

    /// Constructs a RunResources instance with an immutable base URL.
    //
    /// @param baseURL  The base URL for the run. This cannot be changed after
    ///                 construction.
    RunResources() {}

    /// Set the StreamProvider.
    //
    /// This can probably be changed during a run without ill effects.
    void setStreamProvider(std::shared_ptr<StreamProvider> sp) {
        _streamProvider = sp;
    }

    /// Get a StreamProvider instance.
    //
    /// This isn't optional. It must always be available, or nothing
    /// can be loaded.
    //
    /// @return     A StreamProvider 
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
    void setSoundHandler(std::shared_ptr<sound::sound_handler> s) {
        _soundHandler = s;
    } 

    /// Get a pointer to a sound::sound_handler set by a hosting application.
    //
    /// @return     A pointer to a sound::sound_handler, or NULL if none
    ///             has yet been set.
    sound::sound_handler* soundHandler() const {
        return _soundHandler.get();
    }

    void setMediaHandler(std::shared_ptr<media::MediaHandler> s) {
        _mediaHandler = s;
    }

    media::MediaHandler* mediaHandler() const {
        return _mediaHandler.get();
    }

    void setRenderer(std::shared_ptr<Renderer> r) {
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
    void setTagLoaders(std::shared_ptr<const SWF::TagLoadersTable> loaders) {
        _tagLoaders = loaders;
    }

    /// Get the loader function table for parsing a SWF.
    const SWF::TagLoadersTable& tagLoaders() const {
        assert(_tagLoaders.get());
        return *_tagLoaders;
    }

#if 1
    /// Set the renderer backend, agg, opengl, or cairo. This is set
    /// in the users gnashrc file, or can be overridden with the
    /// --hwaccel option to gnash.
    void setRenderBackend(const std::string& x) { _renderer_backend = x; }
    std::string& getRenderBackend() { return _renderer_backend; }

    /// Set the hardware video accleration backend, none or vaapi.
    /// This is set in the users gnashrc file, or can be
    /// overridden with the --render-mode option to gnash.
    std::string& getHWAccelBackend() { return _hwaccel_backend; }
    void setHWAccelBackend(const std::string& x) { _hwaccel_backend = x; }
#endif

private:

    std::shared_ptr<StreamProvider> _streamProvider;

    std::shared_ptr<sound::sound_handler> _soundHandler;
    
    std::shared_ptr<media::MediaHandler> _mediaHandler;

    std::shared_ptr<Renderer> _renderer;

    std::shared_ptr<const SWF::TagLoadersTable> _tagLoaders;

    /// Whether to ue HW video decoding support, no value means disabled.
    /// The only currently supported values are: none or vaapi.
    /// The default is none,
    std::string _hwaccel_backend;

    /// Which renderer backend to use, no value means use the default.
    /// The currently supported values are agg, opengl, or cairo. AGG
    /// being the default.
    std::string _renderer_backend;
};

} // end of gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

