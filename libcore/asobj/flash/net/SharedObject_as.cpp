// SharedObject_as.cpp:  ActionScript "SharedObject" class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "movie_root.h"
#include "GnashSystemNetHeaders.h"
#include "GnashFileUtilities.h" // stat
#include "SimpleBuffer.h"
#include "as_value.h"
#include "action.h"
#include "amf.h"
#include "element.h"
#include "sol.h"
#include "net/SharedObject_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface
#include "VM.h"
#include "Property.h"
#include "string_table.h"
#include "rc.h" // for use of rcfile
#include "URLAccessManager.h"
#include "network.h"
#include "rtmp_client.h"
#include "URL.h"

#include "NetConnection_as.h"
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

// Undefine this to use the Element-based AMF0 decoder/encoder.
// May be useful to test libamf.
#define BUFFERED_AMF_SOL

namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

using namespace amf;
using namespace std;

namespace gnash {

// Forward declarations
namespace {

    as_value sharedobject_connect(const fn_call& fn);
    as_value sharedobject_send(const fn_call& fn);
    as_value sharedobject_flush(const fn_call& fn);
    as_value sharedobject_close(const fn_call& fn);
    as_value sharedobject_getsize(const fn_call& fn);
    as_value sharedobject_setFps(const fn_call& fn);
    as_value sharedobject_clear(const fn_call& fn);
    as_value sharedobject_deleteAll(const fn_call& fn);
    as_value sharedobject_getDiskUsage(const fn_call& fn);
    as_value sharedobject_getRemote(const fn_call& fn);
    as_value sharedobject_data(const fn_call& fn);
    as_value sharedobject_getLocal(const fn_call& fn);
    as_value sharedobject_ctor(const fn_call& fn);
    as_value sharedobject_setdirty(const fn_call& fn);
    as_value sharedobject_setproperty(const fn_call& fn);

    as_object* readSOL(VM& vm, const std::string& filespec);

    as_object* getSharedObjectInterface();
    void attachSharedObjectStaticInterface(as_object& o);
    void flushSOL(SharedObjectLibrary::SoLib::value_type& sol);
    bool validateName(const std::string& solName);
}

// Serializer helper
namespace { 

class PropsSerializer : public AbstractPropertyVisitor
{
public:

    PropsSerializer(SOL& sol, VM& vm)
        :
        _sol(sol),
        _st(vm.getStringTable())
    {};

    void accept(string_table::key key, const as_value& val) 
    {
        AMF amf;
        boost::shared_ptr<amf::Element> el;
        
        const std::string& name = _st.string_table::value(key);

        //log_debug("Serializing SharedObject property %s:%s", name, val);

        if (val.is_string()) {
            std::string str;
            if (!val.is_undefined()) {
                str = val.to_string();
            }
            el.reset(new amf::Element(name, str));
        }
        if (val.is_bool()) {
            bool flag = val.to_bool();
            el.reset(new amf::Element(name, flag));
        }
        if (val.is_number()) { 
            double dub;
            if (val.is_undefined()) {
                dub = 0.0;
            } else {
                dub = val.to_number();
            }
            el.reset(new amf::Element(name, dub));
        }

        if (el) {
            _sol.addObj(el);
        }
    }

private:

    SOL& _sol;
    string_table& _st;
};

/// Class used to serialize properties of an object to a buffer in SOL format
class SOLPropsBufSerializer : public AbstractPropertyVisitor
{

    typedef std::map<as_object*, size_t> PropertiesOffsetTable;

public:

    SOLPropsBufSerializer(SimpleBuffer& buf, VM& vm,
            PropertiesOffsetTable& offsetTable)
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

