// gnashpython.cpp: python bindings to Gnash
// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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
#include "Movie.h"
#include "movie_root.h"
#include "MovieClip.h"
#include "gnash.h"
#include "VM.h"
#include "render.h"
#include "render_handler.h"
#include "render_handler_agg.h"
#include "render_handler_cairo.h"
#include "SystemClock.h"
#include "log.h"
#include "rc.h"
#include "StreamProvider.h" // for passing to RunInfo
#include "RunInfo.h" // for initialization

#include <string>
#include <memory>

#include "gnashpython.h"

#define LOG_QUEUE_MAX 200

#define REQUIRE_MOVIE_LOADED if (!_movieDef) throw GnashException("No Movie Loaded!")
#define REQUIRE_VM_STARTED if (!_movieRoot) throw GnashException("VM not started!")

namespace gnash {

namespace pythonwrapper {

std::deque<std::string> GnashPlayer::_logMessages;

GnashPlayer::GnashPlayer()
	:
    _movieDef(NULL),
    _movieRoot(NULL),
    _renderer(NULL),
    _logFile(gnash::LogFile::getDefaultInstance()),
    _xpos(0),
    _ypos(0),
    _url("")
{
}

GnashPlayer::GnashPlayer(const GnashPlayer& o)
	:
    _movieDef(NULL),
    _movieRoot(NULL),
    _renderer(NULL),
    _logFile(gnash::LogFile::getDefaultInstance()),
    _xpos(0),
    _ypos(0),
    _url("")
{
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
    _logFile.registerLogCallback(&receiveLogMessages);
}

void 
GnashPlayer::receiveLogMessages(const std::string& s)
{
    /// Make sure the _logMessage queue doesn't grow out of control.
    if (_logMessages.size() > LOG_QUEUE_MAX)
    {
        _logMessages.pop_front();
    }
    _logMessages.push_back(s);
}

std::string
GnashPlayer::getLogMessage()
{
    if (! _logMessages.empty() )
    {
        std::string ret = _logMessages.front();
        _logMessages.pop_front();
        return ret;
    }
    return "";
}

size_t
GnashPlayer::logSize()
{
    return _logMessages.size();
}

void
GnashPlayer::close()
{
    gnash::clear();
}

// Set our _url member and pass this to the core.
void
GnashPlayer::setBaseURL(const std::string& url)
{

    // Don't allow empty urls
    if (url == "" || _url != "") return;

    _url = url;


    /// The RunInfo should be populated before parsing.
    _runInfo.reset(new RunInfo(url));
    _runInfo->setStreamProvider(boost::shared_ptr<StreamProvider>(
                new StreamProvider));
}


bool
GnashPlayer::loadMovie(PyObject& pf)
{

    if (_movieDef) return false;

    init();
        
    FILE* file = PyFile_AsFile(&pf);

    std::string filename(PyString_AsString(PyFile_Name(&pf)));

    URL baseurl = URL(filename);

    setBaseURL(baseurl.str());

    // Add URL to sandbox
	RcInitFile& rcfile = RcInitFile::getDefaultInstance();
	const std::string& path = baseurl.path();
	size_t lastSlash = path.find_last_of('/');
	std::string dir = path.substr(0, lastSlash+1);
	rcfile.addLocalSandboxPath(dir);

    // Fail if base URL not set
    if (_url == "") return false;

    std::auto_ptr<IOChannel> in(noseek_fd_adapter::make_stream(fileno(file)));
      
    _movieDef = gnash::create_movie(in, _url, *_runInfo, false);
    
    if (!_movieDef) {
        return false;
    }

	_movieRoot = new movie_root(*_movieDef, _manualClock, *_runInfo);

	_movieDef->completeLoad();
	_movieDef->ensure_frame_loaded(_movieDef->get_frame_count());

	std::auto_ptr<Movie> mi ( _movieDef->createMovie() );

	// Finally, place the root movie on the stage ...
    _movieRoot->setRootMovie( mi.release() );
    
    return true;
}


// Whether debug messages are sent to stdout
void
GnashPlayer::setVerbosity(unsigned verbosity)
{
    _logFile.setVerbosity(verbosity);
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
    _movieRoot->reset();
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

geometry::SnappingRanges2d<int>
GnashPlayer::getInvalidatedRanges() const
{
	using namespace gnash::geometry;

	SnappingRanges2d<float> ranges = _invalidatedBounds;

	// scale by 1/20 (twips to pixels)
	ranges.scale(1.0/20);

	// Convert to integer range.
	SnappingRanges2d<int> pixranges(ranges);

	return pixranges;

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
    return _movieRoot->getRootMovie().get_current_frame();
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
    if (r == "Cairo") {
        _renderer = gnash::renderer::cairo::create_handler(); 
    }
#endif 
#ifdef RENDERER_OPENGL
    if (r == "OpenGL") {
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
GnashPlayer::getCharacterByTarget(const std::string& tgt)
{
    REQUIRE_VM_STARTED;

    gnash::DisplayObject* c = _movieRoot->findCharacterByTarget(tgt);
    
    if (!c) return NULL;
    
    GnashCharacter* chr(new GnashCharacter(c));

    return chr;
}


GnashCharacter*
GnashPlayer::getTopmostMouseEntity()
{
    REQUIRE_VM_STARTED;

    gnash::DisplayObject* c = _movieRoot->getActiveEntityUnderPointer();
    
    if (!c) return NULL;
    
    GnashCharacter* chr(new GnashCharacter(c));

    return chr;
}

GnashCharacter::GnashCharacter(gnash::DisplayObject* c)
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
    _character = NULL;
    delete _character;
}

GnashCharacter*
GnashCharacter::getParent()
{
    gnash::DisplayObject* c = _character->get_parent();

    if (!c) return NULL;
    
    GnashCharacter* chr(new GnashCharacter(c));

    return chr;
}

} // end pythonwrapper
} // end gnash
