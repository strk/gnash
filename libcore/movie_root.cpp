// movie_root.cpp:  The root movie, for Gnash.
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

#include "movie_root.h"

#include <utility>
#include <string>
#include <sstream>
#include <map>
#include <bitset>
#include <cassert>
#include <functional>
#include <boost/algorithm/string/replace.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>

#include "GnashSystemIOHeaders.h" // write()
#include "log.h"
#include "MovieClip.h"
#include "Movie.h" 
#include "VM.h"
#include "ExecutableCode.h"
#include "URL.h"
#include "namedStrings.h"
#include "GnashException.h"
#include "sound_handler.h"
#include "Timers.h"
#include "GnashKey.h"
#include "GnashAlgorithm.h"
#include "GnashNumeric.h"
#include "Global_as.h"
#include "utf8.h"
#include "IOChannel.h"
#include "RunResources.h"
#include "Renderer.h"
#include "ExternalInterface.h"
#include "TextField.h"
#include "Button.h"
#include "Transform.h"
#include "StreamProvider.h"

#ifdef USE_SWFTREE
# include "tree.hh"
#endif

//#define GNASH_DEBUG 1
//#define GNASH_DEBUG_LOADMOVIE_REQUESTS_PROCESSING 1
//#define GNASH_DEBUG_TIMERS_EXPIRATION 1

// Defining the macro below prints info about
// cleanup of live chars (advanceable + key/mouse listeners)
// Is useful in particular to check for cost of multiple scans
// when a movie destruction destrois more elements.
//
// NOTE: I think the whole confusion here was introduced
//       by zou making it "optional" to ::unload() childs
//       when being unloaded. Zou was trying to avoid
//       queuing an onUnload event, which I suggested we'd
//       do by having unload() take an additional argument
//       or similar. Failing to tag childs as unloaded
//       will result in tagging them later (in ::destroy)
//       which will require scanning the lists again
//       (key/mouse + advanceable).
//       See https://savannah.gnu.org/bugs/index.php?21804
//
//#define GNASH_DEBUG_DLIST_CLEANUP 1

namespace gnash {

// Forward declarations
namespace {
    bool generate_mouse_button_events(movie_root& mr, MouseButtonState& ms);
    const DisplayObject* getNearestObject(const DisplayObject* o);
    as_object* getBuiltinObject(movie_root& mr, const ObjectURI& cl);
    void advanceLiveChar(MovieClip* ch);
}

// Utility classes
namespace {

/// Execute an ActiveRelay if the object has that type.
struct ExecuteCallback
{
    void operator()(const as_object* o) const {
        ActiveRelay* a;
        if (isNativeType(o, a)) {
            a->update();
        }
    }
};

/// Identify and delete ExecutableCode that matches a particular target.
class RemoveTargetCode
{
public:
    RemoveTargetCode(DisplayObject* target) : _target(target) {}
    bool operator()(const ExecutableCode& c) const {
        return _target == c.target();
    }
private:
    DisplayObject* _target;
};

void
clear(movie_root::ActionQueue& aq)
{
    std::for_each(aq.begin(), aq.end(), 
            boost::mem_fn(&movie_root::ActionQueue::value_type::clear));
}

} // anonymous namespace


movie_root::movie_root(VirtualClock& clock, const RunResources& runResources)
    :
    _gc(*this),
    _runResources(runResources),
    _vm(*this, clock),
    _interfaceHandler(0),
    _fsCommandHandler(0),
    _stageWidth(1),
    _stageHeight(1),
    m_background_color(255, 255, 255, 255),
    m_background_color_set(false),
    _mouseX(0),
    _mouseY(0),
    _lastTimerId(0),
    _lastKeyEvent(key::INVALID),
    _currentFocus(0),
    _movies(),
    _rootMovie(0),
    _invalidated(true),
    _disableScripts(false),
    _processingActionLevel(PRIORITY_SIZE),
    _hostfd(-1),
    _controlfd(-1),
    _quality(QUALITY_HIGH),
    _alignMode(0),
    _allowScriptAccess(SCRIPT_ACCESS_SAME_DOMAIN),
    _showMenu(true),
    _scaleMode(SCALEMODE_SHOWALL),
    _displayState(DISPLAYSTATE_NORMAL),
    _recursionLimit(), // set in ctor body
    _timeoutLimit(),   // set in ctor body
    _movieAdvancementDelay(83), // ~12 fps by default
    _lastMovieAdvancement(0),
    _unnamedInstance(0),
    _movieLoader(*this)
{
    // This takes care of informing the renderer (if present) too.
    setQuality(QUALITY_HIGH);

    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
    _recursionLimit = rcfile.getScriptsRecursionLimit();
    _timeoutLimit = rcfile.getScriptsTimeout();
}

void
movie_root::disableScripts()
{
    _disableScripts = true;

    // NOTE: we won't clear the action queue now
    // to avoid invalidating iterators as we've
    // been probably called during processing
    // of the queue.
}

movie_root::~movie_root()
{
    clear(_actionQueue);
    _intervalTimers.clear();
    _movieLoader.clear();

    assert(testInvariant());
}

Movie*
movie_root::init(movie_definition* def, const MovieClip::MovieVariables& vars)
{
    _vm.setSWFVersion(def->get_version());

    Movie* mr = def->createMovie(*_vm.getGlobal());
    mr->setVariables(vars);
    setRootMovie(mr);
    return mr;
}

void
movie_root::setRootMovie(Movie* movie)
{
    _rootMovie = movie;

    const movie_definition* md = movie->definition();
    float fps = md->get_frame_rate();
    _movieAdvancementDelay = static_cast<int>(1000/fps);

    _lastMovieAdvancement = _vm.getTime();

    _stageWidth = static_cast<int>(md->get_width_pixels());
    _stageHeight = static_cast<int>(md->get_height_pixels());

    // assert(movie->get_depth() == 0); ?
    movie->set_depth(DisplayObject::staticDepthOffset);

    try {
        setLevel(0, movie);

        // actions in first frame of _level0 must execute now,
        // before next advance,
        // or they'll be executed with _currentframe being set to 2
        processActionQueue();
    }
    catch (const ActionLimitException& al) {
        handleActionLimitHit(al.what());
    }
    catch (const ActionParserException& e) {
        log_error("ActionParserException thrown during setRootMovie: %s",
                e.what());
    }

    cleanupAndCollect();
}

bool
movie_root::abortOnScriptTimeout(const std::string& what) const
{
    std::string msg = what + std::string(". Disable scripts? ");

    bool disable = true;
    if (_interfaceHandler) {
        disable = callInterface<bool>(HostMessage(HostMessage::QUERY, msg));
    }
    else {
        log_error("No user interface registered, assuming 'Yes' answer to "
            "question: %s", msg);
    }
    return disable;
}
   
void
movie_root::registerClass(const SWF::DefinitionTag* sprite, as_function* cls)
{
    _registeredClasses[sprite] = cls;
}

as_function*
movie_root::getRegisteredClass(const SWF::DefinitionTag* sprite) const
{
    RegisteredClasses::const_iterator it = _registeredClasses.find(sprite);
    if (it == _registeredClasses.end()) return 0;
    return it->second;
}

void
movie_root::handleActionLimitHit(const std::string& msg)
{
    log_debug("Disabling scripts: %1%", msg);
    disableScripts();
    clear(_actionQueue);
}

void
movie_root::cleanupAndCollect()
{
    // Cleanup the stack.
    _vm.getStack().clear();

    // Reset the constant pool
    _vm.setConstantPool(0);

    cleanupDisplayList();
    _gc.fuzzyCollect();
}

/* private */
void
movie_root::setLevel(unsigned int num, Movie* movie)
{
    assert(movie != NULL);
    assert(static_cast<unsigned int>(movie->get_depth()) ==
                            num + DisplayObject::staticDepthOffset);


    Levels::iterator it = _movies.find(movie->get_depth());
    if (it == _movies.end()) {
        _movies[movie->get_depth()] = movie; 
    }
    else {
        // don't leak overloaded levels

        MovieClip* lm = it->second;
        if (lm == _rootMovie) {
            // NOTE: this is not enough to trigger
            //       an application reset. Was tested
            //       but not automated. If curious
            //       use swapDepths against _level0
            //       and load into the new target while
            //       a timeout/interval is active.
            log_debug("Replacing starting movie");
        }

        if (num == 0) {

            log_debug("Loading into _level0");

            // NOTE: this was tested but not automated, the
            //       test sets an interval and then loads something
            //       in _level0. The result is the interval is disabled.
            _intervalTimers.clear();

            // TODO: check what else we should do in these cases 
            //       (like, unregistering all childs etc...)
            //       Tested, but not automated, is that other
            //       levels should be maintained alive.
            // Sat Nov 14 10:31:19 CET 2009
            // ^^^ not confirmed in this date, I think other levels 
            //     are dropped too! (strk)

            _stageWidth = movie->widthPixels();
            _stageHeight = movie->heightPixels();

            // notify  stage replacement
            if (_interfaceHandler) {
                const HostMessage e(HostMessage::RESIZE_STAGE,
                        std::make_pair(_stageWidth, _stageHeight));
                _interfaceHandler->call(e);
            }
        }

        it->second->destroy();
        it->second = movie;
    }

    movie->set_invalidated();
    
    /// Notify placement 
    movie->construct();

    assert(testInvariant());
}

void
movie_root::swapLevels(MovieClip* movie, int depth)
{
    assert(movie);

//#define GNASH_DEBUG_LEVELS_SWAPPING 1

    const int oldDepth = movie->get_depth();

#ifdef GNASH_DEBUG_LEVELS_SWAPPING
    log_debug("Before swapLevels (source depth %d, target depth %d) "
            "levels are: ", oldDepth, depth);
    for (Levels::const_iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i) {
        log_debug(" %d: %p (%s @ depth %d)", i->first,
                (void*)(i->second), i->second->getTarget(),
                i->second->get_depth());
    }
#endif
    // should include _level0 !
    if (oldDepth < DisplayObject::staticDepthOffset) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s.swapDepth(%d): movie has a depth (%d) below "
                "static depth zone (%d), won't swap its depth"),
                movie->getTarget(), depth, oldDepth,
                DisplayObject::staticDepthOffset);
        );
        return;
    }

    if (oldDepth >= 0) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s.swapDepth(%d): movie has a depth (%d) below "
                "static depth zone (%d), won't swap its depth"),
                movie->getTarget(), depth, oldDepth,
                DisplayObject::staticDepthOffset);
        );
        return;
    }

    const int oldNum = oldDepth; 
    Levels::iterator oldIt = _movies.find(oldNum);
    if (oldIt == _movies.end()) {
        log_debug("%s.swapDepth(%d): target depth (%d) contains no movie",
            movie->getTarget(), depth, oldNum);
        return;
    }

    const int newNum = depth; 
    movie->set_depth(depth);
    Levels::iterator targetIt = _movies.find(newNum);
    if (targetIt == _movies.end()) {
        _movies.erase(oldIt);
        _movies[newNum] = movie;
    }
    else {
        MovieClip* otherMovie = targetIt->second;
        otherMovie->set_depth(oldDepth);
        oldIt->second = otherMovie;
        targetIt->second = movie;
    }
    
