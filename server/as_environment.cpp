// as_environment.cpp:  Variable, Sprite, and Movie locators, for Gnash.
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
#include "gnashconfig.h"
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
#include "namedStrings.h"
#include "as_function.h" // for as_environment::CallFrame::markReachableResources

#include <string>
#include <utility> // for std::pair
#include <boost/algorithm/string/case_conv.hpp>

// Define this to have find_target() calls trigger debugging output
//#define DEBUG_TARGET_FINDING 1

// Define this to have get_variable() calls trigger debugging output
//#define GNASH_DEBUG_GET_VARIABLE 1

namespace gnash {

as_environment::CallStack as_environment::_localFrames = as_environment::CallStack();

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

    if ( parse_path(varname, path, var) )
    {
        // TODO: let find_target return generic as_objects, or use 'with' stack,
        //       see player2.swf or bug #18758 (strip.swf)
        // @@ TODO: should we use scopeStack here too ?
        as_object* target = find_object(path, &scopeStack); 

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
            log_aserror(_("find_object(\"%s\") [ varname = '%s' - current target = '%s' ] failed"),
                path.c_str(), varname.c_str(),
                m_target->get_text_value().c_str());
            as_value tmp = get_variable_raw(path, scopeStack, retTarget);
            if ( ! tmp.is_undefined() )
            {
                log_aserror(_("...but get_variable_raw(%s, <scopeStack>) succeeded (%s)!"),
                    path.c_str(), tmp.to_debug_string().c_str());
            }
            );
            return as_value(); // TODO: should we check get_variable_raw ?
        }
    }
    else
    {
	// TODO: have this checked by parse_path as an optimization 
	if ( varname.find_first_of('/') != std::string::npos && varname.find_first_of(':') == std::string::npos )
	{
		// Consider it all a path ...
        	as_object* target = find_object(varname, &scopeStack); 
		if ( target ) 
		{
			// ... but only if it resolves to a sprite
			sprite_instance* m = target->to_movie();
			if ( m ) return as_value(m);
		}
	}
        return get_variable_raw(varname, scopeStack, retTarget);
    }
}

as_value
as_environment::get_variable(const std::string& varname) const
{
	static ScopeStack empty_scopeStack;
	return get_variable(varname, empty_scopeStack);
}

static bool validRawVariableName(const std::string& varname)
{
	// check raw variable name validity
	const char* ptr = varname.c_str();
	for (;;)
	{
		ptr = strchr(ptr, ':');
		if ( ! ptr ) break;

		int num=1;
		while (*(++ptr) == ':') ++num;
		if (num>2) 
		{
			//log_debug("Invalid raw variable name...");
			return false;
		}
	} 

	return true;
}

