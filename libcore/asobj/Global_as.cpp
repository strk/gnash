// Global.cpp:  Global ActionScript class setup, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Global_as.h"

#include <map>
#include <limits> 
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>

#include "as_object.h"
#include "builtin_function.h"
#include "movie_root.h"
#include "PropFlags.h"
#include "as_value.h"
#include "as_function.h"
#include "NativeFunction.h" 
#include "AsBroadcaster.h"
#include "Boolean_as.h"
#include "Color_as.h"
#include "Date_as.h" 
#include "Array_as.h" 
#include "Error_as.h"
#include "String_as.h"
#include "Selection_as.h"
#include "Number_as.h"
#include "Math_as.h"
#include "Accessibility_as.h"
#include "ContextMenu_as.h"
#include "ContextMenuItem_as.h"
#include "Key_as.h"
#include "Mouse_as.h"
#include "Microphone_as.h"
#include "Sound_as.h"
#include "Camera_as.h"
#include "Stage_as.h"
#include "MovieClip_as.h"
#include "Function_as.h"
#include "flash/display/BitmapData_as.h"
#include "flash/filters/BitmapFilter_as.h"
#include "flash/geom/ColorTransform_as.h"
#include "LocalConnection_as.h"
#include "XMLSocket_as.h"
#include "SharedObject_as.h"
#include "NetConnection_as.h"
#include "NetStream_as.h"
#include "System_as.h"
#include "TextSnapshot_as.h"
#include "TextField_as.h"
#include "TextFormat_as.h"
#include "flash/text/TextRenderer_as.h"
#include "XML_as.h"
#include "XMLNode_as.h"
#include "flash/external/ExternalInterface_as.h"
#include "MovieClipLoader.h"
#include "movie_definition.h"
#include "Video.h"
#include "extension.h"
#include "VM.h"
#include "Timers.h"
#include "URL.h" 
#include "rc.h"
#include "ClassHierarchy.h"
#include "namedStrings.h"
#include "GnashNumeric.h" // for isfinite replacement
#include "flash_pkg.h"
#include "fn_call.h"
#include "Button.h"
#include "LoadVars_as.h"
#include "Object.h"
#include "LoadableObject.h"

// Common code to warn and return if a required single arg is not present
// and to warn if there are extra args.
#define ASSERT_FN_ARGS_IS_1                        \
    if (fn.nargs < 1) {                            \
    IF_VERBOSE_ASCODING_ERRORS(                    \
            log_aserror(_("%s needs one argument"), __FUNCTION__);        \
            )                                \
         return as_value();                            \
    }                                    \
    IF_VERBOSE_ASCODING_ERRORS(                        \
    if (fn.nargs > 1)                        \
            log_aserror(_("%s has more than one argument"), __FUNCTION__);    \
    )

namespace gnash {

namespace {

    const ClassHierarchy::NativeClasses& avm1Classes();

    as_value global_trace(const fn_call& fn);
    as_value global_isNaN(const fn_call& fn);
    as_value global_isfinite(const fn_call& fn);
    as_value global_unescape(const fn_call& fn);
    as_value global_escape(const fn_call& fn);
    as_value global_parsefloat(const fn_call& fn);
    as_value global_parseint(const fn_call& fn);
    as_value global_assetpropflags(const fn_call& fn);
    as_value global_assetuperror(const fn_call& fn);
    as_value global_asnative(const fn_call& fn);
    as_value global_asnew(const fn_call& fn);
    as_value global_assetnative(const fn_call& fn);
    as_value global_assetnativeaccessor(const fn_call& fn);
    as_value global_asconstructor(const fn_call& fn);
    as_value global_updateAfterEvent(const fn_call& fn);
    as_value global_setTimeout(const fn_call& fn);
    as_value global_clearInterval(const fn_call& fn);
    as_value global_setInterval(const fn_call& fn);
    
    // These are present in the standalone, not sure about the plugin.
    as_value global_enableDebugConsole(const fn_call& fn);
    as_value global_showRedrawRegions(const fn_call& fn);

    // This is a help function for the silly AsSetupError function.
    as_value local_errorConstructor(const fn_call& fn);
    