#ifdef GNASH_DEBUG_LEVELS_SWAPPING
    log_debug("After swapLevels levels are: ");
    for (Levels::const_iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i) {
        log_debug(" %d: %p (%s @ depth %d)", i->first, 
                (void*)(i->second), i->second->getTarget(),
                i->second->get_depth());
    }
#endif
    
    // TODO: invalidate self, not the movie
    //       movie_root::setInvalidated() seems
    //       to do just that, if anyone feels
    //       like more closely research on this
    //       (does level swapping require full redraw always?)
    movie->set_invalidated();
    
    assert(testInvariant());
}

void
movie_root::dropLevel(int depth)
{
    // should be checked by caller
    // TODO: don't use a magic number! See MovieClip::removeMovieClip().
    assert(depth >= 0 && depth <= 1048575);

    Levels::iterator it = _movies.find(depth);
    if (it == _movies.end()) {
        log_error("movie_root::dropLevel called against a movie not "
                "found in the levels container");
        return;
    }

    MovieClip* mo = it->second;
    if (mo == _rootMovie) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Original root movie can't be removed"));
        );
        return;
    }

    // TOCHECK: safe to erase here ?
    mo->unload();
    mo->destroy();
    _movies.erase(it);

    assert(testInvariant());
}

void
movie_root::replaceLevel(unsigned int num, Movie* extern_movie)
{
    extern_movie->set_depth(num + DisplayObject::staticDepthOffset);
    Levels::iterator it = _movies.find(extern_movie->get_depth());
    if (it == _movies.end()) {
        log_error("TESTME: loadMovie called on level %d which is not "
                "available at load time, skipped placement for now");
        return; 
    }

    // TODO: rework this to avoid the double scan 
    setLevel(num, extern_movie);
}

MovieClip*
movie_root::getLevel(unsigned int num) const
{
    Levels::const_iterator i =
        _movies.find(num + DisplayObject::staticDepthOffset);

    if (i == _movies.end()) return 0;

    return i->second;
}

void
movie_root::reset()
{
    sound::sound_handler* sh = _runResources.soundHandler();
    if (sh) sh->reset();

    // reset background color, to allow 
    // next load to set it again.
    m_background_color.set(255, 255, 255, 255);
    m_background_color_set = false;

    // wipe out live chars
    _liveChars.clear();

    // wipe out queued actions
    clear(_actionQueue);

    // wipe out all levels
    _movies.clear();

    // remove all intervals
    _intervalTimers.clear();

    // remove all loadMovie requests
    _movieLoader.clear();

    // Remove button key events.
    _buttonKeys.clear();

    // Cleanup the stack.
    _vm.getStack().clear();

    // Run the garbage collector again
    _gc.fuzzyCollect();

    setInvalidated();

    _disableScripts = false;
}

void
movie_root::setDimensions(size_t w, size_t h)
{
    assert(testInvariant());

    _stageWidth = w;
    _stageHeight = h;

    if (_scaleMode == SCALEMODE_NOSCALE) {
        as_object* stage = getBuiltinObject(*this,
            getURI(_vm, NSV::CLASS_STAGE));
        if (stage) {
            callMethod(stage, getURI(_vm, NSV::PROP_BROADCAST_MESSAGE),
                "onResize");
        }

    }

    assert(testInvariant());
}

bool
movie_root::mouseMoved(boost::int32_t x, boost::int32_t y)
{
    assert(testInvariant());

    _mouseX = x;
    _mouseY = y;
    return notify_mouse_listeners(event_id(event_id::MOUSE_MOVE));
}


bool
movie_root::keyEvent(key::code k, bool down)
{
    _lastKeyEvent = k;
    const size_t keycode = key::codeMap[k][key::KEY];
    if (keycode < key::KEYCOUNT) {
        _unreleasedKeys.set(keycode, down);
    }

    LiveChars copy = _liveChars;
    for (LiveChars::iterator iter = copy.begin(), itEnd=copy.end();
            iter != itEnd; ++iter) {

        MovieClip* const ch = *iter;
        if (ch->unloaded()) continue;

        if (down) {
            ch->notifyEvent(event_id(event_id::KEY_DOWN, key::INVALID)); 
            ch->notifyEvent(event_id(event_id::KEY_PRESS, k));
        }
        else {
            ch->notifyEvent(event_id(event_id::KEY_UP, key::INVALID));   
        }
    }

    // Broadcast event to Key._listeners.
    as_object* key = getBuiltinObject(*this, getURI(_vm, NSV::CLASS_KEY));
    if (key) {

        try {
            // Can throw an action limit exception if the stack limit is 0 or 1,
            // i.e. if the stack is at the limit before it contains anything.
            // A stack limit like that is hardly of any use, but could be used
            // maliciously to crash Gnash.
            if (down) {
                callMethod(key, getURI(_vm, NSV::PROP_BROADCAST_MESSAGE),
                        "onKeyDown");
            }
            else {
                callMethod(key, getURI(_vm,NSV::PROP_BROADCAST_MESSAGE),
                        "onKeyUp");
            }
        }
        catch (const ActionLimitException &e) {
            log_error(_("ActionLimits hit notifying key listeners: %s."),
                    e.what());
            clear(_actionQueue);
        }
    }
    
    if (down) {
        ButtonKeys::const_iterator it =
            _buttonKeys.find(key::codeMap[k][key::SWF]);

        // TODO: this searches through all ButtonActions for the correct
        // one, even though we could easily know which one it's going to be
        // because we search through them all to register the key codes.
        if (it != _buttonKeys.end()) {
            if (!it->second.first->unloaded()) {
                it->second.first->keyPress(k);
            }
        }

        // If we're focused on an editable text field, finally the text
        // is updated
        TextField* tf = dynamic_cast<TextField*>(_currentFocus);
        if (tf) tf->keyInput(k);
    }

    processActionQueue();

    return false; 
}

bool
movie_root::mouseWheel(int delta)
{
    as_object* mouseObj = 
        getBuiltinObject(*this, getURI(_vm, NSV::CLASS_MOUSE));
    if (!mouseObj) return false;
    
    const boost::int32_t x = pixelsToTwips(_mouseX);
    const boost::int32_t y = pixelsToTwips(_mouseY);

    DisplayObject* i = getTopmostMouseEntity(x, y);

    // Always called with two arguments.
    callMethod(mouseObj, getURI(_vm,NSV::PROP_BROADCAST_MESSAGE), "onMouseWheel",
            delta, i ? getObject(i) : as_value());

    return true;
}

bool
movie_root::mouseClick(bool mouse_pressed)
{
    assert(testInvariant());

    _mouseButtonState.isDown = mouse_pressed;

    if (mouse_pressed) {
        return notify_mouse_listeners(event_id(event_id::MOUSE_DOWN));
    }
    return notify_mouse_listeners(event_id(event_id::MOUSE_UP));
}


