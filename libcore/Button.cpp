// Button.cpp:  Mouse-sensitive buttons, for Gnash.
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
#include "gnashconfig.h" // USE_SWF_TREE
#endif

#include "smart_ptr.h" // GNASH_USE_GC
#include "Button.h"
#include "DefineButtonTag.h"
#include "as_value.h"
#include "Button.h"
#include "ActionExec.h"
#include "MovieClip.h"
#include "movie_root.h"
#include "VM.h"
#include "builtin_function.h"
#include "fn_call.h" 
#include "ExecutableCode.h"
#include "namedStrings.h"
#include "Object.h" // for getObjectInterface
#include "StringPredicates.h"
#include "GnashKey.h" 
#include "SoundInfoRecord.h" 
#include "Global_as.h" 

#include <boost/bind.hpp>

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

class ButtonActionPusher {
public:
    ButtonActionPusher(movie_root& mr, DisplayObject* this_ptr)
        :
        called(false),
        _mr(mr),
        _tp(this_ptr)
    {}

    void operator() (const action_buffer& ab)
    {
        _mr.pushAction(ab, boost::intrusive_ptr<DisplayObject>(_tp));
        called = true;
    }

    bool called;

private:
    movie_root& _mr;
    DisplayObject* _tp;
};

}

// Forward declarations
static as_object* getButtonInterface();

namespace {
    void addInstanceProperty(Button& b, DisplayObject* d) {
        if (!d) return;
        const std::string& name = d->get_name();
        if (name.empty()) return;
        b.init_member(name, d, 0);
    }

