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
#include <boost/scoped_array.hpp>
#include <cerrno>

#include "SimpleBuffer.h"
#include "as_value.h"
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

// Define this to use the buffer-based AMF0 decoder/encoder
// rather then libamf. Fixes misc-ming.all/SharedObjectTestRunner
// both behavioural and for memory errors.
// The only failing case in that test is comparison of input
// and output .sol file. This is because ::writeAMF0 encodes
// arrays as STRICT_ARRAY rather then ECMA_ARRAY. Should be
// checked if this is a common need or only SOL-specific.
//
//#define BUFFERED_AMF_SOL

namespace {
//gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
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

namespace { 

class PropsSerializer : public AbstractPropertyVisitor {
    SOL& _sol;
    string_table& _st;
public:
    PropsSerializer(SOL& sol, VM& vm)
        :
        _sol(sol),
        _st(vm.getStringTable())
    {};

    void accept(string_table::key key, const as_value& val) 
        {
            //GNASH_REPORT_FUNCTION;
            AMF amf;
            Element *el = 0;

            const std::string& name = _st.string_table::value(key);

            //log_debug("Serializing SharedObject property %s:%s", name, val);

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

/// Class used to serialize properties of an object to a buffer in SOL format
class SOLPropsBufSerializer : AbstractPropertyVisitor {
    SimpleBuffer& _buf;
    VM& _vm;
    string_table& _st;
    std::map<as_object*, size_t>& _offsetTable;
    mutable bool _error;
public:
    SOLPropsBufSerializer(SimpleBuffer& buf, VM& vm, std::map<as_object*, size_t>& offsetTable)
        :
        _buf(buf),
        _vm(vm),
        _st(vm.getStringTable()),
        _offsetTable(offsetTable),
        _error(false)
	{};
    
    bool success() const { return !_error; }

    virtual void accept(string_table::key key, const as_value& val) 
    {
        if ( _error ) return;

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members
        // of an object don't get back from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member gets back, but only if 
        // not a function. Actually no function gets back.
        // 
        if ( key == NSV::PROP_uuPROTOuu || 
             key == NSV::PROP_CONSTRUCTOR )
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(" skip serialization of specially-named property %s", _st.value(key));
#endif
            return;
        }

        // write property name
        const std::string& name = _st.value(key);
#ifdef GNASH_DEBUG_AMF_SERIALIZE
        log_debug(" serializing property %s", name);
#endif
        boost::uint16_t namelen = name.size();
        _buf.appendNetworkShort(namelen);
        _buf.append(name.c_str(), namelen);
        if ( ! val.writeAMF0(_buf, _offsetTable, _vm) )
        {
            log_error("Problems serializing an object's member %s=%s", name, val);
            _error=true;
        }

        _buf.appendByte(0); // SOL-specific
    }
};

} // anonimous namespace

static void
attachProperties(as_object& o)
{
//    GNASH_REPORT_FUNCTION;
     as_object *proto = new as_object(getObjectInterface());
     o.init_member("data", proto, as_prop_flags::dontDelete|as_prop_flags::readOnly);
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


class SharedObject: public as_object 
{
public:

    ~SharedObject();

    SharedObject()
        :
        as_object(getSharedObjectInterface())
    { 
		attachProperties(*this);
    }

    bool flush() const;

    const std::string& getFilespec() const {
        return _sol.getFilespec();
    }

    void setFilespec(const std::string& s) {
        _sol.setFilespec(s);
    }

    const std::string& getObjectName() const {
        return _sol.getObjectName();
    }

    void setObjectName(const std::string& s) {
        _sol.setObjectName(s);
    }

    size_t size() const { 
        return _sol.size(); // TODO: fix this, is bogus
    }

    bool readSOL(const std::string& filename);

private:

    SOL _sol;
};

SharedObjectLibrary::SharedObjectLibrary(VM& vm)
    :
    _vm(vm),
    _soLib()
{
    _solSafeDir = rcfile.getSOLSafeDir();
    if (_solSafeDir.empty()) {
        log_debug("Empty SOLSafeDir directive: we'll use '/tmp'");
        _solSafeDir = "/tmp/";
    }

    // Check if the base dir exists here
    struct stat statbuf;
    if ( -1 == stat(_solSafeDir.c_str(), &statbuf) )
    {
       log_error("Invalid SOL safe dir %s: %s. Won't save any SharedObject.", _solSafeDir, std::strerror(errno));
        _solSafeDir.clear();
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
    const std::string& swfURL = _vm.getSWFUrl();

    // Get the domain part, or take as 'localhost' if none
    // (loaded from filesystem)
    URL url(swfURL);
//  log_debug(_("BASE URL=%s (%s)"), url.str(), url.hostname());
    _baseDomain = url.hostname();
    if ( _baseDomain.empty() ) _baseDomain = "localhost";

    // Get the path part
    _basePath = url.path();

	// TODO: if the original url was a relative one, the pp uses just
	// the relative portion rather then the resolved absolute path !
}

void
SharedObjectLibrary::markReachableResources() const
{
    for (SoLib::const_iterator it=_soLib.begin(), itE=_soLib.end(); it!=itE; ++it)
    {
        SharedObject* sh = it->second;
        sh->setReachable();
    }
}

static bool createDirForFile(const std::string& filename)
{
    if (filename.find("/", 0) != std::string::npos)
    {
        typedef boost::tokenizer<boost::char_separator<char> > Tok;
        boost::char_separator<char> sep("/");
        Tok t(filename, sep);
        Tok::iterator tit;
        std::string newdir = "/";
        for(tit=t.begin(); tit!=t.end();++tit){
            //cout << *tit << "\n";
            newdir += *tit;
            if (newdir.find("..", 0) != std::string::npos) {
		log_error("Invalid SharedObject path (contains '..'): %s", filename);
                return false;
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
                    return false;
                }
            } // else log_debug("newdir %s ends with .sol", newdir);
            newdir += "/";
        }
    }
    else log_debug("no slash in filespec %s", filename);
    return true;
}

SharedObject*
SharedObjectLibrary::getLocal(const std::string& objName, const std::string& root)
{
    assert ( ! objName.empty() );

    if ( _solSafeDir.empty() ) return 0; // already warned about it at construction time

    // TODO: this check sounds kind of lame, fix it
    if ( rcfile.getSOLLocalDomain() && _baseDomain != "localhost") 
    {
        log_security("Attempting to open SOL file from non localhost-loaded SWF");
        return 0;
    }

    // The optional second argument drops the domain and the swf file name
    std::string key;
    if ( root.empty() ) key = "/" + _baseDomain + "/" + _basePath + "/" + objName;
    else key = root + "/" + objName;

    // TODO: normalize key!

    // If the shared object was already opened, use it.
    SoLib::iterator it = _soLib.find(key);
    if ( it != _soLib.end() )
    {
        log_debug("SharedObject %s already known, returning it", key);
        return it->second;
    }
    log_debug("SharedObject %s not known, creating it", key);

    // Otherwise create a new one and register to the lib
    SharedObject* obj = new SharedObject();
    _soLib[key] = obj;

    obj->setObjectName(objName);

    std::string newspec = _solSafeDir;
    newspec += "/";
    newspec += key;
    newspec += ".sol";
    obj->setFilespec(newspec);

    log_debug("SharedObject path: %s", newspec);
        
    if ( ! obj->readSOL(newspec) )
    {
        log_error("Couldn't read SOL %s, will create on flush/exit", newspec);
    }

    return obj;
}

bool
SharedObject::readSOL(const std::string& filespec)
{
#ifdef BUFFERED_AMF_SOL
    struct stat st;

    if (stat(filespec.c_str(), &st) != 0)
    {
        return false;
    }

    if( st.st_size < 28 )
    {
        log_error("SharedObject::readSOL: SOL file %s is too short (only %s bytes long) to be valid.", filespec, st.st_size);
        return false;
    }

    boost::scoped_array<boost::uint8_t> sbuf(new boost::uint8_t[st.st_size]);
    boost::uint8_t *buf = sbuf.get();
    boost::uint8_t *end = buf + st.st_size;

    // FIXME clear existing key/value pairs?

    try
    {
        std::ifstream ifs(filespec.c_str(), std::ios::binary);
        ifs.read(reinterpret_cast<char *>(buf), st.st_size);

        // TODO check initial bytes, and print warnings if they are fishy

        buf += 16; // skip const-length headers

        // skip past name   TODO add sanity check
        buf += ntohs(*(reinterpret_cast<boost::uint16_t*>(buf)));
        buf += 2;
        
        buf += 4; // skip past padding

        if( buf >= end )
        {
            log_error("SharedObject::readSOL: file ends before data segment");
            return false;
        }

        string_table& strtab = _vm.getStringTable();
        std::vector<as_object*> objRefs;
        boost::intrusive_ptr<as_object> data = getMember(strtab.string_table::find("data")).to_object();

        while( buf < end )
        {
            log_debug("SharedObject::readSOL: reading property name at byte %s", buf - sbuf.get());
            // read property name
            boost::uint16_t len = ntohs(*(reinterpret_cast<boost::uint16_t*>(buf)));
            buf += 2;
            if( buf + len >= end )
            {
                log_error("SharedObject::readSOL: premature end of input");
                break;
            }
            if ( ! len ) {
                log_error("SharedObject::readSOL: empty property name");
                break;
            }
            std::string prop_name(reinterpret_cast<char*>(buf), len);
            buf += len;

            // read value
            as_value as;
            if(as.readAMF0(buf, end, -1 /* read type from buffer */, objRefs, _vm) == false) {
                log_error("SharedObject::readSOL: Parsing SharedObject '%s'", filespec);
                return false;
            }

            log_debug("parsed sol member named '%s' (len %s),  value '%s'", prop_name, len, as);

            // set name/value as a member of this (SharedObject) object
            data->set_member(strtab.find(prop_name), as);
            
            buf += 1; // skip null byte after each property
        }
        log_debug("setting data member: %s, %s", strtab.find(std::string("data")),  as_value(data.get()));
        set_member(strtab.find(std::string("data")), as_value(data.get()));
        return true;
    }
    catch (std::exception& e)
    {
        log_error("SharedObject::readSOL: Reading SharedObject %s: %s", filespec, e.what());
        return false;
    }


#else
    SOL sol;
    log_security("Opening SharedObject file: %s", filespec);
    if (sol.readFile(filespec) == false) {
        log_security("empty or non-existing SOL file \"%s\", will be created on flush/exit", filespec);
        return false;
    }
    
    std::vector<Element *>::const_iterator it, e;
    std::vector<Element *> els = sol.getElements();
    log_debug("Read %d AMF objects from %s", els.size(), filespec);

    string_table& st = _vm.getStringTable();
    string_table::key dataKey =  st.find("data");
    as_value as = getMember(dataKey);
    boost::intrusive_ptr<as_object> ptr = as.to_object();
    
    for (it = els.begin(), e = els.end(); it != e; it++) {
        Element *el = *it;

#if 0 // this would be using as_value::as_value(const Element&)

        std::string name(el->getName());
        as_value val(*el);
        ptr->set_member(st.find(name), val);

#else // this is original code 

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
                log_unimpl("Reading OBJECT type from SharedObject");
                //data.convert_to_object();
                //ptr->set_member(st.string_table::find(el->name), data);
                return false;
                break;

            default:
                // TODO: what about other types?
                log_unimpl("Reading SOL type %d", el->getType());
                return false;
                break;
        } 

#endif

    }

    return true;
#endif
}


SharedObject::~SharedObject()
{
    // flush(); // needs more care, if destroyed after VM we get killed
}


bool
SharedObject::flush() const
{
    const std::string& filespec = _sol.getFilespec();

    if ( ! createDirForFile(filespec) )
    {
        log_error("Couldn't create dir for flushing SharedObject %s", filespec);
        return false;
    }

#ifdef USE_SOL_READONLY
    log_debug(_("SharedObject %s not flushed (compiled as read-only mode)"), filespec);
    return false;
#endif

//    log_debug("Flushing to file %s", filespec);

    VM& vm = getVM();

    if (rcfile.getSOLReadOnly() ) {
        log_security("Attempting to write object %s when it's SOL Read Only is set! Refusing...",
                     filespec);
        return false;
    }
    
    // TODO: cache the dataKey in SharedObject prototype on first use ?
    //       a SharedObject::getDataKey() might do...
    string_table::key dataKey = vm.getStringTable().find("data");
    
    as_value as = const_cast<SharedObject*>(this)->getMember(dataKey);
    log_debug("data member of this SharedObject is %s", as);
    boost::intrusive_ptr<as_object> ptr = as.to_object();
    if ( ! ptr ) {
        log_aserror("'data' member of SharedObject is not an object (%s)",
                  as);
        return true;
    }

#ifdef BUFFERED_AMF_SOL

    gnash::SimpleBuffer buf;
    // see http://osflash.org/documentation/amf/envelopes/sharedobject
    buf.append("\x00\xbf\x00\x00\x00\x00TCSO\x00\x04\x00\x00\x00\x00", 16); // length field filled in later

    // append object name
    std::string object_name = getObjectName();
    boost::uint16_t len = object_name.length();
    buf.appendNetworkShort(len);
    buf.append(object_name.c_str(), len);

    // append padding
    buf.append("\x00\x00\x00\x00", 4);

    // append properties of object
    std::map<as_object*, size_t> offsetTable;
    SOLPropsBufSerializer props(buf, vm, offsetTable);
    ptr->visitPropertyValues(props);
    if ( ! props.success() ) 
    {
        log_error("Could not serialize object");
        return false;
    }

    // fix length field
    *(reinterpret_cast<uint32_t*>(buf.data() + 2)) = htonl(buf.size() - 6);
    
    // TODO write file
    std::ofstream ofs(filespec.c_str(), std::ios::binary);
    if(! ofs) {
        log_error("SharedObject::flush(): Failed opening file '%s' in binary mode", filespec.c_str());
        return false;
    }
    
    if(ofs.write(reinterpret_cast<const char*>(buf.data()), buf.size()).fail() )
    {
        log_error("Error writing %d bytes to output file %s", buf.size(), filespec.c_str());
        ofs.close();
        return false;
    }
    ofs.close();

#else // amf::SOL-based serialization

    SOL sol;
    PropsSerializer props(sol, vm);
    ptr->visitPropertyValues(props);
    // We only want to access files in this directory
    bool ret = sol.writeFile(filespec, getObjectName().c_str());
    if ( ! ret )
    {
        log_error("writing SharedObject file to %s", filespec);
        return false;
    }
#endif

    log_security("SharedObject '%s' written to filesystem.", filespec);
    return true;
}


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

    IF_VERBOSE_ASCODING_ERRORS(
    if ( fn.nargs )
    {
        std::stringstream ss;
        fn.dump_args(ss);
        log_aserror(_("Arguments to SharedObject.flush(%s) will be ignored"), ss.str());
    }
    )

    return as_value(obj->flush());
}

// Set the file name
as_value
sharedobject_getlocal(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;

    VM& vm = fn.env().getVM();
    int swfVersion = vm.getSWFVersion();

    as_value objNameVal;
    if (fn.nargs > 0) objNameVal = fn.arg(0);
    std::string objName = objNameVal.to_string_versioned(swfVersion);
    if ( objName.empty() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("SharedObject.getLocal(%s): %s", _("missing object name"));
        );
        as_value ret; ret.set_null();
        return ret;
    }

    std::string root;
    if (fn.nargs > 1)
    {
        root = fn.arg(1).to_string_versioned(swfVersion);
    }

    log_debug("SO name:%s, root:%s", objName, root);

    SharedObject* obj = vm.getSharedObjectLibrary().getLocal(objName, root);
    as_value ret(obj);
    log_debug("SharedObject.getLocal returning %s", ret);
    return ret;
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
