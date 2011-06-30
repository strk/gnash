// Button.cpp:  Mouse-sensitive buttons, for Gnash.
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
#include "gnashconfig.h" // USE_SWF_TREE
#endif

#include "Button.h"

#include <boost/bind.hpp>
#include <utility>
#include <functional>

#include "DefineButtonTag.h"
#include "as_value.h"
#include "Button.h"
#include "ActionExec.h"
#include "MovieClip.h"
#include "movie_root.h"
#include "VM.h"
#include "NativeFunction.h"
#include "fn_call.h" 
#include "ExecutableCode.h"
#include "namedStrings.h"
#include "StringPredicates.h"
#include "GnashKey.h" 
#include "SoundInfoRecord.h" 
#include "Global_as.h" 
#include "RunResources.h"
#include "sound_definition.h"
#include "Transform.h"

/** \page buttons Buttons and mouse behaviour

Observations about button & mouse behavior

Entities that receive mouse events: only buttons and sprites, AFAIK

When the mouse button goes down, it becomes "captured" by whatever
element is topmost, directly below the mouse at that moment.  While
the mouse is captured, no other entity receives mouse events,
regardless of how the mouse or other elements move.

The mouse remains captured until the mouse button goes up.  The mouse
remains captured even if the element that captured it is removed from
the display list.

If the mouse isn't above a button or sprite when the mouse button goes
down, then the mouse is captured by the background (i.e. mouse events
just don't get sent, until the mouse button goes up again).

Mouse events:

+------------------+---------------+-------------------------------------+
| Event            | Mouse Button  | description                         |
=========================================================================
| onRollOver       |     up        | sent to topmost entity when mouse   |
|                  |               | cursor initially goes over it       |
+------------------+---------------+-------------------------------------+
| onRollOut        |     up        | when mouse leaves entity, after     |
|                  |               | onRollOver                          |
+------------------+---------------+-------------------------------------+
| onPress          |  up -> down   | sent to topmost entity when mouse   |
|                  |               | button goes down.  onRollOver       |
|                  |               | always precedes onPress.  Initiates |
|                  |               | mouse capture.                      |
+------------------+---------------+-------------------------------------+
| onRelease        |  down -> up   | sent to active entity if mouse goes |
|                  |               | up while over the element           |
+------------------+---------------+-------------------------------------+
| onDragOut        |     down      | sent to active entity if mouse      |
|                  |               | is no longer over the entity        |
+------------------+---------------+-------------------------------------+
| onReleaseOutside |  down -> up   | sent to active entity if mouse goes |
|                  |               | up while not over the entity.       |
|                  |               | onDragOut always precedes           |
|                  |               | onReleaseOutside                    |
+------------------+---------------+-------------------------------------+
| onDragOver       |     down      | sent to active entity if mouse is   |
|                  |               | dragged back over it after          |
|                  |               | onDragOut                           |
+------------------+---------------+-------------------------------------+

There is always one active entity at any given time (considering NULL to
be an active entity, representing the background, and other objects that
don't receive mouse events).

When the mouse button is up, the active entity is the topmost element
directly under the mouse pointer.

When the mouse button is down, the active entity remains whatever it
was when the button last went down.

The active entity is the only object that receives mouse events.

!!! The "trackAsMenu" property alters this behavior!  If trackAsMenu
is set on the active entity, then onReleaseOutside is filtered out,
and onDragOver from another entity is allowed (from the background, or
another trackAsMenu entity). !!!


Pseudocode:

active_entity = NULL
mouse_button_state = UP
mouse_inside_entity_state = false
frame loop:
  if mouse_button_state == DOWN

    // Handle trackAsMenu
    if (active_entity->trackAsMenu)
      possible_entity = topmost entity below mouse
      if (possible_entity != active_entity && possible_entity->trackAsMenu)
        // Transfer to possible entity
    active_entity = possible_entity
    active_entity->onDragOver()
    mouse_inside_entity_state = true;

    // Handle onDragOut, onDragOver
    if (mouse_inside_entity_state == false)
      if (mouse is actually inside the active_entity)
        // onDragOver
    active_entity->onDragOver()
        mouse_inside_entity_state = true;

    else // mouse_inside_entity_state == true
      if (mouse is actually outside the active_entity)
        // onDragOut
    active_entity->onDragOut()
    mouse_inside_entity_state = false;

    // Handle onRelease, onReleaseOutside
    if (mouse button is up)
      if (mouse_inside_entity_state)
        // onRelease
        active_entity->onRelease()
      else
        // onReleaseOutside
    if (active_entity->trackAsMenu == false)
          active_entity->onReleaseOutside()
      mouse_button_state = UP
    
  if mouse_button_state == UP
    new_active_entity = topmost entity below the mouse
    if (new_active_entity != active_entity)
      // onRollOut, onRollOver
      active_entity->onRollOut()
      active_entity = new_active_entity
      active_entity->onRollOver()
    
    // Handle press
    if (mouse button is down)
      // onPress
      active_entity->onPress()
      mouse_inside_entity_state = true
      mouse_button_state = DOWN

*/


