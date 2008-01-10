// gnashpython.h: headers for the python bindings to Gnash
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
#ifndef GNASHPYTHON_H
#define GNASHPYTHON_H

#include "gnash.h"
#include "ManualClock.h"
#include "movie_root.h"
#include "movie_definition.h"
#include "render_handler.h"
#include "movie_instance.h" 
#include "character.h"

#include <string> 

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

    // Movie creation
    bool createMovieDefinition();
    bool initVM();
    void setBaseURL(std::string url);
    
    bool initRenderer(const std::string& r);
    bool addRenderer(gnash::render_handler* handler);

    // Movie information
    float getSWFFrameRate() const;
    int getSWFFrameCount() const;
    int getSWFVersion() const;
    float getSWFWidth() const;
    float getSWFHeight() const;
    int getSWFBytesTotal() const;
    int getSWFBytesLoaded() const;
    std::string getSWFURL() const;
    
    int getCurrentFrame() const;
    
    // Sprites
    
    GnashCharacter* getCharacterById(int id);    
    GnashCharacter* getTopmostMouseEntity();

    // Interaction
    void advanceClock(unsigned long ms);
    void advance();
    void pressKey(int code);
    void allowRescale(bool allow);
    void render();
    void restart();
    
private:
    void init();

    gnash::movie_definition* _movieDef;
    gnash::movie_root* _movieRoot;
    gnash::movie_instance* _movieInstance;
    gnash::ManualClock _manualClock;

    gnash::render_handler* _handler;

    // The base URL of the movie;
    std::string _url;
    bool _forceRedraw;
    gnash::InvalidatedRanges _invalidatedBounds;

    // File to open (a bit primitive...)    
    FILE* _fp;
};

class GnashCharacter
{
public:
    GnashCharacter();
    GnashCharacter(gnash::character* c);
    ~GnashCharacter();

    const std::string name() { return _character->get_name(); }
    const float ratio() { return _character->get_ratio(); }
    
private:
    gnash::character*  _character;
    
};

}
}
// end namespace pythonwrapper
#endif