    void registerNatives(as_object& global);
}

Global_as::Global_as(VM& vm)
    :
    as_object(vm),
    _et(new Extension()),
    _classes(this, _et.get()),
    _objectProto(new as_object(*this))
{
}
	
Global_as::~Global_as()
{
}

as_function*
Global_as::createFunction(Global_as::ASFunction function)
{
    as_object* proto = createObject(*this);
    builtin_function* f = new builtin_function(*this, function);
    
    proto->init_member(NSV::PROP_CONSTRUCTOR, f); 
    
    f->init_member(NSV::PROP_PROTOTYPE, proto);

    as_function* fun =
        gnash::getOwnProperty(*this, NSV::CLASS_FUNCTION).to_function();
    if (fun) {
        const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
        f->init_member(NSV::PROP_uuPROTOuu, getMember(*fun,
                    NSV::PROP_PROTOTYPE), flags);
        f->init_member(NSV::PROP_CONSTRUCTOR, fun);
    }
    return f;
}

as_object*
Global_as::createClass(Global_as::ASFunction ctor, as_object* prototype)
{
    as_object* cl = new builtin_function(*this, ctor);
    
    if (prototype) {
        prototype->init_member(NSV::PROP_CONSTRUCTOR, cl); 
        cl->init_member(NSV::PROP_PROTOTYPE, prototype);
    }
    as_function* fun = 
        gnash::getOwnProperty(*this, NSV::CLASS_FUNCTION).to_function();

    if (fun) {
        const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
        cl->init_member(NSV::PROP_uuPROTOuu, getMember(*fun,
                    NSV::PROP_PROTOTYPE), flags);
        cl->init_member(NSV::PROP_CONSTRUCTOR, fun);
    }
    return cl;
}


/// Construct an Array.
//
/// This uses the _global Array class to initialize the "constructor" and
/// "__proto__" properties. If Array.prototype is undefined, those properties
/// are not added.
as_object*
Global_as::createArray()
{
    as_object* array = new as_object(*this);

    as_value ctor = getMember(*this, NSV::CLASS_ARRAY);
    as_object* obj = toObject(ctor, gnash::getVM(*this));
    if (obj) {
        as_value proto;
        if (obj->get_member(NSV::PROP_PROTOTYPE, &proto)) {
            array->init_member(NSV::PROP_CONSTRUCTOR, ctor);
            array->set_prototype(getMember(*obj, NSV::PROP_PROTOTYPE));
        }
    }

    array->init_member(NSV::PROP_LENGTH, 0.0);
    array->setArray();
    return array;
}

void 
Global_as::markReachableResources() const
{
    _classes.markReachableResources();
    _objectProto->setReachable();
    as_object::markReachableResources();
}

void
Global_as::registerClasses()
{
    registerNatives(*this);

    function_class_init(*this, NSV::CLASS_FUNCTION);
    initObjectClass(_objectProto, *this, NSV::CLASS_OBJECT); 

    string_class_init(*this, NSV::CLASS_STRING); 
    array_class_init(*this, NSV::CLASS_ARRAY); 

    // No idea why, but it seems there's a NULL _global.o 
    // defined at player startup...
    // Probably due to the AS-based initialization 
    // Not enumerable but overridable and deletable.
    //
    as_value nullVal; nullVal.set_null();
    init_member("o", nullVal, PropFlags::dontEnum);


    VM& vm = getVM();

    // _global functions.            
    // These functions are only available in SWF6+, but this is just
    // because SWF5 or lower did not have a "_global"
    // reference at all.
    init_member("ASnative", createFunction(global_asnative));
    init_member("ASconstructor", createFunction(global_asconstructor));
    init_member("ASSetPropFlags", vm.getNative(1, 0));
    init_member("ASSetNative", vm.getNative(4, 0));
    init_member("ASSetNativeAccessor", vm.getNative(4, 1));
    init_member("AsSetupError", createFunction(global_assetuperror));
    init_member("updateAfterEvent", vm.getNative(9, 0));
    init_member("trace", vm.getNative(100, 4));

    init_member("setInterval", vm.getNative(250, 0));
    init_member("clearInterval", vm.getNative(250, 1));
    init_member("setTimeout", vm.getNative(250, 2));
 
    // This is an odd function with no properties. There ought to be
    // a better way of implementing this. See also TextFormat.getTextExtent.
    as_function* edc = createFunction(global_enableDebugConsole);
    edc->clearProperties();
    init_member("enableDebugConsole", edc);
    init_member("showRedrawRegions", vm.getNative(1021, 1));
    
    init_member("clearTimeout", getMember(*this, getURI(vm, "clearInterval")));

    _classes.declareAll(avm1Classes());

    // SWF8 visibility:
    const ObjectURI& flash = getURI(vm, "flash");
    flash_package_init(*this, flash); 

    const int version = vm.getSWFVersion();

    if (version > 4) {
            // This is surely not correct, but they are not available
            // in SWF4
            init_member("escape", vm.getNative(100, 0));
            init_member("unescape", vm.getNative(100, 1));
            init_member("parseInt", vm.getNative(100, 2));
            init_member("parseFloat", vm.getNative(100, 3));
            init_member("isNaN", vm.getNative(200, 18));
            init_member("isFinite", vm.getNative(200, 19));

            init_member("NaN", as_value(NaN));
            init_member("Infinity", as_value(
                        std::numeric_limits<double>::infinity()));
    }

    loadExtensions();
}

as_object*
createObject(const Global_as& gl)
{
    as_object* obj = new as_object(gl);
    gl.makeObject(*obj);
    return obj;
}

//-----------------------
// Extensions
//-----------------------
// Scan the plugin directories for all plugins, and load them now.
// FIXME: this should actually be done dynamically, and only
// if a plugin defines a class that a movie actually wants to
// use.
void
Global_as::loadExtensions()
{
    if (RcInitFile::getDefaultInstance().enableExtensions()) {
        log_security(_("Extensions enabled, scanning plugin dir for load"));
        _et->scanAndLoad(*this);
    }
    else {
        log_security(_("Extensions disabled"));
    }
}

void
Global_as::makeObject(as_object& o) const
{
    o.set_prototype(_objectProto);
}

namespace {

const ClassHierarchy::NativeClasses&
avm1Classes()
{
    typedef ClassHierarchy::NativeClass N;

    // Since we maintain separate lists for AVM1 and AVM2, these are all
    // considered to be in the 'Global' namespace (AVM1 has no namespaces)
    // An ObjectURI constructed without a namespace is in the global namespace.
    static const ClassHierarchy::NativeClasses s = boost::assign::list_of

        (N(system_class_init, NSV::CLASS_SYSTEM, 1))
        (N(stage_class_init, NSV::CLASS_STAGE, 1))
        (N(movieclip_class_init, NSV::CLASS_MOVIE_CLIP, 3))
        (N(textfield_class_init, NSV::CLASS_TEXT_FIELD, 3))
        (N(math_class_init, NSV::CLASS_MATH, 4))
        (N(boolean_class_init, NSV::CLASS_BOOLEAN, 5))
        (N(button_class_init, NSV::CLASS_BUTTON, 5))
        (N(color_class_init, NSV::CLASS_COLOR, 5))
        (N(selection_class_init, NSV::CLASS_SELECTION, 5))
        (N(sound_class_init, NSV::CLASS_SOUND, 5))
        (N(xmlsocket_class_init, NSV::CLASS_XMLSOCKET, 5))
        (N(date_class_init, NSV::CLASS_DATE, 5))
        (N(xmlnode_class_init, NSV::CLASS_XMLNODE, 5))
        (N(xml_class_init, NSV::CLASS_XML, 5))
        (N(mouse_class_init, NSV::CLASS_MOUSE, 5))
        (N(number_class_init, NSV::CLASS_NUMBER, 5))
        (N(textformat_class_init, NSV::CLASS_TEXT_FORMAT, 5))
        (N(key_class_init, NSV::CLASS_KEY, 5))
        (N(AsBroadcaster::init, NSV::CLASS_AS_BROADCASTER, 5))
        (N(textsnapshot_class_init, NSV::CLASS_TEXT_SNAPSHOT, 5))
        (N(video_class_init, NSV::CLASS_VIDEO, 6))
        (N(camera_class_init, NSV::CLASS_CAMERA, 5))
        (N(microphone_class_init, NSV::CLASS_MICROPHONE, 5))
        (N(sharedobject_class_init, NSV::CLASS_SHARED_OBJECT, 5))
        (N(loadvars_class_init, NSV::CLASS_LOAD_VARS, 5))
        (N(localconnection_class_init, NSV::CLASS_LOCALCONNECTION, 6))
        (N(netconnection_class_init, NSV::CLASS_NET_CONNECTION, 6))
        (N(netstream_class_init, NSV::CLASS_NET_STREAM, 6))
        (N(contextmenu_class_init, NSV::CLASS_CONTEXTMENU, 5))
        (N(contextmenuitem_class_init, NSV::CLASS_CONTEXTMENUITEM, 5))
        (N(moviecliploader_class_init, NSV::CLASS_MOVIE_CLIP_LOADER, 5))
        (N(Error_class_init, NSV::CLASS_ERROR, 5))
        (N(accessibility_class_init, NSV::CLASS_ACCESSIBILITY, 5));

    return s;
}

as_value
global_trace(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    // Log our argument.
    //
    // @@ what if we get extra args?
    //
    // @@ Nothing needs special treatment,
    //    as_value::to_string() will take care of everything
    log_trace("%s", fn.arg(0).to_string());

    return as_value();
}


as_value
global_isNaN(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    return as_value(static_cast<bool>(isNaN(
                    toNumber(fn.arg(0), getVM(fn)))));
}


as_value
global_isfinite(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    return as_value(static_cast<bool>(isFinite(
                    toNumber(fn.arg(0), getVM(fn)))));
}

/// \brief Encode a string to URL-encoded format
/// converting all dodgy DisplayObjects to %AB hex sequences
//
/// Characters that need escaping are:
/// - ASCII control DisplayObjects: 0-31 and 127
/// - Non-ASCII chars: 128-255
/// - URL syntax DisplayObjects: $ & + , / : ; = ? @
/// - Unsafe DisplayObjects: SPACE " < > # % { } | \ ^ ~ [ ] `
/// Encoding is a % followed by two hexadecimal DisplayObjects, case insensitive.
/// See RFC1738 http://www.rfc-editor.org/rfc/rfc1738.txt,
/// Section 2.2 "URL Character Encoding Issues"
as_value
global_escape(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    std::string input = fn.arg(0).to_string();
    URL::encode(input);
    return as_value(input);
}

/// \brief Decode a string from URL-encoded format
/// converting all hexadecimal sequences to ASCII DisplayObjects.
//
/// A sequence to convert is % followed by two case-independent hexadecimal
/// digits, which is replaced by the equivalent ASCII DisplayObject.
/// See RFC1738 http://www.rfc-editor.org/rfc/rfc1738.txt,
/// Section 2.2 "URL Character Encoding Issues"
as_value
global_unescape(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1

    std::string input = fn.arg(0).to_string();
    URL::decode(input);
    return as_value(input);
}

// parseFloat (string)
as_value
global_parsefloat(const fn_call& fn)
{
    ASSERT_FN_ARGS_IS_1
    
    std::istringstream s(fn.arg(0).to_string());
    double result;
    
    if (!(s >> result)) {
        return as_value(NaN);   
    }    

    return as_value(result);
}

// parseInt(string[, base])
// The second argument, if supplied, is the base.
// If none is supplied, we have to work out the 
// base from the string. Decimal, octal and hexadecimal are
// possible, according to the following rules:
// 1. If the string starts with 0x or 0X, the number is hex.
// 2. The 0x or 0X may be *followed* by '-' or '+' to indicate sign. A number
//    with no sign is positive.
// 3. If the string starts with 0, -0 or +0 and contains only the DisplayObjects
//    0-7.
// 4. If the string starts with *any* other sequence of DisplayObjects, including
//    whitespace, it is decimal.
as_value
global_parseint(const fn_call& fn)
{
    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("%s needs at least one argument"), __FUNCTION__);
        )
        return as_value();
    }

    IF_VERBOSE_ASCODING_ERRORS(
        if (fn.nargs > 2) {
            log_aserror(_("%s has more than two arguments"), __FUNCTION__);
        }
    )

    const std::string& expr = fn.arg(0).to_string();

    // A second argument specifies the base.
    // Parsing still starts after any positive/negative 
    // sign or hex identifier (parseInt("0x123", 8) gives
    // 83, not 0; parseInt(" 0x123", 8) is 0), which is
    // why we do this here.
    size_t base;
    if (fn.nargs > 1) {
        base = toInt(fn.arg(1), getVM(fn));
    
        // Bases from 2 to 36 are valid, otherwise return NaN
        if (base < 2 || base > 36) return as_value(NaN);
    }
    else {
        /// No radix specified, so try parsing as octal or hexadecimal
        try {
            double d;
            if (parseNonDecimalInt(expr, d, false)) return d;
        }
        catch (const boost::bad_lexical_cast&) {
            return as_value(NaN);
        }

        /// The number is not hex or octal, so we'll assume it's base-10.
        base = 10;

    }

    std::string::const_iterator it = expr.begin();

    // Check for expectional case "-0x" or "+0x", which
    // return NaN
    if ((expr.length() > 2) && (*it == '-' || *it == '+') &&
            *(it + 1) == '0' && std::toupper(*(it + 2)) == 'X') {
        return as_value(NaN);
    }
    
    // Try hexadecimal first
    if (expr.substr(0, 2) == "0x" || expr.substr(0, 2) == "0X") it += 2;
    else {
        // Skip leading whitespace
        while(*it == ' ' || *it == '\n' || *it == '\t' || *it == '\r') {
            ++it;
        }
        if (it == expr.end()) return as_value(NaN);
    }    

    bool negative = false;
    if (*it == '-' || *it == '+') {
        if (*it == '-') negative = true;
        
        it++;
        if (it == expr.end()) return as_value(NaN);
    }
    
    // Now we have the base, parse the digits. The iterator should
    // be pointing at the first digit.
    
    const std::string digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Check to see if the first digit is valid, otherwise 
    // return NaN.
    std::string::size_type digit = digits.find(toupper(*it));

    if (digit >= base || digit == std::string::npos) return as_value(NaN);

    // The first digit was valid, so continue from the present position
    // until we reach the end of the string or an invalid DisplayObject,
    // adding valid DisplayObjects to our result.
    // Which DisplayObjects are invalid depends on the base. 
    double result = digit;
    ++it;
    
    while (it != expr.end() && (digit = digits.find(toupper(*it))) < base
            && digit != std::string::npos) {
        result = result * base + digit;
        ++it;
    }

    // Now return the parsed string as an integer.
    return negative ? as_value(-result) : as_value(result);
}

