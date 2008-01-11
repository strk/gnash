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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "GnashException.h"
#include "URL.h"
#include "noseek_fd_adapter.h"
#include "movie_definition.h"
#include "movie_instance.h"
#include "movie_root.h"
#include "sprite_instance.h"
#include "gnash.h"
#include "VM.h"
#include "render.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include "SystemClock.h"
#include "log.h"

#include <cstdio>
#include <string>
#include <memory>

#include "gnashpython.h"

#define REQUIRE_MOVIE_LOADED if (!_movieDef) throw GnashException("No Movie Loaded!")
#define REQUIRE_VM_STARTED if (!_movieDef || !_movieRoot) throw GnashException("VM not started!")

namespace gnash {

namespace pythonwrapper {


GnashPlayer::GnashPlayer()
	:
    _movieDef(NULL),
    _movieRoot(NULL),
    _renderer(NULL),
    _logFile(gnash::LogFile::getDefaultInstance()),
    _xpos(0),
    _ypos(0),
    _url(""),
    _fp(NULL)
{
    init();
}

GnashPlayer::~GnashPlayer()
{
    close();
}

// Initialize the core libs. If this gets called twice:
// f = gnash.Player()
// g = gnash.Player()
// we fail an assertion because the player can only handle
// one movie at once.
// TODO: Find way of checking whether gnashInit() has been called
// Better TODO: Get Gnash to handle more than one movie.
void
GnashPlayer::init()
{
    gnash::gnashInit();
}

void
GnashPlayer::close()
{
    gnash::clear();
}

// Set our _url member and pass this to the core.
void
GnashPlayer::setBaseURL(std::string url)
{

    // Don't allow empty urls
    if (url == "" || _url != "") return;

    _url = url;

    // Pass the base URL to the core libs. We can only do this
    // once! Checking if it's been set when it hasn't trigger
    // an assertion failure...
    gnash::set_base_url( (gnash::URL)_url );
}

// TODO: Read in movies from a python file object.
bool
GnashPlayer::loadMovie(std::string url)
{

    if (_movieDef) return false;
    
    setBaseURL(url);

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
    if (!_movieDef  && !loadMovie(_url)) return false;

    // Initialize the VM with a manual clock
    _movieRoot = &(gnash::VM::init(*_movieDef, _manualClock).getRoot());

    if (!_movieRoot) {
        // Something didn't work
        return false;
    }
    
    _movieDef->completeLoad();
    _movieDef->ensure_frame_loaded(getSWFFrameCount());
    
    if (_fp) {
        fclose (_fp);
    }

    // I don't know why it's done like this.
    auto_ptr<movie_instance> mi (_movieDef->create_movie_instance());

    // Put the instance on stage.
    _movieRoot->setRootMovie( mi.release() ); 
   
    return true; 
}

// Whether the movie can be resized or not
void
GnashPlayer::allowRescale(bool allow)
{
    REQUIRE_VM_STARTED;
    _movieRoot->allowRescaling(allow);
}

// Whether debug messages are sent to stdout
void
GnashPlayer::setVerbose(bool verbose)
{
    // Can't turn this off yet...
    if (verbose) _logFile.setVerbosity();
}

// Move Gnash's sense of time along manually
void
GnashPlayer::advanceClock(unsigned long ms)
{
    REQUIRE_VM_STARTED;
    _manualClock.advance(ms);
}

// This moves the manual clock on automatically by the length of
// time allocated to a frame by the FPS setting.
void
GnashPlayer::advance() {

    REQUIRE_VM_STARTED;

    float fps = getSWFFrameRate();
    unsigned long clockAdvance = long(1000 / fps);
    advanceClock(clockAdvance);

    _movieRoot->advance();
}

// Send a key event to the movie. This is matched to
// gnash::key::code. You could even use this to 
// implement a UI in python.
bool
GnashPlayer::pressKey(int code)
{
    REQUIRE_VM_STARTED;	
    return _movieRoot->notify_key_event((gnash::key::code)code, true);
}

// Move the pointer to the specified coordinates.
bool
GnashPlayer::movePointer(int x, int y)
{
    _xpos = x;
    _ypos = y;
    return _movieRoot->notify_mouse_moved(x, y);
}

// Click the mouse at the specified coordinates.
bool
GnashPlayer::mouseClick()
{
    return ((
    		_movieRoot->notify_mouse_clicked(true, 1) ||
		_movieRoot->notify_mouse_clicked(false, 1)
    		));
}

// Start the movie from the beginning.
void
GnashPlayer::restart()
{
    REQUIRE_VM_STARTED;
    _movieRoot->getRootMovie()->restart();
}

// The number of bytes already loaded.
int
GnashPlayer::getSWFBytesLoaded() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_bytes_loaded();
}

// The number of frames reported in the movie headers.
int
GnashPlayer::getSWFFrameCount() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_frame_count();
}

