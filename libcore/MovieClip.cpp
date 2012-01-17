// MovieClip.cpp:  Stateful live Sprite instance, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#include "gnashconfig.h" // USE_SWFTREE 
#endif

#include "MovieClip.h"

#include <vector>
#include <string>
#include <algorithm> // for std::swap
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind.hpp>

#include "log.h" 
#include "movie_definition.h"
#include "as_value.h"
#include "as_function.h"
#include "TextField.h"
#include "ControlTag.h"
#include "fn_call.h"
#include "movie_root.h"
#include "Movie.h"
#include "swf_event.h"
#include "sprite_definition.h"
#include "ActionExec.h"
#include "VM.h"
#include "Range2d.h" // for getBounds
#include "GnashException.h"
#include "GnashNumeric.h"
#include "GnashAlgorithm.h"
#include "URL.h"
#include "sound_handler.h"
#include "StreamProvider.h"
#include "LoadVariablesThread.h" 
#include "ExecutableCode.h" // for inheritance of ConstructEvent
#include "DynamicShape.h" // for composition
#include "namedStrings.h"
#include "LineStyle.h"
#include "PlaceObject2Tag.h" 
#include "flash/geom/Matrix_as.h"
#include "GnashNumeric.h"
#include "InteractiveObject.h"
#include "DisplayObjectContainer.h"
#include "Global_as.h"
#include "RunResources.h"
#include "Transform.h"
#include "ConstantPool.h" // for PoolGuard

namespace gnash {

//#define GNASH_DEBUG 1
//#define GNASH_DEBUG_TIMELINE 1
//#define GNASH_DEBUG_REPLACE 1
//#define DEBUG_DYNTEXT_VARIABLES 1
//#define GNASH_DEBUG_HITTEST 1
//#define DEBUG_LOAD_VARIABLES 1

// Defining the following macro you'll get a DEBUG lien
// for each call to the drawing API, in a format which is
// easily re-compilable to obtain a smaller testcase
//#define DEBUG_DRAWING_API 1

// Define this to make mouse entity finding verbose
// This includes topmostMouseEntity and findDropTarget
//
//#define DEBUG_MOUSE_ENTITY_FINDING 1

namespace {
    MovieClip::TextFields* textfieldVar(MovieClip::TextFieldIndex* t,
            const ObjectURI& name);
}

// Utility functors.
namespace {

/// ConstructEvent, used for queuing construction
//
/// Its execution will call constructAsScriptObject() 
/// on the target movieclip
///
class ConstructEvent : public ExecutableCode
{
public:

    explicit ConstructEvent(MovieClip* nTarget)
        :
        ExecutableCode(nTarget)
    {}

    virtual void execute() {
        static_cast<MovieClip*>(target())->constructAsScriptObject();
    }

};

/// Generic event  (constructed by id, invoked using notifyEvent
class QueuedEvent : public ExecutableCode
{
public:

    QueuedEvent(MovieClip* nTarget, const event_id& id)
        :
        ExecutableCode(nTarget),
        _eventId(id)
    {}

    virtual void execute() {
        // don't execute any events for destroyed DisplayObject.
        if (!target()->isDestroyed()) {
            static_cast<MovieClip*>(target())->notifyEvent(_eventId);
        }
    }

private:
    const event_id _eventId;
};

/// Find a DisplayObject hit by the given coordinates.
//
/// This class takes care about taking masks layers into
/// account, but nested masks aren't properly tested yet.
///
class MouseEntityFinder
{
public:

    /// @param wp
    ///     Query point in world coordinate space
    ///
    /// @param pp
    ///     Query point in parent coordinate space
    ///
    MouseEntityFinder(point wp, point pp)
        :
        _highestHiddenDepth(std::numeric_limits<int>::min()),
        _m(NULL),
        _candidates(),
        _wp(wp),
        _pp(pp),
        _checked(false)
    {}

    void operator() (DisplayObject* ch) {
        assert(!_checked);
        if (ch->get_depth() <= _highestHiddenDepth) {
            if (ch->isMaskLayer()) {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug("CHECKME: nested mask in MouseEntityFinder. "
                            "This mask is %s at depth %d outer mask masked "
                            "up to depth %d.",
                            ch->getTarget(), ch->get_depth(),
                            _highestHiddenDepth);
                // Hiding mask still in effect...
#endif
            }
            return;
        }

        if (ch->isMaskLayer()) {
            if (!ch->pointInShape(_wp.x, _wp.y)) {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug("Character %s at depth %d is a mask not hitting "
                        "the query point %g,%g and masking up to "
                        "depth %d", ch->getTarget(), ch->get_depth(), 
                        _wp.x, _wp.y, ch->get_clip_depth());
#endif
                _highestHiddenDepth = ch->get_clip_depth();
            }
            else {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug("Character %s at depth %d is a mask hitting the "
                        "query point %g,%g",
                        ch->getTarget(), ch->get_depth(), _wp.x, _wp.y);
#endif 
            }
            return;
        }
        if (!ch->visible()) return;

        _candidates.push_back(ch);
    }

    void checkCandidates() {
        if (_checked) return;
        for (Candidates::reverse_iterator i=_candidates.rbegin(),
                        e=_candidates.rend(); i!=e; ++i) {
            DisplayObject* ch = *i;
            InteractiveObject* te = ch->topmostMouseEntity(_pp.x, _pp.y);
            if (te) {
                _m = te;
                break;
            }
        }
        _checked = true;
    }

    InteractiveObject* getEntity() {
        checkCandidates();
#ifdef DEBUG_MOUSE_ENTITY_FINDING
        if (_m) {
            log_debug("MouseEntityFinder found DisplayObject %s (depth %d) "
                    "hitting point %g,%g",
                    _m->getTarget(), _m->get_depth(), _wp.x, _wp.y);
        }
#endif // DEBUG_MOUSE_ENTITY_FINDING
        return _m;
    }

private:

    /// Highest depth hidden by a mask
    //
    /// This will be -1 initially, and set
    /// the the depth of a mask when the mask
    /// doesn't contain the query point, while
    /// scanning a DisplayList bottom-up
    ///
    int _highestHiddenDepth;

    InteractiveObject* _m;

    typedef std::vector<DisplayObject*> Candidates;
    Candidates _candidates;

    /// Query point in world coordinate space
    point    _wp;

    /// Query point in parent coordinate space
    point    _pp;

    bool _checked;

};

/// Find the first DisplayObject whose shape contain the point
//
/// Point coordinates in world TWIPS
///
class ShapeContainerFinder
{
public:

    ShapeContainerFinder(boost::int32_t x, boost::int32_t y)
        :
        _found(false),
        _x(x),
        _y(y)
    {}

    bool operator()(const DisplayObject* ch) {
        if (ch->pointInShape(_x, _y)) {
            _found = true;
            return false;
        }
        return true;
    }

    bool hitFound() const { return _found; }

private:
    bool _found;
    const boost::int32_t _x;
    const boost::int32_t _y;
};

/// Find the first visible DisplayObject whose shape contain the point
//
/// Point coordinates in world TWIPS
///
class VisibleShapeContainerFinder
{
public:

    VisibleShapeContainerFinder(boost::int32_t x, boost::int32_t y)
        :
        _found(false),
        _x(x),
        _y(y)
    {}

    bool operator()(const DisplayObject* ch) {

        if (ch->pointInVisibleShape(_x, _y)) {
            _found = true;
            return false;
        }
        return true;
    }

