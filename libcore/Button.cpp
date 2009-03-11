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

#include "ActionExec.h"
#include "MovieClip.h"
#include "movie_root.h"
#include "VM.h"
#include "builtin_function.h"
#include "fn_call.h" // for shared ActionScript getter-setters
#include "GnashException.h" // for shared ActionScript getter-setters
#include "ExecutableCode.h"
#include "namedStrings.h"
#include "Object.h" // for getObjectInterface
#include "StringPredicates.h"
#include "GnashKey.h" // key::code
#include "SoundInfoRecord.h" // for use

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
    ButtonActionPusher(movie_root& mr, character* this_ptr)
        :
        called(false),
        _mr(mr),
        _tp(this_ptr)
    {}

    void operator() (const action_buffer& ab)
    {
        _mr.pushAction(ab, boost::intrusive_ptr<character>(_tp));
        called = true;
    }

    bool called;

private:
    movie_root& _mr;
    character* _tp;
};

}

// Forward declarations
static as_object* getButtonInterface();

/// Predicates for standard algorithms.

/// Depth comparator for characters.
static bool charDepthLessThen(const character* ch1, const character* ch2) 
{
    return ch1->get_depth() < ch2->get_depth();
}

/// Predicate for finding active characters.
//
/// Returns true if the character should be skipped:
/// 1) if it is NULL, or 
/// 2) if we don't want unloaded characters and the character is unloaded.
static bool isCharacterNull(character* ch, bool includeUnloaded)
{
    return (!ch || (!includeUnloaded && ch->isUnloaded()));
}

static void
attachButtonInterface(as_object& o)
{

    as_c_function_ptr gettersetter;

    o.init_property(NSV::PROP_uQUALITY, character::quality,
            character::quality);
    
    o.init_property(NSV::PROP_uHIGHQUALITY, character::highquality,
            character::highquality);

    gettersetter = &character::x_getset;
    o.init_property(NSV::PROP_uX, *gettersetter, *gettersetter);

    gettersetter = &character::y_getset;
    o.init_property(NSV::PROP_uY, *gettersetter, *gettersetter);

    gettersetter = &character::xscale_getset;
    o.init_property(NSV::PROP_uXSCALE, *gettersetter, *gettersetter);

    gettersetter = &character::yscale_getset;
    o.init_property(NSV::PROP_uYSCALE, *gettersetter, *gettersetter);

    gettersetter = &character::xmouse_get;
    o.init_readonly_property(NSV::PROP_uXMOUSE, *gettersetter);

    gettersetter = &character::ymouse_get;
    o.init_readonly_property(NSV::PROP_uYMOUSE, *gettersetter);

    gettersetter = &character::alpha_getset;
    o.init_property(NSV::PROP_uALPHA, *gettersetter, *gettersetter);

    gettersetter = &character::visible_getset;
    o.init_property(NSV::PROP_uVISIBLE, *gettersetter, *gettersetter);

    gettersetter = &character::width_getset;
    o.init_property(NSV::PROP_uWIDTH, *gettersetter, *gettersetter);

    gettersetter = &character::height_getset;
    o.init_property(NSV::PROP_uHEIGHT, *gettersetter, *gettersetter);

    gettersetter = &character::rotation_getset;
    o.init_property(NSV::PROP_uROTATION, *gettersetter, *gettersetter);

    gettersetter = &character::parent_getset;
    o.init_property(NSV::PROP_uPARENT, *gettersetter, *gettersetter);
    
    gettersetter = &character::target_getset;
    o.init_property(NSV::PROP_uTARGET, *gettersetter, *gettersetter);

    gettersetter = character::name_getset;
    o.init_property(NSV::PROP_uNAME, gettersetter, gettersetter);
    
    const int unprotected = 0;
    o.init_member(NSV::PROP_ENABLED, true, unprotected);

}

Button::Button(SWF::DefineButtonTag& def, character* parent, int id)
    :
    character(parent, id),
    m_last_mouse_flags(IDLE),
    m_mouse_flags(IDLE),
    m_mouse_state(UP),
    _def(def)
{

    set_prototype(getButtonInterface());

    // check up presence Key events
    if ( _def.hasKeyPressHandler() )
    {
        _vm.getRoot().add_key_listener(this);
    }

}