        if ( val.is_function() )
        {
            log_debug("SOL: skip serialization of FUNCTION property");
            return;
        }

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members
        // of an object don't get back from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member gets back, but only if 
        // not a function. Actually no function gets back.
        // 
        if ( key == NSV::PROP_uuPROTOuu || key == NSV::PROP_CONSTRUCTOR )
        {
#ifdef GNASH_DEBUG_AMF_SERIALIZE
            log_debug(" skip serialization of specially-named property %s",
                    _st.value(key));
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
        // Strict array are never encoded in SharedObject
        if ( ! val.writeAMF0(_buf, _offsetTable, _vm, false) )
        {
            log_error("Problems serializing an object's member %s=%s",
                    name, val);
            _error = true;
        }

        _buf.appendByte(0); // SOL-specific
    }

private:

    SimpleBuffer& _buf;
    VM& _vm;
    string_table& _st;
    PropertiesOffsetTable& _offsetTable;
    bool _error;
};

} // anonymous namespace

class SharedObject_as: public as_object, public RTMPClient
{
public:

    ~SharedObject_as();

    SharedObject_as()
        : as_object(getSharedObjectInterface()),
          _data(0),
          _persistance(0),
	  _connected(false)
    { 
    }

    bool flush(int space = 0) const;

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

    /// This isn't correct, as the default implementation doesn't use SOL
    /// for reading.
    size_t size() const { 
        return _sol.fileSize(); 
    }

    void setData(as_object* data) {

        assert(data);
        _data = data;

        const int flags = PropFlags::dontDelete |
                          PropFlags::readOnly;

        init_property(NSV::PROP_DATA, &sharedobject_data, &sharedobject_data,
                flags);

    }

    as_object* data() {
        return _data;
    }

    const as_object* data() const {
        return _data;
    }

    bool getPersistance() { return _persistance; };
    void setPersistance(bool flag) { _persistance = flag; };

    /// Process the close() method.
    void close();

    /// Process the connect(uri) method.
    void connect(NetConnection_as *obj, const std::string& uri);

    void setURI(const std::string &url) { _uri = url; }
    std::string &getURI() { return _uri; }

    bool isConnected() { return _connected; };
    void isConnected(bool x) { _connected = x; };

protected:
#ifdef GNASH_USE_GC
    void markReachableResources() const;
#endif

private:

    as_object   *_data;
    bool        _persistance;
    SOL		_sol;
    bool	_connected;
    std::string	_uri;
};


SharedObject_as::~SharedObject_as()
{
}


bool
SharedObject_as::flush(int space) const
{

    /// This is called on on destruction of the SharedObject, or (allegedly)
    /// on a call to SharedObject.data, so _data is not guaranteed to exist.
    //
    /// The function should never be called from SharedObject.flush() when
    /// _data is 0.
    if (!_data) return false;

    if (space > 0) {
        log_unimpl("SharedObject.flush() called with a minimum disk space "
                "argument (%d), which is currently ignored", space);
    }

    const std::string& filespec = _sol.getFilespec();

    if (!mkdirRecursive(filespec))
    {
        log_error("Couldn't create dir for flushing SharedObject %s", filespec);
        return false;
    }

#ifdef USE_SOL_READONLY
    log_debug(_("SharedObject %s not flushed (compiled as read-only mode)"),
            filespec);
    return false;
#endif

    if (rcfile.getSOLReadOnly() ) {
        log_security("Attempting to write object %s when it's SOL "
                "Read Only is set! Refusing...", filespec);
        return false;
    }
    
#ifdef BUFFERED_AMF_SOL

    gnash::SimpleBuffer buf;
    // see http://osflash.org/documentation/amf/envelopes/sharedobject

    // length field filled in later
    buf.append("\x00\xbf\x00\x00\x00\x00TCSO\x00\x04\x00\x00\x00\x00", 16); 

    // append object name
    std::string object_name = getObjectName();
    boost::uint16_t len = object_name.length();
    buf.appendNetworkShort(len);
    buf.append(object_name.c_str(), len);

    // append padding
    buf.append("\x00\x00\x00\x00", 4);

    // append properties of object
    VM& vm = getVM(*this);

    std::map<as_object*, size_t> offsetTable;
    SOLPropsBufSerializer props(buf, vm, offsetTable);
    _data->visitPropertyValues(props);
    if ( ! props.success() ) 
    {
        log_error("Could not serialize object");
        return false;
    }

    // fix length field
    *(reinterpret_cast<uint32_t*>(buf.data() + 2)) = htonl(buf.size() - 6);
    
    // TODO write file
    std::ofstream ofs(filespec.c_str(), std::ios::binary);
    if (!ofs) {
        log_error("SharedObject::flush(): Failed opening file '%s' in "
                "binary mode", filespec.c_str());
        return false;
    }
    
    if (ofs.write(reinterpret_cast<const char*>(buf.data()), buf.size()).fail())
    {
        log_error("Error writing %d bytes to output file %s",
                buf.size(), filespec.c_str());
        ofs.close();
        return false;
    }
    ofs.close();

#else // amf::SOL-based serialization

    // append properties of object
    VM& vm = getVM(*this)

    SOL sol;
    PropsSerializer props(sol, vm);
    _data->visitPropertyValues(props);
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

/// Process the close() method.
void
SharedObject_as::close()
{
}

/// Process the connect(uri) method.
void
SharedObject_as::connect(NetConnection_as */* obj */, const std::string& /* uri */)
{
    GNASH_REPORT_FUNCTION;
   
}

SharedObjectLibrary::SharedObjectLibrary(VM& vm)
    :
    _vm(vm)
{
    GNASH_REPORT_FUNCTION;

    _solSafeDir = rcfile.getSOLSafeDir();
    if (_solSafeDir.empty()) {
        log_debug("Empty SOLSafeDir directive: we'll use '/tmp'");
        _solSafeDir = "/tmp/";
    }

    // Check if the base dir exists here
    struct stat statbuf;
    if ( -1 == stat(_solSafeDir.c_str(), &statbuf) )
    {
       log_debug("Invalid SOL safe dir %s: %s. Will try to create on "
               "flush/exit.", _solSafeDir, std::strerror(errno));
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
    // NOTE: using the base url RunResources::baseURL() would mean
    // blindly trusting the SWF publisher as base url is changed
    // by the 'base' attribute of OBJECT or EMBED tags trough
    // -P base=xxx
    //
    const movie_root& mr = _vm.getRoot();
    const std::string& swfURL = mr.getOriginalURL();

    URL url(swfURL);

    // Remember the hostname of our SWF URL. This can be empty if loaded
    // from the filesystem
    _baseDomain = url.hostname();

    const std::string& urlPath = url.path();

    // Get the path part. If loaded from the filesystem, the pp stupidly
    // removes the first directory.
    if (!_baseDomain.empty()) {
        _basePath = urlPath;
    }
    else if (!urlPath.empty()) {
        // _basePath should be empty if there are no slashes or just one.
        std::string::size_type pos = urlPath.find('/', 1);
        if (pos != std::string::npos) {
            _basePath = urlPath.substr(pos);
        }
    }

}

#ifdef GNASH_USE_GC
void
SharedObject_as::markReachableResources() const
{
    if (_data) _data->setReachable();
}
#endif // GNASH_USE_GC

#ifdef GNASH_USE_GC
void
SharedObjectLibrary::markReachableResources() const
{
    for (SoLib::const_iterator it = _soLib.begin(), itE = _soLib.end();
            it != itE; ++it)
    {
        SharedObject_as* sh = it->second;
        sh->setReachable();
    }
}
#endif

/// The SharedObjectLibrary keeps all known SharedObjects alive. They must
/// be flushed on clear(). This is called at the latest by the dtor, which
/// is called at the latest by VM's dtor (currently earlier to avoid problems
/// with the GC).
void
SharedObjectLibrary::clear()
{
    std::for_each(_soLib.begin(), _soLib.end(), &flushSOL);
    _soLib.clear();
}

SharedObjectLibrary::~SharedObjectLibrary()
{
    clear();
}

SharedObject_as*
SharedObjectLibrary::getLocal(const std::string& objName,
        const std::string& root)
{
    assert (!objName.empty());

    // already warned about it at construction time
    if (_solSafeDir.empty()) return 0;

    if (rcfile.getSOLLocalDomain() && !_baseDomain.empty()) 
    {
        log_security("Attempting to open SOL file from non "
                "localhost-loaded SWF");
        return 0;
    }

    // Check that the name is valid; if not, return null
    if (!validateName(objName)) return 0;

    // The 'root' argument, otherwise known as localPath, specifies where
    // in the SWF path the SOL should be stored. It cannot be outside this
    // path.
    std::string requestedPath;

    // If a root is specified, check it first for validity
    if (!root.empty()) {

        const movie_root& mr = _vm.getRoot();
        const std::string& swfURL = mr.getOriginalURL();
        // The specified root may or may not have a domain. If it doesn't,
        // this constructor will add the SWF's domain.
        URL localPath(root, swfURL);
        
        StringNoCaseEqual noCaseCompare;

        // All we care about is whether the domains match. They may be 
        // empty filesystem-loaded.
        if (!noCaseCompare(localPath.hostname(), _baseDomain)) {
            log_security(_("SharedObject path %s is outside the SWF domain "
                        "%s. Cannot access this object."), localPath, 
                        _baseDomain);
            return 0;
        }

        requestedPath = localPath.path();

        // The domains match. Now check that the path is a sub-path of 
        // the SWF's URL. It is done by case-insensitive string comparison,
        // so a double slash in the requested path will fail.
        if (!noCaseCompare(requestedPath,
                    _basePath.substr(0, requestedPath.size()))) {
            log_security(_("SharedObject path %s is not part of the SWF path "
                        "%s. Cannot access this object."), requestedPath, 
                        _basePath);
            return 0;
        }

    }

    // A leading slash is added later
    std::ostringstream solPath;

    // If the domain name is empty, the SWF was loaded from the filesystem.
    // Use "localhost".
    solPath << (_baseDomain.empty() ? "localhost" : _baseDomain);

    // Paths should start with a '/', so we shouldn't have to add another
    // one.
    assert(requestedPath.empty() ? _basePath[0] == '/' :
                                    requestedPath[0] == '/');

    // If no path was requested, use the SWF's path.
    solPath << (requestedPath.empty() ? _basePath : requestedPath) << "/"
            << objName;

    // TODO: normalize key!

    const std::string& key = solPath.str();

    // If the shared object was already opened, use it.
    SoLib::iterator it = _soLib.find(key);
    if ( it != _soLib.end() )
    {
        log_debug("SharedObject %s already known, returning it", key);
        return it->second;
    }

    log_debug("SharedObject %s not loaded. Loading it now", key);

    // Otherwise create a new one and register to the lib
    SharedObject_as* obj = new SharedObject_as;
    _soLib[key] = obj;

    obj->setObjectName(objName);

    std::string newspec = _solSafeDir;
    newspec += "/";
    newspec += key;
    newspec += ".sol";
    obj->setFilespec(newspec);

    log_debug("SharedObject path: %s", newspec);
        
    boost::intrusive_ptr<as_object> data = readSOL(_vm, newspec);

    /// Don't set to 0, or it will initialize a property.
    if (data) obj->setData(data.get());

    return obj;
}

SharedObject_as*
SharedObjectLibrary::getRemote(const std::string& objName,
                               const std::string& uri,
                               const std::string& persistance)
{
    GNASH_REPORT_FUNCTION;

    assert (!objName.empty());

    // Check that the name is valid; if not, return null
    if (!validateName(objName)) {
        return 0;
    }

    // The 'root' argument, otherwise known as localPath, specifies where
    // in the SWF path the SOL should be stored. It cannot be outside this
    // path.
    std::string requestedPath;
    std::ostringstream solPath;
    URL url(uri);
    
    const std::string& key = url.path();

    // If the shared object was already opened, use it.
    SoLib::iterator it = _soLib.find(key);
    if (it != _soLib.end()) {
        log_debug("SharedObject %s already known, returning it", key);
        return it->second;
    } else {
        log_debug("SharedObject %s not loaded. Loading it now", key);
    }

    // Otherwise create a new one and register to the lib
    SharedObject_as* obj = new SharedObject_as;
    _soLib[key] = obj;

    obj->setObjectName(objName);

    // Not persistance on either the client or the server
    if (persistance == "false") {
        obj->setPersistance(false);
    }
    // Persistance only on the server
    if (persistance == "true") {
        obj->setPersistance(true);
    }
    
    boost::intrusive_ptr<as_object> data;
    if (persistance[0] == '/') {
        obj->setPersistance(true);
        boost::intrusive_ptr<as_object> localdata = getLocal(objName, url.path());
        if (localdata) {
            obj->setData(localdata.get());
        }
    }

    return obj;
}

void
sharedobject_class_init(as_object& global, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;
    
    if (cl == NULL) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&sharedobject_ctor, getSharedObjectInterface());
        attachSharedObjectStaticInterface(*cl);
    }
    
    // Register _global.SharedObject
    global.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));    
}