    bool hitFound() const { return _found; }

private:
    bool _found;
    const boost::int32_t _x;
    const boost::int32_t _y;
};

/// Find the first hitable DisplayObject whose shape contain the point 
// 
/// Point coordinates in world TWIPS 
/// 
class HitableShapeContainerFinder
{ 
public: 
    HitableShapeContainerFinder(boost::int32_t x, boost::int32_t y) 
        : 
        _found(false), 
        _x(x), 
        _y(y) 
    {} 

    bool operator()(const DisplayObject* ch) { 
        if (ch->isDynamicMask()) return true; 
        if (ch->pointInShape(_x, _y)) {
            _found = true; 
            return false; 
        } 
        return true; 
    } 

    bool hitFound() const { return _found; } 

private:

    bool _found; 

    // x position in twips.
    const boost::int32_t _x;
    
    // y position in twips.
    const boost::int32_t _y;
}; 

/// A DisplayList visitor used to compute its overall bounds.
//
class BoundsFinder
{
public:
    explicit BoundsFinder(SWFRect& b) : _bounds(b) {}

    void operator()(DisplayObject* ch) {
        // don't include bounds of unloaded DisplayObjects
        if (ch->unloaded()) return;
        SWFRect chb = ch->getBounds();
        SWFMatrix m = getMatrix(*ch);
        _bounds.expand_to_transformed_rect(m, chb);
    }

private:
    SWFRect& _bounds;
};

struct ReachableMarker
{
    void operator()(DisplayObject *ch) const {
        ch->setReachable();
    }
};

/// Find the first visible DisplayObject whose shape contain the point
/// and is not the DisplayObject being dragged or any of its childs
//
/// Point coordinates in world TWIPS
///
class DropTargetFinder
{
public:
    DropTargetFinder(boost::int32_t x, boost::int32_t y, DisplayObject* dragging)
        :
        _highestHiddenDepth(std::numeric_limits<int>::min()),
        _x(x),
        _y(y),
        _dragging(dragging),
        _dropch(0),
        _candidates(),
        _checked(false)
    {}

    void operator()(const DisplayObject* ch) {
        assert(!_checked);
        if (ch->get_depth() <= _highestHiddenDepth) {
            if (ch->isMaskLayer()) {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug("CHECKME: nested mask in DropTargetFinder. "
                        "This mask is %s at depth %d outer mask masked "
                        "up to depth %d.",
                        ch->getTarget(), ch->get_depth(), _highestHiddenDepth);
                // Hiding mask still in effect...
#endif
            }
            return;
        }

        if (ch->isMaskLayer()) {
            if (!ch->visible()) {
                log_debug("FIXME: invisible mask in MouseEntityFinder.");
            }
            if (!ch->pointInShape(_x, _y)) {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug("Character %s at depth %d is a mask not hitting "
                        "the query point %g,%g and masking up to depth %d",
                    ch->getTarget(), ch->get_depth(), _x, _y,
                    ch->get_clip_depth());
#endif 
                _highestHiddenDepth = ch->get_clip_depth();
            }
            else {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug("Character %s at depth %d is a mask "
                            "hitting the query point %g,%g",
                            ch->getTarget(), ch->get_depth(), _x, _y);
#endif
            }
            return;
        }
        _candidates.push_back(ch);
    }

    void checkCandidates() const {
        if (_checked) return;
        for (Candidates::const_reverse_iterator i=_candidates.rbegin(),
                        e=_candidates.rend(); i!=e; ++i) {
            const DisplayObject* ch = *i;
            const DisplayObject* dropChar =
                ch->findDropTarget(_x, _y, _dragging);
            if (dropChar) {
                _dropch = dropChar;
                break;
            }
        }
        _checked = true;
    }

    const DisplayObject* getDropChar() const {
        checkCandidates();
        return _dropch;
    }
private:
    /// Highest depth hidden by a mask
    //
    /// This will be -1 initially, and set
    /// the the depth of a mask when the mask
    /// doesn't contain the query point, while
    /// scanning a DisplayList bottom-up
    ///
    int _highestHiddenDepth;

    boost::int32_t _x;
    boost::int32_t _y;
    DisplayObject* _dragging;
    mutable const DisplayObject* _dropch;

    typedef std::vector<const DisplayObject*> Candidates;
    Candidates _candidates;

    mutable bool _checked;
};

class DisplayListVisitor
{
public:
    DisplayListVisitor(KeyVisitor& v) : _v(v) {}

    void operator()(DisplayObject* ch) const {
         if (!isReferenceable(*ch)) return;
         // Don't enumerate unloaded DisplayObjects
         if (ch->unloaded()) return;
          
         const ObjectURI& name = ch->get_name();
         // Don't enumerate unnamed DisplayObjects
         if (name.empty()) return;
          
         // Referenceable DisplayObject always have an object.
         assert(getObject(ch));
         _v(name);
    }
private:
    KeyVisitor& _v;
};

} // anonymous namespace


MovieClip::MovieClip(as_object* object, const movie_definition* def,
        Movie* r, DisplayObject* parent)
    :
    DisplayObjectContainer(object, parent),
    _def(def),
    _swf(r),
    _playState(PLAYSTATE_PLAY),
    _environment(getVM(*object)),
    _currentFrame(0),
    m_sound_stream_id(-1),
    _hasLooped(false),
    _flushedOrphanedTags(false),
    _callingFrameActions(false),
    _lockroot(false),
    _onLoadCalled(false)
{
    assert(_swf);
    assert(object);

    _environment.set_target(this);
}

MovieClip::~MovieClip()
{
    stopStreamSound();
}

int
MovieClip::getDefinitionVersion() const
{
    return _swf->version();
}

DisplayObject*
MovieClip::getDisplayObjectAtDepth(int depth)
{
    return _displayList.getDisplayObjectAtDepth(depth);
}

void
MovieClip::queueEvent(const event_id& id, int lvl)
{
    std::auto_ptr<ExecutableCode> event(new QueuedEvent(this, id));
    stage().pushAction(event, lvl);
}

/// This handles special properties of MovieClip.
//
/// The only genuine special properties are DisplayList members. These
/// are accessible as properties and are enumerated, but not ownProperties
/// of a MovieClip.
//
/// The TextField variables should probably be handled in a more generic
/// way.
bool
MovieClip::getTextFieldVariables(const ObjectURI& uri, as_value& val)
{
    // Try textfield variables
    TextFields* etc = textfieldVar(_text_variables.get(), uri);
    if (etc) {
        for (TextFields::const_iterator i=etc->begin(), e=etc->end();
                i!=e; ++i) {

            TextField* tf = *i;
            if (tf->getTextDefined()) {
                val = tf->get_text_value();
                return true;
            }
        }
    }
    return false;
}

bool
MovieClip::get_frame_number(const as_value& frame_spec, size_t& frameno) const
{
    // If there is no definition, this is a dynamically-created MovieClip
    // and has no frames.
    if (!_def) return false;

    std::string fspecStr = frame_spec.to_string();

    as_value str(fspecStr);

    const double num = toNumber(str, getVM(*getObject(this)));

    if (!isFinite(num) || int(num) != num || num == 0) {
        bool ret = _def->get_labeled_frame(fspecStr, frameno);
        return ret;
    }

    if (num < 0) return false;

    // all frame numbers > 0 are valid, but a valid frame number may still
    // reference a non-exist frame(eg. frameno > total_frames).
    frameno = size_t(num) - 1;

    return true;
}

/// Execute the actions for the specified frame. 
//
/// The frame_spec could be an integer or a string.
///
void
MovieClip::call_frame_actions(const as_value& frame_spec)
{
    // If there is no definition, this is a dynamically-created MovieClip
    // and has no frames.
    if (!_def) return;

    // TODO: check to see how this can be prevented.
    if (isDestroyed()) return;

    size_t frame_number;
    if (!get_frame_number(frame_spec, frame_number)) {
        // No dice.
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("call_frame('%s') -- invalid frame"),
                        frame_spec);
        );
        return;
    }

    // Execute the ControlTag actions
    // We set _callingFrameActions to true so that add_action_buffer
    // will execute immediately instead of queuing them.
    // NOTE: in case gotoFrame is executed by code in the called frame
    //             we'll temporarly clear the _callingFrameActions flag
    //             to properly queue actions back on the global queue.
    //
    _callingFrameActions = true;
    PoolGuard poolGuard(getVM(*getObject(this)), 0);
    const PlayList* playlist = _def->getPlaylist(frame_number);
    if (playlist) {
        PlayList::const_iterator it = playlist->begin();
        const PlayList::const_iterator e = playlist->end();
        for (; it != e; ++it) {
            (*it)->executeActions(this, _displayList);
        }
    }
    _callingFrameActions = false;

}