Button::~Button()
{
    _vm.getRoot().remove_key_listener(this);
}


bool 
Button::isEnabled()
{
    as_value enabled;
    if (!get_member(NSV::PROP_ENABLED, &enabled)) return false;

    return enabled.to_bool();
}


// called from Key listener only
// (the above line is wrong, it's also called with onConstruct, for instance)
bool
Button::on_event(const event_id& id)
{
    if ( isUnloaded() )
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

    ButtonActionPusher xec(getVM().getRoot(), this); 
    _def.forEachTrigger(id, xec);

    return xec.called;
}

bool
Button::handleFocus() {
    /// Nothing to do, but can receive focus.
    return false;
}


void
Button::restart()
{
    log_error("Button::restart called, from whom??");
}

void
Button::display()
{

    std::vector<character*> actChars;
    getActiveCharacters(actChars);

    // TODO: by keeping chars sorted by depth we'd avoid the sort on display
    std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

    std::for_each(actChars.begin(), actChars.end(),
            std::mem_fun(&character::display)); 

    clear_invalidated();
}


character*
Button::get_topmost_mouse_entity(boost::int32_t x, boost::int32_t y)
// Return the topmost entity that the given point covers.  NULL if none.
// I.e. check against ourself.
{
    if (!isVisible() || !isEnabled())
    {
        return 0;
    }

    //-------------------------------------------------
    // Check our active and visible children first
    //-------------------------------------------------

    typedef std::vector<character*> Chars;
    Chars actChars;
    getActiveCharacters(actChars);

    if ( ! actChars.empty() )
    {
        std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

        SWFMatrix  m = getMatrix();
        point  p(x, y);
        m.invert().transform(p);

        for (Chars::reverse_iterator it=actChars.rbegin(), itE=actChars.rend();
                it!=itE; ++it)
        {
            character* ch = *it;
            if ( ! ch->isVisible() ) continue;
            character *hit = ch->get_topmost_mouse_entity(p.x, p.y);
            if ( hit ) return hit;
        }
    }

    //-------------------------------------------------
    // If that failed, check our hit area
    //-------------------------------------------------

    // Find hit characters
    if ( _hitCharacters.empty() ) return 0;

    // point is in parent's space,
    // we need to convert it in world space
    point  wp(x,y);
    character* parent = get_parent();
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
Button::on_button_event(const event_id& event)
{
    if ( isUnloaded() )
    {
        // We don't respond to events while unloaded. See bug #22982.
        log_debug("Button %s received %s button event while unloaded: ignored",
            getTarget(), event);
        return;
    }

    MouseState new_state = m_mouse_state;
  
    // Set our mouse state (so we know how to render).
    switch (event.id())
    {
        case event_id::ROLL_OUT:
        case event_id::RELEASE_OUTSIDE:
            new_state = UP;
            break;

        case event_id::RELEASE:
        case event_id::ROLL_OVER:
        case event_id::DRAG_OUT:
        case event_id::MOUSE_UP:
            new_state = OVER;
            break;

        case event_id::PRESS:
        case event_id::DRAG_OVER:
        case event_id::MOUSE_DOWN:
            new_state = DOWN;
            break;

        default:
            //abort();  // missed a case?
            log_error(_("Unhandled button event %s"), event);
            break;
    }
    
    set_current_state(new_state);
    
    // Button transition sounds.
    do {

        if (!_def.hasSound()) break;

        // Check if there is a sound handler
        sound::sound_handler* s = _vm.getRoot().runInfo().soundHandler();
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
            _def.buttonSound(bi);

        // character zero is considered as null character
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

            s->playSound(bs.sample->m_sound_handler_id,
                    bs.soundInfo.loopCount,
                    0, // secs offset
                    0, // byte offset
                    env, // envelopes
                    !sinfo.noMultiple // allow multiple instances ?
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

    movie_root& mr = getVM().getRoot();

    ButtonActionPusher xec(mr, this); 
    _def.forEachTrigger(event, xec);

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
    if ( method )
    {
        mr.pushAction(method, this, movie_root::apDOACTION);
    }
}


void
Button::getActiveCharacters(
        std::vector<const character*>& list) const
{
    list.clear();

    // Copy all the characters to the new list, skipping NULL and unloaded
    // characters.
    std::remove_copy_if(_stateCharacters.begin(), _stateCharacters.end(),
            std::back_inserter(list),
            boost::bind(&isCharacterNull, _1, false));

}


void 
Button::getActiveCharacters(
        std::vector<character*>& list, bool includeUnloaded)
{
    list.clear();

    // Copy all the characters to the new list, skipping NULL
    // characters, optionally including unloaded characters.
    std::remove_copy_if(_stateCharacters.begin(), _stateCharacters.end(),
            std::back_inserter(list),
            boost::bind(&isCharacterNull, _1, includeUnloaded));
    
}

void 
Button::get_active_records(ActiveRecords& list, MouseState state)
{
    list.clear();
    
    using namespace SWF;
    const DefineButtonTag::ButtonRecords& br = _def.buttonRecords();
    size_t index = 0;

    for (DefineButtonTag::ButtonRecords::const_iterator i = br.begin(),
            e = br.end(); i != e; ++i, ++index)
    {
        const ButtonRecord& rec =*i;

        if ((state == UP && rec.m_up)
            || (state == DOWN && rec.m_down)
            || (state == OVER && rec.m_over)
            || (state == HIT && rec.m_hit_test))
        {
            list.insert(index);
        }
    }
}

#ifdef GNASH_DEBUG_BUTTON_DISPLAYLIST
static void dump(std::vector< character* >& chars, std::stringstream& ss)
{
    for (size_t i=0, e=chars.size(); i<e; ++i)
    {
        ss << "Record" << i << ": ";
        character* ch = chars[i];
        if ( ! ch ) ss << "NULL.";
        else
        {
            ss << ch->getTarget() << " (depth:" << 
                ch->get_depth()-character::staticDepthOffset-1
                << " unloaded:" << ch->isUnloaded() <<
                " destroyed:" << ch->isDestroyed() << ")";
        }
        ss << std::endl;
    }
}
#endif

void
Button::set_current_state(MouseState new_state)
{
    if (new_state == m_mouse_state)
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
        character* oldch = _stateCharacters[i];
        bool shouldBeThere = ( newChars.find(i) != newChars.end() );

        if ( ! shouldBeThere )
        {
            // is there, but is unloaded: destroy, clear slot and go on
            if ( oldch && oldch->isUnloaded() )
            {
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
                    if ( ! oldch->isDestroyed() ) oldch->destroy();
                    _stateCharacters[i] = NULL;
                }
                else
                {
                    // onUnload handler: shift depth and keep slot
                    int oldDepth = oldch->get_depth();
                    int newDepth = character::removedDepthOffset - oldDepth;
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
            if ( oldch && oldch->isUnloaded() )
            {
                if ( ! oldch->isDestroyed() ) oldch->destroy();
                _stateCharacters[i] = NULL;
                oldch = NULL;
            }

            if ( ! oldch )
            {
                // Not there, instantiate
                SWF::ButtonRecord& bdef = _def.buttonRecords()[i];

                const SWFMatrix& mat = bdef.m_button_matrix;
                const cxform& cx = bdef.m_button_cxform;
                int ch_depth = bdef.m_button_layer + 
                    character::staticDepthOffset + 1;
                int ch_id = bdef.m_character_id;

                character* ch = bdef.m_character_def->createDisplayObject(
                        this, ch_id);
                ch->setMatrix(mat, true); // update caches
                ch->set_cxform(cx); 
                ch->set_depth(ch_depth); 
                assert(ch->get_parent() == this);

                // no way to specify a name for button chars anyway...
                assert(ch->get_name().empty()); 

                if ( ch->wantsInstanceName() )
                {
                    //std::string instance_name = getNextUnnamedInstanceName();
                    ch->set_name(getNextUnnamedInstanceName());
                }

                set_invalidated();

                _stateCharacters[i] = ch;
                ch->stagePlacementCallback(); // give this character a life

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
    m_mouse_state=new_state;
     
}

//
// ActionScript overrides
//



void 
Button::add_invalidated_bounds(InvalidatedRanges& ranges, 
    bool force)
{

    // Not visible anyway
    if (!isVisible()) return;

    ranges.add(m_old_invalidated_ranges);  

    std::vector<character*> actChars;
    getActiveCharacters(actChars);
    std::for_each(actChars.begin(), actChars.end(),
            boost::bind(&character::add_invalidated_bounds, _1,
                boost::ref(ranges), force||m_invalidated)
    );
}

rect
Button::getBounds() const
{
    rect allBounds;

    typedef std::vector<const character*> CharVect;
    CharVect actChars;
    getActiveCharacters(actChars);
    for(CharVect::const_iterator i=actChars.begin(),e=actChars.end(); i!=e; ++i)
    {
        const character* ch = *i;
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
    typedef std::vector<const character*> CharVect;
    CharVect actChars;
    getActiveCharacters(actChars);
    for(CharVect::const_iterator i=actChars.begin(),e=actChars.end(); i!=e; ++i)
    {
        const character* ch = *i;
        if ( ch->pointInShape(x,y) ) return true;
    }
    return false; 
}

as_object*
Button::get_path_element(string_table::key key)
{
    as_object* ch = get_path_element_character(key);
    if ( ch ) return ch;

    const std::string& name = _vm.getStringTable().value(key);
    return getChildByName(name); // possibly NULL
}

character *
Button::getChildByName(const std::string& name)
{
    // Get all currently active characters, including unloaded
    DisplayObjects actChars;
    getActiveCharacters(actChars, true);

    // Lower depth first for duplicated names, so we sort
    std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

    for (DisplayObjects::iterator i=actChars.begin(), e=actChars.end(); i!=e; ++i)
    {

        character* const child = *i;
        const std::string& childname = child->get_name();
 
        if ( _vm.getSWFVersion() >= 7 )
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

    // Don't register this button instance as a live character.

    // Instantiate the hit characters
    ActiveRecords hitChars;
    get_active_records(hitChars, HIT);
    for (ActiveRecords::iterator i=hitChars.begin(),e=hitChars.end(); i!=e; ++i)
    {
        SWF::ButtonRecord& bdef = _def.buttonRecords()[*i];

        const SWFMatrix& mat = bdef.m_button_matrix;
        const cxform& cx = bdef.m_button_cxform;
        int ch_depth = bdef.m_button_layer+character::staticDepthOffset+1;
        int ch_id = bdef.m_character_id;

        character* ch =
            bdef.m_character_def->createDisplayObject(this, ch_id);
        ch->setMatrix(mat, true);  // update caches
    
        // TODO: who cares about color, depth etc.
        ch->set_cxform(cx); 
        ch->set_depth(ch_depth);
        assert(ch->get_parent() == this);
        assert(ch->get_name().empty()); 

        _hitCharacters.push_back(ch);
    }

    // Setup the state characters container
    // It will have a slot for each character record.
    // Some slots will probably be never used (consider HIT-only records)
    // but for now this direct corrispondence between record number
    // and active character will be handy.
    _stateCharacters.resize(_def.buttonRecords().size());

    // Instantiate the default state characters 
    ActiveRecords upChars;
    get_active_records(upChars, UP);

    for (ActiveRecords::iterator i=upChars.begin(),e=upChars.end(); i!=e; ++i)
    {
        int rno = *i;
        SWF::ButtonRecord& bdef = _def.buttonRecords()[rno];

        const SWFMatrix& mat = bdef.m_button_matrix;
        const cxform& cx = bdef.m_button_cxform;
        int ch_depth = bdef.m_button_layer+character::staticDepthOffset+1;
        int ch_id = bdef.m_character_id;

        character* ch = bdef.m_character_def->createDisplayObject(
                this, ch_id);
        ch->setMatrix(mat, true);  // update caches
        ch->set_cxform(cx); 
        ch->set_depth(ch_depth); 
        assert(ch->get_parent() == this);
        assert(ch->get_name().empty()); 

        if ( ch->wantsInstanceName() )
        {
            //std::string instance_name = getNextUnnamedInstanceName();
            ch->set_name(getNextUnnamedInstanceName());
        }

        _stateCharacters[rno] = ch;
        ch->stagePlacementCallback(); // give this character a life
    }

    // There is no INITIALIZE/CONSTRUCT/LOAD/ENTERFRAME/UNLOAD event 
    // for Buttons
}

#ifdef GNASH_USE_GC
void
Button::markReachableResources() const
{
    assert(isReachable());

    _def.setReachable();

    // Mark state characters as reachable
    for (DisplayObjects::const_iterator i=_stateCharacters.begin(), e=_stateCharacters.end();
            i!=e; ++i)
    {
        character* ch = *i;
        if ( ch ) ch->setReachable();
    }

    // Mark hit characters as reachable
    for (DisplayObjects::const_iterator i = _hitCharacters.begin(),
            e=_hitCharacters.end(); i != e; ++i)
    {
        character* ch = *i;
        assert ( ch );
        ch->setReachable();
    }

    // character class members
    markCharacterReachable();
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
        character* ch = *i;
        if ( ! ch ) continue;
        if ( ch->isUnloaded() ) continue;
        if ( ch->unload() ) childsHaveUnload = true;
    }

    // NOTE: we don't need to ::unload or ::destroy here
    //       as the _hitCharacters are never placed on stage.
    //       As an optimization we might not even instantiate
    //       them, and only use the definition and the 
    //       associated transform SWFMatrix... (would take
    //       hit instance off the GC).
    _hitCharacters.clear();

    bool hasUnloadEvent = character::unload();

    return hasUnloadEvent || childsHaveUnload;
}

void
Button::destroy()
{

    for (DisplayObjects::iterator i = _stateCharacters.begin(),
            e=_stateCharacters.end(); i != e; ++i)
    {
        character* ch = *i;
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

    character::destroy();
}

bool
Button::get_member(string_table::key name_key, as_value* val,
    string_table::key nsname)
{
    // FIXME: use addProperty interface for these !!
    // TODO: or at least have a character:: protected method take
    //       care of these ?
    //       Duplicates code in character::get_path_element_character too..
    //
    if (name_key == NSV::PROP_uROOT) {
        // getAsRoot() will take care of _lockroot
        val->set_as_object( const_cast<MovieClip*>( getAsRoot() )    );
        return true;
    }

    // NOTE: availability of _global doesn't depend on VM version
    //             but on actual movie version. Example: if an SWF4 loads
    //             an SWF6 (to, say, _level2), _global will be unavailable
    //             to the SWF4 code but available to the SWF6 one.
    //
    // see MovieClip.as
    if ( getSWFVersion() > 5 && name_key == NSV::PROP_uGLOBAL ) {
        // The "_global" ref was added in SWF6
        val->set_as_object( _vm.getGlobal() );
        return true;
    }

    const std::string& name = _vm.getStringTable().value(name_key);

    movie_root& mr = _vm.getRoot();
    unsigned int levelno;
    if ( mr.isLevelTarget(name, levelno) ) {
        movie_instance* mo = mr.getLevel(levelno).get();
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
                    "the name of an existing character "
                    "in its display list.    "
                    "The member will hide the "
                    "character"), name);
        }
        );
#endif

        return true;
    }

    // Try items on our display list.
    character* ch = getChildByName(name);

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
Button::getSWFVersion() const
{
    return _def.getSWFVersion();
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
Button::init(as_object& global)
{
  // This is going to be the global Button "class"/"function"
  static boost::intrusive_ptr<builtin_function> cl=NULL;

  if ( cl == NULL )
  {
    cl=new builtin_function(&button_ctor, getButtonInterface());
    VM::get().addStatic(cl.get());
  }

  // Register _global.MovieClip
  global.init_member("Button", cl.get());
}

#ifdef USE_SWFTREE
character::InfoTree::iterator 
Button::getMovieInfo(InfoTree& tr, InfoTree::iterator it)
{
    InfoTree::iterator selfIt = character::getMovieInfo(tr, it);
    std::ostringstream os;

    std::vector<character*> actChars;
    getActiveCharacters(actChars, true);
    std::sort(actChars.begin(), actChars.end(), charDepthLessThen);

    os << actChars.size() << " active characters for state " << mouseStateName(m_mouse_state);
    InfoTree::iterator localIter = tr.append_child(selfIt, StringPair(_("Button state"), os.str()));        
    std::for_each(actChars.begin(), actChars.end(), boost::bind(&character::getMovieInfo, _1, tr, localIter)); 

    return selfIt;

}
#endif

const char*
Button::mouseStateName(MouseState s)
{
    switch (s)
    {
        case UP: return "UP";
        case DOWN: return "DOWN";
        case OVER: return "OVER";
        case HIT: return "HIT";
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