// ASSetPropFlags function
as_value
global_assetpropflags(const fn_call& fn)
{
    if (fn.nargs < 3) {
        IF_VERBOSE_ASCODING_ERRORS(    
            log_aserror(_("%s needs at least three arguments"), __FUNCTION__);
        )
        return as_value();
    }
    
    IF_VERBOSE_ASCODING_ERRORS(
        if (fn.nargs > 4) {
            log_aserror(_("%s has more than four arguments"), "AsSetPropFlags");
        }
    );
    
    // object
    as_object* obj = toObject(fn.arg(0), getVM(fn));
    if (!obj) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("Invalid call to ASSetPropFlags: "
            "first argument is not an object: %s"),
            fn.arg(0));
        );
        return as_value();
    }

    // list of child names

    const as_value& props = fn.arg(1);

    const int flagsMask = PropFlags::dontEnum |
                          PropFlags::dontDelete |
                          PropFlags::readOnly |
                          PropFlags::onlySWF6Up |
                          PropFlags::ignoreSWF6 |
                          PropFlags::onlySWF7Up |
                          PropFlags::onlySWF8Up |
                          PropFlags::onlySWF9Up;

    // a number which represents three bitwise flags which
    // are used to determine whether the list of child names should be hidden,
    // un-hidden, protected from over-write, un-protected from over-write,
    // protected from deletion and un-protected from deletion
    const int setTrue = int(toNumber(fn.arg(2), getVM(fn))) & flagsMask;

    // Is another integer bitmask that works like set_true,
    // except it sets the attributes to false. The
    // set_false bitmask is applied before set_true is applied

    // ASSetPropFlags was exposed in Flash 5, however the fourth argument
    // 'set_false' was not required as it always defaulted to the value '~0'. 
    const int setFalse = (fn.nargs < 4 ? 0 : toInt(fn.arg(3), getVM(fn))) &
        flagsMask;

    obj->setPropFlags(props, setFalse, setTrue);

    return as_value();
}

