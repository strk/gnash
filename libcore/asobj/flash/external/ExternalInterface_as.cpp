// ExternalInterface_as.cpp:  ActionScript "ExternalInterface" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "ExternalInterface_as.h"

#include <map>
#include <vector>
#include <sstream>
#include <boost/algorithm/string/erase.hpp>
#include <algorithm>
#include "GnashSystemNetHeaders.h"

#include "ExternalInterface.h"
#include "NativeFunction.h"
#include "StringPredicates.h"
#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "Global_as.h"
#include "GnashException.h" // for ActionException
#include "VM.h"
#include "rc.h"
#include "as_value.h"
#include "as_object.h"
#include "XML_as.h"
#include "Array_as.h"
#include "namedStrings.h"
#include "Global_as.h"
#include "PropertyList.h"
#include "movie_root.h"
#include "log.h"
#include "RunResources.h"
#include "StreamProvider.h"
#include "ObjectURI.h"

#define MAXHOSTNAMELEN 256 // max hostname size. However this is defined in netdb.h

namespace gnash {

namespace {
    as_value externalInterfaceConstructor(const fn_call& fn);
    as_value externalinterface_addCallback(const fn_call& fn);
    as_value externalinterface_call(const fn_call& fn);
    as_value externalinterface_available(const fn_call& fn);
    as_value externalinterface_objectID(const fn_call& fn);
    as_value externalinterface_uArgumentsToXML(const fn_call& fn);
    as_value externalinterface_uArgumentsToAS(const fn_call& fn);
    as_value externalinterface_uAddCallback(const fn_call& fn);
    as_value externalinterface_uArrayToAS(const fn_call& fn);
    as_value externalinterface_uArrayToJS(const fn_call& fn);
    as_value externalinterface_uArrayToXML(const fn_call& fn);
    as_value externalinterface_uCallIn(const fn_call& fn);
    as_value externalinterface_uCallOut(const fn_call& fn);
    as_value externalinterface_uEscapeXML(const fn_call& fn);
    as_value externalinterface_uEvalJS(const fn_call& fn);
    as_value externalinterface_uInitJS(const fn_call& fn);
    as_value externalinterface_uJsQuoteString(const fn_call& fn);
    as_value externalinterface_uObjectID(const fn_call& fn);
    as_value externalinterface_uObjectToAS(const fn_call& fn);
    as_value externalinterface_uObjectToJS(const fn_call& fn);
    as_value externalinterface_uObjectToXML(const fn_call& fn);
    as_value externalinterface_uToAS(const fn_call& fn);
    as_value externalinterface_uToJS(const fn_call& fn);
    as_value externalinterface_uToXML(const fn_call& fn);
    as_value externalinterface_uUnescapeXML(const fn_call& fn);
}

namespace {

class Enumerator : public KeyVisitor
{
public:
    Enumerator(std::vector<ObjectURI>& uris) : _uris(uris) {}
    void operator()(const ObjectURI& u) {
        _uris.push_back(u);
    }
private:
    std::vector<ObjectURI>& _uris;
};

class ArrayToXML
{
public:
    ArrayToXML(as_value& ret, const fn_call& fn)
        :
        _ret(ret),
        _fn(fn),
        _count(0)
    {}

    void operator()(const as_value& val) {
        VM& vm = getVM(_fn);

        newAdd(_ret, "<property id=\"", vm);
        newAdd(_ret, static_cast<double>(_count), vm);
        newAdd(_ret, "\">", vm);
        as_object* ei = 
            findObject(_fn.env(), "flash.external.ExternalInterface");
        const as_value& x = callMethod(ei, getURI(vm, "_toXML"), val);
        newAdd(_ret, x, vm);
        newAdd(_ret, "</property>", vm);
        ++_count;
    }
private:
    as_value& _ret;
    const fn_call& _fn;
    size_t _count;
};

class ArgsToXML
{
public:
    ArgsToXML(as_value& ret, const fn_call& fn)
        :
        _ret(ret),
        _fn(fn)
    {}

    void operator()(const as_value& val) {
        VM& vm = getVM(_fn);
        as_object* ei = 
            findObject(_fn.env(), "flash.external.ExternalInterface");
        const as_value& x = callMethod(ei, getURI(vm, "_toXML"), val);
        newAdd(_ret, x, vm);
    }
private:
    as_value& _ret;
    const fn_call& _fn;
};

}

void
registerExternalInterfaceNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(externalinterface_uInitJS, 14, 0);
    vm.registerNative(externalinterface_uObjectID, 14, 1);
    vm.registerNative(externalinterface_uAddCallback, 14, 2);
    vm.registerNative(externalinterface_uEvalJS, 14, 3);
    vm.registerNative(externalinterface_uCallOut, 14, 4);
    vm.registerNative(externalinterface_uEscapeXML, 14, 5);
    vm.registerNative(externalinterface_uUnescapeXML, 14, 6);
    vm.registerNative(externalinterface_uJsQuoteString, 14, 7);