DisplayObject*
MovieClip::addDisplayListObject(DisplayObject* obj, int depth)
{
    // TODO: only call set_invalidated if this DisplayObject actually overrides
    //             an existing one !
    set_invalidated(); 
    _displayList.placeDisplayObject(obj, depth);
    obj->construct();
    return obj;
}


MovieClip*
MovieClip::duplicateMovieClip(const std::string& newname, int depth,
        as_object* initObject)
{
    DisplayObject* parent_ch = parent();
    if (!parent_ch) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Can't clone root of the movie"));
        );
        return 0;
    }

    MovieClip* parent = parent_ch->to_movie();
    if (!parent) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_error(_("%s parent is not a movieclip, can't clone"),
                getTarget());
        );
        return 0;
    }

    as_object* o = getObjectWithPrototype(getGlobal(*getObject(this)), 
            NSV::CLASS_MOVIE_CLIP);

    MovieClip* newmovieclip = new MovieClip(o, _def.get(), _swf, parent);

    const ObjectURI& nn = getURI(getVM(*getObject(this)), newname);
    newmovieclip->set_name(nn);

    newmovieclip->setDynamic();

    // Copy event handlers from movieclip
    // We should not copy 'm_action_buffer' since the 
    // 'm_method' already contains it
    newmovieclip->set_event_handlers(get_event_handlers());

    // Copy drawable
    newmovieclip->_drawable = _drawable;
    
    newmovieclip->setCxForm(getCxForm(*this));
    newmovieclip->setMatrix(getMatrix(*this), true); 
    newmovieclip->set_ratio(get_ratio());    
    newmovieclip->set_clip_depth(get_clip_depth());    
    
    parent->_displayList.placeDisplayObject(newmovieclip, depth);
    newmovieclip->construct(initObject);
    
    return newmovieclip; 
}

void
MovieClip::queueAction(const action_buffer& action)
{
    stage().pushAction(action, this);
}

void
MovieClip::notifyEvent(const event_id& id)
{
#ifdef GNASH_DEBUG
    log_debug("Event %s invoked for movieclip %s", id, getTarget());
#endif

    // We do not execute ENTER_FRAME if unloaded
    if (id.id() == event_id::ENTER_FRAME && unloaded()) {
#ifdef GNASH_DEBUG
        log_debug("Sprite %s ignored ENTER_FRAME event (is unloaded)",
                getTarget());
#endif
        return;
    }

    if (isButtonEvent(id) && !isEnabled()) {
#ifdef GNASH_DEBUG
        log_debug("Sprite %s ignored button-like event %s as not 'enabled'",
            getTarget(), id);
#endif
        return;
    }

    // Dispatch static event handlers (defined in PlaceObject tags).
    std::auto_ptr<ExecutableCode> code(get_event_handler(id));
    if (code.get()) {
        // Dispatch.
        code->execute();
    }

    // Now call user-defined event handlers, but not for everything.

    // User-defined key events are never called.
    if (isKeyEvent(id)) return;

    // user-defined onInitialize is never called
    if (id.id() == event_id::INITIALIZE) return;

    // NOTE: user-defined onLoad is not invoked for static
    //     clips on which no clip-events are defined.
    //     see testsuite/misc-ming.all/action_execution_order_extend_test.swf
    //
    //     Note that this can't be true for movieclips
    //     not placed by PlaceObject, see
    //     testsuite/misc-ming.all/registerClassTest.swf
    //
    //     Note that this is also not true for movieclips which have
    //     a registered class on them, see
    //     testsuite/misc-ming.all/registerClassTest2.swf
    //
    //     TODO: test the case in which it's MovieClip.prototype.onLoad
    //     defined !
    if (id.id() == event_id::LOAD) {

        // TODO: we're likely making too much noise for nothing here,
        // there must be some action-execution-order related problem instead....
        // See testsuite/misc-ming.all/registerClassTest2.swf for an onLoad 
        // execution order related problem ...
        do {
            // we don't skip calling user-defined onLoad for top-level movies 
            if (!parent()) break;
            // nor if there are clip-defined handler
            if (!get_event_handlers().empty()) break; 
            // nor if it's dynamic  
            if (isDynamic()) break;

            // must be a loaded movie (loadMovie doesn't mark it as 
            // "dynamic" - should it? no, or getBytesLoaded will always
            // return 0)  
            if (!_def) break;

            // if it has a registered class it can have an onLoad 
            // in prototype...
            if (stage().getRegisteredClass(_def.get())) break;

#ifdef GNASH_DEBUG
            log_debug("Sprite %s (depth %d) won't check for user-defined "
                        "LOAD event (is not dynamic, has a parent, "
                        "no registered class and no clip events defined)",
                        getTarget(), get_depth());
#endif
            return;
        } while (0);
            
    }

    // Call the appropriate member function.
    sendEvent(*getObject(this), get_environment(), id.functionURI());

}

as_object*
MovieClip::pathElement(const ObjectURI& uri)
{
    as_object* obj = DisplayObject::pathElement(uri);
    if (obj) return obj;

    // See if we have a match on the display list.
    obj = getObject(getDisplayListObject(uri));
    if (obj) return obj;

    obj = getObject(this);
    assert(obj);

    // See if it's a member
    as_value tmp;
    if (!obj->as_object::get_member(uri, &tmp)) return 0;
    if (!tmp.is_object()) return 0;

    if (tmp.is_sprite()) {
        return getObject(tmp.toDisplayObject(true));
    }

    return toObject(tmp, getVM(*getObject(this)));
}

bool
MovieClip::setTextFieldVariables(const ObjectURI& uri, const as_value& val)
{
    // Try textfield variables
    TextFields* etc = textfieldVar(_text_variables.get(), uri);

    if (!etc) return false;

    for (TextFields::iterator i=etc->begin(), e=etc->end(); i!=e; ++i) {
        (*i)->updateText(val.to_string(getSWFVersion(*getObject(this))));
    }
    return true;
}

/// Remove the 'contents' of the MovieClip, but leave properties and
/// event handlers intact.
void
MovieClip::unloadMovie()
{
    LOG_ONCE(log_unimpl(_("MovieClip.unloadMovie()")));
}

