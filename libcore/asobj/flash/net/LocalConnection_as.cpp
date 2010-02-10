// LocalConnection.cpp:  Connect two SWF movies & send objects, for Gnash.
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

#include "GnashSystemIOHeaders.h"

#include "VM.h"
#include "movie_root.h"
#include "URLAccessManager.h"
#include "URL.h"
#include "log.h"
#include "LocalConnection_as.h"
#include "fn_call.h"
#include "Global_as.h"
#include "builtin_function.h"
#include "NativeFunction.h"
#include "SharedMem.h"
#include "namedStrings.h"
#include "StringPredicates.h"
#include "as_value.h"
#include "AMF.h"
#include "ClockTime.h"
#include "GnashAlgorithm.h"

#include <boost/shared_ptr.hpp>
#include <cerrno>
#include <cstring>
#include <boost/cstdint.hpp> // for boost::?int??_t
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>

/// http://www.osflash.org/localconnection
///
/// Listening
/// To create a listening LocalConnection, you just have to set a thread to:
///
///    1. register the application as a valid LocalConnection listener
///    2. require the mutex to have exclusive access to the shared memory
///         - Gnash currently doesn't use a mutex.
///    3. access the shared memory and check the recipient
///    4. if you are the recipient, read the message and mark it read
///         - in Gnash, the recipient overwrites the message when it has
///           been read. Not established whether this is correct.
///    5. release the shared memory and the mutex
///    6. repeat indefinitely from step 2.
///
/// Sending
/// To send a message to a LocalConnection apparently works like that:
///    1. require the mutex to have exclusive access to the shared memory
///         - Gnash currently has no mutex.
///    2. access the shared memory and check that the listener is connected
///         - It's not clear if the pp checks or not, though it
///           seems not to. Gnash does not.
///    3. if the recipient is registered, write the message
///    4. release the shared memory and the mutex.
//
/// The pp sends some messages without a timestamp and without a size. These
/// are ignored (it's also not clear why it does it).
//
/// Some facts:
///     * The header is 16 bytes,
///     * The message can be up to 40k,
///     * The listeners block starts at 40k+16 = 40976 bytes,
///     * To add a listener, simply append its name in the listeners list
///     (null terminated strings)

namespace {
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();    
} 

namespace gnash {

namespace {
    as_value localconnection_connect(const fn_call& fn);
    as_value localconnection_domain(const fn_call& fn);
    as_value localconnection_send(const fn_call& fn);
    as_value localconnection_new(const fn_call& fn);
    as_value localconnection_close(const fn_call& fn);

    bool validFunctionName(const std::string& func);
    void attachLocalConnectionInterface(as_object& o);

    void removeListener(const std::string& name, SharedMem& mem);
    bool addListener(const std::string& name, SharedMem& mem);
    bool findListener(const std::string& name, SharedMem& mem);
    void getMarker(SharedMem::iterator& i, SharedMem::iterator end);
    void markRead(SharedMem& m);

    /// Read the AMF data and invoke the function.
    void executeAMFFunction(as_object& owner, AMF::Reader& rd);

    struct ConnectionData
    {
        std::string name;
        std::string func;
        boost::uint32_t ts;
        SimpleBuffer data; 
    };
}


void
writeLong(boost::uint8_t*& ptr, boost::uint32_t i)
{
    *ptr = i & 0xff;
    ++ptr;
    *ptr = (i & 0xff00) >> 8;
    ++ptr;
    *ptr = (i & 0xff0000) >> 16;
    ++ptr;
    *ptr = (i & 0xff000000) >> 24;
    ++ptr;
}

inline boost::uint32_t
readLong(const boost::uint8_t* buf) {
	boost::uint32_t s = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
	return s;
}


/// A null byte after the marker or at the beginning of the listeners
/// signifies the end of the list.
template<typename T, size_t N>
std::string
fromByteString(T(&buf)[N])
{
	return std::string(buf, buf + N - 1);
}
    
static const std::string marker(fromByteString("\0::3\0::4\0"));



/// Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
class LocalConnection_as : public ActiveRelay
{

public:

