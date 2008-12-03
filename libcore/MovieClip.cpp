// MovieClip.cpp:  Stateful live Sprite instance, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "log.h" 
#include "action.h" // for call_method_parsed (call_method_args)
#include "MovieClip.h"
#include "movie_definition.h"
#include "as_value.h"
#include "as_function.h"
#include "TextField.h" // for registered variables
#include "ControlTag.h"
#include "fn_call.h"
#include "Key_as.h"
#include "movie_root.h"
#include "movie_instance.h"
#include "swf_event.h"
#include "sprite_definition.h"
#include "ActionExec.h"
#include "builtin_function.h"
#include "smart_ptr.h" // GNASH_USE_GC
#include "VM.h"
#include "Range2d.h" // for getBounds
#include "GnashException.h"
#include "URL.h"
#include "sound_handler.h"
#include "StreamProvider.h"
#include "LoadVariablesThread.h" 
#include "ExecutableCode.h" // for inheritance of ConstructEvent
#include "Object.h" // for getObjectInterface
#include "DynamicShape.h" // for composition
#include "namedStrings.h"
#include "fill_style.h" // for beginGradientFill
#include "styles.h" // for cap_style_e and join_style_e enums
#include "PlaceObject2Tag.h" 
#include "NetStream_as.h"
#include "flash/geom/Matrix_as.h"
#include "ExportableResource.h"

#ifdef USE_SWFTREE
# include "tree.hh"
#endif

#include <vector>
#include <string>
#include <cmath>
#include <algorithm> // for std::swap
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>

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
// This includes get_topmost_mouse_entity and findDropTarget
//
//#define DEBUG_MOUSE_ENTITY_FINDING 1

// Forward declarations
namespace {
    as_object* getMovieClipInterface();
    void attachMovieClipInterface(as_object& o);
    void attachMovieClipProperties(character& o);

    as_value movieclip_transform(const fn_call& fn);
    as_value movieclip_blendMode(const fn_call& fn);
    as_value movieclip_scale9Grid(const fn_call& fn);
    as_value movieclip_attachVideo(const fn_call& fn);
    as_value movieclip_attachAudio(const fn_call& fn);
    as_value movieclip_attachMovie(const fn_call& fn);
    as_value movieclip_unloadMovie(const fn_call& fn);
    as_value movieclip_loadMovie(const fn_call& fn);
    as_value movieclip_getURL(const fn_call& fn);
    as_value movieclip_ctor(const fn_call& fn);
    as_value movieclip_attachBitmap(const fn_call& fn);
    as_value movieclip_beginBitmapFill(const fn_call& fn);
    as_value movieclip_createEmptyMovieClip(const fn_call& fn);
    as_value movieclip_removeMovieClip(const fn_call& fn);
    as_value movieclip_createTextField(const fn_call& fn);
    as_value movieclip_curveTo(const fn_call& fn);
    as_value movieclip_beginFill(const fn_call& fn);
    as_value movieclip_prevFrame(const fn_call& fn);
    as_value movieclip_nextFrame(const fn_call& fn);
    as_value movieclip_endFill(const fn_call& fn);
    as_value movieclip_clear(const fn_call& fn);
    as_value movieclip_lineStyle(const fn_call& fn);
    as_value movieclip_lineTo(const fn_call& fn);
    as_value movieclip_moveTo(const fn_call& fn);
    as_value movieclip_beginGradientFill(const fn_call& fn);
    as_value movieclip_stopDrag(const fn_call& fn);
    as_value movieclip_startDrag(const fn_call& fn);
    as_value movieclip_removeMovieClip(const fn_call& fn);
    as_value movieclip_gotoAndStop(const fn_call& fn);
    as_value movieclip_duplicateMovieClip(const fn_call& fn);
    as_value movieclip_gotoAndPlay(const fn_call& fn);
    as_value movieclip_stop(const fn_call& fn);
    as_value movieclip_play(const fn_call& fn);
    as_value movieclip_setMask(const fn_call& fn);
    as_value movieclip_getDepth(const fn_call& fn);
    as_value movieclip_getBytesTotal(const fn_call& fn);
    as_value movieclip_getBytesLoaded(const fn_call& fn);
    as_value movieclip_getBounds(const fn_call& fn);
    as_value movieclip_hitTest(const fn_call& fn);
    as_value movieclip_globalToLocal(const fn_call& fn);
    as_value movieclip_localToGlobal(const fn_call& fn);
    as_value movieclip_swapDepths(const fn_call& fn);
    as_value movieclip_scrollRect(const fn_call& fn);
    as_value movieclip_getInstanceAtDepth(const fn_call& fn);
    as_value movieclip_getNextHighestDepth(const fn_call& fn);
    as_value movieclip_getTextSnapshot(const fn_call& fn);
    as_value movieclip_tabIndex(const fn_call& fn);
    as_value movieclip_forceSmoothing(const fn_call& fn);
    as_value movieclip_opaqueBackground(const fn_call& fn);
    as_value movieclip_filters(const fn_call& fn);
    as_value movieclip_forceSmoothing(const fn_call& fn);
    as_value movieclip_cacheAsBitmap(const fn_call& fn);
    as_value movieclip_lineGradientStyle(const fn_call& fn);
    as_value movieclip_getRect(const fn_call& fn);
    as_value movieclip_meth(const fn_call& fn);
    as_value movieclip_getSWFVersion(const fn_call& fn);
    as_value movieclip_loadVariables(const fn_call& fn);
    as_value movieclip_(const fn_call& fn);

}

/// Anonymous namespace for module-private definitions
namespace
{

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

/// A DisplayList visitor used to compute its overall bounds.
//
class BoundsFinder {
public:
    rect& _bounds;
    BoundsFinder(rect& b)
        :
        _bounds(b)
    {}
    void operator() (character* ch)
    {
        // don't include bounds of unloaded characters
        if ( ch->isUnloaded() ) return;
        rect chb = ch->getBounds();
        SWFMatrix m = ch->getMatrix();
        _bounds.expand_to_transformed_rect(m, chb);
    }
};

/// A DisplayList visitor used to extract script characters
//
/// Script characters are characters created or transformed
/// by ActionScript. 
///
class ScriptObjectsFinder {
    std::vector<character*>& _dynamicChars;
    std::vector<character*>& _staticChars;
public:
    ScriptObjectsFinder(std::vector<character*>& dynamicChars,
            std::vector<character*>& staticChars)
        :
        _dynamicChars(dynamicChars),
        _staticChars(staticChars)
    {}

    void operator() (character* ch) 
    {
        // don't include bounds of unloaded characters
        if ( ch->isUnloaded() ) return;

        // TODO: Are script-transformed object to be kept ?
        //             Need a testcase for this
        //if ( ! ch->get_accept_anim_moves() )
        //if ( ch->isDynamic() )
        int depth = ch->get_depth();
        if ( depth < character::lowerAccessibleBound || depth >= 0 )
        {
            _dynamicChars.push_back(ch);
        }
        else
        {
            _staticChars.push_back(ch);
        }
    }
};

} // anonymous namespace


MovieClip::MovieClip( movie_definition* def, movie_instance* r,
        character* parent, int id)
    :
    character(parent, id),
    m_root(r),
    _drawable(new DynamicShape()),
    _drawable_inst(_drawable->create_character_instance(this, 0)),
    m_play_state(PLAY),
    m_current_frame(0),
    m_has_looped(false),
    _callingFrameActions(false),
    m_as_environment(_vm),
    _text_variables(),
    m_sound_stream_id(-1),
    _userCxform(),
    _droptarget(),
    _lockroot(false),
    m_def(def)
{
    assert(m_def != NULL);
    assert(m_root != NULL);

    set_prototype(getMovieClipInterface());
            
    //m_root->add_ref();    // @@ circular!
    m_as_environment.set_target(this);

    // TODO: have the 'MovieClip' constructor take care of this !
    attachMovieClipProperties(*this);

}