void
MovieClip::queueLoad()
{
    if ( ! _onLoadCalled ) {
        _onLoadCalled = true;
        // We don't call onLoad for _root up to SWF5
        if ( ! parent() && getSWFVersion(*getObject(this)) < 6 ) return;
        queueEvent(event_id(event_id::LOAD),
                    movie_root::PRIORITY_DOACTION);
    }
}

// child movieclip advance
void
MovieClip::advance()
{
#ifdef GNASH_DEBUG
    log_debug("Advance movieclip '%s' at frame %u/%u",
        getTargetPath(), _currentFrame+1,
        get_frame_count());
#endif

    assert(!unloaded());

    // call_frame should never trigger advance_movieclip
    assert(!_callingFrameActions);

    // We might have loaded NO frames !
    if (get_loaded_frames() == 0) {
        IF_VERBOSE_MALFORMED_SWF(
        LOG_ONCE( log_swferror(_("advance_movieclip: no frames loaded "
                    "for movieclip/movie %s"), getTarget()) );
        );
        return;
    }

    // Process any pending loadVariables request
    processCompletedLoadVariableRequests();

#ifdef GNASH_DEBUG
    size_t frame_count = _def->get_frame_count();

    log_debug("Advance_movieclip for movieclip '%s' - frame %u/%u ",
        getTarget(), _currentFrame+1,
        frame_count);
#endif

    queueLoad();

    // I'm not sure ENTERFRAME goes in a different queue then DOACTION...
    queueEvent(event_id(event_id::ENTER_FRAME), movie_root::PRIORITY_DOACTION);

    // Update current and next frames.
    if (_playState == PLAYSTATE_PLAY) {
#ifdef GNASH_DEBUG
        log_debug("MovieClip::advance_movieclip we're in PLAYSTATE_PLAY mode");
#endif

        const size_t prev_frame = _currentFrame;

#ifdef GNASH_DEBUG
        log_debug("on_event_load called, incrementing");
#endif
        increment_frame_and_check_for_loop();
#ifdef GNASH_DEBUG
        log_debug("after increment we are at frame %u/%u", _currentFrame, frame_count);
#endif

        // Flush any orphaned tags
        // See https://savannah.gnu.org/bugs/index.php?33176
        // WARNING: we might be executing these while a parser
        //          is still pushing on it. The _hasLooped is
        //          trying to avoid that.
        // TODO: find a better way to ensure nobody will be pushing
        //       to orphaned playlist while we execute it.
        if (_currentFrame == 0 && _hasLooped) {

            const size_t frame_count = get_loaded_frames(); 
            if ( frame_count != 1 || ! _flushedOrphanedTags ) {
                IF_VERBOSE_ACTION(
                log_action(_("Flushing orphaned tags in movieclip %1%. "
                    "_currentFrame:%2%, _hasLooped:%3%, frame_count:%4%"),
                    getTargetPath(), _currentFrame, _hasLooped, frame_count)
                );
                _flushedOrphanedTags = true;
                executeFrameTags(frame_count, _displayList,
                    SWF::ControlTag::TAG_DLIST |
                    SWF::ControlTag::TAG_ACTION);
            }
        }

        // Execute the current frame's tags.
        // First time executeFrameTags(0) executed in dlist.cpp(child) or
        // SWFMovieDefinition(root)
        if (_currentFrame != prev_frame) {

            if (_currentFrame == 0 && _hasLooped) {
#ifdef GNASH_DEBUG
                log_debug("Jumping back to frame 0 of movieclip %s",
                        getTarget());
#endif
                restoreDisplayList(0); // seems OK to me.
            }
            else {
#ifdef GNASH_DEBUG
                log_debug("Executing frame%d (0-based) tags of movieclip "
                            "%s", _currentFrame, getTarget());
#endif
                // Make sure _currentFrame is 0-based during execution of
                // DLIST tags
                executeFrameTags(_currentFrame, _displayList,
                        SWF::ControlTag::TAG_DLIST |
                        SWF::ControlTag::TAG_ACTION);
            }
        }

    }
#ifdef GNASH_DEBUG
    else {
        log_debug("MovieClip::advance_movieclip we're in STOP mode");
    }
#endif
}

void
MovieClip::execute_init_action_buffer(const action_buffer& a, int cid)
{
    assert(cid >= 0);

    if (_swf->initializeCharacter(cid)) {
#ifdef GNASH_DEBUG
        log_debug("Queuing init actions for DisplayObject %1% "
                    "in frame %2% of MovieClip %3%",
                cid, _currentFrame, getTarget());
#endif
        std::auto_ptr<ExecutableCode> code(new GlobalCode(a, this));

        stage().pushAction(code, movie_root::PRIORITY_INIT);
    }
    else {
#ifdef GNASH_DEBUG
        log_debug("Init actions for DisplayObject %1% already executed", cid);
#endif
    }
}

void
MovieClip::execute_action(const action_buffer& ab)
{
    ActionExec exec(ab, _environment);
    exec();
}

void
MovieClip::restoreDisplayList(size_t tgtFrame)
{
    // This is not tested as usable for jump-forwards (yet)...
    // TODO: I guess just moving here the code currently in goto_frame
    //             for jump-forwards would do
    assert(tgtFrame <= _currentFrame);

    DisplayList tmplist;
    for (size_t f = 0; f < tgtFrame; ++f) {
        _currentFrame = f;
        executeFrameTags(f, tmplist, SWF::ControlTag::TAG_DLIST);
    }

    // Execute both action tags and DLIST tags of the target frame
    _currentFrame = tgtFrame;
    executeFrameTags(tgtFrame, tmplist, SWF::ControlTag::TAG_DLIST |
                                        SWF::ControlTag::TAG_ACTION);

    _displayList.mergeDisplayList(tmplist, *this);
}

// 0-based frame number !
void
MovieClip::executeFrameTags(size_t frame, DisplayList& dlist, int typeflags)
{
    // If there is no definition, this is a dynamically-created MovieClip
    // and has no frames.
    if (!_def) return;
    if (isDestroyed()) return;

    assert(typeflags);

    const PlayList* playlist = _def->getPlaylist(frame);
    if (playlist) {
    
        IF_VERBOSE_ACTION(
            // Use 1-based frame numbers
            log_action(_("Executing %d tags in frame %d/%d of movieclip %s"),
                playlist->size(), frame + 1, get_frame_count(),
                getTargetPath());
        );

        // Generally tags should be executed in the order they are found in.
        for (PlayList::const_iterator it = playlist->begin(),
                e = playlist->end(); it != e; ++it) {

            if (typeflags & SWF::ControlTag::TAG_DLIST) {
                (*it)->executeState(this, dlist);
            }

            if (typeflags & SWF::ControlTag::TAG_ACTION) {
                (*it)->executeActions(this, _displayList);
            }
        }
    }
}