    /// The size of the shared memory segment.
    static const size_t defaultSize = 64528;
    
    /// Offset of listeners in the shared memory segment.
    static const size_t listenersOffset = 40976;

    /// Create a LocalConnection_as object.
    LocalConnection_as(as_object* owner);
    
    virtual ~LocalConnection_as() {
        close();
    }

    /// Remove ourself as a listener (if connected).
    void close();

    const std::string& domain() {
        return _domain;
    }

    /// Called on advance().
    virtual void update();

    bool connected() const {
        return _connected;
    }

    void connect(const std::string& name);

    void send(boost::shared_ptr<ConnectionData> d)
    {
        assert(d.get());
        VM& vm = getVM(owner());
        const boost::uint32_t time = vm.getTime();
        d->ts = time;
        _queue.push_back(d);
        
        // Register callback so we can send the data on the next advance.
        movie_root& mr = getRoot(owner());
        mr.addAdvanceCallback(this);
    }

private:
    
    /// Work out the domain.
    //
    /// Called once on construction to set _domain, though it will do
    /// no harm to call it again.
    std::string getDomain();
    
    std::string _name;

    // The immutable domain of this LocalConnection_as, based on the 
    // originating SWF's domain.
    const std::string _domain;

    bool _connected;

    SharedMem _shm;

    std::deque<boost::shared_ptr<ConnectionData> > _queue;