// ASconstructor function
// See: http://osflash.org/flashcoders/undocumented/asnative?s=asnative
as_value
global_asconstructor(const fn_call& fn)
{
    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(    
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("ASNative(%s): needs at least two arguments"),
                ss.str());
        )
        return as_value();
    }

    const int sx = toInt(fn.arg(0), getVM(fn));
    const int sy = toInt(fn.arg(1), getVM(fn));

    if (sx < 0 || sy < 0) {
        IF_VERBOSE_ASCODING_ERRORS(    
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("ASconstructor(%s): args must be 0 or above"),
                ss.str());
        )
        return as_value();
    }

    const unsigned int x = static_cast<unsigned int>(sx);
    const unsigned int y = static_cast<unsigned int>(sy);

    VM& vm = getVM(fn);
    as_function* fun = vm.getNative(x, y);
    if (!fun) {
        log_debug(_("No ASnative(%d, %d) registered with the VM"), x, y);
        return as_value();
    }

    Global_as& gl = getGlobal(fn);
    fun->init_member(NSV::PROP_PROTOTYPE, createObject(gl));

    return as_value(fun);
        
}

// ASNative function
// See: http://osflash.org/flashcoders/undocumented/asnative?s=asnative
as_value
global_asnative(const fn_call& fn)
{
    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(    
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("ASNative(%s): needs at least two arguments"),
                ss.str());
        )
        return as_value();
    }

    const int sx = toInt(fn.arg(0), getVM(fn));
    const int sy = toInt(fn.arg(1), getVM(fn));

    if (sx < 0 || sy < 0) {
        IF_VERBOSE_ASCODING_ERRORS(    
            std::ostringstream ss; fn.dump_args(ss);
            log_aserror(_("ASnative(%s): args must be 0 or above"),
                ss.str());
        )
        return as_value();
    }

    const unsigned int x = static_cast<unsigned int>(sx);
    const unsigned int y = static_cast<unsigned int>(sy);

    VM& vm = getVM(fn);
    as_function* fun = vm.getNative(x, y);
    if (!fun) {
        log_debug(_("No ASnative(%d, %d) registered with the VM"), x, y);
        return as_value();
    }
    return as_value(fun);
}