as_value
as_environment::get_variable_raw(
    const std::string& varname,
    const ScopeStack& scopeStack, as_object** retTarget) const
    // varname must be a plain variable name; no path parsing.
{
    //assert(strchr(varname.c_str(), ':') == NULL);

	if ( ! validRawVariableName(varname) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Won't get invalid raw variable name: %s"), varname.c_str());
		);
		return as_value();
	}

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

    if ( swfVersion > 5 && key == NSV::PROP_uGLOBAL )
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
    IF_VERBOSE_ASCODING_ERRORS (
    log_aserror(_("reference to unexisting variable '%s'"),
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
    //log_debug(_("set_variable(%s, %s)"), varname.c_str(), val.to_debug_string().c_str());
    if ( parse_path(varname, path, var) )
    {
    	//log_debug(_("Variable '%s' parsed into path='%s', var='%s'"), varname.c_str(), path.c_str(), var.c_str());
	//target = find_target(path);
        target = find_object(path, &scopeStack); 
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

	if ( ! validRawVariableName(varname) )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Won't set invalid raw variable name: %s"), varname.c_str());
		);
		return;
	}

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
as_environment::parse_path(const std::string& var_path_in,
		std::string& path, std::string& var)
{
#ifdef DEBUG_TARGET_FINDING 
	log_debug("parse_path(%s)", var_path_in.c_str());
#endif

	size_t lastDotOrColon = var_path_in.find_last_of(":.");
	if ( lastDotOrColon == std::string::npos ) return false;

	std::string thePath, theVar;

	thePath.assign(var_path_in, 0, lastDotOrColon);
	theVar.assign(var_path_in, lastDotOrColon+1, var_path_in.length());

#ifdef DEBUG_TARGET_FINDING 
	log_debug("path: %s, var: %s", thePath.c_str(), theVar.c_str());
#endif

	if ( thePath.empty() ) return false;

	// this check should be performed by callers (getvariable/setvariable in particular)
	size_t pathlen = thePath.length();
	size_t i = pathlen-1;
	size_t contiguoscommas = 0;
	while ( i && thePath[i--] == ':' )
	{
		if ( ++contiguoscommas > 1 )
		{
#ifdef DEBUG_TARGET_FINDING 
			log_debug("path '%s' ends with too many colon chars, not considering a path", thePath.c_str());
#endif
			return false;
		}
	}

#ifdef DEBUG_TARGET_FINDING 
	log_debug("contiguoscommas: %d", contiguoscommas);
#endif

	//if ( var.empty() ) return false;

	path = thePath;
	var = theVar;

	return true;
}

bool
as_environment::parse_path(const std::string& var_path,
		as_object** target, as_value& val)
{
	std::string path;
	std::string var;
	if ( ! parse_path(var_path, path, var) ) return false;
        as_object* target_ptr = find_object(path); 
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
	} else if (*p == '.' || *p == '/' || *p == ':') {
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
	as_object* o = find_object(path_in);
	if ( o ) return o->to_character(); // can be NULL (not a character)...
	else return NULL;
}

as_object*
as_environment::find_object(const std::string& path_in, const ScopeStack* scopeStack) const
{
#ifdef DEBUG_TARGET_FINDING 
	log_debug(_("find_object(%s) called"), path_in.c_str());
#endif

    if (path_in.empty())
    {
#ifdef DEBUG_TARGET_FINDING 
	log_debug(_("Returning m_target (empty path)"));
#endif
	return m_target; // or should we return the *original* path ?
    }
    
    std::string path = PROPNAME(path_in);
    VM& vm = VM::get();
    string_table& st = vm.getStringTable();
    int swfVersion = vm.getSWFVersion(); 

    as_object* env = m_target; 
    assert(env);

    bool firstElementParsed = false;
    bool dot_allowed=true;

    const char*	p = path.c_str();
    if (*p == '/')
    {
	// Absolute path.  Start at the root.
	sprite_instance* root = m_target->get_root();
	if ( ! *(++p) )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug(_("Path is '/', return the root (%p)"), (void*)root);
#endif
		return root; // that's all folks.. 
	}

	env = root;
        firstElementParsed = true;
	dot_allowed = false;

#ifdef DEBUG_TARGET_FINDING 
	log_debug(_("Absolute path, start at the root (%p)"), (void*)env);
#endif

    }
#ifdef DEBUG_TARGET_FINDING 
    else
    {
	log_debug(_("Relative path, start at (%s)"), m_target->getTarget().c_str());
    }
#endif
    
    assert (*p);

    std::string	subpart;
    while (env)
    {
	while ( *p == ':' ) ++p;

	// No more components to scan
	if ( ! *p )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug(_("Path is %s, returning the root"), path.c_str());
#endif
		return env;
	}


	const char*	next_slash = next_slash_or_dot(p);
	subpart = p;
	if (next_slash == p)
	{
            IF_VERBOSE_ASCODING_ERRORS(
	    log_aserror(_("invalid path '%s' (p=next_slash=%s)"), path.c_str(), next_slash);
	    );
	    return NULL;
	}
	else if (next_slash)
	{
		if ( *next_slash == '.' )
		{
			if ( ! dot_allowed )
			{
				IF_VERBOSE_ASCODING_ERRORS(
				log_aserror(_("invalid path '%s' (dot not allowed after having seen a slash)"), path.c_str());
				);
				return NULL;
			}
		}
		else if ( *next_slash == '/' )
		{
			dot_allowed = false;
		}

		// Cut off the slash and everything after it.
		subpart.resize(next_slash - p);
	}
	
	assert(subpart[0] != ':');

	// No more components to scan
	if ( subpart.empty() )
	{
#ifdef DEBUG_TARGET_FINDING 
		log_debug(_("No more subparts, env is %p"), (void*)env);
#endif
		break;
	}

	string_table::key subpartKey = st.find(subpart);

	if ( ! firstElementParsed )
	{
		as_object* element = NULL;

		do {

			// Try scope stack
			if ( scopeStack )
			{
				for (size_t i = scopeStack->size(); i > 0; --i)
				{
					// const_cast needed due to non-const as_object::get_member 
					as_object* obj = const_cast<as_object*>((*scopeStack)[i-1].get());
					element = obj->get_path_element(subpartKey);
					if ( element ) break;
				}
				if ( element ) break;
			}

			// Try current target 
			assert(env == m_target);
			element = env->get_path_element(subpartKey);
			if ( element ) break;

			// Looking for _global ?
			as_object* global = VM::get().getGlobal();
			if ( swfVersion > 5 && subpartKey == NSV::PROP_uGLOBAL )
			{
				element = global;
				break;
			}

			// Try globals
			element = global->get_path_element(subpartKey);
			//if ( element ) break;

		} while (0);

		if ( ! element ) 
		{
#ifdef DEBUG_TARGET_FINDING 
			log_debug("subpart %s of path %s not found in any scope stack element", subpart.c_str(), path.c_str());
#endif
			return NULL;
		}

		env = element;
		firstElementParsed = true;
	}
	else
	{

#ifdef DEBUG_TARGET_FINDING 
		log_debug(_("Invoking get_path_element(%s) on object %p (%s)"), subpart.c_str(), (void *)env, env->get_text_value().c_str());
#endif

		as_object* element = env->get_path_element(subpartKey);
		if ( ! element )
		{
#ifdef DEBUG_TARGET_FINDING 
			log_debug(_("Path element %s not found in object %p"), subpart.c_str(), (void *)env);
#endif
			return NULL;
		}
		env = element;
	}

	if (next_slash == NULL)
	{
	    break;
	}
	
	p = next_slash + 1;
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
#ifndef DUMP_LOCAL_REGISTERS_IN_ALL_CALL_FRAMES
	dump(_localFrames.back().registers, out);
#else
	for (CallStack::const_iterator it=_localFrames.begin(),
			itEnd=_localFrames.end();
			it != itEnd; ++it)
	{
		if ( it != _localFrames.begin() ) out << " | ";
		dump(it->registers, out);
	}
#endif
	out << std::endl;
}

