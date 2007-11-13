// Key.cpp:  ActionScript "Key" class (keyboards), for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Key.h"
#include "fn_call.h"
#include "movie_root.h"
#include "action.h" // for call_method
#include "VM.h"
#include "builtin_function.h"
#include "Object.h" // for getObjectInterface()

#include <boost/algorithm/string/case_conv.hpp>

namespace gnash {

/************************************************************************
 *
 * This has been moved from action.cpp, when things are clean
 * everything should have been moved up
 *
 ************************************************************************/

key_as_object::key_as_object()
    :
    as_object(getObjectInterface()),
    m_last_key_event(0)
{
    memset(m_unreleased_keys, 0, sizeof(m_unreleased_keys));
}

bool
key_as_object::is_key_down(int keycode)
{
    if (keycode < 0 || keycode >= key::KEYCOUNT) return false;

    // Select the relevant byte of the bit array:
    int byte_index = keycode >> 3;
    // Find bit within the byte:
    int bit_index = keycode - (byte_index << 3);
    
    uint8_t mask = 1 << bit_index;
    
    if ((m_unreleased_keys[byte_index] & mask) != 0 ) return true;

    return false;
}

void
key_as_object::set_key_down(int code)
{
    if (code < 0 || code >= key::KEYCOUNT) return;

    // This is used for getAscii() of the last key event, so we use gnash's
    // internal code.
    m_last_key_event = code;

    // Key.isDown() only cares about flash keycode, not character, so
    // we lookup keycode to add to m_unreleased_keys.   
    
    int byte_index = key::codeMap[code][1] >> 3;
    int bit_index = key::codeMap[code][1] - (byte_index << 3);
    int mask = 1 << bit_index;

    assert(byte_index >= 0 && byte_index < int(sizeof(m_unreleased_keys)/sizeof(m_unreleased_keys[0])));

    m_unreleased_keys[byte_index] |= mask;
}

void
key_as_object::set_key_up(int code)
{
    if (code < 0 || code >= key::KEYCOUNT) return;

    // This is used for getAscii() of the last key event, so we use gnash's
    // internal code.    
    m_last_key_event = code;
        
    // Key.isDown() only cares about flash keycode, not character, so
    // we lookup keycode to add to m_unreleased_keys.
    int byte_index = key::codeMap[code][1] >> 3;
    int bit_index = key::codeMap[code][1] - (byte_index << 3);
    int mask = 1 << bit_index;

    assert(byte_index >= 0 && byte_index < int(sizeof(m_unreleased_keys)/sizeof(m_unreleased_keys[0])));

    m_unreleased_keys[byte_index] &= ~mask;
}

#ifndef NEW_KEY_LISTENER_LIST_DESIGN
void 
key_as_object::notify_listeners(const event_id key_event_type)
{
    
    std::string funcname = key_event_type.get_function_name();
    // There is no user defined "onKeyPress" event handler
    if( ( funcname != "onKeyDown") && (funcname != "onKeyUp") )
        return;

    VM& vm = VM::get();
    if ( vm.getSWFVersion() < 7 )
    {
        boost::to_lower(funcname, vm.getLocale());
    }

    // Notify listeners.
    for (std::vector<boost::intrusive_ptr<as_object> >::iterator iter = m_listeners.begin();
         iter != m_listeners.end(); ++iter) {
      if (*iter == NULL)
        continue;

      boost::intrusive_ptr<as_object>  listener = *iter; // Hold an owning reference.
      as_value method;

       if (listener->get_member(VM::get().getStringTable().find(funcname), &method))
        call_method(method, NULL /* or root? */, listener.get(), 0, 0);
    }
}
#endif // ndef NEW_KEY_LISTENER_LIST_DESIGN

#ifdef NEW_KEY_LISTENER_LIST_DESIGN
void
key_as_object::add_listener(const KeyListener& listener)
{
    _vm.getRoot().add_key_listener(listener);
}

void
key_as_object::remove_listener(boost::intrusive_ptr<as_object> listener)
{
    // Should keep consistent with definiton in movie_root.h
    typedef std::set<KeyListener> KeyListeners;

    KeyListeners & listeners = _vm.getRoot().getKeyListeners();
    
    KeyListeners::iterator target = listeners.find(KeyListener(listener));

	KeyListeners::iterator it_end = listeners.end();

    if(target != it_end)
    {
        target->unregisterUserHandler();
    }
}

#else // ndef NEW_KEY_LISTENER_LIST_DESIGN

void
key_as_object::add_listener(boost::intrusive_ptr<as_object> listener)
{
    // Should we bother doing this every time someone calls add_listener(),
    // or should we perhaps skip this check and use unique later?
    std::vector<boost::intrusive_ptr<as_object> >::const_iterator end = m_listeners.end();
    for (std::vector<boost::intrusive_ptr<as_object> >::iterator iter = m_listeners.begin();
         iter != end; ++iter) {
      if (*iter == NULL) {
        // Already in the list.
        return;
      }
    }

    m_listeners.push_back(listener);
}

void
key_as_object::remove_listener(boost::intrusive_ptr<as_object> listener)
{

    for (std::vector<boost::intrusive_ptr<as_object> >::iterator iter = m_listeners.begin(); iter != m_listeners.end(); )
    {
        if (*iter == listener)
        {
            iter = m_listeners.erase(iter);
            continue;
        }
        iter++;
    }
}
#endif // ndef NEW_KEY_LISTENER_LIST_DESIGN

int
key_as_object::get_last_key() const
{
    return m_last_key_event;
}


as_value
key_add_listener(const fn_call& fn)
{

    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Key.addListener needs one argument (the listener object)"));
        );
        return as_value();
    }

    boost::intrusive_ptr<as_object> toadd = fn.arg(0).to_object();
    if (toadd == NULL)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Key.addListener passed a NULL object; ignored"));
        );
        return as_value();
    }

