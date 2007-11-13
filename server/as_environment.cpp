// as_environment.cpp:  Variable, Sprite, and Movie locators, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: as_environment.cpp,v 1.107 2007/11/13 19:11:20 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smart_ptr.h"
#include "as_environment.h"
#include "sprite_instance.h"
#include "shape_character_def.h"
#include "as_value.h"
#include "with_stack_entry.h"
#include "VM.h"
#include "log.h"
#include "Property.h"
#include "as_object.h"

#include <string>
#include <utility> // for std::pair
#include <boost/algorithm/string/case_conv.hpp>

// Define this to have find_target() calls trigger debugging output
//#define DEBUG_TARGET_FINDING 1

// Define this to have get_variable() calls trigger debugging output
//#define GNASH_DEBUG_GET_VARIABLE 1

namespace gnash {

// Return the value of the given var, if it's defined.
as_value
as_environment::get_variable(const std::string& varname,
		const ScopeStack& scopeStack, as_object** retTarget) const
{
    // Path lookup rigamarole.
    std::string	path;
    std::string	var;

#ifdef GNASH_DEBUG_GET_VARIABLE
    log_debug(_("get_variable(%s)"), varname.c_str());
#endif

    bool is_slash_based;
    if (parse_path(varname, path, var, &is_slash_based))
    {
        //as_value target_val = get_variable_raw(path, scopeStack);
        //as_object* target = target_val.to_object();
        // TODO: let find_target return generic as_objects, or use 'with' stack,
        //       see player2.swf or bug #18758 (strip.swf)
        // @@ TODO: should we use scopeStack here too ?
        as_object* target = is_slash_based ? find_object_slashsyntax(path, &scopeStack) : find_object_dotsyntax(path, &scopeStack);

        if (target)
        {
            as_value	val;
            target->get_member(VM::get().getStringTable().find(var), &val);
            if ( retTarget ) *retTarget = target;
            return val;
        }
        else
        {

            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("find_object%s(\"%s\") [ varname = '%s' - current target = '%s' ] failed"),
                is_slash_based ? "_slashsyntax" : "_dotsyntax",
                path.c_str(), varname.c_str(),
                m_target->get_text_value().c_str()
            );
            );

            as_value tmp = get_variable_raw(path, scopeStack, retTarget);
            if ( ! tmp.is_undefined() )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("...but get_variable_raw(%s, <scopeStack>) succeeded (%s)!"),
                    path.c_str(), tmp.to_debug_string().c_str());
                )
            }
			return as_value();
        }
    }
    else
    {
        return get_variable_raw(varname, scopeStack, retTarget);
    }
}

as_value
as_environment::get_variable(const std::string& varname) const
{
	static ScopeStack empty_scopeStack;
	return get_variable(varname, empty_scopeStack);
}

as_value
as_environment::get_variable_raw(
    const std::string& varname,
    const ScopeStack& scopeStack, as_object** retTarget) const
    // varname must be a plain variable name; no path parsing.
{
    assert(strchr(varname.c_str(), ':') == NULL);

    as_value    val;

    VM& vm = VM::get();
    int swfVersion = vm.getSWFVersion();
    string_table& st = vm.getStringTable();
    string_table::key key = st.find(varname);

    // Check the scope stack.
    for (size_t i = scopeStack.size(); i > 0; --i)
    {
        // const_cast needed due to non-const as_object::get_member 
        as_object* obj = const_cast<as_object*>(scopeStack[i-1].get());
        if (obj && obj->get_member(key, &val))
        {
            // Found the var in with context.
            //log_debug("Found %s in object %d/%d of scope stack (%p)", varname.c_str(), i, scopeStack.size(), obj);
            if ( retTarget ) *retTarget = obj;
            return val;
        }
    }

    // Check locals for getting them
    if ( swfVersion < 6 ) // for SWF6 and up locals should be in the scope stack
    {
        if ( findLocal(varname, val, retTarget) ) 
        {
            return val;
        }
    }


    // Check current target members. TODO: shouldn't target be in scope stack ?
    if (m_target->get_member(key, &val)) {
        if ( retTarget ) *retTarget = m_target;
        return val;
    }

    // Looking for "this" 
    if (varname == "this") {
        val.set_as_object(_original_target);
        if ( retTarget ) *retTarget = NULL; // correct ??
        return val;
    }

    as_object* global = vm.getGlobal();

    if ( swfVersion > 5 && varname == "_global" )
    {
        // The "_global" ref was added in SWF6
        if ( retTarget ) *retTarget = NULL; // correct ??
        return as_value(global);
    }

    if (global->get_member(key, &val))
    {
        if ( retTarget ) *retTarget = global;
        return val;
    }
    
    // Fallback.
    // FIXME, should this be log_error?  or log_swferror?
    IF_VERBOSE_ACTION (
    log_action(_("get_variable_raw(\"%s\") failed, returning UNDEFINED"),
           varname.c_str());
    );

    return as_value();
}