bool
movie_root::fire_mouse_event()
{
    assert(testInvariant());

    boost::int32_t x = pixelsToTwips(_mouseX);
    boost::int32_t y = pixelsToTwips(_mouseY);

    // Generate a mouse event
    _mouseButtonState.topmostEntity = getTopmostMouseEntity(x, y);

    // Set _droptarget if dragging a sprite
    DisplayObject* draggingChar = getDraggingCharacter();
    if (draggingChar) {
        MovieClip* dragging = draggingChar->to_movie();
        if (dragging) {
            // TODO: optimize making findDropTarget and getTopmostMouseEntity
            //       use a single scan.
            const DisplayObject* dropChar = findDropTarget(x, y, dragging);
            if (dropChar) {
                // Use target of closest script DisplayObject containing this
                dropChar = getNearestObject(dropChar);
                dragging->setDropTarget(dropChar->getTargetPath());
            }
            else dragging->setDropTarget("");
        }
    }

    bool need_redraw = false;

    // FIXME: need_redraw might also depend on actual
    //        actions execution (consider updateAfterEvent).

    try {
        need_redraw = generate_mouse_button_events(*this, _mouseButtonState);
        processActionQueue();
    }
    catch (const ActionLimitException& al) {
        handleActionLimitHit(al.what());
    }

    return need_redraw;

}

std::pair<boost::int32_t, boost::int32_t>
movie_root::mousePosition() const
{
    assert(testInvariant());
    return std::make_pair(_mouseX, _mouseY);
}

void
movie_root::setDragState(const DragState& st)
{
    _dragState = st;

    DisplayObject* ch = _dragState->getCharacter();

    if (ch && !_dragState->isLockCentered()) {
        // Get coordinates of the DisplayObject's origin
        point origin(0, 0);
        SWFMatrix chmat = getWorldMatrix(*ch);
        point world_origin;
        chmat.transform(&world_origin, origin);

        // Get current mouse coordinates
        const point world_mouse(pixelsToTwips(_mouseX), pixelsToTwips(_mouseY));

        boost::int32_t xoffset = world_mouse.x - world_origin.x;
        boost::int32_t yoffset = world_mouse.y - world_origin.y;

        _dragState->setOffset(xoffset, yoffset);
    }
    assert(testInvariant());
}

void
movie_root::doMouseDrag()
{
    DisplayObject* dragChar = getDraggingCharacter(); 
    if (!dragChar) return; // nothing to do

    if (dragChar->unloaded()) {
        // Reset drag state if dragging char was unloaded
        _dragState.reset();
        return; 
    }

    point world_mouse(pixelsToTwips(_mouseX), pixelsToTwips(_mouseY));

    SWFMatrix parent_world_mat;
    DisplayObject* p = dragChar->parent();
    if (p) {
        parent_world_mat = getWorldMatrix(*p);
    }

    if (!_dragState->isLockCentered()) {
        world_mouse.x -= _dragState->xOffset();
        world_mouse.y -= _dragState->yOffset();
    }

    if (_dragState->hasBounds()) {
        SWFRect bounds;
        // bounds are in local coordinate space
        bounds.enclose_transformed_rect(parent_world_mat,
                _dragState->getBounds());
        // Clamp mouse coords within a defined SWFRect.
        bounds.clamp(world_mouse);
    }

    parent_world_mat.invert().transform(world_mouse);            
    // Place our origin so that it coincides with the mouse coords
    // in our parent frame.
    // TODO: add a DisplayObject::set_translation ?
    SWFMatrix local = getMatrix(*dragChar);
    local.set_translation(world_mouse.x, world_mouse.y);
     
    // no need to update caches when only changing translation
    dragChar->setMatrix(local);
}

boost::uint32_t
movie_root::addIntervalTimer(std::auto_ptr<Timer> timer)
{
    assert(timer.get());
    assert(testInvariant());
            
    const size_t id = ++_lastTimerId;

    assert(_intervalTimers.find(id) == _intervalTimers.end());

    boost::shared_ptr<Timer> t(timer);
    _intervalTimers.insert(std::make_pair(id, t));

    return id;
}
    
bool
movie_root::clearIntervalTimer(boost::uint32_t x)
{
    TimerMap::iterator it = _intervalTimers.find(x);
    if (it == _intervalTimers.end()) return false;

    // We do not remove the element here because
    // we might have been called during execution
    // of another timer, thus during a scan of the _intervalTimers
    // container. If we use erase() here, the iterators in executeTimers
    // would be invalidated. Rather, executeTimers() would check container
    // elements for being still active and remove the cleared one in a safe way
    // at each iteration.
    it->second->clearInterval();

    return true;

}

bool
movie_root::advance()
{
    // We can't actually rely on now being later than _lastMovieAdvancement,
    // so we will have to check. Otherwise we risk elapsed being
    // contructed from a negative value.
    const size_t now = std::max<size_t>(_vm.getTime(), _lastMovieAdvancement);

    bool advanced = false;

    try {

        const size_t elapsed = now - _lastMovieAdvancement;
        if (elapsed >= _movieAdvancementDelay)
        {
            advanced = true;
            advanceMovie();

            // To catch-up lateness we pretend we advanced when 
            // was time for it. 
            // NOTE:
            //   now - _lastMovieAdvancement
            // gives you actual lateness in milliseconds
            //
            // TODO: make 'catchup' setting user-settable
            //       as it helps A/V sync but sacrifices 
            //       smoothness of animation which is very
            //       important for games.
            static const bool catchup = true;
            if (catchup) {
                _lastMovieAdvancement += _movieAdvancementDelay;
            } else {
                _lastMovieAdvancement = now;
            }

        }

        //log_debug("Lateness: %d", now-_lastMovieAdvancement);
        
        executeAdvanceCallbacks();
        
        executeTimers();
    
    }
    catch (const ActionLimitException& al) {
        // The PP does not disable scripts when the stack limit is reached,
        // but rather struggles on. 
        // TODO: find a test case for this, if confirmed fix accordingly
        handleActionLimitHit(al.what());
    }
    catch (const ActionParserException& e) {
        log_error(_("Buffer overread during advance: %s"), e.what());
        clear(_actionQueue);
    }

    return advanced;
}
    
void
movie_root::advanceMovie()
{

    // Do mouse drag, if needed
    doMouseDrag();

    // Advance all non-unloaded DisplayObjects in the LiveChars list
    // in reverse order (last added, first advanced)
    // NOTE: can throw ActionLimitException
    advanceLiveChars(); 

    // Process loadMovie requests
    // 
    // NOTE: should be done before executing timers,
    //      see swfdec's test/trace/loadmovie-case-{5,6}.swf 
    // NOTE: processing loadMovie requests after advanceLiveChars
    //       is known to fix more tests in misc-mtasc.all/levels.swf
    //       to be checked if it keeps the swfdec testsuite safe
    //
    _movieLoader.processCompletedRequests();

    // Process queued actions
    // NOTE: can throw ActionLimitException
    processActionQueue();

    cleanupAndCollect();

    assert(testInvariant());
}

int
movie_root::timeToNextFrame() const
{
    unsigned int now = _vm.getTime();
    const int elapsed = now - _lastMovieAdvancement;
    return _movieAdvancementDelay - elapsed;
}

void
movie_root::display()
{
    assert(testInvariant());

    clearInvalidated();

    // TODO: should we consider the union of all levels bounds ?
    const SWFRect& frame_size = _rootMovie->get_frame_size();
    if ( frame_size.is_null() )
    {
        // TODO: check what we should do if other levels
        //       have valid bounds
        log_debug("original root movie had null bounds, not displaying");
        return;
    }

    Renderer* renderer = _runResources.renderer();
    if (!renderer) return;

    Renderer::External ex(*renderer, m_background_color,
            _stageWidth, _stageHeight,
            frame_size.get_x_min(), frame_size.get_x_max(),
            frame_size.get_y_min(), frame_size.get_y_max());

    for (Levels::iterator i=_movies.begin(), e=_movies.end(); i!=e; ++i) {
        MovieClip* movie = i->second;

        movie->clear_invalidated();

        if (movie->visible() == false) continue;

        // null frame size ? don't display !
        const SWFRect& sub_frame_size = movie->get_frame_size();

        if (sub_frame_size.is_null()) {
            log_debug("_level%u has null frame size, skipping", i->first);
            continue;
        }

        movie->display(*renderer, Transform());

    }
}

bool
movie_root::notify_mouse_listeners(const event_id& event)
{

    LiveChars copy = _liveChars;
    for (LiveChars::iterator iter = copy.begin(), itEnd=copy.end();
            iter != itEnd; ++iter)
    {
        MovieClip* const ch = *iter;
        if (!ch->unloaded()) {
            ch->mouseEvent(event);
        }
    }

    const ObjectURI& propMouse = getURI(_vm, NSV::CLASS_MOUSE);
    const ObjectURI& propBroadcastMessage =
        getURI(_vm, NSV::PROP_BROADCAST_MESSAGE);

    as_object* mouseObj = getBuiltinObject(*this, propMouse);
    if (mouseObj) {

        // Can throw an action limit exception if the stack limit is 0 or 1.
        // A stack limit like that is hardly of any use, but could be used
        // maliciously to crash Gnash.
        try {
            callMethod(mouseObj, propBroadcastMessage, event.functionName());
        }
        catch (const ActionLimitException& e) {
            log_error(_("ActionLimits hit notifying mouse events: %s."),
                    e.what());
            clear(_actionQueue);
        }
    }

    assert(testInvariant());

    if (!copy.empty()) {
        // process actions queued in the above step
        processActionQueue();
    }
    return fire_mouse_event();
}