#ifdef NEW_KEY_LISTENER_LIST_DESIGN

    ko->add_listener(KeyListener(toadd, KeyListener::USER_DEF));

#else // ndef NEW_KEY_LISTENER_LIST_DESIGN

    ko->add_listener(toadd);

#endif // ndef NEW_KEY_LISTENER_LIST_DESIGN

    return as_value();
}


// Return the ascii value of the last key pressed.
/// FIXME: return the ascii number(not string) of the last pressed key!
static as_value   
key_get_ascii(const fn_call& fn)
{
    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr);

    int code = ko->get_last_key();
    
		return as_value(gnash::key::codeMap[code][2]);
}

// Returns the keycode of the last key pressed.
static as_value   
key_get_code(const fn_call& fn)
{
    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr);
		
		int code = ko->get_last_key();
		
    return as_value(key::codeMap[code][1]);
}

/// Return true if the specified (first arg keycode) key is pressed.
static as_value   
key_is_down(const fn_call& fn)
{
    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr);

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Key.isDown needs one argument (the key code)"));
        );
        return as_value();
    }

    int keycode = fn.arg(0).to_number<int>();

    return as_value(ko->is_key_down(keycode));
}

/// \brief
/// Given the keycode of NUM_LOCK or CAPSLOCK, returns true if
/// the associated state is on.
///
static as_value   
key_is_toggled(const fn_call& /* fn */)
{
    log_unimpl("Key.isToggled");
    // @@ TODO
    return as_value(false);
}

/// Remove a previously-added listener.
static as_value    
key_remove_listener(const fn_call& fn)
{

    if (fn.nargs < 1)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Key.removeListener needs one argument (the listener object)"));
        );
        return as_value();
    }

    boost::intrusive_ptr<as_object> toremove = fn.arg(0).to_object();
    if (toremove == NULL)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Key.removeListener passed a NULL object; ignored"));
        );
        return as_value();
    }

    boost::intrusive_ptr<key_as_object> ko = ensureType<key_as_object>(fn.this_ptr); 

    ko->remove_listener(toremove);

    return as_value();
}

void key_class_init(as_object& global)
{

//  GNASH_REPORT_FUNCTION;
//
    int swfversion = VM::get().getSWFVersion();

    // Create built-in key object.
    // NOTE: _global.Key *is* an object, not a constructor
    as_object*  key_obj = new key_as_object;

    // constants
#define KEY_CONST(k) key_obj->init_member(#k, key::codeMap[key::k][1])
    KEY_CONST(BACKSPACE);
    KEY_CONST(CAPSLOCK);
    KEY_CONST(CONTROL);
    KEY_CONST(DELETEKEY);
    KEY_CONST(DOWN);
    KEY_CONST(END);
    KEY_CONST(ENTER);
    KEY_CONST(ESCAPE);
    KEY_CONST(HOME);
    KEY_CONST(INSERT);
    KEY_CONST(LEFT);
    KEY_CONST(PGDN);
    KEY_CONST(PGUP);
    KEY_CONST(RIGHT);
    KEY_CONST(SHIFT);
    KEY_CONST(SPACE);
    KEY_CONST(TAB);
    KEY_CONST(UP);

    // methods
    key_obj->init_member("getAscii", new builtin_function(key_get_ascii));
    key_obj->init_member("getCode", new builtin_function(key_get_code));
    key_obj->init_member("isDown", new builtin_function(key_is_down));
    key_obj->init_member("isToggled", new builtin_function(key_is_toggled));

    // These are only for SWF6 and up
    if ( swfversion > 5 )
    {
        key_obj->init_member("addListener", new builtin_function(key_add_listener));
        key_obj->init_member("removeListener", new builtin_function(key_remove_listener));
    }

    global.init_member("Key", key_obj);
}

#ifdef GNASH_USE_GC
#ifndef NEW_KEY_LISTENER_LIST_DESIGN
void
key_as_object::markReachableResources() const
{
    for (Listeners::const_iterator i=m_listeners.begin(), e=m_listeners.end();
            i != e; ++i)
    {
        (*i)->setReachable();
    }
}
#endif // ndef NEW_KEY_LISTENER_LIST_DESIGN
#endif // def GNASH_USE_GC

} // end of gnash namespace