    // The timestamp of our last write to the shared memory.
    boost::uint32_t _lastTime;

};

const size_t LocalConnection_as::listenersOffset;
const size_t LocalConnection_as::defaultSize;

LocalConnection_as::LocalConnection_as(as_object* owner)
    :
    ActiveRelay(owner),
    _domain(getDomain()),
    _connected(false),
    _shm(defaultSize),
    _lastTime(0)
{
}


/// From observing the behaviour of the pp, the following seem to be true.
//
/// (Behaviour may be different on other platforms).
//
/// Send:
///   Is timestamp there?
///     If yes, check value. If it's zero, proceed. If it's not older than 3
///     or 4 seconds, leave it alone and wait.
///     If it's older than 3 or 4 seconds, remove the listener from
///     listeners, zero timestamp.
///
///     Else: 
///       Check if the correct listener is present. If not, send most recent
///       data without header. Clear queue.
///       If correct listener is present, send first thing in buffer with
///       timestamp.
//
/// Receive:
///     If there is a timestamp, Check if data is for us. If it is, get it,
///     zero timestamp, call function.
void
LocalConnection_as::update()
{

    // No-op if already attached. Nothing to do if it fails, but we
    // should probably stop trying.
    if (!_shm.attach()) {
        log_error("Failed to attach shared memory segment");
        return;
    }

    // We need the lock to prevent simultaneous reads/writes from other
    // processes.
    SharedMem::Lock lock(_shm);
    if (!lock.locked()) {
        log_debug("Failed to get shm lock");
        return;
    }

    SharedMem::iterator ptr = _shm.begin();
    
    // First check timestamp data.

    // These are not network byte order by default, but not sure about 
    // host byte order.
    const boost::uint32_t timestamp = readLong(ptr + 8);

    const size_t size = readLong(ptr + 12);

    // If we are listening, we only care if there is a timestamp, and
    // then only if it's intended for us.
    //
    // If not, we want to remove this data if it has expired.
    if (timestamp) {

        // Start after 16-byte header.
        const boost::uint8_t* b = ptr + 16;

        // End at reported size of AMF sequence.
        const boost::uint8_t* end = b + size;


        AMF::Reader rd(b, end, getGlobal(owner()));
        as_value a;

        // Get the connection name. That's all we need to remove expired
        // data.
        if (!rd(a)) {
            log_error("Invalid connection name data");
            return;
        }
        const std::string& connection = a.to_string();

        // Now check if data we wrote has expired. There's no really
        // reliable way of checking that we genuinely wrote it.
        if (_lastTime == timestamp) {
            
            const size_t timeout = 4 * 1000;

            VM& vm = getVM(owner());
            const boost::uint32_t timeNow = vm.getTime();

            if (timeNow - timestamp > timeout) {
                log_debug("Data %s expired at %s. Removing its target "
                        "as a listener", timestamp, timeNow);
                removeListener(connection, _shm);
                markRead(_shm);
                _lastTime = 0;
            }
        }

        // If we are listening and the data is for us, get the rest of it
        // and call the method.
        if (_connected && connection == _domain + ":" + _name) {
            executeAMFFunction(owner(), rd);
            // Zero the timestamp bytes to signal that the shared memory
            // can be written again.
            markRead(_shm);
        }
        else {
            // The data has not expired and we didn't read it. Leave it
            // alone until it's expired or someone else has read it.
            return;
        }
    }

    // If we have no data to send, there's nothing more to do.
    if (_queue.empty()) {
        // ...except remove the callback if we aren't listening for anything.
        if (!_connected) {
            movie_root& mr = getRoot(owner());
            mr.removeAdvanceCallback(this);
        }
        return;
    }

    // Get the first buffer.
    boost::shared_ptr<ConnectionData> cd = _queue.front();
    _queue.pop_front();

    // If the correct listener isn't there, iterate until we find one or
    // there aren't any left.
    while (!findListener(_domain + ":" + cd->name, _shm)) {
        if (_queue.empty()) {
            // Make sure we send the empty header later.
            cd->ts = 0;
            break;
        }
        cd = _queue.front();
        _queue.pop_front();
    }

    // Yes
    const char i[] = { 1, 0, 0, 0, 1, 0, 0, 0 };
    std::copy(i, i + arraySize(i), ptr);

    SimpleBuffer& buf = cd->data;

    SharedMem::iterator tmp = ptr + 8;
    writeLong(tmp, cd->ts);
    writeLong(tmp, cd->ts ? buf.size() : 0);
    std::copy(buf.data(), buf.data() + buf.size(), tmp);

    _lastTime = cd->ts;
    return;

}

/// Closes the LocalConnection object.
//
/// This removes the advanceCallback (so we can be removed by the GC) and
/// removes this object as a listener from the shared memory listeners
/// section.
void
LocalConnection_as::close()
{
    if (!_connected) return;
    
    movie_root& mr = getRoot(owner());
    mr.removeAdvanceCallback(this);
    _connected = false;
    
    SharedMem::Lock lock(_shm);
    if (!lock.locked()) {
        log_error("Failed to get lock on shared memory! Will not remove "
                "listener");
        return;
    }

    removeListener(_domain + ":" + _name, _shm);
    
}

/// Makes the LocalConnection object listen.
/// 
/// The name is a symbolic name like "lc1", that is used by the
/// send() command to signify which local connection to send the
/// object to.
//
/// When connect is called, this object adds its domain + name plus some
/// other bits of information to the listeners portion of the shared memory.
/// It also sets the initial bytes of the shared memory to a set
/// pattern.
//
/// The connection will fail if a listener with the same id (domain + name)
/// already exists. ActionScript isn't informed of this failure.
void
LocalConnection_as::connect(const std::string& name)
{
    assert(!name.empty());

    _name = name;
    
    if (!_shm.attach()) {
        log_error("Failed to open shared memory segment");
        return;
    }

    SharedMem::iterator ptr = _shm.begin();

    // We can't connect if there is already a listener with the same name.
    if (!addListener(_domain + ":" + _name, _shm)) {
        return;
    }
        
    const char i[] = { 1, 0, 0, 0, 1, 0, 0, 0 };
    std::copy(i, i + 8, ptr);

    movie_root& mr = getRoot(owner());
    mr.addAdvanceCallback(this);

    _connected = true;
    
    return;
}

/// String representing the domain of the current SWF file.
//
/// This is set on construction, as it should be constant.
/// The domain is either the "localhost", or the hostname from the
/// network connection. This behaviour changed for SWF v7. Prior to v7
/// only the domain was returned, ie dropping off node names like
/// "www". As of v7, the behaviour is to return the full host
/// name. Gnash supports both behaviours based on the version.
std::string
LocalConnection_as::getDomain()
{
    
    URL url(getRoot(owner()).getOriginalURL());

    if (url.hostname().empty()) {
        return "localhost";
    }

    // Adjust the name based on the swf version. Prior to v7, the nodename part
    // was removed. For v7 or later. the full hostname is returned. The
    // localhost is always just the localhost.
    if (getSWFVersion(owner()) > 6) {
        return url.hostname();
    }

    const std::string& domain = url.hostname();

    std::string::size_type pos;
    pos = domain.rfind('.');

    // If there is no '.', return the whole thing.
    if (pos == std::string::npos) {
        return domain;
    }

    pos = domain.rfind(".", pos - 1);
    
    // If there is no second '.', return the whole thing.
    if (pos == std::string::npos) {
        return domain;
    }

    // Return everything after the second-to-last '.'
    // FIXME: this must be wrong, or it would return 'org.uk' for many
    // UK websites, and not even Adobe is that stupid. I think.
    return domain.substr(pos + 1);

}

void
localconnection_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, localconnection_new,
            attachLocalConnectionInterface, 0, uri);
}

