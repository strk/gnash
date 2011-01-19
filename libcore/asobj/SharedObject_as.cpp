// SharedObject_as.cpp:  ActionScript "SharedObject" class, for Gnash.
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

#include "SharedObject_as.h"

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdio>

#include "smart_ptr.h" 
#include "movie_root.h"
#include "GnashSystemNetHeaders.h"
#include "GnashFileUtilities.h" // stat
#include "SimpleBuffer.h"
#include "as_value.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h" 
#include "VM.h"
#include "Property.h"
#include "VM.h"
#include "rc.h" // for use of rcfile
#include "URL.h"
#include "NetConnection_as.h"
#include "Object.h"
#include "AMFConverter.h"
#include "GnashAlgorithm.h"
#include "RunResources.h"
#include "StreamProvider.h"
#include "namedStrings.h"

namespace {
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}


namespace gnash {

// Forward declarations
namespace {

    as_value sharedobject_connect(const fn_call& fn);
    as_value sharedobject_send(const fn_call& fn);
    as_value sharedobject_flush(const fn_call& fn);
    as_value sharedobject_close(const fn_call& fn);
    as_value sharedobject_getSize(const fn_call& fn);
    as_value sharedobject_setFps(const fn_call& fn);
    as_value sharedobject_clear(const fn_call& fn);
    as_value sharedobject_deleteAll(const fn_call& fn);
    as_value sharedobject_getDiskUsage(const fn_call& fn);
    as_value sharedobject_getRemote(const fn_call& fn);
    as_value sharedobject_data(const fn_call& fn);
    as_value sharedobject_getLocal(const fn_call& fn);
    as_value sharedobject_ctor(const fn_call& fn);

    as_object* readSOL(VM& vm, const std::string& filespec);

    /// Encode the SharedObject data.
    //
    /// @param name     The name of the SharedObject.
    /// @param data     The data object to encode.
    /// @param buf      The SimpleBuffer to encode the data to.
    bool encodeData(const std::string& name, as_object& data,
            SimpleBuffer& buf);

    /// Encode the 2 header bytes and data length field.
    //
    /// @param size     The genuine size of the object data.
    /// @param buf      The SimpleBuffer to encode the data to.
    void encodeHeader(const size_t size, SimpleBuffer& buf);

    void attachSharedObjectInterface(as_object& o);
    void attachSharedObjectStaticInterface(as_object& o);
    void flushSOL(SharedObjectLibrary::SoLib::value_type& sol);
    bool validateName(const std::string& solName);

    SharedObject_as* createSharedObject(Global_as& gl);
}

// Serializer helper
namespace { 

/// Class used to serialize properties of an object to a buffer in SOL format
class SOLPropsBufSerializer : public PropertyVisitor
{

public:

    SOLPropsBufSerializer(amf::Writer w, VM& vm)
        :
        _writer(w),
        _vm(vm),
        _error(false),
        _count(0)
	{}
    
    /// Check if the object data was successfully encoded.
    //
    /// Success means that no encoding errors were encountered and at least one
    /// property was encoded.
    bool success() const {
        return !_error && _count;
    }

    /// Visitor called for each property to serialize.
    //
    /// SOL serialization calls this for all existing properties, whether
    /// visible or not.
    //
    /// accept() returns false if the visitor should stop, true if it should
    /// go on to the next property.
    virtual bool accept(const ObjectURI& uri, const as_value& val) 
    {
        assert(!_error);

        if (val.is_function()) {
            log_debug("SOL: skip serialization of FUNCTION property");
            return true;
        }

        // Test conducted with AMFPHP:
        // '__proto__' and 'constructor' members of an object are not returned
        // from an 'echo-service'.
        // Dunno if they are not serialized or just not sent back.
        // A '__constructor__' member is returned, but only if 
        // not a function; no functions are returned at all.
	
	// TODO: check if this comparison must _really_ be case-sensitive
	bool caseless = false;
	ObjectURI::CaseEquals eq(_vm.getStringTable(), caseless);
        if (eq(uri, NSV::PROP_uuPROTOuu) || eq(uri, NSV::PROP_CONSTRUCTOR)) {
            return true;
        }

        // write property name
        const std::string& name = toString(_vm, uri);
        
        _writer.writePropertyName(name);

        // Strict array are never encoded in SharedObject
        if (!val.writeAMF0(_writer)) {
            log_error("Problems serializing an object's member %s=%s",
                    name, val);
            _error = true;

            // Stop visiting.
            return false;
        }

        // This is SOL specific.
        boost::uint8_t end(0);
        _writer.writeData(&end, 1);
        ++_count;
        return true;
    }

private:

    amf::Writer _writer;

    /// String table for looking up property names as strings.
    VM& _vm;

    /// Whether an error has been encountered.
    bool _error;

    /// How many properties have been encoded.
    //
    /// A boolean would do just as nicely, but we may want to use the count
    /// info.
    size_t _count;
};

} // anonymous namespace

class SharedObject_as : public Relay
{
public:

    SharedObject_as(as_object& owner)
        :
        _owner(owner),
        _data(0),
        _connected(false)
    { 
    }

    virtual ~SharedObject_as();

    /// The as_object that owns this Relay.
    as_object& owner() {
        return _owner;
    }

    /// Write the data as a SOL file.
    //
    /// If there is no data to write, the file is removed.
    bool flush(int space = 0) const;

    /// The filename of this SharedObject.
    const std::string& getFilespec() const {
        return _filename;
    }

    /// Set the filename of the SharedObject.
    void setFilespec(const std::string& s) {
        _filename = s;
    }

    /// Set the name of this SharedObject.
    void setObjectName(const std::string& s) {
        _name = s;
    }

    /// Return the size of the data plus header.
    size_t size() const {
        if (!_data) return 0;
        SimpleBuffer buf;

        // The header comprises 2 bytes and a length field of 4 bytes.
        if (encodeData(_name, *_data, buf)) {
            return buf.size() + 6;
        }
        return 0;
    }

    /// Set the data object.
    //
    /// It seems much cleaner not to implement this as a getter/setter as it
    /// currently is, but it is shown that flush() does not call data if
    /// data is a getter-setter. This suggests that flush does not access
    /// the data object directly, which in turn suggests it's not a real
    /// property. This is the only test that fails if the data object is a
    /// simple object property.
    void setData(as_object* data) {

        assert(data);
        _data = data;

        const int flags = PropFlags::dontDelete |
                          PropFlags::readOnly;

        _owner.init_property(NSV::PROP_DATA, &sharedobject_data,
                &sharedobject_data, flags);

    }

    /// Get the data object.
    as_object* data() const {
        return _data;
    }

    /// Close the SharedObject
    //
    /// Note that this currently does nothing.
    void close();

    /// Are we connected? 
    bool connected() const { return _connected; }

    /// Override from Relay.
    virtual void setReachable(); 

private:

    /// The as_object to which this Relay belongs.
    as_object& _owner;

    /// An object to store and access the actual data.
    as_object* _data;

    std::string _name;

    std::string _filename;

    /// Are we connected? (No).
    bool _connected;

};


SharedObject_as::~SharedObject_as()
{
}


/// Returns false if the data cannot be written to file.
//
/// If there is no data, the file is removed and the function returns true.
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

    const std::string& filespec = getFilespec();

    if (!mkdirRecursive(filespec)) {
        log_error("Couldn't create dir for flushing SharedObject %s", filespec);
        return false;
    }

#ifdef USE_SOL_READONLY
    log_debug(_("SharedObject %s not flushed (compiled as read-only mode)"),
            filespec);
    return false;
#endif

    if (rcfile.getSOLReadOnly()) {
        log_security("Attempting to write object %s when it's SOL "
                "Read Only is set! Refusing...", filespec);
        return false;
    }
    
    // Open file
    std::ofstream ofs(filespec.c_str(), std::ios::binary);
    if (!ofs) {
        log_error("SharedObject::flush(): Failed opening file '%s' in "
                "binary mode", filespec.c_str());
        return false;
    }