// Obsolete ASnew function (exists only as ASnative(2, 0))
as_value
global_asnew(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("ASNative (2, 0) - old ASnew"));
    return as_value();
}

/// ASSetNative(targetObject, major, properties, minor)
//
/// Sets a series of properties on targetObject using the native table.
/// The third argument is generally documented to be an array, but in fact
/// it is always converted to a string and parsed.
as_value
global_assetnative(const fn_call& fn)
{
    if (fn.nargs < 3) {
        return as_value();
    }

    as_object* targetObject = toObject(fn.arg(0), getVM(fn));
    if (!targetObject) {
        return as_value();
    }

    const int major = toInt(fn.arg(1), getVM(fn));
    if (major < 0) return as_value();

    const std::string& props = fn.arg(2).to_string();
    const int minor =
        fn.nargs > 3 ? std::max<boost::int32_t>(toInt(fn.arg(3), getVM(fn)), 0) : 0;

    std::string::const_iterator pos = props.begin();

    VM& vm = getVM(fn);

    size_t i = 0;

    // pos is always the position after the last located property.
    while (pos != props.end()) {

        // If there are no further commas, find the end of the string.
        std::string::const_iterator comma = std::find(pos, props.end(), ',');

        const char num = *pos;
        
        int flag;

        switch (num) {
            case '6':
                flag = PropFlags::onlySWF6Up;
                ++pos;
                break;
            case '7':
                flag = PropFlags::onlySWF7Up;
                ++pos;
                break;
            case '8':
                flag = PropFlags::onlySWF8Up;
                ++pos;
                break;
            case '9':
                flag = PropFlags::onlySWF9Up;
                ++pos;
                break;
            default:
                flag = 0;

        }
        const std::string& property = std::string(pos, comma);
        if (!property.empty()) {
            targetObject->init_member(property,
                    vm.getNative(major, minor + i), flag);
        }
        if (comma == props.end()) break;
        pos = comma + 1;
        ++i;
    }
    return as_value();
}

