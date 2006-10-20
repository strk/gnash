// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
//

/* $Id: as_environment.cpp,v 1.23 2006/10/20 15:42:10 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smart_ptr.h"
#include "as_environment.h"
#include "sprite_instance.h"
#include "character.h"
#include "as_value.h"
#include "with_stack_entry.h"

namespace gnash {

// Return the value of the given var, if it's defined.
as_value
as_environment::get_variable(const tu_string& varname,
		const std::vector<with_stack_entry>& with_stack) const
{
    // Path lookup rigamarole.
    character*	target = m_target;
    tu_string	path;
    tu_string	var;
    //log_msg("get_variable(%s)", varname.c_str());
    if (parse_path(varname, path, var)) {
	target = find_target(path);	// @@ Use with_stack here too???  Need to test.
	if (target) {
	    as_value	val;
	    target->get_member(var, &val);
	    return val;
	} else {
	    log_error("find_target(\"%s\") failed", path.c_str());
	    return as_value();
	}
    } else {
	return this->get_variable_raw(varname, with_stack);
    }
}

as_value
as_environment::get_variable(const tu_string& varname) const
{
	static std::vector<with_stack_entry> empty_with_stack;
	return get_variable(varname, empty_with_stack);
}

as_value
as_environment::get_variable_raw(
    const tu_string& varname,
    const std::vector<with_stack_entry>& with_stack) const
    // varname must be a plain variable name; no path parsing.
{
    assert(strchr(varname.c_str(), ':') == NULL);
    assert(strchr(varname.c_str(), '/') == NULL);
    assert(strchr(varname.c_str(), '.') == NULL);

    as_value	val;

    // Check the with-stack.
    for (size_t i = with_stack.size(); i > 0; --i) {
	as_object* obj = with_stack[i-1].m_object.get_ptr();
	if (obj && obj->get_member(varname, &val)) {
	    // Found the var in this context.
	    return val;
	}
    }

    // Check locals.
    int	local_index = find_local(varname);
    if (local_index >= 0) {
	// Get local var.
	return m_local_frames[local_index].m_value;
    }
    
    // Looking for "this"?
    if (varname == "this") {
	val.set_as_object(m_target);
	return val;
    }

    // Check target members.
    if (m_target->get_member(varname, &val)) {
	return val;
    }
    
    // Check built-in constants.
    if (varname == "_root" || varname == "_level0") {
	return as_value(m_target->get_root_movie());
    }
    if (varname == "_global") {
	return as_value(s_global.get_ptr());
    }
    if (s_global->get_member(varname, &val)) {
	return val;
    }
    
    // Fallback.
	IF_VERBOSE_ACTION (
    log_action("get_variable_raw(\"%s\" failed, returning UNDEFINED.)",
	       varname.c_str());
	);

    return as_value();
}

// varname must be a plain variable name; no path parsing.
as_value
as_environment::get_variable_raw(const tu_string& varname) const
{
	static std::vector<with_stack_entry> empty_with_stack;
	return get_variable_raw(varname, empty_with_stack);
}

// Given a path to variable, set its value.
void
as_environment::set_variable(
    const tu_string& varname,
    const as_value& val,
    const std::vector<with_stack_entry>& with_stack)
{
	IF_VERBOSE_ACTION (
    log_action("-------------- %s = %s",
	       varname.c_str(), val.to_string());
	);

    // Path lookup rigamarole.
    character* target = m_target;
    tu_string	path;
    tu_string	var;
    //log_msg("set_variable(%s, %s)", varname.c_str(), val.to_string());
    if (parse_path(varname, path, var)) {
	target = find_target(path);
	if (target) {
	    target->set_member(var, val);
	}
    } else {
	this->set_variable_raw(varname, val, with_stack);
    }
}

void
as_environment::set_variable(
		const tu_string& varname,
		const as_value& val)
{
	static std::vector<with_stack_entry> empty_with_stack;
	set_variable(varname, val, empty_with_stack);
}

// No path rigamarole.
void
as_environment::set_variable_raw(
    const tu_string& varname,
    const as_value& val,
    const std::vector<with_stack_entry>& with_stack)
{
	// Check the with-stack.
	for (int i = with_stack.size() - 1; i >= 0; i--)
	{
		as_object*	obj = with_stack[i].m_object.get_ptr();
		as_value	dummy;
		if (obj && obj->get_member(varname, &dummy)) {
		    // This object has the member; so set it here.
		    obj->set_member(varname, val);
		    return;
		}
	}
    
    // Check locals.
    int	local_index = find_local(varname);
    if (local_index >= 0) {
	// Set local var.
	m_local_frames[local_index].m_value = val;
	return;
    }
    
    assert(m_target);

    m_target->set_member(varname, val);
}

void
as_environment::set_variable_raw(
		const tu_string& varname,
		const as_value& val)
{
	static std::vector<with_stack_entry> empty_with_stack;
	set_variable_raw(varname, val, empty_with_stack);
}

// Set/initialize the value of the local variable.
void
as_environment::set_local(const tu_string& varname, const as_value& val)
{
    // Is it in the current frame already?
    int	index = find_local(varname);
    if (index < 0) {
	// Not in frame; create a new local var.
	
	assert(varname.length() > 0);	// null varnames are invalid!
	m_local_frames.push_back(frame_slot(varname, val));
    } else {
	// In frame already; modify existing var.
	m_local_frames[index].m_value = val;
    }
}
	
// Add a local var with the given name and value to our
// current local frame.  Use this when you know the var
// doesn't exist yet, since it's faster than set_local();
// e.g. when setting up args for a function.
void
as_environment::add_local(const tu_string& varname, const as_value& val)
{
    assert(varname.length() > 0);
    m_local_frames.push_back(frame_slot(varname, val));
}

// Create the specified local var if it doesn't exist already.
void
as_environment::declare_local(const tu_string& varname)
{
    // Is it in the current frame already?
    int	index = find_local(varname);
    if (index < 0) {
	// Not in frame; create a new local var.
	assert(varname.length() > 0);	// null varnames are invalid!
	m_local_frames.push_back(frame_slot(varname, as_value()));
    } else {
	// In frame already; don't mess with it.
    }
}
	
bool
as_environment::get_member(const tu_stringi& varname, as_value* val) const
{
    return m_variables.get(varname, val);
}


void
as_environment::set_member(const tu_stringi& varname, const as_value& val)
{
    m_variables[varname] = val;
}

as_value&
as_environment::local_register(uint8_t n)
{
	assert( n < m_local_register.size() );
	return m_local_register[n];
}

void
as_environment::drop_local_registers(unsigned int register_count)
{
	assert(register_count <= m_local_register.size());
	m_local_register.resize(m_local_register.size() - register_count);
}

void
as_environment::add_local_registers(unsigned int register_count)
{
	m_local_register.resize(m_local_register.size() + register_count);
}


#if 0
as_value*
as_environment::local_register_ptr(unsigned int reg)
{
	// We index the registers from the end of the register
	// array, so we don't have to keep base/frame
	// pointers.

	if (reg > m_local_register.size())
	{
		log_error("Invalid local register %d, stack only has "
			"%ld entries",
		  	reg, m_local_register.size());
	
		return &m_global_register[0];
	}
    
	return &m_local_register[m_local_register.size() - reg];
}
#endif

// Search the active frame for the named var; return its index
// in the m_local_frames stack if found.
// 
// Otherwise return -1.
int
as_environment::find_local(const tu_string& varname) const
{
    // Linear search sucks, but is probably fine for
    // typical use of local vars in script.  There could
    // be pathological breakdowns if a function has tons
    // of locals though.  The ActionScript bytecode does
    // not help us much by using strings to index locals.
    
    for (int i = m_local_frames.size() - 1; i >= 0; i--) {
	const frame_slot&	slot = m_local_frames[i];
	if (slot.m_name.length() == 0) {
	    // End of local frame; stop looking.
	    return -1;
	} else if (slot.m_name == varname) {
	    // Found it.
	    return i;
	}
    }
    return -1;
}

/* public static */
bool
as_environment::parse_path(const tu_string& var_path,
		tu_string& path, tu_string& var) 
{
//log_msg("parse_path(%s)", var_path.c_str());
    // Search for colon.
    int	colon_index = 0;
    int	var_path_length = var_path.length();
    for ( ; colon_index < var_path_length; colon_index++) {
	if (var_path[colon_index] == ':') {
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
		break;
	    }
	}
	if (colon_index < 0) {
//log_msg(" no colon index");
	    return false;
	}
    }
    
    // Make the subparts.
    
    // Var.
    var = &var_path[colon_index + 1];
    
    // Path.
    if (colon_index > 0) {
	if (var_path[colon_index - 1] == '/') {
	    // Trim off the extraneous trailing slash.
	    colon_index--;
	}
    }
    // @@ could be better.  This whole usage of tu_string is very flabby...
    path = var_path;
    path.resize(colon_index);
    