    // Encode data part.
    SimpleBuffer buf;
    if (!encodeData(_name, *_data, buf)) {
        std::remove(filespec.c_str());
        return true;
    }

    // Encode header part.
    SimpleBuffer header;
    encodeHeader(buf.size(), header);
    
    // Write header
    ofs.write(reinterpret_cast<const char*>(header.data()), header.size());
    if (!ofs) {
        log_error("Error writing SOL header");
        return false;
    }

    // Write AMF data
    ofs.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    if (!ofs) {
        log_error("Error writing %d bytes to output file %s",
                buf.size(), filespec.c_str());
        return false;
    }
    ofs.close();

    log_security("SharedObject '%s' written to filesystem.", filespec);
    return true;
}

void
SharedObject_as::close()
{
}

SharedObjectLibrary::SharedObjectLibrary(VM& vm)
    :
    _vm(vm)
{

    _solSafeDir = rcfile.getSOLSafeDir();
    if (_solSafeDir.empty()) {
        log_debug("Empty SOLSafeDir directive: we'll use '/tmp'");
        _solSafeDir = "/tmp/";
    }

    // Check if the base dir exists here
    struct stat statbuf;
    if (stat(_solSafeDir.c_str(), &statbuf) == -1) {
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
    const movie_root& mr = _vm.getRoot();
    const URL& url = mr.runResources().streamProvider().baseURL();

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

void
SharedObject_as::setReachable() 
{
    _owner.setReachable();
    if (_data) _data->setReachable();
}

void
SharedObjectLibrary::markReachableResources() const
{
    foreachSecond(_soLib.begin(), _soLib.end(),
                  &SharedObject_as::setReachable);
}

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

as_object*
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

        const URL& swfURL = mr.runResources().streamProvider().baseURL();
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
    if (it != _soLib.end()) {
        log_debug("SharedObject %s already known, returning it", key);
        return &it->second->owner();
    }

    log_debug("SharedObject %s not loaded. Loading it now", key);

    // Otherwise create a new one and register to the lib
    SharedObject_as* sh = createSharedObject(*_vm.getGlobal());
    if (!sh) return 0;

    sh->setObjectName(objName);

    std::string newspec = _solSafeDir;
    newspec += "/";
    newspec += key;
    newspec += ".sol";
    sh->setFilespec(newspec);

    log_debug("SharedObject path: %s", newspec);
        
    as_object* data = readSOL(_vm, newspec);

    /// Don't set to 0, or it will initialize a property.
    if (data) sh->setData(data);
    
    // The SharedObjectLibrary must set this as reachable.
    _soLib[key] = sh;

    return &sh->owner();
}


void
sharedobject_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);
    as_object* proto = createObject(gl);
    attachSharedObjectInterface(*proto);
    as_object* cl = gl.createClass(&sharedobject_ctor, proto);
    attachSharedObjectStaticInterface(*cl);
    
    // Register _global.SharedObject
    where.init_member(uri, cl, as_object::DefaultFlags);    
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
    vm.registerNative(sharedobject_getSize, 2106, 4);
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
}


void
attachSharedObjectStaticInterface(as_object& o)
{
    VM& vm = getVM(o);

    const int flags = 0;

    Global_as& gl = getGlobal(o);
    o.init_member("getLocal", gl.createFunction(sharedobject_getLocal), flags);
    o.init_member("getRemote",
            gl.createFunction(sharedobject_getRemote), flags);

    const int hiddenOnly = PropFlags::dontEnum;

    o.init_member("deleteAll",  vm.getNative(2106, 206), hiddenOnly);
    o.init_member("getDiskUsage",  vm.getNative(2106, 207), hiddenOnly);
}


as_value
sharedobject_clear(const fn_call& fn)
{
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);
    UNUSED(obj);
    
    LOG_ONCE(log_unimpl (__FUNCTION__));

    return as_value();
}

as_value
sharedobject_connect(const fn_call& fn)
{

    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);
    
    UNUSED(obj);

    if (fn.nargs < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("SharedObject.connect(): needs at least "
                    "one argument"));
        );
        return as_value();
    }

    LOG_ONCE(log_unimpl("SharedObject.connect()"));

    return as_value();
}