namespace gnash {

namespace {
    as_value button_blendMode(const fn_call& fn);
    as_value button_cacheAsBitmap(const fn_call& fn);
    as_value button_filters(const fn_call& fn);
    as_value button_scale9Grid(const fn_call& fn);
    as_value button_setTabIndex(const fn_call& fn);
    as_value button_getTabIndex(const fn_call& fn);
    as_value button_getDepth(const fn_call& fn);
}

namespace {

class ButtonActionExecutor {
public:
    ButtonActionExecutor(as_environment& env)
        :
        _env(env)
    {}

    void operator() (const action_buffer& ab)
    {
        ActionExec exec(ab, _env);
        exec();
    }
private:
    as_environment& _env;
};

class ButtonActionPusher
{
public:
    ButtonActionPusher(movie_root& mr, DisplayObject* this_ptr)
        :
        _mr(mr),
        _tp(this_ptr)
    {}

    void operator()(const action_buffer& ab)
    {
        _mr.pushAction(ab, _tp);
    }

private:
    movie_root& _mr;
    DisplayObject* _tp;
};

class ButtonKeyRegisterer : public std::unary_function<int, void>
{
public:
    ButtonKeyRegisterer(movie_root& mr, Button* this_ptr)
        :
        _mr(mr),
        _tp(this_ptr)
    {}

    void operator()(int code) const
    {
        _mr.registerButtonKey(code, _tp);
    }

private:
    movie_root& _mr;
    Button* _tp;
};

}

namespace {
    void addInstanceProperty(Button& b, DisplayObject* d) {
        if (!d) return;
        const ObjectURI& name = d->get_name();
        if (name.empty()) return;
        getObject(&b)->init_member(name, getObject(d), 0);
    }