    vm.registerNative(externalinterface_available, 14, 100);

}

void
externalinterface_class_init(as_object& where, const ObjectURI& uri)
{
    where.init_destructive_property(uri, externalInterfaceConstructor, 0);
}

namespace {

void
attachExternalInterfaceStaticInterface(as_object& o)
{    
    
    const int swf8Flags = PropFlags::onlySWF8Up;
    
    VM& vm = getVM(o);
 
    // Native functions
    o.init_member("_initJS", vm.getNative(14, 0), swf8Flags);
    o.init_member("_objectID", vm.getNative(14, 1), swf8Flags);
    o.init_member("_addCallback", vm.getNative(14, 2), swf8Flags);
    o.init_member("_evalJS", vm.getNative(14, 3), swf8Flags);
    o.init_member("_callOut", vm.getNative(14, 4), swf8Flags);
    o.init_member("_escapeXML", vm.getNative(14, 5), swf8Flags);
    o.init_member("_unescapeXML", vm.getNative(14, 6), swf8Flags);
    o.init_member("_jsQuoteString", vm.getNative(14, 7), swf8Flags);

    // Native properties
    NativeFunction* n = vm.getNative(14, 100);
    o.init_property("available", *n, *n, swf8Flags);

    Global_as& gl = getGlobal(o);
    
    // ActionScript functions
    o.init_member("addCallback",
                  gl.createFunction(externalinterface_addCallback));
    
    o.init_member("call", gl.createFunction(externalinterface_call));
    
    // Undocumented ActionScript functions.
    o.init_member("_argumentsToXML",
              gl.createFunction(externalinterface_uArgumentsToXML));
    o.init_member("_argumentsToAS",
              gl.createFunction(externalinterface_uArgumentsToAS));
    o.init_member("_arrayToAS",
                  gl.createFunction(externalinterface_uArrayToAS));
    o.init_member("_arrayToJS",
                  gl.createFunction(externalinterface_uArrayToJS));
    o.init_member("_arrayToXML",
                  gl.createFunction(externalinterface_uArrayToXML));
    o.init_member("_callIn",
                  gl.createFunction(externalinterface_uCallIn));
    o.init_member("_objectToAS",
                  gl.createFunction(externalinterface_uObjectToAS));
    o.init_member("_objectToJS",
                  gl.createFunction(externalinterface_uObjectToJS));
    o.init_member("_objectToXML",
                  gl.createFunction(externalinterface_uObjectToXML));
    o.init_member("_toAS",
                  gl.createFunction(externalinterface_uToAS));
    o.init_member("_toJS",
                  gl.createFunction(externalinterface_uToJS));
    o.init_member("_toXML",
                  gl.createFunction(externalinterface_uToXML));
    
    // Apparently the pp calls:
    //
    // AsSetPropFlags(flash.external.ExternalInterface, null, 4103) 
    //
    // here, but it seems that the properties actually are visible in SWF6
    // and SWF7, at least for the flashplayer 9. So we just make sure they
    // are read-only.
    as_object* null = 0;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, &o, null, 7);
}

/// This adds a function that can be called from javascript.
//
/// TODO: addCallback takes three arguments; only two are handled here.
as_value
externalinterface_addCallback(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);

    if (mr.getControlFD() <= 0) {
        log_debug("ExternalInterface not accessible when running standalone.");
        return as_value(false);
    }

    if (fn.nargs > 1) {
        const as_value& name_as = fn.arg(0);
        std::string name = name_as.to_string();
        if (fn.arg(1).is_object()) {
            log_debug("adding callback %s", name);
            as_object* asCallback = toObject(fn.arg(1), getVM(fn));
            mr.addExternalCallback(name, asCallback);
        }
    }

    // Returns true unless unavailable (which we checked above)
    return as_value(true);    
}

// This calls a Javascript function in the browser.
as_value
externalinterface_call(const fn_call& fn)
{
    movie_root& mr = getRoot(fn);
    as_value val;

    if (mr.getControlFD() <= 0) {
        log_debug("ExternalInterface not accessible on call.");
        val.set_null();
        return val;
    }

    if (fn.nargs > 1) {
        const as_value& methodName_as = fn.arg(0);
        const std::string methodName = methodName_as.to_string();
        const std::vector<as_value>& args = fn.getArgs();
        log_debug("Calling External method \"%s\"", methodName);
        std::string result = mr.callExternalJavascript(methodName, args);
        if (!result.empty()) {
            val = ExternalInterface::parseXML(result);
            // There was an error trying to Invoke the callback
            if (result == ExternalInterface::makeString("Error")
                || (result == ExternalInterface::makeString("SecurityError"))) {
                log_trace(_("VAL: %s"), val);
                val.set_undefined();
            }
        } 
    }
    
    return val;
}