void
registerSharedObjectNative(as_object& o)
{
    VM& vm = getVM(o);

    // ASnative table registration
    vm.registerNative(sharedobject_connect, 2106, 0);
    vm.registerNative(sharedobject_send, 2106, 1);
    vm.registerNative(sharedobject_flush, 2106, 2);
    vm.registerNative(sharedobject_close, 2106, 3);
    vm.registerNative(sharedobject_getsize, 2106, 4);
    vm.registerNative(sharedobject_setFps, 2106, 5);
    vm.registerNative(sharedobject_clear, 2106, 6);

    // FIXME: getRemote and getLocal use both these methods,
    // but aren't identical with either of them.
    // TODO: The first method looks in a library and returns either a
    // SharedObject or null. The second takes a new SharedObject as
    // its first argument and populates its data member (more or less
    // like readSOL). This is only important for ASNative compatibility.
    vm.registerNative(sharedobject_getLocal, 2106, 202);
    vm.registerNative(sharedobject_getRemote, 2106, 203);
    vm.registerNative(sharedobject_getLocal, 2106, 204);
    vm.registerNative(sharedobject_getRemote, 2106, 205);
    
    vm.registerNative(sharedobject_deleteAll, 2106, 206);
    vm.registerNative(sharedobject_getDiskUsage, 2106, 207);
    vm.registerNative(sharedobject_setdirty, 2106, 208);
    vm.registerNative(sharedobject_setproperty, 2106, 209);
}