    void removeInstanceProperty(Button& b, DisplayObject* d) {
        if (!d) return;
        const ObjectURI& name = d->get_name();
        if (name.empty()) return;
        getObject(&b)->delProperty(name);
    }
}

/// Predicates for standard algorithms.

/// Depth comparator for DisplayObjects.
static bool charDepthLessThen(const DisplayObject* ch1, const DisplayObject* ch2) 
{
    return ch1->get_depth() < ch2->get_depth();
}

/// Predicate for finding active DisplayObjects.
//
/// Returns true if the DisplayObject should be skipped:
/// 1) if it is NULL, or 
/// 2) if we don't want unloaded DisplayObjects and the DisplayObject is unloaded.
static bool isCharacterNull(DisplayObject* ch, bool includeUnloaded)
{
    return (!ch || (!includeUnloaded && ch->unloaded()));
}

static void
attachButtonInterface(as_object& o)
{
    
    const int unprotected = 0;
    o.init_member(NSV::PROP_ENABLED, true, unprotected);
    o.init_member("useHandCursor", true, unprotected);
    
    const int swf8Flags = PropFlags::onlySWF8Up;
    VM& vm = getVM(o);

    o.init_property("tabIndex", *vm.getNative(105, 1), *vm.getNative(105, 2),
            swf8Flags);
    
    o.init_member("getDepth", vm.getNative(105, 3), unprotected);

    NativeFunction* gs;
    gs = vm.getNative(105, 4);
    o.init_property("scale9Grid", *gs, *gs, swf8Flags);
    gs = vm.getNative(105, 5);
    o.init_property("filters", *gs, *gs, swf8Flags);
    gs = vm.getNative(105, 6);
    o.init_property("cacheAsBitmap", *gs, *gs, swf8Flags);
    gs = vm.getNative(105, 7);
    o.init_property("blendMode", *gs, *gs, swf8Flags);

}

Button::Button(as_object* object, const SWF::DefineButtonTag* def,
        DisplayObject* parent)
    :
    InteractiveObject(object, parent),
    _mouseState(MOUSESTATE_UP),
    _def(def)
{
    assert(object);
}

Button::~Button()
{
}

bool
Button::trackAsMenu()
{
    // TODO: check whether the AS or the tag value takes precedence.
    as_object* obj = getObject(this);
    assert(obj);

    VM& vm = getVM(*obj);

    as_value track;
    // TODO: use NSV
    const ObjectURI& propTrackAsMenu = getURI(vm, "trackAsMenu");
    if (obj->get_member(propTrackAsMenu, &track)) {
        return toBool(track, vm);
    }
    if (_def) return _def->trackAsMenu();
    return false;
}

bool 
Button::isEnabled()
{
    as_object* obj = getObject(this);
    assert(obj);

    as_value enabled;
    if (!obj->get_member(NSV::PROP_ENABLED, &enabled)) return false;

    return toBool(enabled, getVM(*obj));
}


void
Button::notifyEvent(const event_id& id)
{
    if (unloaded()) {
        // We don't respond to events while unloaded
        // See bug #22982
        return; 
    }

    assert(id.id() == event_id::KEY_PRESS);
    assert(id.keyCode() != key::INVALID);

    ButtonActionPusher xec(stage(), this); 
    _def->forEachTrigger(id, xec);
}

bool
Button::handleFocus()
{
    /// Nothing to do, but can receive focus.
    return false;
}


void
Button::display(Renderer& renderer, const Transform& base)
{
    const DisplayObject::MaskRenderer mr(renderer, *this);

    const Transform xform = base * transform();

    DisplayObjects actChars;
    getActiveCharacters(actChars);

    // TODO: by keeping chars sorted by depth we'd avoid the sort on display
    std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

    for (DisplayObjects::iterator it = actChars.begin(), e = actChars.end();
            it != e; ++it) {
        (*it)->display(renderer, xform);
    }

    clear_invalidated();
}


// Return the topmost entity that the given point covers.  NULL if none.
// I.e. check against ourself.
InteractiveObject*
Button::topmostMouseEntity(boost::int32_t x, boost::int32_t y)
{
    if (!visible() || !isEnabled())
    {
        return 0;
    }

    //-------------------------------------------------
    // Check our active and visible children first
    //-------------------------------------------------

    DisplayObjects actChars;
    getActiveCharacters(actChars);

    if ( ! actChars.empty() )
    {
        std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

        SWFMatrix m = getMatrix(*this);
        point  p(x, y);
        m.invert().transform(p);

        for (DisplayObjects::reverse_iterator it = actChars.rbegin(),
                itE=actChars.rend(); it!=itE; ++it)
        {
            DisplayObject* ch = *it;
            if ( ! ch->visible() ) continue;
            InteractiveObject *hit = ch->topmostMouseEntity(p.x, p.y);
            if ( hit ) return hit;
        }
    }

    //-------------------------------------------------
    // If that failed, check our hit area
    //-------------------------------------------------

    // Find hit DisplayObjects
    if ( _hitCharacters.empty() ) return 0;

    // point is in p's space,
    // we need to convert it in world space
    point  wp(x,y);
    DisplayObject* p = parent();
    if (p) {
        getWorldMatrix(*p).transform(wp);
    }

    for (DisplayObjects::const_iterator i = _hitCharacters.begin(),
         e = _hitCharacters.end(); i !=e; ++i)
    {
        if ((*i)->pointInVisibleShape(wp.x, wp.y))
        {
            // The mouse is inside the shape.
            return this;
        }
    }

    return NULL;
}


void
Button::mouseEvent(const event_id& event)
{
    if (unloaded()) {
        // We don't respond to events while unloaded. See bug #22982.
        log_debug("Button %s received %s button event while unloaded: ignored",
            getTarget(), event);
        return;
    }

    MouseState new_state = _mouseState;
  
    // Set our mouse state (so we know how to render).
    switch (event.id())
    {
        case event_id::ROLL_OUT:
        case event_id::RELEASE_OUTSIDE:
            new_state = MOUSESTATE_UP;
            break;

        case event_id::RELEASE:
        case event_id::ROLL_OVER:
        case event_id::DRAG_OUT:
        case event_id::MOUSE_UP:
            new_state = MOUSESTATE_OVER;
            break;

        case event_id::PRESS:
        case event_id::DRAG_OVER:
        case event_id::MOUSE_DOWN:
            new_state = MOUSESTATE_DOWN;
            break;

        default:
            //abort();  // missed a case?
            log_error(_("Unhandled button event %s"), event);
            break;
    }
    
    set_current_state(new_state);
    
    // Button transition sounds.
    do {

        if (!_def->hasSound()) break;

        // Check if there is a sound handler
        sound::sound_handler* s = getRunResources(*getObject(this)).soundHandler();
        if (!s) break;

        int bi; // button sound array index [0..3]

        switch (event.id())
        {
            case event_id::ROLL_OUT:
                bi = 0;
                break;
            case event_id::ROLL_OVER:
                bi = 1;
                break;
            case event_id::PRESS:
                bi = 2;
                break;
            case event_id::RELEASE:
                bi = 3;
                break;
            default:
                bi = -1;
                break;
        }

        // no sound for this transition
        if (bi < 0) break;

        const SWF::DefineButtonSoundTag::ButtonSound& bs = 
            _def->buttonSound(bi);

        // character zero is considered as null character
        if (!bs.soundID) break;

        // No actual sound ?
        if (!bs.sample) break;

        if (bs.soundInfo.stopPlayback) {
            s->stop_sound(bs.sample->m_sound_handler_id);
        }
        else {
            const SWF::SoundInfoRecord& sinfo = bs.soundInfo;

            const sound::SoundEnvelopes* env = 
                sinfo.envelopes.empty() ? 0 : &sinfo.envelopes;

            s->startSound(bs.sample->m_sound_handler_id,
                    bs.soundInfo.loopCount,
                    env, // envelopes
                    !sinfo.noMultiple, // allow multiple instances ?
                    sinfo.inPoint,
                    sinfo.outPoint
                    );
        }

    } while(0);

    // From: "ActionScript - The Definitive Guide" by Colin Moock
    // (chapter 10: Events and Event Handlers)
    //
    // "Event-based code [..] is said to be executed asynchronously
    //  because the triggering of events can occur at arbitrary times."
    //
    // We'll push to the global list. The movie_root will process
    // the action queue on mouse event.
    //

    movie_root& mr = stage();

    ButtonActionPusher xec(mr, this); 
    _def->forEachTrigger(event, xec);

    // check for built-in event handler.
    std::auto_ptr<ExecutableCode> code (get_event_handler(event));
    if (code.get()) {
        mr.pushAction(code, movie_root::PRIORITY_DOACTION);
    }

    sendEvent(*getObject(this), get_environment(), event.functionURI());
}


void
Button::getActiveCharacters(ConstDisplayObjects& list) const
{
    list.clear();

    // Copy all the DisplayObjects to the new list, skipping NULL and unloaded
    // DisplayObjects.
    std::remove_copy_if(_stateCharacters.begin(), _stateCharacters.end(),
            std::back_inserter(list),
            boost::bind(&isCharacterNull, _1, false));

}


void 
Button::getActiveCharacters(DisplayObjects& list, bool includeUnloaded)
{
    list.clear();

    // Copy all the DisplayObjects to the new list, skipping NULL
    // DisplayObjects, optionally including unloaded DisplayObjects.
    std::remove_copy_if(_stateCharacters.begin(), _stateCharacters.end(),
            std::back_inserter(list),
            boost::bind(&isCharacterNull, _1, includeUnloaded));
    
}

void 
Button::get_active_records(ActiveRecords& list, MouseState state)
{
    list.clear();
    
    using namespace SWF;
    const DefineButtonTag::ButtonRecords& br = _def->buttonRecords();
    size_t index = 0;

    for (DefineButtonTag::ButtonRecords::const_iterator i = br.begin(),
            e = br.end(); i != e; ++i, ++index)
    {
        const ButtonRecord& rec =*i;
        if (rec.hasState(state)) list.insert(index);
    }
}

#ifdef GNASH_DEBUG_BUTTON_DISPLAYLIST
static void dump(Button::DisplayObjects& chars, std::stringstream& ss)
{
    for (size_t i=0, e=chars.size(); i<e; ++i)
    {
        ss << "Record" << i << ": ";
        DisplayObject* ch = chars[i];
        if ( ! ch ) ss << "NULL.";
        else
        {
            ss << ch->getTarget() << " (depth:" << 
                ch->get_depth()-DisplayObject::staticDepthOffset-1
                << " unloaded:" << ch->unloaded() <<
                " destroyed:" << ch->isDestroyed() << ")";
        }
        ss << std::endl;
    }
}
#endif

void
Button::set_current_state(MouseState new_state)
{
    if (new_state == _mouseState)
        return;
        
#ifdef GNASH_DEBUG_BUTTON_DISPLAYLIST
    std::stringstream ss;
    ss << "at set_current_state enter: " << std::endl;
    dump(_stateCharacters, ss);
    log_debug("%s", ss.str());
#endif

    // Get new state records
    ActiveRecords newChars;
    get_active_records(newChars, new_state);

    // For each possible record, check if it should still be there
    for (size_t i=0, e=_stateCharacters.size(); i<e; ++i)
    {
        DisplayObject* oldch = _stateCharacters[i];
        bool shouldBeThere = ( newChars.find(i) != newChars.end() );

        if ( ! shouldBeThere )
        {

            // is there, but is unloaded: destroy, clear slot and go on
            if ( oldch && oldch->unloaded() ) {
                removeInstanceProperty(*this, oldch);
                if ( ! oldch->isDestroyed() ) oldch->destroy();
                _stateCharacters[i] = NULL;
                oldch = NULL;
            }

            if ( oldch ) // the one we have should not be there... unload!
            {
                set_invalidated();

                if ( ! oldch->unload() )
                {
                    // No onUnload handler: destroy and clear slot
                    removeInstanceProperty(*this, oldch);
                    if (!oldch->isDestroyed()) oldch->destroy();
                    _stateCharacters[i] = NULL;
                }
                else
                {
                    // onUnload handler: shift depth and keep slot
                    int oldDepth = oldch->get_depth();
                    int newDepth = DisplayObject::removedDepthOffset - oldDepth;
#ifdef GNASH_DEBUG_BUTTON_DISPLAYLIST
                    log_debug("Removed button record shifted from depth %d to depth %d", oldDepth, newDepth);
#endif
                    oldch->set_depth(newDepth);
                }
            }
        }
        else // should be there
        {
            // Is there already, but is unloaded: destroy and consider as gone
            if ( oldch && oldch->unloaded() )
            {
                removeInstanceProperty(*this, oldch);
                if ( ! oldch->isDestroyed() ) oldch->destroy();
                _stateCharacters[i] = NULL;
                oldch = NULL;
            }

            if (!oldch) {
                // Not there, instantiate
                const SWF::ButtonRecord& rec = _def->buttonRecords()[i];
                DisplayObject* ch = rec.instantiate(this);
                
                set_invalidated();
                _stateCharacters[i] = ch;
                addInstanceProperty(*this, ch);
                ch->construct(); 
            }
        }
    }

#ifdef GNASH_DEBUG_BUTTON_DISPLAYLIST
    ss.str("");
    ss << "at set_current_state end: " << std::endl;
    dump(_stateCharacters, ss);
    log_debug("%s", ss.str());
#endif

    // Remember current state
    _mouseState=new_state;
     
}

void 
Button::add_invalidated_bounds(InvalidatedRanges& ranges, bool force)
{

    // Not visible anyway
    if (!visible()) return;

    ranges.add(m_old_invalidated_ranges);  

    DisplayObjects actChars;
    getActiveCharacters(actChars);
    std::for_each(actChars.begin(), actChars.end(),
            boost::bind(&DisplayObject::add_invalidated_bounds, _1,
                boost::ref(ranges), force || invalidated())
    );
}

SWFRect
Button::getBounds() const
{
    SWFRect allBounds;

    typedef std::vector<const DisplayObject*> Chars;
    Chars actChars;
    getActiveCharacters(actChars);
    for(Chars::const_iterator i=actChars.begin(),e=actChars.end(); i!=e; ++i)
    {
        const DisplayObject* ch = *i;
        // Child bounds need be transformed in our coordinate space
        SWFRect lclBounds = ch->getBounds();
        SWFMatrix m = getMatrix(*ch);
        allBounds.expand_to_transformed_rect(m, lclBounds);
    }

    return allBounds;
}

bool
Button::pointInShape(boost::int32_t x, boost::int32_t y) const
{
    typedef std::vector<const DisplayObject*> Chars;
    Chars actChars;
    getActiveCharacters(actChars);
    for(Chars::const_iterator i=actChars.begin(),e=actChars.end(); i!=e; ++i)
    {
        const DisplayObject* ch = *i;
        if (ch->pointInShape(x,y)) return true;
    }
    return false; 
}

void
Button::construct(as_object* initObj)
{
    // This can happen if attachMovie is called with an exported Button and
    // an init object. The attachment happens, but the init object is not used
    // (see misc-ming.all/attachMovieTest.swf).
    if (initObj) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("Button placed with an init object. This will "
                "be ignored.");
        );
    }

    saveOriginalTarget(); // for soft refs

    // Don't register this button instance as a live DisplayObject.

    // Instantiate the hit DisplayObjects
    ActiveRecords hitChars;
    get_active_records(hitChars, MOUSESTATE_HIT);
    for (ActiveRecords::iterator i=hitChars.begin(),e=hitChars.end(); i!=e; ++i)
    {
        const SWF::ButtonRecord& rec = _def->buttonRecords()[*i];

        // These should not be named!
        DisplayObject* ch = rec.instantiate(this, false);
        _hitCharacters.push_back(ch);
    }

    // Setup the state DisplayObjects container
    // It will have a slot for each DisplayObject record.
    // Some slots will probably be never used (consider HIT-only records)
    // but for now this direct corrispondence between record number
    // and active DisplayObject will be handy.
    _stateCharacters.resize(_def->buttonRecords().size());

    // Instantiate the default state DisplayObjects 
    ActiveRecords upChars;
    get_active_records(upChars, MOUSESTATE_UP);

    for (ActiveRecords::iterator i = upChars.begin(), e=upChars.end();
            i != e; ++i)
    {
        int rno = *i;
        const SWF::ButtonRecord& rec = _def->buttonRecords()[rno];

        DisplayObject* ch = rec.instantiate(this);

        _stateCharacters[rno] = ch;
        addInstanceProperty(*this, ch);
        ch->construct();
    }

    // There is no INITIALIZE/CONSTRUCT/LOAD/ENTERFRAME/UNLOAD event 
    // for Buttons

    // Register key events.
    if (_def->hasKeyPressHandler()) {
        ButtonKeyRegisterer r(stage(), this);
        _def->visitKeyCodes(r);
    }

}