as_value
sharedobject_close(const fn_call& fn)
{
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);

    obj->close();

    return as_value();
}

as_value
sharedobject_setFps(const fn_call& fn)
{
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);
    UNUSED(obj);

    LOG_ONCE(log_unimpl("SharedObject.setFps"));
    return as_value();
}

as_value
sharedobject_send(const fn_call& fn)
{
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);
    UNUSED(obj);
    LOG_ONCE(log_unimpl("SharedObject.send"));
    return as_value();
}

/// Returns false only if there was a failure writing data to file.
as_value
sharedobject_flush(const fn_call& fn)
{    
    GNASH_REPORT_FUNCTION;

    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);

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
        space = toInt(fn.arg(0), getVM(fn));
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
    const int swfVersion = getSWFVersion(fn);

    as_value objNameVal;
    if (fn.nargs > 0) objNameVal = fn.arg(0);

    const std::string objName = objNameVal.to_string(swfVersion);
    if (objName.empty()) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("SharedObject.getLocal(%s): missing object name");
        );
        as_value ret;
        ret.set_null();
        return ret;
    }

    std::string root;
    if (fn.nargs > 1) {
        root = fn.arg(1).to_string(swfVersion);
    }

    log_debug("SO name:%s, root:%s", objName, root);

    VM& vm = getVM(fn);

    as_object* obj = vm.getSharedObjectLibrary().getLocal(objName, root);

    as_value ret(obj);
    log_debug("SharedObject.getLocal returning %s", ret);
    return ret;
}

as_value
sharedobject_getRemote(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl("SharedObject.getRemote()"));
    return as_value();
}


/// Officially undocumented.
//
/// Takes a URL argument and deletes all SharedObjects under that URL.
as_value
sharedobject_deleteAll(const fn_call& fn)
{
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);

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
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);

    UNUSED(obj);

    LOG_ONCE(log_unimpl("SharedObject.getDiskUsage()"));
    return as_value();
}


as_value
sharedobject_data(const fn_call& fn)
{ 
//    GNASH_REPORT_FUNCTION;
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);
    return as_value(obj->data());
}

as_value
sharedobject_getSize(const fn_call& fn)
{
    SharedObject_as* obj = ensure<ThisIsNative<SharedObject_as> >(fn);
    return as_value(obj->size());
}

as_value
sharedobject_ctor(const fn_call& /*fn*/)
{
    return as_value(); 
}

/// Return true if the name is a valid SOL name.
//
/// The official docs claim that '%' is also an invalid character 
/// but that is incorrect (see actionscript.all/SharedObject.as)
bool
validateName(const std::string& solName)
{
    // A double forward slash isn't allowed
    std::string::size_type pos = solName.find("//");
    if (pos != std::string::npos) return false;

    // These character are also illegal
    pos = solName.find_first_of(",~;\"'<&>?#:\\ ");

    return (pos == std::string::npos);
}