bool
as_environment::del_variable_raw(
    const std::string& varname,
    const ScopeStack& scopeStack) 
    // varname must be a plain variable name; no path parsing.
{
	assert( ! strpbrk(varname.c_str(), ":/.") );

	string_table::key varkey = VM::get().getStringTable().find(varname);
	as_value	val;

	// Check the with-stack.
	for (size_t i = scopeStack.size(); i > 0; --i)
	{
		// const_cast needed due to non-const as_object::get_member 
		as_object* obj = const_cast<as_object*>(scopeStack[i-1].get());
		if (obj)
		{
			std::pair<bool,bool> ret = obj->delProperty(varkey);
			if (ret.first)
			{
			    return ret.second;
			}
		}
	}

	// Check locals for deletion.
	if ( delLocal(varname) )
	{
		return true;
	}


	// Try target
	std::pair<bool,bool> ret = m_target->delProperty(varkey);
	if ( ret.first )
	{
		return ret.second;
	}

	// TODO: try 'this' ? Add a testcase for it !

	// Try _global 
	return VM::get().getGlobal()->delProperty(varkey).second;
}

// varname must be a plain variable name; no path parsing.
as_value
as_environment::get_variable_raw(const std::string& varname) const
{
	static ScopeStack empty_scopeStack;
	return get_variable_raw(varname, empty_scopeStack);
}

// Given a path to variable, set its value.
void
as_environment::set_variable(
    const std::string& varname,
    const as_value& val,
    const ScopeStack& scopeStack)
{
	IF_VERBOSE_ACTION (
    log_action("-------------- %s = %s",
	       varname.c_str(), val.to_debug_string().c_str());
	);

    // Path lookup rigamarole.
    as_object* target = m_target;
    std::string	path;
    std::string	var;
    //log_msg(_("set_variable(%s, %s)"), varname.c_str(), val.to_debug_string().c_str());
    bool is_slash_based;
    if (parse_path(varname, path, var, &is_slash_based)) {
    	//log_msg(_("Variable '%s' parsed into path='%s', var='%s'"), varname.c_str(), path.c_str(), var.c_str());
	//target = find_target(path);
        target = is_slash_based ? find_object_slashsyntax(path, &scopeStack) : find_object_dotsyntax(path, &scopeStack); 
	if (target)
	{
	    target->set_member(VM::get().getStringTable().find(var), val);
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Path target '%s' not found while setting %s=%s"),
			path.c_str(), varname.c_str(), val.to_debug_string().c_str());
		);
	}
    } else {
	set_variable_raw(varname, val, scopeStack);
    }
}

void
as_environment::set_variable(
		const std::string& varname,
		const as_value& val)
{
	static ScopeStack empty_scopeStack;
	set_variable(varname, val, empty_scopeStack);
}

// No path rigamarole.
void
as_environment::set_variable_raw(
    const std::string& varname,
    const as_value& val,
    const ScopeStack& scopeStack)
{

    VM& vm = VM::get();
    int swfVersion = vm.getSWFVersion();
    string_table& st = vm.getStringTable();
    string_table::key varkey = st.find(varname);

    if ( swfVersion < 6 ) 
    {
        // in SWF5 and lower, scope stack should just contain 'with' elements 

        // Check the with-stack.
        for (size_t i = scopeStack.size(); i > 0; --i)
        {
            // const_cast needed due to non-const as_object::get_member 
            as_object* obj = const_cast<as_object*>(scopeStack[i-1].get());
            if (obj && obj->update_member(varkey, val).first )
            {
		return;
#if 0
		Property* prop = obj->findUpdatableProperty(varkey);
		if ( prop )
		{
			//prop->setValue(*obj, val);
			obj->set_member(varkey, val);
			return;
		}
#endif
            }
        }

        // Check locals for setting them
        if ( setLocal(varname, val) ) return;

    }
    else // SWF >= 6
    {

        // Check the scope-stack (would include locals)
        //
        for (size_t i = scopeStack.size(); i > 0; --i)
        {
            // const_cast needed due to non-const as_object::get_member 
            as_object* obj = const_cast<as_object*>(scopeStack[i-1].get());
            if (obj && obj->update_member(varkey, val).first )
            {
		return;
#if 0
		Property* prop = obj->findUpdatableProperty(varkey);
		if ( prop )
		{
			//prop->setValue(*obj, val);
			obj->set_member(varkey, val);
			return;
		}
#endif
            }
        }

    }
    
    // TODO: shouldn't m_target be in the scope chain ?
	assert(m_target);
	m_target->set_member(varkey, val);
}