void
MovieClip::goto_frame(size_t target_frame_number)
{
#if defined(DEBUG_GOTOFRAME) || defined(GNASH_DEBUG_TIMELINE)
    log_debug("movieclip %s ::goto_frame(%d) - current frame is %d",
        getTargetPath(), target_frame_number, _currentFrame);
#endif

    // goto_frame stops by default.
    // ActionGotoFrame tells the movieClip to go to the target frame 
    // and stop at that frame. 
    setPlayState(PLAYSTATE_STOP);

    if (target_frame_number > _def->get_frame_count() - 1) {

        target_frame_number = _def->get_frame_count() - 1;

        if (!_def->ensure_frame_loaded(target_frame_number + 1)) {
            log_error(_("Target frame of a gotoFrame(%d) was never loaded, "
                        "although frame count in header (%d) said we "
                        "should have found it"),
                        target_frame_number+1, _def->get_frame_count());
            return; 
        }

        // Just set _currentframe and return.
        _currentFrame = target_frame_number;

        // don't push actions, already tested.
        return;
    }

    if (target_frame_number == _currentFrame) {
        // don't push actions
        return;
    }

    // Unless the target frame is the next one, stop playback of soundstream
    if (target_frame_number != _currentFrame + 1) {
        stopStreamSound();
    }

    const size_t loaded_frames = get_loaded_frames();

    // target_frame_number is 0-based, get_loaded_frames() is 1-based
    // so in order to goto_frame(3) loaded_frames must be at least 4
    // if goto_frame(4) is called, and loaded_frames is 4 we're jumping
    // forward
    if (target_frame_number >= loaded_frames) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("GotoFrame(%d) targets a yet "
            "to be loaded frame (%d). "
            "We'll wait for it but a more correct form "
            "is explicitly using WaitForFrame instead"),
            target_frame_number+1,
            loaded_frames);

        );
        if (!_def->ensure_frame_loaded(target_frame_number + 1)) {
            log_error(_("Target frame of a gotoFrame(%d) was never loaded, "
                        "although frame count in header (%d) said we should"
                        " have found it"),
                        target_frame_number + 1, _def->get_frame_count());
            return;
        }
    }

    // Construct the DisplayList of the target frame
    if (target_frame_number < _currentFrame) {

        // Go backward to a previous frame
        // NOTE: just in case we're being called by code in a called frame
        // we'll backup and resume the _callingFrameActions flag
        bool callingFrameActionsBackup = _callingFrameActions;
        _callingFrameActions = false;

        // restoreDisplayList takes care of properly setting the 
        // _currentFrame variable
        restoreDisplayList(target_frame_number);
        assert(_currentFrame == target_frame_number);
        _callingFrameActions = callingFrameActionsBackup;
    }
    else {
        // Go forward to a later frame
        // We'd immediately return if target_frame_number == _currentFrame
        assert(target_frame_number > _currentFrame);
        while (++_currentFrame < target_frame_number) {
            //for (size_t f = _currentFrame+1; f<target_frame_number; ++f) 
            // Second argument requests that only "DisplayList" tags
            // are executed. This means NO actions will be
            // pushed on m_action_list.
            executeFrameTags(_currentFrame, _displayList,
                    SWF::ControlTag::TAG_DLIST);
        }
        assert(_currentFrame == target_frame_number);

        // Now execute target frame tags (queuing actions)
        // NOTE: just in case we're being called by code in a called frame
        //             we'll backup and resume the _callingFrameActions flag
        bool callingFrameActionsBackup = _callingFrameActions;
        _callingFrameActions = false;
        executeFrameTags(target_frame_number, _displayList,
                SWF::ControlTag::TAG_DLIST | SWF::ControlTag::TAG_ACTION);
        _callingFrameActions = callingFrameActionsBackup;
    }

    assert(_currentFrame == target_frame_number);
}

bool
MovieClip::goto_labeled_frame(const std::string& label)
{
    // If there is no definition, this is a dynamically-created MovieClip
    // and has no frames. (We are also probably not called in this case).
    if (!_def) return false;

    size_t target_frame;
    if (_def->get_labeled_frame(label, target_frame)) {
        goto_frame(target_frame);
        return true;
    }

    IF_VERBOSE_MALFORMED_SWF(
        log_swferror(_("MovieClip::goto_labeled_frame('%s') "
            "unknown label"), label);
    );
    return false;
}

void
MovieClip::draw(Renderer& renderer, const Transform& xform)
{
    const DisplayObject::MaskRenderer mr(renderer, *this);

    _drawable.finalize();
    _drawable.display(renderer, xform);
    _displayList.display(renderer, xform);
}

void
MovieClip::display(Renderer& renderer, const Transform& base)
{
    // Note: DisplayList::display() will take care of the visibility checking.
    //
    // Whether a DisplayObject should be rendered or not is dependent
    // on its parent: i.e. if its parent is a mask, this DisplayObject
    // should be rendered to the mask buffer even it is invisible.
    
    // Draw everything with our own transform.
    const Transform xform = base * transform();
    draw(renderer, xform);
    clear_invalidated();
}

void MovieClip::omit_display()
{
    if (childInvalidated()) _displayList.omit_display();
    clear_invalidated();
}

void
MovieClip::attachCharacter(DisplayObject& newch, int depth, as_object* initObj)
{ 
    _displayList.placeDisplayObject(&newch, depth);
    newch.construct(initObj);
}

DisplayObject*
MovieClip::add_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{
    // If this MovieClip has no definition, it should also have no ControlTags,
    // and this shouldn't be called.
    assert(_def);
    assert(tag);

    // No tags should ever be executed on destroyed MovieClips.
    assert(!isDestroyed());

    SWF::DefinitionTag* cdef = _def->getDefinitionTag(tag->getID());
    if (!cdef) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("MovieClip::add_display_object(): "
                    "unknown cid = %d"), tag->getID());
        );
        return NULL;
    }
    
    DisplayObject* existing_char = dlist.getDisplayObjectAtDepth(tag->getDepth());
    
    if (existing_char) return NULL;

    Global_as& gl = getGlobal(*getObject(this));
    VM& vm = getVM(*getObject(this));
    DisplayObject* ch = cdef->createDisplayObject(gl, this);

    if (tag->hasName()) ch->set_name(getURI(vm, tag->getName()));
    else if (isReferenceable(*ch)) {
        const ObjectURI& instance_name = getNextUnnamedInstanceName();
        ch->set_name(instance_name);
    }

    if (tag->hasBlendMode()) {
        boost::uint8_t bm = tag->getBlendMode();
        ch->setBlendMode(static_cast<DisplayObject::BlendMode>(bm));
    }

    // Attach event handlers (if any).
    const SWF::PlaceObject2Tag::EventHandlers& event_handlers =
        tag->getEventHandlers();

    for (size_t i = 0, n = event_handlers.size(); i < n; ++i) {
        const swf_event& ev = event_handlers[i];
        ch->add_event_handler(ev.event(), ev.action());
    }

    // TODO: check if we should check those has_xxx flags first.
    ch->setCxForm(tag->getCxform());
    ch->setMatrix(tag->getMatrix(), true); // update caches
    ch->set_ratio(tag->getRatio());
    ch->set_clip_depth(tag->getClipDepth());
    
    dlist.placeDisplayObject(ch, tag->getDepth());
    ch->construct();
    return ch;
}

void 
MovieClip::move_display_object(const SWF::PlaceObject2Tag* tag, DisplayList& dlist)
{    
    boost::uint16_t ratio = tag->getRatio();
    // clip_depth is not used in MOVE tag(at least no related tests). 
    dlist.moveDisplayObject(
        tag->getDepth(), 
        tag->hasCxform() ? &tag->getCxform() : NULL,
        tag->hasMatrix() ? &tag->getMatrix() : NULL,
        tag->hasRatio() ? &ratio : NULL);
}

