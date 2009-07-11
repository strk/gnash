// MovieClip.cpp:  Stateful live Sprite instance, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "MovieFactory.h"
#include "log.h" 
#include "action.h" // for call_method_parsed (call_method_args)
#include "MovieClip.h"
#include "movie_definition.h"
#include "as_value.h"
#include "as_function.h"
#include "Bitmap.h"
#include "TextField.h"
#include "ControlTag.h"
#include "fn_call.h"
#include "flash/ui/Keyboard_as.h"
#include "movie_root.h"
#include "Movie.h"
#include "swf_event.h"
#include "sprite_definition.h"
#include "ActionExec.h"
#include "builtin_function.h"
#include "smart_ptr.h" // GNASH_USE_GC
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
#include "Object.h" // for getObjectInterface
#include "DynamicShape.h" // for composition
#include "namedStrings.h"
#include "styles.h" // for cap_style_e and join_style_e enums
#include "PlaceObject2Tag.h" 
#include "NetStream_as.h"
#include "flash/display/BitmapData_as.h"
#include "flash/geom/Matrix_as.h"
#include "ExportableResource.h"
#include "GnashNumeric.h"
#include "InteractiveObject.h"
#include "DisplayObjectContainer.h"

// TODO: get rid of this include.
#include "flash/display/MovieClip_as.h"

#include <vector>
#include <string>
#include <algorithm> // for std::swap
#include <boost/algorithm/string/case_conv.hpp>
//#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

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

/// Anonymous namespace for module-private definitions
namespace {

/// ConstructEvent, used for queuing construction
//
/// It's execution will call constructAsScriptObject() 
/// on the target movieclip
///
class ConstructEvent: public ExecutableCode {

public:

    ConstructEvent(MovieClip* nTarget)
        :
        _target(nTarget)
    {}


    ExecutableCode* clone() const
    {
        return new ConstructEvent(*this);
    }

    virtual void execute()
    {
        _target->constructAsScriptObject();
    }

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///    - the action target (_target)
    ///
    virtual void markReachableResources() const
    {
        _target->setReachable();
    }
#endif // GNASH_USE_GC

private:

    MovieClip* _target;

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

    void operator() (DisplayObject* ch)
    {
        assert(!_checked);
        if ( ch->get_depth() <= _highestHiddenDepth )
        {
            if ( ch->isMaskLayer() )
            {
                log_debug(_("CHECKME: nested mask in MouseEntityFinder. "
                            "This mask is %s at depth %d outer mask masked "
                            "up to depth %d."),
                            ch->getTarget(), ch->get_depth(),
                            _highestHiddenDepth);
                // Hiding mask still in effect...
            }
            return;
        }

        if ( ch->isMaskLayer() )
        {
            if ( ! ch->pointInShape(_wp.x, _wp.y) )
            {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug(_("Character %s at depth %d is a mask not hitting "
                        "the query point %g,%g and masking up to "
                        "depth %d"), ch->getTarget(), ch->get_depth(), 
                        _wp.x, _wp.y, ch->get_clip_depth());
#endif
                _highestHiddenDepth = ch->get_clip_depth();
            }
            else
            {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug(_("Character %s at depth %d is a mask hitting the "
                        "query point %g,%g"),
                        ch->getTarget(), ch->get_depth(), _wp.x, _wp.y);
#endif 
            }
            return;
        }
        if (! ch->visible()) return;

        _candidates.push_back(ch);
    }