MovieClip::~MovieClip()
{
    stopStreamSound();

    // We might have been deleted by Quit... 
    //assert(isDestroyed());

    _vm.getRoot().remove_key_listener(this);
    _vm.getRoot().remove_mouse_listener(this);

    for (LoadVariablesThreads::iterator it=_loadVariableRequests.begin();
            it != _loadVariableRequests.end(); ++it)
    {
        delete *it;
    }
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

character*
MovieClip::get_character_at_depth(int depth)
{
    return m_display_list.get_character_at_depth(depth);
}

// Set *val to the value of the named member and
// return true, if we have the named member.
// Otherwise leave *val alone and return false.
bool
MovieClip::get_member(string_table::key name_key, as_value* val,
    string_table::key nsname)
{
    // FIXME: use addProperty interface for these !!
    // TODO: or at least have a character:: protected method take
    //             care of these ?
    //             Duplicates code in character::get_path_element_character too..
    //
    if (getSWFVersion() > 4 && name_key == NSV::PROP_uROOT)
    {

        // getAsRoot() will take care of _lockroot
        val->set_as_object( const_cast<MovieClip*>( getAsRoot() )    );
        return true;
    }

    // NOTE: availability of _global doesn't depend on VM version
    //             but on actual movie version. Example: if an SWF4 loads
    //             an SWF6 (to, say, _level2), _global will be unavailable
    //             to the SWF4 code but available to the SWF6 one.
    //
    if ( getSWFVersion() > 5 && name_key == NSV::PROP_uGLOBAL ) // see MovieClip.as
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
        movie_instance* mo = _vm.getRoot().getLevel(levelno).get();
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
    character* ch;
    if ( _vm.getSWFVersion() >= 7 ) {
        ch = m_display_list.get_character_by_name(name);
    }
    else ch = m_display_list.get_character_by_name_i(name);
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
    TextFieldPtrVect* etc = get_textfield_variable(name);
    if ( etc )
    {
        for (TextFieldPtrVect::const_iterator i=etc->begin(), e=etc->end();
                i!=e; ++i)
        {
            TextFieldPtr tf = *i;
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
    //GNASH_REPORT_FUNCTION;

    std::string fspecStr = frame_spec.to_string();

    as_value str(fspecStr);

    double num = str.to_number();

    //log_debug("get_frame_number(%s), num: %g", frame_spec, num);

    if ( ! utility::isFinite(num) || int(num) != num || num == 0)
    {
        bool ret = m_def->get_labeled_frame(fspecStr, frameno);
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
void MovieClip::call_frame_actions(const as_value& frame_spec)
{
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

#if 0 // why would we want to do this ?
    // Set the current sound_stream_id to -1, meaning that no stream are
    // active. If there are an active stream it will be updated while
    // executing the ControlTags.
    set_sound_stream_id(-1);
#endif

    // Execute the ControlTag actions
    // We set _callingFrameActions to true so that add_action_buffer
    // will execute immediately instead of queuing them.
    // NOTE: in case gotoFrame is executed by code in the called frame
    //             we'll temporarly clear the _callingFrameActions flag
    //             to properly queue actions back on the global queue.
    //
    _callingFrameActions=true;
    const PlayList* playlist = m_def->getPlaylist(frame_number);
    if ( playlist )
    {
    PlayList::const_iterator it = playlist->begin();
        const PlayList::const_iterator e = playlist->end();
    for(; it != e; it++)
    {
        (*it)->execute_action(this, m_display_list);
    }
    }
    _callingFrameActions=false;

}

character* MovieClip::add_empty_movieclip(const char* name, int depth)
{
    // empty_movieclip_def will be deleted during deleting movieclip
    sprite_definition* empty_sprite_def =
        new sprite_definition(*get_movie_definition());

    MovieClip* movieclip = new MovieClip(empty_sprite_def, m_root, this, 0);
    movieclip->set_name(name);
    movieclip->setDynamic();

    // TODO: only call set_invalidated if this character actually overrides
    //             an existing one !
    set_invalidated(); 

    m_display_list.place_character(movieclip, depth);     

    return movieclip;
}

boost::intrusive_ptr<character>
MovieClip::add_textfield(const std::string& name, int depth, int x, int y, float width, float height)
{
    // Create a definition (TODO: cleanup this thing, definitions should be immutable!)
    
    // Set textfield bounds
    rect bounds(0, 0, PIXELS_TO_TWIPS(width), PIXELS_TO_TWIPS(height));

    // Create an instance
    boost::intrusive_ptr<character> txt_char = new TextField(this, bounds);

    // Give name and mark as dynamic
    txt_char->set_name(name);
    txt_char->setDynamic();

    // Set _x and _y
    SWFMatrix txt_matrix;
    txt_matrix.set_translation(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
    txt_char->setMatrix(txt_matrix, true); // update caches (altought shouldn't be needed as we only set translation)

    // Here we add the character to the displayList.    
    m_display_list.place_character(txt_char.get(), depth); 

    return txt_char;
}

boost::intrusive_ptr<MovieClip> 
MovieClip::duplicateMovieClip(const std::string& newname, int depth,
        as_object* initObject)
{
    character* parent_ch = get_parent();
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

    boost::intrusive_ptr<MovieClip> newmovieclip = new MovieClip(m_def.get(),
            m_root, parent, get_id());
    newmovieclip->set_name(newname);

    newmovieclip->setDynamic();

    //if ( initObject ) newmovieclip->copyProperties(*initObject);
    //else newmovieclip->copyProperties(*this);

    // Copy event handlers from movieclip
    // We should not copy 'm_action_buffer' since the 
    // 'm_method' already contains it
    newmovieclip->set_event_handlers(get_event_handlers());

    // Copy drawable
    newmovieclip->_drawable = new DynamicShape(*_drawable);
    
    newmovieclip->set_cxform(get_cxform());    
    newmovieclip->copyMatrix(*this); // copy SWFMatrix and caches
    newmovieclip->set_ratio(get_ratio());    
    newmovieclip->set_clip_depth(get_clip_depth());    
    
    parent->m_display_list.place_character(newmovieclip.get(), depth, 
            initObject);
    
    return newmovieclip; 
}

/* public */
void
MovieClip::queueAction(const action_buffer& action)
{
    movie_root& root = _vm.getRoot();
    root.pushAction(action, boost::intrusive_ptr<MovieClip>(this));
}

/* private */
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
    if ( id.m_id == event_id::ENTER_FRAME && isUnloaded() )
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
            
    // First, check for clip event handler.
    {
        std::auto_ptr<ExecutableCode> code ( get_event_handler(id) );
        if ( code.get() )
        {
            // Dispatch.
            code->execute();

            called = true;
        }
    }

    // Fall through and call the function also, if it's defined!


    // user-defined onInitialize is never called
    if ( id.m_id == event_id::INITIALIZE )
    {
            testInvariant();
            return called;
    }


    // NOTE: user-defined onLoad is not invoked for static
    //             clips on which no clip-events are defined.
    //             see testsuite/misc-ming.all/action_execution_order_extend_test.swf
    //
    //     Note that this can't be true for movieclips
    //     not placed by PlaceObject, see
    //     testsuite/misc-ming.all/registerClassTest.swf
    //
    //     Note that this is also not true for movieclips which have
    //     a registered class on them, see
    //     testsuite/misc-ming.all/registerClassTest2.swf
    //
    //     TODO: test the case in which it's MovieClip.prototype.onLoad defined !
    //
    if ( id.m_id == event_id::LOAD )
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

            sprite_definition* def =
                dynamic_cast<sprite_definition*>(m_def.get());

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
            getUserDefinedEventHandler(id.get_function_key());

        if ( method )
        {
            call_method0(as_value(method.get()), &m_as_environment, this);
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
    //log_debug("%s.get_path_element(%s) called", getTarget(), _vm.getStringTable().value(key));
    as_object* obj = get_path_element_character(key);
    if ( obj )
    {
        return obj;
    }

    std::string name = _vm.getStringTable().value(key);

    // See if we have a match on the display list.
    character* ch;
    if ( _vm.getSWFVersion() >= 7 ) ch = 
        m_display_list.get_character_by_name(name);

    else ch = m_display_list.get_character_by_name_i(name);

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
    //             which would scan the child characters again
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
#ifdef DEBUG_DYNTEXT_VARIABLES
    //log_debug(_("movieclip[%p]::set_member(%s, %s)"), (void*)this, VM::get().getStringTable().value(name), val);
#endif

    //if ( val.is_function() )
    //{
    //    checkForKeyOrMouseEvent(VM::get().getStringTable().value(name));
    //}

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
    TextFieldPtrVect* etc = get_textfield_variable(_vm.getStringTable().value(name));
    if ( etc )
    {
#ifdef DEBUG_DYNTEXT_VARIABLES
        log_debug(_("it's a Text Variable, associated with %d TextFields"), etc->size());
#endif
        for (TextFieldPtrVect::iterator i=etc->begin(), e=etc->end(); i!=e; ++i)
        {
            TextFieldPtr tf = *i;
            tf->updateText(val.to_string());
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

void
MovieClip::advance_sprite()
{

    assert(!isUnloaded());

    // call_frame should never trigger advance_movieclip
    assert(!_callingFrameActions);

    // We might have loaded NO frames !
    if ( get_loaded_frames() == 0 )
    {
        IF_VERBOSE_MALFORMED_SWF(
        LOG_ONCE( log_swferror(_("advance_movieclip: no frames loaded "
                    "for movieclip/movie %s"), getTarget()) );
        );
        return;
    }


    // Process any pending loadVariables request
    processCompletedLoadVariableRequests();

#ifdef GNASH_DEBUG
    size_t frame_count = m_def->get_frame_count();

    log_debug(_("Advance_movieclip for movieclip '%s' - frame %u/%u "),
        getTarget(), m_current_frame,
        frame_count);
#endif

    // I'm not sure ENTERFRAME goes in a different queue then DOACTION...
    queueEvent(event_id::ENTER_FRAME, movie_root::apDOACTION);
    //queueEvent(event_id::ENTER_FRAME, apENTERFRAME);

    // Update current and next frames.
    if (m_play_state == PLAY)
    {
#ifdef GNASH_DEBUG
        log_debug(_("MovieClip::advance_movieclip we're in PLAY mode"));
#endif

        int prev_frame = m_current_frame;

#ifdef GNASH_DEBUG
        log_debug(_("on_event_load called, incrementing"));
#endif
        increment_frame_and_check_for_loop();
#ifdef GNASH_DEBUG
        log_debug(_("after increment we are at frame %u/%u"), m_current_frame, frame_count);
#endif

        // Execute the current frame's tags.
        // First time execute_frame_tags(0) executed in dlist.cpp(child) or SWFMovieDefinition(root)
        if (m_current_frame != (size_t)prev_frame)
        {
            if ( m_current_frame == 0 && has_looped() )
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
                            "%s"), m_current_frame, getTarget());
#endif
                // Make sure m_current_frame is 0-based during execution of DLIST tags
                execute_frame_tags(m_current_frame, m_display_list,
                        TAG_DLIST|TAG_ACTION);
            }
        }
    }
#ifdef GNASH_DEBUG
    else
    {
        log_debug(_("MovieClip::advance_movieclip we're in STOP mode"));
        // shouldn't we execute frame tags anyway when in STOP mode ?
        //execute_frame_tags(m_current_frame);
    }
#endif
}

// child movieclip advance
void
MovieClip::advance()
{
//    GNASH_REPORT_FUNCTION;

#ifdef GNASH_DEBUG
    log_debug(_("Advance movieclip '%s' at frame %u/%u"),
        getTargetPath(), m_current_frame,
        get_frame_count());
#endif

    // child movieclip frame rate is the same the root movieclip frame rate
    // that's why it is not needed to analyze 'm_time_remainder'

    advance_sprite();

}

void
MovieClip::execute_init_action_buffer(const action_buffer& a, int cid)
{
    // WARNING! get_root() would depend on _lockroot !!
    movie_instance* mi = m_root; 
    if ( mi->setCharacterInitialized(cid) )
    {
#ifdef GNASH_DEBUG
        log_debug(_("Queuing init actions in frame %d of movieclip %s"),
                m_current_frame, getTarget());
#endif
        std::auto_ptr<ExecutableCode> code ( 
                new GlobalCode(a, boost::intrusive_ptr<MovieClip>(this)) );

        movie_root& root = _vm.getRoot();
        root.pushAction(code, movie_root::apINIT);
    }
    else
    {
#ifdef GNASH_DEBUG
        log_debug(_("Init actions for character %d already executed"), cid);
#endif
    }
}

void
MovieClip::execute_action(const action_buffer& ab)
{
    as_environment& env = m_as_environment; // just type less

    ActionExec exec(ab, env);
    exec();
}

/*private*/
void
MovieClip::restoreDisplayList(size_t tgtFrame)
{
    // This is not tested as usable for jump-forwards (yet)...
    // TODO: I guess just moving here the code currently in goto_frame
    //             for jump-forwards would do
    assert(tgtFrame <= m_current_frame);

    // Just invalidate this character before jumping back.
    // Should be optimized, but the invalidating model is not clear enough,
    // and there are some old questions spreading the source files.
    set_invalidated();

    DisplayList m_tmp_display_list;
    for (size_t f = 0; f<tgtFrame; ++f)
    {
        m_current_frame = f;
        execute_frame_tags(f, m_tmp_display_list, TAG_DLIST);
    }

    // Execute both action tags and DLIST tags of the target frame
    m_current_frame = tgtFrame;
    execute_frame_tags(tgtFrame, m_tmp_display_list, TAG_DLIST|TAG_ACTION);

    m_display_list.mergeDisplayList(m_tmp_display_list);
}

// 0-based frame number !
void
MovieClip::execute_frame_tags(size_t frame, DisplayList& dlist, int typeflags)
{
    testInvariant();

    assert(typeflags);

    const PlayList* playlist = m_def->getPlaylist(frame);
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

        if ( (typeflags&TAG_DLIST) && (typeflags&TAG_ACTION) )
        {
            for( ; it != e; it++)
            {
                (*it)->execute(this, dlist);
            }
        }
        else if ( typeflags & TAG_DLIST )
        {
            for( ; it != e; it++)
            {
                (*it)->execute_state(this, dlist);
            }
        }
        else
        {
            assert(typeflags & TAG_ACTION);
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
        getTargetPath(), target_frame_number, m_current_frame);
#endif

    // goto_frame stops by default.
    // ActionGotoFrame tells the movieClip to go to the target frame 
    // and stop at that frame. 
    set_play_state(STOP);

    if ( target_frame_number > m_def->get_frame_count() - 1)
    {
        target_frame_number = m_def->get_frame_count() - 1;

        if ( ! m_def->ensure_frame_loaded(target_frame_number+1) )
        {
            log_error(_("Target frame of a gotoFrame(%d) was never loaded,"
                        "although frame count in header (%d) said we "
                        "should have found it"),
                        target_frame_number+1, m_def->get_frame_count());
            return; // ... I guess, or not ?
        }

        // Just set _currentframe and return.
        m_current_frame = target_frame_number;

        // don't push actions, already tested.
        return;
    }

    if (target_frame_number == m_current_frame)
    {
        // don't push actions
        return;
    }

    // Unless the target frame is the next one, stop playback of soundstream
    if (target_frame_number != m_current_frame+1 )
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
        if ( ! m_def->ensure_frame_loaded(target_frame_number+1) )
        {
            log_error(_("Target frame of a gotoFrame(%d) was never loaded, "
                        "although frame count in header (%d) said we should"
                        " have found it"),
                        target_frame_number+1, m_def->get_frame_count());
            return; // ... I guess, or not ?
        }
    }


    //
    // Construct the DisplayList of the target frame
    //

    if (target_frame_number < m_current_frame)
    {
        // Go backward to a previous frame
        // NOTE: just in case we're being called by code in a called frame
        // we'll backup and resume the _callingFrameActions flag
        bool callingFrameActionsBackup = _callingFrameActions;
        _callingFrameActions = false;

        // restoreDisplayList takes care of properly setting the 
        // m_current_frame variable
        restoreDisplayList(target_frame_number);
        assert(m_current_frame == target_frame_number);
        _callingFrameActions = callingFrameActionsBackup;
    }
    else
    // Go forward to a later frame
    {
        // We'd immediately return if target_frame_number == m_current_frame
        assert(target_frame_number > m_current_frame);
        while (++m_current_frame < target_frame_number)
        {
            //for (size_t f = m_current_frame+1; f<target_frame_number; ++f) 
            // Second argument requests that only "DisplayList" tags
            // are executed. This means NO actions will be
            // pushed on m_action_list.
            execute_frame_tags(m_current_frame, m_display_list, TAG_DLIST);
        }
        assert(m_current_frame == target_frame_number);


        // Now execute target frame tags (queuing actions)
        // NOTE: just in case we're being called by code in a called frame
        //             we'll backup and resume the _callingFrameActions flag
        bool callingFrameActionsBackup = _callingFrameActions;
        _callingFrameActions = false;
        execute_frame_tags(target_frame_number, m_display_list,
                TAG_DLIST|TAG_ACTION);
        _callingFrameActions = callingFrameActionsBackup;
    }

    assert(m_current_frame == target_frame_number);
}

bool MovieClip::goto_labeled_frame(const std::string& label)
{
    size_t target_frame;
    if (m_def->get_labeled_frame(label, target_frame))
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

void MovieClip::display()
{
    //GNASH_REPORT_FUNCTION;

    // Note: 
    // DisplayList::Display() will take care of the visibility checking.
    //
    // Whether a character should be rendered or not is dependent on its parent.
    // i.e. if its parent is a mask, this character should be rendered to the mask
    // buffer even it is invisible.
    //

    
    // render drawable (ActionScript generated graphics)
    
    _drawable->finalize();
    // TODO: I'd like to draw the definition directly..
    //             but it seems that the backend insists in
    //             accessing the *parent* of the character
    //             passed as "instance" for the drawing.
    //             When displaying top-level movie this will
    //             be NULL and gnash will segfault
    //             Thus, this drawable_instance is basically just
    //             a container for a parent :(
    _drawable_inst->display();
    
    
    // descend the display list
    m_display_list.display();
     
    clear_invalidated();
}

void MovieClip::omit_display()
{
    if (m_child_invalidated)
        m_display_list.omit_display();
        
    clear_invalidated();
}

bool
MovieClip::attachCharacter(character& newch, int depth, as_object* initObject)
{ 
    m_display_list.place_character(&newch, depth, initObject);    

    // FIXME: check return from place_character above ?
    return true; 
}

character*
MovieClip::add_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{
    assert(m_def);
    assert(tag);

    character_def* cdef = m_def->get_character_def(tag->getID());
    if (!cdef)
    {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("MovieClip::add_display_object(): "
                    "unknown cid = %d"), tag->getID());
        );
        return NULL;
    }
    
    character* existing_char = dlist.get_character_at_depth(tag->getDepth());
    
    if (existing_char) return NULL;

    boost::intrusive_ptr<character> ch =
        cdef->create_character_instance(this, tag->getID());

    if (tag->hasName()) ch->set_name(tag->getName());
    else if (ch->wantsInstanceName())
    {
        std::string instance_name = getNextUnnamedInstanceName();
        ch->set_name(instance_name);
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
    
    dlist.place_character(ch.get(), tag->getDepth());
    return ch.get();
}

void 
MovieClip::move_display_object(const SWF::PlaceObject2Tag* tag, DisplayList& dlist)
{    
    int ratio = tag->getRatio();
    // clip_depth is not used in MOVE tag(at least no related tests). 
    dlist.move_character(
        tag->getDepth(), 
        tag->hasCxform() ? &tag->getCxform() : NULL,
        tag->hasMatrix() ? &tag->getMatrix() : NULL,
        tag->hasRatio()    ? &ratio    : NULL,
        NULL);
}

void MovieClip::replace_display_object(const SWF::PlaceObject2Tag* tag, DisplayList& dlist)
{
    assert(m_def != NULL);
    assert(tag != NULL);

    character_def*    cdef = m_def->get_character_def(tag->getID());
    if (cdef == NULL)
    {
        log_error(_("movieclip::replace_display_object(): "
            "unknown cid = %d"), tag->getID());
        return;
    }
    assert(cdef);

    character* existing_char = dlist.get_character_at_depth(tag->getDepth());

    if (existing_char)
    {
        // if the existing character is not a shape, move it instead of replace
        if ( existing_char->isActionScriptReferenceable() )
        {
            move_display_object(tag, dlist);
            return;
        }
        else
        {
            boost::intrusive_ptr<character> ch = 
                cdef->create_character_instance(this, tag->getID());

            // TODO: check if we can drop this for REPLACE!
            // should we rename the character when it's REPLACE tag?
            if(tag->hasName())
            {
                ch->set_name(tag->getName());
            }
            else if(ch->wantsInstanceName())
            {
                std::string instance_name = getNextUnnamedInstanceName();
                ch->set_name(instance_name);
            }
            if(tag->hasRatio())
            {
                ch->set_ratio(tag->getRatio());
            }
            if(tag->hasCxform())
            {
                ch->set_cxform(tag->getCxform());
            }
            if(tag->hasMatrix())
            {
                ch->setMatrix(tag->getMatrix(), true); // update caches
            }

            // use SWFMatrix from the old character if tag doesn't provide one.
            dlist.replace_character(ch.get(), tag->getDepth(), 
                !tag->hasCxform(), 
                !tag->hasMatrix());
        }
    }
    else // non-existing character
    {
        log_error(_("MovieClip::replace_display_object: could not "
                "find any character at depth %d"), tag->getDepth());
    } 
}

void
MovieClip::remove_display_object(const SWF::PlaceObject2Tag* tag,
        DisplayList& dlist)
{
    set_invalidated();
    dlist.remove_character(tag->getDepth());
}

void
MovieClip::replace_display_object(character* ch, int depth, 
        bool use_old_cxform, bool use_old_matrix)
{
    assert(ch);
    m_display_list.replace_character(ch, depth,
            use_old_cxform, use_old_matrix);
}

int
MovieClip::get_id_at_depth(int depth)
{
    character* ch = m_display_list.get_character_at_depth(depth);
    if ( ! ch ) return -1;
    return ch->get_id();
}

void
MovieClip::increment_frame_and_check_for_loop()
{
    size_t frame_count = get_loaded_frames(); 
    if ( ++m_current_frame >= frame_count )
    {
        // Loop.
        m_current_frame = 0;
        m_has_looped = true;
        if (frame_count > 1)
        {
            //m_display_list.reset();
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
    return can_handle_mouse_event();
}

/// Find a character hit by the given coordinates.
//
/// This class takes care about taking masks layers into
/// account, but nested masks aren't properly tested yet.
///
class MouseEntityFinder {

    /// Highest depth hidden by a mask
    //
    /// This will be -1 initially, and set
    /// the the depth of a mask when the mask
    /// doesn't contain the query point, while
    /// scanning a DisplayList bottom-up
    ///
    int _highestHiddenDepth;

    character* _m;

    typedef std::vector<character*> Candidates;
    Candidates _candidates;

    /// Query point in world coordinate space
    point    _wp;

    /// Query point in parent coordinate space
    point    _pp;

    bool _checked;

public:

    /// @param wp
    ///     Query point in world coordinate space
    ///
    /// @param pp
    ///     Query point in parent coordinate space
    ///
 MouseEntityFinder(point    wp, point    pp)
        :
        _highestHiddenDepth(std::numeric_limits<int>::min()),
        _m(NULL),
        _candidates(),
        _wp(wp),
        _pp(pp),
        _checked(false)
    {}

    void operator() (character* ch)
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
            //if ( ! ch->get_visible() ) {
            //    log_debug("invisible mask in MouseEntityFinder.");
            //}
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

        if ( ! ch->get_visible() ) return;

        _candidates.push_back(ch);

    }

    void checkCandidates()
    {
        if ( _checked ) return;
        for (Candidates::reverse_iterator i=_candidates.rbegin(),
                        e=_candidates.rend(); i!=e; ++i)
        {
            character* ch = *i;
            character* te = ch->get_topmost_mouse_entity(_pp.x, _pp.y);
            if ( te )
            {
                _m = te;
                break;
            }
        }
        _checked = true;
    }

    character* getEntity()
    {
        checkCandidates();
#ifdef DEBUG_MOUSE_ENTITY_FINDING
        if ( _m ) 
        {
            log_debug(_("MouseEntityFinder found character %s (depth %d) "
                    "hitting point %g,%g"),
                    _m->getTarget(), _m->get_depth(), _wp.x, _wp.y);
        }
#endif // DEBUG_MOUSE_ENTITY_FINDING
        return _m;
    }
        
};

/// Find the first character whose shape contain the point
//
/// Point coordinates in world TWIPS
///
class ShapeContainerFinder {

    bool _found;
    boost::int32_t    _x;
    boost::int32_t    _y;

public:

    ShapeContainerFinder(boost::int32_t x, boost::int32_t y)
        :
        _found(false),
        _x(x),
        _y(y)
    { }

    bool operator() (character* ch)
    {
        if ( ch->pointInShape(_x, _y) )
        {
            _found = true;
            return false;
        }
        else return true;
    }

    bool hitFound() { return _found; }
        
};

/// Find the first visible character whose shape contain the point
//
/// Point coordinates in world TWIPS
///
class VisibleShapeContainerFinder {

    bool _found;
    boost::int32_t    _x;
    boost::int32_t    _y;

public:

    VisibleShapeContainerFinder(boost::int32_t x, boost::int32_t    y)
        :
        _found(false),
        _x(x),
        _y(y)
    {}

    bool operator() (character* ch)
    {
        if ( ch->pointInVisibleShape(_x, _y) )
        {
            _found = true;
            return false;
        }
        else return true;
    }

    bool hitFound() { return _found; }
        
};

/// Find the first hitable character whose shape contain the point 
// 
/// Point coordinates in world TWIPS 
/// 
class HitableShapeContainerFinder { 
    bool _found; 
    boost::int32_t    _x; // TWIPS
    boost::int32_t    _y; // TWIPS
        
public: 
    HitableShapeContainerFinder(boost::int32_t x, boost::int32_t y) 
            : 
    _found(false), 
    _x(x), 
    _y(y) 
    {} 

    bool operator() (character* ch) 
    { 
        if( ch->isDynamicMask() ) 
        { 
            return true; 
        } 
        else if ( ch->pointInShape(_x, _y) ) 
        {
            _found = true; 
            return false; 
        } 
        else 
        { 
            return true; 
        }
    } 

    bool hitFound() { return _found; } 
}; 

bool
MovieClip::pointInShape(boost::int32_t x, boost::int32_t y) const
{
    ShapeContainerFinder finder(x, y);
    const_cast<DisplayList&>(m_display_list).visitBackward(finder);
    if ( finder.hitFound() ) return true;
    return _drawable_inst->pointInShape(x, y); 
}

bool
MovieClip::pointInVisibleShape(boost::int32_t x, boost::int32_t y) const
{
    if ( ! get_visible() ) return false;
    if ( isDynamicMask() && ! can_handle_mouse_event() )
    {
        // see testsuite/misc-ming.all/masks_test.swf
#ifdef GNASH_DEBUG_HITTEST
        log_debug(_("%s is a dynamic mask and can't handle mouse "
                    "events, no point will hit it"), getTarget());
#endif
        return false;
    }
    character* mask = getMask(); // dynamic one
    if ( mask && mask->get_visible() && ! mask->pointInShape(x, y) )
    {
#ifdef GNASH_DEBUG_HITTEST
        log_debug(_("%s is dynamically masked by %s, which "
                "doesn't hit point %g,%g"), getTarget(),
                mask->getTarget(), x, y);
#endif
        return false;
    }
    VisibleShapeContainerFinder finder(x, y);
    const_cast<DisplayList&>(m_display_list).visitBackward(finder);
    if ( finder.hitFound() ) return true;
    return _drawable_inst->pointInVisibleShape(x, y); 
}

bool
MovieClip::pointInHitableShape(boost::int32_t x, boost::int32_t y) const
{
    if ( isDynamicMask() && !can_handle_mouse_event() )
    {
        return false;
    }

    character* mask = getMask(); 
    if ( mask && ! mask->pointInShape(x, y) )
    {
        return false;
    }
            
    HitableShapeContainerFinder finder(x, y);
    m_display_list.visitBackward(finder);

    if ( finder.hitFound() )
    {
        return true;
    } 
    else
    {
        return _drawable_inst->pointInShape(x, y); 
    }
}

character*
MovieClip::get_topmost_mouse_entity(boost::int32_t x, boost::int32_t y)
{
    //GNASH_REPORT_FUNCTION;

    if (get_visible() == false)
    {
        return NULL;
    }

    // point is in parent's space, we need to convert it in world space
    point    wp(x, y);
    character* parent = get_parent();
    if ( parent ) 
    {
        // WARNING: if we have NO parent, our parent is the Stage (movie_root)
        //          so, in case we'll add a "stage" matrix, we'll need to take
        //          it into account here.
        // TODO: actually, why are we insisting in using parent's
        //          coordinates for this method at all ?
        parent->getWorldMatrix().transform(wp);
    }

    if ( can_handle_mouse_event() )
    {
        if ( pointInVisibleShape(wp.x, wp.y) ) return this;
        else return NULL;
    }

    SWFMatrix    m = getMatrix();
    point    pp(x, y);
    m.invert().transform(pp);

    MouseEntityFinder finder(wp, pp);
    m_display_list.visitAll(finder);
    character* ch = finder.getEntity();

    // It doesn't make any sense to query _drawable_inst, as it's
    // a generic character and not a referencable character.

    return ch; // might be NULL
}

/// Find the first visible character whose shape contain the point
/// and is not the character being dragged or any of its childs
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
    character* _dragging;
    mutable const character* _dropch;

    typedef std::vector<const character*> Candidates;
    Candidates _candidates;

    mutable bool _checked;

public:

    DropTargetFinder(boost::int32_t x, boost::int32_t y, character* dragging)
        :
        _highestHiddenDepth(std::numeric_limits<int>::min()),
        _x(x),
        _y(y),
        _dragging(dragging),
        _dropch(0),
        _candidates(),
        _checked(false)
    {}

    void operator() (const character* ch)
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
            if ( ! ch->get_visible() )
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
            const character* ch = *i;
            const character* dropChar = ch->findDropTarget(_x, _y, _dragging);
            if ( dropChar )
            {
                _dropch = dropChar;
                break;
            }
        }
        _checked = true;
    }

    const character* getDropChar() const
    {
        checkCandidates();
        return _dropch;
    }
};

const character*
MovieClip::findDropTarget(boost::int32_t x, boost::int32_t y,
        character* dragging) const
{
    if ( this == dragging ) return 0; // not here...

    if ( ! get_visible() ) return 0; // isn't me !

    DropTargetFinder finder(x, y, dragging);
    m_display_list.visitAll(finder);

    // does it hit any child ?
    const character* ch = finder.getDropChar();
    if ( ch )
    {
        // TODO: find closest actionscript referenceable container
        //             (possibly itself)
        return ch;
    }

    // does it hit us ?
    if ( _drawable_inst->pointInVisibleShape(x, y) )
    {
        return this;
    }

    return NULL;
}

bool
MovieClip::can_handle_mouse_event() const
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

    for (size_t i = 0; i < size; i++)
    {
        const event_id &event = EH[i];

        // Check event handlers
        if ( get_event_handler(event.id()).get() )
        {
            return true;
        }

        // Check user-defined event handlers
        if ( getUserDefinedEventHandler(event.get_function_key()) )
        {
            return true;
        }
    }

    return false;
}
        
void MovieClip::restart()
{
// see Whack-a-doc.swf, we tried to restart an unloaded character.
// It shouldn't happen anyway.
// TODO: drop this function.

    // Stop any streaming sound associated with us
    stopStreamSound();

    if( ! isUnloaded() )
    {
        restoreDisplayList(0); 
    }

    m_play_state = PLAY;
}

character*
MovieClip::get_character(int /* character_id */)
{
    //return m_def->get_character_def(character_id);
    // @@ TODO -- look through our dlist for a match
    log_unimpl(_("%s doesn't even check for a char"),
        __PRETTY_FUNCTION__);
    return NULL;
}


void
MovieClip::stop_drag()
{
    //assert(m_parent == NULL); // why should we care ?
    _vm.getRoot().stop_drag();
}

float
MovieClip::get_background_alpha() const
{
    // @@ this doesn't seem right...
    return _vm.getRoot().get_background_alpha();
}

void
MovieClip::set_background_color(const rgba& color)
{
    _vm.getRoot().set_background_color(color);
}

static bool isTextFieldUnloaded(boost::intrusive_ptr<TextField>& p)
{
    return p->isUnloaded();
}

/*private*/
void
MovieClip::cleanup_textfield_variables()
{
    // nothing to do
    if ( ! _text_variables.get() ) return;

    TextFieldMap& m = *_text_variables;

    for (TextFieldMap::iterator i=m.begin(), ie=m.end(); i!=ie; ++i)
    {
        TextFieldPtrVect& v=i->second;
        TextFieldPtrVect::iterator lastValid = 
            std::remove_if(v.begin(), v.end(),
                    boost::bind(isTextFieldUnloaded, _1));
        v.erase(lastValid, v.end());
        // TODO: remove the map element if vector is empty
        //if ( v.empty() )
        //{
        //}
    }
}


/* public */
void
MovieClip::set_textfield_variable(const std::string& name, TextField* ch)
{
    assert(ch);

    // lazy allocation
    if ( ! _text_variables.get() )
    {
        _text_variables.reset(new TextFieldMap);
    }
    
    (*_text_variables)[name].push_back(ch);
}

/* private */
MovieClip::TextFieldPtrVect*
MovieClip::get_textfield_variable(const std::string& name)
{
    // nothing allocated yet...
    if ( ! _text_variables.get() ) return NULL;

    // TODO: should variable name be considered case-insensitive ?
    TextFieldMap::iterator it = _text_variables->find(name);
    if ( it == _text_variables->end() )
    {
        return 0;
    }
    else
    {
        return &(it->second);
    }
} 


void 
MovieClip::add_invalidated_bounds(InvalidatedRanges& ranges, 
    bool force)
{

    // nothing to do if this movieclip is not visible
    if (!m_visible || get_cxform().is_invisible() )
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
    
    
    m_display_list.add_invalidated_bounds(ranges, force||m_invalidated);

    _drawable_inst->add_invalidated_bounds(ranges, force||m_invalidated);

}


/// register characters as key listeners if they have clip key events defined.
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
    assert(!isUnloaded());

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
    if ( get_parent() == 0 )
    {
#ifdef GNASH_DEBUG
        log_debug(_("Executing tags of frame0 in movieclip %s"), getTarget());
#endif
        execute_frame_tags(0, m_display_list, TAG_DLIST|TAG_ACTION);

        if ( _vm.getSWFVersion() > 5 )
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
        execute_frame_tags(0, m_display_list, TAG_DLIST|TAG_ACTION);
    }

    // We execute events immediately when the stage-placed character 
    // is dynamic, This is becase we assume that this means that 
    // the character is placed during processing of actions (opposed 
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
        log_debug(_("Queuing INITIALIZE event for movieclip %s"), getTarget());
#endif
        queueEvent(event_id::INITIALIZE, movie_root::apINIT);

#ifdef GNASH_DEBUG
        log_debug(_("Queuing CONSTRUCT event for movieclip %s"), getTarget());
#endif
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
#ifdef GNASH_DEBUG
        log_debug(_("Sprite %s is dynamic, sending "
                "INITIALIZE and CONSTRUCT events immediately"), getTarget());
#endif

        on_event(event_id::INITIALIZE);
    }


}

