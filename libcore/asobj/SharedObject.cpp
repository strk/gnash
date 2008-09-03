// SharedObject.cpp:  ActionScript "SharedObject" class, for Gnash.
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
#include "gnashconfig.h" // USE_SOL_READ_ONLY
#endif

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/tokenizer.hpp>
#include <cerrno>

#include "amf.h"
#include "element.h"
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
#include "URLAccessManager.h"
#include "URL.h"
#include "rc.h" // for use of rcfile

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

using namespace amf;

namespace gnash {

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

as_value sharedobject_clear(const fn_call& fn);
as_value sharedobject_flush(const fn_call& fn);
as_value sharedobject_getlocal(const fn_call& fn);
as_value sharedobject_getsize(const fn_call& fn);
as_value sharedobject_ctor(const fn_call& fn);

void sharedobject_iter(SOL &sol, string_table::key key, const as_value &reference);

class PropsSerializer {
    SOL& _sol;
    string_table& _st;
public:
    PropsSerializer(SOL& sol, VM& vm)
        :
        _sol(sol),
        _st(vm.getStringTable())
    {};

    void operator() (string_table::key key, const as_value& val) const
        {
            //GNASH_REPORT_FUNCTION;
            AMF amf;
            Element *el = 0;

            const std::string& name = _st.string_table::value(key);

//          cerr << "FIXME: yes!!!!! " << name << ": "<< val << std::endl;

            if (val.is_string()) {
                std::string str;
                if (!val.is_undefined()) {
                    str = val.to_string();
                }
                el = new amf::Element;
                el->init(name, str);
            }
            if (val.is_bool()) {
                bool flag = val.to_bool();
                el = new amf::Element;
                el->init(name, flag);
            }
            if (val.is_number()) { 
                double dub;
                if (val.is_undefined()) {
                    dub = 0.0;
                } else {
                    dub = val.to_number();
                }
                el = new amf::Element;
                el->init(name, dub);
            }

            if (el) {
                _sol.addObj(el);
            }
        }
};

static void
attachProperties(as_object& o)
{
//    GNASH_REPORT_FUNCTION;
     as_object *proto = new as_object();
     o.init_member("data", proto);
}

static void
attachSharedObjectInterface(as_object& o)
{
//    GNASH_REPORT_FUNCTION;

    VM& vm = o.getVM();
    const int swfVersion = vm.getSWFVersion();

    // clear, flush and getSize not in SWF<6 , it seems
    if ( swfVersion < 6 ) return; 

    o.init_member("clear", new builtin_function(sharedobject_clear));
    o.init_member("flush", new builtin_function(sharedobject_flush));
    o.init_member("getSize", new builtin_function(sharedobject_getsize));
}

static void
attachSharedObjectStaticInterface(as_object& o)
{
//    GNASH_REPORT_FUNCTION;

    o.init_member("getLocal", new builtin_function(sharedobject_getlocal));
}

static as_object*
getSharedObjectInterface()
{
//    GNASH_REPORT_FUNCTION;

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
		attachProperties(*this);
    }
};


as_value
sharedobject_clear(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<SharedObject> obj = ensureType<SharedObject>(fn.this_ptr);
    UNUSED(obj);
    
    LOG_ONCE(log_unimpl (__FUNCTION__));

    return as_value();
}

as_value
sharedobject_flush(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<SharedObject> obj = ensureType<SharedObject>(fn.this_ptr);

//    log_debug("Flushing to file %s", obj->getFilespec());        
    VM& vm = obj->getVM();

#ifndef USE_SOL_READONLY
    if (rcfile.getSOLReadOnly() ) {
        log_security("Attempting to write object %s when it's SOL Read Only is set! Refusing...",
                     obj->getFilespec());
        return as_value(false);
    }
    
    // TODO: cache the dataKey in SharedObject prototype on first use ?
    //       a SharedObject::getDataKey() might do...
    string_table::key dataKey = vm.getStringTable().find("data");
    
    as_value as = obj->getMember(dataKey);
    boost::intrusive_ptr<as_object> ptr = as.to_object();
    if ( ! ptr ) {
        log_error("'data' member of SharedObject is not an object (%s)",
                  as);
        return as_value();
    }
    
    SOL sol;
    PropsSerializer props(sol, vm);
    ptr->visitPropertyValues(props);
    // We only want to access files in this directory
    std::string newspec; 
    newspec += obj->getFilespec();
    bool ret = sol.writeFile(newspec, obj->getObjectName().c_str());
    if ( ! ret )
    {
        log_error("writing SharedObject file to %s", newspec);
        return as_value(false);
    }
    log_security("SharedObject '%s' written to filesystem.", newspec);
    return as_value(true); // TODO: check expected return type from SharedObject.flush
#else
    return as_value(false);
#endif
}