as_value
externalinterface_available(const fn_call& fn)
{
    
    movie_root& m = getRoot(fn);
    bool mode = false;

    // If we're not running under a browser as a plugin, then just
    // return, as ExternalInterface is only available as a plugin.
    if (m.getHostFD() < 0) {
        return false;
    }
    
    switch (m.getAllowScriptAccess()) {
      case movie_root::SCRIPT_ACCESS_NEVER:
          mode = false;
          break;
          
      case movie_root::SCRIPT_ACCESS_SAME_DOMAIN:
      {
          const RunResources& r = m.runResources();
          const std::string& baseurl = r.streamProvider().baseURL().str();
          char hostname[MAXHOSTNAMELEN] = {};
          
          if (::gethostname(hostname, MAXHOSTNAMELEN) != 0) {
              mode = false;
          }
          
          // The hostname is empty if running the standalone Gnash from
          // a terminal, so we can assume the default of sameDomain applies.
          URL localPath(hostname, baseurl);
          // If the URL has a file protocol, then 
          if (r.streamProvider().allow(localPath)) {
              return as_value(true);
          }
          if (localPath.hostname().empty()) {
              mode = false;
          } else {
              StringNoCaseEqual noCaseCompare;
              
              if (!noCaseCompare(localPath.hostname(), hostname)) {
                  log_security(_("ExternalInterface path %s is outside "
                                 "the SWF domain %s. Cannot access this "
                                 "object."), localPath, hostname);
                  mode = false;
              }
          }
          break;
      }
      
      case movie_root::SCRIPT_ACCESS_ALWAYS:
          mode = true;
          break;
    }
    
    return as_value(mode);
}

as_value
externalinterface_objectID(const fn_call& fn)
{

    movie_root& mr = getRoot(fn);
    MovieClip *mc = mr.getLevel(0);
    VM& vm = getVM(fn);
    
    as_value id;
    getObject(mc)->get_member(getURI(vm, "id"), &id);

    as_value name;
    getObject(mc)->get_member(getURI(vm, "name"), &name);

    if (id.is_undefined() && !name.is_undefined()) {
        return name;
    } else if (!id.is_undefined() && name.is_undefined()) {
        return id;
    } else if (id.is_undefined() && name.is_undefined()) {
        return as_value();
    }
    
    return as_value();
}