void
as_environment::set_variable_raw(
		const std::string& varname,
		const as_value& val)
{
	static ScopeStack empty_scopeStack;
	set_variable_raw(varname, val, empty_scopeStack);
}

// Set/initialize the value of the local variable.
void
as_environment::set_local(const std::string& varname, const as_value& val)
{
	// why would you want to set a local if there's no call frame on the
	// stack ?
	assert( ! _localFrames.empty() );

	string_table::key varkey = VM::get().getStringTable().find(varname);
	// Is it in the current frame already?
	if ( setLocal(varname, val) )
	{
		return;
	}
	else
	{
		// Not in frame; create a new local var.
		assert( ! varname.empty() ); // null varnames are invalid!
		LocalVars& locals = _localFrames.back().locals;
		//locals.push_back(as_environment::frame_slot(varname, val));
		locals->set_member(varkey, val);
	}
}
	
// Create the specified local var if it doesn't exist already.
void
as_environment::declare_local(const std::string& varname)
{
	as_value tmp;
	if ( ! findLocal(varname, tmp) )
	{
		// Not in frame; create a new local var.
		assert( ! _localFrames.empty() );
		assert( ! varname.empty() );	// null varnames are invalid!
		LocalVars& locals = _localFrames.back().locals;
		//locals.push_back(as_environment::frame_slot(varname, as_value()));
		locals->set_member(VM::get().getStringTable().find(varname), as_value());
	}
}

/* public static */
bool
as_environment::parse_path(const std::string& var_path,
		std::string& path, std::string& var, bool* is_slash_based) 
{
//log_msg(_("parse_path(%s)"), var_path.c_str());
    // Search for colon.
    int	colon_index = 0;
    int	var_path_length = var_path.length();
    for ( ; colon_index < var_path_length; colon_index++) {
	if (var_path[colon_index] == ':') {
            if ( is_slash_based ) *is_slash_based = true;
	    // Found it.
	    break;
	}
    }
    
    if (colon_index >= var_path_length)	{
	// No colon.  Is there a '.'?  Find the last
	// one, if any.
	for (colon_index = var_path_length - 1; colon_index >= 0; colon_index--) {
	    if (var_path[colon_index] == '.') {
		// Found it.
                if ( is_slash_based ) *is_slash_based = false;
		break;
	    }
	}
	if (colon_index < 0) {
//log_msg(_(" no colon index"));
	    return false;
	}
    }
    
    // Make the subparts.
    
    // Var.
    var = &var_path[colon_index + 1];
    
    // @@ could be better. 
    path = var_path;
    path.resize(colon_index);
    
//log_msg(_(" path=%s var=%s"), path.c_str(), var.c_str());

    return true;
}

bool
as_environment::parse_path(const std::string& var_path,
		as_object** target, as_value& val)
{
	string path;
	string var;
	bool is_slash_based;
	if( ! parse_path(var_path, path, var, &is_slash_based) ) return false;
        as_object* target_ptr = is_slash_based ? find_object_slashsyntax(path) : find_object_dotsyntax(path); 
	if ( ! target_ptr ) return false;

	target_ptr->get_member(VM::get().getStringTable().find(var), &val);
	*target = target_ptr;
	return true;
}

// Search for next '.' or '/' character in this word.  Return
// a pointer to it, or to NULL if it wasn't found.
static const char*
next_slash_or_dot(const char* word)
{
    for (const char* p = word; *p; p++)	{
	if (*p == '.' && p[1] == '.') {
	    p++;
	} else if (*p == '.' || *p == '/') {
	    return p;
	}
    }
    
    return NULL;
}

