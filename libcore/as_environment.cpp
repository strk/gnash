// as_environment.cpp:  Variable, Sprite, and Movie locators, for Gnash.
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

#include "as_environment.h"

#include <string>
#include <utility>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>

#include "MovieClip.h"
#include "movie_root.h"
#include "as_value.h"
#include "VM.h"
#include "log.h"
#include "Property.h"
#include "as_object.h"
#include "namedStrings.h"
#include "CallStack.h"
#include "Global_as.h"

// Define this to have find_target() calls trigger debugging output
//#define DEBUG_TARGET_FINDING 1

// Define this to have get_variable() calls trigger debugging output
//#define GNASH_DEBUG_GET_VARIABLE 1

namespace gnash {

namespace {
    /// Find a variable in the given as_object
    //
    /// @param varname
    /// Name of the local variable
    ///
    /// @param ret
    /// If a variable is found it's assigned to this parameter.
    /// Untouched if the variable is not found.
    ///
    /// @return true if the variable was found, false otherwise
    bool getLocal(as_object& locals, const std::string& name, as_value& ret);

    bool findLocal(as_object& locals, const std::string& varname, as_value& ret,
            as_object** retTarget);

    /// Delete a local variable
    //
    /// @param varname
    /// Name of the local variable
    ///
    /// @return true if the variable was found and deleted, false otherwise
    bool deleteLocal(as_object& locals, const std::string& varname);

    /// Set a variable of the given object, if it exists.
    //
    /// @param varname
    /// Name of the local variable
    ///
    /// @param val
    /// Value to assign to the variable
    ///
    /// @return true if the variable was found, false otherwise
    bool setLocal(as_object& locals, const std::string& varname,
        const as_value& val);

    as_object* getElement(as_object* obj, const ObjectURI& uri);

    /// @param retTarget
    /// If not NULL, the pointer will be set to the actual object containing the
    /// found variable (if found).
    as_value getVariableRaw(const as_environment& env,
        const std::string& varname,
        const as_environment::ScopeStack& scope,
        as_object** retTarget = 0);

    void setVariableRaw(const as_environment& env, const std::string& varname,
        const as_value& val, const as_environment::ScopeStack& scope);

    // Search for next '.' or '/' character in this word.  Return
    // a pointer to it, or null if it wasn't found.
    static const char* next_slash_or_dot(const char* word);