as_value
externalInterfaceConstructor(const fn_call& fn)
{
    log_debug("Loading flash.external.ExternalInterface class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(emptyFunction, proto);

    attachExternalInterfaceStaticInterface(*cl);
    return cl;
}

as_value
externalinterface_uArgumentsToXML(const fn_call& fn)
{
    as_value ret("<arguments>");

    if (fn.nargs) {
        VM& vm = getVM(fn);
        as_object *obj = toObject(fn.arg(0), vm);
        if (obj) {
            ArgsToXML tx(ret, fn);
            size_t size = arrayLength(*obj);
            if (size) {
                for (size_t i = 1; i < size; ++i) {
                    tx(getOwnProperty(*obj, arrayKey(vm, i)));
                }
            }
        }
    }
    
    newAdd(ret, "</arguments>", getVM(fn));
    return ret;
}

as_value
externalinterface_uArgumentsToAS(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uAddCallback(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uArrayToAS(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uArrayToJS(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uArrayToXML(const fn_call& fn)
{
    as_value ret("<array>");

    if (fn.nargs) {
        as_object *obj = toObject(fn.arg(0), getVM(fn));
        if (obj) {
            ArrayToXML tx(ret, fn);
            foreachArray(*obj, tx);
        }
    }
    
    newAdd(ret, "</array>", getVM(fn));
    return ret;
}

as_value
externalinterface_uCallIn(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uCallOut(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uEvalJS(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uInitJS(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uJsQuoteString(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uObjectID(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uObjectToAS(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uObjectToJS(const fn_call& /*fn*/)
{
	LOG_ONCE(log_unimpl(__FUNCTION__) );
	return as_value();
}

as_value
externalinterface_uObjectToXML(const fn_call& fn)
{
    VM& vm = getVM(fn);

    as_value ret("<object>");

    if (fn.nargs) {
        as_object* obj = toObject(fn.arg(0), getVM(fn));

        if (obj) {

            string_table& st = getStringTable(fn);

            typedef std::vector<ObjectURI> URIs;
            URIs uris;

            // Fake AS enumeration.
            Enumerator en(uris);
            obj->visitKeys(en);

            for (URIs::const_reverse_iterator i = uris.rbegin(), e = uris.rend();
                    i != e; ++i) {
                const std::string& id = i->toString(st);

                newAdd(ret, "<property id=\"", vm);
                newAdd(ret, id, vm);
                newAdd(ret, "\">", vm);

                as_object* ei = 
                    findObject(fn.env(), "flash.external.ExternalInterface");
                const as_value& val = getMember(*obj, *i); 
                newAdd(ret, callMethod(ei, getURI(vm, "_toXML"), val), vm);
                newAdd(ret, "</property>", vm);
            }
        }
    }

    newAdd(ret, "</object>", vm);
    return ret;

}

as_value
externalinterface_uToJS(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
externalinterface_uToXML(const fn_call& fn)
{

    // Probably implemented with switch(typeof value)
    if (fn.nargs) {

        as_object* ei = 
            findObject(fn.env(), "flash.external.ExternalInterface");
        VM& vm = getVM(fn);

        const as_value& val = fn.arg(0);
        if (val.is_string()) {
            as_value ret = "<string>";
            newAdd(ret, callMethod(ei, getURI(vm, "_escapeXML"), val), vm);
            newAdd(ret, "</string>", vm);
            return ret;
        }
        if (val.is_undefined()) {
            return as_value("<undefined/>");
        }
        if (val.is_number()) {
            as_value ret = "<number>";
            newAdd(ret, val, vm);
            newAdd(ret, "</number>", vm);
            return ret;
        }
        if (val.is_null()) {
            return as_value("<null/>");
        }
        if (val.is_bool()) {
            return toBool(val, vm) ? as_value("<true/>") : as_value("<false/>");
        }
        if (val.is_object()) {
            as_object* obj = toObject(val, vm);
            assert(obj);
            if (hasOwnProperty(*obj, NSV::PROP_LENGTH)) {
                return callMethod(ei, getURI(vm, "_arrayToXML"), val);
            }
            return callMethod(ei, getURI(vm, "_objectToXML"), val);
        }
    }
    return as_value("<null/>");
}

as_value
externalinterface_uToAS(const fn_call& fn)
{
    if (!fn.nargs) return as_value();

    as_value arg = fn.arg(0);
    as_object* o = toObject(arg, getVM(fn));

    if (!o) {
        return as_value();
    }

    VM& vm = getVM(fn);
    // TODO: use NSV ?
    const ObjectURI& nodeName = getURI(vm, "nodeName");
    const ObjectURI& firstChild = getURI(vm, "firstChild");

    const as_value& nn = getMember(*o, nodeName);

    if (equals(nn, as_value("number"), vm)) {
        as_object* fc = toObject(getMember(*o, firstChild), vm);
        const as_value v = callMethod(fc, NSV::PROP_TO_STRING);
        // This should call Number(obj.firstChild.toString()), i.e. use
        // the non-constructing number conversion function, but the extra
        // code needed to implement that isn't worth it.
        return as_value(toNumber(v, vm));
    }
    if (equals(nn, as_value("string"), getVM(fn))) {
        as_object* ei =
            findObject(fn.env(), "flash.external.ExternalInterface");
        as_value fc = getMember(*o, firstChild);
        return callMethod(ei, getURI(vm, "_unescapeXML"),
                fc.to_string(getSWFVersion(fn)));
    }
    if (equals(nn, as_value("false"), getVM(fn))) {
        return as_value(false);
    }
    if (equals(nn, as_value("true"), getVM(fn))) {
        return as_value(true);
    }
    if (equals(nn, as_value("null"), getVM(fn))) {
        as_value null;
        null.set_null();
        return null;
    }
    if (equals(nn, as_value("undefined"), getVM(fn))) {
        return as_value();
    }
    if (equals(nn, as_value("object"), getVM(fn))) {
        as_object* ei =
            findObject(fn.env(), "flash.external.ExternalInterface");
        return callMethod(ei, getURI(vm, "_objectToXML"), o);
    }
    if (equals(nn, as_value("array"), getVM(fn))) {
        as_object* ei =
            findObject(fn.env(), "flash.external.ExternalInterface");
        return callMethod(ei, getURI(vm, "_arrayToXML"), o);
    }
    if (equals(nn, as_value("class"), getVM(fn))) {
        as_value fc = getMember(*o, firstChild);
        return findObject(fn.env(), fc.to_string(getSWFVersion(fn)));
    }
    return as_value();
}

as_value
externalinterface_uEscapeXML(const fn_call& fn)
{
    if (fn.nargs == 1) {
        std::string str(fn.arg(0).to_string());
        escapeXML(str);
        return as_value(str);
    }
    
    return as_value();
}

as_value
externalinterface_uUnescapeXML(const fn_call& fn)
{
    if (fn.nargs == 1) {
        std::string str = fn.arg(0).to_string();
        gnash::unescapeXML(str);
        return as_value(str);
    }
    
    return as_value();
}

} // end of anonymous namespace used for callbacks

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