void
registerLocalConnectionNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(localconnection_connect, 2200, 0);
    vm.registerNative(localconnection_send, 2200, 1);
    vm.registerNative(localconnection_close, 2200, 2);
    vm.registerNative(localconnection_domain, 2200, 3);
}


// Anonymous namespace for module-statics
namespace {

/// Instantiate a new LocalConnection object within a flash movie
as_value
localconnection_new(const fn_call& fn)
{
    // TODO: this doesn't happen on construction.
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new LocalConnection_as(obj));
    return as_value();
}

/// The callback for LocalConnection::close()
as_value
localconnection_close(const fn_call& fn)
{
    LocalConnection_as* relay = ensure<ThisIsNative<LocalConnection_as> >(fn);
    relay->close();
    return as_value();
}

/// The callback for LocalConnectiono::connect()
as_value
localconnection_connect(const fn_call& fn)
{
    LocalConnection_as* relay = ensure<ThisIsNative<LocalConnection_as> >(fn);

    // If already connected, don't try again until close() is called.
    if (relay->connected()) return false;

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("LocalConnection.connect() expects exactly "
                    "1 argument"));
        );
        return as_value(false);
    }

    if (!fn.arg(0).is_string()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("LocalConnection.connect(): first argument must "
                    "be a string"));
        );
        return as_value(false);
    }
    
    if (fn.arg(0).to_string().empty()) {
        return as_value(false);
    }

    std::string connection = fn.arg(0).to_string();
   
    relay->connect(connection);

    // We don't care whether connected or not.
    return as_value(true);
}

/// The callback for LocalConnection::domain()
as_value
localconnection_domain(const fn_call& fn)
{
    LocalConnection_as* relay = ensure<ThisIsNative<LocalConnection_as> >(fn);
    return as_value(relay->domain());
}