/*private*/
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

        sprite_definition* def = dynamic_cast<sprite_definition*>(m_def.get());

        // We won't "construct" top-level movies
        if ( ! def ) break;

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

                set_member(NSV::PROP_uuCONSTRUCTORuu, ctor);
                if ( swfversion == 6 )
                {
                    set_member(NSV::PROP_CONSTRUCTOR, ctor);
                }

                // Provide a 'super' reference..
                // Super is computed from the object we're constructing,
                // It will work as long as we did set its __proto__ 
                // and __constructor__ properties already.
                as_object* super = get_super();

                as_environment& env = get_environment();
                fn_call call(this, &env);
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

    bool childHaveUnloadHandler = m_display_list.unload();

    // We won't be displayed again, so worth releasing
    // some memory. The drawable might take a lot of memory
    // on itself.
    _drawable->clear();
    // TODO: drop the _drawable_inst too ?
    //             it would require _drawable_inst to possibly be NULL,
    //             which wouldn't be bad at all actually...

    bool selfHaveUnloadHandler = character::unload();

    bool shouldKeepAlive = (selfHaveUnloadHandler || childHaveUnloadHandler);

    return shouldKeepAlive;
}

bool
MovieClip::loadMovie(const URL& url, const std::string* postdata)
{
    // Get a pointer to our own parent 
    character* parent = get_parent();
    if (parent)
    {
        if (postdata)
        {
            log_debug(_("Posting data '%s' to url '%s'"), postdata, url.str());
        }
        
        const movie_root& mr = _vm.getRoot();

        boost::intrusive_ptr<movie_definition> md (
                create_library_movie(url, mr.runInfo(), NULL, true, postdata));

        if (!md)
        {
            log_error(_("can't create movie_definition for %s"),
                url.str());
            return false;
        }

        boost::intrusive_ptr<movie_instance> extern_movie;
        extern_movie = md->create_movie_instance(parent);
        if (extern_movie == NULL)
        {
            log_error(_("can't create extern movie_instance "
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
        unsigned int level = get_depth()-character::staticDepthOffset;
        
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
        if (sendVarsMethod == METHOD_POST)
        {
            // use POST method
            _loadVariableRequests.push_back(
                    new LoadVariablesThread(url, postdata));
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
            _loadVariableRequests.push_back(new LoadVariablesThread(url));
        }
        _loadVariableRequests.back()->process();
    }
    catch (NetworkException& ex)
    {
        log_error(_("Could not load variables from %s"), url.str());
    }

}

/*private*/
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
        if ( request.completed() )
        {
            processCompletedLoadVariableRequest(request);
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
        // _root.swapDepth([0..1048575])
        //log_error(_("Can't remove movieclip %s as it has no parent"),
        //getTarget());
    }

}

rect
MovieClip::getBounds() const
{
    rect bounds;
    BoundsFinder f(bounds);
    const_cast<DisplayList&>(m_display_list).visitAll(f);
    rect drawableBounds = _drawable->get_bound();
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

    void operator() (character* ch)
    {
        // don't enumerate unloaded characters
        if ( ch->isUnloaded() ) return;

        _env.push(ch->get_name());
    }
};

void
MovieClip::enumerateNonProperties(as_environment& env) const
{
    EnumerateVisitor visitor(env);
    m_display_list.visitAll(visitor);
}

void
MovieClip::cleanupDisplayList()
{
    //log_debug("%s.cleanDisplayList() called, current dlist is %p", 
    //getTarget(), (void*)&m_display_list);
    m_display_list.removeUnloaded();

    cleanup_textfield_variables();
}

#ifdef GNASH_USE_GC
struct ReachableMarker {
    void operator() (character *ch)
    {
        ch->setReachable();
    }
};
void
MovieClip::markReachableResources() const
{
    ReachableMarker marker;

    m_display_list.visitAll(marker);

    _drawable->setReachable();

    _drawable_inst->setReachable();

    m_as_environment.markReachableResources();

    // Mark our own definition
    if ( m_def.get() ) m_def->setReachable();

    // Mark textfields in the TextFieldMap
    if ( _text_variables.get() )
    {
        for(TextFieldMap::const_iterator i=_text_variables->begin(),
                    e=_text_variables->end();
                i!=e; ++i)
        {
            const TextFieldPtrVect& tfs=i->second;
            for (TextFieldPtrVect::const_iterator j=tfs.begin(), je=tfs.end(); j!=je; ++j)
            {
                if ( (*j)->isUnloaded() )
                {
                    // NOTE: cleanup_display_list should have cleared 
                    // these up on ::cleanupDisplayList.
                    // I guess if we get more might be due to ::destroy 
                    // calls happening after our own ::cleanupDisplayList
                    // call. Should be ok to postpone cleanup on next 
                    // ::advance, or we should cleanup here (locally) 
                    // although we're a 'const' method...
                    // Yet another approach would be for TextField::unload
                    // to unregister self from our map, but TextField 
                    // doesn't really store a pointer to the movieclip
                    // it's registered against.
                    //
                    //log_debug("Unloaded TextField in registered textfield "
                    //"variables container on ::markReachableResources");
                }
                (*j)->setReachable();
            }
        }
    }

    // Mark our relative root
    assert(m_root != NULL);
    m_root->setReachable();

    markCharacterReachable();

}
#endif // GNASH_USE_GC

void
MovieClip::destroy()
{
    stopStreamSound();

    m_display_list.destroy();

    /// We don't need these anymore
    clearProperties();

    character::destroy();
}

cxform
MovieClip::get_world_cxform() const
{
    cxform cf = character::get_world_cxform();
    cf.concatenate(_userCxform); 
    return cf;
}

movie_instance*
MovieClip::get_root() const
{
    return m_root;
}

const MovieClip*
MovieClip::getAsRoot() const
{
    //log_debug("getAsRoot called for movieclip %s, with _lockroot "
    //"%d and version %d", getTarget(), getLockRoot(), getSWFVersion());

    // TODO1: as an optimization, if swf version < 7 
    //                we might as well just return m_root, 
    //                the whole chain from this movieclip to it's
    //                m_root should have the same version...
    //
    // TODO2: implement this with iteration rather
    //                then recursion.
    //                

    character* parent = get_parent();
    if ( ! parent ) return this; // no parent, we're the root

    // If we have a parent, we descend to it unless 
    // our _lockroot is true AND our or the VM's
    // SWF version is > 6
    //
    int topSWFVersion = getVM().getRoot().get_movie_definition()->get_version();
    //int topSWFVersion = getVM().getSWFVersion() > 6;

    if ( getSWFVersion() > 6 || topSWFVersion > 6 )
    {
        if ( getLockRoot() )
        {
            return this; // locked
        }
    }

    return parent->getAsRoot();
}

as_value
MovieClip::lockroot_getset(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

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
MovieClip::set_play_state(play_state s)
{
    if ( s == m_play_state ) return; // nothing to do
    if ( s == MovieClip::STOP ) stopStreamSound();
    m_play_state = s;
}

#ifdef USE_SWFTREE

class MovieInfoVisitor {

    character::InfoTree& _tr;
    character::InfoTree::iterator _it;

public:
    MovieInfoVisitor(character::InfoTree& tr, character::InfoTree::iterator it)
        :
        _tr(tr),
        _it(it)
    {}

    void operator() (character* ch)
    {
        // Should we still print these?
        //if ( ch->isUnloaded() ) return; 

        ch->getMovieInfo(_tr, _it);
    }
};

character::InfoTree::iterator 
MovieClip::getMovieInfo(InfoTree& tr, InfoTree::iterator it)
{
    InfoTree::iterator selfIt = character::getMovieInfo(tr, it);
    std::ostringstream os;
    os << m_display_list.size();
    InfoTree::iterator localIter = tr.append_child(selfIt,
            StringPair(_("Children"), os.str()));            
    //localIter = tr.append_child(localIter, StringPair("child1", "fake"));
    //localIter = tr.append_child(localIter, StringPair("child2", "fake"));

    MovieInfoVisitor v(tr, localIter);
    m_display_list.visitAll(v);

    return selfIt;

}

#endif // USE_SWFTREE

void
movieclip_class_init(as_object& global)
{
    // This is going to be the global MovieClip "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl=NULL;

    if ( cl == NULL )
    {
        cl=new builtin_function(&movieclip_ctor, getMovieClipInterface());
        VM::get().addStatic(cl.get());

        // replicate all interface to class, to be able to access
        // all methods as static functions
        //attachMovieClipInterface(*cl);
                 
    }

    // Register _global.MovieClip
    global.init_member("MovieClip", cl.get());
}


//---------------------
//  MovieClip interface
//---------------------

namespace {

/// Properties (and/or methods) *inherited* by MovieClip instances
void
attachMovieClipInterface(as_object& o)
{
        VM& vm = o.getVM();

        o.init_member("attachMovie", vm.getNative(900, 0)); 
        o.init_member("swapDepths", vm.getNative(900, 1));
        o.init_member("localToGlobal", vm.getNative(900, 2));
        o.init_member("globalToLocal", vm.getNative(900, 3));
        o.init_member("hitTest", vm.getNative(900, 4));
        o.init_member("getBounds", vm.getNative(900, 5));
        o.init_member("getBytesTotal", vm.getNative(900, 6));
        o.init_member("getBytesLoaded", vm.getNative(900, 7));
        o.init_member("play", vm.getNative(900, 12));
        o.init_member("stop", vm.getNative(900, 13));
        o.init_member("nextFrame", vm.getNative(900, 14));
        o.init_member("prevFrame", vm.getNative(900, 15));
        o.init_member("gotoAndPlay", vm.getNative(900, 16));
        o.init_member("gotoAndStop", vm.getNative(900, 17));
        o.init_member("duplicateMovieClip", vm.getNative(900, 18));
        o.init_member("removeMovieClip", vm.getNative(900, 19));
        o.init_member("startDrag", vm.getNative(900, 20));
        o.init_member("stopDrag", vm.getNative(900, 21));
        o.init_member("loadMovie", new builtin_function(movieclip_loadMovie));
        o.init_member("loadVariables", new builtin_function(
                    movieclip_loadVariables));
        o.init_member("unloadMovie", new builtin_function(
                    movieclip_unloadMovie));
        o.init_member("getURL", new builtin_function(movieclip_getURL));
        o.init_member("getSWFVersion", new builtin_function(
                    movieclip_getSWFVersion));
        o.init_member("meth", new builtin_function(movieclip_meth));
        o.init_member("enabled", true);
        o.init_member("useHandCursor", true);
        o.init_property("_lockroot", &MovieClip::lockroot_getset,
              &MovieClip::lockroot_getset);
        o.init_member("beginBitmapFill", new builtin_function(
                    movieclip_beginBitmapFill));
        o.init_member("getRect", new builtin_function(
                    movieclip_getRect));
        o.init_member("lineGradientStyle", new builtin_function(
                    movieclip_lineGradientStyle));
        o.init_member("attachBitmap", new builtin_function(
                    movieclip_attachBitmap));
        o.init_property("blendMode", &movieclip_blendMode,
                &movieclip_blendMode);
        o.init_property("cacheAsBitmap", &movieclip_cacheAsBitmap, 
                &movieclip_cacheAsBitmap);
        o.init_property("filters", &movieclip_filters, &movieclip_filters);
        o.init_property("forceSmoothing", &movieclip_forceSmoothing,
                &movieclip_forceSmoothing);
        o.init_property("opaqueBackground", &movieclip_opaqueBackground,
                &movieclip_opaqueBackground);
        o.init_property("scale9Grid", &movieclip_scale9Grid,
                movieclip_scale9Grid);
        o.init_property("scrollRect", &movieclip_scrollRect,
			&movieclip_scrollRect);
        o.init_property("tabIndex", &movieclip_tabIndex, &movieclip_tabIndex);
        o.init_property("transform", &movieclip_transform, 
                &movieclip_transform);

        const int swf6Flags = as_prop_flags::dontDelete |
                    as_prop_flags::dontEnum |
                    as_prop_flags::onlySWF6Up;

        o.init_member("attachAudio", vm.getNative(900, 8), swf6Flags);
        o.init_member("attachVideo", vm.getNative(900, 9), swf6Flags);
        o.init_member("getDepth", vm.getNative(900, 10), swf6Flags);
        o.init_member("setMask", vm.getNative(900, 11), swf6Flags);
        o.init_member("createEmptyMovieClip", vm.getNative(901, 0), swf6Flags);
        o.init_member("beginFill", vm.getNative(901, 1), swf6Flags);
        o.init_member("beginGradientFill", vm.getNative(901, 2), swf6Flags);
        o.init_member("moveTo", vm.getNative(901, 3), swf6Flags);
        o.init_member("lineTo", vm.getNative(901, 4), swf6Flags);
        o.init_member("curveTo", vm.getNative(901, 5), swf6Flags);
        o.init_member("lineStyle", vm.getNative(901, 6), swf6Flags);
        o.init_member("endFill", vm.getNative(901, 7), swf6Flags);
        o.init_member("clear", vm.getNative(901, 8), swf6Flags);
        o.init_member("createTextField", vm.getNative(104, 200), swf6Flags);
        o.init_member("getTextSnapshot", 
                new builtin_function(movieclip_getTextSnapshot), swf6Flags);

        const int swf7Flags = as_prop_flags::dontDelete |
                    as_prop_flags::dontEnum |
                    as_prop_flags::onlySWF7Up;

        o.init_member("getNextHighestDepth", new builtin_function(
                    movieclip_getNextHighestDepth), swf7Flags);
        o.init_member("getInstanceAtDepth", new builtin_function(
                    movieclip_getInstanceAtDepth), swf7Flags);

}

void
registerNatives(VM& vm)
{
    // Natives are always here    (at least in swf5 I guess)
    vm.registerNative(movieclip_attachMovie, 900, 0); 
    // TODO: generalize to character::swapDepths_method ?
    vm.registerNative(movieclip_swapDepths, 900, 1); 
    vm.registerNative(movieclip_localToGlobal, 900, 2);
    vm.registerNative(movieclip_globalToLocal, 900, 3);
    vm.registerNative(movieclip_hitTest, 900, 4);
    vm.registerNative(movieclip_getBounds, 900, 5);
    vm.registerNative(movieclip_getBytesTotal, 900, 6);
    vm.registerNative(movieclip_getBytesLoaded, 900, 7);
    vm.registerNative(movieclip_attachAudio, 900, 8);
    vm.registerNative(movieclip_attachVideo, 900, 9);
    // TODO: generalize to character::getDepth_method ?
    vm.registerNative(movieclip_getDepth, 900, 10);
    vm.registerNative(movieclip_setMask, 900, 11); 
    vm.registerNative(movieclip_play, 900, 12); 
    vm.registerNative(movieclip_stop, 900, 13);
    vm.registerNative(movieclip_nextFrame, 900, 14);
    vm.registerNative(movieclip_prevFrame, 900, 15);
    vm.registerNative(movieclip_gotoAndPlay, 900, 16);
    vm.registerNative(movieclip_gotoAndStop, 900, 17);
    vm.registerNative(movieclip_duplicateMovieClip, 900, 18);
    vm.registerNative(movieclip_removeMovieClip, 900, 19);
    vm.registerNative(movieclip_startDrag, 900, 20);
    vm.registerNative(movieclip_stopDrag, 900, 21);
    vm.registerNative(movieclip_createEmptyMovieClip, 901, 0);
    vm.registerNative(movieclip_beginFill, 901, 1);
    vm.registerNative(movieclip_beginGradientFill, 901, 2);
    vm.registerNative(movieclip_moveTo, 901, 3);
    vm.registerNative(movieclip_lineTo, 901, 4);
    vm.registerNative(movieclip_curveTo, 901, 5);
    vm.registerNative(movieclip_lineStyle, 901, 6);
    vm.registerNative(movieclip_endFill, 901, 7);
    vm.registerNative(movieclip_clear, 901, 8);

    vm.registerNative(movieclip_createTextField, 104, 200);

}

as_value
movieclip_play(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    movieclip->set_play_state(MovieClip::PLAY);
    return as_value();
}

as_value
movieclip_stop(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    movieclip->set_play_state(MovieClip::STOP);

    return as_value();
}


//removeMovieClip() : Void
as_value
movieclip_removeMovieClip(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);
    movieclip->removeMovieClip();
    return as_value();
}


as_value
movieclip_blendMode(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    log_unimpl(_("MovieClip.blendMode()"));
    return as_value();
}


as_value
movieclip_cacheAsBitmap(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    log_unimpl(_("MovieClip.cacheAsBitmap()"));
    return as_value();
}


as_value
movieclip_filters(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    LOG_ONCE(log_unimpl(_("MovieClip.filters()")));
    return as_value();
}


as_value
movieclip_forceSmoothing(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    log_unimpl(_("MovieClip.forceSmoothing()"));
    return as_value();
}


as_value
movieclip_opaqueBackground(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    log_unimpl(_("MovieClip.opaqueBackground()"));
    return as_value();
}

    
as_value
movieclip_scale9Grid(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    log_unimpl(_("MovieClip.scale9Grid()"));
    return as_value();
}


as_value
movieclip_scrollRect(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    log_unimpl(_("MovieClip.scrollRect()"));
    return as_value();
}


as_value
movieclip_tabIndex(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);
    log_unimpl(_("MovieClip.tabIndex()"));
    return as_value();
}


// attachMovie(idName:String, newName:String,
//                         depth:Number [, initObject:Object]) : MovieClip
as_value
movieclip_attachMovie(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 3 || fn.nargs > 4)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("attachMovie called with wrong number of arguments"
                " expected 3 to 4, got (%d) - returning undefined"), fn.nargs);
        );
        return as_value();
    }

    // Get exported resource 
    const std::string& id_name = fn.arg(0).to_string();

    boost::intrusive_ptr<ExportableResource> exported = 
        movieclip->get_movie_definition()->get_exported_resource(id_name);

    if (!exported)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("attachMovie: '%s': no such exported resource - "
            "returning undefined"), id_name);
        );
        return as_value(); 
    }
    
    character_def* exported_movie =
        dynamic_cast<character_def*>(exported.get());

    if (!exported_movie)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("attachMovie: exported resource '%s' "
            "is not a character definition (%s) -- "
            "returning undefined"), id_name,
            typeid(*(exported.get())).name());
        );
        return as_value();
    }

    const std::string& newname = fn.arg(1).to_string();

    // Movies should be attachable from -16384 to 2130690045, according to
    // kirupa (http://www.kirupa.com/developer/actionscript/depths2.htm)
    // Tests in misc-ming.all/DepthLimitsTest.c show that 2130690044 is the
    // maximum valid depth.
    const double depth = fn.arg(2).to_number();
    
    // This also checks for overflow, as both numbers are expressible as
    // boost::int32_t.
    if (depth < character::lowerAccessibleBound ||
            depth > character::upperAccessibleBound)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.attachMovie: invalid depth %d "
                    "passed; not attaching"), depth);
        );
        return as_value();
    }
    
    boost::int32_t depthValue = static_cast<boost::int32_t>(depth);

    boost::intrusive_ptr<character> newch =
        exported_movie->create_character_instance(movieclip.get(), 0);

