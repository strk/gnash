// gnashpython.h: headers for the python bindings to Gnash
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

#ifndef GNASHPYTHON_H
#define GNASHPYTHON_H

#ifdef HAVE_CONFIG_H
# include "gnashconfig.h"
#endif

#include "Range2d.h"
#include "gnash.h"
#include "ManualClock.h"
#include "movie_root.h"
#include "movie_definition.h"
#include "render_handler.h"
#include "DisplayObject.h"
#include "log.h"
#include "RunInfo.h" // for composition
#include "IOChannel.h"

#include <string>
#include <memory>
#include <deque>
#include <boost/python.hpp>
#include <boost/cstdint.hpp> // For C99 int types

// Because the Python bindings are there to allow flexible access
// to Gnash in an interpreted language, there need to be many
// checks on initialization order to avoid memory errors.

// Boost 1.33 seems to dislike auto_ptrs in the class
// declaration. Perhaps I'm not handling them correctly, perhaps
// it's better in 1.34.

// Forward declarations
namespace gnash {
namespace pythonwrapper {
        class GnashCharacter;
        class GnashPlayer;
}
}

namespace gnash {

namespace pythonwrapper {

class GnashPlayer
{

public:

    GnashPlayer();
    ~GnashPlayer();

    // This seems required by BOOST_PYTHON_MODULE ?
    GnashPlayer(const GnashPlayer&) {};

    // For exiting
    void close();

    /// Movie creation
    
    /// Takes a python file object
    bool loadMovie(PyObject& pf);

    // Renderer
    bool setRenderer(const std::string& r);

    // Movie information
    float getSWFFrameRate() const;
    int getSWFFrameCount() const;
    int getSWFVersion() const;
    float getSWFWidth() const;
    float getSWFHeight() const;
    int getSWFBytesTotal() const;
    int getSWFBytesLoaded() const;
    std::string getSWFURL() const;

    // Player state
    int getCurrentFrame() const;
    
    // Sprites
    GnashCharacter* getCharacterById(int id);    
    GnashCharacter* getTopmostMouseEntity();

    // Interaction
    void advanceClock(unsigned long ms);
    void advance();
    void render(bool forceRedraw);
    void restart();
    void setVerbosity(unsigned verbosity);
    
    geometry::SnappingRanges2d<int> getInvalidatedRanges() const;
    
    // Move the pointer to position x, y.
    // @ return whether the move triggered an event needing a redraw. Use this
    // to decide whether to rerender.
    bool movePointer(int x, int y);
    
    // Send a mouse click event at the current position of the pointer.
    // @ return whether the move triggered an event needing a redraw. Use this
    // to decide whether to rerender.
    bool mouseClick();

    // @ return whether the keypress triggered an event needing a redraw. Use this
    // to decide whether to rerender.
    bool pressKey(int code);

    static std::string getLogMessage();
    static size_t logSize();
    
private:
    void init();
    void setBaseURL(const std::string& url);
    bool addRenderer(gnash::render_handler* handler);

    gnash::movie_definition* _movieDef;
    gnash::movie_root* _movieRoot;
    gnash::ManualClock _manualClock;
    gnash::render_handler* _renderer;

    gnash::InvalidatedRanges _invalidatedBounds;
 
    gnash::LogFile& _logFile;
    
    // The position of our pointer
    int _xpos, _ypos;

    // The base URL of the movie;
    std::string _url;

    static std::deque<std::string> _logMessages;

    static void receiveLogMessages(const std::string& s);

    std::auto_ptr<RunInfo> _runInfo;
};

class GnashCharacter
{
public:
    GnashCharacter();
    GnashCharacter(gnash::DisplayObject* c);
    ~GnashCharacter();

    // The only constant aspect of a character?
    int id() { return _character->get_id(); }

    const std::string& name() { return _character->get_name(); }

    //const char* textName() { return _character->get_text_name(); }

    std::string target() { return _character->getTarget(); }

    int ratio() { return _character->get_ratio(); }

    int depth() { return _character->get_depth(); }
    
    int clipDepth() { return _character->get_clip_depth(); }
    
    boost::uint32_t height() { return _character->get_height(); }
    
    boost::uint32_t width() { return _character->get_width(); }

    bool visible() { return _character->isVisible(); }
    
    void advance() { _character->advance(); }
    
    GnashCharacter* getParent();
    
private:
    gnash::DisplayObject*  _character;
};

}
}
// end namespace pythonwrapper
#endif