//log_msg(" path=%s var=%s", path.c_str(), var.c_str());

    return true;
}

character*
as_environment::find_target(const as_value& val) const
{
	if (val.get_type() == as_value::OBJECT)
	{
		as_object* obj = val.to_object();
		assert (obj);
		character* s=dynamic_cast<character*>(obj);
		return s; // might be NULL
	}
	else if (val.get_type() == as_value::STRING)
	{
		return find_target(val.to_tu_string());
	}
	else
	{
		log_error("error: %s: invalid path; neither string nor object",
			val.to_string());
		return NULL;
	}
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

// Find the sprite/movie referenced by the given path.
//
// Supports both /slash/syntax and dot.syntax
//
character*
as_environment::find_target(const tu_string& path) const
{
    if (path.length() <= 0) {
	return m_target;
    }
    
    // we'd have returned m_target in this case
    //assert(path.length() > 0);
    
    character* env = m_target;
    assert(env);
    
    const char*	p = path.c_str();

    if (*p == '/') {
	// Absolute path.  Start at the root.
	env = env->get_root_movie();
	p++;
    }
    
    if (*p == '\0') {
	return env;
    }

    tu_string	subpart;
    while (env) {
	const char*	next_slash = next_slash_or_dot(p);
	subpart = p;
	if (next_slash == p) {
	    log_error("error: invalid path '%s'", path.c_str());
	    break;
	} else if (next_slash) {
	    // Cut off the slash and everything after it.
	    subpart.resize(next_slash - p);
	}
	
	env = env->get_relative_target(subpart);
	//@@   _level0 --> root, .. --> parent, . --> this, other == character
	
	if (env == NULL || next_slash == NULL) {
	    break;
	}
	
	p = next_slash + 1;
    }
    return env;
}

int
as_environment::get_version() const
{
	sprite_instance* si=m_target->get_root_movie();
	movie_definition* md=si->get_movie_definition();
	return md->get_version();
}

}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