/// LocalConnection.send()
//
/// Returns false only if the call was syntactically incorrect.
///
/// The pp only ever seems have one send sequence (at least in the first 512
/// bytes). Subsequent sends overwrite any sequence in shared memory.
//
/// The pp sometimes sends calls with no timestamp and no length. These
/// appear to be ignored, so it's not clear what the point is.
as_value
localconnection_send(const fn_call& fn)
{
    LocalConnection_as* relay = ensure<ThisIsNative<LocalConnection_as> >(fn);

    // At least 2 args (connection name, function) required.
    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("LocalConnection.send(%s): requires at least 2 "
                    "arguments"), os.str());
        );
        return as_value(false);
        
    }

    // Both the first two arguments must be a string
    if (!fn.arg(0).is_string() || !fn.arg(1).is_string()) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("LocalConnection.send(%s): requires at least 2 "
                    "arguments"), os.str());
        );
        return as_value(false);
    }
    
    const std::string& name = fn.arg(0).to_string();
    const std::string& func = fn.arg(1).to_string();

    if (!validFunctionName(func)) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("LocalConnection.send(%s): requires at least 2 "
                    "arguments"), os.str());
        );
        return as_value(false);
    }
    
    boost::shared_ptr<ConnectionData> cd(new ConnectionData());

    SimpleBuffer& buf = cd->data;

    // Don't know whether strict arrays are allowed
    AMF::Writer w(buf, false);
    const std::string uri(relay->domain() + ":" + name);
    w.writeString(uri);
    w.writeString("localhost");
    w.writeString(func);

    for (size_t i = fn.nargs - 1; i > 1; --i) {
        fn.arg(i).writeAMF0(w);
    }

    // Now we have a valid call.

    // We'll return true if the LocalConnection is disabled too, as
    // the return value doesn't indicate success of the connection.
    if (rcfile.getLocalConnection()) {
        log_security("Attempting to write to disabled LocalConnection!");
        return as_value(true);
    }

    cd->name = name;
    cd->func = func;

    relay->send(cd);

    return as_value(true);
}


void
attachLocalConnectionInterface(as_object& o)
{
    VM& vm = getVM(o);
    o.init_member("connect", vm.getNative(2200, 0));
    o.init_member("send", vm.getNative(2200, 1));
    o.init_member("close", vm.getNative(2200, 2));
    o.init_member("domain", vm.getNative(2200, 3));
}

/// These names are invalid as a function name.
bool
validFunctionName(const std::string& func)
{

    if (func.empty()) return false;

    typedef std::vector<std::string> ReservedNames;

    static const ReservedNames reserved = boost::assign::list_of
        ("send")
        ("onStatus")
        ("close")
        ("connect")
        ("domain")
        ("allowDomain");

    const ReservedNames::const_iterator it =
        std::find_if(reserved.begin(), reserved.end(),
                boost::bind(StringNoCaseEqual(), _1, func));
        
    return (it == reserved.end());
}

// When a listener is removed, subsequent listeners are copied to the
// beginning. The byte after the marker is overwritten. If no listeners
// are left, the first byte becomes 0.
void
removeListener(const std::string& name, SharedMem& mem)
{
    assert(attached(mem));

    log_debug("Removing listener %s", name);

    SharedMem::iterator ptr = mem.begin() + LocalConnection_as::listenersOffset;

    // No listeners if the first byte is 0.
    if (!*ptr) return;

    SharedMem::iterator found = 0;

    SharedMem::iterator next;
    
    // Next should always point to the beginning of a listener.
    while ((next = std::find(ptr, mem.end(), '\0')) != mem.end()) {

        // Move next to where it should be (beginning of next string).
        getMarker(next, mem.end());

        // Check whether we've found the string (should only be once).
        if (std::equal(name.c_str(), name.c_str() + name.size(), ptr)) {
            found = ptr;
        }

        // Found last listener (or reached the end).
        if (next == mem.end() || !*next) {

            if (!found) return;

            // Name and marker.
            const ptrdiff_t size = name.size() + marker.size();

            // Copy listeners backwards to fill in the gaps.
            std::copy(found + size, next, found);

            // Add a nul terminator.
            *(next -size) = '\0';
            
            return;
        }

        ptr = next;
    }
    

}

