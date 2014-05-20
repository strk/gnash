// gui.cpp:  Top level GUI for SWF player, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gui.h"

#include <vector>
#include <algorithm> 
#include <boost/assign/list_of.hpp>

#include "MovieClip.h"
#include "Renderer.h"
#include "sound_handler.h"
#include "movie_root.h"
#include "VM.h"
#include "DisplayObject.h"
#include "GnashEnums.h"
#include "RunResources.h"
#include "StreamProvider.h"
#include "ScreenShotter.h"
#include "Movie.h"

#ifdef GNASH_FPS_DEBUG
#include "ClockTime.h"
#include <boost/format.hpp>
#endif

/// Define this to make sure each frame is fully rendered from ground up
/// even if no motion has been detected in the movie.
//#define FORCE_REDRAW 1

/// Define this if you want to debug the *detection* of region updates only.
/// This will disable region updates for the backend (GUI+renderer) completely 
/// so that only the last region (red frame) will be visible. However, this 
/// slows down rendering as each frame is fully re-rendered. If you want to 
/// debug the GUI part, however (see if blitting the region works), then you 
/// probably won't define this.
#ifndef DISABLE_REGION_UPDATES_DEBUGGING 
//#define REGION_UPDATES_DEBUGGING_FULL_REDRAW 1
#endif 

#ifndef DISABLE_REGION_UPDATES_DEBUGGING
// a runtime check would make the { x; } block conditionally executed
#define IF_DEBUG_REGION_UPDATES(x) { if (_showUpdatedRegions) { x } }
#else
#define IF_DEBUG_REGION_UPDATES(x) 
#endif

// Define this to have gnash print the mouse pointer coordinates
// as the mouse moves. See also ENABLE_KEYBOARD_MOUSE_MOVEMENTS
// to have more control over mouse pointer.
//
//#define DEBUG_MOUSE_COORDINATES 1

namespace gnash {

struct Gui::Display
{
    Display(Gui& g, movie_root& r) : _g(g), _r(r) {}
    void operator()() const {
		InvalidatedRanges world_ranges;
		world_ranges.setWorld();
		_g.setInvalidatedRegions(world_ranges);
        _g.display(&_r);
    }
private:
    Gui& _g;
    movie_root& _r;
};

Gui::Gui(RunResources& r) :
    _loop(true),
    _xid(0),
    _width(1),
    _height(1),
    _runResources(r),
    _interval(0),
    _redraw_flag(true),
    _fullscreen(false),
    _mouseShown(true),
    _maxAdvances(0),
    _advances(0),
    _xscale(1.0f),
    _yscale(1.0f),
    _xoffset(0),
    _yoffset(0)
#ifdef GNASH_FPS_DEBUG
    ,fps_counter(0)
    ,fps_counter_total(0)
    ,fps_timer(0)
    ,fps_timer_interval(0.0)
    ,frames_dropped(0)
#endif
    ,_movieDef(0)
    ,_stage(0)
    ,_stopped(false)
    ,_started(false)
    ,_showUpdatedRegions(false)

    // NOTE: it's important that _systemClock is constructed
    //       before and destroyed after _virtualClock !
    ,_systemClock()
    ,_virtualClock(_systemClock)
#ifdef ENABLE_KEYBOARD_MOUSE_MOVEMENTS 
    ,_xpointer(0)
    ,_ypointer(0)
    ,_keyboardMouseMovements(true) // TODO: base default on gnashrc or always false and provide menu item to toggle
    ,_keyboardMouseMovementsStep(1)
#endif
{

}

Gui::Gui(unsigned long xid, float scale, bool loop, RunResources& r)
	:
    _loop(loop),
    _xid(xid),
    _width(1),
    _height(1),
    _runResources(r),
    _interval(0),
    _redraw_flag(true),
    _fullscreen(false),
    _mouseShown(true),
    _maxAdvances(0),
    _advances(0),
    _xscale(scale),
    _yscale(scale),
    _xoffset(0), // TODO: x and y offset will need update !
    _yoffset(0)
#ifdef GNASH_FPS_DEBUG
    ,fps_counter(0)    
    ,fps_counter_total(0)    
    ,fps_timer(0)
    ,fps_timer_interval(0.0)
    ,frames_dropped(0)
#endif        
    ,_movieDef(0)
    ,_stage(0)
    ,_stopped(false)
    ,_started(false)
    ,_showUpdatedRegions(false)