void
Button::markOwnResources() const
{

    // Mark state DisplayObjects as reachable
    for (DisplayObjects::const_iterator i = _stateCharacters.begin(),
            e = _stateCharacters.end(); i != e; ++i)
    {
        DisplayObject* ch = *i;
        if (ch) ch->setReachable();
    }

    // Mark hit DisplayObjects as reachable
    std::for_each(_hitCharacters.begin(), _hitCharacters.end(),
            std::mem_fun(&DisplayObject::setReachable));

}

bool
Button::unloadChildren()
{
    GNASH_REPORT_FUNCTION;

    bool childsHaveUnload = false;

    // We need to unload all children, or the global instance list
    // will keep growing forever !
    for (DisplayObjects::iterator i = _stateCharacters.begin(),
            e = _stateCharacters.end(); i != e; ++i)
    {
        DisplayObject* ch = *i;
        if (!ch || ch->unloaded()) continue;
        if (ch->unload()) childsHaveUnload = true;
    }

    // NOTE: we don't need to ::unload or ::destroy here
    //       as the _hitCharacters are never placed on stage.
    //       As an optimization we might not even instantiate
    //       them, and only use the definition and the 
    //       associated transform SWFMatrix... (would take
    //       hit instance off the GC).
    _hitCharacters.clear();

    return childsHaveUnload;
}