void
MovieClip::replace_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{
    // A MovieClip without a definition cannot have any ControlTags, so this
    // should not be called.
    assert(_def);
    assert(tag != NULL);

    const boost::uint16_t id = tag->getID();

    SWF::DefinitionTag* cdef = _def->getDefinitionTag(id);
    if (!cdef) {
        log_error(_("movieclip::replace_display_object(): "
            "unknown cid = %d"), id);
        return;
    }
    assert(cdef);

    DisplayObject* existing_char = dlist.getDisplayObjectAtDepth(tag->getDepth());

    if (!existing_char) {
        log_error(_("MovieClip::replace_display_object: could not "
                    "find any DisplayObject at depth %d"), tag->getDepth());
        return;
    }

    // if the existing DisplayObject is not a shape, move it instead
    // of replacing.
    if (isReferenceable(*existing_char)) {
        move_display_object(tag, dlist);
        return;
    }

    Global_as& gl = getGlobal(*getObject(this));
    DisplayObject* ch = cdef->createDisplayObject(gl, this);
    

    // TODO: check if we can drop this for REPLACE!
    // should we rename the DisplayObject when it's REPLACE tag?
    if (tag->hasName()) {
        VM& vm = getVM(*getObject(this));
        ch->set_name(getURI(vm, tag->getName()));
    }
    else if (isReferenceable(*ch)) {
        ch->set_name(getNextUnnamedInstanceName());
    }
    if (tag->hasRatio()) {
        ch->set_ratio(tag->getRatio());
    }
    if (tag->hasCxform()) {
        ch->setCxForm(tag->getCxform());
    }
    if (tag->hasMatrix()) {
        ch->setMatrix(tag->getMatrix(), true); 
    }

    // use SWFMatrix from the old DisplayObject if tag doesn't provide one.
    dlist.replaceDisplayObject(ch, tag->getDepth(), 
        !tag->hasCxform(), !tag->hasMatrix());
    ch->construct();
}

void
MovieClip::remove_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{
    set_invalidated();
    dlist.removeDisplayObject(tag->getDepth());
}

void
MovieClip::remove_display_object(int depth, int)
{
    set_invalidated();
    _displayList.removeDisplayObject(depth);
}

void
MovieClip::increment_frame_and_check_for_loop()
{
    const size_t frame_count = get_loaded_frames(); 
    if (++_currentFrame >= frame_count) {
        // Loop.
        _currentFrame = 0;
        _hasLooped = true;
        // Make sure the streaming sound can start again.
        stopStreamSound();
    }
}

bool
MovieClip::handleFocus()
{
    as_object* obj = getObject(this);
    assert(obj);

    // For SWF6 and above: the MovieClip can always receive focus if
    // focusEnabled evaluates to true.
    if (getSWFVersion(*obj) > 5) {
        as_value focusEnabled;
        if (obj->get_member(NSV::PROP_FOCUS_ENABLED, &focusEnabled)) {
            if (toBool(focusEnabled, getVM(*obj))) return true; 
        }
    }
        
    // If focusEnabled doesn't evaluate to true or for SWF5, return true
    // only if at least one mouse event handler is defined.
    return mouseEnabled();
}

bool
MovieClip::pointInShape(boost::int32_t x, boost::int32_t y) const
{
    ShapeContainerFinder finder(x, y);
    _displayList.visitBackward(finder);
    if ( finder.hitFound() ) return true;
    return hitTestDrawable(x, y);
}

bool
MovieClip::pointInVisibleShape(boost::int32_t x, boost::int32_t y) const
{
    if (! visible()) return false;
    if (isDynamicMask() && ! mouseEnabled()) {
        // see testsuite/misc-ming.all/masks_test.swf
#ifdef GNASH_DEBUG_HITTEST
        log_debug("%s is a dynamic mask and can't handle mouse "
                    "events, no point will hit it", getTarget());
#endif
        return false;
    }
    const DisplayObject* mask = getMask(); // dynamic one
    if (mask && mask->visible() && !mask->pointInShape(x, y)) {
#ifdef GNASH_DEBUG_HITTEST
        log_debug("%s is dynamically masked by %s, which "
                "doesn't hit point %g,%g", getTarget(),
                mask->getTarget(), x, y);
#endif
        return false;
    }
    VisibleShapeContainerFinder finder(x, y);
    _displayList.visitBackward(finder);
    if (finder.hitFound()) return true;
    return hitTestDrawable(x, y);
}

inline bool
MovieClip::hitTestDrawable(boost::int32_t x, boost::int32_t y) const
{
    const SWFMatrix wm = getWorldMatrix(*this).invert();
    point lp(x, y);
    wm.transform(lp);
    if (!_drawable.getBounds().point_test(lp.x, lp.y)) return false;
    return _drawable.pointTestLocal(lp.x, lp.y, wm);
}

bool
MovieClip::pointInHitableShape(boost::int32_t x, boost::int32_t y) const
{
    if (isDynamicMask() && !mouseEnabled()) return false;

    const DisplayObject* mask = getMask(); 
    if (mask && !mask->pointInShape(x, y)) return false;
            
    HitableShapeContainerFinder finder(x, y);
    _displayList.visitBackward(finder);
    if (finder.hitFound()) return true; 
    
    return hitTestDrawable(x, y); 
}

InteractiveObject*
MovieClip::topmostMouseEntity(boost::int32_t x, boost::int32_t y)
{
    if (!visible()) return 0;

    // point is in parent's space, we need to convert it in world space
    point wp(x, y);
    DisplayObject* p = parent();
    if (p) {
        // WARNING: if we have NO parent, our parent is the Stage (movie_root)
        //          so, in case we'll add a "stage" matrix, we'll need to take
        //          it into account here.
        // TODO: actually, why are we insisting in using parent's
        //          coordinates for this method at all ?
        getWorldMatrix(*p).transform(wp);
    }

    if (mouseEnabled()) {
        if (pointInVisibleShape(wp.x, wp.y)) return this;
        return 0;
    }

    SWFMatrix m = getMatrix(*this);
    m.invert();
    point pp(x, y);
    m.transform(pp);

    MouseEntityFinder finder(wp, pp);
    _displayList.visitAll(finder);
    InteractiveObject* ch = finder.getEntity();

    // It doesn't make any sense to query _drawable, as it's
    // not an InteractiveObject.
    return ch; 
}

const DisplayObject*
MovieClip::findDropTarget(boost::int32_t x, boost::int32_t y,
        DisplayObject* dragging) const
{
    if (this == dragging) return 0; // not here...

    if (!visible()) return 0; // isn't me !

    DropTargetFinder finder(x, y, dragging);
    _displayList.visitAll(finder);

    // does it hit any child ?
    const DisplayObject* ch = finder.getDropChar();
    if (ch) {
        // TODO: find closest actionscript referenceable container
        //             (possibly itself)
        return ch;
    }

    // does it hit us ?
    if (hitTestDrawable(x, y)) return this;

    return 0;
}

bool
MovieClip::trackAsMenu()
{
    as_object* obj = getObject(this);
    assert(obj);

    as_value track;
    VM& vm = getVM(*obj);
    // TODO: use namedStrings here
    return (obj->get_member(getURI(vm, "trackAsMenu"), &track) &&
            toBool(track, vm));
}

bool
MovieClip::mouseEnabled() const
{
    if (!isEnabled()) return false;

    // Event handlers that qualify as mouse event handlers.
    static const event_id EH[] = {
        event_id(event_id::PRESS),
        event_id(event_id::RELEASE),
        event_id(event_id::RELEASE_OUTSIDE),
        event_id(event_id::ROLL_OVER),
        event_id(event_id::ROLL_OUT),
        event_id(event_id::DRAG_OVER),
        event_id(event_id::DRAG_OUT),
    };

    const size_t size = arraySize(EH);

    for (size_t i = 0; i < size; ++i) {
        const event_id &event = EH[i];

        // Check event handlers
        if (hasEventHandler(event_id(event.id()))) {
            return true;
        }
    }
    return false;
}