as_object*
readSOL(VM& vm, const std::string& filespec)
{

    Global_as& gl = *vm.getGlobal();

    // The 'data' member is initialized only on getLocal() (and probably
    // getRemote()): i.e. when there is some data, or when it's ready to
    // be added.
    as_object* data = createObject(gl);

    struct stat st;

    if (stat(filespec.c_str(), &st) != 0) {
        // No existing SOL file. A new one will be created.
        log_debug("No existing SOL %s found. Will create on flush/exit.",
		  filespec);
        return data;
    }

    const size_t size = st.st_size;

    if (size < 28) {
        // A SOL file exists, but it was invalid. Count it as not existing.
        log_error("readSOL: SOL file %s is too short "
		  "(only %s bytes long) to be valid.", filespec, st.st_size);
        return data;
    }

    boost::scoped_array<boost::uint8_t> sbuf(new boost::uint8_t[size]);
    const boost::uint8_t *buf = sbuf.get();
    const boost::uint8_t *end = buf + size;

    try {
        std::ifstream ifs(filespec.c_str(), std::ios::binary);
        ifs.read(reinterpret_cast<char*>(sbuf.get()), size);

        // TODO check initial bytes, and print warnings if they are fishy

        buf += 16; // skip const-length headers

        // skip past name   TODO add sanity check
        buf += ntohs(*(reinterpret_cast<const boost::uint16_t*>(buf)));
        buf += 2;
        
        buf += 4; // skip past padding

        if (buf >= end) {
            // In this case there is no data member.
            log_error("readSOL: file ends before data segment");
            return data;
        }

        amf::Reader rd(buf, end, gl);

        while (buf != end) {

            log_debug("readSOL: reading property name at "
                    "byte %s", buf - sbuf.get());
            // read property name
            
            if (end - buf < 2) {
                log_error("SharedObject: end of buffer while reading length");
                break;
            }

            const boost::uint16_t len = 
                ntohs(*(reinterpret_cast<const boost::uint16_t*>(buf)));
            buf += 2;

            if (!len) {
                log_error("readSOL: empty property name");
                break;
            }

            if (end - buf < len) {
                log_error("SharedObject::readSOL: premature end of input");
                break;
            }

            std::string prop_name(reinterpret_cast<const char*>(buf), len);
            buf += len;

            // read value
            as_value as;

            if (!rd(as)) {
                log_error("SharedObject: error parsing SharedObject '%s'",
                        filespec);
                return 0;
            }

            log_debug("parsed sol member named '%s' (len %s),  value '%s'",
                    prop_name, len, as);

            // set name/value as a member of this (SharedObject) object
            data->set_member(getURI(vm, prop_name), as);
            
            if (buf == end) break;;

            buf += 1; // skip null byte after each property
        }
        return data;
    }

    catch (std::exception& e) {
        log_error("readSOL: Reading SharedObject %s: %s", 
		  filespec, e.what());
        return 0;
    }

}


void
flushSOL(SharedObjectLibrary::SoLib::value_type& sol)
{
    sol.second->flush();
}

SharedObject_as*
createSharedObject(Global_as& gl)
{
    as_function* ctor = getMember(gl, NSV::CLASS_SHARED_OBJECT).to_function();
    if (!ctor) return 0;
    as_environment env(getVM(gl));
    fn_call::Args args;
    as_object* o = constructInstance(*ctor, env, args);

    std::auto_ptr<SharedObject_as> sh(new SharedObject_as(*o));
    o->setRelay(sh.release());

    // We know what it is...
    return &static_cast<SharedObject_as&>(*o->relay());;
}

/// Encode header data.
//
/// Note that the separation of header and data here is arbitrary.
void
encodeHeader(const size_t size, SimpleBuffer& buf)
{
    const boost::uint8_t header[] = { 0x00, 0xbf };
    
    // Initial header byters
    buf.append(header, arraySize(header));
    
    // Size of data plus 14 (complete size of data after size field).
    buf.appendNetworkLong(size);
}

/// This writes everything after the 'length' field of the SOL data.
bool
encodeData(const std::string& name, as_object& data, SimpleBuffer& buf)
{
    // Write the remaining header-like information.
    const boost::uint8_t magic[] = { 'T', 'C', 'S', 'O',
        0x00, 0x04, 0x00, 0x00, 0x00, 0x00 };

    // Magic SharedObject bytes.
    buf.append(magic, arraySize(magic)); 

    // SharedObject name
    const boost::uint16_t len = name.length();
    buf.appendNetworkShort(len);
    buf.append(name.c_str(), len);

    // Padding
    const boost::uint8_t padding[] = { 0, 0, 0, 0 };
    buf.append(padding, arraySize(padding));
    
    // see http://osflash.org/documentation/amf/envelopes/sharedobject
    // Do not encode strict arrays!
    amf::Writer w(buf, false);
    VM& vm = getVM(data);

    SOLPropsBufSerializer props(w, vm);

    // Visit all existing properties.
    data.visitProperties<Exists>(props);

    if (!props.success()) {
        // There are good reasons for this to fail, so it's not an error. Real
        // errors are logged during serialization.
        log_debug("Did not serialize object");
        return false;
    }
    return true;
}

} // anonymous namespace
} // end of gnash namespace