    static bool validRawVariableName(const std::string& varname);

}

as_value as_environment::undefVal;

as_environment::as_environment(VM& vm)
    :
    _vm(vm),
    _stack(_vm.getStack()),
    _target(0),
    _original_target(0)
{
}

as_object*
findObject(const as_environment& ctx, const std::string& path,
        const as_environment::ScopeStack* scope)
{
    if (path.empty()) {
        return getObject(ctx.target());
    }
    
    VM& vm = ctx.getVM();
    string_table& st = vm.getStringTable();
    const int swfVersion = vm.getSWFVersion();
    ObjectURI globalURI(NSV::PROP_uGLOBAL);

    bool firstElementParsed = false;
    bool dot_allowed = true;

    // This points to the current object being used for lookup.
    as_object* env; 
    const char* p = path.c_str();

    // Check if it's an absolute path
    if (*p == '/') {

        MovieClip* root = 0;
        if (ctx.target()) root = ctx.target()->getAsRoot();
        else {
            if (ctx.get_original_target()) {
                root = ctx.get_original_target()->getAsRoot();
            }
            return 0;
        }

        // If the path is just "/" return the root.
        if (!*(++p)) return getObject(root);

        // Otherwise we start at the root for lookup.
        env = getObject(root);
        firstElementParsed = true;
        dot_allowed = false;

    }
    else {
        env = getObject(ctx.target());
    }
    
    assert (*p);

    std::string subpart;

    while (1) {

        // Skip past all colons (why?)
        while (*p == ':') ++p;

        if (!*p) {
            // No more components to scan, so return the currently found
            // object.
            return env;
        }

        // Search for the next '/', ':' or '.'.
        const char* next_slash = next_slash_or_dot(p);
        subpart = p;

        // Check whether p was pointing to one of those characters already.
        if (next_slash == p) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("invalid path '%s' (p=next_slash=%s)"),
                path, next_slash);
            );
            return 0;
        }

        if (next_slash) {
            if (*next_slash == '.') {

                if (!dot_allowed) {
                    IF_VERBOSE_ASCODING_ERRORS(
                        log_aserror(_("invalid path '%s' (dot not allowed "
                                "after having seen a slash)"), path);
                    );
                    return 0;
                }
                // No dot allowed after a double-dot.
                if (next_slash[1] == '.') dot_allowed = false;
            }
            else if (*next_slash == '/') {
                dot_allowed = false;
            }

            // Cut off the slash and everything after it.
            subpart.resize(next_slash - p);
        }
        
        assert(subpart[0] != ':');

        // No more components to scan
        if (subpart.empty()) break;

        const ObjectURI subpartURI(getURI(vm, subpart));

        if (!firstElementParsed) {
            as_object* element(0);

            do {
                // Try scope stack
                if (scope) {
                    for (size_t i = scope->size(); i > 0; --i) {
                        as_object* obj = (*scope)[i-1];
                        
                        element = getElement(obj, subpartURI);
                        if (element) break;
                    }
                    if (element) break;
                }

                // Try current target  (if any)
                assert(env == getObject(ctx.target()));
                if (env) {
                    element = getElement(env, subpartURI);
                    if (element) break;
                }

                // Looking for _global ?
                as_object* global = vm.getGlobal();
                const bool nocase = caseless(*global);

                if (swfVersion > 5) {
                    const ObjectURI::CaseEquals ce(st, nocase);
                    if (ce(subpartURI, globalURI)) {
                        element = global;
                        break;
                    }
                }

                // Look for globals.
                element = getElement(global, subpartURI);

            } while (0);

            if (!element) return 0;

            env = element;
            firstElementParsed = true;
        }
        else {

            assert(env);
            as_object* element = getElement(env, subpartURI);
            if (!element) return 0;
            env = element;
        }

        if (!next_slash) break;
        
        p = next_slash + 1;
    }
    return env;
}

int
as_environment::get_version() const
{
    return _vm.getSWFVersion();
}

void
as_environment::markReachableResources() const
{
    if (_target) _target->setReachable();
    if (_original_target) _original_target->setReachable();
}

as_value
getVariable(const as_environment& env, const std::string& varname,
        const as_environment::ScopeStack& scope, as_object** retTarget)
{
    // Path lookup rigamarole.
    std::string path;
    std::string var;

    if (parsePath(varname, path, var)) {
        // TODO: let find_target return generic as_objects, or use 'with' stack,
        //       see player2.swf or bug #18758 (strip.swf)
        as_object* target = findObject(env, path, &scope); 

        if (target) {
            as_value val;
            target->get_member(getURI(env.getVM(), var), &val);
            if (retTarget) *retTarget = target;
            return val;
        }
        else {
            return as_value();
        }
    }

    if (varname.find('/') != std::string::npos &&
            varname.find(':') == std::string::npos) {

        // Consider it all a path ...
        as_object* target = findObject(env, varname, &scope); 
        if (target) {
            // ... but only if it resolves to a sprite
            DisplayObject* d = target->displayObject();
            MovieClip* m = d ? d->to_movie() : 0;
            if (m) return as_value(getObject(m));
        }
    }
    return getVariableRaw(env, varname, scope, retTarget);
}

void
setVariable(const as_environment& env, const std::string& varname,
    const as_value& val, const as_environment::ScopeStack& scope)
{
    IF_VERBOSE_ACTION(
        log_action("-------------- %s = %s", varname, val);
    );

    // Path lookup rigamarole.
    std::string path;
    std::string var;

    if (parsePath(varname, path, var)) {
        as_object* target = findObject(env, path, &scope); 
        if (target) {
            target->set_member(getURI(env.getVM(), var), val);
        }
        else {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Path target '%s' not found while setting %s=%s"),
                path, varname, val);
            );
        }
        return;
    }

    setVariableRaw(env, varname, val, scope);
}

