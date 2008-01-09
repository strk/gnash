// gnashpython.cpp: python bindings to Gnash
// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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


#include "GnashException.h"
#include "URL.h"
#include "noseek_fd_adapter.h"
#include "movie_definition.h"
#include "movie_instance.h"
#include "movie_root.h"
#include "sprite_instance.h"
#include "as_environment.h"
#include "gnash.h" // for create_movie and create_library_movie and for gnash::key namespace
#include "VM.h" // for initialization
#include "sound_handler.h" // for creating the "test" sound handlers
#include "render.h" // for get_render_handler
#include "types.h" // for rgba class
#include "render.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include "SystemClock.h"
#include "impl.h"

#include <cstdio>
#include <string>
#include <memory> // for auto_ptr
#include <boost/shared_ptr.hpp>

#include "gnashpython.h"
#include "sound_handler.h"

namespace gnash {

namespace pythonwrapper {


GnashPlayer::GnashPlayer()
{
    init();
}

GnashPlayer::~GnashPlayer()
{
}

void
GnashPlayer::init() {
    gnash::gnashInit();
}

void
GnashPlayer::setBaseURL(std::string url)
{
    if (url == "" || _url != "") return;

    _url = url;

    // Pass the base URL to the core libs. We can only do this
    // once! Checking if it's been set when it hasn't trigger
    // an assertion failure...
    gnash::set_base_url( (gnash::URL)_url );
}

bool
GnashPlayer::createMovieDefinition()
{

    // Fail if base URL not set
    if (_url == "") return false;

    _fp = fopen(_url.c_str(), "rb");
    
    if (!_fp) return false;

    std::auto_ptr<tu_file> in(noseek_fd_adapter::make_stream(fileno(_fp)));
      
    _movieDef = gnash::create_movie(in, _url, false);
    
    if (!_movieDef) {
        // Something didn't work
        fclose(_fp);
	return false;
    }
    
    return true;
}

bool
GnashPlayer::initVM()
{

    // If movie definition hasn't already been created, try to create it,
    // fail if that doesn't work either.
    if (!_movieDef && !createMovieDefinition()) return false;

    // Initialize the VM with a manual clock
    _movieRoot = &(gnash::VM::init(*_movieDef, _manualClock).getRoot());

    if (!_movieRoot) {
        // Something didn't work
        return false;
    }
    
    _movieDef->completeLoad();
    _movieDef->ensure_frame_loaded(getSWFFrameCount());
    
    fclose (_fp);

    auto_ptr<movie_instance> mi (_movieDef->create_movie_instance());

    // Set _movie before calling ::render
    _movieInstance = mi.get();

    // Finally, place the root movie on the stage ...
    _movieRoot->setRootMovie( mi.release() ); 
   
    return true; 
}

void
GnashPlayer::allowRescale(bool allow)
{
    if (!_movieRoot || !_movieDef) return;
    _movieRoot->allowRescaling(allow);
}

void
GnashPlayer::advanceClock(unsigned long ms)
{
    if (!_movieRoot || !_movieDef) return;
    _manualClock.advance(ms);
}

void
GnashPlayer::advance() {

    if (!_movieRoot || !_movieDef) return;

    float fps = getSWFFrameRate();
    unsigned long clockAdvance = long(1000/fps);
    advanceClock(clockAdvance);

    _movieRoot->advance();
}

void
GnashPlayer::pressKey(int code)
{
    if (!_movieRoot) return;	
    _movieRoot->notify_key_event((gnash::key::code)code, true);
}

void
GnashPlayer::restart()
{
    if (!_movieDef || !_movieRoot) return;
    _movieRoot->getRootMovie()->restart();
}


int
GnashPlayer::getSWFBytesLoaded() const
{
    if (!_movieDef) return 0;
    return _movieDef->get_bytes_loaded();
}

int
GnashPlayer::getSWFFrameCount() const
{
    if (!_movieDef) return 0;
    return _movieDef->get_frame_count();
}

std::string
GnashPlayer::getSWFURL() const
{
    if (!_movieDef) return "";
    return _movieDef->get_url();
}

int
GnashPlayer::getSWFBytesTotal() const
{
    if (!_movieDef) return 0;
    return _movieDef->get_bytes_total();
}

int
GnashPlayer::getSWFVersion() const
{
    if (!_movieDef) return 0;
    return _movieDef->get_version();
}

float
GnashPlayer::getSWFWidth() const
{
    if (!_movieDef) return 0;
    return _movieDef->get_width_pixels();
}

float
GnashPlayer::getSWFHeight() const
{
    if (!_movieDef) return 0;
    return _movieDef->get_height_pixels();
}

float
GnashPlayer::getSWFFrameRate() const
{
    if (!_movieDef) return 0;
    return _movieDef->get_frame_rate();
}

int
GnashPlayer::getCurrentFrame() const
{
    if (!_movieDef || !_movieRoot) return 0;
    return _movieRoot->getRootMovie()->get_current_frame();
}

// Rendering stuff

bool
GnashPlayer::initRenderer(const std::string& r)
{
    if (_handler) return false;

#ifdef RENDERER_AGG
    if (r.substr(0,4) == "AGG_") {
        _handler = gnash::create_render_handler_agg(r.substr(4).c_str());
    }
#endif
#ifdef RENDERER_CAIRO
    else if (r == "Cairo") {
        _handler = (gnash::renderer::cairo::create_handler(); 
    }
#endif 
#ifdef RENDERER_OPENGL
    else if (r == "OpenGL") {
        _handler = gnash::create_render_handler_ogl(false);
    }
#endif

    if (_handler) {
       return addRenderer(_handler); // False if it fails
    }
    else {
        return false;
    }
    
    return true;
}

void
GnashPlayer::render()
{

    if (!_handler) return;

    _invalidatedBounds.setNull();

    _movieRoot->add_invalidated_bounds(_invalidatedBounds, false);

    // Force full redraw by using a WORLD invalidated ranges
    gnash::InvalidatedRanges ranges = _invalidatedBounds; 
    if ( _forceRedraw ) {
        ranges.setWorld(); // set to world if asked a full redraw
        _forceRedraw = false; // reset to no forced redraw
    }
    
    gnash::set_render_handler(_handler);

    _handler->set_invalidated_regions(ranges);
    _movieRoot->display();
	
}

bool
GnashPlayer::addRenderer(gnash::render_handler* handler)
{
    if (!handler->initTestBuffer(getSWFWidth(), getSWFHeight())) {
        cout << "Failed to init renderer, but the renderer was okay" << "\n";
        return false;
    }

    gnash::set_render_handler(handler);
	
    return true;
}

// Gnash Character

GnashCharacter*
GnashPlayer::getCharacterById(int id)
{
    if (!_movieDef || !_movieRoot) return NULL;

    gnash::character* c = _movieRoot->getRootMovie()->get_character(id);
    
    if (!c) return NULL;
    
    GnashCharacter* chr(new GnashCharacter(c));

    return chr;
}

GnashCharacter*
GnashPlayer::getTopmostMouseEntity()
{
    if (!_movieDef || !_movieRoot) return NULL;

    gnash::character* c = _movieRoot->getActiveEntityUnderPointer();
    
    if (!c) return NULL;
    
    GnashCharacter* chr(new GnashCharacter(c));

    return chr;
}

GnashCharacter::GnashCharacter(gnash::character* c)
    :
    _character(c)
{
}

GnashCharacter::GnashCharacter()
{
}

GnashCharacter::~GnashCharacter()
{
}

} // end pythonwrapper
} // end gnash