/// SharedObject AS interface
namespace {

void
attachSharedObjectInterface(as_object& o)
{

    VM& vm = getVM(o);

    const int flags = PropFlags::dontEnum |
                      PropFlags::dontDelete |
                      PropFlags::onlySWF6Up;

    o.init_member("connect", vm.getNative(2106, 0), flags);
    o.init_member("send", vm.getNative(2106, 1), flags);
    o.init_member("flush", vm.getNative(2106, 2), flags);
    o.init_member("close", vm.getNative(2106, 3), flags);
    o.init_member("getSize", vm.getNative(2106, 4), flags);
    o.init_member("setFps", vm.getNative(2106, 5), flags);
    o.init_member("clear", vm.getNative(2106, 6), flags);

    o.init_member("setDirty", vm.getNative(2106, 7), flags);
    o.init_member("setProperty", vm.getNative(2106, 8), flags);
}


void
attachSharedObjectStaticInterface(as_object& o)
{
    VM& vm = getVM(o);

    const int flags = 0;

    Global_as* gl = getGlobal(o);
    o.init_member("getLocal", 
            gl->createFunction(sharedobject_getLocal), flags);
    o.init_member("getRemote",
            gl->createFunction(sharedobject_getRemote), flags);

    const int hiddenOnly = PropFlags::dontEnum;

    o.init_member("deleteAll",  vm.getNative(2106, 206), hiddenOnly);
    o.init_member("getDiskUsage",  vm.getNative(2106, 207), hiddenOnly);
}


as_object*
getSharedObjectInterface()
{

    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object(getObjectInterface());
        attachSharedObjectInterface(*o);
    }
    return o.get();
}