#ifndef GNASH_USE_GC
    assert(newch->get_ref_count() > 0);
#endif // ndef GNASH_USE_GC

    newch->set_name(newname);
    newch->setDynamic();

    boost::intrusive_ptr<as_object> initObj;

    if (fn.nargs > 3 ) {
        initObj = fn.arg(3).to_object();
        if (!initObj) {
            // This is actually a valid thing to do,
            // the documented behaviour is to just NOT
            // initialize the properties in this
            // case.
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Fourth argument of attachMovie doesn't cast to "
                    "an object (%s), we'll act as if it wasn't given"),
                    fn.arg(3));
            );
        }
    }

    // place_character() will set depth on newch
    if (!movieclip->attachCharacter(*newch, depthValue, initObj.get()))
    {
        log_error(_("Could not attach character at depth %d"), depthValue);
        return as_value();
    }

    return as_value(newch.get());
}


// attachAudio(id:Object) : Void
as_value
movieclip_attachAudio(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    if (!fn.nargs)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("MovieClip.attachAudio(): %s", _("missing arguments"));
        );
        return as_value();
    }

    as_object* obj = fn.arg(0).to_object().get();
    if ( ! obj )
    { 
        std::stringstream ss; fn.dump_args(ss);
        // TODO: find out what to do here
        log_error("MovieClip.attachAudio(%s): first arg doesn't cast to "
                "an object", ss.str());
        return as_value();
    }

    NetStream_as* ns = dynamic_cast<NetStream_as*>(obj);
    if ( ! ns )
    { 
        std::stringstream ss; fn.dump_args(ss);
        // TODO: find out what to do here
        log_error("MovieClip.attachAudio(%s): first arg doesn't cast to a "
                "NetStream", ss.str());
        return as_value();
    }

    ns->setAudioController(movieclip.get());

    LOG_ONCE( log_unimpl("MovieClip.attachAudio() - TESTING") );
    return as_value();
}