void
Button::destroy()
{
    GNASH_REPORT_FUNCTION;

    stage().removeButtonKey(this);

    for (DisplayObjects::iterator i = _stateCharacters.begin(),
            e=_stateCharacters.end(); i != e; ++i) {
        DisplayObject* ch = *i;
        if (!ch || ch->isDestroyed()) continue;
        ch->destroy();
    }

    // NOTE: we don't need to ::unload or ::destroy here
    //       as the _hitCharacters are never placed on stage.
    //       As an optimization we might not even instantiate
    //       them, and only use the definition and the 
    //       associated transform SWFMatrix... (would take
    //       hit instance off the GC).
    _hitCharacters.clear();

    DisplayObject::destroy();
}

int
Button::getDefinitionVersion() const
{
    return _def->getSWFVersion();
}

static as_value
button_ctor(const fn_call& /*fn*/)
{
    return as_value();
}

void
button_class_init(as_object& global, const ObjectURI& uri)
{
    // This is going to be the global Button "class"/"function"
    Global_as& gl = getGlobal(global);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(&button_ctor, proto);
    attachButtonInterface(*proto);

    // Register _global.MovieClip
    global.init_member(uri, cl, as_object::DefaultFlags);
}

void
registerButtonNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(button_setTabIndex, 105, 1);
    vm.registerNative(button_getTabIndex, 105, 2);
    vm.registerNative(button_getDepth, 105, 3);
    vm.registerNative(button_scale9Grid, 105, 4);
    vm.registerNative(button_filters, 105, 5);
    vm.registerNative(button_cacheAsBitmap, 105, 6);
    vm.registerNative(button_blendMode, 105, 7);
}

