// LocalConnection.cpp:  Connect two SWF movies & send objects, for Gnash.
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

#include "LocalConnection_as.h"

#include <cerrno>
#include <cstring>
#include <cstdint> // for boost::?int??_t
#include <functional>

#include "GnashSystemIOHeaders.h"

#include "VM.h"
#include "movie_root.h"
#include "URL.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h"
#include "SharedMem.h"
#include "namedStrings.h"
#include "StringPredicates.h"
#include "as_value.h"
#include "AMFConverter.h"
#include "ClockTime.h"
#include "GnashAlgorithm.h"
#include "RunResources.h"
#include "StreamProvider.h"


/// From observing the behaviour of the pp, the following seem to be true.
//
/// (Behaviour may be different on other platforms).
//
/// Sending
///
/// A sender checks the timestamp value. If it is zero, the data may be
/// overwritten. If it is not, we check whether we can delete it.
///
/// Data expires after 4 seconds. Processes only delete data they (think they)
/// wrote. If the timestamp matches the timestamp of the last data we sent,
/// we assume it's our data. If it's expired, mark it for overwriting and
/// continue.
///
/// We continue to check whether the data has expired as long as the data
/// is not marked for overwriting. No changes are made to the buffer by a
/// sender as long as the timestamp is there.
///
/// Once the buffer is ready for writing, check if the correct listener is
/// present. If it is, send the first message in our queue and store its
/// timestamp. If the listener is not present, go through the queue until
/// a message for an existing listener is found. If none is found, the
/// last message in the buffer is sent with no timestamp. (It's not clear if
/// there's any point in sending it, but it happens).
//
/// Receiving
///
/// A listener registers itself by adding its name to the listeners section
/// of the shared memory. The name is null-terminated and followed by a
/// further marker, which is of the form "::x\0::y\0". The x and y characters
/// are always numbers, e.g. ::3::4, ::3::1, ::3::2. We do not know the
/// significance of these numbers.
//
/// A listener merely checks whether the data has a timestamp and if the
/// data is intended for it (by reading the first string field). If it is,
/// the data is deserialized, the encoded function called, and the data
/// marked for deletion.
//
/// Functions are encoded in a particular order after the timestamp and length
/// fields:
///     1. connection name (domain:connection) [string]
///     2. domain [string]
/// {
///     3. The following optional data:
///         [boolean] (always false?)
///         [boolean] (always false?)
///         [number] (e.g. 0, 1)
///         [number] (e.g. 8, 6)
///     4. Sometimes the filename [string]. The presence of this may depend
///        on the first number.
/// }
///     5. The name of the function to call.
///     6. The arguments in reverse(!) order.   
//
/// Notes
/// 1. We don't know what happens when data from another process is left in
///    the buffer with a timestamp. Does it ever get overwritten?
/// 2. The timestamp seems to be allocated when LocalConnection.send is called,
///    even though the message may be sent much later.
/// 3. We can probably stop checking the data if (a) we have nothing more to
///    send, (b) we are not connected, and (c) the last data was not written
///    by us. Gnash doesn't do the additional check for (c), so will never
///    remove the advance callback if data with a timestamp from another
///    process stays in the buffer. Note 1 also relates to this.
//
/// http://www.osflash.org/localconnection
///
/// Some facts:
///     * The header is 16 bytes,
///     * The message can be up to 40k,
///     * The listeners block starts at 40k+16 = 40976 bytes,

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

    std::string getDomain(as_object& o);

    void removeListener(const std::string& name, SharedMem& mem);
    bool addListener(const std::string& name, SharedMem& mem);
    bool findListener(const std::string& name, SharedMem& mem);
    void getMarker(SharedMem::iterator& i, SharedMem::iterator end);
    void markRead(SharedMem& m);
    inline std::uint32_t getTimestamp(const VM& vm);

    /// Read the AMF data and invoke the function.
    void executeAMFFunction(as_object& owner, amf::Reader& rd);

    struct ConnectionData
    {
        std::string name;
        std::uint32_t ts;
        SimpleBuffer data; 
    };
}


void
writeLong(std::uint8_t*& ptr, std::uint32_t i)
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

inline std::uint32_t
readLong(const std::uint8_t* buf) {
	std::uint32_t s = buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24;
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
    //
    /// @param owner    The as_object that owns this Relay.
    LocalConnection_as(as_object* owner);
    
    virtual ~LocalConnection_as() {}

    /// Remove ourself as a listener (if connected).
    void close();

    const std::string& domain() {
        return _domain;
    }

    /// Called on advance().
    //
    /// Handles sending and receiving.
    virtual void update();

    bool connected() const {
        return _connected;
    }

    void connect(const std::string& name);

    void send(std::unique_ptr<ConnectionData> d)
    {
        assert(d.get());
        VM& vm = getVM(owner());
        d->ts = getTimestamp(vm);
        _queue.push_back(std::move(d));
        
        // Register callback so we can send the data on the next advance.
        movie_root& mr = getRoot(owner());
        mr.addAdvanceCallback(this);
    }

private:
    
    std::string _name;

    // The immutable domain of this LocalConnection_as, based on the 
    // originating SWF's domain.
    const std::string _domain;

    bool _connected;

    SharedMem _shm;

    std::deque<std::unique_ptr<ConnectionData>> _queue;

    // The timestamp of our last write to the shared memory.
    std::uint32_t _lastTime;

};