// MovieClip.attachVideo
as_value
movieclip_attachVideo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);

    LOG_ONCE( log_unimpl("MovieClip.attachVideo()") );
    return as_value();
}


//createEmptyMovieClip(name:String, depth:Number) : MovieClip
as_value
movieclip_createEmptyMovieClip(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs != 2)
    {
        if (fn.nargs < 2)
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("createEmptyMovieClip needs "
                    "2 args, but %d given,"
                    " returning undefined"),
                    fn.nargs);
            );
            return as_value();
        }
        else
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("createEmptyMovieClip takes "
                    "2 args, but %d given, discarding"
                    " the excess"),
                    fn.nargs);
            )
        }
    }

    // Unlike other MovieClip methods, the depth argument of an empty movie clip
    // can be any number. All numbers are converted to an int32_t, and are valid
    // depths even when outside the usual bounds.
    character* ch = movieclip->add_empty_movieclip(fn.arg(0).to_string().c_str(),
            fn.arg(1).to_int());
    return as_value(ch);
}

as_value
movieclip_getDepth(const fn_call& fn)
{
    // TODO: make this a character::getDepth_method function...
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    const int n = movieclip->get_depth();

    return as_value(n);
}

//swapDepths(target:Object|target:Number)
//
// Returns void.
as_value
movieclip_swapDepths(const fn_call& fn)
{

    boost::intrusive_ptr<MovieClip> movieclip =
        ensureType<MovieClip>(fn.this_ptr);

    const int this_depth = movieclip->get_depth();

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s.swapDepths() needs one arg"), movieclip->getTarget());
        );
        return as_value();
    }

    // Lower bound of source depth below which swapDepth has no effect
    // (below Timeline/static zone)
    if ( this_depth < character::lowerAccessibleBound )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss;
            fn.dump_args(ss);
            log_aserror(_("%s.swapDepths(%s): won't swap a clip below "
                    "depth %d (%d)"),
            movieclip->getTarget(), ss.str(), character::lowerAccessibleBound,
                this_depth);
        );
        return as_value();
    }

    typedef boost::intrusive_ptr<character> CharPtr;
    typedef boost::intrusive_ptr<MovieClip> SpritePtr;

    SpritePtr this_parent = dynamic_cast<MovieClip*>(
            movieclip->get_parent());

    //CharPtr target = NULL;
    int target_depth = 0;

    // movieclip.swapDepth(movieclip)
    if ( SpritePtr target_movieclip = fn.arg(0).to_sprite() )
    {
        if ( movieclip == target_movieclip )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s.swapDepths(%s): invalid call, swapping to self?"),
                movieclip->getTarget(), target_movieclip->getTarget());
            );
            return as_value();
        }

        SpritePtr target_parent =
            dynamic_cast<MovieClip*>(movieclip->get_parent());
        if ( this_parent != target_parent )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s.swapDepths(%s): invalid call, the two "
                    "characters don't have the same parent"),
                movieclip->getTarget(), target_movieclip->getTarget());
            );
            return as_value();
        }

        target_depth = target_movieclip->get_depth();

        // Check we're not swapping the our own depth so
        // to avoid unecessary bounds invalidation and immunizing
        // the instance from subsequent PlaceObjec tags attempting
        // to transform it.
        if ( movieclip->get_depth() == target_depth )
        {
            IF_VERBOSE_ASCODING_ERRORS(
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("%s.swapDepths(%s): ignored, source and "
                    "target characters have the same depth %d"),
                    movieclip->getTarget(), ss.str(), target_depth);
            );
            return as_value();
        }
    }

    // movieclip.swapDepth(depth)
    else
    {
        double td = fn.arg(0).to_number();
        if ( isNaN(td) )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("%s.swapDepths(%s): first argument invalid "
                "(neither a movieclip nor a number)"),
                movieclip->getTarget(), ss.str());
            );
            return as_value();
        }

        target_depth = int(td);

        // Check we're not swapping the our own depth so
        // to avoid unecessary bounds invalidation and immunizing
        // the instance from subsequent PlaceObjec tags attempting
        // to transform it.
        if ( movieclip->get_depth() == target_depth )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror(_("%s.swapDepths(%s): ignored, character already "
                    "at depth %d"),
                movieclip->getTarget(), ss.str(), target_depth);
            );
            return as_value();
        }


        // TODO : check other kind of validities ?


    }

    if ( this_parent )
    {
        this_parent->swapDepths(movieclip.get(), target_depth);
    }
    else
    {
        movie_root& root = movieclip->getVM().getRoot();
        root.swapLevels(movieclip, target_depth);
        return as_value();
    }

    return as_value();

}

// TODO: wrap the functionality in a MovieClip method
//             and invoke it from here, this should only be a wrapper
//
//duplicateMovieClip(name:String, depth:Number, [initObject:Object]) : MovieClip
as_value
movieclip_duplicateMovieClip(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);
    
    if (fn.nargs < 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.duplicateMovieClip() needs 2 or 3 args"));
                );
        return as_value();
    }

    const std::string& newname = fn.arg(0).to_string();

    // Depth as in attachMovie
    const double depth = fn.arg(1).to_number();
    
    // This also checks for overflow, as both numbers are expressible as
    // boost::int32_t.
    if (depth < character::lowerAccessibleBound ||
            depth > character::upperAccessibleBound)
    {
        IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("MovieClip.duplicateMovieClip: "
                        "invalid depth %d passed; not duplicating"), depth);
        );    
        return as_value();
    }
    
    boost::int32_t depthValue = static_cast<boost::int32_t>(depth);

    boost::intrusive_ptr<MovieClip> ch;

    // Copy members from initObject
    if (fn.nargs == 3)
    {
        boost::intrusive_ptr<as_object> initObject = fn.arg(2).to_object();
        ch = movieclip->duplicateMovieClip(newname, depthValue,
                initObject.get());
    }
    else
    {
        ch = movieclip->duplicateMovieClip(newname, depthValue);
    }

    return as_value(ch.get());
}