DisplayObject*
movie_root::getFocus()
{
    assert(testInvariant());
    return _currentFocus;
}

bool
movie_root::setFocus(DisplayObject* to)
{

    // Nothing to do if current focus is the same as the new focus. 
    // _level0 also seems unable to receive focus under any circumstances
    // TODO: what about _level1 etc ?
    if (to == _currentFocus ||
            to == static_cast<DisplayObject*>(_rootMovie)) {
        return false;
    }

    if (to && !to->handleFocus()) {
        // TODO: not clear whether to remove focus in this case.
        return false;
    }

    // Undefined or NULL DisplayObject removes current focus. Otherwise, try
    // setting focus to the new DisplayObject. If it fails, remove current
    // focus anyway.

    // Store previous focus, as the focus needs to change before onSetFocus
    // is called and listeners are notified.
    DisplayObject* from = _currentFocus;

    if (from) {
        // Perform any actions required on killing focus (only TextField).
        from->killFocus();

        /// A valid focus must have an associated object.
        assert(getObject(from));
        callMethod(getObject(from), NSV::PROP_ON_KILL_FOCUS, getObject(to));
    }

    _currentFocus = to;

    if (to) {
        assert(getObject(to));
        callMethod(getObject(to), NSV::PROP_ON_SET_FOCUS, getObject(from));
    }

    as_object* sel = getBuiltinObject(*this, NSV::CLASS_SELECTION);

    // Notify Selection listeners with previous and new focus as arguments.
    // Either argument may be null.
    if (sel) {
        callMethod(sel, NSV::PROP_BROADCAST_MESSAGE, "onSetFocus",
                getObject(from), getObject(to));
    }

    assert(testInvariant());

    return true;
}

DisplayObject*
movie_root::getActiveEntityUnderPointer() const
{
    return _mouseButtonState.activeEntity;
}

DisplayObject*
movie_root::getDraggingCharacter() const
{
    return _dragState ? _dragState->getCharacter() : 0;
}

const DisplayObject*
movie_root::getEntityUnderPointer() const
{
    const boost::int32_t x = pixelsToTwips(_mouseX);
    const boost::int32_t y = pixelsToTwips(_mouseY);
    return findDropTarget(x, y, getDraggingCharacter()); 
}


void
movie_root::setQuality(Quality q)
{
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();

    /// Overridden quality if not negative.
    if (rcfile.qualityLevel() >= 0) {
        int ql = rcfile.qualityLevel();
        ql = std::min<int>(ql, QUALITY_BEST);
        q = static_cast<Quality>(ql);
    }

    if ( _quality != q )
    {
        // Force a redraw if quality changes
        //
        // redraw should only happen on next
        // frame advancement (tested)
        //
        setInvalidated();

        _quality = q;
    }

    // We always tell the renderer, because it could
    // be the first time we do
    Renderer* renderer = _runResources.renderer();
    if (renderer) renderer->setQuality(_quality);

}

/// Get actionscript width of stage, in pixels. The width
/// returned depends on the scale mode.
size_t
movie_root::getStageWidth() const
{
    if (_scaleMode == SCALEMODE_NOSCALE) {
        return _stageWidth;    
    }

    // If scaling is allowed, always return the original movie size.
    if (_rootMovie) {
        return static_cast<size_t>(_rootMovie->widthPixels());
    } 
    return 0;
}

/// Get actionscript height of stage, in pixels. The height
/// returned depends on the scale mode.
size_t
movie_root::getStageHeight() const
{
    if (_scaleMode == SCALEMODE_NOSCALE) {
        return _stageHeight;    
    }

    // If scaling is allowed, always return the original movie size.
    if (_rootMovie) {
        return static_cast<size_t>(_rootMovie->heightPixels());
    } 
    return 0;
}

/// Takes a short int bitfield: the four bits correspond
/// to the AlignMode enum 
void
movie_root::setStageAlignment(short s)
{
    _alignMode = s;
    callInterface(HostMessage(HostMessage::UPDATE_STAGE));
}

/// The mode is one of never, always, with sameDomain the default
void
movie_root::setAllowScriptAccess(AllowScriptAccessMode mode)
{
    _allowScriptAccess = mode;
}

movie_root::AllowScriptAccessMode
movie_root::getAllowScriptAccess()
{
    return _allowScriptAccess;
}

/// Returns a pair of enum values giving the actual alignment
/// of the stage after align mode flags are evaluated.
movie_root::StageAlign
movie_root::getStageAlignment() const
{
    /// L takes precedence over R. Default is centred.
    StageHorizontalAlign ha = STAGE_H_ALIGN_C;
    if (_alignMode.test(STAGE_ALIGN_L)) ha = STAGE_H_ALIGN_L;
    else if (_alignMode.test(STAGE_ALIGN_R)) ha = STAGE_H_ALIGN_R;

    /// T takes precedence over B. Default is centred.
    StageVerticalAlign va = STAGE_V_ALIGN_C;
    if (_alignMode.test(STAGE_ALIGN_T)) va = STAGE_V_ALIGN_T;
    else if (_alignMode.test(STAGE_ALIGN_B)) va = STAGE_V_ALIGN_B;

    return std::make_pair(ha, va);
}

/// Returns a string that represents the boolean state of the _showMenu
/// variable
bool
movie_root::getShowMenuState() const
{
    return _showMenu;
}

/// Sets the value of _showMenu and calls the gui handler to process the 
/// fscommand to change the display of the context menu
void
movie_root::setShowMenuState(bool state)
{
    _showMenu = state;
    //FIXME: The gui code for show menu is semantically different than what
    //   ActionScript expects it to be. In gtk.cpp the showMenu function hides
    //   or shows the menubar. Flash expects this option to disable some 
    //   context menu items.
    // callInterface is the proper handler for this
    callInterface(HostMessage(HostMessage::SHOW_MENU, _showMenu)); 
}

/// Returns the string representation of the current align mode,
/// which must always be in the order: LTRB
std::string
movie_root::getStageAlignMode() const
{
    std::string align;
    if (_alignMode.test(STAGE_ALIGN_L)) align.push_back('L');
    if (_alignMode.test(STAGE_ALIGN_T)) align.push_back('T');
    if (_alignMode.test(STAGE_ALIGN_R)) align.push_back('R');
    if (_alignMode.test(STAGE_ALIGN_B)) align.push_back('B');
    
    return align;
}

void
movie_root::setStageScaleMode(ScaleMode sm)
{
    if (_scaleMode == sm) return; // nothing to do

    bool notifyResize = false;
    
    // If we go from or to noScale, we notify a resize
    // if and only if display viewport is != then actual
    // movie size. If there is not yet a _rootMovie (when scaleMode
    // is passed as a parameter to the player), we also don't notify a 
    // resize.
    if (_rootMovie && 
            (sm == SCALEMODE_NOSCALE || _scaleMode == SCALEMODE_NOSCALE)) {

        const movie_definition* md = _rootMovie->definition();
        log_debug("Going to or from scaleMode=noScale. Viewport:%dx%d "
                "Def:%dx%d", _stageWidth, _stageHeight,
                md->get_width_pixels(), md->get_height_pixels());

        if ( _stageWidth != md->get_width_pixels()
             || _stageHeight != md->get_height_pixels() )
        {
            notifyResize = true;
        }
    }

    _scaleMode = sm;
    callInterface(HostMessage(HostMessage::UPDATE_STAGE));

    if (notifyResize) {
        as_object* stage = getBuiltinObject(*this, NSV::CLASS_STAGE);
        if (stage) {
            callMethod(stage, NSV::PROP_BROADCAST_MESSAGE, "onResize");
        }
    }
}

void
movie_root::setStageDisplayState(const DisplayState ds)
{
    _displayState = ds;

    as_object* stage = getBuiltinObject(*this, NSV::CLASS_STAGE);
    if (stage) {
        const bool fs = _displayState == DISPLAYSTATE_FULLSCREEN;
        callMethod(stage, NSV::PROP_BROADCAST_MESSAGE, "onFullScreen", fs);
    }

    if (!_interfaceHandler) return; // No registered callback
    
    HostMessage e(HostMessage::SET_DISPLAYSTATE, _displayState);
    callInterface(e);
}

void
movie_root::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
    if (isInvalidated()) {
        ranges.setWorld();
        return;
    }

    for (Levels::reverse_iterator i=_movies.rbegin(), e=_movies.rend(); i!=e;
                        ++i) {
        i->second->add_invalidated_bounds(ranges, force);
    }

}

size_t
movie_root::minPopulatedPriorityQueue() const
{
    for (size_t l = 0; l < PRIORITY_SIZE; ++l) {
        if (!_actionQueue[l].empty()) return l;
    }
    return PRIORITY_SIZE;
}