as_value
sharedobject_clear(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> obj = 
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(obj);
    
    LOG_ONCE(log_unimpl (__FUNCTION__));

    return as_value();
}

as_value
sharedobject_setdirty(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> obj = 
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(obj);
    
    LOG_ONCE(log_unimpl (__FUNCTION__));

    return as_value();
}

as_value
sharedobject_setproperty(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;    
    boost::intrusive_ptr<SharedObject_as> obj = 
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(obj);
    
    LOG_ONCE(log_unimpl (__FUNCTION__));

    return as_value();
}

as_value
sharedobject_connect(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;    

    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("SharedObject.connect(): needs at least "
                    "one argument"));
        );
        return as_value();
    }

    // Although the ActionScript spec says connect() takes two
    // arguments, the HAXE implementation only supports one.
    // So we have to make sure the NetCnnection object we get
    // passed is already had the URI specified to connect to.
    if (fn.nargs > 1) {
	const as_value& uri = fn.arg(1);
	const VM& vm = getVM(fn);
 	const std::string& uriStr = uri.to_string_versioned(vm.getSWFVersion());
    }
    
    boost::intrusive_ptr<NetConnection_as> nc =
	boost::dynamic_pointer_cast<NetConnection_as>(						     fn.arg(0).to_object(*getGlobal(fn)));

    // This is always set without validification.fooc->setURI(uriStr);
    string str = nc->getURI();
    obj->setPath(str);
    URL uri = nc->getURI();
    Network *net = new Network;

    net->setProtocol(uri.protocol());
    net->setHost(uri.hostname());
    net->setPort(strtol(uri.port().c_str(), NULL, 0) & 0xffff);

    // Check first arg for validity 
    if (getSWFVersion(fn) > 6) {
        nc->connect();
    } else {
        if (fn.nargs > 0) {
            std::stringstream ss; fn.dump_args(ss);
            log_unimpl("SharedObject.connect(%s): args after the first are "
                    "not supported", ss.str());
        }
        nc->connect();
    }
    
    return as_value();
}

as_value
sharedobject_close(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);

    obj->close();

    return as_value();
}

as_value
sharedobject_setFps(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);
    UNUSED(obj);

    LOG_ONCE(log_unimpl("SharedObject.setFps"));
    return as_value();
}

as_value
sharedobject_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);

    if (obj->isConnected() == false) {
	obj->connectToServer(obj->getURI());
    }
    
    return as_value();
}

as_value
sharedobject_flush(const fn_call& fn)
{    
    GNASH_REPORT_FUNCTION;

    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);

    IF_VERBOSE_ASCODING_ERRORS(
        if (fn.nargs > 1)
        {
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("Arguments to SharedObject.flush(%s) will be "
                    "ignored"), ss.str());
        }
    );

    int space = 0;
    if (fn.nargs) {
        space = fn.arg(0).to_int();
    }

    /// If there is no data member, returns undefined.
    if (!obj->data()) return as_value();

    // If there is an object data member, returns the success of flush().
    return as_value(obj->flush(space));
}

// Set the file name
as_value
sharedobject_getLocal(const fn_call& fn)
{
    int swfVersion = getSWFVersion(fn);

    as_value objNameVal;
    if (fn.nargs > 0) objNameVal = fn.arg(0);
    std::string objName = objNameVal.to_string_versioned(swfVersion);
    if ( objName.empty() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("SharedObject.getLocal(%s): %s", 
                _("missing object name"));
        );
        as_value ret;
        ret.set_null();
        return ret;
    }

    std::string root;
    if (fn.nargs > 1)
    {
        root = fn.arg(1).to_string_versioned(swfVersion);
    }

    log_debug("SO name:%s, root:%s", objName, root);

    VM& vm = getVM(fn);

    SharedObject_as* obj = vm.getSharedObjectLibrary().getLocal(objName, root);

    as_value ret(obj);
    log_debug("SharedObject.getLocal returning %s", ret);
    return ret;
}