as_value
movieclip_gotoAndPlay(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_play needs one arg"));
        );
        return as_value();
    }

    size_t frame_number;
    if ( ! movieclip->get_frame_number(fn.arg(0), frame_number) )
    {
        // No dice.
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_play('%s') -- invalid frame"),
                    fn.arg(0));
        );
        return as_value();
    }

    // Convert to 0-based
    movieclip->goto_frame(frame_number);
    movieclip->set_play_state(MovieClip::PLAY);
    return as_value();
}

as_value movieclip_gotoAndStop(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_stop needs one arg"));
        );
        return as_value();
    }

    size_t frame_number;
    if ( ! movieclip->get_frame_number(fn.arg(0), frame_number) )
    {
        // No dice.
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("movieclip_goto_and_stop('%s') -- invalid frame"),
                    fn.arg(0));
        );
        return as_value();
    }

    // Convert to 0-based
    movieclip->goto_frame(frame_number);
    movieclip->set_play_state(MovieClip::STOP);
    return as_value();
}

as_value movieclip_nextFrame(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    const size_t frame_count = movieclip->get_frame_count();
    const size_t current_frame = movieclip->get_current_frame();
    if (current_frame < frame_count)
    {
        movieclip->goto_frame(current_frame + 1);
    }
    movieclip->set_play_state(MovieClip::STOP);
    return as_value();
}

as_value
movieclip_prevFrame(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    const size_t current_frame = movieclip->get_current_frame();
    if (current_frame > 0)
    {
        movieclip->goto_frame(current_frame - 1);
    }
    movieclip->set_play_state(MovieClip::STOP);
    return as_value();
}

as_value
movieclip_getBytesLoaded(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(movieclip->get_bytes_loaded());
}

as_value
movieclip_getBytesTotal(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    // @@ horrible uh ?
    return as_value(movieclip->get_bytes_total());
}

// MovieClip.loadMovie(url:String [,variables:String]).
//
// Returns 1 for "get", 2 for "post", and otherwise 0. Case-insensitive.
// This *always* calls MovieClip.meth.
as_value
movieclip_loadMovie(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    as_value val;
    if (fn.nargs > 1)
    {
        val = movieclip->callMethod(NSV::PROP_METH, fn.arg(1));
    }
    else val = movieclip->callMethod(NSV::PROP_METH);

    if (fn.nargs < 1) // url
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.loadMovie() "
            "expected 1 or 2 args, got %d - returning undefined"),
            fn.nargs);
        );
        return as_value();
    }

    const std::string& urlstr = fn.arg(0).to_string();
    if (urlstr.empty())
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("First argument of MovieClip.loadMovie(%s) "
            "evaluates to an empty string - "
            "returning undefined"),
            ss.str());
        );
        return as_value();
    }

    movie_root& mr = movieclip->getVM().getRoot();
    std::string target = movieclip->getTarget();

    // TODO: if GET/POST should send variables of *this* movie,
    // no matter if the target will be replaced by another movie !!
    const MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(val.to_int());

    std::string data;

    // This is just an optimization if we aren't going
    // to send the data anyway. It might be wrong, though.
    if (method != MovieClip::METHOD_NONE)
    {
        movieclip->getURLEncodedVars(data);
    }
 
    mr.loadMovie(urlstr, target, data, method);

    return as_value();
}

// my_mc.loadVariables(url:String [, variables:String]) : Void
as_value
movieclip_loadVariables(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    // This always calls MovieClip.meth, even when there are no
    // arguments.
    as_value val;
    if (fn.nargs > 1)
    {
        val = movieclip->callMethod(NSV::PROP_METH, fn.arg(1));
    }
    else val = movieclip->callMethod(NSV::PROP_METH);

    if (fn.nargs < 1) // url
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.loadVariables() "
            "expected 1 or 2 args, got %d - returning undefined"),
            fn.nargs);
        );
        return as_value();
    }

    const std::string& urlstr = fn.arg(0).to_string();
    if (urlstr.empty())
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("First argument passed to MovieClip.loadVariables(%s) "
            "evaluates to an empty string - "
            "returning undefined"),
            ss.str());
        );
        return as_value();
    }

    const MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(val.to_int());

    movieclip->loadVariables(urlstr, method);
    log_debug("MovieClip.loadVariables(%s) - TESTING ", urlstr);

    return as_value();
}

// my_mc.unloadMovie() : Void
as_value
movieclip_unloadMovie(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    movieclip->unloadMovie();

    return as_value();
}

as_value
movieclip_hitTest(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    switch (fn.nargs)
    {
        case 1: // target
        {
            const as_value& tgt_val = fn.arg(0);
            character* target = fn.env().find_target(tgt_val.to_string());
            if ( ! target )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Can't find hitTest target %s"),
                    tgt_val);
                );
                return as_value();
            }

            rect thisbounds = movieclip->getBounds();
            SWFMatrix thismat = movieclip->getWorldMatrix();
            thismat.transform(thisbounds);

            rect tgtbounds = target->getBounds();
            SWFMatrix tgtmat = target->getWorldMatrix();
            tgtmat.transform(tgtbounds);

            return thisbounds.getRange().intersects(tgtbounds.getRange());

            break;
        }

        case 2: // x, y
        {
            boost::int32_t x = PIXELS_TO_TWIPS(fn.arg(0).to_number());
            boost::int32_t y = PIXELS_TO_TWIPS(fn.arg(1).to_number());

            return movieclip->pointInBounds(x, y);
        }

        case 3: // x, y, shapeFlag
        {
             boost::int32_t x = PIXELS_TO_TWIPS(fn.arg(0).to_number());
             boost::int32_t y = PIXELS_TO_TWIPS(fn.arg(1).to_number());
             bool shapeFlag = fn.arg(2).to_bool();

             if ( ! shapeFlag ) return movieclip->pointInBounds(x, y);
             else return movieclip->pointInHitableShape(x, y);
        }

        default:
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("hitTest() called with %u args"),
                    fn.nargs);
            );
            break;
        }
    }

    return as_value();

}

as_value
movieclip_createTextField(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if (fn.nargs < 6) // name, depth, x, y, width, height
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField called with %d args, "
            "expected 6 - returning undefined"), fn.nargs);
        );
        return as_value();
    }

    std::string txt_name = fn.arg(0).to_string();

    int txt_depth = fn.arg(1).to_int();

    int txt_x = fn.arg(2).to_int();

    int txt_y = fn.arg(3).to_int();

    int txt_width = fn.arg(4).to_int();
    if ( txt_width < 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField: negative width (%d)"
            " - reverting sign"), txt_width);
        );
        txt_width = -txt_width;
    }

    int txt_height = fn.arg(5).to_int();
    if ( txt_height < 0 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("createTextField: negative height (%d)"
            " - reverting sign"), txt_height);
        );
        txt_height = -txt_height;
    }

    boost::intrusive_ptr<character> txt = movieclip->add_textfield(txt_name,
            txt_depth, txt_x, txt_y, txt_width, txt_height);

    // createTextField returns void, it seems
    if ( movieclip->getVM().getSWFVersion() > 7 ) return as_value(txt.get());
    else return as_value(); 
}

//getNextHighestDepth() : Number
as_value
movieclip_getNextHighestDepth(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    int nextdepth = movieclip->getNextHighestDepth();
    return as_value(static_cast<double>(nextdepth));
}

//getInstanceAtDepth(depth:Number) : MovieClip
as_value
movieclip_getInstanceAtDepth(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("MovieClip.getInstanceAtDepth(): missing depth argument");
        );
        return as_value();
    }

    int depth = fn.arg(0).to_int();
    boost::intrusive_ptr<character> ch = movieclip->get_character_at_depth(depth);
    if ( ! ch ) return as_value(); // we want 'undefined', not 'null'
    return as_value(ch.get());
}

/// MovieClip.getURL(url:String[, window:String[, method:String]])
//
/// TODO: test this properly.
/// Returns void.
as_value
movieclip_getURL(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
            ensureType<MovieClip>(fn.this_ptr);

    std::string urlstr;
    std::string target;

    as_value val;
    if (fn.nargs > 2)
    {
        val = movieclip->callMethod(NSV::PROP_METH, fn.arg(2));
    }
    else val = movieclip->callMethod(NSV::PROP_METH);

    switch (fn.nargs)
    {
        case 0:
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("No arguments passed to MovieClip.getURL()"));
            );
            return as_value();
        }
        default:
        {
            IF_VERBOSE_ASCODING_ERRORS(
                std::ostringstream os;
                fn.dump_args(os);
                log_aserror(_("MovieClip.getURL(%s): extra arguments "
                    "dropped"), os.str());
            );
        }
        case 3:
            // This argument has already been handled.
        case 2:
             target = fn.arg(1).to_string();
        case 1:
             urlstr = fn.arg(0).to_string();
             break;
    }


    MovieClip::VariablesMethod method =
        static_cast<MovieClip::VariablesMethod>(val.to_int());

    std::string vars;

    if (method != MovieClip::METHOD_NONE)
    {
        // Get encoded vars.
        movieclip->getURLEncodedVars(vars);
    }

    movie_root& m = movieclip->getVM().getRoot();
    
    m.getURL(urlstr, target, vars, method);

    return as_value();
}

// getSWFVersion() : Number
as_value
movieclip_getSWFVersion(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    return as_value(movieclip->getSWFVersion());
}

// MovieClip.meth(<string>) : Number
//
// Parses case-insensitive "get" and "post" into 1 and 2, 0 anything else
// 
as_value
movieclip_meth(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    if (!fn.nargs) return as_value(MovieClip::METHOD_NONE); 

    const as_value& v = fn.arg(0);
    boost::intrusive_ptr<as_object> o = v.to_object();
    if ( ! o )
    {
        log_debug(_("meth(%s): first argument doesn't cast to object"), v);
        return as_value(MovieClip::METHOD_NONE);
    }

    as_value lc = o->callMethod(NSV::PROP_TO_LOWER_CASE);

    std::string s = lc.to_string();

    if (s == "get") return as_value(MovieClip::METHOD_GET);
    if (s == "post") return as_value(MovieClip::METHOD_POST);
    return as_value(MovieClip::METHOD_NONE);
}


// getTextSnapshot() : TextSnapshot
as_value
movieclip_getTextSnapshot(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    LOG_ONCE( log_unimpl("MovieClip.getTextSnapshot()") );
    return as_value();
}


// getBounds(targetCoordinateSpace:Object) : Object
as_value
movieclip_getBounds(const fn_call& fn)
{
    boost::intrusive_ptr<character> movieclip =
        ensureType<character>(fn.this_ptr);

    rect bounds = movieclip->getBounds();

    if ( fn.nargs > 0 )
    {
        character* target = fn.arg(0).to_character();
        if ( ! target )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.getBounds(%s): invalid call, first "
                    "arg must be a character"),
                fn.arg(0));
            );
            return as_value();
        }

        SWFMatrix tgtwmat = target->getWorldMatrix();
        SWFMatrix srcwmat = movieclip->getWorldMatrix();

        srcwmat.transform(bounds);
        tgtwmat.invert().transform(bounds);
    }

    // Magic numbers here... dunno why
    double xMin = 6710886.35;
    double yMin = 6710886.35;
    double xMax = 6710886.35;
    double yMax = 6710886.35;

    if ( !bounds.is_null() )
    {
        // Round to the twip
        xMin = TWIPS_TO_PIXELS(bounds.get_x_min());
        yMin = TWIPS_TO_PIXELS(bounds.get_y_min());
        xMax = TWIPS_TO_PIXELS(bounds.get_x_max());
        yMax = TWIPS_TO_PIXELS(bounds.get_y_max());
    }

    boost::intrusive_ptr<as_object> bounds_obj(new as_object());
    bounds_obj->init_member("xMin", as_value(xMin));
    bounds_obj->init_member("yMin", as_value(yMin));
    bounds_obj->init_member("xMax", as_value(xMax));
    bounds_obj->init_member("yMax", as_value(yMax));

    return as_value(bounds_obj.get());
}

as_value
movieclip_globalToLocal(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    as_value ret;

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal() takes one arg"));
        );
        return ret;
    }

    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
    if ( ! obj )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "first argument doesn't cast to an object"),
            fn.arg(0));
        );
        return ret;
    }

    as_value tmp;
    boost::int32_t    x = 0;
    boost::int32_t    y = 0;

    if ( ! obj->get_member(NSV::PROP_X, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "object parameter doesn't have an 'x' member"),
            fn.arg(0));
        );
        return ret;
    }
    x = PIXELS_TO_TWIPS(tmp.to_number());

    if ( ! obj->get_member(NSV::PROP_Y, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.globalToLocal(%s): "
                "object parameter doesn't have an 'y' member"),
            fn.arg(0));
        );
        return ret;
    }
    y = PIXELS_TO_TWIPS(tmp.to_number());

    point    pt(x, y);
    SWFMatrix world_mat = movieclip->getWorldMatrix();
    world_mat.invert().transform(pt);

    obj->set_member(NSV::PROP_X, TWIPS_TO_PIXELS(pt.x));
    obj->set_member(NSV::PROP_Y, TWIPS_TO_PIXELS(pt.y));

    return ret;
}

as_value
movieclip_localToGlobal(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    as_value ret;

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal() takes one arg"));
        );
        return ret;
    }

    boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
    if ( ! obj )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "first argument doesn't cast to an object"),
            fn.arg(0));
        );
        return ret;
    }

    as_value tmp;
    boost::int32_t    x = 0;
    boost::int32_t    y = 0;

    if ( ! obj->get_member(NSV::PROP_X, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "object parameter doesn't have an 'x' member"),
            fn.arg(0));
        );
        return ret;
    }
    x = PIXELS_TO_TWIPS(tmp.to_number());

    if ( ! obj->get_member(NSV::PROP_Y, &tmp) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("MovieClip.localToGlobal(%s): "
                "object parameter doesn't have an 'y' member"),
            fn.arg(0));
        );
        return ret;
    }
    y = PIXELS_TO_TWIPS(tmp.to_number());

    point    pt(x, y);
    SWFMatrix world_mat = movieclip->getWorldMatrix();
    world_mat.transform(pt);

    obj->set_member(NSV::PROP_X, TWIPS_TO_PIXELS(pt.x));
    obj->set_member(NSV::PROP_Y, TWIPS_TO_PIXELS(pt.y));
    return ret;

}