    void checkCandidates()
    {
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

    InteractiveObject* getEntity()
    {
        checkCandidates();
#ifdef DEBUG_MOUSE_ENTITY_FINDING
        if ( _m ) 
        {
            log_debug(_("MouseEntityFinder found DisplayObject %s (depth %d) "
                    "hitting point %g,%g"),
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

    bool operator() (DisplayObject* ch) {
        if (ch->pointInShape(_x, _y)) {
            _found = true;
            return false;
        }
        return true;
    }

    bool hitFound() { return _found; }

private:
    bool _found;
    boost::int32_t    _x;
    boost::int32_t    _y;
};

/// Find the first visible DisplayObject whose shape contain the point
//
/// Point coordinates in world TWIPS
///
class VisibleShapeContainerFinder
{
public:

    VisibleShapeContainerFinder(boost::int32_t x, boost::int32_t    y)
        :
        _found(false),
        _x(x),
        _y(y)
    {}

    bool operator() (DisplayObject* ch)
    {
        if (ch->pointInVisibleShape(_x, _y)) {
            _found = true;
            return false;
        }
        return true;
    }

    bool hitFound() { return _found; }

private:
    bool _found;
    boost::int32_t    _x;
    boost::int32_t    _y;
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

    bool operator() (DisplayObject* ch) 
    { 
        if (ch->isDynamicMask()) return true; 
        if (ch->pointInShape(_x, _y)) {
            _found = true; 
            return false; 
        } 
        return true; 
    } 

    bool hitFound() { return _found; } 

private:
    bool _found; 
    boost::int32_t _x; // TWIPS
    boost::int32_t _y; // TWIPS
}; 

/// A DisplayList visitor used to compute its overall bounds.
//
class BoundsFinder
{
public:
    BoundsFinder(rect& b) : _bounds(b) {}

    void operator() (DisplayObject* ch) {
        // don't include bounds of unloaded DisplayObjects
        if ( ch->unloaded() ) return;
        rect chb = ch->getBounds();
        SWFMatrix m = ch->getMatrix();
        _bounds.expand_to_transformed_rect(m, chb);
    }

private:
    rect& _bounds;
};

/// A DisplayList visitor used to extract script DisplayObjects
//
/// Script DisplayObjects are DisplayObjects created or transformed
/// by ActionScript. 
///
class ScriptObjectsFinder
{
public:
    ScriptObjectsFinder(std::vector<DisplayObject*>& dynamicChars,
            std::vector<DisplayObject*>& staticChars)
        :
        _dynamicChars(dynamicChars),
        _staticChars(staticChars)
    {}

    void operator() (DisplayObject* ch) {
        // don't include bounds of unloaded DisplayObjects
        if ( ch->unloaded() ) return;

        // TODO: Are script-transformed object to be kept ?
        //             Need a testcase for this
        //if ( ! ch->get_accept_anim_moves() )
        //if ( ch->isDynamic() )
        int depth = ch->get_depth();
        if (depth < DisplayObject::lowerAccessibleBound || depth >= 0) {
            _dynamicChars.push_back(ch);
        }
        else _staticChars.push_back(ch);
    }

private:
    std::vector<DisplayObject*>& _dynamicChars;
    std::vector<DisplayObject*>& _staticChars;
};

} // anonymous namespace


MovieClip::MovieClip(const movie_definition* const def, Movie* r,
        DisplayObject* parent, int id)
    :
    DisplayObjectContainer(parent, id),
    _def(def),
    _swf(r),
    _playState(PLAYSTATE_PLAY),
    _currentFrame(0),
    _hasLooped(false),
    _callingFrameActions(false),
    _environment(_vm),
    m_sound_stream_id(-1),
    _droptarget(),
    _lockroot(false)
{
    assert(_swf);

    if (!isAS3(getVM())) {
        set_prototype(getMovieClipAS2Interface());
        attachMovieClipAS2Properties(*this);
    }
    else {
        set_prototype(getMovieClipAS3Interface());
    }
            
    _environment.set_target(this);

}

MovieClip::~MovieClip()
{
    stopStreamSound();

    _vm.getRoot().remove_key_listener(this);
    _vm.getRoot().remove_mouse_listener(this);

    deleteAllChecked(_loadVariableRequests);
}

int
MovieClip::getSWFVersion() const
{
    return _swf->version();
}

// Execute the actions in the action list, in the given
// environment. The list of action will be consumed
// starting from the first element. When the function returns
// the list should be empty.
void
MovieClip::execute_actions(MovieClip::ActionList& action_list)
{
    // action_list may be changed due to actions (appended-to)
    // This loop is probably quicker than using an iterator
    // and a final call to .clear(), as repeated calls to
    // .size() or .end() are no quicker (and probably slower)
    // than pop_front(), which is constant time.
    while ( ! action_list.empty() )
    {
        const action_buffer* ab = action_list.front();
        action_list.pop_front(); 

        execute_action(*ab);
    }
}

DisplayObject*
MovieClip::getDisplayObjectAtDepth(int depth)
{
    return _displayList.getDisplayObjectAtDepth(depth);
}

// Set *val to the value of the named member and
// return true, if we have the named member.
// Otherwise leave *val alone and return false.
bool
MovieClip::get_member(string_table::key name_key, as_value* val,
    string_table::key nsname)
{
    // FIXME: use addProperty interface for these !!
    // TODO: or at least have a DisplayObject protected method take
    //       care of these ?
    //       Duplicates code in DisplayObject::getPathElementSeparator too.
    if (getSWFVersion() > 4 && name_key == NSV::PROP_uROOT)
    {
        // getAsRoot() will take care of _lockroot
        val->set_as_object(getAsRoot());
        return true;
    }

    // NOTE: availability of _global doesn't depend on VM version
    //             but on actual movie version. Example: if an SWF4 loads
    //             an SWF6 (to, say, _level2), _global will be unavailable
    //             to the SWF4 code but available to the SWF6 one.
    //
    if (getSWFVersion() > 5 && name_key == NSV::PROP_uGLOBAL) 
    {
        // The "_global" ref was added in SWF6
        val->set_as_object( _vm.getGlobal() );
        return true;
    }

    const std::string& name = _vm.getStringTable().value(name_key);

    movie_root& mr = _vm.getRoot();
    unsigned int levelno;
    if ( mr.isLevelTarget(name, levelno) )
    {
        Movie* mo = _vm.getRoot().getLevel(levelno).get();
        if ( mo )
        {
            val->set_as_object(mo);
            return true;
        }
        else
        {
            return false;
        }
    }

    // Own members take precendence over display list items 
    // (see testcase VarAndCharClash.swf in testsuite/misc-ming.all)
    as_object* owner = NULL;
    Property* prop = findProperty(name_key, nsname, &owner);
    if ( prop && owner == this ) 
    {
        try { *val = prop->getValue(*this); }
        catch (ActionLimitException&) { throw; }
        catch (ActionTypeError& ex) {
            log_error(_("Caught exception: %s"), ex.what());
            return false;
        }
        return true;
    }

    // Try items on our display list.
    DisplayObject* ch;
    if ( _vm.getSWFVersion() >= 7 ) {
        ch = _displayList.getDisplayObjectByName(name);
    }
    else ch = _displayList.getDisplayObjectByName_i(name);
    if (ch) {
            // Found object.

            // If the object is an ActionScript referenciable one we
            // return it, otherwise we return ourselves
            if ( ch->isActionScriptReferenceable() )
            {
                val->set_as_object(ch);
            }
            else
            {
                val->set_as_object(this);
            }

            return true;
    }

    // Try textfield variables
    TextFields* etc = get_textfield_variable(name);
    if ( etc )
    {
        for (TextFields::const_iterator i=etc->begin(), e=etc->end();
                i!=e; ++i)
        {
            boost::intrusive_ptr<TextField> tf = i->get();
            if ( tf->getTextDefined() )
            {
                val->set_string(tf->get_text_value());
                return true;
            }
        }
    }

    // Inherited members come last 
    // (see testcase VarAndCharClash.swf in testsuite/misc-ming.all)
    if ( prop )
    {
        assert(owner != this);
        try { *val = prop->getValue(*this); }
        catch (ActionLimitException&) { throw; }
        catch (ActionTypeError& ex)
        {
                log_error(_("Caught exception: %s"), ex.what());
                return false;
        }
        return true;
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

    double num = str.to_number();

    //log_debug("get_frame_number(%s), num: %g", frame_spec, num);

    if (!isFinite(num) || int(num) != num || num == 0)
    {
        bool ret = _def->get_labeled_frame(fspecStr, frameno);
        //log_debug("get_labeled_frame(%s) returned %d, frameno is %d", fspecStr, ret, frameno);
        return ret;
    }

    if ( num < 0 ) return false;

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

    size_t frame_number;
    if ( ! get_frame_number(frame_spec, frame_number) )
    {
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
    _callingFrameActions=true;
    const PlayList* playlist = _def->getPlaylist(frame_number);
    if ( playlist )
    {
    PlayList::const_iterator it = playlist->begin();
        const PlayList::const_iterator e = playlist->end();
    for(; it != e; it++)
    {
        (*it)->execute_action(this, _displayList);
    }
    }
    _callingFrameActions=false;

}

DisplayObject*
MovieClip::add_empty_movieclip(const std::string& name, int depth)
{
    MovieClip* movieclip = new MovieClip(0, _swf, this, 0);
    movieclip->set_name(name);
    movieclip->setDynamic();

    // TODO: only call set_invalidated if this DisplayObject actually overrides
    //             an existing one !
    set_invalidated(); 

    _displayList.placeDisplayObject(movieclip, depth);     

    return movieclip;
}

boost::intrusive_ptr<DisplayObject>
MovieClip::add_textfield(const std::string& name, int depth, int x, int y,
        float width, float height)
{
    
    // Set textfield bounds
    rect bounds(0, 0, pixelsToTwips(width), pixelsToTwips(height));

    // Create an instance
    boost::intrusive_ptr<DisplayObject> txt_char = new TextField(this, bounds);

    // Give name and mark as dynamic
    txt_char->set_name(name);
    txt_char->setDynamic();

    // Set _x and _y
    SWFMatrix txt_matrix;
    txt_matrix.set_translation(pixelsToTwips(x), pixelsToTwips(y));
    // update caches (although shouldn't be needed as we only set translation)
    txt_char->setMatrix(txt_matrix, true); 

    // Here we add the DisplayObject to the displayList.    
    _displayList.placeDisplayObject(txt_char.get(), depth); 

    return txt_char;
}

boost::intrusive_ptr<MovieClip> 
MovieClip::duplicateMovieClip(const std::string& newname, int depth,
        as_object* initObject)
{
    DisplayObject* parent_ch = get_parent();
    if ( ! parent_ch )
    {
        log_error(_("Can't clone root of the movie"));
        return NULL;
    }
    MovieClip* parent = parent_ch->to_movie();
    if ( ! parent )
    {
        log_error(_("%s parent is not a movieclip, can't clone"), getTarget());
        return NULL;
    }

    boost::intrusive_ptr<MovieClip> newmovieclip = new MovieClip(_def.get(),
            _swf, parent, get_id());
    newmovieclip->set_name(newname);

    newmovieclip->setDynamic();

    // Copy event handlers from movieclip
    // We should not copy 'm_action_buffer' since the 
    // 'm_method' already contains it
    newmovieclip->set_event_handlers(get_event_handlers());

    // Copy drawable
    newmovieclip->_drawable = _drawable;
    
    newmovieclip->set_cxform(get_cxform());    
    newmovieclip->copyMatrix(*this); // copy SWFMatrix and caches
    newmovieclip->set_ratio(get_ratio());    
    newmovieclip->set_clip_depth(get_clip_depth());    
    
    parent->_displayList.placeDisplayObject(newmovieclip.get(), depth, 
            initObject);
    
    return newmovieclip; 
}

void
MovieClip::queueAction(const action_buffer& action)
{
    movie_root& root = _vm.getRoot();
    root.pushAction(action, boost::intrusive_ptr<MovieClip>(this));
}

void
MovieClip::queueActions(ActionList& actions)
{
    for(ActionList::const_iterator it=actions.begin(), itEnd=actions.end();
                     it != itEnd; ++it)
    {
        const action_buffer* buf = *it;
        queueAction(*buf);
    }
}

bool
MovieClip::on_event(const event_id& id)
{
    testInvariant();

#ifdef GNASH_DEBUG
    log_debug(_("Event %s invoked for movieclip %s"), id, getTarget());
#endif

    // We do not execute ENTER_FRAME if unloaded
    if ( id.id() == event_id::ENTER_FRAME && unloaded() )
    {
#ifdef GNASH_DEBUG
        log_debug(_("Sprite %s ignored ENTER_FRAME event (is unloaded)"), getTarget());
#endif
        return false;
    }

    if ( id.is_button_event() && ! isEnabled() )
    {
#ifdef GNASH_DEBUG
        log_debug(_("Sprite %s ignored button-like event %s as not 'enabled'"),
            getTarget(), id);
#endif
        return false;
    }

    bool called = false;
            
    std::auto_ptr<ExecutableCode> code ( get_event_handler(id) );
    if ( code.get() )
    {
        // Dispatch.
        code->execute();

        called = true;
    }

    // user-defined onInitialize is never called
    if ( id.id() == event_id::INITIALIZE ) {
        testInvariant();
        return called;
    }

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
    if ( id.id() == event_id::LOAD )
    {
        // TODO: we're likely making too much noise for nothing here,
        // there must be some action-execution-order related problem instead....
        // See testsuite/misc-ming.all/registerClassTest2.swf for an onLoad 
        // execution order related problem ...
        do
        {
            // we don't skip calling user-defined onLoad for top-level movies 
            if ( ! get_parent() ) break;
            // nor if there are clip-defined handler
            if ( ! get_event_handlers().empty() ) break; 
            // nor if it's dynamic  
            if ( isDynamic() ) break;

            const sprite_definition* def =
                dynamic_cast<const sprite_definition*>(_def.get());

            // must be a loaded movie (loadMovie doesn't mark it as 
            // "dynamic" - should it? no, or getBytesLoaded will always
            // return 0)  
            if ( ! def ) break;

            // if it has a registered class it can have an onLoad 
            // in prototype...
            if ( def->getRegisteredClass() ) break;

#ifdef GNASH_DEBUG
            log_debug(_("Sprite %s (depth %d) won't check for user-defined "
                        "LOAD event (is not dynamic, has a parent, "
                        "no registered class and no clip events defined)"),
                        getTarget(), get_depth());
            testInvariant();
#endif
            return called;
        } while (0);
            
    }

    // Check for member function.
    if (! id.is_key_event ())
    {
        boost::intrusive_ptr<as_function> method = 
            getUserDefinedEventHandler(id.functionKey());

        if ( method )
        {
            call_method0(as_value(method.get()), _environment, this);
            called = true;
        }
    }

    // TODO: if this was UNLOAD release as much memory as possible ?
    //             Verify if this is possible, in particular check order in
    //             which unload handlers of parent and childs is performed
    //             and wheter unload of child can access members of parent.

    testInvariant();

    return called;
}

as_object*
MovieClip::get_path_element(string_table::key key)
{
    as_object* obj = getPathElementSeparator(key);
    if (obj) return obj;

    std::string name = _vm.getStringTable().value(key);

    // See if we have a match on the display list.
    DisplayObject* ch;
    if ( _vm.getSWFVersion() >= 7 ) ch = 
        _displayList.getDisplayObjectByName(name);

    else ch = _displayList.getDisplayObjectByName_i(name);

            // TODO: should we check for isActionScriptReferenceable here ?
    if ( ch )
    {
        // If the object is an ActionScript referenciable one we
        // return it, otherwise we return ourselves
        if ( ch->isActionScriptReferenceable() ) return ch;
        else return this;
    }

    // See if it's a member

    // NOTE: direct use of the base class's get_member avoids
    //             triggering a call to MovieClip::get_member
    //             which would scan the child DisplayObjects again
    //             w/out a need for it

    as_value tmp;
    if ( !as_object::get_member(key, &tmp, 0) )
    {
        return NULL;
    }
    if ( ! tmp.is_object() )
    {
        return NULL;
    }
    if ( tmp.is_sprite() )
    {
        return tmp.to_sprite(true);
    }

    return tmp.to_object().get();
}

bool
MovieClip::set_member(string_table::key name,
        const as_value& val, string_table::key nsname, bool ifFound)
{

    bool found = false;

    // Try textfield variables
    //
    // FIXME: Turn textfield variables into Getter/Setters (Properties)
    //                so that as_object::set_member will do this automatically.
    //                The problem is that setting a TextVariable named after
    //                a builtin property will prevent *any* setting for the
    //                property (ie: have a textfield use _x as variable name and
    //                be scared)
    //
    TextFields* etc = get_textfield_variable(_vm.getStringTable().value(name));
    if ( etc )
    {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("it's a Text Variable, associated with %d TextFields"), etc->size());
#endif
        for (TextFields::iterator i=etc->begin(), e=etc->end(); i!=e; ++i)
        {
            (*i)->updateText(val.to_string());
        }
        found = true;
    }
#ifdef DEBUG_DYNTEXT_VARIABLES
    else
    {
        log_debug(_("it's NOT a Text Variable"));
    }
#endif

    // If that didn't work call the default set_member
    if (as_object::set_member(name, val, nsname, ifFound)) found=true;

    return found;
}

/// Remove the 'contents' of the MovieClip, but leave properties and
/// event handlers intact.
void
MovieClip::unloadMovie()
{
    LOG_ONCE(log_unimpl("MovieClip.unloadMovie()"));
}

// child movieclip advance
void
MovieClip::advance()
{

#ifdef GNASH_DEBUG
    log_debug(_("Advance movieclip '%s' at frame %u/%u"),
        getTargetPath(), _currentFrame,
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

    log_debug(_("Advance_movieclip for movieclip '%s' - frame %u/%u "),
        getTarget(), _currentFrame,
        frame_count);
#endif

    // I'm not sure ENTERFRAME goes in a different queue then DOACTION...
    queueEvent(event_id::ENTER_FRAME, movie_root::apDOACTION);

    // Update current and next frames.
    if (_playState == PLAYSTATE_PLAY)
    {
#ifdef GNASH_DEBUG
        log_debug(_("MovieClip::advance_movieclip we're in PLAYSTATE_PLAY mode"));
#endif

        int prev_frame = _currentFrame;

#ifdef GNASH_DEBUG
        log_debug(_("on_event_load called, incrementing"));
#endif
        increment_frame_and_check_for_loop();
#ifdef GNASH_DEBUG
        log_debug(_("after increment we are at frame %u/%u"), _currentFrame, frame_count);
#endif

        // Execute the current frame's tags.
        // First time executeFrameTags(0) executed in dlist.cpp(child) or
        // SWFMovieDefinition(root)
        if (_currentFrame != (size_t)prev_frame)
        {
            if ( _currentFrame == 0 && has_looped() )
            {
#ifdef GNASH_DEBUG
                log_debug(_("Jumping back to frame 0 of movieclip %s"),
                        getTarget());
#endif
                restoreDisplayList(0); // seems OK to me.
            }
            else
            {
#ifdef GNASH_DEBUG
                log_debug(_("Executing frame%d (0-based) tags of movieclip "
                            "%s"), _currentFrame, getTarget());
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
    else
    {
        log_debug(_("MovieClip::advance_movieclip we're in STOP mode"));
    }
#endif

}

void
MovieClip::execute_init_action_buffer(const action_buffer& a, int cid)
{
    if ( _swf->setCharacterInitialized(cid) )
    {
#ifdef GNASH_DEBUG
        log_debug(_("Queuing init actions in frame %d of movieclip %s"),
                _currentFrame, getTarget());
#endif
        std::auto_ptr<ExecutableCode> code ( 
                new GlobalCode(a, boost::intrusive_ptr<MovieClip>(this)) );

        movie_root& root = _vm.getRoot();
        root.pushAction(code, movie_root::apINIT);
    }
    else
    {
#ifdef GNASH_DEBUG
        log_debug(_("Init actions for DisplayObject %d already executed"), cid);
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

    // Just invalidate this DisplayObject before jumping back.
    // Should be optimized, but the invalidating model is not clear enough,
    // and there are some old questions spreading the source files.
    set_invalidated();

    DisplayList tmplist;
    for (size_t f = 0; f<tgtFrame; ++f)
    {
        _currentFrame = f;
        executeFrameTags(f, tmplist, SWF::ControlTag::TAG_DLIST);
    }

    // Execute both action tags and DLIST tags of the target frame
    _currentFrame = tgtFrame;
    executeFrameTags(tgtFrame, tmplist, SWF::ControlTag::TAG_DLIST |
                                        SWF::ControlTag::TAG_ACTION);

    _displayList.mergeDisplayList(tmplist);
}

// 0-based frame number !
void
MovieClip::executeFrameTags(size_t frame, DisplayList& dlist, int typeflags)
{
    // If there is no definition, this is a dynamically-created MovieClip
    // and has no frames.
    if (!_def) return;

    testInvariant();
    assert(typeflags);

    const PlayList* playlist = _def->getPlaylist(frame);
    if ( playlist )
    {
        PlayList::const_iterator it = playlist->begin();
        PlayList::const_iterator e = playlist->end();
    
        IF_VERBOSE_ACTION(
            // Use 1-based frame numbers
            log_action(_("Executing %d tags in frame %d/%d of movieclip %s"),
                playlist->size(), frame + 1, get_frame_count(),
                getTargetPath());
        );

        if ((typeflags & SWF::ControlTag::TAG_DLIST) && 
                (typeflags & SWF::ControlTag::TAG_ACTION) )
        {
            for( ; it != e; it++)
            {
                (*it)->execute(this, dlist);
            }
        }
        else if ( typeflags & SWF::ControlTag::TAG_DLIST )
        {
            for( ; it != e; it++)
            {
                (*it)->execute_state(this, dlist);
            }
        }
        else
        {
            assert(typeflags & SWF::ControlTag::TAG_ACTION);
            for( ; it != e; it++)
            {
                (*it)->execute_action(this, dlist);
            }
        }
    }

    testInvariant();
}

void
MovieClip::goto_frame(size_t target_frame_number)
{
#if defined(DEBUG_GOTOFRAME) || defined(GNASH_DEBUG_TIMELINE)
    log_debug(_("movieclip %s ::goto_frame(%d) - current frame is %d"),
        getTargetPath(), target_frame_number, _currentFrame);
#endif

    // goto_frame stops by default.
    // ActionGotoFrame tells the movieClip to go to the target frame 
    // and stop at that frame. 
    setPlayState(PLAYSTATE_STOP);

    if ( target_frame_number > _def->get_frame_count() - 1)
    {
        target_frame_number = _def->get_frame_count() - 1;

        if ( ! _def->ensure_frame_loaded(target_frame_number+1) )
        {
            log_error(_("Target frame of a gotoFrame(%d) was never loaded,"
                        "although frame count in header (%d) said we "
                        "should have found it"),
                        target_frame_number+1, _def->get_frame_count());
            return; // ... I guess, or not ?
        }

        // Just set _currentframe and return.
        _currentFrame = target_frame_number;

        // don't push actions, already tested.
        return;
    }

    if (target_frame_number == _currentFrame)
    {
        // don't push actions
        return;
    }

    // Unless the target frame is the next one, stop playback of soundstream
    if (target_frame_number != _currentFrame+1 )
    {
        stopStreamSound();
    }

    size_t loaded_frames = get_loaded_frames();
    // target_frame_number is 0-based, get_loaded_frames() is 1-based
    // so in order to goto_frame(3) loaded_frames must be at least 4
    // if goto_frame(4) is called, and loaded_frames is 4 we're jumping
    // forward
    if ( target_frame_number >= loaded_frames )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("GotoFrame(%d) targets a yet "
            "to be loaded frame (%d) loaded). "
            "We'll wait for it but a more correct form "
            "is explicitly using WaitForFrame instead"),
            target_frame_number+1,
            loaded_frames);

        );
        if ( ! _def->ensure_frame_loaded(target_frame_number+1) )
        {
            log_error(_("Target frame of a gotoFrame(%d) was never loaded, "
                        "although frame count in header (%d) said we should"
                        " have found it"),
                        target_frame_number+1, _def->get_frame_count());
            return; // ... I guess, or not ?
        }
    }


    //
    // Construct the DisplayList of the target frame
    //

    if (target_frame_number < _currentFrame)
    {
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
    else
    // Go forward to a later frame
    {
        // We'd immediately return if target_frame_number == _currentFrame
        assert(target_frame_number > _currentFrame);
        while (++_currentFrame < target_frame_number)
        {
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
    if (_def->get_labeled_frame(label, target_frame))
    {
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
MovieClip::display(Renderer& renderer)
{

    // Note: 
    // DisplayList::Display() will take care of the visibility checking.
    //
    // Whether a DisplayObject should be rendered or not is dependent
    // on its parent: i.e. if its parent is a mask, this DisplayObject
    // should be rendered to the mask buffer even it is invisible.
    
    // render drawable (ActionScript generated graphics)
    _drawable.finalize();
    _drawable.display(renderer, *this);
    
    
    // descend the display list
    _displayList.display(renderer);
     
    clear_invalidated();
}

void MovieClip::omit_display()
{
    if (m_child_invalidated) _displayList.omit_display();
        
    clear_invalidated();
}

bool
MovieClip::attachCharacter(DisplayObject& newch, int depth,
        as_object* initObject)
{ 
    _displayList.placeDisplayObject(&newch, depth, initObject);    

    // FIXME: check return from placeDisplayObject above ?
    return true; 
}

std::auto_ptr<GnashImage>
MovieClip::drawToBitmap(const SWFMatrix& /* mat */, const cxform& /* cx */,
            DisplayObject::BlendMode /* bm */, const rect& /* clipRect */,
            bool /* smooth */)
{
    return std::auto_ptr<GnashImage>();
}

void
MovieClip::attachBitmap(boost::intrusive_ptr<BitmapData_as> bd, int depth)
{
    DisplayObject* ch = new Bitmap(bd, this, 0);
    attachCharacter(*ch, depth, 0);
}

DisplayObject*
MovieClip::add_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{

    // If this MovieClip has no definition, it should also have no ControlTags,
    // and this shouldn't be called.
    assert(_def);
    assert(tag);

    SWF::DefinitionTag* cdef = _def->getDefinitionTag(tag->getID());
    if (!cdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("MovieClip::add_display_object(): "
                    "unknown cid = %d"), tag->getID());
        );
        return NULL;
    }
    
    DisplayObject* existing_char = dlist.getDisplayObjectAtDepth(tag->getDepth());
    
    if (existing_char) return NULL;

    boost::intrusive_ptr<DisplayObject> ch =
        cdef->createDisplayObject(this, tag->getID());

    if (tag->hasName()) ch->set_name(tag->getName());
    else if (ch->wantsInstanceName())
    {
        std::string instance_name = getNextUnnamedInstanceName();
        ch->set_name(instance_name);
    }

    if (tag->hasBlendMode()) {
        boost::uint8_t bm = tag->getBlendMode();
        ch->setBlendMode(static_cast<DisplayObject::BlendMode>(bm));
    }

    // Attach event handlers (if any).
    const std::vector<swf_event*>& event_handlers = tag->getEventHandlers();
    for (size_t i = 0, n = event_handlers.size(); i < n; i++)
    {
        swf_event* ev = event_handlers[i];
        ch->add_event_handler(ev->event(), ev->action());
    }

    // TODO: check if we should check those has_xxx flags first.
    ch->set_cxform(tag->getCxform());
    ch->setMatrix(tag->getMatrix(), true); // update caches
    ch->set_ratio(tag->getRatio());
    ch->set_clip_depth(tag->getClipDepth());
    
    dlist.placeDisplayObject(ch.get(), tag->getDepth());
    return ch.get();
}

void 
MovieClip::move_display_object(const SWF::PlaceObject2Tag* tag, DisplayList& dlist)
{    
    int ratio = tag->getRatio();
    // clip_depth is not used in MOVE tag(at least no related tests). 
    dlist.moveDisplayObject(
        tag->getDepth(), 
        tag->hasCxform() ? &tag->getCxform() : NULL,
        tag->hasMatrix() ? &tag->getMatrix() : NULL,
        tag->hasRatio()    ? &ratio    : NULL,
        NULL);
}

void
MovieClip::replace_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{
    // A MovieClip without a definition cannot have any ControlTags, so this
    // should not be called.
    assert(_def);
    assert(tag != NULL);

    SWF::DefinitionTag* cdef = _def->getDefinitionTag(tag->getID());
    if (cdef == NULL)
    {
        log_error(_("movieclip::replace_display_object(): "
            "unknown cid = %d"), tag->getID());
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
    if (existing_char->isActionScriptReferenceable()) {
        move_display_object(tag, dlist);
        return;
    }

    boost::intrusive_ptr<DisplayObject> ch = 
        cdef->createDisplayObject(this, tag->getID());

    // TODO: check if we can drop this for REPLACE!
    // should we rename the DisplayObject when it's REPLACE tag?
    if(tag->hasName()) {
        ch->set_name(tag->getName());
    }
    else if (ch->wantsInstanceName()) {
        std::string instance_name = getNextUnnamedInstanceName();
        ch->set_name(instance_name);
    }
    if (tag->hasRatio()) {
        ch->set_ratio(tag->getRatio());
    }
    if (tag->hasCxform()) {
        ch->set_cxform(tag->getCxform());
    }
    if (tag->hasMatrix()) {
        ch->setMatrix(tag->getMatrix(), true); 
    }

    // use SWFMatrix from the old DisplayObject if tag doesn't provide one.
    dlist.replaceDisplayObject(ch.get(), tag->getDepth(), 
        !tag->hasCxform(), !tag->hasMatrix());
}

void
MovieClip::remove_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{
    set_invalidated();
    dlist.removeDisplayObject(tag->getDepth());
}

void
MovieClip::replace_display_object(DisplayObject* ch, int depth, 
        bool use_old_cxform, bool use_old_matrix)
{
    assert(ch);
    _displayList.replaceDisplayObject(ch, depth,
            use_old_cxform, use_old_matrix);
}

int
MovieClip::get_id_at_depth(int depth)
{
    DisplayObject* ch = _displayList.getDisplayObjectAtDepth(depth);
    if ( ! ch ) return -1;
    return ch->get_id();
}

void
MovieClip::increment_frame_and_check_for_loop()
{
    size_t frame_count = get_loaded_frames(); 
    if ( ++_currentFrame >= frame_count )
    {
        // Loop.
        _currentFrame = 0;
        _hasLooped = true;
        if (frame_count > 1)
        {
            //_displayList.reset();
        }
    }

}

bool
MovieClip::handleFocus()
{

    // For SWF6 and above: the MovieClip can always receive focus if
    // focusEnabled evaluates to true.
    if (_vm.getSWFVersion() > 5) {
        as_value focusEnabled;
        if (get_member(NSV::PROP_FOCUS_ENABLED, &focusEnabled)) {
            if (focusEnabled.to_bool() == true) return true; 
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
    const_cast<DisplayList&>(_displayList).visitBackward(finder);
    if ( finder.hitFound() ) return true;
    return hitTestDrawable(x, y);
}

bool
MovieClip::pointInVisibleShape(boost::int32_t x, boost::int32_t y) const
{
    if ( ! visible() ) return false;
    if ( isDynamicMask() && ! mouseEnabled() )
    {
        // see testsuite/misc-ming.all/masks_test.swf
#ifdef GNASH_DEBUG_HITTEST
        log_debug(_("%s is a dynamic mask and can't handle mouse "
                    "events, no point will hit it"), getTarget());
#endif
        return false;
    }
    const DisplayObject* mask = getMask(); // dynamic one
    if ( mask && mask->visible() && ! mask->pointInShape(x, y) )
    {
#ifdef GNASH_DEBUG_HITTEST
        log_debug(_("%s is dynamically masked by %s, which "
                "doesn't hit point %g,%g"), getTarget(),
                mask->getTarget(), x, y);
#endif
        return false;
    }
    VisibleShapeContainerFinder finder(x, y);
    const_cast<DisplayList&>(_displayList).visitBackward(finder);
    if (finder.hitFound()) return true;
    return hitTestDrawable(x, y);
}

inline bool
MovieClip::hitTestDrawable(boost::int32_t x, boost::int32_t y) const
{
    SWFMatrix wm = getWorldMatrix();
    wm.invert();
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
    //GNASH_REPORT_FUNCTION;

    if (!visible()) return 0;

    // point is in parent's space, we need to convert it in world space
    point wp(x, y);
    DisplayObject* parent = get_parent();
    if ( parent ) 
    {
        // WARNING: if we have NO parent, our parent is the Stage (movie_root)
        //          so, in case we'll add a "stage" matrix, we'll need to take
        //          it into account here.
        // TODO: actually, why are we insisting in using parent's
        //          coordinates for this method at all ?
        parent->getWorldMatrix().transform(wp);
    }

    if (mouseEnabled())
    {
        if (pointInVisibleShape(wp.x, wp.y)) return this;
        else return NULL;
    }

    SWFMatrix    m = getMatrix();
    point    pp(x, y);
    m.invert().transform(pp);

    MouseEntityFinder finder(wp, pp);
    _displayList.visitAll(finder);
    InteractiveObject* ch = finder.getEntity();

    // It doesn't make any sense to query _drawable, as it's
    // not an InteractiveObject.
    return ch; 
}

/// Find the first visible DisplayObject whose shape contain the point
/// and is not the DisplayObject being dragged or any of its childs
//
/// Point coordinates in world TWIPS
///
class DropTargetFinder {

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

    void operator() (const DisplayObject* ch)
    {
        assert(!_checked);
        if ( ch->get_depth() <= _highestHiddenDepth )
        {
            if ( ch->isMaskLayer() )
            {
                log_debug(_("CHECKME: nested mask in DropTargetFinder. "
                        "This mask is %s at depth %d outer mask masked "
                        "up to depth %d."),
                        ch->getTarget(), ch->get_depth(), _highestHiddenDepth);
                // Hiding mask still in effect...
            }
            return;
        }

        if ( ch->isMaskLayer() )
        {
            if ( ! ch->visible() )
            {
                log_debug(_("FIXME: invisible mask in MouseEntityFinder."));
            }
            if ( ! ch->pointInShape(_x, _y) )
            {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug(_("Character %s at depth %d is a mask not hitting "
                        "the query point %g,%g and masking up to depth %d"),
                    ch->getTarget(), ch->get_depth(), _x, _y,
                    ch->get_clip_depth());
#endif 
                _highestHiddenDepth = ch->get_clip_depth();
            }
            else
            {
#ifdef DEBUG_MOUSE_ENTITY_FINDING
                log_debug(_("Character %s at depth %d is a mask "
                            "hitting the query point %g,%g"),
                            ch->getTarget(), ch->get_depth(), _x, _y);
#endif
            }

            return;
        }

        _candidates.push_back(ch);

    }

    void checkCandidates() const
    {
        if ( _checked ) return;
        for (Candidates::const_reverse_iterator i=_candidates.rbegin(),
                        e=_candidates.rend(); i!=e; ++i)
        {
            const DisplayObject* ch = *i;
            const DisplayObject* dropChar = ch->findDropTarget(_x, _y, _dragging);
            if ( dropChar )
            {
                _dropch = dropChar;
                break;
            }
        }
        _checked = true;
    }

    const DisplayObject* getDropChar() const
    {
        checkCandidates();
        return _dropch;
    }
};

const DisplayObject*
MovieClip::findDropTarget(boost::int32_t x, boost::int32_t y,
        DisplayObject* dragging) const
{
    if ( this == dragging ) return 0; // not here...

    if ( ! visible() ) return 0; // isn't me !

    DropTargetFinder finder(x, y, dragging);
    _displayList.visitAll(finder);

    // does it hit any child ?
    const DisplayObject* ch = finder.getDropChar();
    if ( ch )
    {
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
    as_value track;
    string_table& st = _vm.getStringTable();
    return (get_member(st.find("trackAsMenu"), &track) && track.to_bool());
}

bool
MovieClip::mouseEnabled() const
{
    if ( ! isEnabled() ) return false;

    // Event handlers that qualify as mouse event handlers.
    static const event_id EH[] =
    {
        event_id(event_id::PRESS),
        event_id(event_id::RELEASE),
        event_id(event_id::RELEASE_OUTSIDE),
        event_id(event_id::ROLL_OVER),
        event_id(event_id::ROLL_OUT),
        event_id(event_id::DRAG_OVER),
        event_id(event_id::DRAG_OUT),
    };

    static const size_t size = sizeof(EH) / sizeof(EH[0]);

    for (size_t i = 0; i < size; ++i)
    {
        const event_id &event = EH[i];

        // Check event handlers
        if ( get_event_handler(event.id()).get() )
        {
            return true;
        }

        // Check user-defined event handlers
        if ( getUserDefinedEventHandler(event.functionKey()) )
        {
            return true;
        }
    }

    return false;
}

void
MovieClip::stop_drag()
{
    _vm.getRoot().stop_drag();
}

void
MovieClip::set_background_color(const rgba& color)
{
    _vm.getRoot().set_background_color(color);
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
MovieClip::set_textfield_variable(const std::string& name, TextField* ch)
{
    assert(ch);

    // lazy allocation
    if ( ! _text_variables.get() )
    {
        _text_variables.reset(new TextFieldIndex);
    }
    
    (*_text_variables)[name].push_back(ch);
}

MovieClip::TextFields*
MovieClip::get_textfield_variable(const std::string& name)
{
    // nothing allocated yet...
    if ( ! _text_variables.get() ) return NULL;

    // TODO: should variable name be considered case-insensitive ?
    TextFieldIndex::iterator it = _text_variables->find(name);
    if (it == _text_variables->end()) return 0;
    else return &(it->second);
} 


void 
MovieClip::add_invalidated_bounds(InvalidatedRanges& ranges, 
    bool force)
{

    // nothing to do if this movieclip is not visible
    if (!visible() || get_cxform().is_invisible() )
    {
        ranges.add(m_old_invalidated_ranges); // (in case we just hided)
        return;
    }

    if ( ! m_invalidated && ! m_child_invalidated && ! force )
    {
        return;
    }
    
 
    // m_child_invalidated does not require our own bounds
    if ( m_invalidated || force )            
    {
        // Add old invalidated bounds
        ranges.add(m_old_invalidated_ranges); 
    }
    
    
    _displayList.add_invalidated_bounds(ranges, force||m_invalidated);

    /// Add drawable.
    rect bounds;
    bounds.expand_to_transformed_rect(getWorldMatrix(), _drawable.getBounds());
    ranges.add(bounds.getRange());

}


/// register DisplayObjects as key listeners if they have clip key events defined.
/// Don't call twice for the same chracter.
void
MovieClip::registerAsListener()
{
    _vm.getRoot().add_key_listener(this);
    _vm.getRoot().add_mouse_listener(this);
}


// WARNING: THIS SNIPPET NEEDS THE CHARACTER TO BE "INSTANTIATED", that is,
//          its target path needs to exist, or any as_value for it will be
//          a dangling reference to an unexistent movieclip !
//          NOTE: this is just due to the wrong steps, see comment in header
void
MovieClip::stagePlacementCallback(as_object* initObj)
{
    assert(!unloaded());

    saveOriginalTarget();

#ifdef GNASH_DEBUG
    log_debug(_("Sprite '%s' placed on stage"), getTarget());
#endif

    // Register this movieclip as a live one
    _vm.getRoot().addLiveChar(this);
  

    // Register this movieclip as a core broadcasters listener
    registerAsListener();

    // It seems it's legal to place 0-framed movieclips on stage.
    // See testsuite/misc-swfmill.all/zeroframe_definemovieclip.swf

    // Now execute frame tags and take care of queuing the LOAD event.
    //
    // DLIST tags are executed immediately while ACTION tags are queued.
    //
    // For _root movie, LOAD event is invoked *after* actions in first frame
    // See misc-ming.all/action_execution_order_test4.{c,swf}
    //
    assert(!_callingFrameActions); // or will not be queuing actions
    if (!get_parent())
    {
#ifdef GNASH_DEBUG
        log_debug(_("Executing tags of frame0 in movieclip %s"), getTarget());
#endif
        executeFrameTags(0, _displayList, SWF::ControlTag::TAG_DLIST |
                SWF::ControlTag::TAG_ACTION);

        if (_vm.getSWFVersion() > 5)
        {
#ifdef GNASH_DEBUG
            log_debug(_("Queuing ONLOAD event for movieclip %s"), getTarget());
#endif
            queueEvent(event_id::LOAD, movie_root::apDOACTION);
        }

    }
    else
    {
#ifdef GNASH_DEBUG
        log_debug(_("Queuing ONLOAD event for movieclip %s"), getTarget());
#endif
        queueEvent(event_id::LOAD, movie_root::apDOACTION);

#ifdef GNASH_DEBUG
        log_debug(_("Executing tags of frame0 in movieclip %s"), getTarget());
#endif
        executeFrameTags(0, _displayList, SWF::ControlTag::TAG_DLIST |
                SWF::ControlTag::TAG_ACTION);
    }

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
    if (!isDynamic())
    {
        assert(!initObj);
#ifdef GNASH_DEBUG
        log_debug(_("Queuing INITIALIZE and CONSTRUCT events for movieclip %s"),
                getTarget());
#endif
        queueEvent(event_id::INITIALIZE, movie_root::apINIT);

        std::auto_ptr<ExecutableCode> code ( new ConstructEvent(this) );
        _vm.getRoot().pushAction(code, movie_root::apCONSTRUCT);

    }
    else {

        // Properties from an initObj must be copied before construction, but
        // after the display list has been populated, so that _height and
        // _width (which depend on bounds) are correct.
        if (initObj) {
            copyProperties(*initObj);
        }

        constructAsScriptObject(); 

        // Tested in testsuite/swfdec/duplicateMovieclip-events.c and
        // testsuite/swfdec/clone-sprite-events.c not to call on_event
        // immediately.
        queueEvent(event_id::INITIALIZE, movie_root::apINIT);
    }


}


void
MovieClip::constructAsScriptObject()
{
#ifdef GNASH_DEBUG
    log_debug(_("constructAsScriptObject called for movieclip %s"), 
            getTarget());
#endif

    bool eventHandlersInvoked = false;

    do {

        if ( _name.empty() )
        {
            // instance name will be needed for properly setting up
            // a reference to 'this' object for ActionScript actions.
            // If the instance doesn't have a name, it will NOT be
            // an ActionScript referenciable object so we don't have
            // anything more to do.
            break;
        }

        const sprite_definition* def = 
            dynamic_cast<const sprite_definition*>(_def.get());

        // We won't "construct" top-level movies
        if (!def) break;

        as_function* ctor = def->getRegisteredClass();
#ifdef GNASH_DEBUG
        log_debug(_("Attached movieclips %s registered class is %p"),
                getTarget(), (void*)ctor); 
#endif

        // TODO: builtin constructors are different from user-defined ones
        // we should likely change that. See also vm/ASHandlers.cpp 
        // (construct_object)
        if ( ctor && ! ctor->isBuiltin() )
        {
            // Set the new prototype *after* the constructor was called
            boost::intrusive_ptr<as_object> proto = ctor->getPrototype();
            set_prototype(proto);

            // Call event handlers *after* setting up the __proto__
            // but *before* calling the registered class constructor
            on_event(event_id::CONSTRUCT);
            eventHandlersInvoked = true;

            int swfversion = _vm.getSWFVersion();

            // Set the '__constructor__' and 'constructor' members, as well
            // as calling the actual constructor.
            //
            // TODO: this would be best done by an
            // as_function::constructInstance() method. We have one but it
            // returns a new object rather then initializing a given object.
            // We just need to add another one...
            if ( swfversion > 5 )
            {

                const int flags = as_prop_flags::dontEnum;

                set_member(NSV::PROP_uuCONSTRUCTORuu, ctor);
                set_member_flags(NSV::PROP_uuCONSTRUCTORuu, flags);
                if ( swfversion == 6 )
                {
                    set_member(NSV::PROP_CONSTRUCTOR, ctor);
                    set_member_flags(NSV::PROP_CONSTRUCTOR, flags);
                }

                // Provide a 'super' reference..
                // Super is computed from the object we're constructing,
                // It will work as long as we did set its __proto__ 
                // and __constructor__ properties already.
                as_object* super = get_super();

                as_environment& env = get_environment();
                fn_call call(this, env);
                call.super = super;

                    // we don't use the constructor return (should we?)
                (*ctor)(call);
            }
        }

    } while (0);

    /// Invoke event handlers if not done yet
    if ( ! eventHandlersInvoked )
    {
        on_event(event_id::CONSTRUCT);
    }
}

bool
MovieClip::unload()
{
#ifdef GNASH_DEBUG
    log_debug(_("Unloading movieclip '%s'"), getTargetPath());
#endif

    // stop any pending streaming sounds
    stopStreamSound();

    bool childHaveUnloadHandler = _displayList.unload();

    // We won't be displayed again, so worth releasing
    // some memory. The drawable might take a lot of memory
    // on itself.
    _drawable.clear();

    bool selfHaveUnloadHandler = DisplayObject::unload();

    bool shouldKeepAlive = (selfHaveUnloadHandler || childHaveUnloadHandler);

    return shouldKeepAlive;
}


bool
MovieClip::loadMovie(const URL& url, const std::string* postdata)
{
    // Get a pointer to our own parent 
    DisplayObject* parent = get_parent();
    if (parent)
    {
        if (postdata)
        {
            log_debug(_("Posting data '%s' to url '%s'"), postdata, url.str());
        }
        
        const movie_root& mr = _vm.getRoot();

        boost::intrusive_ptr<movie_definition> md(
            MovieFactory::makeMovie(url, mr.runInfo(), NULL, true, postdata));

        if (!md)
        {
            log_error(_("can't create movie_definition for %s"),
                url.str());
            return false;
        }

        boost::intrusive_ptr<Movie> extern_movie;
        extern_movie = md->createMovie(parent);
        if (extern_movie == NULL)
        {
            log_error(_("can't create extern Movie "
                "for %s"), url.str());
            return false;
        }

        // Parse query string
        VariableMap vars;
        url.parse_querystring(url.querystring(), vars);
        extern_movie->setVariables(vars);

        // Set lockroot to our value of it
        extern_movie->setLockRoot(getLockRoot());

        // Copy event handlers
        // see testsuite/misc-ming.all/loadMovieTest.swf
        const Events& clipEvs = get_event_handlers();
        // top-level movies can't have clip events, right ?
        assert (extern_movie->get_event_handlers().empty());
        extern_movie->set_event_handlers(clipEvs);

        const std::string& name = get_name();
        assert (parent == extern_movie->get_parent());

        MovieClip* parent_sp = parent->to_movie();
        assert(parent_sp);
       
        if( !name.empty() )
        {
            // TODO: check empty != none...
            extern_movie->set_name(name);
        }
        extern_movie->set_clip_depth(get_clip_depth());
    
        parent_sp->replace_display_object(extern_movie.get(), get_depth(),
                     true, true);
    }
    else
    {
        movie_root& root = _vm.getRoot();
        unsigned int level = get_depth()-DisplayObject::staticDepthOffset;
        
#ifndef GNASH_USE_GC
        // Make sure we won't kill ourself !
        assert(get_ref_count() > 1);
#endif // ndef GNASH_USE_GC

        // how about lockRoot here ?
        root.loadLevel(level, url); // extern_movie.get());
    }

    return true;
}

void 
MovieClip::loadVariables(const std::string& urlstr, 
        VariablesMethod sendVarsMethod)
{
    // Host security check will be will be done by LoadVariablesThread
    // (down by getStream, that is)
    
    const movie_root& mr = _vm.getRoot();
    URL url(urlstr, mr.runInfo().baseURL());

    std::string postdata;
    
    // Encode our vars for sending.
    if (sendVarsMethod != METHOD_NONE) getURLEncodedVars(postdata);

    try 
    {
        const StreamProvider& sp = _vm.getRoot().runInfo().streamProvider();
        
        if (sendVarsMethod == METHOD_POST)
        {
            // use POST method
            _loadVariableRequests.push_back(
                    new LoadVariablesThread(sp, url, postdata));
        }
        else
        {
            // use GET method
            if (sendVarsMethod == METHOD_GET)
            {
                // Append variables
                std::string qs = url.querystring();
                if (qs.empty()) url.set_querystring(postdata);
                else url.set_querystring(qs + "&" + postdata);
            }
            _loadVariableRequests.push_back(new LoadVariablesThread(sp, url));
        }
        _loadVariableRequests.back()->process();
    }
    catch (NetworkException& ex)
    {
        log_error(_("Could not load variables from %s"), url.str());
    }

}

void
MovieClip::processCompletedLoadVariableRequest(LoadVariablesThread& request)
{
    assert(request.completed());

    // TODO: consider adding a setVariables(std::map) for use by this
    //             and by Player class when dealing with -P command-line switch

    string_table& st = _vm.getStringTable();
    LoadVariablesThread::ValuesMap& vals = request.getValues();
    for (LoadVariablesThread::ValuesMap::const_iterator it=vals.begin(),
            itEnd=vals.end();
        it != itEnd; ++it)
    {
        const std::string name = PROPNAME(it->first);
        const std::string& val = it->second;
#ifdef DEBUG_LOAD_VARIABLES
        log_debug(_("Setting variable '%s' to value '%s'"), name, val);
#endif
        set_member(st.find(name), val);
    }

    // We want to call a clip-event too if available, see bug #22116
    on_event(event_id::DATA);
}

/*private*/
void
MovieClip::processCompletedLoadVariableRequests()
{
    // Nothing to do (just for clarity)
    if ( _loadVariableRequests.empty() ) return;

    for (LoadVariablesThreads::iterator it=_loadVariableRequests.begin();
            it != _loadVariableRequests.end(); )
    {
        LoadVariablesThread& request = *(*it);
        if (request.completed())
        {
            processCompletedLoadVariableRequest(request);
            delete *it;
            it = _loadVariableRequests.erase(it);
        }
        else ++it;
    }
}

void
MovieClip::setVariables(VariableMap& vars)
{
    string_table& st = _vm.getStringTable();
    for (VariableMap::const_iterator it=vars.begin(), itEnd=vars.end();
        it != itEnd; ++it)
    {
        const std::string& name = it->first;
        const std::string& val = it->second;
        set_member(st.find(PROPNAME(name)), val);
    }
}

void
MovieClip::removeMovieClip()
{
    int depth = get_depth();
    if ( depth < 0 || depth > 1048575 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("removeMovieClip(%s): movieclip depth (%d) out of the "
            "'dynamic' zone [0..1048575], won't remove"),
            getTarget(), depth);
        );
        return;
    }

    MovieClip* parent = dynamic_cast<MovieClip*>(get_parent());
    if (parent)
    {
        // second argument is arbitrary, see comments above
        // the function declaration in MovieClip.h
        parent->remove_display_object(depth, 0);
    }
    else
    {
        // removing _level#
        _vm.getRoot().dropLevel(depth);
        // I guess this can only happen if someone uses 
        // _swf.swapDepth([0..1048575])
    }

}

rect
MovieClip::getBounds() const
{
    rect bounds;
    BoundsFinder f(bounds);
    const_cast<DisplayList&>(_displayList).visitAll(f);
    rect drawableBounds = _drawable.getBounds();
    bounds.expand_to_rect(drawableBounds);
    
    return bounds;
}

bool
MovieClip::isEnabled() const
{
    as_value enabled;
    // const_cast needed due to get_member being non-const due to the 
    // possibility that a getter-setter would actually modify us ...
    if (!const_cast<MovieClip*>(this)->get_member(NSV::PROP_ENABLED, &enabled))
    {
         // We're enabled if there's no 'enabled' member...
         return true;
    }
    return enabled.to_bool();
}

bool
MovieClip::allowHandCursor() const
{
    as_value val;
    // const_cast needed due to get_member being non-const due to the 
    // possibility that a getter-setter would actually modify us ...
    if (!const_cast<MovieClip*>(this)->get_member(
                NSV::PROP_USEHANDCURSOR, &val))
    {
         // true if not found..
         return true;
    }
    return val.to_bool();
}

class EnumerateVisitor {

    as_environment& _env;

public:
    EnumerateVisitor(as_environment& env)
        :
        _env(env)
    {}

    void operator() (DisplayObject* ch)
    {
        // Don't enumerate unloaded DisplayObjects
        if (ch->unloaded()) return;
        
        const std::string& name = ch->get_name();
        // Don't enumerate unnamed DisplayObjects
        if (name.empty()) return;
        
        _env.push(name);
    }
};

void
MovieClip::enumerateNonProperties(as_environment& env) const
{
    EnumerateVisitor visitor(env);
    _displayList.visitAll(visitor);
}

void
MovieClip::cleanupDisplayList()
{
    //log_debug("%s.cleanDisplayList() called, current dlist is %p", 
    //getTarget(), (void*)&_displayList);
    _displayList.removeUnloaded();

    cleanup_textfield_variables();
}

#ifdef GNASH_USE_GC
struct ReachableMarker {
    void operator() (DisplayObject *ch)
    {
        ch->setReachable();
    }
};
void
MovieClip::markReachableResources() const
{
    ReachableMarker marker;

    _displayList.visitAll(marker);

    _environment.markReachableResources();

    // Mark our own definition
    if (_def.get()) _def->setReachable();

    // Mark textfields in the TextFieldIndex
    if ( _text_variables.get() )
    {
        for (TextFieldIndex::const_iterator i=_text_variables->begin(),
                    e=_text_variables->end();
                i!=e; ++i)
        {
            const TextFields& tfs=i->second;
            std::for_each(tfs.begin(), tfs.end(), 
                        boost::mem_fn(&DisplayObject::setReachable));
        }
    }

    // Mark our relative root
    _swf->setReachable();

    markDisplayObjectReachable();

}
#endif // GNASH_USE_GC

void
MovieClip::destroy()
{
    stopStreamSound();
    _displayList.destroy();
    DisplayObject::destroy();
}

cxform
MovieClip::get_world_cxform() const
{
    cxform cf = DisplayObject::get_world_cxform();
    cf.concatenate(_userCxform); 
    return cf;
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
    //
    // TODO2: implement this with iteration rather
    //                then recursion.
    //                

    DisplayObject* parent = get_parent();
    if ( ! parent ) return this; // no parent, we're the root

    // If we have a parent, we descend to it unless 
    // our _lockroot is true AND our or the VM's
    // SWF version is > 6
    int topSWFVersion = getVM().getRoot().getRootMovie().version();

    if (getSWFVersion() > 6 || topSWFVersion > 6) {
        if (getLockRoot()) return this;
    }

    return parent->getAsRoot();
}

as_value
MovieClip::lockroot_getset(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = ensureType<MovieClip>(fn.this_ptr);

    as_value rv;
    if ( fn.nargs == 0 ) // getter
    {
        rv.set_bool(ptr->getLockRoot());
    }
    else // setter
    {
        ptr->setLockRoot(fn.arg(0).to_bool());
    }
    return rv;

}

void
MovieClip::setStreamSoundId(int id)
{
    if ( id != m_sound_stream_id )
    {
        log_debug(_("Stream sound id from %d to %d, stopping old"),
                m_sound_stream_id, id);
        stopStreamSound();
    }
    m_sound_stream_id = id;
}

void
MovieClip::stopStreamSound()
{
    if ( m_sound_stream_id == -1 ) return; // nothing to do

    sound::sound_handler* handler = _vm.getRoot().runInfo().soundHandler();
    if (handler)
    {
        handler->stop_sound(m_sound_stream_id);
    }

    m_sound_stream_id = -1;
}

void
MovieClip::setPlayState(PlayState s)
{
    if (s == _playState) return; // nothing to do
    if (s == PLAYSTATE_STOP) stopStreamSound();
    _playState = s;
}

} // namespace gnash