/// Undocumented
as_value
sharedobject_getRemote(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;

    int swfVersion = getSWFVersion(fn);
    as_value objNameVal;

    if (fn.nargs > 0) {
        objNameVal = fn.arg(0);
    }
    
    std::string objName = objNameVal.to_string_versioned(swfVersion);
    if (objName.empty()) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("SharedObject.getRemote(%s): %s", 
                _("missing object name"));
        );
        as_value ret;
        ret.set_null();
        return ret;
    }

    std::string root;
    std::string persistance;
    if (fn.nargs > 1) {
        root = fn.arg(1).to_string_versioned(swfVersion);
        persistance = fn.arg(2).to_string_versioned(swfVersion);
    }

    log_debug("SO name:%s, root:%s, persitance: %s", objName, root, persistance);

    VM& vm = getVM(fn);

    SharedObject_as* obj = vm.getSharedObjectLibrary().getRemote(objName, root, persistance);

    as_value ret(obj);
    log_debug("SharedObject.getRemote returning %s", ret);
    
//     string_table::key methodKey = NSV::PROP_ON_STATUS;
//     callMethod(methodKey, as_value());
    
    return ret;
}


/// Undocumented
//
/// Takes a URL argument and deletes all SharedObjects under that URL.
as_value
sharedobject_deleteAll(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);

    UNUSED(obj);

    LOG_ONCE(log_unimpl("SharedObject.deleteAll()"));
    return as_value();
}

/// Undocumented
//
/// Should be quite obvious what it does.
as_value
sharedobject_getDiskUsage(const fn_call& fn)
{
 //    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);

    UNUSED(obj);

    LOG_ONCE(log_unimpl("SharedObject.getDiskUsage()"));
    return as_value();
}


as_value
sharedobject_data(const fn_call& fn)
{ 
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);
    return as_value(obj->data());
}

as_value
sharedobject_getsize(const fn_call& fn)
{
    boost::intrusive_ptr<SharedObject_as> obj =
        ensureType<SharedObject_as>(fn.this_ptr);
    return as_value(obj->size());
}

as_value
sharedobject_ctor(const fn_call& /* fn */)
{
    boost::intrusive_ptr<as_object> obj = new SharedObject_as;
    
    return as_value(obj.get()); // will keep alive
}

/// Return true if the name is a valid SOL name.
//
/// The official docs claim that '%' is also an invalid DisplayObject,
/// but that is incorrect (see actionscript.all/SharedObject.as)
bool
validateName(const std::string& solName)
{
    // A double forward slash isn't allowed
    std::string::size_type pos = solName.find("//");
    if (pos != std::string::npos) return false;

    // These DisplayObjects are also illegal
    pos = solName.find_first_of(",~;\"'<&>?#:\\ ");

    return (pos == std::string::npos);
}

