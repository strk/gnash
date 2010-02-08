// LocalConnection.cpp:  Connect two SWF movies & send objects, for Gnash.
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
//


#include "GnashSystemIOHeaders.h"

#include "VM.h"
#include "movie_root.h"
#include "URLAccessManager.h"
#include "URL.h"
#include "log.h"
#include "net/LocalConnection_as.h"
#include "network.h"
#include "fn_call.h"
#include "Global_as.h"
#include "builtin_function.h"
#include "NativeFunction.h"
#include "shm.h"
#include "namedStrings.h"
#include "StringPredicates.h"
#include "as_value.h"
#include "amf.h"
#include "ClockTime.h"

#include <cerrno>
#include <cstring>
#include <boost/cstdint.hpp> // for boost::?int??_t
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
using namespace amf;

// http://www.osflash.org/localconnection
//
// Listening
// To create a listening LocalConnection, you just have to set a thread to:
//
//    1.register the application as a valid LocalConnection listener,
//    2.require the mutex to have exclusive access to the shared memory,
//    3.access the shared memory and check the recipient,
//    4.if you are the recipient, read the message and mark it read,
//    5.release the shared memory and the mutex,
//    6.repeat indefinitely from step 2.
//
// Sending
// To send a message to a LocalConnection apparently works like that:
//    1. require the mutex to have exclusive access to the shared memory,
//    2. access the shared memory and check that the listener is connected,
//    3. if the recipient is registered, write the message,
//    4. release the shared memory and the mutex.
//
// The main thing you have to care about is the timestamp - simply call GetTickCount()
//  and the message size. If your message is correctly encoded, it should be received
// by the listening LocalConnection 
//
// Some facts:
//     * The header is 16 bytes,
//     * The message can be up to 40k,
//     * The listeners block starts at 40k+16 = 40976 bytes,
//     * To add a listener, simply append its name in the listeners list (null terminated strings)
//
namespace {

gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();    

} // anonymous namespace

namespace gnash {

namespace {
    as_value localconnection_connect(const fn_call& fn);
    as_value localconnection_domain(const fn_call& fn);
    as_value localconnection_send(const fn_call& fn);
    as_value localconnection_new(const fn_call& fn);
    as_value localconnection_close(const fn_call& fn);

    bool validFunctionName(const std::string& func);

    void attachLocalConnectionInterface(as_object& o);
    as_object* getLocalConnectionInterface();
}

void
writeLong(char*& ptr, boost::uint32_t i)
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

bool
findListener(const std::string& name, const char* shm, const char* end)
{
    // Listeners offset
    const size_t pos = 40976;
    assert(end - shm > static_cast<int>(pos));
    const char* ptr = shm + pos;

    const char* next;

    while ((next = std::find(ptr, end, 0)) != end) {

        // End of listeners.
        if (next == ptr) break;

        if (std::equal(name.c_str(), name.c_str() + name.size(), ptr)) {
            return true;
        }

        ptr = next + 1;
    }

    return false;

}

void
dumpListeners(const char* shm, const char* end)
{
    // Listeners offset
    const size_t pos = 40976;
    assert(end - shm > static_cast<int>(pos));
    const char* ptr = shm + pos;

    const char* next;

    while ((next = std::find(ptr, end, 0)) != end) {

        // End of listeners.
        if (next == ptr) break;
        
        log_debug("Listener: %s", std::string(ptr, next));

        ptr = next + 1;
    }

}

void
removeListener(const std::string& name, char* shm, char* end)
{
    const size_t pos = 40976;
    assert(end - shm > static_cast<int>(pos));
    char* ptr = shm + pos;

    char* next;

    char* found = 0;

    while ((next = std::find(ptr, end, 0)) != end) {

        // End of listeners.
        if (next == ptr) {
            if (!found) return;

            // Name and null terminator.
            const ptrdiff_t size = name.size() + 1;

            // Copy listeners backwards to fill in the gaps.
            std::copy(found + size, ptr, found);

            // Overwrite  removed string.
            std::fill_n(ptr - size, size, '\0');
            
            return;
        }

        if (std::equal(name.c_str(), name.c_str() + name.size(), ptr)) {
            found = ptr;
        }

        ptr = next + 1;
    }
    

}

void
addListener(const std::string& list, char* shm, char* end)
{

    // Listeners offset
    const size_t pos = 40976;
    assert(end - shm > static_cast<int>(pos));
    char* ptr = shm + pos;

    char* next;

    while ((next = std::find(ptr, end, 0)) != end) {

        // End of listeners.
        if (next == ptr) {
            log_debug("Adding us");
            std::copy(list.c_str(), list.c_str() + list.size() + 1, next);
            return;
        }

        ptr = next + 1;
    }

}

/// Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
class LocalConnection_as : public ActiveRelay
{

public:

    LocalConnection_as(as_object* owner);
    
    virtual ~LocalConnection_as() {
        close();
    }

    /// Remove ourself as a listener (if connected).
    void close();

    const std::string& domain() {
        return _domain;
    }

    const std::string& name() { return _name; };

    /// Called on advance().
    virtual void update();

    bool connected() const {
        return _connected;
    }

    void connect(const std::string& name);

    void send(const std::string& name, const std::string& func,
            const SimpleBuffer& buf)
    {
        char i[] = { 1, 0, 0, 0, 1, 0, 0, 0 };
        _shm.attach(0, false);
        const size_t headerSize = 16;

        char* ptr = _shm.getAddr();
        
        const std::string uri(_domain + ":" + name);

        if (!findListener(uri, ptr, ptr + _shm.getSize())) {
            dumpListeners(ptr, ptr + _shm.getSize());
            log_error("Listener not added!");
        }
        
        std::map<as_object*, size_t> offsets;
        SimpleBuffer b;
        as_value n(uri), f(func), p("localhost");
        n.writeAMF0(b, offsets, getVM(owner()), false);
        p.writeAMF0(b, offsets, getVM(owner()), false);
        f.writeAMF0(b, offsets, getVM(owner()), false);

        assert(_shm.getSize() > headerSize + b.size() + buf.size());
        
        std::copy(i, i + sizeof(i), ptr);
        ptr += 8;
        boost::uint32_t time = clocktime::getTicks();
        writeLong(ptr, time);
        writeLong(ptr, buf.size() + b.size());

        if (!_shm.getAddr()) return;
        std::copy(b.data(), b.data() + b.size(), ptr);
        std::copy(buf.data(), buf.data() + buf.size(), ptr + b.size());
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

    Shm _shm;

};

LocalConnection_as::LocalConnection_as(as_object* owner)
    :
    ActiveRelay(owner),
    _domain(getDomain()),
    _connected(false)
{
    log_debug("The domain for this host is: %s", _domain);
}


/// From observing the behaviour of the pp, the following seem to be true.
//
/// (Behaviour may be different on other platforms).
//
/// If there is no timestamp the sent sequence is ignored.
void
LocalConnection_as::update()
{
    assert(_connected);

    std::vector<as_object*> refs;

    // These are not network byte order by default, but not sure about 
    // host byte order.
    const boost::uint32_t timestamp = readLong(
            reinterpret_cast<boost::uint8_t*>(_shm.getAddr() + 8));

    const size_t size = readLong(
            reinterpret_cast<boost::uint8_t*>(_shm.getAddr() + 12));


    // This seems to be quite normal, so don't log.
    if (!timestamp || !size) return;
    
#if 1
    std::string s(_shm.getAddr(), _shm.getAddr() + 512);
    const boost::uint8_t* sptr = reinterpret_cast<const boost::uint8_t*>(s.c_str());
    log_debug("%s \n %s", hexify(sptr, s.size(), false),
            hexify(sptr, s.size(), true));
#endif


    // Start after 16-byte header.
    const boost::uint8_t* b = reinterpret_cast<boost::uint8_t*>(_shm.getAddr()
            + 16);

    // End at reported size of AMF sequence.
    const boost::uint8_t* end = reinterpret_cast<const boost::uint8_t*>(b +
            size);
    
    log_debug("Timestamp: %s, size: %s", timestamp, size);

    as_value a;

    // Connection. If this is not for us, we don't need to do anything.
    a.readAMF0(b, end, -1, refs, getVM(owner()));
    const std::string& connection = a.to_string();
    if (connection != _name) return;
    
    // Protocol
    a.readAMF0(b, end, -1, refs, getVM(owner()));
    log_debug("Protocol: %s", a);
    
    // The name of the function to call.
    a.readAMF0(b, end, -1, refs, getVM(owner()));
    log_debug("Method: %s", a);
    const std::string& meth = a.to_string();

    // These are in reverse order!
    std::vector<as_value> d;
    while(a.readAMF0(b, end, -1, refs, getVM(owner()))) {
        d.push_back(a);
    }
  
    std::reverse(d.begin(), d.end());

    fn_call::Args args;
    args.swap(d);

    // Zero the bytes read (though it's not clear if this is necessary).
    std::fill(_shm.getAddr(), _shm.getAddr() + size + 16, '\0');

    log_debug("Meth: %s", meth);

    // Call the method on this LocalConnection object.
    string_table& st = getStringTable(owner());
    as_function* f = owner().getMember(st.find(meth)).to_function();

    invoke(f, as_environment(getVM(owner())), &owner(), args);

}

/// \brief Closes (disconnects) the LocalConnection object.
void
LocalConnection_as::close()
{
    if (!_connected) return;

    movie_root& mr = getRoot(owner());
    removeListener(_name, _shm.getAddr(), _shm.getAddr() + _shm.getSize());
    mr.removeAdvanceCallback(this);
    _connected = false;
}

/// \brief Prepares the LocalConnection object to receive commands from a
/// LocalConnection.send() command.
/// 
/// The name is a symbolic name like "lc_name", that is used by the
/// send() command to signify which local connection to send the
/// object to.
void
LocalConnection_as::connect(const std::string& name)
{

    assert(!name.empty());

    _name = name;
    
    if (!_shm.attach(0, true)) {
        return;
    }

    char* ptr = _shm.getAddr();

    if (!ptr) {
        log_error("Failed to open shared memory segment: \"%s\"", _name);
        return; 
    }

    addListener(_name, ptr, ptr + _shm.getSize());
    dumpListeners(ptr, ptr + _shm.getSize());

    movie_root& mr = getRoot(owner());
    mr.addAdvanceCallback(this);

    _connected = true;
    
    return;
}

/// \brief Returns a string representing the superdomain of the
/// location of the current SWF file.
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

    std::string connection_name = relay->domain();    
    connection_name +=":";
    connection_name += fn.arg(0).to_string();
   
    relay->connect(connection_name);

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

    std::map<as_object*, size_t> offsets;
    SimpleBuffer buf;
    
    for (size_t i = fn.nargs - 1; i > 1; --i) {
        fn.arg(i).writeAMF0(buf, offsets, getVM(fn), false);
    }

    // Now we have a valid call.

    // It is useful to see what's supposed being sent, so we log
    // this every time until implemented.
    std::ostringstream os;
    fn.dump_args(os);
    log_unimpl(_("LocalConnection.send unimplemented %s"), os.str());

    // We'll return true if the LocalConnection is disabled too, as
    // the return value doesn't indicate success of the connection.
    if (rcfile.getLocalConnection() ) {
        log_security("Attempting to write to disabled LocalConnection!");
        return as_value(true);
    }

    relay->send(name, func, buf);

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

} // anonymous namespace

} // end of gnash namespace

//Adding for testing commit