size_t
movie_root::processActionQueue(size_t lvl)
{
    ActionQueue::value_type& q = _actionQueue[lvl];

    assert(minPopulatedPriorityQueue() == lvl);

#ifdef GNASH_DEBUG
    static unsigned calls=0;
    ++calls;
    bool actionsToProcess = !q.empty();
    if (actionsToProcess) {
        log_debug(" Processing %d actions in priority queue %d (call %u)",
                    q.size(), lvl, calls);
    }
#endif

    // _actionQueue may be changed due to actions (appended-to)
    // this loop might be optimized by using an iterator
    // and a final call to .clear() 
    while (!q.empty()) {

        std::auto_ptr<ExecutableCode> code(q.pop_front().release());
        code->execute();

        size_t minLevel = minPopulatedPriorityQueue();
        if (minLevel < lvl) {
#ifdef GNASH_DEBUG
            log_debug(" Actions pushed in priority %d (< "
                    "%d), restarting the scan (call"
                    " %u)", minLevel, lvl, calls);
#endif
            return minLevel;
        }
    }

    assert(q.empty());

#ifdef GNASH_DEBUG
    if (actionsToProcess) {
        log_debug(" Done processing actions in priority queue "
                "%d (call %u)", lvl, calls);
    }
#endif

    return minPopulatedPriorityQueue();
}

void
movie_root::flushHigherPriorityActionQueues()
{
    if (!processingActions()) {
        // only flush the actions queue when we are 
        // processing the queue.
        // ie. we don't want to flush the queue 
        // during executing user event handlers,
        // which are not pushed at the moment.
        return;
    }

    if (_disableScripts) {
        /// cleanup anything pushed later..
        clear(_actionQueue);
        return;
    }

    int lvl=minPopulatedPriorityQueue();
    while (lvl < _processingActionLevel) {
        lvl = processActionQueue(lvl);
    }

}

void
movie_root::addLoadableObject(as_object* obj, std::auto_ptr<IOChannel> str)
{
    boost::shared_ptr<IOChannel> io(str.release());
    _loadCallbacks.push_back(LoadCallback(io, obj));
}

void
movie_root::addAdvanceCallback(ActiveRelay* obj)
{
    _objectCallbacks.insert(obj);
}

void
movie_root::removeAdvanceCallback(ActiveRelay* obj)
{
    _objectCallbacks.erase(obj);
}

void
movie_root::processActionQueue()
{
    if (_disableScripts) {
        /// cleanup anything pushed later..
        clear(_actionQueue);
        return;
    }

    _processingActionLevel = minPopulatedPriorityQueue();

    while (_processingActionLevel < PRIORITY_SIZE) {
        _processingActionLevel = processActionQueue(_processingActionLevel);
    }

    // Cleanup the stack.
    _vm.getStack().clear();

}

void
movie_root::removeQueuedConstructor(DisplayObject* target)
{
    ActionQueue::value_type& pr = _actionQueue[PRIORITY_CONSTRUCT];
    pr.erase_if(RemoveTargetCode(target));
}

void
movie_root::pushAction(std::auto_ptr<ExecutableCode> code, size_t lvl)
{
    assert(lvl < PRIORITY_SIZE);
    _actionQueue[lvl].push_back(code);
}

void
movie_root::pushAction(const action_buffer& buf, DisplayObject* target)
{
#ifdef GNASH_DEBUG
    log_debug("Pushed action buffer for target %s", 
            target->getTargetPath());
#endif

    std::auto_ptr<ExecutableCode> code(new GlobalCode(buf, target));

    _actionQueue[PRIORITY_DOACTION].push_back(code);
}

void
movie_root::executeAdvanceCallbacks()
{

    if (!_objectCallbacks.empty()) {

        // We have two considerations:
        // 1. any update can change the active callbacks by removing or
        //    adding to the original callbacks list.
        // 2. Additionally, an as_object may destroy its own Relay. This can
        //    happen if the callback itself calls a native constructor on
        //    an object that already has a Relay. If this is an ActiveRelay
        //    registered with movie_root, a pointer to the destroyed object
        //    will still be held, resulting in memory corruption. This is an
        //    *extremely* unlikely case, but we are very careful!
        //
        // By copying to a new container we avoid errors caused by changes to
        // the original set (such as infinite recursions or invalidated
        // iterators). We also know that no as_object will be destroyed
        // during processing, even though its Relay may be.
        std::vector<as_object*> currentCallbacks;

        std::transform(_objectCallbacks.begin(), _objectCallbacks.end(),
            std::back_inserter(currentCallbacks),
            boost::bind(CreatePointer<as_object>(),
                boost::bind(std::mem_fun(&ActiveRelay::owner), _1)));

        std::for_each(currentCallbacks.begin(), currentCallbacks.end(),
                ExecuteCallback());
    }

    if (!_loadCallbacks.empty()) {
        _loadCallbacks.remove_if(
                std::mem_fun_ref(&movie_root::LoadCallback::processLoad));
    }

    // _controlfd is set when running as a child process of a hosting
    // application. If it is set, we have to check the socket connection
    // for XML messages.
    if (_controlfd) {
    boost::shared_ptr<ExternalInterface::invoke_t> invoke = 
        ExternalInterface::ExternalEventCheck(_controlfd);
        if (invoke) {
            if (processInvoke(invoke.get()) == false) {
                if (!invoke->name.empty()) {
                    log_error("Couldn't process ExternalInterface Call %s",
                          invoke->name);
                }
            }
        }    
    }
    
    processActionQueue();
}

bool
movie_root::processInvoke(ExternalInterface::invoke_t *invoke)
{
    GNASH_REPORT_FUNCTION;

    if (!invoke || invoke->name.empty()) return false;

    log_debug("Processing %s call from the Browser.", invoke->name);

    std::stringstream ss;       // ss is the response string

    // These are the default methods used by ExternalInterface
    if (invoke->name == "Quit") {
        // Leave to the hosting application. If there isn't one or it 
        // chooses not to exit, that's fine.
        if (_interfaceHandler) _interfaceHandler->exit();

    } else if (invoke->name == "SetVariable") {
        MovieClip *mc = getLevel(0);
        as_object *obj = getObject(mc);
        VM &vm = getVM();
        std::string var = invoke->args[0].to_string();
        as_value &val = invoke->args[1] ;
        obj->set_member(getURI(vm, var), val);
    // SetVariable doesn't send a response
    } else if (invoke->name == "GetVariable") {
        MovieClip *mc = getLevel(0);
        as_object *obj = getObject(mc);
        VM &vm = getVM();
        std::string var = invoke->args[0].to_string();
        as_value val;
        obj->get_member(getURI(vm, var), &val);
        // GetVariable sends the value of the variable
        ss << ExternalInterface::toXML(val);
    } else if (invoke->name == "GotoFrame") {
        log_unimpl("ExternalInterface::GotoFrame()");
        // GotoFrame doesn't send a response
    } else if (invoke->name == "IsPlaying") {
        const bool result = 
            callInterface<bool>(HostMessage(HostMessage::EXTERNALINTERFACE_ISPLAYING));
        as_value val(result);
        ss << ExternalInterface::toXML(val);    
    } else if (invoke->name == "LoadMovie") {
        log_unimpl("ExternalInterface::LoadMovie()");
    // LoadMovie doesn't send a response
    } else if (invoke->name == "Pan") {
        std::string arg = invoke->args[0].to_string();
        arg += ":";
        arg += invoke->args[0].to_string();
        arg += ":";
        arg += invoke->args[1].to_string();
        arg += ":";
        arg += invoke->args[2].to_string();
        callInterface(HostMessage(HostMessage::EXTERNALINTERFACE_PAN, arg));
    // Pan doesn't send a response
    } else if (invoke->name == "PercentLoaded") {
        MovieClip *mc = getLevel(0);
        int loaded = mc->get_bytes_loaded();
        int total = mc->get_bytes_total();
        as_value val((loaded/total) * 100);
        // PercentLoaded sends the percentage
        ss << ExternalInterface::toXML(val);    
    } else if (invoke->name == "Play") {
        callInterface(HostMessage(HostMessage::EXTERNALINTERFACE_PLAY));
    // Play doesn't send a response
    } else if (invoke->name == "Rewind") {
        callInterface(HostMessage(HostMessage::EXTERNALINTERFACE_REWIND));
    // Rewind doesn't send a response
    } else if (invoke->name == "SetZoomRect") {
        std::string arg = invoke->args[0].to_string();
        arg += ":";
        arg += invoke->args[0].to_string();
        arg += ":";
        arg += invoke->args[1].to_string();
        arg += ":";
        arg += invoke->args[2].to_string();
        arg += ":";
        arg += invoke->args[3].to_string();
        callInterface(HostMessage(HostMessage::EXTERNALINTERFACE_SETZOOMRECT, arg));
    // SetZoomRect doesn't send a response
    } else if (invoke->name == "StopPlay") {
        callInterface(HostMessage(HostMessage::EXTERNALINTERFACE_STOPPLAY));
    // StopPlay doesn't send a response
    } else if (invoke->name == "Zoom") {
        std::string var = invoke->args[0].to_string();
        callInterface(HostMessage(HostMessage::EXTERNALINTERFACE_ZOOM, var));
    // Zoom doesn't send a response
    } else if (invoke->name == "TotalFrames") {
        MovieClip *mc = getLevel(0);
        as_value val(mc->get_loaded_frames());
    // TotalFrames sends the number of frames in the movie
        ss << ExternalInterface::toXML(val);
    } else {
        std::string result = callExternalCallback(invoke->name, invoke->args);
        if (result == ExternalInterface::makeString("Error")) {
            return false;
        } else if (result == ExternalInterface::makeString("SecurityError")) {
            return false;
        }
        return true;
    }

    if (!ss.str().empty()) {
        if (_hostfd >= 0) {
            log_debug(_("Attempt to write response to ExternalInterface "
                        "requests fd %d"), _hostfd);
            int ret = write(_hostfd, ss.str().c_str(), ss.str().size());
            if (ret == -1) {
            log_error(_("Could not write to user-provided host requests "
                    "fd %d: %s"), _hostfd, std::strerror(errno));
            }
        }
    } else {
        log_debug("No response needed for %s request", invoke->name);
    }

    return true;
}