void
MovieClip::set_background_color(const rgba& color)
{
    stage().set_background_color(color);
}

void
MovieClip::cleanup_textfield_variables()
{
    // nothing to do
    if (!_text_variables.get()) return;

    TextFieldIndex& m = *_text_variables;

    for (TextFieldIndex::iterator i=m.begin(), ie=m.end(); i!=ie; ++i)
    {
        TextFields& v=i->second;
        TextFields::iterator lastValid = std::remove_if(v.begin(), v.end(),
                    boost::mem_fn(&DisplayObject::unloaded));
        v.erase(lastValid, v.end());
    }
}


void
MovieClip::set_textfield_variable(const ObjectURI& name, TextField* ch)
{
    assert(ch);

    // lazy allocation
    if (!_text_variables.get()) {
        _text_variables.reset(new TextFieldIndex);
    }
    
    (*_text_variables)[name].push_back(ch);
}

DisplayObject*
MovieClip::getDisplayListObject(const ObjectURI& uri)
{
    as_object* obj = getObject(this);
    assert(obj);

    string_table& st = getStringTable(*obj);

    // Try items on our display list.
    DisplayObject* ch = _displayList.getDisplayObjectByName(st, uri,
            caseless(*obj));

    if (!ch) return 0;

    // Found object.

    // If the object is an ActionScript referenciable one we
    // return it, otherwise we return ourselves
    if (isReferenceable(*ch)) {
        return ch;
    }
    return this;
}

void 
MovieClip::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{
    // nothing to do if this movieclip is not visible
    if (!visible() || invisible(getCxForm(*this))) {
        ranges.add(m_old_invalidated_ranges); 
        return;
    }

    if (!invalidated() && !childInvalidated() && !force) return;
 
    // m_child_invalidated does not require our own bounds
    if (invalidated() || force) {
        // Add old invalidated bounds
        ranges.add(m_old_invalidated_ranges); 
    }
    
    _displayList.add_invalidated_bounds(ranges, force || invalidated());

    /// Add drawable.
    SWFRect bounds;
    bounds.expand_to_transformed_rect(getWorldMatrix(*this),
            _drawable.getBounds());

    ranges.add(bounds.getRange());
}


void
MovieClip::constructAsScriptObject()
{
    as_object* mc = getObject(this);
    
    // A MovieClip should always have an associated object.
    assert(mc);

    if (!parent()) {
        mc->init_member("$version", getVM(*mc).getPlayerVersion(), 0); 
    }

    const sprite_definition* def = 
        dynamic_cast<const sprite_definition*>(_def.get());

    // We won't "construct" top-level movies
    as_function* ctor = def ? stage().getRegisteredClass(def) : 0;

#ifdef GNASH_DEBUG
    log_debug("Attached movieclips %s registered class is %p",
            getTarget(), (void*)ctor); 
#endif

    // Set this MovieClip object to be an instance of the class.
    if (ctor) {
        Property* proto = ctor->getOwnProperty(NSV::PROP_PROTOTYPE);
        if (proto) mc->set_prototype(proto->getValue(*ctor));
    }

    // Send the construct event. This must be done after the __proto__ 
    // member is set. It is always done.
    notifyEvent(event_id(event_id::CONSTRUCT));
        
    if (ctor) {
        const int swfversion = getSWFVersion(*mc);
        if (swfversion > 5) {
            fn_call::Args args;
            ctor->construct(*mc, get_environment(), args);
        }
    }
}

void
MovieClip::construct(as_object* initObj)
{
    assert(!unloaded());

    saveOriginalTarget();

#ifdef GNASH_DEBUG
    log_debug("Sprite '%s' placed on stage", getTarget());
#endif

    // Register this movieclip as a live one
    stage().addLiveChar(this);

    // It seems it's legal to place 0-framed movieclips on stage.
    // See testsuite/misc-swfmill.all/zeroframe_definemovieclip.swf

    // Now execute frame tags and take care of queuing the LOAD event.
    //
    // DLIST tags are executed immediately while ACTION tags are queued.
    //
    // For clips w/out event handlers, LOAD event is invoked *after*
    // actions in first frame
    // See misc-ming.all/action_order/action_execution_order_test4.{c,swf}
    //
    assert(!_callingFrameActions); // or will not be queuing actions

    if ( ! get_event_handlers().empty() ) {
        queueLoad();
    }

    executeFrameTags(0, _displayList, SWF::ControlTag::TAG_DLIST |
            SWF::ControlTag::TAG_ACTION);

    // We execute events immediately when the stage-placed DisplayObject 
    // is dynamic, This is becase we assume that this means that 
    // the DisplayObject is placed during processing of actions (opposed 
    // that during advancement iteration).
    //
    // A more general implementation might ask movie_root about its state
    // (iterating or processing actions?)
    // Another possibility to inspect could be letting movie_root decide
    // when to really queue and when rather to execute immediately the 
    // events with priority INITIALIZE or CONSTRUCT ...
    if (!isDynamic()) {

#ifdef GNASH_DEBUG
        log_debug("Queuing INITIALIZE and CONSTRUCT events for movieclip %s",
                getTarget());
#endif

        std::auto_ptr<ExecutableCode> code(new ConstructEvent(this));
        stage().pushAction(code, movie_root::PRIORITY_CONSTRUCT);

    }
    else {

        // Properties from an initObj must be copied before construction, but
        // after the display list has been populated, so that _height and
        // _width (which depend on bounds) are correct.
        if (initObj) {

            as_object* mc = getObject(this);
            
            // A MovieClip should always have an associated object.
            assert(mc);

            mc->copyProperties(*initObj);
        }
        constructAsScriptObject();
    }

    // Tested in testsuite/swfdec/duplicateMovieclip-events.c and
    // testsuite/swfdec/clone-sprite-events.c not to call notifyEvent
    // immediately.
    queueEvent(event_id(event_id::INITIALIZE), movie_root::PRIORITY_INIT);
}

bool
MovieClip::unloadChildren()
{
#ifdef GNASH_DEBUG
    log_debug("Unloading movieclip '%s'", getTargetPath());
#endif

    // stop any pending streaming sounds
    stopStreamSound();

    // We won't be displayed again, so worth releasing
    // some memory. The drawable might take a lot of memory
    // on itself.
    _drawable.clear();
    
    const bool childHandler = _displayList.unload();

    if (!unloaded()) {
        queueEvent(event_id(event_id::UNLOAD), movie_root::PRIORITY_DOACTION);
    }

    // Check whether our child MovieClips or this MovieCLip have an unload
    // handler.
    const bool unloadHandler = 
        childHandler || hasEventHandler(event_id(event_id::UNLOAD));

    // If there's no unload handler, make sure any queued constructor for
    // this MovieClip is not executed!
    if (!unloadHandler) {
        stage().removeQueuedConstructor(this);
    }

    return unloadHandler;
}

