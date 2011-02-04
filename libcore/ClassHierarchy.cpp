// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "ClassHierarchy.h"

#include <boost/bind.hpp>

#include "as_object.h"
#include "PropFlags.h"
#include "as_value.h"
#include "namedStrings.h"
#include "as_function.h"
#include "Global_as.h"
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
            flags |= PropFlags::onlySWF9Up;
            break;
        case 8:
            flags |= PropFlags::onlySWF8Up;
            break;
        case 7:
            flags |= PropFlags::onlySWF7Up;
            break;
        case 6:
            flags |= PropFlags::onlySWF6Up;
            break;
    }
}

class declare_extension_function : public as_function
{
private:
    ClassHierarchy::ExtensionClass _decl;
    as_object *mTarget;
    Extension *mExtension;

public:
    bool isBuiltin() { return true; }

    declare_extension_function(ClassHierarchy::ExtensionClass &c, as_object *g,
            Extension* e)
        :
        as_function(getGlobal(*g)),
        _decl(c),
        mTarget(g),
        mExtension(e)
    {
    }

    virtual as_value call(const fn_call& fn)
    {
        string_table& st = getStringTable(fn);
        log_debug("Loading extension class %s", st.value(getName(_decl.uri)));

        if (mExtension->initModuleWithFunc(_decl.file_name,
            _decl.init_name, *mTarget))
        {
            // Successfully loaded it, now find it, set its proto, and return.
            as_value us;
            mTarget->get_member(_decl.uri, &us);
            return us;
        }
        // Error here -- not successful in loading.
        log_error("Could not load class %s", st.value(getName(_decl.uri)));
        return as_value();
    }
};

class declare_native_function : public as_function
{

public:

    bool isBuiltin() { return true; }

    declare_native_function(const ClassHierarchy::NativeClass &c, as_object *g)
        :
        as_function(getGlobal(*g)),
        _decl(c),
        mTarget(g)
    {
    }

    virtual as_value call(const fn_call& fn)
    {
        string_table& st = getStringTable(fn);
        log_debug("Loading native class %s", st.value(getName(_decl.uri)));

        _decl.initializer(*mTarget, _decl.uri);
        // Successfully loaded it, now find it, set its proto, and return.
        as_value us;
        if (mTarget->get_member(_decl.uri, &us)) {
            if (!toObject(us, getVM(fn))) {
                log_error("Native class %s is not an object after "
                        "initialization (%s)",
                        st.value(getName(_decl.uri)), us);
            }
        }
        else
        {
            log_error("Native class %s is not found after initialization", 
                st.value(getName(_decl.uri)));
        }
        return us;
    }

private:

    ClassHierarchy::NativeClass _decl;
    as_object *mTarget;

};

} // end anonymous namespace

ClassHierarchy::~ClassHierarchy()
{
}

bool
ClassHierarchy::declareClass(ExtensionClass& c)
{
    if (!mExtension) return false; 

    as_function* getter(new declare_extension_function(c, mGlobal, mExtension));

    int flags = PropFlags::dontEnum;
    addVisibilityFlag(flags, c.version);
    return mGlobal->init_destructive_property(c.uri, *getter, flags);
}

bool
ClassHierarchy::declareClass(const NativeClass& c)
{
    as_function* getter = new declare_native_function(c, mGlobal);
    
    int flags = PropFlags::dontEnum;
    addVisibilityFlag(flags, c.version);
    return mGlobal->init_destructive_property(c.uri, *getter, flags);
}


void
ClassHierarchy::declareAll(const NativeClasses& classes)
{
    // This is necessary to resolve the overload...
    bool(ClassHierarchy::*nf)(const NativeClass& f) =
        &ClassHierarchy::declareClass;

    std::for_each(classes.begin(), classes.end(), boost::bind(nf, this, _1));
}

} // end of namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