// This is like ASSetNative, but attaches getter/setters.
as_value
global_assetnativeaccessor(const fn_call& fn)
{
    if (fn.nargs < 3) {
        return as_value();
    }

    as_object* targetObject = toObject(fn.arg(0), getVM(fn));
    if (!targetObject) {
        return as_value();
    }

    const int major = toInt(fn.arg(1), getVM(fn));
    if (major < 0) return as_value();

    const std::string& props = fn.arg(2).to_string();
    const int minor =
        fn.nargs > 3 ? std::max<boost::int32_t>(toInt(fn.arg(3), getVM(fn)), 0) : 0;

    std::string::const_iterator pos = props.begin();

    VM& vm = getVM(fn);

    size_t i = 0;

    // pos is always the position after the last located property.
    while (pos != props.end()) {

        // If there are no further commas, find the end of the string.
        std::string::const_iterator comma = std::find(pos, props.end(), ',');

        const char num = *pos;
        
        int flag;

        switch (num) {
            case '6':
                flag = PropFlags::onlySWF6Up;
                ++pos;
                break;
            case '7':
                flag = PropFlags::onlySWF7Up;
                ++pos;
                break;
            case '8':
                flag = PropFlags::onlySWF8Up;
                ++pos;
                break;
            case '9':
                flag = PropFlags::onlySWF9Up;
                ++pos;
                break;
            default:
                flag = 0;

        }
        const std::string& property = std::string(pos, comma);
        if (!property.empty()) {
            NativeFunction* getset = vm.getNative(major, minor + i);
            targetObject->init_property(property, *getset, *getset, flag);
        }
        if (comma == props.end()) break;
        pos = comma + 1;
        ++i;
    }
    return as_value();
}