// Set the file name
as_value
sharedobject_getlocal(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    // This should return a SharedObject, and it's a static function
    
    // FIXME:
    //    We shouldn't have a single SharedObject static, but one for
    //    each key the user code asks for. These might be maintained
    //    in a static "library" member of the SharedObject class
    //    exposing methods to get one element by name, which would be
    //    either created or reused, taking care about registering
    //    the static with the VM. There will likely be a need to
    //    be signaled restarts as unflushed objects should not be
    //    accessible with their state on restart..
    //
    static boost::intrusive_ptr<SharedObject> obj;
    if ( ! obj )
    { 
        obj = new SharedObject();
        obj->getVM().addStatic(obj.get());
    }
    
    std::string::size_type pos;
    std::string rootdir;
    if (fn.nargs > 0) {
        std::string filespec = fn.arg(0).to_string();
        // If there is a second argument to getLocal(), it replaces
        // the default path, which is the swf file name, with this
        // supplied path.
        // the object name appears to be the same as the file name, but
        // minus the suffix. 
        if ((pos = filespec.find(".sol", 0) == std::string::npos)) {
            obj->setObjectName(filespec);
            filespec += ".sol";
        } else {
            std::string objname = filespec.substr(0, filespec.size() - 4);
            obj->setObjectName(objname);
        }
        obj->setFilespec(filespec);
    }

    std::string newspec = rcfile.getSOLSafeDir();
    if (newspec.size() == 0) {
        newspec = "/tmp/";
    }

    // TODO: check if the base dir exists here, or skip the flush
    struct stat statbuf;
    if ( -1 == stat(newspec.c_str(), &statbuf) )
    {
       log_error("Invalid SOL safe dir %s: %s", newspec, std::strerror(errno));
        return as_value(false);
    }
    
    // Which URL we should use here is under research.
    // The reference player uses the URL from which definition
    // of the call to SharedObject.getLocal was parsed.
    //
    // There is in Gnash support for tracking action_buffer 
    // urls but not yet an interface to fetch it from fn_call;
    // also, it's not clear how good would the model be (think
    // of movie A loading movie B creating the SharedObject).
    //
    // What we'll do for now is use the URL of the initially
    // loaded SWF, so that in the A loads B scenario above the
    // domain would be the one of A, not B.
    //
    // NOTE: using the base url (get_base_url) would mean
    // blindly trusting the SWF publisher as base url is changed
    // by the 'base' attribute of OBJECT or EMBED tags trough
    // -P base=xxx
    //
    const std::string& origURL = obj->getVM().getSWFUrl(); 
    
    URL url(origURL);
//  log_debug(_("BASE URL=%s (%s)"), url.str(), url.hostname());

    // Get the domain part, or take as 'localhost' if none
    // (loaded from filesystem)
    //
    std::string domain = url.hostname();
    if (domain.empty()) domain = "localhost";
    
    // Get the path part
    std::string swfile = url.path();
	// TODO: if the original url was a relative one, the pp uses just
	// the relative portion rather then the resolved absolute path !
    
    if ( rcfile.getSOLLocalDomain() && domain != "localhost") 
    {
        log_security("Attempting to open non localhost created SOL file!! %s",
                     obj->getFilespec());
        return as_value(false);
    }

    // The optional second argument drops the domain and the swf file name
    //
    // NOTE: having more then 2 args should still use the second
    //       (and discard the subsequents).
    //
    if (fn.nargs > 1) {
        rootdir = fn.arg(1).to_string();
        log_debug("The rootdir is: %s", rootdir);
        newspec += rootdir;
    } else {
        newspec += "/";    
        newspec += domain;
        newspec += "/";
        newspec += swfile;
        newspec += "/";
    }
    
    //log_debug("newspec before adding obj's filespec: %s", newspec);
    newspec += obj->getFilespec();
    obj->setFilespec(newspec);
        
    if (newspec.find("/", 0) != std::string::npos) {
        typedef boost::tokenizer<boost::char_separator<char> > Tok;
        boost::char_separator<char> sep("/");
        Tok t(newspec, sep);
        Tok::iterator tit;
        std::string newdir = "/";
        for(tit=t.begin(); tit!=t.end();++tit){
            //cout << *tit << "\n";
            newdir += *tit;
            if (newdir.find("..", 0) != std::string::npos) {
		log_error("Invalid SharedObject path (contains '..'): %s", newspec);
                return as_value(false);
            }
            // Don't try to create a directory of the .sol file name!
            // TODO: don't fail if the movie url has a component ending with .sol (eh...)
            //
            if (newdir.rfind(".sol") != (newdir.size()-4)) {
#ifndef _WIN32
                int ret = mkdir(newdir.c_str(), S_IRUSR|S_IWUSR|S_IXUSR);
#else
                int ret = mkdir(newdir.c_str());
#endif
                if ((errno != EEXIST) && (ret != 0)) {
                    log_error("Couldn't create directory for .sol files: %s\n\t%s",
                              newdir, std::strerror(errno));
                    return as_value(false);
                }
            } // else log_debug("newdir %s ends with .sol", newdir);
            newdir += "/";
        }
    } // else log_debug("no slash in filespec %s", obj->getFilespec());

    SOL sol;
    log_security("Opening SharedObject file: %s", newspec);
    if (sol.readFile(newspec) == false) {
        log_security("empty or non-existing SOL file \"%s\", will be created on flush/exit", newspec);
        return as_value(obj.get());
    }
    
    std::vector<Element *>::const_iterator it, e;
    std::vector<Element *> els = sol.getElements();
    log_debug("Read %d AMF objects from %s", els.size(), newspec);

    string_table& st = obj->getVM().getStringTable();
    string_table::key dataKey =  obj->getVM().getStringTable().find("data");
    as_value as = obj->getMember(dataKey);
    boost::intrusive_ptr<as_object> ptr = as.to_object();
    
    for (it = els.begin(), e = els.end(); it != e; it++) {
        Element *el = *it;

        switch (el->getType())
        {
            case Element::NUMBER_AMF0:
            {
                double dub =  *(reinterpret_cast<double*>(el->getData()));
                ptr->set_member(st.string_table::find(el->getName()), as_value(dub));
                break;
            }

            case Element::BOOLEAN_AMF0:
                ptr->set_member(st.string_table::find(el->getName()),
                                            as_value(el->to_bool()));
                break;

            case Element::STRING_AMF0:
            {
                if (el->getLength() == 0) {
                    ptr->set_member(st.string_table::find(el->getName()), as_value(""));
                    break;
                }
                
                std::string str(reinterpret_cast<const char*>(el->getData()), el->getLength());
                ptr->set_member(st.string_table::find(el->getName()), as_value(str));
                break;
            }

            case Element::OBJECT_AMF0:
                // TODO: implement!
                //data.convert_to_object();
                //ptr->set_member(st.string_table::find(el->name), data);
                break;

            default:
                // TODO: what about other types?
                break;
        } 

    }

//    ptr->dump_members();        // FIXME: debug crap
    
    return as_value(obj.get()); // will keep alive
}

as_value
sharedobject_getsize(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<SharedObject> obj = ensureType<SharedObject>(fn.this_ptr);
    return as_value(obj->size());
}

as_value
sharedobject_ctor(const fn_call& /* fn */)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<as_object> obj = new SharedObject;
//    static boost::intrusive_ptr<as_object> obj = new as_object(getSharedObjectInterface());
    
    return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void sharedobject_class_init(as_object& global)
{
//    GNASH_REPORT_FUNCTION;
    // This is going to be the global SharedObject "class"/"function"
    static boost::intrusive_ptr<builtin_function> cl;
    
    if (cl == NULL) {
        cl=new builtin_function(&sharedobject_ctor, getSharedObjectInterface());
        attachSharedObjectStaticInterface(*cl);
    }
    
    // Register _global.SharedObject
    global.init_member("SharedObject", cl.get());    
}

} // end of gnash namespace