// The URL of the stream.
std::string
GnashPlayer::getSWFURL() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_url();
}

// The length in bytes reported in the movie headers.
int
GnashPlayer::getSWFBytesTotal() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_bytes_total();
}

// The version of the root movie.
int
GnashPlayer::getSWFVersion() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_version();
}

// The width of the movie.
float
GnashPlayer::getSWFWidth() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_width_pixels();
}

// The height of the movie.
float
GnashPlayer::getSWFHeight() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_height_pixels();
}

// The movie's frame rate.
float
GnashPlayer::getSWFFrameRate() const
{
    REQUIRE_MOVIE_LOADED;
    return _movieDef->get_frame_rate();
}

// The current position in the movie.
int
GnashPlayer::getCurrentFrame() const
{
    REQUIRE_VM_STARTED;
    return _movieRoot->getRootMovie()->get_current_frame();
}

//
//    Renderer functions
//

// Initialize the named renderer  
bool
GnashPlayer::setRenderer(const std::string& r)
{

    // Set pointer to NULL in case we want to change renderer, which
    // is entirely possible. This is necessary to check if the switch
    // succeeded or not.
    _renderer = NULL;

#ifdef RENDERER_AGG
    if (r.substr(0,4) == "AGG_") {
        _renderer = gnash::create_render_handler_agg(r.substr(4).c_str());
    }
#endif
#ifdef RENDERER_CAIRO
    else if (r == "Cairo") {
        _renderer = (gnash::renderer::cairo::create_renderer(); 
    }
#endif 
#ifdef RENDERER_OPENGL
    else if (r == "OpenGL") {
        _renderer = gnash::create_render_handler_ogl(false);
    }
#endif

    if (!_renderer) {
        // If the handler doesn't exist or can't be opened, return false.
        return false;
    }

    // Try to add the renderer.
    return addRenderer(_renderer);

}

// Test and set the render handler, returning false if anything
// goes wrong.
bool
GnashPlayer::addRenderer(gnash::render_handler* handler)
{
    // A brief test to see if the renderer works.
    if (!handler->initTestBuffer(getSWFWidth(), getSWFHeight())) {
        return false;
    }

    gnash::set_render_handler(handler);
	
    return true;
}

// Render the frame
void
GnashPlayer::render(bool forceRedraw)
{

    if (!_renderer) return;

    _invalidatedBounds.setNull();

    _movieRoot->add_invalidated_bounds(_invalidatedBounds, false);

    gnash::InvalidatedRanges ranges = _invalidatedBounds; 

    if (forceRedraw) {
        // Change invalidated regions to cover entire movie
        // if the caller asked for a full redraw
        ranges.setWorld();
    }

    _renderer->set_invalidated_regions(ranges);
    _movieRoot->display();
	
}

//
// Wrapper class for characters (doesn't work)
//

GnashCharacter*
GnashPlayer::getCharacterById(int id)
{
    REQUIRE_VM_STARTED;

    gnash::character* c = _movieRoot->getRootMovie()->get_character(id);
    
    if (!c) return NULL;
    
    GnashCharacter* chr(new GnashCharacter(c));

    return chr;
}

GnashCharacter*
GnashPlayer::getTopmostMouseEntity()
{
    REQUIRE_VM_STARTED;

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
    :
    _character(NULL)
{
}

GnashCharacter::~GnashCharacter()
{
}

} // end pythonwrapper
} // end gnash