as_value
movieclip_setMask(const fn_call& fn)
{
    // swfdec/test/image/mask-textfield-6.swf shows that setMask should also
    // work against TextFields, we have no tests for other character types so
    // we generalize it for any character.
    boost::intrusive_ptr<character> maskee = 
        ensureType<character>(fn.this_ptr);

    if ( ! fn.nargs )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("%s.setMask() : needs an argument"), maskee->getTarget());
        );
        return as_value();
    }

    const as_value& arg = fn.arg(0);
    if ( arg.is_null() || arg.is_undefined() )
    {
        // disable mask
        maskee->setMask(NULL);
    }
    else
    {

        boost::intrusive_ptr<as_object> obj ( arg.to_object() );
        character* mask = dynamic_cast<character*>(obj.get());
        if ( ! mask )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s.setMask(%s) : first argument is not a character"),
                maskee->getTarget(), arg);
            );
            return as_value();
        }

        // ch is possibly NULL, which is intended
        maskee->setMask(mask);
    }

    //log_debug("MovieClip.setMask() TESTING");

    return as_value(true);
}

as_value
movieclip_endFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.endFill(%s): args will be discarded"),
            ss.str());
    }
    );
#ifdef DEBUG_DRAWING_API
    log_debug("%s.endFill();", movieclip->getTarget());
#endif
    movieclip->endFill();
    return as_value();
}

as_value
movieclip_lineTo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 2 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.lineTo() needs at least two arguments"));
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 2 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.lineTo(%s): args after the first two "
                        "will be discarded"), ss.str());
    }
    );

    double x = fn.arg(0).to_number();
    double y = fn.arg(1).to_number();
        
    if ( ! utility::isFinite(x) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.lineTo(%s) : non-finite first argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        x = 0;
    }
     
    if ( ! utility::isFinite(y) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.lineTo(%s) : non-finite second argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        y = 0;
    }

#ifdef DEBUG_DRAWING_API
    log_debug("%s.lineTo(%g,%g);", movieclip->getTarget(), x, y);
#endif
    movieclip->lineTo(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
    return as_value();
}

as_value
movieclip_moveTo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 2 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.moveTo() takes two args"));
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 2 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.moveTo(%s): args after the first two will "
                        "be discarded"), ss.str());
    }
    );

    double x = fn.arg(0).to_number();
    double y = fn.arg(1).to_number();
     
    if ( ! utility::isFinite(x) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.moveTo(%s) : non-finite first argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        x = 0;
    }
     
    if ( ! utility::isFinite(y) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.moveTo(%s) : non-finite second argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        y = 0;
    }

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.moveTo(%g,%g);"), movieclip->getTarget(), x, y);
#endif
    movieclip->moveTo(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
    return as_value();
}

// SWF6,7: lineStyle(thickness:Number, rgb:Number, alpha:Number) : Void
//
//    SWF8+: lineStyle(thickness:Number, rgb:Number, alpha:Number,
//                                     pixelHinting:Boolean, noScale:String,
//                                     capsStyle:String, jointStyle:String,
//                                     miterLimit:Number) : Void
as_value
movieclip_lineStyle(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    if ( ! fn.nargs )
    {
        movieclip->resetLineStyle();
        return as_value();
    }

    boost::uint8_t r = 0;
    boost::uint8_t g = 0;
    boost::uint8_t b = 0;
    boost::uint8_t a = 255;
    boost::uint16_t thickness = 0;
    bool scaleThicknessVertically = true;
    bool scaleThicknessHorizontally = true;
    bool pixelHinting = false;
    bool noClose = false;
    cap_style_e capStyle = CAP_ROUND;
    join_style_e joinStyle = JOIN_ROUND;
    float miterLimitFactor = 1.0f;

    int arguments = fn.nargs;

    const int swfVersion = movieclip->getVM().getSWFVersion();
    if (swfVersion < 8 && fn.nargs > 3)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("MovieClip.lineStyle(%s): args after the "
                          "first three will be discarded"), ss.str());
            );
        arguments = 3;
    }

    switch (arguments)
    {
        default:
            IF_VERBOSE_ASCODING_ERRORS(
                std::ostringstream ss;
                fn.dump_args(ss);
                log_aserror(_("MovieClip.lineStyle(%s): args after the "
                              "first eight will be discarded"), ss.str());
                );
        case 8:
            miterLimitFactor = utility::clamp<int>(fn.arg(7).to_int(), 1, 255);
        case 7:
        {
            std::string joinStyleStr = fn.arg(6).to_string();
            if (joinStyleStr == "miter") joinStyle = JOIN_MITER;
            else if (joinStyleStr == "round") joinStyle = JOIN_ROUND;
            else if (joinStyleStr == "bevel") joinStyle = JOIN_BEVEL;
            else
            {
                IF_VERBOSE_ASCODING_ERRORS(
                    std::ostringstream ss;
                    fn.dump_args(ss);
                    log_aserror(_("MovieClip.lineStyle(%s): invalid joinStyle"
                                "value '%s' (valid values: %s|%s|%s)"),
                        ss.str(), joinStyleStr, "miter", "round", "bevel");
                );
            }
        }
        case 6:
        {
            const std::string capStyleStr = fn.arg(5).to_string();
            if (capStyleStr == "none") capStyle = CAP_NONE;
            else if (capStyleStr == "round") capStyle = CAP_ROUND;
            else if (capStyleStr == "square") capStyle = CAP_SQUARE;
            else
            {
                IF_VERBOSE_ASCODING_ERRORS(
                    std::ostringstream ss;
                    fn.dump_args(ss);
                    log_aserror(_("MovieClip.lineStyle(%s): invalid capStyle "
                               "value '%s' (valid values: none|round|square)"),
                               ss.str(), capStyleStr);
                );
            }
        }
        case 5:
        {
            // Both values to be set here are true, so just set the
            // appropriate values to false.
            const std::string noScaleString = fn.arg(4).to_string();
            if (noScaleString == "none")
            {
                scaleThicknessVertically = false;
                scaleThicknessHorizontally = false;
            }
            else if (noScaleString == "vertical")
            {
                scaleThicknessVertically = false;
            }
            else if (noScaleString == "horizontal")
            {
                scaleThicknessHorizontally = false;
            }
            else if (noScaleString != "normal")
            {
                IF_VERBOSE_ASCODING_ERRORS(
                    std::ostringstream ss;
                    fn.dump_args(ss);
                    log_aserror(_("MovieClip.lineStyle(%s): invalid "
                                    "noScale value '%s' (valid values: "
                                    "%s|%s|%s|%s)"),
                                    ss.str(), noScaleString, "none",
                                    "vertical", "horizontal", "normal");
                );
            }
        }
        case 4:
            pixelHinting = fn.arg(3).to_bool();
        case 3:
        {
            const float alphaval = utility::clamp<float>(fn.arg(2).to_number(),
                                     0, 100);
            a = boost::uint8_t(255 * (alphaval / 100));
        }
        case 2:
        {
            boost::uint32_t rgbval = boost::uint32_t(
            utility::clamp<float>(fn.arg(1).to_number(), 0, 16777216));
            r = boost::uint8_t((rgbval & 0xFF0000) >> 16);
            g = boost::uint8_t((rgbval & 0x00FF00) >> 8);
            b = boost::uint8_t((rgbval & 0x0000FF) );
        }
        case 1:
            thickness = boost::uint16_t(PIXELS_TO_TWIPS(utility::clamp<float>(
                            fn.arg(0).to_number(), 0, 255)));
            break;
    }

    rgba color(r, g, b, a);

#ifdef DEBUG_DRAWING_API
    log_debug("%s.lineStyle(%d,%d,%d,%d);", movieclip->getTarget(), thickness, r, g, b);
#endif
    movieclip->lineStyle(thickness, color,
    scaleThicknessVertically, scaleThicknessHorizontally,
    pixelHinting, noClose, capStyle, capStyle, joinStyle, miterLimitFactor);

    return as_value();
}

as_value
movieclip_curveTo(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip =
            ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 4 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("MovieClip.curveTo() takes four args"));
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 4 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.curveTo(%s): args after the first four "
                "will be discarded"), ss.str());
    }
    );

    double cx = fn.arg(0).to_number();
    double cy = fn.arg(1).to_number();
    double ax = fn.arg(2).to_number();
    double ay = fn.arg(3).to_number();

    if ( ! utility::isFinite(cx) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite first argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        cx = 0;
    }
     
    if ( ! utility::isFinite(cy) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite second argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        cy = 0;
    }

    if ( ! utility::isFinite(ax) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite third argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(0));
        );
        ax = 0;
    }
     
    if ( ! utility::isFinite(ay) )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.curveTo(%s) : non-finite fourth argument (%s), "
            "converted to zero"), movieclip->getTarget(),
            ss.str(), fn.arg(1));
        );
        ay = 0;
    }

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.curveTo(%g,%g,%g,%g);"), movieclip->getTarget(),
            cx, cy, ax, ay);
#endif
    movieclip->curveTo(PIXELS_TO_TWIPS(cx), PIXELS_TO_TWIPS(cy),
            PIXELS_TO_TWIPS(ax), PIXELS_TO_TWIPS(ay));

    return as_value();
}

as_value
movieclip_clear(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.clear(%s): args will be discarded"),
            ss.str());
    }
    );

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.clear();"), movieclip->getTarget());
#endif
    movieclip->clear();

    return as_value();
}

as_value
movieclip_beginFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    boost::uint8_t r = 0;
    boost::uint8_t g = 0;
    boost::uint8_t b = 0;
    boost::uint8_t a = 255;

    if ( fn.nargs > 0 )
    {
        // 2^24 is the max here
        boost::uint32_t rgbval = boost::uint32_t(
                utility::clamp<float>(fn.arg(0).to_number(), 0, 16777216));
        r = boost::uint8_t( (rgbval&0xFF0000) >> 16);
        g = boost::uint8_t( (rgbval&0x00FF00) >> 8);
        b = boost::uint8_t( (rgbval&0x0000FF) );

        if ( fn.nargs > 1 )
        {
            a = 255 * utility::clamp<int>(fn.arg(1).to_int(), 0, 100) / 100;
            IF_VERBOSE_ASCODING_ERRORS(
            if ( fn.nargs > 2 )
            {
                std::stringstream ss; fn.dump_args(ss);
                log_aserror(_("MovieClip.beginFill(%s): args after the "
                        "first will be discarded"), ss.str());
            }
            );
        }

    }

    rgba color(r, g, b, a);

#ifdef DEBUG_DRAWING_API
    log_debug(_("%s.beginFill(%d,%d,%d);"), movieclip->getTarget(), r, g, b);
#endif
    movieclip->beginFill(color);

    return as_value();
}