    void removeInstanceProperty(Button& b, DisplayObject* d) {
        if (!d) return;
        const std::string& name = d->get_name();
        if (name.empty()) return;
        b.delProperty(getStringTable(b).find(name));
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

    as_c_function_ptr gettersetter;

    o.init_property(NSV::PROP_uQUALITY, DisplayObject::quality,
            DisplayObject::quality);
    
    o.init_property(NSV::PROP_uHIGHQUALITY, DisplayObject::highquality,
            DisplayObject::highquality);

    gettersetter = &DisplayObject::x_getset;
    o.init_property(NSV::PROP_uX, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::y_getset;
    o.init_property(NSV::PROP_uY, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::xscale_getset;
    o.init_property(NSV::PROP_uXSCALE, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::yscale_getset;
    o.init_property(NSV::PROP_uYSCALE, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::xmouse_get;
    o.init_readonly_property(NSV::PROP_uXMOUSE, *gettersetter);

    gettersetter = &DisplayObject::ymouse_get;
    o.init_readonly_property(NSV::PROP_uYMOUSE, *gettersetter);

    gettersetter = &DisplayObject::alpha_getset;
    o.init_property(NSV::PROP_uALPHA, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::visible_getset;
    o.init_property(NSV::PROP_uVISIBLE, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::width_getset;
    o.init_property(NSV::PROP_uWIDTH, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::height_getset;
    o.init_property(NSV::PROP_uHEIGHT, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::rotation_getset;
    o.init_property(NSV::PROP_uROTATION, *gettersetter, *gettersetter);

    gettersetter = &DisplayObject::parent_getset;
    o.init_property(NSV::PROP_uPARENT, *gettersetter, *gettersetter);
    
    gettersetter = &DisplayObject::target_getset;
    o.init_property(NSV::PROP_uTARGET, *gettersetter, *gettersetter);

    gettersetter = DisplayObject::name_getset;
    o.init_property(NSV::PROP_uNAME, gettersetter, gettersetter);
    
    const int unprotected = 0;
    o.init_member(NSV::PROP_ENABLED, true, unprotected);

}

Button::Button(const SWF::DefineButtonTag* const def, DisplayObject* parent,
        int id)
    :
    InteractiveObject(parent, id),
    _lastMouseFlags(FLAG_IDLE),
    _mouseFlags(FLAG_IDLE),
    _mouseState(MOUSESTATE_UP),
    _def(def)
{

    set_prototype(getButtonInterface());

    // check up presence Key events
    if (_def->hasKeyPressHandler()) {
        getRoot(*this).add_key_listener(this);
    }

}

Button::~Button()
{
    getRoot(*this).remove_key_listener(this);
}

bool
Button::trackAsMenu()
{
    // TODO: check whether the AS or the tag value takes precedence.
    as_value track;
    string_table& st = getStringTable(*this);
    if (get_member(st.find("trackAsMenu"), &track)) {
        return track.to_bool();
    }
    if (_def) return _def->trackAsMenu();
    return false;
}

bool 
Button::isEnabled()
{
    as_value enabled;
    if (!get_member(NSV::PROP_ENABLED, &enabled)) return false;

    return enabled.to_bool();
}


bool
Button::on_event(const event_id& id)
{
    if (unloaded())
    {
        // We dont' respond to events while unloaded
        // See bug #22982
#if 0 // debugging..
        log_debug("Button %s received %s event while unloaded: ignored",
            getTarget(), id);
#endif
        return false; 
    }

    // We only respond keypress events
    if ( id.id() != event_id::KEY_PRESS ) return false;

    // We only respond to valid key code (should we assert here?)
    if ( id.keyCode() == key::INVALID ) return false;

    ButtonActionPusher xec(getRoot(*this), this); 
    _def->forEachTrigger(id, xec);

    return xec.called;
}

bool
Button::handleFocus() {
    /// Nothing to do, but can receive focus.
    return false;
}


void
Button::display(Renderer& renderer)
{

    DisplayObjects actChars;
    getActiveCharacters(actChars);

    // TODO: by keeping chars sorted by depth we'd avoid the sort on display
    std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

    for (DisplayObjects::iterator it = actChars.begin(), e = actChars.end();
            it != e; ++it) {
        (*it)->display(renderer);
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

        SWFMatrix  m = getMatrix();
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

    // point is in parent's space,
    // we need to convert it in world space
    point  wp(x,y);
    DisplayObject* parent = get_parent();
    if ( parent )
    {
        parent->getWorldMatrix().transform(wp);
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
    if ( unloaded() )
    {
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
        sound::sound_handler* s = getRunResources(*this).soundHandler();
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

        // DisplayObject zero is considered as null DisplayObject
        if (!bs.soundID) break;

        // No actual sound ?
        if (!bs.sample) break;

        if (bs.soundInfo.stopPlayback)
        {
            s->stop_sound(bs.sample->m_sound_handler_id);
        }
        else
        {
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

    movie_root& mr = getRoot(*this);

    ButtonActionPusher xec(mr, this); 
    _def->forEachTrigger(event, xec);

    // check for built-in event handler.
    std::auto_ptr<ExecutableCode> code ( get_event_handler(event) );
    if ( code.get() )
    {
        //log_debug(_("Got statically-defined handler for event: %s"), event);
        mr.pushAction(code, movie_root::apDOACTION);
    }

    // Call conventional attached method.
    boost::intrusive_ptr<as_function> method =
        getUserDefinedEventHandler(event.functionKey());
    if (method) {
        mr.pushAction(method, this, movie_root::apDOACTION);
    }
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
                ch->stagePlacementCallback(); 
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
                boost::ref(ranges), force||m_invalidated)
    );
}

rect
Button::getBounds() const
{
    rect allBounds;

    typedef std::vector<const DisplayObject*> Chars;
    Chars actChars;
    getActiveCharacters(actChars);
    for(Chars::const_iterator i=actChars.begin(),e=actChars.end(); i!=e; ++i)
    {
        const DisplayObject* ch = *i;
        // Child bounds need be transformed in our coordinate space
        rect lclBounds = ch->getBounds();
        SWFMatrix m = ch->getMatrix();
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

as_object*
Button::get_path_element(string_table::key key)
{
    as_object* ch = getPathElementSeparator(key);
    if ( ch ) return ch;

    const std::string& name = getStringTable(*this).value(key);
    return getChildByName(name); // possibly NULL
}

DisplayObject *
Button::getChildByName(const std::string& name)
{
    // Get all currently active DisplayObjects, including unloaded
    DisplayObjects actChars;
    getActiveCharacters(actChars, true);

    // Lower depth first for duplicated names, so we sort
    std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

    for (DisplayObjects::iterator i=actChars.begin(), e=actChars.end(); i!=e; ++i)
    {

        DisplayObject* const child = *i;
        const std::string& childname = child->get_name();
 
        if (getSWFVersion(*this) >= 7 )
        {
            if ( childname == name ) return child;
        }
        else
        {
            StringNoCaseEqual noCaseCompare;
            if ( noCaseCompare(childname, name) ) return child;
        }
    }

    return NULL;
}

void
Button::stagePlacementCallback(as_object* initObj)
{

    // Not sure how this can happen, but blip.tv does it.
    if (initObj) {
        log_unimpl("Button placed with an initObj. How did this happen? "
                "We'll copy the properties anyway");
        copyProperties(*initObj);
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
        ch->stagePlacementCallback(); // give this DisplayObject a life
    }

    // There is no INITIALIZE/CONSTRUCT/LOAD/ENTERFRAME/UNLOAD event 
    // for Buttons
}

#ifdef GNASH_USE_GC
void
Button::markReachableResources() const
{
    assert(isReachable());

    _def->setReachable();

    // Mark state DisplayObjects as reachable
    for (DisplayObjects::const_iterator i=_stateCharacters.begin(), e=_stateCharacters.end();
            i!=e; ++i)
    {
        DisplayObject* ch = *i;
        if ( ch ) ch->setReachable();
    }

    // Mark hit DisplayObjects as reachable
    for (DisplayObjects::const_iterator i = _hitCharacters.begin(),
            e=_hitCharacters.end(); i != e; ++i)
    {
        DisplayObject* ch = *i;
        assert ( ch );
        ch->setReachable();
    }

    // DisplayObject class members
    markDisplayObjectReachable();
}
#endif // GNASH_USE_GC

bool
Button::unload()
{

    bool childsHaveUnload = false;

    // We need to unload all children, or the global instance list
    // will keep growing forever !
    for (DisplayObjects::iterator i = _stateCharacters.begin(),
            e = _stateCharacters.end(); i != e; ++i)
    {
        DisplayObject* ch = *i;
        if ( ! ch ) continue;
        if ( ch->unloaded() ) continue;
        if ( ch->unload() ) childsHaveUnload = true;
    }

    // NOTE: we don't need to ::unload or ::destroy here
    //       as the _hitCharacters are never placed on stage.
    //       As an optimization we might not even instantiate
    //       them, and only use the definition and the 
    //       associated transform SWFMatrix... (would take
    //       hit instance off the GC).
    _hitCharacters.clear();

    bool hasUnloadEvent = DisplayObject::unload();

    return hasUnloadEvent || childsHaveUnload;
}

void
Button::destroy()
{

    for (DisplayObjects::iterator i = _stateCharacters.begin(),
            e=_stateCharacters.end(); i != e; ++i)
    {
        DisplayObject* ch = *i;
        if ( ! ch ) continue;
        if ( ch->isDestroyed() ) continue;
        ch->destroy();
        *i = 0;
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

bool
Button::get_member(string_table::key name_key, as_value* val,
    string_table::key nsname)
{
    // FIXME: use addProperty interface for these !!
    // TODO: or at least have a DisplayObject:: protected method take
    //       care of these ?
    //       Duplicates code in DisplayObject::getPathElementSeparator too..
    //
    if (name_key == NSV::PROP_uROOT) {
        // getAsRoot() will take care of _lockroot
        val->set_as_object(getAsRoot());
        return true;
    }

    // NOTE: availability of _global doesn't depend on VM version
    //             but on actual movie version. Example: if an SWF4 loads
    //             an SWF6 (to, say, _level2), _global will be unavailable
    //             to the SWF4 code but available to the SWF6 one.
    //
    // see MovieClip.as
    if (getMovieVersion() > 5 && name_key == NSV::PROP_uGLOBAL ) {
        // The "_global" ref was added in SWF6
        val->set_as_object(getGlobal(*this));
        return true;
    }

    const std::string& name = getStringTable(*this).value(name_key);

    movie_root& mr = getRoot(*this);
    unsigned int levelno;
    if ( mr.isLevelTarget(name, levelno) ) {
        Movie* mo = mr.getLevel(levelno).get();
        if ( mo ) {
            val->set_as_object(mo);
            return true;
        }
        else {
            return false;
        }
    }

    // TOCHECK : Try object members, BEFORE display list items
    //
    if (as_object::get_member(name_key, val, nsname))
    {

    // ... trying to be useful to Flash coders ...
    // The check should actually be performed before any return
    // prior to the one due to a match in the DisplayList.
    // It's off by default anyway, so not a big deal.
    // See bug #18457
#define CHECK_FOR_NAME_CLASHES 1
#ifdef CHECK_FOR_NAME_CLASHES
        IF_VERBOSE_ASCODING_ERRORS(
        if ( getChildByName(name) )
        {
            log_aserror(_("A button member (%s) clashes with "
                    "the name of an existing DisplayObject "
                    "in its display list.    "
                    "The member will hide the "
                    "DisplayObject"), name);
        }
        );
#endif

        return true;
    }

    // Try items on our display list.
    DisplayObject* ch = getChildByName(name);

    if (ch) {
        // Found object.

        // If the object is an ActionScript referenciable one we
        // return it, otherwise we return ourselves
        if ( ch->isActionScriptReferenceable() ) {
            val->set_as_object(ch);
        }
        else {
            val->set_as_object(this);
        }

        return true;
    }

    return false;

}

int
Button::getMovieVersion() const
{
    return _def->getSWFVersion();
}

static as_object*
getButtonInterface()
{
  static boost::intrusive_ptr<as_object> proto;
  if ( proto == NULL )
  {
    proto = new as_object(getObjectInterface());
    VM::get().addStatic(proto.get());

    attachButtonInterface(*proto);
  }
  return proto.get();
}

static as_value
button_ctor(const fn_call& /* fn */)
{
  boost::intrusive_ptr<as_object> clip = new as_object(getButtonInterface());
  return as_value(clip.get());
}

void
Button::init(as_object& global, const ObjectURI& uri)
{
  // This is going to be the global Button "class"/"function"
  static boost::intrusive_ptr<as_object> cl=NULL;

  if ( cl == NULL )
  {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&button_ctor, getButtonInterface());
    VM::get().addStatic(cl.get());
  }

  // Register _global.MovieClip
  global.init_member("Button", cl.get());
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
            StringPair(_("Button state"), os.str()));

    os.str("");
    os << std::boolalpha << isEnabled();
    localIter = tr.append_child(selfIt, StringPair(_("Enabled"), os.str()));

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
        default: return "UNKNOWN (error?)";
    }
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