static const char*
find_next_slash(const char* word)
{
    for (const char* p = word; *p; p++)	{
	if (*p == '/') {
	    return p;
	}
    }
    
    return NULL;
}

static const char*
find_next_dot(const char* word)
{
    for (const char* p = word; *p; p++)	{
	if (*p == '.' ) {
	    return p;
	}
    }
    
    return NULL;
}

// Find the sprite/movie referenced by the given path.
//
// Supports both /slash/syntax and dot.syntax
//
character*
as_environment::find_target(const std::string& path_in) const
{
#ifdef DEBUG_TARGET_FINDING 
	log_msg(_("find_target(%s) called"), path_in.c_str());
#endif

    if (path_in.empty())
    {
#ifdef DEBUG_TARGET_FINDING 
	log_msg(_("Returning m_target (empty path)"));
#endif
	return m_target; // or should we return the *original* path ?
    }
    
    string path = path_in;
    VM& vm = VM::get();
    string_table& st = vm.getStringTable();
    int swfVersion = vm.getSWFVersion(); 

    // Convert to lower case if needed
    if ( swfVersion < 7 ) boost::to_lower(path);

    as_object* env = m_target; 
    assert(env);

    const char*	p = path.c_str();
    if (*p == '/') {
	// Absolute path.  Start at the root.
	sprite_instance* root = m_target->get_root_movie();
	if ( ! *(++p) )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_msg(_("Path is '/', return the root (%p)"), (void*)root);
#endif
		return root; // that's all folks.. 
	}

	env = root;

#ifdef DEBUG_TARGET_FINDING 
	log_msg(_("Absolute path, start at the root (%p)"), (void*)env);
#endif

    }
#ifdef DEBUG_TARGET_FINDING 
    else
    {
	log_msg(_("Relative path, start at (%s)"), m_target->getTarget().c_str());
    }
#endif
    
    assert (*p);

    std::string	subpart;
    while (env)
    {
	const char*	next_slash = next_slash_or_dot(p);
	subpart = p;
	if (next_slash == p)
	{
            IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror(_("invalid path '%s'"), path.c_str());
	    );
	    return NULL;
	    //break;
	}
	else if (next_slash)
	{
	    // Cut off the slash and everything after it.
	    subpart.resize(next_slash - p);
	    // Remove any column in the subpart
	}
	
	while ( ! subpart.empty() && subpart[0] == ':' ) subpart = subpart.substr(1);

	// No more components to scan
	if ( subpart.empty() )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_msg(_("No more subparts, env is %p"), (void*)env);
#endif
		break;
	}

#ifdef DEBUG_TARGET_FINDING 
	log_msg(_("Invoking get_path_element(%s) on object %p (%s)"), subpart.c_str(), (void *)env, env->get_text_value().c_str());
#endif

	as_object* element = env->get_path_element(st.find(subpart));
        if ( ! element )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_msg(_("Path element %s not found in object %p"), subpart.c_str(), (void *)env);
#endif
		return NULL;
	}
	env = element;

	//@@   _level0 --> root, .. --> parent, . --> this, other == character
	
	if (next_slash == NULL)
	{
	    break;
	}
	
	p = next_slash + 1;
    }
    return env->to_movie(); // can be NULL (not a movie)...
}

as_object*
as_environment::find_object_dotsyntax(const std::string& path, const ScopeStack* scopeStack) const
{
#ifdef DEBUG_TARGET_FINDING 
	log_msg(_("find_object_dotsyntax(%s) called"), path.c_str());
#endif

    VM& vm = VM::get();

    if ( path.empty() )
    {
#ifdef DEBUG_TARGET_FINDING 
        log_msg(_("Returning m_target (empty path)"));
#endif
        return m_target;
    }
    
    // we'd have returned m_target in this case
    //assert(path.length() > 0);
    
    as_object* env = m_target;
    if ( scopeStack && ! scopeStack->empty() ) env = scopeStack->back().get();
    assert(env);
    
    const char*	p = path.c_str();
    unsigned int depth=0; // number of iterations
    while (env)
    {
        const char* next_dot = find_next_dot(p); // TODO: use std::string::find
    	std::string subpart = p;
        if (next_dot == p)
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("invalid path '%s'"), path.c_str());
            );
            return NULL; // TODO: check me
        }
        else if (next_dot)
        {
            // Cut off the slash and everything after it.
            subpart.resize(next_dot - p);
        }