bool
delVariable(const as_environment& ctx, const std::string& varname,
    const as_environment::ScopeStack& scope) 
{
    // varname must be a plain variable name; no path parsing.
    assert(varname.find_first_of(":/.") == std::string::npos);

    VM& vm = ctx.getVM();

    const ObjectURI& varkey = getURI(vm, varname);

    // Check the with-stack.
    for (size_t i = scope.size(); i > 0; --i) {
        as_object* obj = scope[i - 1];

        if (obj) {
            std::pair<bool, bool> ret = obj->delProperty(varkey);
            if (ret.first) {
                return ret.second;
            }
        }
    }

    // Check locals for deletion.
    if (vm.calling() && deleteLocal(vm.currentCall().locals(), varname)) {
        return true;
    }

    // Try target
    std::pair<bool, bool> ret = getObject(ctx.target())->delProperty(varkey);
    if (ret.first) {
        return ret.second;
    }

    // TODO: try 'this' ? Add a testcase for it !

    // Try _global 
    return vm.getGlobal()->delProperty(varkey).second;
}

bool
parsePath(const std::string& var_path_in, std::string& path, std::string& var)
{

    const size_t lastDotOrColon = var_path_in.find_last_of(":.");
    if (lastDotOrColon == std::string::npos) return false;

    const std::string p(var_path_in, 0, lastDotOrColon);
    const std::string v(var_path_in, lastDotOrColon + 1, var_path_in.size());

#ifdef DEBUG_TARGET_FINDING 
    log_debug("path: %s, var: %s", p, v);
#endif

    if (p.empty()) return false;

    // The path may apparently not end with more than one colon.
    if (p.size() > 1 && !p.compare(p.size() - 2, 2, "::")) return false;

    path = p;
    var = v;

    return true;
}

namespace {

static bool
validRawVariableName(const std::string& varname)
{
    if (varname.empty()) return false;

    if (varname[0] == '.') return false;
   
    if (varname[0] == ':' &&
            varname.find_first_of(":.", 1) == std::string::npos) {
        return false;
    }
    return (varname.find(":::") == std::string::npos);
}

// No path rigamarole.
void
setVariableRaw(const as_environment& env, const std::string& varname,
    const as_value& val, const as_environment::ScopeStack& scope)
{

    if (!validRawVariableName(varname)) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Won't set invalid raw variable name: %s"), varname);
        );
        return;
    }

    VM& vm = env.getVM();
    const ObjectURI& varkey = getURI(vm, varname);

    // in SWF5 and lower, scope stack should just contain 'with' elements 

    // Check the scope stack.
    for (size_t i = scope.size(); i > 0; --i) {
        as_object* obj = scope[i - 1];
        if (obj && obj->set_member(varkey, val, true)) {
            return;
        }
    }
    
    const int swfVersion = vm.getSWFVersion();
    if (swfVersion < 6 && vm.calling()) {
       if (setLocal(vm.currentCall().locals(), varname, val)) return;
    }
    
    // TODO: shouldn't _target be in the scope chain ?
    if (env.target()) getObject(env.target())->set_member(varkey, val);
    else if (env.get_original_target()) {
        getObject(env.get_original_target())->set_member(varkey, val);
    }
    else {
        log_error("as_environment::setVariableRaw(%s, %s): "
           "neither current target nor original target are defined, "
           "can't set the variable",
           varname, val);
    }
}

