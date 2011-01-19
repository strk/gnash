// Object.cpp:  Implementation of ActionScript Object class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "Movie.h"
#include "DisplayObject.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "NativeFunction.h" 
#include "movie_definition.h" 
#include "sprite_definition.h"
#include "VM.h" 
#include "namedStrings.h" // for NSV::PROP_TO_STRING
#include "Global_as.h"
#include "Object.h"

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

// Forward declarations
namespace {

    as_value object_addproperty(const fn_call&);
    as_value object_registerClass(const fn_call& fn);
    as_value object_hasOwnProperty(const fn_call&);
    as_value object_isPropertyEnumerable(const fn_call&);
    as_value object_isPrototypeOf(const fn_call&);
    as_value object_watch(const fn_call&);
    as_value object_unwatch(const fn_call&);
    as_value object_toLocaleString(const fn_call&);
    as_value object_ctor(const fn_call& fn);
    as_value object_valueOf(const fn_call& fn);
    as_value object_toString(const fn_call& fn);

    void attachObjectInterface(as_object& o);

}


void
registerObjectNative(as_object& global)
{
    VM& vm = getVM(global);

    vm.registerNative(object_watch, 101, 0); 
    vm.registerNative(object_unwatch, 101, 1); 
    vm.registerNative(object_addproperty, 101, 2); 
    vm.registerNative(object_valueOf, 101, 3); 
    vm.registerNative(object_toString, 101, 4); 
    vm.registerNative(object_hasOwnProperty, 101, 5); 
    vm.registerNative(object_isPrototypeOf, 101, 6); 
    vm.registerNative(object_isPropertyEnumerable, 101, 7); 
    vm.registerNative(object_registerClass, 101, 8);
    vm.registerNative(object_ctor, 101, 9);
}

// extern (used by Global.cpp)
void
initObjectClass(as_object* proto, as_object& where, const ObjectURI& uri)
{

    assert(proto);

    // Object is a native constructor.
    VM& vm = getVM(where);
    as_object* cl = vm.getNative(101, 9);
    cl->init_member(NSV::PROP_PROTOTYPE, proto);
    proto->init_member(NSV::PROP_CONSTRUCTOR, cl);

    attachObjectInterface(*proto);

    // The as_function ctor takes care of initializing these, but they
    // are different for the Object class.
    const int readOnly = PropFlags::readOnly;
    cl->set_member_flags(NSV::PROP_uuPROTOuu, readOnly);
    cl->set_member_flags(NSV::PROP_CONSTRUCTOR, readOnly);
    cl->set_member_flags(NSV::PROP_PROTOTYPE, readOnly);

    const int readOnlyFlags = as_object::DefaultFlags | PropFlags::readOnly;
    cl->init_member("registerClass", vm.getNative(101, 8), readOnlyFlags);
             
    // Register _global.Object (should only be visible in SWF5 up)
    int flags = PropFlags::dontEnum; 
    where.init_member(uri, cl, flags);

}


namespace {

void
attachObjectInterface(as_object& o)
{
    VM& vm = getVM(o);

    // We register natives despite swf version,
    Global_as& gl = getGlobal(o);

    o.init_member("valueOf", vm.getNative(101, 3));
    o.init_member("toString", vm.getNative(101, 4));
    o.init_member("toLocaleString", gl.createFunction(object_toLocaleString));

    int swf6flags = PropFlags::dontEnum | 
        PropFlags::dontDelete | 
        PropFlags::onlySWF6Up;

    o.init_member("addProperty", vm.getNative(101, 2), swf6flags);
    o.init_member("hasOwnProperty", vm.getNative(101, 5), swf6flags);
    o.init_member("isPropertyEnumerable", vm.getNative(101, 7), swf6flags);
    o.init_member("isPrototypeOf", vm.getNative(101, 6), swf6flags);
    o.init_member("watch", vm.getNative(101, 0), swf6flags);
    o.init_member("unwatch", vm.getNative(101, 1), swf6flags);
}


as_value
object_ctor(const fn_call& fn)
{

    if (fn.nargs == 1) {
        as_object* obj = toObject(fn.arg(0), getVM(fn));
        if (obj) return as_value(obj);
    }

    if (fn.nargs > 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Too many args to Object constructor"));
        );
    }

    Global_as& gl = getGlobal(fn);

    if (!fn.isInstantiation()) {
        return new as_object(gl);
    }

    return as_value();

}

/// Object.toString returns one of two values: [type Function] if it is a 
/// function, [object Object] if it is an object. Gnash wrongly regards super
/// as a function, so we have to handle that too.
as_value
object_toString(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    return as_value(obj->stringValue());
}

as_value
object_valueOf(const fn_call& fn)
{
    return fn.this_ptr;
}


/// The return value is not dependent on the result of add_property (though
/// this is always true anyway), but rather on the validity of the arguments.
as_value
object_addproperty(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    /// Extra arguments are just ignored.
    if ( fn.nargs < 3 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss;
        fn.dump_args(ss);
        log_aserror(_("Invalid call to Object.addProperty(%s) - "
            "expected 3 arguments (<name>, <getter>, <setter>)"),
                   ss.str());
        );

        // if we've been given more args then needed there's
        // no need to abort here
        if ( fn.nargs < 3 )
        {
            return as_value(false);
        }
    }

    const std::string& propname = fn.arg(0).to_string();
    if (propname.empty())
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Invalid call to Object.addProperty() - "
            "empty property name"));
        );
        return as_value(false);
    }

    as_function* getter = fn.arg(1).to_function();
    if (!getter)
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Invalid call to Object.addProperty() - "
            "getter is not an AS function"));
        );
        return as_value(false);
    }

    as_function* setter = NULL;
    const as_value& setterval = fn.arg(2);
    if (!setterval.is_null())
    {
        setter = setterval.to_function();
        if (!setter)
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Invalid call to Object.addProperty() - "
                "setter is not null and not an AS function (%s)"),
                setterval);
            );
            return as_value(false);
        }
    }

    // Now that we checked everything, let's call the as_object
    // interface for getter/setter properties :)
    obj->add_property(propname, *getter, setter);

    return as_value(true);
}