#ifdef USE_SWFTREE
DisplayObject::InfoTree::iterator 
Button::getMovieInfo(InfoTree& tr, InfoTree::iterator it)
{
    InfoTree::iterator selfIt = DisplayObject::getMovieInfo(tr, it);
    std::ostringstream os;

    DisplayObjects actChars;
    getActiveCharacters(actChars, true);
    std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

    os << actChars.size() << " active DisplayObjects for state " <<
        mouseStateName(_mouseState);
    InfoTree::iterator localIter = tr.append_child(selfIt,
            std::make_pair(_("Button state"), os.str()));

    os.str("");
    os << std::boolalpha << isEnabled();
    localIter = tr.append_child(selfIt, std::make_pair(_("Enabled"), os.str()));

    std::for_each(actChars.begin(), actChars.end(),
            boost::bind(&DisplayObject::getMovieInfo, _1, tr, localIter)); 

    return selfIt;

}
#endif

const char*
Button::mouseStateName(MouseState s)
{
    switch (s)
    {
        case MOUSESTATE_UP: return "UP";
        case MOUSESTATE_DOWN: return "DOWN";
        case MOUSESTATE_OVER: return "OVER";
        case MOUSESTATE_HIT: return "HIT";
        default: std::abort();
    }
}

namespace {

as_value
button_blendMode(const fn_call& fn)
{
    Button* obj = ensure<IsDisplayObject<Button> >(fn);
    UNUSED(obj);
    return as_value();
}

as_value
button_cacheAsBitmap(const fn_call& fn)
{
    Button* obj = ensure<IsDisplayObject<Button> >(fn);
    UNUSED(obj);
    return as_value();
}

as_value
button_filters(const fn_call& fn)
{
    Button* obj = ensure<IsDisplayObject<Button> >(fn);
    UNUSED(obj);
    return as_value();
}

as_value
button_scale9Grid(const fn_call& fn)
{
    Button* obj = ensure<IsDisplayObject<Button> >(fn);
    UNUSED(obj);
    return as_value();
}

as_value
button_getTabIndex(const fn_call& fn)
{
    Button* obj = ensure<IsDisplayObject<Button> >(fn);
    UNUSED(obj);
    return as_value();
}

as_value
button_setTabIndex(const fn_call& fn)
{
    Button* obj = ensure<IsDisplayObject<Button> >(fn);
    UNUSED(obj);
    return as_value();
}

as_value
button_getDepth(const fn_call& fn)
{
    Button* obj = ensure<IsDisplayObject<Button> >(fn);
    UNUSED(obj);
    return as_value();
}

} // anonymous namespace
} // end of namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