#ifdef DEBUG_TARGET_FINDING 
        log_msg(_("Subpart ==  %s"), subpart.c_str());
#endif
	
        // No more components to scan
        if ( subpart.empty() )
        {
#ifdef DEBUG_TARGET_FINDING 
            log_debug(_("No more subparts, env is %p"), (void*)env);
#endif
            break;
        }

#ifdef DEBUG_TARGET_FINDING 
        log_debug(_("Invoking get_path_element(%s) on object %p"), subpart.c_str(), (void *)env);
#endif

        //as_value tmp;
        string_table::key subpartkey = vm.getStringTable().find(subpart);

        // TODO: make sure sprite_instances know about ".."
	as_object* nextElement = env->get_path_element(subpartkey);
	if ( ! nextElement )
        //if (!env->get_member(subpartkey, &tmp) )
        {
            // Try this and _global, but only at first iteration...

            if ( depth > 0 ) 
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("Member %s for object %p not found (dotsyntax).  Path was %s"),
                    subpart.c_str(), (void *)env, path.c_str());
                );
                return NULL;
            }

            if ( subpart == "this" )
            {
                //tmp.set_as_object(m_target); 
                nextElement = m_target; 
            }
            else
            {
                nextElement = vm.getGlobal()->get_path_element(subpartkey); 
                if ( ! nextElement )
                {
                  IF_VERBOSE_ASCODING_ERRORS(
                  log_aserror(_("Path element '%s' of variable '%s' not found in object %p nor in _global (dotsyntax)"),
                      subpart.c_str(), path.c_str(), (void *)env);
                  );
                  return NULL;
                }
            }
        } 
	assert(nextElement);

        env = nextElement; // tmp.to_object().get();
#ifndef GNASH_USE_GC
        assert(env->get_ref_count() > 0); // still alive...
#endif // ndef GNASH_USE_GC


        //@@   _level0 --> root, .. --> parent, . --> this, other == character
	
        if (next_dot == NULL)
        {
            break;
        }
	
        p = next_dot + 1;
        ++depth;
    }

    return env;
}

as_object*
as_environment::find_object_slashsyntax(const std::string& path, const ScopeStack* scopeStack) const
{
#ifdef DEBUG_TARGET_FINDING 
	log_msg(_("find_object_slashsyntax(%s) called"), path.c_str());
#endif

    if ( path.empty() )
    {
#ifdef DEBUG_TARGET_FINDING 
        log_debug(_("Returning m_target (empty path)"));
#endif
        return m_target;
    }
    
    as_object* env = m_target;
    if ( scopeStack && ! scopeStack->empty() ) env = scopeStack->back().get();
    assert(env);
    
    const char*	p = path.c_str();

    if (*p == '/')
    {
        // Absolute path.  Start at the *absolute* root.
        // TODO: should this be VM::get().getRoot().get_root_movie() ?
        env = m_target->get_root_movie();
#ifdef DEBUG_TARGET_FINDING 
        log_debug(_("Absolute path, start at the root (%p)"), (void*)env);
#endif
        p++;
    }
    
    if (*p == '\0')
    {
#ifdef DEBUG_TARGET_FINDING 
        log_msg(_("Slash-only path, returning root movie"));
#endif
        assert(path=="/");
        assert(env == m_target->get_root_movie());
        return env;
    }

    VM& vm = VM::get();

    unsigned int depth=0; // number of iterations
    while (env)
    {
    	std::string	subpart;
        const char* next_slash = find_next_slash(p); // TODO: use std::string::find
        subpart = p;
        if (next_slash == p)
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("invalid path '%s'"), path.c_str());
            );
            return NULL; // TODO: check me
        }
        else if (next_slash)
        {
            // Cut off the slash and everything after it.
            // TODO: optimize by using offsets in the original string
            subpart.resize(next_slash - p);
        }

#ifdef DEBUG_TARGET_FINDING 
        log_debug(_("Subpart ==  %s"), subpart.c_str());