as_object*
readSOL(VM& vm, const std::string& filespec)
{

#ifdef BUFFERED_AMF_SOL

    // The 'data' member is initialized only on getLocal() (and probably
    // getRemote()): i.e. when there is some data, or when it's ready to
    // be added.
    as_object* data = new as_object(getObjectInterface());

    struct stat st;

    if (stat(filespec.c_str(), &st) != 0) {
        // No existing SOL file. A new one will be created.
        log_debug("No existing SOL %s found. Will create on flush/exit.",
		  filespec);
        return data;
    }

    if (st.st_size < 28) {
        // A SOL file exists, but it was invalid. Count it as not existing.
        log_error("SharedObject::readSOL: SOL file %s is too short "
		  "(only %s bytes long) to be valid.", filespec, st.st_size);
        return data;
    }
    
    boost::scoped_array<boost::uint8_t> sbuf(new boost::uint8_t[st.st_size]);
    const boost::uint8_t *buf = sbuf.get();
    const boost::uint8_t *end = buf + st.st_size;

    try {
        std::ifstream ifs(filespec.c_str(), std::ios::binary);
        ifs.read(reinterpret_cast<char*>(sbuf.get()), st.st_size);

        // TODO check initial bytes, and print warnings if they are fishy

        buf += 16; // skip const-length headers

        // skip past name   TODO add sanity check
        buf += ntohs(*(reinterpret_cast<const boost::uint16_t*>(buf)));
        buf += 2;
        
        buf += 4; // skip past padding

        if (buf >= end) {
            // In this case there is no data member.
            log_error("SharedObject::readSOL: file ends before data segment");
            return data;
        }

        std::vector<as_object*> objRefs;

        while (buf < end) {
            log_debug("SharedObject::readSOL: reading property name at "
                    "byte %s", buf - sbuf.get());
            // read property name
            boost::uint16_t len = 
                ntohs(*(reinterpret_cast<const boost::uint16_t*>(buf)));
            buf += 2;

            if( buf + len >= end ) {
                log_error("SharedObject::readSOL: premature end of input");
                break;
            }
            if ( ! len ) {
                log_error("SharedObject::readSOL: empty property name");
                break;
            }
            std::string prop_name(reinterpret_cast<const char*>(buf), len);
            buf += len;

            // read value
            as_value as;
            if (!as.readAMF0(buf, end, -1, objRefs, vm)) {
                log_error("SharedObject::readSOL: Parsing SharedObject '%s'",
                        filespec);
                return false;
            }

            log_debug("parsed sol member named '%s' (len %s),  value '%s'",
                    prop_name, len, as);

            // set name/value as a member of this (SharedObject) object
            string_table& st = vm.getStringTable();
            data->set_member(st.find(prop_name), as);
            
            buf += 1; // skip null byte after each property
        }
        return data;
    }

    catch (std::exception& e) {
        log_error("SharedObject::readSOL: Reading SharedObject %s: %s", 
		  filespec, e.what());
        return 0;
    }

#else
    SOL sol;
    log_security("Opening SharedObject file: %s", filespec);
    if (sol.readFile(filespec) == false) {
        log_security("empty or non-existing SOL file \"%s\", will be "
                "created on flush/exit", filespec);
        return false;
    }
    
    std::vector<boost::shared_ptr<amf::Element> >::const_iterator it, e;
    std::vector<boost::shared_ptr<amf::Element> > els = sol.getElements();
    log_debug("Read %d AMF objects from %s", els.size(), filespec);

    as_value as = getMember(NSV::PROP_DATA);
    boost::intrusive_ptr<as_object> ptr = as.to_object(*getGlobal(fn));
    
    for (it = els.begin(), e = els.end(); it != e; it++) {
        boost::shared_ptr<amf::Element> el = *it;

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
                ptr->set_member(st.string_table::find(el->getName()),
                        as_value(dub));
                break;
            }

            case Element::BOOLEAN_AMF0:
                ptr->set_member(st.string_table::find(el->getName()),
                                            as_value(el->to_bool()));
                break;

            case Element::STRING_AMF0:
            {
                if (el->getLength() == 0) {
                    ptr->set_member(st.string_table::find(el->getName()), "");
                    break;
                }
                
                std::string str(reinterpret_cast<const char*>(el->getData()),
                        el->getLength());
                ptr->set_member(st.string_table::find(el->getName()), str);
                break;
            }

            case Element::OBJECT_AMF0:
                // TODO: implement!
                log_unimpl("Reading OBJECT type from SharedObject");
                //data.convert_to_object(*getGlobal(fn));
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


void
flushSOL(SharedObjectLibrary::SoLib::value_type& sol)
{
//    GNASH_REPORT_FUNCTION;
    sol.second->flush();
}

} // anonymous namespace
} // end of gnash namespace
