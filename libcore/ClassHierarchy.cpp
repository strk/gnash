// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "as_object.h"
#include "as_prop_flags.h"
#include "as_value.h"
#include "namedStrings.h"
#include "ClassHierarchy.h"
#include "as_function.h"
#include "builtin_function.h"
#include "asClass.h"
#include "Object.h"
#include "extension.h"

namespace gnash {

// anonymous namespace
namespace { 

void
addVisibilityFlag(int& flags, int version)
{
    // TODO: more visibility for swf10+?
    switch (version)
    {
        default:
            return;
        case 9:
            flags |= as_prop_flags::onlySWF9Up;
            break;
        case 8:
            flags |= as_prop_flags::onlySWF8Up;
            break;
        case 7:
            flags |= as_prop_flags::onlySWF7Up;
            break;
        case 6:
            flags |= as_prop_flags::onlySWF6Up;
            break;
    }
}

class declare_extension_function : public as_function
{
private:
    ClassHierarchy::ExtensionClass mDeclaration;
    as_object *mTarget;
    Extension *mExtension;

public:
    bool isBuiltin() { return true; }

    declare_extension_function(ClassHierarchy::ExtensionClass &c,
        as_object *g, Extension* e)
        :
        mDeclaration(c),
        mTarget(g),
        mExtension(e)
    {
        init_member("constructor", as_function::getFunctionConstructor().get());
    }

    virtual as_value operator()(const fn_call& fn)
    {
        string_table& st = getStringTable(fn);
        log_debug("Loading extension class %s", st.value(mDeclaration.name));

        as_value super;
        if (mDeclaration.super_name)
        {
            // Check to be sure our super exists.
            // This will trigger its instantiation if necessary.
            if (!mTarget->get_member(mDeclaration.super_name, &super))
            {
                // Error here -- doesn't exist.
                log_error("Can't find %s (Superclass of %s)",
                    st.value(mDeclaration.super_name),
                    st.value(mDeclaration.name));
                super.set_undefined();
                return super;
            }
            if (!super.is_as_function())
            {
                // Error here -- not an object.
                log_error("%s (Superclass of %s) is not a function (%s)",
                    st.value(mDeclaration.super_name),
                    st.value(mDeclaration.name), super);
                super.set_undefined();
                return super;
            }
        }
        if (mExtension->initModuleWithFunc(mDeclaration.file_name,
            mDeclaration.init_name, *mTarget))
        {
            // Successfully loaded it, now find it, set its proto, and return.
            as_value us;
            mTarget->get_member(mDeclaration.name, &us);
            if (mDeclaration.super_name && !us.to_object()->hasOwnProperty(NSV::PROP_uuPROTOuu))
            {
                us.to_object()->set_prototype(super.to_as_function()->getPrototype());
            }
            return us;
        }
        // Error here -- not successful in loading.
        log_error("Could not load class %s", st.value(mDeclaration.name));
        super.set_undefined();
        return super;
    }
};

class declare_native_function : public as_function
{
private:
    ClassHierarchy::NativeClass mDeclaration;
    as_object *mTarget;

public:
    bool isBuiltin() { return true; }

    declare_native_function(const ClassHierarchy::NativeClass &c,
        as_object *g)
        :
        mDeclaration(c),
        mTarget(g)
    {
        // does it make any sense to set a 'constructor' here ??
        //init_member("constructor", this);
        //init_member("constructor", as_function::getFunctionConstructor().get());
    }