as_value
getVariableRaw(const as_environment& env, const std::string& varname,
    const as_environment::ScopeStack& scope, as_object** retTarget)
{

    if (!validRawVariableName(varname)) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Won't get invalid raw variable name: %s"), varname);
        );
        return as_value();
    }

    as_value val;

    VM& vm = env.getVM();
    const int swfVersion = vm.getSWFVersion();
    const ObjectURI& key = getURI(vm, varname);

    // Check the scope stack.
    for (size_t i = scope.size(); i > 0; --i) {

        as_object* obj = scope[i - 1];
        if (obj && obj->get_member(key, &val)) {
            if (retTarget) *retTarget = obj;
            return val;
        }
    }

    // Check locals for getting them
    // for SWF6 and up locals should be in the scope stack
    if (swfVersion < 6 && vm.calling()) {
       if (findLocal(vm.currentCall().locals(), varname, val, retTarget)) {
           return val;
       }
    }

    // Check current target members. TODO: shouldn't target be in scope stack ?
    if (env.target()) {
        as_object* obj = getObject(env.target());
        assert(obj);
        if (obj->get_member(key, &val)) {
            if (retTarget) *retTarget = obj;
            return val;
        }
    }
    else if (env.get_original_target()) {
        as_object* obj = getObject(env.get_original_target());
        assert(obj);
        if (obj->get_member(key, &val)) {
            if (retTarget) *retTarget = obj;
            return val;
        }
    }

    // AS1 has neither "this" nor any global object.
    if (swfVersion < 5) return as_value();

    const ObjectURI::CaseEquals eq(getVM(env).getStringTable(),
            swfVersion < 7);

    // Looking for "this" 
    if (eq(key, NSV::PROP_THIS)) {
        val.set_as_object(getObject(env.get_original_target()));
        if (retTarget) *retTarget = NULL; 
        return val;
    }

    as_object* global = vm.getGlobal();
    if (swfVersion > 5 && eq(key, NSV::PROP_uGLOBAL)) {
#ifdef GNASH_DEBUG_GET_VARIABLE
        log_debug("Took %s as _global, returning _global", varname);
#endif
        // The "_global" ref was added in SWF6
        if (retTarget) *retTarget = NULL; // correct ??
        return as_value(global);
    }

    if (global->get_member(key, &val)) {
#ifdef GNASH_DEBUG_GET_VARIABLE
        log_debug("Found %s in _global", varname);
#endif
        if (retTarget) *retTarget = global;
        return val;
    }
    
    // Fallback.
    // FIXME, should this be log_error?  or log_swferror?
    IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("reference to non-existent variable '%s'"), varname);
    );

    return as_value();
}

bool
getLocal(as_object& locals, const std::string& name, as_value& ret)
{
    return locals.get_member(getURI(getVM(locals), name), &ret);
}

bool
findLocal(as_object& locals, const std::string& varname, as_value& ret,
        as_object** retTarget) 
{

    if (getLocal(locals, varname, ret)) {
        if (retTarget) *retTarget = &locals;
        return true;
    }

    return false;
}

bool
deleteLocal(as_object& locals, const std::string& varname)
{
    return locals.delProperty(getURI(getVM(locals), varname)).second;
}

bool
setLocal(as_object& locals, const std::string& varname, const as_value& val)
{
    Property* prop = locals.getOwnProperty(getURI(getVM(locals), varname));
    if (!prop) return false;
    prop->setValue(locals, val);
    return true;
}

as_object*
getElement(as_object* obj, const ObjectURI& uri)
{
    DisplayObject* d = obj->displayObject();
    if (d) return d->pathElement(uri);
    return getPathElement(*obj, uri);
}

static const char*
next_slash_or_dot(const char* word)
{
    for (const char* p = word; *p; p++) {
        if (*p == '.' && p[1] == '.') {
            ++p;
        }
        else if (*p == '.' || *p == '/' || *p == ':') {
            return p;
        }
    }
    return 0;
}

} // unnamed namespace 

DisplayObject*
findTarget(const as_environment& env, const std::string& path)
{
    return get<DisplayObject>(findObject(env, path));
}


string_table&
getStringTable(const as_environment& env)
{
    return env.getVM().getStringTable();
}

movie_root&
getRoot(const as_environment& env)
{
    return env.getVM().getRoot();
}

Global_as&
getGlobal(const as_environment& env)
{
    return *env.getVM().getGlobal();
}

int
getSWFVersion(const as_environment& env)
{
    return env.getVM().getSWFVersion();
}

} // end of gnash namespace



// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