void
movie_root::executeTimers()
{
#ifdef GNASH_DEBUG_TIMERS_EXPIRATION
        log_debug("Checking %d timers for expiry", _intervalTimers.size());
#endif

    unsigned long now = _vm.getTime();

    typedef std::multimap<unsigned int, boost::shared_ptr<Timer> >
        ExpiredTimers;

    ExpiredTimers expiredTimers;

    for (TimerMap::iterator it = _intervalTimers.begin(),
            itEnd = _intervalTimers.end(); it != itEnd; ) {

        TimerMap::iterator nextIterator = it;
        ++nextIterator;

        boost::shared_ptr<Timer> timer(it->second);

        if (timer->cleared()) {
            // this timer was cleared, erase it
            _intervalTimers.erase(it);
        }
        else {
            unsigned long elapsed;
            if (timer->expired(now, elapsed)) {
                expiredTimers.insert(std::make_pair(elapsed, timer));
            }
        }

        it = nextIterator;
    }

    foreachSecond(expiredTimers.begin(), expiredTimers.end(),
                  &Timer::executeAndReset);

    if (!expiredTimers.empty()) processActionQueue();

}

void
movie_root::markReachableResources() const
{
    _vm.markReachableResources();

    foreachSecond(_movies.rbegin(), _movies.rend(), &MovieClip::setReachable);

    // Mark original top-level movie
    // This should always be in _movies, but better make sure
    assert(_rootMovie);
    _rootMovie->setReachable();

    // Mark mouse entities 
    _mouseButtonState.markReachableResources();
    
    // Mark timer targets
    foreachSecond(_intervalTimers.begin(), _intervalTimers.end(),
                  &Timer::markReachableResources);

    std::for_each(_objectCallbacks.begin(), _objectCallbacks.end(),
            std::mem_fun(&ActiveRelay::setReachable));

    std::for_each(_loadCallbacks.begin(), _loadCallbacks.end(),
            std::mem_fun_ref(&movie_root::LoadCallback::setReachable));

    // Mark LoadMovieRequest handlers as reachable
    _movieLoader.setReachable();

    // Mark resources reachable by queued action code
    for (size_t lvl = 0; lvl < PRIORITY_SIZE; ++lvl)
    {
        const ActionQueue::value_type& q = _actionQueue[lvl];
        std::for_each(q.begin(), q.end(),
                std::mem_fun_ref(&ExecutableCode::markReachableResources));
    }

    if (_currentFocus) _currentFocus->setReachable();

    // Mark DisplayObject being dragged, if any
    if (_dragState) _dragState->markReachableResources();

    // NOTE: cleanupDisplayList() should have cleaned up all
    // unloaded live characters. The remaining ones should be marked
    // by their parents.
#if ( GNASH_PARANOIA_LEVEL > 1 ) || defined(ALLOW_GC_RUN_DURING_ACTIONS_EXECUTION)
    for (LiveChars::const_iterator i=_liveChars.begin(), e=_liveChars.end();
            i!=e; ++i) {
#ifdef ALLOW_GC_RUN_DURING_ACTIONS_EXECUTION
        (*i)->setReachable();
#else
        assert((*i)->isReachable());
#endif
    }
#endif
    
}

InteractiveObject*
movie_root::getTopmostMouseEntity(boost::int32_t x, boost::int32_t y) const
{

    for (Levels::const_reverse_iterator i=_movies.rbegin(), e=_movies.rend();
            i != e; ++i)
    {
        InteractiveObject* ret = i->second->topmostMouseEntity(x, y);
        if (ret) return ret;
    }

    return 0;
}

const DisplayObject *
movie_root::findDropTarget(boost::int32_t x, boost::int32_t y,
        DisplayObject* dragging) const
{

    for (Levels::const_reverse_iterator i=_movies.rbegin(), e=_movies.rend();
            i!=e; ++i) {
        
        const DisplayObject* ret = i->second->findDropTarget(x, y, dragging);
        if (ret) return ret;
    }
    return 0;
}

/// This should store a callback object in movie_root.
//
/// TODO: currently it doesn't.
void
movie_root::addExternalCallback(const std::string& name, as_object* callback)
{
    UNUSED(callback);

    // When an external callback is added, we have to notify the plugin
    // that this method is available.
    if (_hostfd >= 0) {
        std::vector<as_value> fnargs;
        fnargs.push_back(name);
        std::string msg = ExternalInterface::makeInvoke("addMethod", fnargs);
        
        const size_t ret = ExternalInterface::writeBrowser(_hostfd, msg);
        if (ret != msg.size()) {
            log_error(_("Could not write to browser fd #%d: %s"),
                      _hostfd, std::strerror(errno));
        }
    }
}    

/// This calls a JavaScript method in the web page
///
/// @example "ExternalInterace::call message"
///
/// <pre>
/// <invoke name="methodname" returntype="xml">
///      <arguments></arguments>
///             ...
///      <arguments></arguments>
/// </invoke>
///
/// May return any supported type like Number or String in XML format.
///
/// </pre>
std::string
movie_root::callExternalJavascript(const std::string &name, 
                                   const std::vector<as_value> &fnargs)
{
    std::string result;
    // If the browser is connected, we send an Invoke message to the
    // browser.
    if (_controlfd >= 0 && _hostfd >= 0) {
        std::string msg = ExternalInterface::makeInvoke(name, fnargs);
        
        const size_t ret = ExternalInterface::writeBrowser(_hostfd, msg);
        if (ret != msg.size()) {
            log_error(_("Could not write to browser fd #%d: %s"),
                      _hostfd, std::strerror(errno));
        } else {
            // Now read the response from the browser after it's exectuted
            // the JavaScript function.
            result = ExternalInterface::readBrowser(_controlfd);
        }
    }

    return result;
}

// Call one of the registered callbacks, and return the result to
// Javascript in the browser.
std::string
movie_root::callExternalCallback(const std::string &name, 
                 const std::vector<as_value> &fnargs)
{

    MovieClip *mc = getLevel(0);
    as_object *obj = getObject(mc);

    const ObjectURI& key = getURI(getVM(), name);
    // FIXME: there has got to be a better way of handling the variable
    // length arg list
    as_value val;
    switch (fnargs.size()) {
      case 0:
          val = callMethod(obj, key);
          break;
      case 1:
          val = callMethod(obj, key, fnargs[0]);
          break;
      case 2:
          val = callMethod(obj, key, fnargs[0], fnargs[1]);
          break;
      case 3:
          val = callMethod(obj, key, fnargs[0], fnargs[1], fnargs[2]);
          break;
      default:
          val = callMethod(obj, key);
          break;
    }

    std::string result;
    if (val.is_null()) {
        // Return an error
        result = ExternalInterface::makeString("Error");
    } else {
        result = ExternalInterface::toXML(val);
    }
        
    // If the browser is connected, we send an Invoke message to the
    // browser.
    if (_hostfd >= 0) {
        const size_t ret = ExternalInterface::writeBrowser(_hostfd, result);
        if (ret != result.size()) {
            log_error(_("Could not write to browser fd #%d: %s"),
                      _hostfd, std::strerror(errno));
        }
    }

    return result;
}

void
movie_root::removeButtonKey(Button* listener)
{
    // Remove the button and the key associated with it from the map.
    for (ButtonKeys::iterator i = _buttonKeys.begin(), e = _buttonKeys.end();
            i != e;) {
        if (i->second.first == listener) {
            _buttonKeys.erase(i++);
        }
        else ++i;
    }

}

void
movie_root::registerButtonKey(int code, Button* listener)
{
    const size_t frame = _rootMovie->get_current_frame();
    ButtonKeys::const_iterator it = _buttonKeys.find(code);

    if (it != _buttonKeys.end() && it->second.second < frame) {
        return;
    }
    _buttonKeys[code] = std::make_pair(listener, frame);
}

