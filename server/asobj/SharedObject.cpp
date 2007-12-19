// SharedObject.cpp:  ActionScript "SharedObject" class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "amf.h"
#include "sol.h"
#include "SharedObject.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "VM.h"
#include "PropertyList.h"
#include "Property.h"
#include "string_table.h"

using namespace amf;

namespace gnash {

as_value sharedobject_clear(const fn_call& fn);
as_value sharedobject_flush(const fn_call& fn);
as_value sharedobject_getlocal(const fn_call& fn);
as_value sharedobject_getsize(const fn_call& fn);
as_value sharedobject_ctor(const fn_call& fn);

void sharedobject_iter(SOL &sol, string_table::key key, const as_value &reference);

class PropsSerializer {
    SOL& _sol; public: PropsSerializer(SOL& sol) : _sol(sol) {};
    virtual as_value operator() (string_table::key key, const as_value& val)
        {
            GNASH_REPORT_FUNCTION;
            AMF amf;
            AMF::amf_element_t el;
            string_table& st = VM::get().getStringTable();
            string str = st.string_table::value(key);
            cerr << "FIXME: yes!!!!! " << str << ": "<< val.to_string() << endl;

            if (val.is_string()) {
                string str;
                if (val.is_undefined()) {
                    str = "";
                } else {
                    str = val.to_string();
                }
                amf.createElement(&el, str, str);
            }
            if (val.is_bool()) {
                bool b;
                amf.createElement(&el, str, b);
            }
            if (val.is_number()) { 
                double dub;
                if (val.is_undefined()) {
                    dub = 0.0;
                } else {
                    dub = val.to_number();
                }
                amf.createElement(&el, str, dub);
            }
            
            _sol.addObj(el);
            return as_value(true);
        }
};

static void
attachProperties(as_object& o)
{
    GNASH_REPORT_FUNCTION;
     as_object *proto = new as_object();
     o.init_member("data", proto);
}

static void
attachSharedObjectInterface(as_object& o)
{
    GNASH_REPORT_FUNCTION;
    // TODO: clear, flush and getSize not in SWF<6 , it seems
    o.init_member("clear", new builtin_function(sharedobject_clear));
    o.init_member("flush", new builtin_function(sharedobject_flush));
    //o.init_member("getLocal", new builtin_function(sharedobject_getlocal));
    o.init_member("getSize", new builtin_function(sharedobject_getsize));
    attachProperties(o);
}

static void
attachSharedObjectStaticInterface(as_object& o)
{
    GNASH_REPORT_FUNCTION;
    o.init_member("getLocal", new builtin_function(sharedobject_getlocal));
    attachProperties(o);
}

static as_object*
getSharedObjectInterface()
{
    GNASH_REPORT_FUNCTION;
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object(getObjectInterface());
        attachSharedObjectInterface(*o);
    }
    return o.get();
}


class SharedObject: public as_object, public amf::SOL
{
public:
    SharedObject()
        :
        as_object(getSharedObjectInterface())
        { 
        }
};

#if 0
// Turn each property into an AMF element
void
sharedobject_iter(SOL &sol, string_table::key key, const as_value &reference)
{
    GNASH_REPORT_FUNCTION;

    AMF amf;
    AMF::amf_element_t el;
    string_table& st = VM::get().getStringTable();
    string str = st.string_table::value(key);
//    cerr << "FIXME: yes!!!!! " << str << ": "<< reference.to_string() << endl;

    if (reference.is_string()) {
        string str = reference.to_string();
        amf.createElement(&el, str, str);
    }
    if (reference.is_bool()) {
        bool b;
        amf.createElement(&el, str, b);
    }
    if (reference.is_number()) {
        double dub = reference.to_number();
        amf.createElement(&el, str, dub);
    }

    sol.addObj(el);
}
#endif

as_value
sharedobject_clear(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<SharedObject> obj = ensureType<SharedObject>(fn.this_ptr);
    UNUSED(obj);
    
    static bool warned=false;
    if ( ! warned ) {
        log_unimpl (__FUNCTION__);
        warned=true;
    }
    return as_value();
}

as_value
sharedobject_flush(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    
    boost::intrusive_ptr<SharedObject> obj = ensureType<SharedObject>(fn.this_ptr);

//    log_msg("Flushing to file %s", obj->getFilespec().c_str());

     string_table::key dataKey = VM::get().getStringTable().find("data");
     as_value as = obj->getMember(dataKey);
     boost::intrusive_ptr<as_object> ptr = as.to_object();
     
      auto_ptr<SOL> sol(new SOL);
      PropsSerializer props(*sol);     
      ptr->visitPropertyValues(props);
      sol->writeFile(obj->getFilespec(), obj->getObjectName().c_str());
     return as_value(true);
}

as_value
sharedobject_getlocal(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    // This should return a SharedObject, and it's a static function
    
//    static boost::intrusive_ptr<as_object> obj = new as_object(getSharedObjectInterface());
    static boost::intrusive_ptr<SharedObject> obj = new SharedObject();

    if (fn.nargs > 0) {
        std::string filespec = fn.arg(0).to_string();
        obj->setFilespec(filespec);
        obj->setObjectName(filespec);
        log_msg("Opening SharedObject file: %s", filespec.c_str());
    }
    
    return as_value(obj.get()); // will keep alive
}

as_value
sharedobject_getsize(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<SharedObject> obj = ensureType<SharedObject>(fn.this_ptr);
    return as_value(obj->size());
}

as_value
sharedobject_ctor(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
//    boost::intrusive_ptr<as_object> obj = new SharedObject;
    static boost::intrusive_ptr<as_object> obj = new as_object(getSharedObjectInterface());
    
    return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void sharedobject_class_init(as_object& global)
{
    GNASH_REPORT_FUNCTION;
    // This is going to be the global SharedObject "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;
    
    if (cl == NULL) {
        cl=new builtin_function(&sharedobject_ctor, getSharedObjectInterface());
        // replicate all interface to class, to be able to access
        // all methods as static functions
        attachSharedObjectStaticInterface(*cl);
    }
    
    // Register _global.SharedObject
    global.init_member("SharedObject", cl.get());    
}

} // end of gnash namespace