#endif
	
        // No more components to scan
        if ( subpart.empty() )
        {
#ifdef DEBUG_TARGET_FINDING 
            log_debug(_("No more subparts, env is %p"), (void*)env);
#endif
            break;
        }

        if ( subpart == ".." )
        {
            character* ch = dynamic_cast<character*>(env);
            if ( ! ch )
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("'..' element in path '%s' follows a non-character object %p"),
                    path.c_str(), (void *)env);
                );
                return NULL;
            }
            env = ch->get_parent();
            if ( ! env ) // root movie doesn't have a parent
            {
                IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("'..' in path '%s' follows a character "
                    "with no parent (%s : %p) (root is %p)"),
                    path.c_str(), ch->get_text_value().c_str(), (void *)ch,
                    (void *)vm.getRoot().get_root_movie());
			    );

			    // if we override env, getvariable.as fails [line 57]
			    //env = ch;
            }
        }
	    else
	    {

#ifdef DEBUG_TARGET_FINDING 
		    log_debug(_("Invoking get_path_element(%s) on object %p"), subpart.c_str(), (void *)env);
#endif
    		//as_value tmp;
    		string_table::key subpartkey = vm.getStringTable().find(subpart);
    		// TODO: make sure sprite_instances know about ".."
		as_object* nextElement = env->get_path_element(subpartkey);
		if ( ! nextElement )
    		//if (!env->get_member(subpartkey, &tmp) )
    		{
    			// Try this and _global, but only at first iteration...
    
    			if ( depth > 0 ) 
    			{
    				IF_VERBOSE_ASCODING_ERRORS(
    				log_aserror(_("Member %s for object %p not found (slashsyntax). Path was %s"),
    					subpart.c_str(), (void *)env, path.c_str());
    				);
    				return NULL;
    			}
    
    			if ( subpart == "this" )
    			{
    				//tmp.set_as_object(m_target);
                		nextElement = m_target; 
    			}
    
    			else
			{
				nextElement = vm.getGlobal()->get_path_element(subpartkey); 
				if ( ! nextElement )
				{
					IF_VERBOSE_ASCODING_ERRORS(
					log_aserror(_("Element '%s' of variable '%s' not found in object %p nor in _global (slashsyntax)"),
						subpart.c_str(), path.c_str(), (void *)env);
					);
					return NULL;
				}
			}
		} 

		assert(nextElement);

		env = nextElement;
#ifndef GNASH_USE_GC
		assert(env->get_ref_count() > 0);
#endif // ndef GNASH_USE_GC
	    }

	    //@@   _level0 --> root, .. --> parent, . --> this, other == character
	
	    if (next_slash == NULL)
	    {
	        break;
	    }
	
	    p = next_slash + 1;
	    ++depth;
    }
    return env;
}

int
as_environment::get_version() const
{
	return VM::get().getSWFVersion();
}

static void
dump(const as_environment::Registers& r, std::ostream& out) 
{
	for (size_t i=0; i<r.size(); ++i)
	{
		if (i) out << ", ";
		out << i << ':' << '"' << r[i] << '"';
	}
}

void
as_environment::dump_local_registers(std::ostream& out) const
{
	if ( _localFrames.empty() ) return;
	out << "Local registers: ";
	for (CallStack::const_iterator it=_localFrames.begin(),
			itEnd=_localFrames.end();
			it != itEnd; ++it)
	{
		if ( it != _localFrames.begin() ) out << " | ";
		dump(it->registers, out);
	}
	out << std::endl;
}

static void
dump(const as_environment::LocalVars& locals, std::ostream& out)
{
	typedef std::map<std::string, as_value> PropMap;
	PropMap props;
	const_cast<as_object*>(locals.get())->dump_members(props);
	
	//log_msg("FIXME: implement dumper for local variables now that they are simple objects");
	int count = 0;
	for (PropMap::iterator i=props.begin(), e=props.end(); i!=e; ++i)
	{
		if (count++) out << ", ";
		// TODO: define output operator for as_value !
		out << i->first << "==" << i->second.to_debug_string();
	}
	out << std::endl;
}

void
as_environment::dump_local_variables(std::ostream& out) const
{
	if ( _localFrames.empty() ) return;
	out << "Local variables: ";
	for (CallStack::const_iterator it=_localFrames.begin(),
			itEnd=_localFrames.end();
			it != itEnd; ++it)
	{
		if ( it != _localFrames.begin() ) out << " | ";
		dump(it->locals, out);
	}
	out << std::endl;
}