// updateAfterEvent function
as_value
global_updateAfterEvent(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("updateAfterEvent()"));
    return as_value();
}

as_value
local_errorConstructor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    const as_value& arg = fn.nargs ? fn.arg(0) : as_value();
    VM& vm = getVM(fn);
    obj->set_member(getURI(vm, "message"), arg);
    return as_value();
}


/// Sets a range of Error subclasses.
as_value
global_assetuperror(const fn_call& fn)
{
    if (!fn.nargs) return as_value();

    // This should actually call String.split, but since our Array is
    // wrong we may as well do it like this for now.
    const std::string& errors = fn.arg(0).to_string();

    std::string::const_iterator pos = errors.begin();

    Global_as& gl = getGlobal(fn);

    // pos is always the position after the last located error.
    for (;;) {

        // If there are no further commas, find the end of the string.
        std::string::const_iterator comma = std::find(pos, errors.end(), ',');

        const std::string& err = std::string(pos, comma);

        VM& vm = getVM(fn);

        as_function* ctor = getMember(gl, NSV::CLASS_ERROR).to_function();
        if (ctor) {
            fn_call::Args args;
            as_object* proto = constructInstance(*ctor, fn.env(), args);

            // Not really sure what the point of this is.
            gl.createClass(local_errorConstructor, proto);
            proto->set_member(getURI(vm, "name"), err);
            proto->set_member(getURI(vm, "message"), err);
        }
        
        if (comma == errors.end()) break;
        pos = comma + 1;
    }
    return as_value();
}

as_value
global_setInterval(const fn_call& fn)
{
	if (fn.nargs < 2) {
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- need at least 2 arguments",
				ss.str());
		);
		return as_value();
	}

	unsigned timer_arg = 1;

	as_object* obj = toObject(fn.arg(0), getVM(fn));
	if (!obj) {

		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- first argument is not an object or function",
				ss.str());
		);
		return as_value();
	}

    ObjectURI methodName;

	// Get interval function
	as_function* as_func = obj->to_function(); 
	if (!as_func) {
		methodName = getURI(getVM(fn), fn.arg(1).to_string());
		timer_arg = 2;
	}


	if (fn.nargs < timer_arg + 1) {
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- missing timeout argument",
				ss.str());
        );
		return as_value();
	}

	// Get interval time
	unsigned long ms =
        static_cast<unsigned long>(toNumber(fn.arg(timer_arg), getVM(fn)));
	// TODO: check validity of interval time number ?

	// Parse arguments 
    fn_call::Args args;
	for (unsigned i = timer_arg + 1; i < fn.nargs; ++i) {
		args += fn.arg(i);
	}

	std::auto_ptr<Timer> timer;
	if (as_func) {
		timer.reset(new Timer(*as_func, ms, fn.this_ptr, args));
	}
	else {
		timer.reset(new Timer(obj, methodName, ms, args));
	}
    
	movie_root& root = getRoot(fn);

    // TODO: check what should happen to overflows.
	const int id = root.addIntervalTimer(timer);
	return as_value(id);
}