as_value
object_registerClass(const fn_call& fn)
{

    if (fn.nargs != 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Invalid call to Object.registerClass(%s) - "
                "expected 2 arguments (<symbol>, <constructor>)"),
                ss.str());
        );

        // if we've been given more args then needed there's
        // no need to abort here
        if (fn.nargs < 2) {
            return as_value(false);
        }
    }

    const std::string& symbolid = fn.arg(0).to_string();
    if (symbolid.empty())
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Invalid call to Object.registerClass(%s) - "
                "first argument (symbol id) evaluates to empty string"),
                ss.str());
        );
        return as_value(false);
    }

    as_function* theclass = fn.arg(1).to_function();
    if (!theclass) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Invalid call to Object.registerClass(%s) - "
                "second argument (class) is not a function)"), ss.str());
            );
        return as_value(false);
    }

    // Find the exported resource

    // Using definition of current target fixes the youtube beta case
    // https://savannah.gnu.org/bugs/index.php?23130
    DisplayObject* tgt = fn.env().target();
    if (!tgt) {
        log_error("current environment has no target, wouldn't know "
                "where to look for symbol required for registerClass"); 
        return as_value(false);
    }

    Movie* relRoot = tgt->get_root();
    assert(relRoot);
    const movie_definition* def = relRoot->definition();
    
    // We only care about definitions, not other exportable resources.
    const boost::uint16_t id = def->exportID(symbolid);
    SWF::DefinitionTag* d = def->getDefinitionTag(id);

    if (!d) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Object.registerClass(%s, %s): "
                "can't find exported symbol"),
                symbolid, typeName(theclass));
            );
        return as_value(false);
    }

    // Check that the exported resource is a sprite_definition
    // (we're looking for a MovieClip symbol)
    sprite_definition* exp_clipdef(dynamic_cast<sprite_definition*>(d));

    if (!exp_clipdef) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Object.registerClass(%s, %s): "
            "exported symbol is not a MovieClip symbol "
            "(sprite_definition), but a %s"),
            symbolid, typeName(theclass), typeName(d));
        );
        return as_value(false);
    }

    exp_clipdef->registerClass(theclass);
    return as_value(true);
}


as_value
object_hasOwnProperty(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Object.hasOwnProperty() requires one arg"));
        );
        return as_value(false);
    }
    const as_value& arg = fn.arg(0);
    const std::string& propname = arg.to_string();
    if (arg.is_undefined() || propname.empty())
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Invalid call to Object.hasOwnProperty('%s')"), arg);
        );
        return as_value(false);
    }

    const bool found = hasOwnProperty(*obj, getURI(getVM(fn), propname));
    return as_value(found);
}

as_value
object_isPropertyEnumerable(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Object.isPropertyEnumerable() requires one arg"));
        );
        return as_value();
    }
    const as_value& arg = fn.arg(0);
    const std::string& propname = arg.to_string();
    if (arg.is_undefined() || propname.empty()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("Invalid call to Object.isPropertyEnumerable('%s')"),
                arg);
            );
        return as_value();
    }

    Property* prop = obj->getOwnProperty(getURI(getVM(fn),propname));

    if (!prop) {
        return as_value(false);
    }

    return as_value(!prop->getFlags().test<PropFlags::dontEnum>());
}


as_value
object_isPrototypeOf(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Object.isPrototypeOf() requires one arg"));
        );
        return as_value(false); 
    }

    as_object* arg = toObject(fn.arg(0), getVM(fn));
    if (!arg) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("First arg to Object.isPrototypeOf(%s) is "
                    "not an object"), fn.arg(0));
            );
        return as_value(false);
    }

    return as_value(obj->prototypeOf(*arg));

}


as_value
object_watch(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if ( fn.nargs < 2 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Object.watch(%s): missing arguments"));
        );
        return as_value(false);
    }

    const as_value& propval = fn.arg(0);
    const as_value& funcval = fn.arg(1);

    if ( ! funcval.is_function() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Object.watch(%s): second argument is not a function"));
        );
        return as_value(false);
    }

    VM& vm = getVM(fn);

    std::string propname = propval.to_string();
    const ObjectURI& propkey = getURI(vm, propname);
    as_function* trig = funcval.to_function();
    as_value cust; if ( fn.nargs > 2 ) cust = fn.arg(2);

    return as_value(obj->watch(propkey, *trig, cust));
}


as_value
object_unwatch(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror(_("Object.unwatch(%s): missing argument"));
        );
        return as_value(false);
    }

    const as_value& propval = fn.arg(0);

    VM& vm = getVM(fn);

    std::string propname = propval.to_string();
    const ObjectURI& propkey = getURI(vm, propname);

    return as_value(obj->unwatch(propkey));
}


as_value
object_toLocaleString(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    return callMethod(obj, NSV::PROP_TO_STRING);
}
  
} // anonymous namespace
} // namespace gnash