void
MovieClip::getLoadedMovie(Movie* extern_movie)
{
    DisplayObject* p = parent();
    if (p) {
        extern_movie->set_parent(p);

        // Copy own lockroot value
        extern_movie->setLockRoot(getLockRoot());

        // Copy own event handlers
        // see testsuite/misc-ming.all/loadMovieTest.swf
        const Events& clipEvs = get_event_handlers();
        // top-level movies can't have clip events, right ?
        assert (extern_movie->get_event_handlers().empty());
        extern_movie->set_event_handlers(clipEvs);

        // Copy own name
        // TODO: check empty != none...
        const ObjectURI& name = get_name();
        if (!name.empty()) extern_movie->set_name(name);

        // Copy own clip depth (TODO: check this)
        extern_movie->set_clip_depth(get_clip_depth());

        // Replace ourselves in parent
        // TODO: don't pretend our parent is a MovieClip,
        //       could as well be a button I guess...
        //       At most we should require it to be a
        //       DisplayObjectContainer and log an error if it's not.
        MovieClip* parent_sp = p->to_movie();
        assert(parent_sp);
        parent_sp->_displayList.replaceDisplayObject(extern_movie, get_depth(),
                true, true);
        extern_movie->construct();
    }
    else {
        // replaceLevel will set depth for us
        stage().replaceLevel(get_depth() - DisplayObject::staticDepthOffset,
                              extern_movie);
    }
}

void 
MovieClip::loadVariables(const std::string& urlstr, 
        VariablesMethod sendVarsMethod)
{
    // Host security check will be will be done by LoadVariablesThread
    // (down by getStream, that is)
    const movie_root& mr = stage();
    URL url(urlstr, mr.runResources().streamProvider().baseURL());

    std::string postdata;
    
    // Encode our vars for sending.
    if (sendVarsMethod != METHOD_NONE) {
        postdata = getURLEncodedVars(*getObject(this));
    }

    try {
        const StreamProvider& sp = 
            getRunResources(*getObject(this)).streamProvider();
        
        if (sendVarsMethod == METHOD_POST) {
            // use POST method
            _loadVariableRequests.push_back(
                    new LoadVariablesThread(sp, url, postdata));
        }
        else {
            // use GET method
            if (sendVarsMethod == METHOD_GET) {
                // Append variables
                std::string qs = url.querystring();
                if (qs.empty()) url.set_querystring(postdata);
                else url.set_querystring(qs + "&" + postdata);
            }
            _loadVariableRequests.push_back(new LoadVariablesThread(sp, url));
        }
        _loadVariableRequests.back().process();
    }
    catch (const NetworkException& ex) {
        log_error(_("Could not load variables from %s"), url.str());
    }
}

void
MovieClip::processCompletedLoadVariableRequest(LoadVariablesThread& request)
{
    assert(request.completed());

    MovieVariables& vals = request.getValues();
    setVariables(vals);

    // We want to call a clip-event too if available, see bug #22116
    notifyEvent(event_id(event_id::DATA));
}

void
MovieClip::processCompletedLoadVariableRequests()
{
    // Nothing to do (just for clarity)
    if (_loadVariableRequests.empty()) return;

    for (LoadVariablesThreads::iterator it=_loadVariableRequests.begin();
            it != _loadVariableRequests.end();) {

        LoadVariablesThread& request = *it;
        if (request.completed()) {
            processCompletedLoadVariableRequest(request);
            it = _loadVariableRequests.erase(it);
        }
        else ++it;
    }
}

void
MovieClip::setVariables(const MovieVariables& vars)
{
    VM& vm = getVM(*getObject(this));
    for (MovieVariables::const_iterator it=vars.begin(), itEnd=vars.end();
        it != itEnd; ++it) {

        const std::string& name = it->first;
        const std::string& val = it->second;
        getObject(this)->set_member(getURI(vm, name), val);
    }
}

void
MovieClip::removeMovieClip()
{
    const int depth = get_depth();
    if (depth < 0 || depth > 1048575) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("removeMovieClip(%s): movieclip depth (%d) out of "
                "the 'dynamic' zone [0..1048575], won't remove"),
                getTarget(), depth);
        );
        return;
    }

    MovieClip* p = dynamic_cast<MovieClip*>(parent());
    if (p) {
        // second argument is arbitrary, see comments above
        // the function declaration in MovieClip.h
        p->remove_display_object(depth, 0);
    }
    else {
        // removing _level#
        stage().dropLevel(depth);
        // I guess this can only happen if someone uses 
        // _swf.swapDepth([0..1048575])
    }

}

SWFRect
MovieClip::getBounds() const
{
    SWFRect bounds;
    BoundsFinder f(bounds);
    _displayList.visitAll(f);
    SWFRect drawableBounds = _drawable.getBounds();
    bounds.expand_to_rect(drawableBounds);
    
    return bounds;
}

bool
MovieClip::isEnabled() const
{
    as_object* obj = getObject(this);
    assert(obj);

    as_value enabled;
    if (!obj->get_member(NSV::PROP_ENABLED, &enabled)) {
         // We're enabled if there's no 'enabled' member...
         return true;
    }
    return toBool(enabled, getVM(*obj));
}


void
MovieClip::visitNonProperties(KeyVisitor& v) const
{
    DisplayListVisitor dv(v);
    _displayList.visitAll(dv);
}

void
MovieClip::cleanupDisplayList()
{
    _displayList.removeUnloaded();
    cleanup_textfield_variables();
}

void
MovieClip::markOwnResources() const
{
    ReachableMarker marker;

    _displayList.visitAll(marker);

    _environment.markReachableResources();

    // Mark textfields in the TextFieldIndex
    if (_text_variables.get()) {
        for (TextFieldIndex::const_iterator i=_text_variables->begin(),
                    e=_text_variables->end(); i!=e; ++i) {

            const TextFields& tfs=i->second;
            std::for_each(tfs.begin(), tfs.end(), 
                        boost::mem_fn(&DisplayObject::setReachable));
        }
    }

    // Mark our relative root
    _swf->setReachable();
}

void
MovieClip::destroy()
{
    stopStreamSound();
    _displayList.destroy();
    DisplayObject::destroy();
}

Movie*
MovieClip::get_root() const
{
    return _swf;
}

MovieClip*
MovieClip::getAsRoot()
{

    // TODO1: as an optimization, if swf version < 7 
    //                we might as well just return _swf, 
    //                the whole chain from this movieclip to it's
    //                _swf should have the same version...
    DisplayObject* p = parent();

    // no parent, we're the root
    if (!p) return this; 

    // If we have a parent, we descend to it unless 
    // our _lockroot is true AND our or the VM's
    // SWF version is > 6
    const int topSWFVersion = stage().getRootMovie().version();

    if (getDefinitionVersion() > 6 || topSWFVersion > 6) {
        if (getLockRoot()) return this;
    }

    return p->getAsRoot();
}


void
MovieClip::setStreamSoundId(int id)
{
    if (id != m_sound_stream_id) {
        log_debug("Stream sound id from %d to %d, stopping old",
                m_sound_stream_id, id);
        stopStreamSound();
    }
    m_sound_stream_id = id;
}

void
MovieClip::stopStreamSound()
{
    if (m_sound_stream_id == -1) return; // nothing to do

    sound::sound_handler* handler =
        getRunResources(*getObject(this)).soundHandler();

    if (handler) {
        handler->stopStreamingSound(m_sound_stream_id);
    }

    stage().stopStream(m_sound_stream_id);

    m_sound_stream_id = -1;
}

void
MovieClip::setPlayState(PlayState s)
{
    if (s == _playState) return; // nothing to do
    if (s == PLAYSTATE_STOP) stopStreamSound();
    _playState = s;
}

namespace {

MovieClip::TextFields*
textfieldVar(MovieClip::TextFieldIndex* t, const ObjectURI& name)
{
    // nothing allocated yet...
    if (!t) return 0;

    // TODO: should variable name be considered case-insensitive ?
    MovieClip::TextFieldIndex::iterator it = t->find(name);
    if (it == t->end()) return 0;
    return &(it->second);
} 

} // unnamed namespace
} // namespace gnash