static void
dump(const as_environment::LocalVars& locals, std::ostream& out)
{
	typedef std::map<std::string, as_value> PropMap;
	PropMap props;
	const_cast<as_object*>(locals.get())->dump_members(props);
	
	//log_debug("FIXME: implement dumper for local variables now that they are simple objects");
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
#ifndef DUMP_LOCAL_VARIABLES_IN_ALL_CALL_FRAMES
	dump(_localFrames.back().locals, out);
#else
	for (CallStack::const_iterator it=_localFrames.begin(),
			itEnd=_localFrames.end();
			it != itEnd; ++it)
	{
		if ( it != _localFrames.begin() ) out << " | ";
		dump(it->locals, out);
	}
#endif
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
as_environment::popCallFrame()
{
	assert(!_localFrames.empty());
	_localFrames.pop_back();
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

void
as_environment::dump_stack(std::ostream& out, unsigned int limit) const
{
	unsigned int si=0, n=m_stack.size();
	if ( limit && n > limit )
	{
		si=n-limit;
		out << "Stack (last " << limit << " of " << n << " items): ";
	}
	else
	{
		out << "Stack: ";
	}

	for (unsigned int i=si; i<n; i++)
	{
		if (i!=si) out << " | ";
		out << '"' << m_stack[i] << '"';
	}
	out << std::endl;
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
	if (locals)
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