/// Two listeners with the same name are never added.
bool
findListener(const std::string& name, SharedMem& mem)
{
    assert(attached(mem));

    SharedMem::iterator ptr = mem.begin() + LocalConnection_as::listenersOffset;

    SharedMem::iterator next;

    if (!*ptr) return false;
    while ((next = std::find(ptr, mem.end(), '\0' != mem.end()))) {

        if (std::equal(name.c_str(), name.c_str() + name.size(), ptr)) {
            return true;
        }

        getMarker(next, mem.end());

        // Found last listener.
        if (!*next) return false;
        ptr = next;
    }
    return false;
}

/// Two listeners with the same name are never added.
bool
addListener(const std::string& name, SharedMem& mem)
{
    assert(attached(mem));

    SharedMem::iterator ptr = mem.begin() + LocalConnection_as::listenersOffset;

    SharedMem::iterator next;

    if (!*ptr) {
        next = ptr;
    }
    else {
        while ((next = std::find(ptr, mem.end(), '\0')) != mem.end()) {

            getMarker(next, mem.end());
            
            if (std::equal(name.c_str(), name.c_str() + name.size(), ptr)) {
                log_debug("Not adding duplicated listener");
                return false;
            }

            // Found last listener.
            if (!*next) break;
            ptr = next;
        }
        if (next == mem.end()) {
            log_error("No space for listener in shared memory!");
            return false;
        }
    }

    // Copy name and marker to listeners section.
    std::string id(name + marker);
    std::copy(id.c_str(), id.c_str() + id.size(), next);

    // Always add an extra null after the final listener.
    *(next + id.size()) = '\0';

    return true;
}

/// Check whether there is a marker after the listener name and skip it.
//
/// @param i        Always moved to point to the next listener string.
/// @param end      The end of the shared memory second to read.
//
/// A marker looks like this "::3\0::4\0" or "::3\0::2\0". We don't know
/// what the numbers mean, or which ones are valid.
//
/// Currently this check ignores the digits.
void
getMarker(SharedMem::iterator& i, SharedMem::iterator end)
{
    // i points to 0 before marker.
    assert(*i == '\0');
    if (i == end) return;

    // Move to after null.
    ++i;

    // Then check for marker.
    if (end - i < 8) return;

    const char m[] = "::";

    // Check for "::" patterns.
    if (!std::equal(i, i + 2, m) || !std::equal(i + 4, i + 6, m)) {
        return;
    }

    // Check for terminating 0.
    if (*(i + 8) != '\0') {
        return;
    }

    i += 8;
    return;

}

void
executeAMFFunction(as_object& o, AMF::Reader& rd)
{
    as_value a;

    if (!rd(a)) {
        log_error("Invalid protocol");
        return;
    }
    log_debug("Protocol: %s", a);
    
    if (!rd(a)) {
        log_error("Invalid function name");
        return;
    }

    // If the value after the protocol is a boolean, there is 
    // a set of extra data. We don't know what it's for,
    // so log it.
    if (a.is_bool()) {
        if (rd(a)) log_debug("Bool: %s", a);
        if (rd(a)) log_debug("Number: %s", a);
        if (rd(a)) log_debug("Number: %s", a);
        if (rd(a)) log_debug("Filename: %s", a);
        if (!rd(a)) return;
    }

    // The name of the function to call.
    log_debug("Method: %s", a);
    const std::string& meth = a.to_string();

    // These are in reverse order!
    std::vector<as_value> d;
    while(rd(a)) d.push_back(a);
    std::reverse(d.begin(), d.end());
    fn_call::Args args;
    args.swap(d);

    // Call the method on this LocalConnection object.
    string_table& st = getStringTable(o);
    as_function* f = o.getMember(st.find(meth)).to_function();

    invoke(f, as_environment(getVM(o)), &o, args);
}

/// Zero timestamp and length bytes to mark the data as overwritable.
void
markRead(SharedMem& m)
{
    std::fill_n(m.begin() + 8, 8, 0);
}
} // anonymous namespace

} // end of gnash namespace