    // NOTE: it's important that _systemClock is constructed
    //       before and destroyed after _virtualClock !
    ,_systemClock()
    ,_virtualClock(_systemClock)
#ifdef ENABLE_KEYBOARD_MOUSE_MOVEMENTS 
    ,_xpointer(0)
    ,_ypointer(0)
    ,_keyboardMouseMovements(true) // TODO: base default on gnashrc or always false and provide menu item to toggle
    ,_keyboardMouseMovementsStep(1)
#endif
{
}

Gui::~Gui()
{
    if ( _movieDef.get() ) {
        log_debug("~Gui - _movieDef refcount: %d", _movieDef->get_ref_count());
    }

#ifdef GNASH_FPS_DEBUG
    if ( fps_timer_interval ) {
        std::cerr << "Total frame advances/drops: "
                  << fps_counter_total << "/" << frames_dropped << std::endl;
    }
#endif
}
    
void
Gui::setClipboard(const std::string&)
{
    LOG_ONCE(log_unimpl(_("Clipboard not yet supported in this GUI")));
}

void
Gui::setFullscreen()
{
    log_unimpl(_("Fullscreen not yet supported in this GUI"));
}

void
Gui::resizeWindow(int /*width*/, int /*height*/)
{
    log_unimpl(_("Window resize not yet supported in this GUI"));
}

void
Gui::unsetFullscreen()
{
    log_unimpl(_("Fullscreen not yet supported in this GUI"));
}

void
Gui::quit()
{
    // Take a screenshot of the last frame if required.
    if (_screenShotter.get() && _renderer.get()) {
        Display dis(*this, *_stage);
        _screenShotter->last(*_renderer, &dis);
    }
    
    quitUI();
}

void
Gui::hideMenu()
{
    LOG_ONCE(log_unimpl(_("Menu show/hide not yet supported in this GUI")));
}

bool
Gui::showMouse(bool /* show */)
{
    LOG_ONCE(log_unimpl(_("Mouse show/hide not yet supported in this GUI")));
    return true;
}

void
Gui::showMenu(bool /* show */)
{
    LOG_ONCE(log_unimpl(_("Menu show/hide not yet supported in this GUI")));
}

void
Gui::allowScale(bool allow)
{
    if (!_stage) {
        log_error("Gui::allowScale called before a movie_root was available");
        return;
    }
    
    if (allow) _stage->setStageScaleMode(movie_root::SCALEMODE_SHOWALL);
    else _stage->setStageScaleMode(movie_root::SCALEMODE_NOSCALE);
}

void
Gui::toggleFullscreen()
{
    /// Sends request to Gnash core to change display state.
    if (_fullscreen) {
        _stage->setStageDisplayState(movie_root::DISPLAYSTATE_NORMAL);
    } else {
        _stage->setStageDisplayState(movie_root::DISPLAYSTATE_FULLSCREEN);
    } 
}

void
Gui::restart()
{
    _stage->reset();
    _started = false;
    start();
}

void
Gui::updateStageMatrix()
{
    if (!_stage) {
        // When VM initializes, we'll get a call to resize_view, which
        // would call us again.
        log_error(_("Can't update stage matrix till VM is initialized"));
        return;
    }
    
    assert(_stage); // when VM is initialized this should hold
    
    float swfwidth = _movieDef->get_width_pixels();
    float swfheight = _movieDef->get_height_pixels();
    
    // Fetch scale mode
    movie_root::ScaleMode scaleMode = _stage->getStageScaleMode();

    switch (scaleMode) {
        case movie_root::SCALEMODE_NOSCALE:
            _xscale = _yscale = 1.0f;
            break;
        
        case movie_root::SCALEMODE_SHOWALL:
            // set new scale value ( user-pixel / pseudo-pixel ). Do
            // not divide by zero, or we end up with an invalid
            // stage matrix that returns nan values.			
            _xscale = (swfwidth == 0.0f) ? 1.0f : _width / swfwidth;
            _yscale = (swfheight == 0.0f) ? 1.0f : _height / swfheight;

            // Scale proportionally, using smallest scale
            if (_xscale < _yscale) {
                _yscale = _xscale;
            } else if (_yscale < _xscale) {
                _xscale = _yscale;
            }
            break;

        case movie_root::SCALEMODE_NOBORDER:
            // set new scale value ( user-pixel / pseudo-pixel )
            _xscale = (swfwidth == 0.0f) ? 1.0f : _width / swfwidth;
            _yscale = (swfheight == 0.0f) ? 1.0f : _height / swfheight;
            
            // Scale proportionally, using biggest scale
            if (_xscale > _yscale) {
                _yscale = _xscale;
            } else if (_yscale > _xscale) {
                _xscale = _yscale;
            }
            break;
        
        case movie_root::SCALEMODE_EXACTFIT:
            // NOTE: changing aspect ratio is valid!
            _xscale = (swfwidth == 0.0f) ? 1.0f : _width / swfwidth;
            _yscale = (swfheight == 0.0f) ? 1.0f : _height / swfheight;
            break;
        
        default:
            log_error(_("Invalid scaleMode %d"), scaleMode);
            break;
    }
    
    _xoffset=0;
    _yoffset=0;
    
    // Fetch align mode
    movie_root::StageAlign align = _stage->getStageAlignment();
    movie_root::StageHorizontalAlign halign = align.first;
    movie_root::StageVerticalAlign valign = align.second;
    
    // Handle horizontal alignment
    switch ( halign ) {
      case movie_root::STAGE_H_ALIGN_L:
      {
          // _xoffset=0 is fine
          break;
      }
      
      case movie_root::STAGE_H_ALIGN_R:
      {
          // Offsets in pixels
          float defWidth = swfwidth *= _xscale;
          float diffWidth = _width-defWidth;
          _xoffset = diffWidth;
          break;
      }
        
      case movie_root::STAGE_V_ALIGN_C:
      {
          // Offsets in pixels
          float defWidth = swfwidth *= _xscale;
          float diffWidth = _width-defWidth;
          _xoffset = diffWidth/2.0;
          break;
      }
        
      default:
      {
          log_error(_("Invalid horizontal align %d"), valign);
          break;
      }
    }
    
    // Handle vertical alignment
    switch ( valign ) {
      case movie_root::STAGE_V_ALIGN_T:
      {
          // _yoffset=0 is fine
          break;
      }
      
      case movie_root::STAGE_V_ALIGN_B:
      {
          float defHeight = swfheight *= _yscale;
          float diffHeight = _height-defHeight;
          _yoffset = diffHeight;
          break;
      }
        
      case movie_root::STAGE_V_ALIGN_C:
      {
          float defHeight = swfheight *= _yscale;
          float diffHeight = _height-defHeight;
          _yoffset = diffHeight/2.0;
          break;
      }
        
      default:
      {
          log_error(_("Invalid vertical align %d"), valign);
          break;
      }
    }
    
    //log_debug("updateStageMatrix: scaleMode:%d, valign:%d, halign:%d",
    //scaleMode, valign, halign);
    
    // TODO: have a generic set_matrix ?
    if (_renderer.get()) {
        _renderer->set_scale(_xscale, _yscale);
        _renderer->set_translation(_xoffset, _yoffset);
    } else {
        //log_debug("updateStageMatrix: could not signal updated stage
        //matrix to renderer (no renderer registered)");
    }
    
    // trigger redraw
    //_redraw_flag |= (_width!=width) || (_height!=height);
    _redraw_flag = true; // this fixes bug #21971
}


void
Gui::resize_view(int width, int height)
{
    GNASH_REPORT_FUNCTION;
    
    assert(width > 0);
    assert(height > 0);

    if (_stage && _started) {
        _stage->setDimensions(width, height);
    }

    _width = width;
    _height = height;
    _validbounds.setTo(0, 0, _width, _height);
    
    updateStageMatrix();
    
    if ( _stage && _started ) {
        display(_stage);
    }
}


void
Gui::toggleSound()
{
    assert (_stage);
    // @todo since we registered the sound handler, shouldn't we know
    //       already what it is ?!
#ifdef USE_SOUND
    sound::sound_handler* s = _stage->runResources().soundHandler();
    
    if (!s) return;

    if (s->is_muted()) s->unmute();
    else s->mute();
#endif  // USE_SOUND
}

void
Gui::notifyMouseMove(int ux, int uy) 
{
    movie_root* m = _stage;
    
    if ( ! _started ) return;
    
    if ( _stopped ) return;
    
    // A stage pseudopixel is user pixel / _xscale wide
    std::int32_t x = (ux-_xoffset) / _xscale;
    
    // A stage pseudopixel is user pixel / _xscale high
    std::int32_t y = (uy-_yoffset) / _yscale;
    
#ifdef DEBUG_MOUSE_COORDINATES
    log_debug("mouse @ %d,%d", x, y);
#endif
    
    if ( m->mouseMoved(x, y) ) {
        // any action triggered by the
        // event required screen refresh
        display(m);
    }
    
    DisplayObject* activeEntity = m->getActiveEntityUnderPointer();
    if ( activeEntity ) {
        if ( activeEntity->isSelectableTextField() ) {
            setCursor(CURSOR_INPUT);
        } else if ( activeEntity->allowHandCursor() ) {
            setCursor(CURSOR_HAND);
        } else {
            setCursor(CURSOR_NORMAL);
        }
    } else {
        setCursor(CURSOR_NORMAL);
    }
    
#ifdef ENABLE_KEYBOARD_MOUSE_MOVEMENTS
    _xpointer = ux;
    _ypointer = uy;
#endif


}

void
Gui::notifyMouseWheel(int delta)
{
    movie_root* m = _stage;
    assert(m);

    if (!_started) return;
    if (_stopped) return;

    if (m->mouseWheel(delta)) {
        // any action triggered by the
        // event required screen refresh
        display(m);
    }
} 

void
Gui::notifyMouseClick(bool mouse_pressed) 
{
    movie_root* m = _stage;
    assert(m);

    if (!_started) return;
    if (_stopped) return;

    if (m->mouseClick(mouse_pressed)) {
        // any action triggered by the
        // event required screen refresh
        display(m);
    }
}

void
Gui::refreshView()
{
    movie_root* m = _stage;

    if ( ! _started ) return;

    assert(m);
    _redraw_flag=true;
    display(m);
}


void
Gui::notify_key_event(gnash::key::code k, int modifier, bool pressed) 
{

    // Handle GUI shortcuts
    if (pressed) {
        if (k == gnash::key::ESCAPE) {
            if (isFullscreen()) {
                _stage->setStageDisplayState(movie_root::DISPLAYSTATE_NORMAL);
            }
        }
	
        if (modifier & gnash::key::GNASH_MOD_CONTROL) {
            switch (k) {
              case gnash::key::o:
              case gnash::key::O:
                  takeScreenShot();
                  break;
              case gnash::key::r:
              case gnash::key::R:
                  restart();
                  break;
              case gnash::key::p:
              case gnash::key::P:
                  pause();
                  break;
              case gnash::key::l:
              case gnash::key::L:
                  refreshView();
                  break;
              case gnash::key::q:
              case gnash::key::Q:
              case gnash::key::w:
              case gnash::key::W:
                  quit();
                  break;
              case gnash::key::f:
              case gnash::key::F:
                  toggleFullscreen();
                  break;
              case gnash::key::h:
              case gnash::key::H:
                  showUpdatedRegions(!showUpdatedRegions());
                  break;
              case gnash::key::MINUS:
              {
                  // Max interval allowed: 1 second (1FPS)
                  const size_t ni = std::min<size_t>(_interval + 2, 1000u);
                  setInterval(ni);
                  break;
              }
              case gnash::key::PLUS:
              {
                  // Min interval allowed: 1/100 second (100FPS)
                  const size_t ni = std::max<size_t>(_interval - 2, 10u);
                  setInterval(ni);
                  break;
              }
              case gnash::key::EQUALS:
              {
                  if (_stage) {
                      const float fps = _stage->getRootMovie().frameRate();
                      // Min interval allowed: 1/100 second (100FPS)
                      const size_t ni = 1000.0/fps;
                      setInterval(ni);
                  }
                  break;
              }
              default:
                  break;
            }
            
#ifdef ENABLE_KEYBOARD_MOUSE_MOVEMENTS
            if ( _keyboardMouseMovements ) {
                int step = _keyboardMouseMovementsStep; 
                // x5 if SHIFT is pressed
                if (modifier & gnash::key::GNASH_MOD_SHIFT) step *= 5; 
                switch (k) {
                  case gnash::key::UP:
                  {
                      int newx = _xpointer;
                      int newy = _ypointer-step;
                      if ( newy < 0 ) newy=0;
                      notifyMouseMove(newx, newy);
                      break;
                  }
                  case gnash::key::DOWN:
                  {
                      int newx = _xpointer;
                      int newy = _ypointer+step;
                      if ( newy >= _height ) newy = _height-1;
                      notifyMouseMove(newx, newy);
                      break;
                  }
                  case gnash::key::LEFT:
                  {
                      int newx = _xpointer-step;
                      int newy = _ypointer;
                      if ( newx < 0 ) newx = 0;
                      notifyMouseMove(newx, newy);
                      break;
                  }
                  case gnash::key::RIGHT:
                  {
                      const int newy = _ypointer;
                      int newx = _xpointer + step;
                      if ( newx >= _width ) newx = _width-1;
                      notifyMouseMove(newx, newy);
                      break;
                  }
                  default:
                      break;
                }
            }
#endif // ENABLE_KEYBOARD_MOUSE_MOVEMENTS
        }
    }
    
    if (!_started) return;
    
    if (_stopped) return;
    
    if (_stage->keyEvent(k, pressed)) {
        // any action triggered by the
        // event required screen refresh
        display(_stage);
    }
    
}

bool
Gui::display(movie_root* m)
{
    assert(m == _stage); // why taking this arg ??

    assert(_started);
    
    InvalidatedRanges changed_ranges;
    bool redraw_flag;
    
    // Should the frame be rendered completely, even if it did not change?
#ifdef FORCE_REDRAW
    redraw_flag = true;
#else	
    redraw_flag = _redraw_flag || want_redraw();
#endif	
    
    // reset class member if we do a redraw now
    if (redraw_flag) _redraw_flag=false;
    
    // Find out the surrounding frame of all characters which
    // have been updated. This just checks what region of the stage has changed
    // due to ActionScript code, the timeline or user events. The GUI can still
    // choose to render a different part of the stage. 
    //
    if (!redraw_flag) {
        
        // choose snapping ranges factor 
        changed_ranges.setSnapFactor(1.3f);  
	
        // Use multi ranges only when GUI/Renderer supports it
        // (Useless CPU overhead, otherwise)
        changed_ranges.setSingleMode(!want_multiple_regions());
        
        // scan through all sprites to compute invalidated bounds  
        m->add_invalidated_bounds(changed_ranges, false);
	
        // grow ranges by a 2 pixels to avoid anti-aliasing issues		
        changed_ranges.growBy(40.0f / _xscale);
	
        // optimize ranges
        changed_ranges.combineRanges();	
    }
    
    // TODO: Remove this and want_redraw to avoid confusion!?
    if (redraw_flag)  {
        changed_ranges.setWorld();
    }
    
    // DEBUG ONLY:
    // This is a good place to inspect the invalidated bounds state. Enable
    // the following block (and parts of it) if you need to. 
#if 0
    {
        // This may print a huge amount of information, but is useful to analyze
        // the (visible) object structure of the movie and the flags of the
        // characters. For example, a characters should have set the 
        // m_child_invalidated flag if at least one of it's childs has the
        // invalidated flag set.
        log_debug("DUMPING CHARACTER TREE"); 
        
        InfoTree tr;
        InfoTree::iterator top = tr.begin();
        _stage->getMovieInfo(tr, top);
        
        for (InfoTree::iterator i = tr.begin(), e = tr.end();
             i != e; ++i) {
            std::cout << std::string(tr.depth(i) * 2, ' ') << i->first << ": " << 
                i->second << std::endl;
        }
        
        
        // less verbose, and often necessary: see the exact coordinates of the
        // invalidated bounds (mainly to see if it's NULL or something else).	
        std::cout << "Calculated changed ranges: " << changed_ranges << "\n";
    }
#endif
    
    // Avoid drawing of stopped movies
    if ( ! changed_ranges.isNull() ) { // use 'else'?
        // Tell the GUI(!) that we only need to update this
        // region. Note the GUI can do whatever it wants with
        // this information. It may simply ignore the bounds
        // (which will normally lead into a complete redraw),
        // or it may extend or shrink the bounds as it likes. So,
        // by calling set_invalidated_bounds we have no guarantee
        // that only this part of the stage is rendered again.
#ifdef REGION_UPDATES_DEBUGGING_FULL_REDRAW
        // redraw the full screen so that only the
        // *new* invalidated region is visible
        // (helps debugging)
        InvalidatedRanges world_ranges;
        world_ranges.setWorld();
        setInvalidatedRegions(world_ranges);
#else
        setInvalidatedRegions(changed_ranges);
#endif
        
        // TODO: should this be called even if we're late ?
        beforeRendering();
        
        // Render the frame, if not late.
        // It's up to the GUI/renderer combination
        // to do any clipping, if desired.     
        m->display();
        
        // show invalidated region using a red rectangle
        // (Flash debug style)
        IF_DEBUG_REGION_UPDATES (
            if (_renderer.get() && !changed_ranges.isWorld()) {
                for (size_t rno = 0; rno < changed_ranges.size(); rno++) {
                    const geometry::Range2d<int>& bounds = 
                        changed_ranges.getRange(rno);
                    
                    float xmin = bounds.getMinX();
                    float xmax = bounds.getMaxX();
                    float ymin = bounds.getMinY();
                    float ymax = bounds.getMaxY();
                    
                    const std::vector<point> box = boost::assign::list_of
                        (point(xmin, ymin))
                        (point(xmax, ymin))
                        (point(xmax, ymax))
                        (point(xmin, ymax));
                    
                    _renderer->draw_poly(box, rgba(0,0,0,0), rgba(255,0,0,255),
                                         SWFMatrix(), false);
                    
                }
            }
            );
        
        // show frame on screen
        renderBuffer();	
    };
    
    return true;
}

void
Gui::play()
{
    if ( ! _stopped ) return;
    
    _stopped = false;
    if ( ! _started ) {
        start();
    } else {
        assert (_stage);
#ifdef USE_SOUND
        // @todo since we registered the sound handler, shouldn't we know
        //       already what it is ?!
        sound::sound_handler* s = _stage->runResources().soundHandler();
        if ( s ) s->unpause();
#endif  // USE_SOUND
        // log_debug("Starting virtual clock");
        _virtualClock.resume();
    }

    playHook ();
}

void
Gui::stop()
{
    // _stage must be registered before this is called.
    assert(_stage);

    if ( _stopped ) return;
    if ( isFullscreen() ) unsetFullscreen();

    _stopped = true;

    // @todo since we registered the sound handler, shouldn't we know
    //       already what it is ?!
    sound::sound_handler* s = _stage->runResources().soundHandler();
    if ( s ) s->pause();

    // log_debug("Pausing virtual clock");
    _virtualClock.pause();

    stopHook();
}

void
Gui::pause()
{
    if (_stopped) {
        play();
        return;
    }

    // TODO: call stop() instead ?
    // The only thing I see is that ::stop exits full-screen,
    // but I'm not sure that's intended behaviour

    // @todo since we registered the sound handler, shouldn't we know
    //       already what it is ?!
    sound::sound_handler* s = _stage->runResources().soundHandler();
    if (s) s->pause();
    _stopped = true;

    // log_debug("Pausing virtual clock");
    _virtualClock.pause();

    stopHook();
}

void
Gui::start()
{
    assert ( ! _started );
    if (_stopped) {
        log_error(_("GUI is in stop mode, won't start application"));
        return;
    }

    // Initializes the stage with a Movie and the passed flash vars.
    _stage->init(_movieDef.get(), _flashVars);

    bool background = true; // ??
    _stage->set_background_alpha(background ? 1.0f : 0.05f);

    // to properly update stageMatrix if scaling is given  
    resize_view(_width, _height); 

    // @todo since we registered the sound handler, shouldn't we know
    //       already what it is ?!
#ifdef USE_SOUND
    sound::sound_handler* s = _stage->runResources().soundHandler();
    if ( s ) {
        if ( ! _audioDump.empty() ) {
            s->setAudioDump(_audioDump);
        }
        s->unpause();
    }
#endif  // USE_SOUND
    _started = true;
    
    // log_debug("Starting virtual clock");
    _virtualClock.resume();

}

bool
Gui::advanceMovie(bool doDisplay)
{
    if (isStopped()) {
        return false;
    }

    if (!_started) {
        start();
    }

    Display dis(*this, *_stage);
    gnash::movie_root* m = _stage;
    
    // Define REVIEW_ALL_FRAMES to have *all* frames
    // consequentially displayed. Useful for debugging.
    //#define REVIEW_ALL_FRAMES 1
    
#ifndef REVIEW_ALL_FRAMES
    // Advance movie by one frame
    const bool advanced = m->advance();
#else
    const size_t cur_frame = m->getRootMovie().get_current_frame();
    const size_t tot_frames = m->getRootMovie().get_frame_count();
    const bool advanced = m->advance();
    
    m->getRootMovie().ensureFrameLoaded(tot_frames);
    m->goto_frame(cur_frame + 1);
    m->getRootMovie().setPlayState(gnash::MovieClip::PLAYSTATE_PLAY);
    // log_debug("Frame %d", m->getRootMovie().get_current_frame());
#endif
    
#ifdef GNASH_FPS_DEBUG
    // will be a no-op if fps_timer_interval is zero
    if (advanced) {
        fpsCounterTick();
    }
#endif
    
    if (doDisplay && visible()) {
        display(m);
    }
    
    if (!loops()) {
        // can be 0 on malformed SWF
        const size_t curframe = m->getRootMovie().get_current_frame(); 
        const MovieClip& si = m->getRootMovie();
        if (curframe + 1 >= si.get_frame_count()) {
            quit(); 
        }
    }
    
    if (_screenShotter.get() && _renderer.get()) {
        _screenShotter->screenShot(*_renderer, _advances, doDisplay ? 0 : &dis);
    }
    
    // Only increment advances and check for exit condition when we've
    // really changed frame.
    if (advanced) {
        /// Quit if we've reached the frame advance limit.
        if (_maxAdvances && (_advances > _maxAdvances)) {
            quit();
        }
        ++_advances;
    }

	return advanced;
}

void
Gui::setScreenShotter(std::unique_ptr<ScreenShotter> ss)
{
    _screenShotter.reset(ss.release());
}

void
Gui::takeScreenShot()
{
    if (!_screenShotter.get()) {
        // If no ScreenShotter exists, none was requested at startup.
        // We use a default filename pattern.
        URL url(_runResources.streamProvider().baseURL());
        std::string::size_type p = url.path().rfind('/');
        const std::string& name = (p == std::string::npos) ? url.path() :
            url.path().substr(p + 1);
        const std::string& filename = "screenshot-" + name + "-%f";
        _screenShotter.reset(new ScreenShotter(filename, GNASH_FILETYPE_PNG));
    }
    assert (_screenShotter.get());
    _screenShotter->now();
}

void
Gui::setCursor(gnash_cursor_type /*newcursor*/)
{
    /* do nothing */
}

bool
Gui::want_redraw()
{
    return false;
}

void
Gui::setInvalidatedRegion(const SWFRect& /*bounds*/)
{
    /* do nothing */
}

void
Gui::setInvalidatedRegions(const InvalidatedRanges& ranges)
{
    // fallback to single regions
    geometry::Range2d<int> full = ranges.getFullArea();
    
    SWFRect bounds;
    
    if (full.isFinite()) {
        bounds = SWFRect(full.getMinX(), full.getMinY(),
                full.getMaxX(), full.getMaxY());
    }
    else if (full.isWorld()) {
        bounds.set_world();
    }
    
    setInvalidatedRegion(bounds);
}

#ifdef USE_SWFTREE

std::unique_ptr<movie_root::InfoTree>
Gui::getMovieInfo() const
{
    std::unique_ptr<movie_root::InfoTree> tr;

    if (!_stage) {
        return tr;
    }

    tr.reset(new movie_root::InfoTree());

    // Top nodes for the tree:
    // 1. VM information
    // 2. "Stage" information
    // 3. ...

    movie_root::InfoTree::iterator topIter = tr->begin();
    movie_root::InfoTree::iterator firstLevelIter;

    VM& vm = _stage->getVM();

    std::ostringstream os;

    //
    /// VM top level
    //
    os << "SWF " << vm.getSWFVersion();
    topIter = tr->insert(topIter, std::make_pair("Root SWF version", os.str()));

    // This short-cut is to avoid a bug in movie_root's getMovieInfo,
    // which relies on the availability of a _rootMovie for doing
    // it's work, while we don't set it if we didn't start..
    // 
    if (! _started) {
        topIter = tr->insert(topIter, std::make_pair("Stage properties", 
                    "not constructed yet"));
        return tr;
    }

    movie_root& stage = vm.getRoot();
    stage.getMovieInfo(*tr, topIter);

    //
    /// Mouse entities
    //
    topIter = tr->insert(topIter, std::make_pair("Mouse Entities", ""));

    const DisplayObject* ch;
    ch = stage.getActiveEntityUnderPointer();
    if (ch) {
        std::stringstream ss;
        ss << ch->getTarget() << " (" + typeName(*ch)
           << " - depth:" << ch->get_depth()
           << " - useHandCursor:" << ch->allowHandCursor()
           << ")";
    	firstLevelIter = tr->append_child(topIter, 
                std::make_pair("Active entity under mouse pointer", ss.str()));
    }

    ch = stage.getEntityUnderPointer();
    if (ch) {
        std::stringstream ss;
        ss << ch->getTarget() << " (" + typeName(*ch) 
           << " - depth:" << ch->get_depth()
           << ")";
        firstLevelIter = tr->append_child(topIter, 
            std::make_pair("Topmost entity under mouse pointer", ss.str()));
    }
    
    ch = stage.getDraggingCharacter();
    if (ch) {
        std::stringstream ss;
        ss << ch->getTarget() << " (" + typeName(*ch) 
           << " - depth:" << ch->get_depth() << ")";
    	firstLevelIter = tr->append_child(topIter,
                std::make_pair("Dragging character: ", ss.str()));
    }

    //
    /// GC row
    //
    topIter = tr->insert(topIter, std::make_pair("GC Statistics", ""));
    GC::CollectablesCount cc;
    _stage->gc().countCollectables(cc);
    
    const std::string lbl = "GC managed ";
    for (GC::CollectablesCount::iterator i=cc.begin(), e=cc.end(); i!=e; ++i) {
        const std::string& typ = i->first;
        std::ostringstream ss;
        ss << i->second;
        firstLevelIter = tr->append_child(topIter,
                    std::make_pair(lbl + typ, ss.str()));
    }

    tr->sort(firstLevelIter.begin(), firstLevelIter.end());

    return tr;
}

#endif

#ifdef GNASH_FPS_DEBUG
void 
Gui::fpsCounterTick()
{

  // increment this *before* the early return so that
  // frame count on exit is still valid
  ++fps_counter_total;

  if (! fps_timer_interval) {
      return;
  }

  std::uint64_t current_timer = clocktime::getTicks();

  // TODO: keep fps_timer_interval in milliseconds to avoid the multiplication
  //       at each fpsCounterTick call...
  std::uint64_t interval_ms = (std::uint64_t)(fps_timer_interval * 1000.0);

  if (fps_counter_total==1) {
    fps_timer = current_timer;
    fps_start_timer = current_timer;
  }
  
  ++fps_counter;
  
  if (current_timer - fps_timer >= interval_ms) {
  
    float secs = (current_timer - fps_timer) / 1000.0;
    float secs_total = (current_timer - fps_start_timer)/1000.0;
        
    float rate = fps_counter/secs;
    
    if (secs > 10000000) {
      // the timers are unsigned, so when the clock runs "backwards" it leads
      // to a very high difference value. In theory, this should never happen
      // with ticks, but it does on my machine (which may have a hw problem?).
      std::cerr << "Time glitch detected, need to restart FPS counters, sorry..." << std::endl;
      
      fps_timer = current_timer;
      fps_start_timer = current_timer;
      fps_counter_total = 0;
      fps_counter = 0;
      return;
    } 
     
    // first FPS message?
    if (fps_timer == fps_start_timer) {     // they're ints, so we can compare
      fps_rate_min = rate;
      fps_rate_max = rate; 
    } else {
      fps_rate_min = std::min<float>(fps_rate_min, rate);
      fps_rate_max = std::max<float>(fps_rate_max, rate);
    }
    
    float avg = fps_counter_total / secs_total; 
  
    //log_debug("Effective frame rate: %0.2f fps", (float)(fps_counter/secs));
    std::cerr << boost::format("Effective frame rate: %0.2f fps "
                               "(min %0.2f, avg %0.2f, max %0.2f, "
                               "%u frames in %0.1f secs total, "
                               "dropped %u)") % rate %
                               fps_rate_min % avg % fps_rate_max %
                               fps_counter_total % secs_total %
                               frames_dropped << std::endl;
      
    fps_counter = 0;
    fps_timer = current_timer;
    
  }
   
}
#endif

void
Gui::addFlashVars(Gui::VariableMap& from)
{
    for (VariableMap::iterator i=from.begin(), ie=from.end(); i!=ie; ++i) {
        _flashVars[i->first] = i->second;
    }
}

void
Gui::setMovieDefinition(movie_definition* md)
{
    assert(!_movieDef);
    _movieDef = md;
}

void
Gui::setStage(movie_root* stage)
{
    assert(stage);
    assert(!_stage);
    _stage = stage;
}

bool
Gui::yesno(const std::string& question)
{
    log_error(_("This GUI didn't override 'yesno', assuming 'yes' answer to "
                "question: %s"), question);
    return true;
}

void
Gui::setQuality(Quality q)
{
    if (!_stage) {
        log_error(_("Gui::setQuality called before a movie_root was available"));
        return;
    }
    _stage->setQuality(q);
}

Quality
Gui::getQuality() const
{
    if (!_stage) {
	log_error(_("Gui::getQuality called before a movie_root was available"));
	// just a guess..
	return QUALITY_HIGH;
    }
    return _stage->getQuality();
}

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