void
movie_root::cleanupDisplayList()
{

#define GNASH_DEBUG_INSTANCE_LIST 1

#ifdef GNASH_DEBUG_INSTANCE_LIST
    static size_t maxLiveChars = 0;
#endif

    // Let every sprite cleanup the local DisplayList
    //
    // TODO: we might skip this additional scan by delegating
    //       cleanup of the local DisplayLists in the ::display
    //       method of each sprite, but that will introduce 
    //       problems when we implement skipping ::display()
    //       when late on FPS. Alternatively we may have the
    //       MovieClip::markReachableResources take care
    //       of cleaning up unloaded... but that will likely
    //       introduce problems when allowing the GC to run
    //       at arbitrary times.
    //       The invariant to keep is that cleanup of unloaded DisplayObjects
    //       in local display lists must happen at the *end* of global action
    //       queue processing.
    foreachSecond(_movies.rbegin(), _movies.rend(),
                  &MovieClip::cleanupDisplayList);

    // Now remove from the instance list any unloaded DisplayObject
    // Note that some DisplayObjects may be unloaded but not yet destroyed,
    // in this case we'll also destroy them, which in turn might unload
    // further DisplayObjects, maybe already scanned, so we keep scanning
    // the list until no more unloaded-but-non-destroyed DisplayObjects
    // are found.
    // Keeping unloaded-but-non-destroyed DisplayObjects wouldn't really hurt
    // in that ::advanceLiveChars would skip any unloaded DisplayObjects.
    // Still, the more we remove the less work GC has to do...
    //

    bool needScan;
#ifdef GNASH_DEBUG_DLIST_CLEANUP
    int scansCount = 0;
#endif
    do {
#ifdef GNASH_DEBUG_DLIST_CLEANUP
        scansCount++;
        int cleaned =0;
#endif
        needScan=false;

        // Remove unloaded MovieClips from the _liveChars list
        for (LiveChars::iterator i = _liveChars.begin(), e = _liveChars.end();
                i != e;) {
            MovieClip* ch = *i;
            if (ch->unloaded()) {
                // the sprite might have been destroyed already
                // by effect of an unload() call with no onUnload
                // handlers available either in self or child
                // DisplayObjects
                if (!ch->isDestroyed()) {

#ifdef GNASH_DEBUG_DLIST_CLEANUP
                    cout << ch->getTarget() << "(" << typeName(*ch) <<
                        ") was unloaded but not destroyed, destroying now" <<
                        endl;
#endif
                    ch->destroy();
                    // destroy() might mark already-scanned chars as unloaded
                    needScan = true; 
                }
#ifdef GNASH_DEBUG_DLIST_CLEANUP
                else {
                    cout << ch->getTarget() << "(" << typeName(*ch) <<
                        ") was unloaded and destroyed" << endl;
                }
#endif

                i = _liveChars.erase(i);

#ifdef GNASH_DEBUG_DLIST_CLEANUP
                cleaned++;
#endif
            }
            else ++i; 
        }

#ifdef GNASH_DEBUG_DLIST_CLEANUP
        cout << " Scan " << scansCount << " cleaned " << cleaned <<
            " instances" << endl;
#endif
    } while (needScan);

#ifdef GNASH_DEBUG_INSTANCE_LIST
    if (_liveChars.size() > maxLiveChars) {
        maxLiveChars = _liveChars.size();
        log_debug("Global instance list grew to %d entries", maxLiveChars);
    }
#endif

}

void
movie_root::advanceLiveChars()
{

#ifdef GNASH_DEBUG
    log_debug("---- movie_root::advance: %d live DisplayObjects in "
            "the global list", _liveChars.size());
#endif

    std::for_each(_liveChars.begin(), _liveChars.end(),
            boost::bind(advanceLiveChar, _1));
}

void
movie_root::set_background_color(const rgba& color)
{
    if (m_background_color_set) return;
    m_background_color_set = true;
    
    rgba newcolor = color;
    newcolor.m_a = m_background_color.m_a;

    if (m_background_color != newcolor) {
        setInvalidated();
        m_background_color = newcolor;
    }
}

void
movie_root::set_background_alpha(float alpha)
{

    boost::uint8_t newAlpha = clamp<int>(frnd(alpha * 255.0f), 0, 255);

    if (m_background_color.m_a != newAlpha) {
        setInvalidated();
        m_background_color.m_a = newAlpha;
    }
}

DisplayObject*
movie_root::findCharacterByTarget(const std::string& tgtstr) const
{
    if (tgtstr.empty()) return 0;

    // NOTE: getRootMovie() would be problematic in case the original
    //       root movie is replaced by a load to _level0... 
    //       (but I guess we'd also drop loadMovie requests in that
    //       case... just not tested)
    as_object* o = getObject(_movies.begin()->second);
    assert(o);

    std::string::size_type from = 0;
    while (std::string::size_type to = tgtstr.find('.', from)) {
        std::string part(tgtstr, from, to - from);

        // TODO: there is surely a cleaner way to implement path finding.
        const ObjectURI& uri = getURI(_vm, part);
        o = o->displayObject() ?
            o->displayObject()->pathElement(uri) :
            getPathElement(*o, uri);

        if (!o) {
#ifdef GNASH_DEBUG_TARGET_RESOLUTION
            log_debug("Evaluating DisplayObject target path: element "
                    "'%s' of path '%s' not found", part, tgtstr);
#endif
            return NULL;
        }
        if (to == std::string::npos) break;
        from = to + 1;
    }
    return get<DisplayObject>(o);
}

void
movie_root::getURL(const std::string& urlstr, const std::string& target,
        const std::string& data, MovieClip::VariablesMethod method)
{

    log_network("%s: HOSTFD is %d",  __FUNCTION__, _hostfd);
    
    if (_hostfd < 0) {
        /// If there is no hosting application, call the URL launcher. For
        /// safety, we resolve the URL against the base URL for this run.
        /// The data is not sent at all.
        URL url(urlstr, _runResources.streamProvider().baseURL());

        gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
        std::string command = rcfile.getURLOpenerFormat();

        /// Try to avoid letting flash movies execute
        /// arbitrary commands (sic)
        ///
        /// Maybe we should exec here, but if we do we might have problems
        /// with complex urlOpenerFormats like:
        ///    firefox -remote 'openurl(%u)'
        ///
        std::string safeurl = url.encode(urlstr);
        boost::replace_all(command, "%u", safeurl);
        
        log_debug (_("Launching URL: %s"), command);
        const int ret = std::system(command.c_str());
        if (ret == -1) {
            log_error(_("Fork failed launching url opener '%s'"), command);
        }
        return;
    }

    /// This is when there is a hosting application.
    std::vector<as_value> fnargs;
    // The first argument we push on the stack is the URL
    fnargs.push_back(as_value(urlstr));
    
    // The second argument we push is the method
    switch (method) {
      case MovieClip::METHOD_POST:
          fnargs.push_back(as_value("POST"));
          break;
      case MovieClip::METHOD_GET:
          fnargs.push_back(as_value("GET"));
          break;
      case MovieClip::METHOD_NONE:
      default:
          fnargs.push_back(as_value("GET"));
          break;
    }

    // The third argument is the target, which is something like _blank
    // or _self.
    if (!target.empty()) {
        fnargs.push_back(as_value(target));
    }
    // Add any data as the optional 4th argument
    if (!data.empty()) {
        // We have to write a value here so the data field is the fourth
        if (target.empty()) {
            fnargs.push_back(as_value("none"));
        }
        fnargs.push_back(as_value(data));
    }

    // TODO: should mutex-protect this ?
    // NOTE: we are assuming the hostfd is set in blocking mode here..

    log_debug(_("Attempt to write geturl requests fd #%d"), _hostfd);

    std::string msg = ExternalInterface::makeInvoke("getURL", fnargs);

    const size_t ret = ExternalInterface::writeBrowser(_hostfd, msg);
    if (ret < msg.size()) {
        log_error(_("Could only write %d bytes to fd #%d"),
          ret, _hostfd);
    }
}

void
movie_root::setScriptLimits(boost::uint16_t recursion, boost::uint16_t timeout)
{
    
    if ( recursion == _recursionLimit && _timeoutLimit == timeout ) {
        // avoid the debug log...
        return;
    }

    if ( RcInitFile::getDefaultInstance().lockScriptLimits() )
    {
        LOG_ONCE( log_debug(_("SWF ScriptLimits tag attempting to set "
            "recursionLimit=%1% and scriptsTimeout=%2% ignored "
            "as per rcfile directive"), recursion, timeout) );
        return;
    }

    // This tag reported in some sources to be ignored for movies
    // below SWF7. However, on Linux with PP version 9, the tag
    // takes effect on SWFs of any version.
    log_debug(_("Setting script limits: max recursion %d, "
            "timeout %d seconds"), recursion, timeout);

    _recursionLimit = recursion;
    _timeoutLimit = timeout;
    
}


#ifdef USE_SWFTREE
void
movie_root::getMovieInfo(InfoTree& tr, InfoTree::iterator it)
{

    // Stage
    const movie_definition* def = _rootMovie->definition();
    assert(def);

    it = tr.insert(it, std::make_pair("Stage Properties", ""));

    InfoTree::iterator localIter =  tr.append_child(it,
            std::make_pair("Root VM version",
                def->isAS3() ? "AVM2 (unsupported)" : "AVM1"));
    
    std::ostringstream os;
    os << "SWF " << def->get_version();
    localIter = tr.append_child(it, std::make_pair("Root SWF version",
                os.str()));
    localIter = tr.append_child(it, std::make_pair("URL", def->get_url()));

    // TODO: format this better?
    localIter = tr.append_child(it, std::make_pair("Descriptive metadata",
                                        def->getDescriptiveMetadata()));
 
    /// Stage: real dimensions.
    os.str("");
    os << def->get_width_pixels() <<
        "x" << def->get_height_pixels();
    localIter = tr.append_child(it, std::make_pair("Real dimensions",
                os.str()));

    /// Stage: rendered dimensions.
    os.str("");
    os << _stageWidth << "x" << _stageHeight;
    localIter = tr.append_child(it, std::make_pair("Rendered dimensions",
                os.str()));

    // Stage: scripts state (enabled/disabled)
    localIter = tr.append_child(it, std::make_pair("Scripts",
                _disableScripts ? " disabled" : "enabled"));
     
    getCharacterTree(tr, it);    
}