const size_t LocalConnection_as::listenersOffset;
const size_t LocalConnection_as::defaultSize;

LocalConnection_as::LocalConnection_as(as_object* o)
    :
    ActiveRelay(o),
    _domain(getDomain(owner())),
    _connected(false),
    _shm(defaultSize),
    _lastTime(0)
{
}

void
LocalConnection_as::update()
{
    // Check whether local connection is disabled(!): brilliant choice of
    // function name.
    if (rcfile.getLocalConnection()) {
        log_security(_("Attempting to write to disabled LocalConnection!"));
        movie_root& mr = getRoot(owner());
        mr.removeAdvanceCallback(this);
        return;
    }

    // No-op if already attached. Nothing to do if it fails, but we
    // should probably stop trying.
    if (!_shm.attach()) {
        log_error(_("Failed to attach shared memory segment"));
        return;
    }

    // We need the lock to prevent simultaneous reads/writes from other
    // processes.
    SharedMem::Lock lock(_shm);
    if (!lock.locked()) {
        log_error(_("Failed to get shm lock"));
        return;
    }

    SharedMem::iterator ptr = _shm.begin();
    
    // First check timestamp data.

    // These are not network byte order by default, but not sure about 
    // host byte order.
    const std::uint32_t timestamp = readLong(ptr + 8);
    const std::uint32_t size = readLong(ptr + 12);

    // As long as there is a timestamp in the shared memory, we mustn't
    // write anything.
    //
    // We check if this is data we are listening for. If it is, read it and
    // mark for overwriting.
    //
    // If not, we keep checking until the data has been overwritten by
    // another listener or until it's expired. If it's expired, we
    // mark for overwriting.
    if (timestamp) {

        // Start after 16-byte header.
        const std::uint8_t* b = ptr + 16;

        // End at reported size of AMF sequence.
        const std::uint8_t* end = b + size;

        amf::Reader rd(b, end, getGlobal(owner()));
        as_value a;

        // Get the connection name. That's all we need to remove expired
        // data.
        if (!rd(a)) {
            log_error(_("Invalid connection name data"));
            return;
        }
        const std::string& connection = a.to_string();

        // Now check if data we wrote has expired. There's no really
        // reliable way of checking that we genuinely wrote it.
        if (_lastTime == timestamp) {
            
            const size_t timeout = 4 * 1000;

            VM& vm = getVM(owner());
            const std::uint32_t timeNow = getTimestamp(vm);

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
    std::unique_ptr<ConnectionData> cd = std::move(_queue.front());
    _queue.pop_front();

    // If the correct listener isn't there, iterate until we find one or
    // there aren't any left.
    while (!findListener(_domain + ":" + cd->name, _shm)) {
        if (_queue.empty()) {
            // Make sure we send the empty header later.
            cd->ts = 0;
            break;
        }
        cd = std::move(_queue.front());
        _queue.pop_front();
    }

    // Yes, there is data to send.
    const char i[] = { 1, 0, 0, 0, 1, 0, 0, 0 };
    std::copy(i, i + arraySize(i), ptr);

    SimpleBuffer& buf = cd->data;

    SharedMem::iterator tmp = ptr + 8;
    writeLong(tmp, cd->ts);
    writeLong(tmp, cd->ts ? buf.size() : 0);
    std::copy(buf.data(), buf.data() + buf.size(), tmp);

    // Note the timestamp of our last send. We will keep calling update()
    // until the data has expired or been read.
    _lastTime = cd->ts;

}

/// Closes the LocalConnection object.
//
/// This removes the advanceCallback (so we can be removed by the GC) and
/// removes this object as a listener from the shared memory listeners
/// section.
void
LocalConnection_as::close()
{
    // We may be waiting either to send or to receive, so in both cases
    // make sure update() isn't called again.
    movie_root& mr = getRoot(owner());
    mr.removeAdvanceCallback(this);
    
    if (!_connected) return;
    _connected = false;
    
    SharedMem::Lock lock(_shm);
    if (!lock.locked()) {
        log_error(_("Failed to get lock on shared memory! Will not remove "
                    "listener"));
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
        log_error(_("Failed to open shared memory segment"));
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

void
localconnection_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, localconnection_new,
            attachLocalConnectionInterface, nullptr, uri);
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
    
    std::unique_ptr<ConnectionData> cd(new ConnectionData());

    SimpleBuffer& buf = cd->data;

    // Don't know whether strict arrays are allowed
    amf::Writer w(buf, false);
    const std::string& domain = relay->domain();
    
    w.writeString(domain + ":" + name);
    w.writeString(domain);
    w.writeString(func);

    for (size_t i = fn.nargs - 1; i > 1; --i) {
        fn.arg(i).writeAMF0(w);
    }

    // Now we have a valid call.

    cd->name = name;

    relay->send(std::move(cd));

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

    static const ReservedNames reserved = {
        "send",
        "onStatus",
        "close",
        "connect",
        "domain",
        "allowDomain"};

    const ReservedNames::const_iterator it =
        std::find_if(reserved.begin(), reserved.end(),
                std::bind(StringNoCaseEqual(), std::placeholders::_1, func));
        
    return (it == reserved.end());
}

// When a listener is removed, subsequent listeners are copied to the
// beginning. The byte after the marker is overwritten. If no listeners
// are left, the first byte becomes 0.
void
removeListener(const std::string& name, SharedMem& mem)
{
    assert(attached(mem));

    SharedMem::iterator ptr = mem.begin() + LocalConnection_as::listenersOffset;

    // No listeners if the first byte is 0.
    if (!*ptr) return;

    SharedMem::iterator found = nullptr;

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

    // No listeners at all.
    if (!*ptr) return false;

    while ((next = std::find(ptr, mem.end(), '\0')) != mem.end()) {

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
        // There are no listeners.
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
            log_error(_("No space for listener in shared memory!"));
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
    if (*(i + 7) != '\0') return;

    i += 8;
    return;

}

/// Read the function data, call the function.
//
/// This function does not mark the data for overwriting.
void
executeAMFFunction(as_object& o, amf::Reader& rd)
{
    as_value a;

    if (!rd(a) || !a.is_string()) {
        log_error(_("Invalid domain %s"), a);
        return;
    }
    const std::string& domain = a.to_string();
    log_debug("Domain: %s", domain);
    
    if (!rd(a)) {
        log_error(_("Invalid function name %s"), a);
        return;
    }

    // This is messy and verbose because we don't know what it means.
    // If the value after the domain is a boolean, it appears to signify a
    // set of extra data. It's logged so that we can find exceptions more
    // easily.
    if (a.is_bool()) {

        // Both bools have been false in all the examples I've seen.
        log_debug("First bool: %s", a);
        if (rd(a)) log_debug("Second Bool: %s", a);

        // We guess that the first number describes the number of data fields
        // after the second number, before the function name.
        if (rd(a)) log_debug("First Number: %s", a);

        // Handle negative numbers.
        const size_t count = std::max<int>(0, toInt(a, getVM(o)));

        // We don't know what the second number signifies.
        if (rd(a)) log_debug("Second Number: %s", a);

        for (size_t i = 0; i < count; ++i) {
            if (!rd(a)) {
                log_error(_("Fewer AMF fields than expected."));
                return;
            }
            log_debug("Data: %s", a);
        }

        // Now we expect the next field to be the method to call.
        if (!rd(a)) return;
    }

    const std::string& meth = a.to_string();

    // These are in reverse order!
    std::vector<as_value> d;
    while(rd(a)) d.push_back(a);
    std::reverse(d.begin(), d.end());
    fn_call::Args args;
    args.swap(d);

    // Call the method on this LocalConnection object.
    VM& vm = getVM(o);
    as_function* f = getMember(o, getURI(vm, meth)).to_function();

    invoke(f, as_environment(getVM(o)), &o, args);
}

/// Zero timestamp and length bytes to mark the data as overwritable.
void
markRead(SharedMem& m)
{
    std::fill_n(m.begin() + 8, 8, 0);
}
    
/// Return a number usable as a timestamp.
//
/// Different players use different values here. The Linux players use:
/// Version 9: the time since player startup
/// Version 10: the system uptime.
//
/// Version 10 fails if it recieves a value outside the signed 32-bit int
/// range, so we surmise that there is an undocumented conversion to signed
/// in that player. We make sure the value never exceeds 0x7fffffff.
inline std::uint32_t
getTimestamp(const VM& vm)
{
    return vm.getTime() & 0x7fffffff;
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
getDomain(as_object& o)
{
    
    const URL& url = getRunResources(o).streamProvider().baseURL();

    if (url.hostname().empty()) {
        return "localhost";
    }

    // Adjust the name based on the swf version. Prior to v7, the nodename part
    // was removed. For v7 or later. the full hostname is returned. The
    // localhost is always just the localhost.
    if (getSWFVersion(o) > 6) {
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

} // anonymous namespace

} // end of gnash namespace