as_value
global_setTimeout(const fn_call& fn)
{
	if (fn.nargs < 2) {
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setTimeout(%s) "
			"- need at least 2 arguments",
			ss.str());
		);
		return as_value();
	}

	unsigned timer_arg = 1;

	as_object* obj = toObject(fn.arg(0), getVM(fn));
	if (!obj) {
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- first argument is not an object or function",
				ss.str());
		);
		return as_value();
	}

    ObjectURI methodName;

	// Get interval function
	as_function* as_func = obj->to_function(); 
	if (!as_func) {
		methodName = getURI(getVM(fn), fn.arg(1).to_string());
		timer_arg = 2;
	}


	if (fn.nargs < timer_arg + 1) {
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setTimeout(%s): missing "
                "timeout argument", ss.str());
		);
		return as_value();
	}

	// Get interval time
	unsigned long ms =
        static_cast<unsigned long>(toNumber(fn.arg(timer_arg), getVM(fn)));

	// Parse arguments 
    fn_call::Args args;
	for (unsigned i = timer_arg + 1; i < fn.nargs; ++i) {
		args += fn.arg(i);
	}

	std::auto_ptr<Timer> timer;
	if (as_func) {
		timer.reset(new Timer(*as_func, ms, fn.this_ptr, args, true));
	}
	else {
		timer.reset(new Timer(obj, methodName, ms, args, true));
	}
    
	movie_root& root = getRoot(fn);

    // TODO: check what should happen to overflows.
	const int id = root.addIntervalTimer(timer);
	return as_value(id);
}
  
as_value
global_clearInterval(const fn_call& fn)
{
    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
                log_aserror("clearInterval requires one argument, got none");
        );
        return as_value();
    }

    const boost::uint32_t id = toInt(fn.arg(0), getVM(fn));

	movie_root& root = getRoot(fn);
	return as_value(root.clearIntervalTimer(id));
}

as_value
global_showRedrawRegions(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("_global.showRedrawRegions"));
    return as_value();
}

as_value
global_enableDebugConsole(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("_global.enableDebugConsole"));
    return as_value();
}

void
registerNatives(as_object& global)
{
    VM& vm = getVM(global);

    // ASNew was dropped as an API function but exists
    // as ASnative.
    vm.registerNative(global_assetpropflags, 1, 0);
    vm.registerNative(global_asnew, 2, 0);    
    vm.registerNative(global_assetnative, 4, 0);    
    vm.registerNative(global_assetnativeaccessor, 4, 1);
    vm.registerNative(global_updateAfterEvent, 9, 0);    
    vm.registerNative(global_escape, 100, 0);
    vm.registerNative(global_unescape, 100, 1);
    vm.registerNative(global_parseint, 100, 2);
    vm.registerNative(global_parsefloat, 100, 3);
    vm.registerNative(global_trace, 100, 4);
    vm.registerNative(global_isNaN, 200, 18);
    vm.registerNative(global_isfinite, 200, 19);
    vm.registerNative(global_setInterval, 250, 0);
    vm.registerNative(global_clearInterval, 250, 1);
    vm.registerNative(global_setTimeout, 250, 2);
    
    vm.registerNative(global_showRedrawRegions, 1021, 1);

    registerObjectNative(global);
    registerFunctionNative(global);
    registerStringNative(global);
    registerArrayNative(global);
    registerNumberNative(global);
    registerBooleanNative(global);
    registerMovieClipNative(global);
    registerSelectionNative(global);
    registerColorNative(global);
    registerMathNative(global);
    registerSystemNative(global);
    registerAccessibilityNative(global);
    registerStageNative(global);
    registerTextFieldNative(global);
    registerButtonNative(global);
    registerVideoNative(global);
    registerMovieClipLoaderNative(global);
    registerXMLSocketNative(global);
    registerSharedObjectNative(global);
    registerKeyNative(global);
    registerNetStreamNative(global);
    registerCameraNative(global);
    registerMicrophoneNative(global);
    registerTextSnapshotNative(global);
    registerSoundNative(global);
    registerLocalConnectionNative(global);
    registerBitmapFilterNative(global);
    registerColorTransformNative(global);
    registerExternalInterfaceNative(global);
    registerBitmapDataNative(global);

    AsBroadcaster::registerNative(global);
    registerTextFormatNative(global);
    registerDateNative(global);
    Mouse_as::registerNative(global);

    // LoadableObject has natives shared between LoadVars and XML, so 
    // should be registered first.
    registerLoadableNative(global);
    registerXMLNative(global);
    registerXMLNodeNative(global);
}

} // anonymous namespace
} // namespace gnash