    virtual as_value operator()(const fn_call& fn)
    {
        string_table& st = getStringTable(fn);
        log_debug("Loading native class %s", st.value(mDeclaration.name));

        mDeclaration.initializer(*mTarget);
        // Successfully loaded it, now find it, set its proto, and return.
        as_value us;
        if (mTarget->get_member(mDeclaration.name, &us,
                    mDeclaration.namespace_name)) {

            as_value super;
            if (mDeclaration.super_name)
            {
                // Check to be sure our super exists.
                // This will trigger its instantiation if necessary.
                if (!mTarget->get_member(mDeclaration.super_name, &super))
                {
                    // Error here -- doesn't exist.
                    log_error("Can't find %s (Superclass of %s)",
                        st.value(mDeclaration.super_name),
                        st.value(mDeclaration.name));
                    super.set_undefined();
                    return super;
                }
                if (!super.is_as_function())
                {
                    // Error here -- not an object.
                    log_error("%s (Superclass of %s) is not a function (%s)",
                        st.value(mDeclaration.super_name),
                        st.value(mDeclaration.name), super);
                    super.set_undefined();
                    return super;
                }
                assert(super.to_as_function());
            }
            if (!us.to_object()) {
                log_error("Native class %s is not an object after "
                        "initialization (%s)", st.value(mDeclaration.name), us);
            }
            if (mDeclaration.super_name &&
                    !us.to_object()->hasOwnProperty(NSV::PROP_uuPROTOuu)) {
                
                us.to_object()->set_prototype(
                        super.to_as_function()->getPrototype());
            }
        }
        else
        {
            log_error("Native class %s is not found after initialization", 
                st.value(mDeclaration.name));
        }
        return us;
    }
};

} // end anonymous namespace

ClassHierarchy::~ClassHierarchy()
{
}

bool
ClassHierarchy::declareClass(ExtensionClass& c)
{
    if (!mExtension) return false; 

    mGlobalNamespace->stubPrototype(*this, c.name);
    mGlobalNamespace->getClass(c.name)->setDeclared();
    mGlobalNamespace->getClass(c.name)->setSystem();

    boost::intrusive_ptr<as_function> getter =
        new declare_extension_function(c, mGlobal, mExtension);


    int flags=as_prop_flags::dontEnum;
    addVisibilityFlag(flags, c.version);
    return mGlobal->init_destructive_property(c.name, *getter, flags);
}

bool
ClassHierarchy::declareClass(const NativeClass& c)
{
    // AS2 classes should be registered with namespace 0, so they all
    // appear in a single global namespace.
    asNamespace *nso = findNamespace(c.namespace_name);

    if (!nso) nso = addNamespace(c.namespace_name);

    nso->stubPrototype(*this, c.name);
    nso->getClass(c.name)->setDeclared();
    nso->getClass(c.name)->setSystem();

    boost::intrusive_ptr<as_function> getter =
        new declare_native_function(c, mGlobal);
    
    int flags = as_prop_flags::dontEnum;
    addVisibilityFlag(flags, c.version);
    return mGlobal->init_destructive_property(c.name, *getter, flags,
            c.namespace_name);
}


void
ClassHierarchy::declareAll(const NativeClasses& classes)
{
    // This is necessary to resolve the overload...
    bool(ClassHierarchy::*nf)(const NativeClass& f) =
        &ClassHierarchy::declareClass;

    std::for_each(classes.begin(), classes.end(), boost::bind(nf, this, _1));
}

void
ClassHierarchy::markReachableResources() const
{
    // TODO
}

std::ostream&
operator<<(std::ostream& os, const ClassHierarchy::NativeClass& c)
{
    string_table& st = VM::get().getStringTable();

    os << "("
        << " name:" << st.value(c.name)
        << " super:" << st.value(c.super_name)
        << " namespace:" << st.value(c.namespace_name)
        << " version:" << c.version
        << ")";

    return os;
}

std::ostream&
operator<<(std::ostream& os, const ClassHierarchy::ExtensionClass& c)
{
    string_table& st = VM::get().getStringTable();

    os << "(file:" << c.file_name
        << " init:" << c.init_name
        << " name:" << st.value(c.name)
        << " super:" << st.value(c.super_name)
        << " namespace:" << st.value(c.namespace_name)
        << " version:" << c.version
        << ")";

    return os;
}

} // end of namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
