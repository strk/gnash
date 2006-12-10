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

//

/* $Id: as_environment.cpp,v 1.39 2006/12/10 18:39:22 strk Exp $ */

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

#include <string>
#include <utility> // for std::pair

namespace gnash {

// Return the value of the given var, if it's defined.
as_value
as_environment::get_variable(const std::string& varname,
		const std::vector<with_stack_entry>& with_stack) const
{
    // Path lookup rigamarole.
    character*	target = m_target;
    std::string	path;
    std::string	var;
    //log_msg("get_variable(%s)", varname.c_str());
    if (parse_path(varname, path, var)) {
	target = find_target(path);	// @@ Use with_stack here too???  Need to test.
	if (target) {
	    as_value	val;
	    target->get_member(var.c_str(), &val);
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
as_environment::get_variable(const std::string& varname) const
{
	static std::vector<with_stack_entry> empty_with_stack;
	return get_variable(varname, empty_with_stack);
}

as_value
as_environment::get_variable_raw(
    const std::string& varname,
    const std::vector<with_stack_entry>& with_stack) const
    // varname must be a plain variable name; no path parsing.
{
    assert(strchr(varname.c_str(), ':') == NULL);
    assert(strchr(varname.c_str(), '/') == NULL);
    assert(strchr(varname.c_str(), '.') == NULL);

    as_value	val;

    // Check the with-stack.
    for (size_t i = with_stack.size(); i > 0; --i) {
	as_object* obj = with_stack[i-1].m_object.get();
	if (obj && obj->get_member(varname.c_str(), &val)) {
	    // Found the var in this context.
	    return val;
	}
    }

    // Check locals.
    LocalFrames::const_iterator it = findLocal(varname);
    if (it != endLocal()) {
	// Get local var.
	return it->m_value;
    }
    
    // Looking for "this"?
    if (varname == "this") {
	val.set_as_object(m_target);
	return val;
    }

    // Check target members.
    if (m_target->get_member(varname.c_str(), &val)) {
	return val;
    }
    
    // Check built-in constants.
    if (varname == "_root" || varname == "_level0") {
	return as_value(m_target->get_root_movie());
    }

    as_object* global = VM::get().getGlobal();

    if (varname == "_global") {
	return as_value(global);
    }
    if (global->get_member(varname.c_str(), &val)) {
	return val;
    }
    
    // Fallback.
	IF_VERBOSE_ACTION (
    log_action("get_variable_raw(\"%s\" failed, returning UNDEFINED.)",
	       varname.c_str());
	);

    return as_value();
}

bool
as_environment::del_variable_raw(
    const std::string& varname,
    const std::vector<with_stack_entry>& with_stack) 
    // varname must be a plain variable name; no path parsing.
{
	assert(strchr(varname.c_str(), ':') == NULL);
	assert(strchr(varname.c_str(), '/') == NULL);
	assert(strchr(varname.c_str(), '.') == NULL);

	as_value	val;

	// Check the with-stack.
	for (size_t i = with_stack.size(); i > 0; --i)
	{
		as_object* obj = with_stack[i-1].m_object.get();
		if (obj)
		{
			std::pair<bool,bool> ret = obj->delProperty(varname);
			if (ret.first)
			{
			    return ret.second;
			}
		}
	}

	// Check locals.
	LocalFrames::iterator it = findLocal(varname);
	if (it != endLocal())
	{
		// delete local var.
		// This sucks, we need m_local_frames to be a list
		// or map, NOT A VECTOR !
		m_local_frames.erase(it);
		return true;
	}

	// Try target
	std::pair<bool,bool> ret = m_target->delProperty(varname);
	if ( ret.first )
	{
		return ret.second;
	}

	// Try _global
	return VM::get().getGlobal()->delProperty(varname).second;
}

// varname must be a plain variable name; no path parsing.
as_value
as_environment::get_variable_raw(const std::string& varname) const
{
	static std::vector<with_stack_entry> empty_with_stack;
	return get_variable_raw(varname, empty_with_stack);
}

// Given a path to variable, set its value.
void
as_environment::set_variable(
    const std::string& varname,
    const as_value& val,
    const std::vector<with_stack_entry>& with_stack)
{
	IF_VERBOSE_ACTION (
    log_action("-------------- %s = %s",
	       varname.c_str(), val.to_string());
	);

    // Path lookup rigamarole.
    character* target = m_target;
    std::string	path;
    std::string	var;
    //log_msg("set_variable(%s, %s)", varname.c_str(), val.to_string());
    if (parse_path(varname, path, var)) {
	target = find_target(path);
	if (target) {
	    target->set_member(var.c_str(), val);
	}
    } else {
	set_variable_raw(varname, val, with_stack);
    }
}

void
as_environment::set_variable(
		const std::string& varname,
		const as_value& val)
{
	static std::vector<with_stack_entry> empty_with_stack;
	set_variable(varname, val, empty_with_stack);
}

// No path rigamarole.
void
as_environment::set_variable_raw(
    const std::string& varname,
    const as_value& val,
    const std::vector<with_stack_entry>& with_stack)
{
	// Check the with-stack.
	for (int i = with_stack.size() - 1; i >= 0; i--)
	{
		as_object*	obj = with_stack[i].m_object.get();
		as_value	dummy;
		if (obj && obj->get_member(varname.c_str(), &dummy)) {
		    // This object has the member; so set it here.
		    obj->set_member(varname.c_str(), val);
		    return;
		}
	}
    
    // Check locals.
    LocalFrames::iterator it = findLocal(varname);
    if (it != endLocal()) {
	// Set local var.
	it->m_value = val;
	return;
    }
    
    assert(m_target);

    m_target->set_member(varname.c_str(), val);
}

void
as_environment::set_variable_raw(
		const std::string& varname,
		const as_value& val)
{
	static std::vector<with_stack_entry> empty_with_stack;
	set_variable_raw(varname, val, empty_with_stack);
}

// Set/initialize the value of the local variable.
void
as_environment::set_local(const std::string& varname, const as_value& val)
{
    // Is it in the current frame already?
    LocalFrames::iterator it = findLocal(varname), itEnd=endLocal();
    if (it == itEnd) {
	// Not in frame; create a new local var.
	assert(varname.length() > 0);	// null varnames are invalid!
	m_local_frames.push_back(frame_slot(varname, val));
    } else {
	// In frame already; modify existing var.
	it->m_value = val;
    }
}
	
// Add a local var with the given name and value to our
// current local frame.  Use this when you know the var
// doesn't exist yet, since it's faster than set_local();
// e.g. when setting up args for a function.
void
as_environment::add_local(const std::string& varname, const as_value& val)
{
    assert(varname.length() > 0);
    m_local_frames.push_back(frame_slot(varname, val));
}

// Create the specified local var if it doesn't exist already.
void
as_environment::declare_local(const std::string& varname)
{
    // Is it in the current frame already?
    LocalFrames::const_iterator it = findLocal(varname), itEnd=endLocal();
    if (it == itEnd) {
	// Not in frame; create a new local var.
	assert(varname.length() > 0);	// null varnames are invalid!
	m_local_frames.push_back(frame_slot(varname, as_value()));
    } else {
	// In frame already; don't mess with it.
    }
}
	
bool
as_environment::get_member(const std::string& varname, as_value* val) const
{
    Variables::const_iterator it = _variables.find(varname);
    if ( it == _variables.end() ) return false;
    
    *val = it->second;
    return true;
}


void
as_environment::set_member(const std::string& varname, const as_value& val)
{
    _variables[varname] = val;
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

/* public static */
bool
as_environment::parse_path(const std::string& var_path,
		std::string& path, std::string& var) 
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

    // @@ could be better. 
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
		return find_target(std::string(val.to_string()));
	}
	else
	{
		log_error("as_environment::find_target: '%s': "
			"invalid path; neither string nor object",
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
as_environment::find_target(const std::string& path) const
{
	//log_msg("find_target(%s) called", path.c_str());

    if (path.length() <= 0) {
	//log_msg("Returning m_target");
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
	//log_msg("Absolute path, start at the root (%p)", (void*)env);
	p++;
    }
    
    if (*p == '\0') {
	return env;
    }

    std::string	subpart;
    while (env) {
	const char*	next_slash = next_slash_or_dot(p);
	subpart = p;
	if (next_slash == p) {
	    log_error("invalid path '%s'", path.c_str());
	    break;
	} else if (next_slash) {
	    // Cut off the slash and everything after it.
	    subpart.resize(next_slash - p);
	}
	
	// No more components to scan
	if ( subpart.empty() ) break;

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
	assert(m_target);
	sprite_instance* si=m_target->get_root_movie();
	assert(si);
	movie_definition* md=si->get_movie_definition();
	assert(md);
	return md->get_version();
}

void
as_environment::dump_local_registers(std::ostream& out) const
{
	size_t n=m_local_register.size();
	if ( ! n ) return;
	out << "Local registers: ";
	for (unsigned int i=0; i<n; i++)
	{
		if (i) out << " | ";
		out << '"' << m_local_register[i].to_string() << '"';
	}
	out << std::endl;
}

void
as_environment::dump_global_registers(std::ostream& out) const
{
	std::string registers;
	int defined=0;
	for (unsigned int i=0; i<4; ++i)
	{
		if (i) registers += std::string(" | ");
		registers += std::string("\"") +
			m_global_register[i].to_std_string() +
			std::string("\"");
		if ( ! m_global_register[i].is_undefined() ) defined++;
	}
	if ( defined ) out << "Global registers (" << defined << "): " << registers << std::endl;
}

/*private*/
as_environment::LocalFrames::iterator
as_environment::findLocal(const std::string& varname)
{
#if 0
	LocalFrames::reverse_iterator itEnd=m_local_frames.rend();
	for (LocalFrames::reverse_iterator it=m_local_frames.rbegin();
			it != itEnd;
			++it)
	{
		const frame_slot& slot = *it;
		if ( slot.m_name.length() == 0 )
		{
			// End of local frame; stop looking.
			return itEnd;
		}
		else if ( slot.m_name == varname )
		{
			return it;
		}
	}
	return itEnd;
#else
	for (int i = m_local_frames.size() - 1; i >= 0; i--)
	{
		const frame_slot&       slot = m_local_frames[i];
		if (slot.m_name.length() == 0)
		{
			// End of local frame; stop looking.
			return endLocal();
		}
		else if (slot.m_name == varname)
		{
			// Found it.
			return beginLocal()+i;
		}
	}
	return endLocal();
#endif
}

}


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