void
movie_root::getCharacterTree(InfoTree& tr, InfoTree::iterator it)
{

    InfoTree::iterator localIter;

    /// Stage: number of live MovieClips.
    std::ostringstream os;
    os << _liveChars.size();
    localIter = tr.append_child(it, std::make_pair(_("Live MovieClips"),
                os.str()));

    /// DisplayObject tree
    for (Levels::const_iterator i = _movies.begin(), e = _movies.end();
            i != e; ++i) {
        i->second->getMovieInfo(tr, localIter);
    }

}

#endif

void
movie_root::handleFsCommand(const std::string& cmd, const std::string& arg)
    const
{
    if (_fsCommandHandler) _fsCommandHandler->notify(cmd, arg);
}

bool
isLevelTarget(int version, const std::string& name, unsigned int& levelno)
{
    if (version > 6) {
        if (name.compare(0, 6, "_level")) return false;
    }
    else {
        StringNoCaseEqual noCaseCmp;
        if (!noCaseCmp(name.substr(0, 6), "_level")) return false;
    }

    if (name.find_first_not_of("0123456789", 7) != std::string::npos) {
        return false;
    }
    // getting 0 here for "_level" is intentional
    levelno = std::strtoul(name.c_str() + 6, NULL, 0); 
    return true;

}

short
stringToStageAlign(const std::string& str)
{
    short am = 0;

    // Easy enough to do bitwise - std::bitset is not
    // really necessary!
    if (str.find_first_of("lL") != std::string::npos) {
        am |= 1 << movie_root::STAGE_ALIGN_L;
    } 

    if (str.find_first_of("tT") != std::string::npos) {
        am |= 1 << movie_root::STAGE_ALIGN_T;
    } 

    if (str.find_first_of("rR") != std::string::npos) {
        am |= 1 << movie_root::STAGE_ALIGN_R;
    } 

    if (str.find_first_of("bB") != std::string::npos) {
        am |= 1 << movie_root::STAGE_ALIGN_B;
    }

    return am;

}

void
movie_root::LoadCallback::setReachable() const
{
    _obj->setReachable();
}

bool
movie_root::LoadCallback::processLoad()
{

    if (!_stream) {
        callMethod(_obj, NSV::PROP_ON_DATA, as_value());
        return true;
    }

    const size_t chunksize = 65535;
    boost::uint8_t chunk[chunksize];

    size_t actuallyRead = _stream->readNonBlocking(chunk, chunksize);

    // We must still call onData if the stream is in error condition, e.g.
    // when an HTTP 404 error is returned.
    if (_stream->bad()) {
        callMethod(_obj, NSV::PROP_ON_DATA, as_value());
        return true;
    }

    if (actuallyRead) {

        // set total size only on first read
        if (_buf.empty()) {
            _obj->set_member(NSV::PROP_uBYTES_TOTAL, _stream->size());
        }

        _buf.append(chunk, actuallyRead);

        _obj->set_member(NSV::PROP_uBYTES_LOADED, _buf.size());

        log_debug("LoadableObject Loaded %d bytes, reaching %d/%d",
            actuallyRead, _buf.size(), _stream->size());
    }

    // We haven't finished till EOF 
    if (!_stream->eof()) return false;

    log_debug("LoadableObject reached EOF (%d/%d loaded)",
                _buf.size(), _stream->size());

    // got nothing, won't bother BOFs of nulls
    if (_buf.empty()) {
        callMethod(_obj, NSV::PROP_ON_DATA, as_value());
        return true;
    }

    // Terminate the string
    _buf.appendByte('\0');

    // Strip BOM, if any.
    // See http://savannah.gnu.org/bugs/?19915
    utf8::TextEncoding encoding;
    size_t size = _buf.size();

    // NOTE: the call below will possibly change 'size' parameter
    char* bufptr = utf8::stripBOM((char*)_buf.data(), size, encoding);
    if (encoding != utf8::encUTF8 && encoding != utf8::encUNSPECIFIED) {
        log_unimpl("%s to utf8 conversion in LoadableObject input parsing", 
                utf8::textEncodingName(encoding));
    }

    // NOTE: Data copy here !!
    as_value dataVal(bufptr); 

    // NOTE: we could release memory associated
    // with the buffer here, before invoking a new method,
    // but at the time of writing there's no method of SimpleBuffer
    // providing memory release except destruction. Will be
    // destroyed as soon as we return though...

    // NOTE: Another data copy here !
    callMethod(_obj, NSV::PROP_ON_DATA, dataVal);

    return true;
}

void
movie_root::callInterface(const HostInterface::Message& e) const
{
    if (!_interfaceHandler) {
        log_error("Hosting application registered no callback for events/queries"
            ", can't call %s(%s)");
        return;
    }
    _interfaceHandler->call(e);
}


inline bool
movie_root::testInvariant() const
{
    // TODO: fill this function !
    // The _movies map can not invariantably
    // be non-empty as the stage is autonomous
    // itself
    //assert( ! _movies.empty() );

    return true;
}

namespace {

// Return whether any action triggered by this event requires display redraw.
//
/// TODO: make this code more readable !
bool
generate_mouse_button_events(movie_root& mr, MouseButtonState& ms)
{
    // Did this event trigger any action that needs redisplay ?
    bool need_redisplay = false;

    // TODO: have mouseEvent return
    // whether the action must trigger
    // a redraw.

    if (ms.wasDown) {
        // TODO: Handle trackAsMenu dragOver
        // Handle onDragOut, onDragOver
        if (!ms.wasInsideActiveEntity) {

            if (ms.topmostEntity == ms.activeEntity) {

                // onDragOver
                if (ms.activeEntity) {
                    ms.activeEntity->mouseEvent(event_id(event_id::DRAG_OVER));
                    need_redisplay=true;
                }
                ms.wasInsideActiveEntity = true;
            }
        }
        else if (ms.topmostEntity != ms.activeEntity) {
            // onDragOut
            if (ms.activeEntity) {
                ms.activeEntity->mouseEvent(event_id(event_id::DRAG_OUT));
                need_redisplay=true;
            }
            ms.wasInsideActiveEntity = false;
        }

        // Handle onRelease, onReleaseOutside
        if (!ms.isDown) {
            // Mouse button just went up.
            ms.wasDown = false;

            if (ms.activeEntity) {
                if (ms.wasInsideActiveEntity) {
                    // onRelease
                    ms.activeEntity->mouseEvent(event_id(event_id::RELEASE));
                    need_redisplay = true;
                }
                else {
                    // TODO: Handle trackAsMenu 
                    // onReleaseOutside
                    ms.activeEntity->mouseEvent(
                            event_id(event_id::RELEASE_OUTSIDE));
                    // We got out of active entity
                    ms.activeEntity = 0; // so we don't get RollOut next...
                    need_redisplay = true;
                }
            }
        }
        return need_redisplay;
    }

    else {
        // New active entity is whatever is below the mouse right now.
        if (ms.topmostEntity != ms.activeEntity) {
            // onRollOut
            if (ms.activeEntity) {
                ms.activeEntity->mouseEvent(event_id(event_id::ROLL_OUT));
                need_redisplay=true;
            }

            ms.activeEntity = ms.topmostEntity;

            // onRollOver
            if (ms.activeEntity) {
                ms.activeEntity->mouseEvent(event_id(event_id::ROLL_OVER));
                need_redisplay=true;
            }

            ms.wasInsideActiveEntity = true;
        }

        // mouse button press
        if (ms.isDown) {
            // onPress

            // Try setting focus on the new DisplayObject. This will handle
            // all necessary events and removal of current focus.
            // Do not set focus to NULL.
            if (ms.activeEntity) {
                mr.setFocus(ms.activeEntity);

                ms.activeEntity->mouseEvent(event_id(event_id::PRESS));
                need_redisplay = true;
            }

            ms.wasInsideActiveEntity = true;
            ms.wasDown = true;
        }
    
    }
    return need_redisplay;

}

const DisplayObject*
getNearestObject(const DisplayObject* o)
{
    while (1) {
        assert(o);
        if (isReferenceable(*o)) return o;
        o = o->parent();
    }
}

as_object*
getBuiltinObject(movie_root& mr, const ObjectURI& cl)
{
    Global_as& gl = *mr.getVM().getGlobal();

    as_value val;
    if (!gl.get_member(cl, &val)) return 0;
    return toObject(val, mr.getVM());
}

void
advanceLiveChar(MovieClip* mo)
{
    if (!mo->unloaded()) {
#ifdef GNASH_DEBUG
        log_debug("    advancing DisplayObject %s", ch->getTarget());
#endif
        mo->advance();
    }
#ifdef GNASH_DEBUG
    else {
        log_debug("    DisplayObject %s is unloaded, not advancing it",
                mo->getTarget());
    }
#endif
}

} // anonymous namespace
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