as_value
movieclip_beginGradientFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    if ( fn.nargs < 5 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): invalid call: 5 arguments "
                "needed"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs > 5 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("MovieClip.beginGradientFill(%s): args after "
                        "the first five will be discarded"), ss.str());
    }
    );

    bool radial = false;
    std::string typeStr = fn.arg(0).to_string();
    // Case-sensitive comparison needed for this ...
    if ( typeStr == "radial" ) radial = true;
    else if ( typeStr == "linear" ) radial = false;
    else
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): first arg must be "
            "'radial' or 'linear'"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    typedef boost::intrusive_ptr<as_object> ObjPtr;

    ObjPtr colors = fn.arg(1).to_object();
    ObjPtr alphas = fn.arg(2).to_object();
    ObjPtr ratios = fn.arg(3).to_object();
    ObjPtr matrixArg = fn.arg(4).to_object();

    if ( ! colors || ! alphas || ! ratios || ! matrixArg )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): one or more of the "
            " args from 2nd to 5th don't cast to objects"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    // ----------------------------
    // Parse SWFMatrix
    // ----------------------------
    
    //
    // TODO: fix the SWFMatrix build-up, it is NOT correct for
    //             rotation.
    //             For the "boxed" SWFMatrixType and radial fills this
    //             is not a problem as this code just discards the
    //             rotation (which doesn't make sense), but for
    //             the explicit SWFMatrix type (a..i) it is a problem.
    //             The whole code can likely be simplified by 
    //             always transforming the gnash gradients to the
    //             expected gradients and subsequently applying
    //             user-specified SWFMatrix; for 'boxed' SWFMatrixType
    //             this simplification would increas cost, but
    //             it's too early to apply optimizations to the
    //             code (correctness first!!).
    //

    SWFMatrix mat;
    SWFMatrix input_matrix;

    if ( matrixArg->getMember(NSV::PROP_MATRIX_TYPE).to_string() == "box" )
    {
        
        boost::int32_t valX = PIXELS_TO_TWIPS(
                matrixArg->getMember(NSV::PROP_X).to_number()); 
        boost::int32_t valY = PIXELS_TO_TWIPS(
                matrixArg->getMember(NSV::PROP_Y).to_number()); 
        boost::int32_t valW = PIXELS_TO_TWIPS(
                matrixArg->getMember(NSV::PROP_W).to_number()); 
        boost::int32_t valH = PIXELS_TO_TWIPS(
                matrixArg->getMember(NSV::PROP_H).to_number()); 
        float valR = matrixArg->getMember(NSV::PROP_R).to_number(); 

        if ( radial )
        {
            // Radial gradient is 64x64 twips.
            input_matrix.set_scale(64.0/valW, 64.0/valH);

            // For radial gradients, dunno why translation must be negative...
            input_matrix.concatenate_translation( -valX, -valY );

            // NOTE: rotation is intentionally discarded as it would
            //             have no effect (theoretically origin of the radial
            //             fill is at 0,0 making any rotation meaningless).

        }
        else
        {
            // Linear gradient is 256x1 twips.
            //
            // No idea why we should use the 256 value for Y scale, but 
            // empirically seems to give closer results. Note that it only
            // influences rotation, which is still not correct...
            // TODO: fix it !
            input_matrix.set_scale_rotation(256.0/valW, 256.0/valH, -valR);

            // For linear gradients, dunno why translation must be negative...
            input_matrix.concatenate_translation( -valX, -valY );
        }

        mat.concatenate(input_matrix);
    }
    else
    {
        float valA = matrixArg->getMember(NSV::PROP_A).to_number() ; // xx
        float valB = matrixArg->getMember(NSV::PROP_B).to_number() ; // yx
        float valD = matrixArg->getMember(NSV::PROP_D).to_number() ; // xy
        float valE = matrixArg->getMember(NSV::PROP_E).to_number() ; // yy
        boost::int32_t valG = PIXELS_TO_TWIPS(
                matrixArg->getMember(NSV::PROP_G).to_number()); // x0
        boost::int32_t valH = PIXELS_TO_TWIPS(
                matrixArg->getMember(NSV::PROP_H).to_number()); // y0

        input_matrix.sx    = valA * 65536; // sx
        input_matrix.shx = valB * 65536; // shy
        input_matrix.shy = valD * 65536; // shx
        input_matrix.sy    = valE * 65536; // sy
        input_matrix.tx = valG; // x0
        input_matrix.ty = valH; // y0

        // This is the SWFMatrix that would transform the gnash
        // gradient to the expected flash gradient.
        // Transformation is different for linear and radial
        // gradient for Gnash (in flash they should be the same)
        SWFMatrix gnashToFlash;

        if ( radial )
        {

            // Gnash radial gradients are 64x64 with center at 32,32
            // Should be 20x20 with center at 0,0
            const double g2fs = 20.0/64.0; // gnash to flash scale
            gnashToFlash.set_scale(g2fs, g2fs);
            gnashToFlash.concatenate_translation(-32, -32);

        }
        else
        {
            // First define a SWFMatrix that would transform
            // the gnash gradient to the expected flash gradient:
            // this means translating our gradient to put the
            // center of gradient at 0,0 and then scale it to
            // have a size of 20x20 instead of 256x1 as it is
            //
            // Gnash linear gradients are 256x1 with center at 128,0
            // Should be 20x20 with center at 0,0
            gnashToFlash.set_scale(20.0/256.0, 20.0/1);
            gnashToFlash.concatenate_translation(-128, 0);

        }

        // Apply gnash to flash SWFMatrix before user-defined one
        input_matrix.concatenate(gnashToFlash);

        // Finally, and don't know why, take
        // the inverse of the resulting SWFMatrix as
        // the one which would be used.
        mat = input_matrix;
        mat.invert();
    }

    // ----------------------------
    // Create the gradients vector
    // ----------------------------

    size_t ngradients = colors->getMember(NSV::PROP_LENGTH).to_int();
    // Check length compatibility of all args
    if ( ngradients != (size_t)alphas->getMember(NSV::PROP_LENGTH).to_int() ||
        ngradients != (size_t)ratios->getMember(NSV::PROP_LENGTH).to_int() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("%s.beginGradientFill(%s): colors, alphas and "
            "ratios args don't have same length"),
            movieclip->getTarget(), ss.str());
        );
        return as_value();
    }

    // TODO: limit ngradients to a max ?
    if ( ngradients > 8 )
    {
        std::stringstream ss; fn.dump_args(ss);
        log_debug(_("%s.beginGradientFill(%s) : too many array elements"
            " for colors and ratios (%d), will trim to 8"), 
            movieclip->getTarget(), ss.str(), ngradients); 
        ngradients = 8;
    }

    VM& vm = movieclip->getVM();
    string_table& st = vm.getStringTable();

    std::vector<gradient_record> gradients;
    gradients.reserve(ngradients);
    for (size_t i=0; i<ngradients; ++i)
    {

        string_table::key key = st.find(boost::lexical_cast<std::string>(i));

        as_value colVal = colors->getMember(key);
        boost::uint32_t col = colVal.is_number() ? colVal.to_int() : 0;

        as_value alpVal = alphas->getMember(key);
        boost::uint8_t alp = alpVal.is_number() ? 
            utility::clamp<int>(alpVal.to_int(), 0, 255) : 0;

        as_value ratVal = ratios->getMember(key);
        boost::uint8_t rat = ratVal.is_number() ? 
            utility::clamp<int>(ratVal.to_int(), 0, 255) : 0;

        rgba color;
        color.parseRGB(col);
        color.m_a = alp;

        gradients.push_back(gradient_record(rat, color));
    }

    if ( radial )
    {
        movieclip->beginRadialGradientFill(gradients, mat);
    }
    else
    {
        movieclip->beginLinearGradientFill(gradients, mat);
    }

    LOG_ONCE( log_debug("MovieClip.beginGradientFill() TESTING") );
    return as_value();
}

// startDrag([lockCenter:Boolean], [left:Number], [top:Number],
//    [right:Number], [bottom:Number]) : Void`
as_value
movieclip_startDrag(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);

    drag_state st;
    st.setCharacter( movieclip.get() );

    // mark this character is transformed.
    movieclip->transformedByScript();

    if ( fn.nargs )
    {
        st.setLockCentered( fn.arg(0).to_bool() );

        if ( fn.nargs >= 5)
        {
            double x0 = fn.arg(1).to_number();
            double y0 = fn.arg(2).to_number();
            double x1 = fn.arg(3).to_number();
            double y1 = fn.arg(4).to_number();

            // check for infinite values
            bool gotinf = false;
            if ( ! utility::isFinite(x0) ) { x0=0; gotinf=true; }
            if ( ! utility::isFinite(y0) ) { y0=0; gotinf=true; }
            if ( ! utility::isFinite(x1) ) { x1=0; gotinf=true; }
            if ( ! utility::isFinite(y1) ) { y1=0; gotinf=true; }

            // check for swapped values
            bool swapped = false;
            if ( y1 < y0 )
            {
                std::swap(y1, y0);
                swapped = true;
            }

            if ( x1 < x0 )
            {
                std::swap(x1, x0);
                swapped = true;
            }

            IF_VERBOSE_ASCODING_ERRORS(
                if ( gotinf || swapped ) {
                    std::stringstream ss; fn.dump_args(ss);
                    if ( swapped ) { 
                        log_aserror(_("min/max bbox values in "
                            "MovieClip.startDrag(%s) swapped, fixing"),
                            ss.str());
                    }
                    if ( gotinf ) {
                        log_aserror(_("non-finite bbox values in "
                            "MovieClip.startDrag(%s), took as zero"),
                            ss.str());
                    }
                }
            );

            rect bounds(PIXELS_TO_TWIPS(x0), PIXELS_TO_TWIPS(y0),
                    PIXELS_TO_TWIPS(x1), PIXELS_TO_TWIPS(y1));
            st.setBounds(bounds);
        }
    }

    movieclip->getVM().getRoot().set_drag_state(st);

    log_debug("MovieClip.startDrag() TESTING");
    return as_value();
}

// stopDrag() : Void
as_value
movieclip_stopDrag(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> movieclip = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(movieclip);

    movieclip->getVM().getRoot().stop_drag();

    log_debug("MovieClip.stopDrag() TESTING");
    return as_value();
}


as_value
movieclip_beginBitmapFill(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
movieclip_getRect(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
movieclip_lineGradientStyle(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
movieclip_attachBitmap(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}


as_value
movieclip_ctor(const fn_call& /* fn */)
{
    boost::intrusive_ptr<as_object> clip = 
        new as_object(getMovieClipInterface());

    return as_value(clip.get());
}


as_value
movieclip_currentframe_get(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(std::min(ptr->get_loaded_frames(),
                ptr->get_current_frame() + 1));
}

as_value
movieclip_totalframes_get(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(ptr->get_frame_count());
}

as_value
movieclip_framesloaded_get(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(ptr->get_loaded_frames());
}

as_value
movieclip_droptarget_getset(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return ptr->getDropTarget();
}

as_value
movieclip_url_getset(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);

    return as_value(ptr->get_movie_definition()->get_url());
}

as_value
movieclip_highquality_getset(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);

    if ( fn.nargs == 0 ) // getter
    {
        // We don't support quality settings
        return as_value(true);
    }
    else // setter
    {
        LOG_ONCE( log_unimpl("MovieClip._highquality setting") );
    }
    return as_value();
}

// TODO: move this to character class, _focusrect seems a generic property
as_value
movieclip_focusrect_getset(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);

    if ( fn.nargs == 0 ) // getter
    {
        // Is a yellow rectangle visible around a focused movie clip (?)
        // We don't support focuserct settings
        return as_value(false);
    }
    else // setter
    {
        LOG_ONCE( log_unimpl("MovieClip._focusrect setting") );
    }
    return as_value();
}

as_value
movieclip_soundbuftime_getset(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
    UNUSED(ptr);

    if ( fn.nargs == 0 ) // getter
    {
        // Number of seconds before sound starts to stream.
        return as_value(0.0);
    }
    else // setter
    {
        LOG_ONCE( log_unimpl("MovieClip._soundbuftime setting") );
    }
    return as_value();
}

as_value
movieclip_transform(const fn_call& fn)
{
    boost::intrusive_ptr<MovieClip> ptr = 
        ensureType<MovieClip>(fn.this_ptr);
        
    VM& vm = ptr->getVM();
    string_table& st = ptr->getVM().getStringTable();

    as_value flash;
    if (!vm.getGlobal()->get_member(st.find("flash"), &flash))
    {
        log_error("No flash object found!");
        return as_value();
    }
    boost::intrusive_ptr<as_object> flashObj = flash.to_object();

    if (!flashObj)
    {
        log_error("flash isn't an object!");
        return as_value();
    }
        
    as_value geom;
    if (!flashObj->get_member(st.find("geom"), &geom))
    {
        log_error("No flash.geom object found!");
        return as_value();
    }
    boost::intrusive_ptr<as_object> geomObj = geom.to_object();

    if (!geomObj)
    {
        log_error("flash.geom isn't an object!");
        return as_value();
    }
        
    as_value transform;
    if (!geomObj->get_member(st.find("Transform"), &transform))
    {
        log_error("No flash.geom.Transform object found!");
        return as_value();
    }        

    boost::intrusive_ptr<as_function> transformCtor =
        transform.to_as_function();

    if (!transformCtor)
    {
        log_error("flash.geom.Transform isn't a function!");
        return as_value();
    }

    // Construct a flash.geom.Transform object with "this" as argument.
    std::auto_ptr< std::vector<as_value> > args (new std::vector<as_value>);
    args->push_back(ptr.get());

    boost::intrusive_ptr<as_object> transformObj =
        transformCtor->constructInstance(fn.env(), args);

    return as_value(transformObj.get());
}

/// Properties (and/or methods) attached to every *instance* of a MovieClip 
void
attachMovieClipProperties(character& o)
{
    //int target_version = o.getVM().getSWFVersion();

    // This is a normal property, can be overridden, deleted and enumerated
    // See swfdec/test/trace/movieclip-version-#.swf for why we only initialize this
    // if we don't have a parent
    if ( ! o.get_parent() ) o.init_member( "$version",
            VM::get().getPlayerVersion(), 0); 

    //
    // Properties (TODO: move to appropriate SWF version section)
    //
    
    as_c_function_ptr gettersetter;

    gettersetter = character::x_getset;
    o.init_property(NSV::PROP_uX, gettersetter, gettersetter);

    gettersetter = character::y_getset;
    o.init_property(NSV::PROP_uY, gettersetter, gettersetter);

    gettersetter = character::xscale_getset;
    o.init_property(NSV::PROP_uXSCALE, gettersetter, gettersetter);

    gettersetter = character::yscale_getset;
    o.init_property(NSV::PROP_uYSCALE, gettersetter, gettersetter);

    gettersetter = character::xmouse_get;
    o.init_readonly_property(NSV::PROP_uXMOUSE, gettersetter);

    gettersetter = character::ymouse_get;
    o.init_readonly_property(NSV::PROP_uYMOUSE, gettersetter);

    gettersetter = character::alpha_getset;
    o.init_property(NSV::PROP_uALPHA, gettersetter, gettersetter);

    gettersetter = character::visible_getset;
    o.init_property(NSV::PROP_uVISIBLE, gettersetter, gettersetter);

    gettersetter = character::width_getset;
    o.init_property(NSV::PROP_uWIDTH, gettersetter, gettersetter);

    gettersetter = character::height_getset;
    o.init_property(NSV::PROP_uHEIGHT, gettersetter, gettersetter);

    gettersetter = character::rotation_getset;
    o.init_property(NSV::PROP_uROTATION, gettersetter, gettersetter);

    gettersetter = character::parent_getset;
    o.init_property(NSV::PROP_uPARENT, gettersetter, gettersetter);

    gettersetter = movieclip_currentframe_get;
    o.init_property(NSV::PROP_uCURRENTFRAME, gettersetter, gettersetter);

    gettersetter = movieclip_totalframes_get;
    o.init_property(NSV::PROP_uTOTALFRAMES, gettersetter, gettersetter);

    gettersetter = movieclip_framesloaded_get;
    o.init_property(NSV::PROP_uFRAMESLOADED, gettersetter, gettersetter);

    gettersetter = character::target_getset;
    o.init_property(NSV::PROP_uTARGET, gettersetter, gettersetter);

    gettersetter = character::name_getset;
    o.init_property(NSV::PROP_uNAME, gettersetter, gettersetter);

    gettersetter = movieclip_droptarget_getset;
    o.init_property(NSV::PROP_uDROPTARGET, gettersetter, gettersetter);

    gettersetter = movieclip_url_getset;
    o.init_property(NSV::PROP_uURL, gettersetter, gettersetter);

    gettersetter = movieclip_highquality_getset;
    o.init_property(NSV::PROP_uHIGHQUALITY, gettersetter, gettersetter);

    gettersetter = movieclip_focusrect_getset;
    o.init_property(NSV::PROP_uFOCUSRECT, gettersetter, gettersetter);

    gettersetter = movieclip_soundbuftime_getset;
    o.init_property(NSV::PROP_uSOUNDBUFTIME, gettersetter, gettersetter);

}

as_object*
getMovieClipInterface()
{
    static boost::intrusive_ptr<as_object> proto;
    if ( proto == NULL )
    {
        proto = new as_object(getObjectInterface());
        VM& vm = VM::get();
        vm.addStatic(proto.get());
        registerNatives(vm);
        attachMovieClipInterface(*proto);
        //proto->init_member("constructor", new builtin_function(movieclip_ctor));
    }
    return proto.get();
}

} // anonymous namespace

} // namespace gnash