void
as_environment::dump_global_registers(std::ostream& out) const
{
	std::string registers;

	std::stringstream ss;

	ss << "Global registers: ";
	int defined=0;
	for (unsigned int i=0; i<numGlobalRegisters; ++i)
	{
		if ( m_global_register[i].is_undefined() ) continue;

		if ( defined++ ) ss <<  ", ";

		ss << i << ":" << m_global_register[i].to_debug_string();

	}
	if ( defined ) out << ss.str() << std::endl;
}

/*private*/
bool
as_environment::findLocal(const std::string& varname, as_value& ret, as_object** retTarget)
{
	if ( _localFrames.empty() ) return false;
	if ( findLocal(_localFrames.back().locals, varname, ret) )
	{
		if ( retTarget ) *retTarget = _localFrames.back().locals.get();
		return true;
	}
	return false;
}

/* static private */
bool
as_environment::findLocal(LocalVars& locals, const std::string& name, as_value& ret)
{
	return locals->get_member(VM::get().getStringTable().find(name), &ret);
}

/* static private */
bool
as_environment::delLocal(LocalVars& locals, const std::string& varname)
{
	return locals->delProperty(VM::get().getStringTable().find(varname)).second;
}

/* private */
bool
as_environment::delLocal(const std::string& varname)
{
	if ( _localFrames.empty() ) return false;
	return delLocal(_localFrames.back().locals, varname);
}

/* private */
bool
as_environment::setLocal(const std::string& varname, const as_value& val)
{
	if ( _localFrames.empty() ) return false;
	return setLocal(_localFrames.back().locals, varname, val);
}

/* static private */
bool
as_environment::setLocal(LocalVars& locals,
		const std::string& varname, const as_value& val)
{
	Property* prop = locals->getOwnProperty(VM::get().getStringTable().find(varname));
	if ( ! prop ) return false;
	prop->setValue(*locals, val);
	return true;
}


void
as_environment::padStack(size_t offset, size_t count)
{
	assert( offset <= m_stack.size() );
	m_stack.insert(m_stack.begin()+offset, count, as_value());
}

void
as_environment::pushCallFrame(as_function* func)
{
	const unsigned maxstacksize = 255;

	if ( _localFrames.size() == maxstacksize )
	{
		char buf[256];
		snprintf(buf, 255, _("Max stack count reached (%u)"),
				maxstacksize);

		// throw something
		throw ActionLimitException(buf); 
	}
	_localFrames.push_back(CallFrame(func));
}

void
as_environment::set_target(character* target)
{
	assert(target);
	if ( ! m_target ) _original_target = target;
	m_target = target;
}

void
as_environment::add_local(const std::string& varname, const as_value& val)
{
	assert( ! varname.empty() );	// null varnames are invalid!
	assert( ! _localFrames.empty() );
	LocalVars& locals = _localFrames.back().locals;
	//locals.push_back(frame_slot(varname, val));
	locals->set_member(VM::get().getStringTable().find(varname), val);
}

as_environment::CallFrame::CallFrame(as_function* funcPtr)
	:
	locals(new as_object()),
	func(funcPtr)
{
}

#ifdef GNASH_USE_GC
/// Mark all reachable resources
//
/// Reachable resources would be registers and
/// locals (expected to be empty?) and function.
void
as_environment::CallFrame::markReachableResources() const
{
	if ( func ) func->setReachable();
	for (Registers::const_iterator i=registers.begin(), e=registers.end(); i!=e; ++i)
	{
		i->setReachable();
	}
	locals->setReachable();
}

void
as_environment::markReachableResources() const
{
	for (size_t i=0; i<4; ++i)
	{
		m_global_register[i].setReachable();
	}

	if ( m_target ) m_target->setReachable();
	if ( _original_target ) _original_target->setReachable();

	assert ( _localFrames.empty() );
#if 1 // I think we expect the stack to be empty !
	for (CallStack::const_iterator i=_localFrames.begin(), e=_localFrames.end(); i!=e; ++i)
	{
		i->markReachableResources();
	}
#endif

	assert ( m_stack.empty() );
#if 1 // I think we expect the stack to be empty !
	for (std::vector<as_value>::const_iterator i=m_stack.begin(), e=m_stack.end(); i!=e; ++i)
	{
		i->setReachable();
	}
#endif
}
#endif // GNASH_USE_GC

} // end of gnash namespace



// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
