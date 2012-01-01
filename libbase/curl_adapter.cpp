// NetworkAdapter.cpp:  Interface to libcurl to read HTTP streams, for Gnash.
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

#ifdef ANDROID
#include <sys/select.h>
#endif

#include "NetworkAdapter.h"
#include "utility.h" // UNUSED macro
#include "log.h"
#include "IOChannel.h"
#include "WallClockTimer.h"
#include "GnashSleep.h"

#include <iostream> // std::cerr

#ifndef USE_CURL

namespace gnash {

// Stub for warning about access when no libcurl is defined.

std::auto_ptr<IOChannel>
NetworkAdapter::makeStream(const std::string& /*url*/, 
        const std::string& /*cachefile*/)
{
    log_error(_("libcurl is not available, but "
                "Gnash has attempted to use the curl adapter"));
    return std::auto_ptr<IOChannel>();
}

std::auto_ptr<IOChannel>
NetworkAdapter::makeStream(const std::string& url,
        const std::string& /*postdata*/,
        const std::string& cachefile)
{
    return makeStream(url, cachefile);
}

std::auto_ptr<IOChannel>
NetworkAdapter::makeStream(const std::string& url,
           const std::string& /*postdata*/,
            const RequestHeaders& /*headers*/,
           const std::string& cachefile)
{
    return makeStream(url, cachefile);
}

} // namespace gnash

#else // def USE_CURL

extern "C" {
#include <curl/curl.h>
}

#include "utility.h"
#include "GnashException.h"
#include "rc.h"
#include "GnashSystemFDHeaders.h"

#include <map>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstdio> // cached data uses a *FILE
#include <cstdlib> // std::getenv

#include <boost/version.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/thread/mutex.hpp>

#if BOOST_VERSION < 103500
# include <boost/thread/detail/lock.hpp>
#endif


//#define GNASH_CURL_VERBOSE 1

// define this if you want seeks back to be reported
//#define GNASH_CURL_WARN_SEEKSBACK 1


namespace gnash {

namespace {

inline
void lock(boost::mutex& mut)
{
#if BOOST_VERSION < 103500
    // see https://savannah.gnu.org/bugs/index.php?32579#comment7
    boost::detail::thread::lock_ops<boost::mutex>::lock(mut);
#else
    mut.lock();
#endif
}

inline
void unlock(boost::mutex& mut)
{
#if BOOST_VERSION < 103500
    // see https://savannah.gnu.org/bugs/index.php?32579#comment7
    boost::detail::thread::lock_ops<boost::mutex>::unlock(mut);
#else
    mut.unlock();
#endif
}


/***********************************************************************
 *
 *  CurlSession definition and implementation
 *
 **********************************************************************/

/// A libcurl session consists in a shared handle
/// sharing DNS cache and COOKIES.
///
class CurlSession {

public:

    /// Get CurlSession singleton
    static CurlSession& get();

    /// Get the shared handle
    CURLSH* getSharedHandle() { return _shandle; }

private:

    /// Initialize a libcurl session
    //
    /// A libcurl session consists in a shared handle
    /// sharing DNS cache and COOKIES.
    ///
    /// Also, global libcurl initialization function will
    /// be invoked, so MAKE SURE NOT TO CONSTRUCT multiple
    /// CurlSession instances in a multithreaded environment!
    ///
    /// AGAIN: this is NOT thread-safe !
    ///
    CurlSession();

    /// Cleanup curl session stuff (including global lib init)
    ~CurlSession();


    // the libcurl share handle, for sharing cookies
    CURLSH* _shandle;

    // mutex protecting share state
    boost::mutex _shareMutex;

    // mutex protecting shared cookies
    boost::mutex _cookieMutex;

    // mutex protecting shared dns cache
    boost::mutex _dnscacheMutex;

    /// Import cookies, if requested
    //
    /// This method will lookup GNASH_COOKIES_IN
    /// in the environment, and if existing, will
    /// parse the file sending each line to a fake
    /// easy handle created ad-hoc
    ///
    void importCookies();

    /// Export cookies, if requested
    //
    /// This method will lookup GNASH_COOKIES_OUT
    /// in the environment, and if existing, will
    /// create the file writing any cookie currently
    /// in the jar
    ///
    void exportCookies();

    /// Shared handle data locking function
    void lockSharedHandle(CURL* handle, curl_lock_data data,
            curl_lock_access access);

    /// Shared handle data unlocking function
    void unlockSharedHandle(CURL* handle, curl_lock_data data);

    /// Shared handle locking function
    static void lockSharedHandleWrapper(CURL* handle, curl_lock_data data,
            curl_lock_access access, void* userptr)
    {
        CurlSession* ci = static_cast<CurlSession*>(userptr);
        ci->lockSharedHandle(handle, data, access);
    }

    /// Shared handle unlocking function
    static void unlockSharedHandleWrapper(CURL* handle, curl_lock_data data,
            void* userptr)
    {
        // data defines what data libcurl wants to unlock, and you must
        // make sure that only one lock is given at any time for each kind
        // of data.
        // userptr is the pointer you set with CURLSHOPT_USERDATA. 
        CurlSession* ci = static_cast<CurlSession*>(userptr);
        ci->unlockSharedHandle(handle, data);
    }

};

CurlSession&
CurlSession::get()
{
    static CurlSession cs;
    return cs;
}

CurlSession::~CurlSession()
{
    log_debug("~CurlSession");
    exportCookies();

    CURLSHcode code;
    int retries=0;
    while ( (code=curl_share_cleanup(_shandle)) != CURLSHE_OK ) {
        if ( ++retries > 10 ) {
            log_error(_("Failed cleaning up share handle: %s. Giving up after %d retries."),
		      curl_share_strerror(code), retries);
            break;
        }
        log_error(_("Failed cleaning up share handle: %s. Will try again in a second."),
		  curl_share_strerror(code));
        gnashSleep(1000000);
    }
    _shandle = 0;
    curl_global_cleanup();
}

CurlSession::CurlSession()
    :
    _shandle(0),
    _shareMutex(),
    _cookieMutex(),
    _dnscacheMutex()
{
    // TODO: handle an error here (throw an exception)
    curl_global_init(CURL_GLOBAL_ALL);

    _shandle = curl_share_init();
    if (! _shandle) {
        throw GnashException("Failure initializing curl share handle");
    }

    CURLSHcode ccode;

    // Register share locking function
    ccode = curl_share_setopt(_shandle, CURLSHOPT_LOCKFUNC,
            lockSharedHandleWrapper);
    if ( ccode != CURLSHE_OK ) {
        throw GnashException(curl_share_strerror(ccode));
    }

    // Register share unlocking function
    ccode = curl_share_setopt(_shandle, CURLSHOPT_UNLOCKFUNC,
            unlockSharedHandleWrapper);
    if ( ccode != CURLSHE_OK ) {
        throw GnashException(curl_share_strerror(ccode));
    }

    // Activate sharing of cookies and DNS cache
    ccode = curl_share_setopt(_shandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
    if ( ccode != CURLSHE_OK ) {
        throw GnashException(curl_share_strerror(ccode));
    }

    // Activate sharing of DNS cache (since we're there..)
    ccode = curl_share_setopt(_shandle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    if ( ccode != CURLSHE_OK ) {
        throw GnashException(curl_share_strerror(ccode));
    }

    // Pass ourselves as the userdata
    ccode = curl_share_setopt(_shandle, CURLSHOPT_USERDATA, this);
    if ( ccode != CURLSHE_OK ) {
        throw GnashException(curl_share_strerror(ccode));
    }

    importCookies();
}

void
CurlSession::lockSharedHandle(CURL* handle, curl_lock_data data,
        curl_lock_access access)
{
    UNUSED(handle); // possibly being the 'easy' handle triggering the request ?

    // data defines what data libcurl wants to lock, and you must make
    // sure that only one lock is given at any time for each kind of data.
    // access defines what access type libcurl wants, shared or single.

    // TODO: see if we may make use of the 'access' parameter
    UNUSED(access);

    switch (data) {
        case CURL_LOCK_DATA_DNS:
            //log_debug("Locking DNS cache mutex");
            lock(_dnscacheMutex);
            //log_debug("DNS cache mutex locked");
            break;
        case CURL_LOCK_DATA_COOKIE:
            //log_debug("Locking cookies mutex");
            lock(_cookieMutex); 
            //log_debug("Cookies mutex locked");
            break;
        case CURL_LOCK_DATA_SHARE:
            //log_debug("Locking share mutex");
            lock(_shareMutex); 
            //log_debug("Share mutex locked");
            break;
        case CURL_LOCK_DATA_SSL_SESSION:
            log_error(_("lockSharedHandle: SSL session locking unsupported"));
            break;
        case CURL_LOCK_DATA_CONNECT:
            log_error(_("lockSharedHandle: connect locking unsupported"));
            break;
        case CURL_LOCK_DATA_LAST:
            log_error(_("lockSharedHandle: last locking unsupported ?!"));
            break;
        default:
            log_error(_("lockSharedHandle: unknown shared data %d"), data);
            break;
    }
}

void
CurlSession::unlockSharedHandle(CURL* handle, curl_lock_data data)
{
    UNUSED(handle); // possibly being the 'easy' handle triggering the request ?

    // data defines what data libcurl wants to lock, and you must make
    // sure that only one lock is given at any time for each kind of data.
    switch (data) {
    case CURL_LOCK_DATA_DNS:
	//log_debug("Unlocking DNS cache mutex");
	unlock(_dnscacheMutex);
	break;
    case CURL_LOCK_DATA_COOKIE:
	//log_debug("Unlocking cookies mutex");
	unlock(_cookieMutex);
	break;
    case CURL_LOCK_DATA_SHARE:
	//log_debug("Unlocking share mutex");
	unlock(_shareMutex);
	break;
    case CURL_LOCK_DATA_SSL_SESSION:
	log_error(_("unlockSharedHandle: SSL session locking unsupported"));
	break;
    case CURL_LOCK_DATA_CONNECT:
	log_error(_("unlockSharedHandle: connect locking unsupported"));
	break;
    case CURL_LOCK_DATA_LAST:
	log_error(_("unlockSharedHandle: last locking unsupported ?!"));
	break;
    default:
	log_error(_("unlockSharedHandle: unknown shared data %d"), data);
	break;
    }
}


/***********************************************************************
 *
 *  CurlStreamFile definition
 *
 **********************************************************************/

/// libcurl based IOChannel, for network uri accesses
class CurlStreamFile : public IOChannel
{

public:

    typedef std::map<std::string, std::string> PostData;

    /// Open a stream from the specified URL
    CurlStreamFile(const std::string& url, const std::string& cachefile);

    /// Open a stream from the specified URL posting the specified variables
    //
    /// @param url
    ///    The url to post to.
    ///
    /// @param vars
    ///    The url-encoded post data.
    ///
    CurlStreamFile(const std::string& url, const std::string& vars,
            const std::string& cachefile);
    
    CurlStreamFile(const std::string& url, const std::string& vars,
                   const NetworkAdapter::RequestHeaders& headers,
                   const std::string& cachefile);

    ~CurlStreamFile();

    // See dox in IOChannel.h
    virtual std::streamsize read(void *dst, std::streamsize bytes);

    // See dox in IOChannel.h
    virtual std::streamsize readNonBlocking(void *dst, std::streamsize bytes);

    // See dox in IOChannel.h
    virtual bool eof() const;

    // See dox in IOChannel.h
    virtual bool bad() const {
        return _error;
    }

    /// Report global position within the file
    virtual std::streampos tell() const;

    /// Put read pointer at given position
    virtual bool seek(std::streampos pos);

    /// Put read pointer at eof
    virtual void go_to_end();

    /// Returns the size of the stream
    //
    /// If size of the stream is unknown, 0 is returned.
    /// In that case you might try calling this function
    /// again after filling the cache a bit...
    ///
    /// Another approach might be filling the cache ourselves
    /// aiming at obtaining a useful value.
    ///
    virtual size_t size() const;

private:

    void init(const std::string& url, const std::string& cachefile);

    // Use this file to cache data
    FILE* _cache;

    // _cache file descriptor
    int _cachefd;

    // we keep a copy here to be sure the char*
    // is alive for the whole CurlStreamFile lifetime
    // TODO: don't really do this :)
    std::string _url;

    // the libcurl easy handle
    CURL *_handle;

    // the libcurl multi handle
    CURLM *_mhandle;

    // transfer in progress
    int _running;

    // stream error
    // false on no error.
    // Example of errors would be:
    //    404 - file not found
    //    timeout occurred
    bool _error;

    // Post data. Empty if no POST has been requested
    std::string _postdata;

    // Current size of cached data
    std::streampos _cached;

    /// Total stream size.
    //
    /// This will be 0 until known
    ///
    mutable size_t _size;

    // Attempt at filling the cache up to the given size.
    // Will call libcurl routines to fetch data.
    void fillCache(std::streampos size);

    // Process pending curl messages (handles 404)
    void processMessages();

    // Filling the cache as much as possible w/out blocking.
    // Will call libcurl routines to fetch data.
    void fillCacheNonBlocking();

    // Append sz bytes to the cache
    std::streamsize cache(void *from, std::streamsize sz);

    // Callback for libcurl, will be called
    // by fillCache() and will call cache()
    static size_t recv(void *buf, size_t size, size_t nmemb, void *userp);

    // List of custom headers for this stream.
    struct curl_slist *_customHeaders;
};


/***********************************************************************
 *
 *  Statics and CurlStreamFile implementation
 *
 **********************************************************************/

/*static private*/
size_t
CurlStreamFile::recv(void *buf, size_t size, size_t nmemb, void *userp)
{
#ifdef GNASH_CURL_VERBOSE
    log_debug("curl write callback called for (%d) bytes",
        size * nmemb);
#endif
    CurlStreamFile* stream = static_cast<CurlStreamFile*>(userp);
    return stream->cache(buf, size * nmemb);
}


/*private*/
std::streamsize
CurlStreamFile::cache(void *from, std::streamsize size)
{
    // take note of current position
    long curr_pos = std::ftell(_cache);

    // seek to the end
    std::fseek(_cache, 0, SEEK_END);

    std::streamsize wrote = std::fwrite(from, 1, size, _cache);
    if ( wrote < 1 ) {
        boost::format fmt = boost::format("writing to cache file: requested "
                                          "%d, wrote %d (%s)") %
                                          size % wrote % std::strerror(errno);
        throw GnashException(fmt.str());
    }

    // Set the size of cached data
    _cached = std::ftell(_cache);

    // reset position for next read
    std::fseek(_cache, curr_pos, SEEK_SET);

    return wrote;
}

/*private*/
void
CurlStreamFile::fillCacheNonBlocking()
{
    if ( ! _running ) {
#if GNASH_CURL_VERBOSE
        log_debug("Not running: fillCacheNonBlocking returning");
#endif
        return;
    }

    CURLMcode mcode;

    do {
        mcode = curl_multi_perform(_mhandle, &_running);
    } while (mcode == CURLM_CALL_MULTI_PERFORM); // && _running ?

    if (mcode != CURLM_OK) {
        throw GnashException(curl_multi_strerror(mcode));
    }

    // handle 404
    processMessages();
}


/*private*/
void
CurlStreamFile::fillCache(std::streampos size)
{

#if GNASH_CURL_VERBOSE
    log_debug("fillCache(%d), called, currently cached: %d", size, _cached);
#endif 

    assert(size >= 0);

    if ( ! _running || _cached >= size) {
#if GNASH_CURL_VERBOSE
        if (!_running) log_debug("Not running: returning");
        else log_debug("Already enough bytes cached: returning");
#endif
        return;
    }

    fd_set readfd, writefd, exceptfd;
    int maxfd;
    CURLMcode mcode;
    timeval tv;

    // Hard-coded slect timeout.
    // This number is kept low to give more thread switch
    // opportunities while waiting for a load.
    const long maxSleepUsec = 10000;  // 1/100 of a second

    const unsigned int userTimeout = static_cast<unsigned int>(
            RcInitFile::getDefaultInstance().getStreamsTimeout()*1000);

#ifdef GNASH_CURL_VERBOSE
    log_debug("User timeout is %u milliseconds", userTimeout);
#endif

    WallClockTimer lastProgress; // timer since last progress
    while (_running) {
        fillCacheNonBlocking(); // NOTE: will handle 404
	
        // Do this here to avoid calling select()
        // when we have enough bytes anyway, or
        // we reached EOF
        if (_cached >= size || !_running) break; 
	
#if GNASH_CURL_VERBOSE
        //log_debug("cached: %d, size: %d", _cached, size);
#endif
	
        // Zero these out _before_ calling curl_multi_fdset!
        FD_ZERO(&readfd);
        FD_ZERO(&writefd);
        FD_ZERO(&exceptfd);

        mcode = curl_multi_fdset(_mhandle, &readfd, &writefd, 
                &exceptfd, &maxfd);

        if (mcode != CURLM_OK) {
            // This is a serious error, not just a failure to add any
            // fds.
            throw GnashException(curl_multi_strerror(mcode));
        }

#ifdef GNASH_CURL_VERBOSE
        log_debug("Max fd: %d", maxfd);
#endif

        // A value of -1 means no file descriptors were added.
        if (maxfd < 0) {
#if GNASH_CURL_VERBOSE
            log_debug("curl_multi_fdset: maxfd == %1%", maxfd);
#endif
	    // As of libcurl 7.21.x, the DNS resolving appears to be going
	    // on in the background, so curl_multi_fdset fails to return
	    // anything useful. So we use the user timeout value to
	    // give DNS enough time to resolve the lookup.
            if (userTimeout && lastProgress.elapsed() > userTimeout) {
                log_error(_("FIXME: Timeout (%u milliseconds) while loading "
			    "from URL %s"), userTimeout, _url);
                // TODO: should we set _error here ?
                return;
            } else {
                continue;
            }
        }

        tv.tv_sec = 0;
        tv.tv_usec = maxSleepUsec;

#ifdef GNASH_CURL_VERBOSE
        log_debug("select() with %d milliseconds timeout", maxSleepUsec*1000);
#endif

        // Wait for data on the filedescriptors until a timeout set
        // in gnashrc.
        int ret = select(maxfd + 1, &readfd, &writefd, &exceptfd, &tv);

// select() will always fail on OS/2 and AmigaOS4 as we can't select
// on file descriptors, only on sockets
#if !defined(__OS2__) && !defined(__amigaos4__) && !defined(WIN32)
        if ( ret == -1 ) {
            if ( errno == EINTR ) {
                // we got interrupted by a signal
                // let's consider this as a timeout
#ifdef GNASH_CURL_VERBOSE
                log_debug("select() was interrupted by a signal");
#endif
                ret = 0;
            } else {
                // something unexpected happened
                boost::format fmt = boost::format(
                    "error polling data from connection to %s: %s ")
                    % _url % strerror(errno);
                throw GnashException(fmt.str());
            }
        }
#endif
        if ( ! ret ) {
            // Timeout, check the clock to see
            // if we expired the user Timeout
#ifdef GNASH_CURL_VERBOSE
            log_debug("select() timed out, elapsed is %u",
		      lastProgress.elapsed());
#endif
            if (userTimeout && lastProgress.elapsed() > userTimeout) {
                log_error(_("Timeout (%u milliseconds) while loading "
                            "from URL %s"), userTimeout, _url);
                // TODO: should we set _error here ?
                return;
            }
        } else {
            // Activity, reset the timer...
#ifdef GNASH_CURL_VERBOSE
            log_debug("FD activity, resetting progress timer");
#endif
            lastProgress.restart();
        }

    }

    // Processing messages is already done by fillCacheNonBlocking,
    // so we might likely avoid it here.
    processMessages();

}

/*private*/
void
CurlStreamFile::processMessages()
{
    CURLMsg *curl_msg;
    
    // The number of messages left in the queue (not used by us).
    int msgs;
    while ((curl_msg = curl_multi_info_read(_mhandle, &msgs))) {
        // Only for completed transactions
        if (curl_msg->msg == CURLMSG_DONE) {
	    
            // HTTP transaction succeeded
            if (curl_msg->data.result == CURLE_OK) {
		
                long code;
		
                // Check HTTP response
                curl_easy_getinfo(curl_msg->easy_handle,
				  CURLINFO_RESPONSE_CODE, &code);
		
                if ( code >= 400 ) {
                    log_error(_("HTTP response %ld from URL %s"),
			      code, _url);
                    _error = true;
                    _running = false;
                } else {
                    log_debug("HTTP response %ld from URL %s",
				code, _url);
                }

            } else {
                // Transaction failed, pass on curl error.
                log_error(_("CURL: %s"), curl_easy_strerror(
                                    curl_msg->data.result));
                _error = true;
            }

        }

    }
}


/*private*/
void
CurlStreamFile::init(const std::string& url, const std::string& cachefile)
{
    _customHeaders = 0;

    _url = url;
    _running = 1;
    _error = false;

    _cached = 0;
    _size = 0;

    _handle = curl_easy_init();
    _mhandle = curl_multi_init();

    const RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    
    if (!cachefile.empty()) {
        _cache = std::fopen(cachefile.c_str(), "w+b");
        if (!_cache) {

            log_error(_("Could not open specified path as cache file. Using a temporary file instead"));
            _cache = std::tmpfile();
        }
    } else {
	_cache = std::tmpfile();
    }

    if ( ! _cache ) {
        throw GnashException(_("Could not create temporary cache file"));
    }
    _cachefd = fileno(_cache);

    CURLcode ccode;

    // Override cURL's default verification of SSL certificates
    // This is insecure, so log security warning.
    // Equivalent to curl -k or curl --insecure.
    if (rcfile.insecureSSL()) {
        log_security(_("Allowing connections to SSL sites with invalid "
		       "certificates"));        
	
        ccode = curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYPEER, 0);
        if ( ccode != CURLE_OK ) {
            throw GnashException(curl_easy_strerror(ccode));
        }
	
        ccode = curl_easy_setopt(_handle, CURLOPT_SSL_VERIFYHOST, 0);
        if ( ccode != CURLE_OK ) {
            throw GnashException(curl_easy_strerror(ccode));
        }
    }

    // Get shared data
    ccode = curl_easy_setopt(_handle, CURLOPT_SHARE,
            CurlSession::get().getSharedHandle());

    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // Set expiration time for DNS cache entries, in seconds
    // 0 disables caching
    // -1 makes cache entries never expire
    // default is 60
    //
    // NOTE: this snippet is here just as a placeholder for whoever
    //       will feel a need to make this parametrizable
    //
    ccode = curl_easy_setopt(_handle, CURLOPT_DNS_CACHE_TIMEOUT, 60);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    ccode = curl_easy_setopt(_handle, CURLOPT_USERAGENT, "Gnash-" VERSION);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

#ifdef GNASH_CURL_VERBOSE
    // for verbose operations
    ccode = curl_easy_setopt(_handle, CURLOPT_VERBOSE, 1);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }
#endif

/* from libcurl-tutorial(3)
When using multiple threads you should set the CURLOPT_NOSIGNAL  option
to TRUE for all handles. Everything will work fine except that timeouts
are not honored during the DNS lookup - which you can  work  around  by
*/
    ccode = curl_easy_setopt(_handle, CURLOPT_NOSIGNAL, true);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // set url
    ccode = curl_easy_setopt(_handle, CURLOPT_URL, _url.c_str());
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    //curl_easy_setopt(_handle, CURLOPT_NOPROGRESS, false);


    // set write data and function
    ccode = curl_easy_setopt(_handle, CURLOPT_WRITEDATA, this);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    ccode = curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION,
        CurlStreamFile::recv);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    ccode = curl_easy_setopt(_handle, CURLOPT_FOLLOWLOCATION, 1);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    //fillCache(32); // pre-cache 32 bytes
    //curl_multi_perform(_mhandle, &_running);
}

/*public*/
CurlStreamFile::CurlStreamFile(const std::string& url,
        const std::string& cachefile)
{
    log_debug("CurlStreamFile %p created", this);
    init(url, cachefile);

    // CURLMcode ret =
    CURLMcode mcode = curl_multi_add_handle(_mhandle, _handle);
    if ( mcode != CURLM_OK ) {
        throw GnashException(curl_multi_strerror(mcode));
    }
}

/*public*/
CurlStreamFile::CurlStreamFile(const std::string& url, const std::string& vars,
       const std::string& cachefile)
{
    log_debug("CurlStreamFile %p created", this);
    init(url, cachefile);

    _postdata = vars;

    CURLcode ccode;

    ccode = curl_easy_setopt(_handle, CURLOPT_POST, 1);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // libcurl needs to access the POSTFIELDS during 'perform' operations,
    // so we must use a string whose lifetime is ensured to be longer then
    // the multihandle itself.
    // The _postdata member should meet this requirement
    ccode = curl_easy_setopt(_handle, CURLOPT_POSTFIELDS, _postdata.c_str());
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // This is to support binary strings as postdata
    // NOTE: in version 7.11.1 CURLOPT_POSTFIELDSIZE_LARGE was added
    //       this one takes a long, that one takes a curl_off_t
    //
    ccode = curl_easy_setopt(_handle, CURLOPT_POSTFIELDSIZE, _postdata.size());
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // Disable sending an Expect: header, as some older HTTP/1.1
    // don't implement them, and some (namely lighttpd/1.4.19,
    // running on openstreetmap.org at time of writing) return
    // a '417 Expectance Failure' response on getting that.
    assert ( ! _customHeaders );
    _customHeaders = curl_slist_append(_customHeaders, "Expect:");
    ccode = curl_easy_setopt(_handle, CURLOPT_HTTPHEADER, _customHeaders);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    CURLMcode mcode = curl_multi_add_handle(_mhandle, _handle);
    if ( mcode != CURLM_OK ) {
        throw GnashException(curl_multi_strerror(mcode));
    }

}

/*public*/
CurlStreamFile::CurlStreamFile(const std::string& url, const std::string& vars,
        const NetworkAdapter::RequestHeaders& headers,
        const std::string& cachefile)
{
    log_debug("CurlStreamFile %p created", this);
    init(url, cachefile);

    _postdata = vars;

    // Disable sending an Expect: header, as some older HTTP/1.1
    // don't implement them, and some (namely lighttpd/1.4.19,
    // running on openstreetmap.org at time of writing) return
    // a '417 Expectance Failure' response on getting that.
    //
    // Do this before adding user-requested headers so user
    // specified ones take precedence
    //
    assert ( ! _customHeaders );
    _customHeaders = curl_slist_append(_customHeaders, "Expect:");

    for (NetworkAdapter::RequestHeaders::const_iterator i = headers.begin(),
	     e = headers.end(); i != e; ++i) {
        // Check here to see whether header name is allowed.
        if (!NetworkAdapter::isHeaderAllowed(i->first)) continue;
        std::ostringstream os;
        os << i->first << ": " << i->second;
        _customHeaders = curl_slist_append(_customHeaders, os.str().c_str());
    }

    CURLcode ccode;

    ccode = curl_easy_setopt(_handle, CURLOPT_HTTPHEADER, _customHeaders);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    ccode = curl_easy_setopt(_handle, CURLOPT_POST, 1);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // libcurl needs to access the POSTFIELDS during 'perform' operations,
    // so we must use a string whose lifetime is ensured to be longer then
    // the multihandle itself.
    // The _postdata member should meet this requirement
    ccode = curl_easy_setopt(_handle, CURLOPT_POSTFIELDS, _postdata.c_str());
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // This is to support binary strings as postdata
    // NOTE: in version 7.11.1 CURLOPT_POSTFIELDSIZE_LARGE was added
    //       this one takes a long, that one takes a curl_off_t
    //
    ccode = curl_easy_setopt(_handle, CURLOPT_POSTFIELDSIZE, _postdata.size());
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    CURLMcode mcode = curl_multi_add_handle(_mhandle, _handle);
    if ( mcode != CURLM_OK ) {
        throw GnashException(curl_multi_strerror(mcode));
    }

}


/*public*/
CurlStreamFile::~CurlStreamFile()
{
    log_debug("CurlStreamFile %p deleted", this);
    curl_multi_remove_handle(_mhandle, _handle);
    curl_easy_cleanup(_handle);
    curl_multi_cleanup(_mhandle);
    std::fclose(_cache);
    if ( _customHeaders ) curl_slist_free_all(_customHeaders); 
}

/*public*/
std::streamsize
CurlStreamFile::read(void *dst, std::streamsize bytes)
{
    if ( eof() || _error ) return 0;

#ifdef GNASH_CURL_VERBOSE
    log_debug("read(%d) called", bytes);
#endif

    fillCache(bytes + tell());
    if ( _error ) return 0; // error can be set by fillCache

#ifdef GNASH_CURL_VERBOSE
    log_debug("_cache.tell = %d", tell());
#endif

    return std::fread(dst, 1, bytes, _cache);
}

/*public*/
std::streamsize
CurlStreamFile::readNonBlocking(void *dst, std::streamsize bytes)
{
#ifdef GNASH_CURL_VERBOSE
    log_debug("readNonBlocking(%d) called", bytes);
#endif

    if ( eof() || _error ) return 0;

    fillCacheNonBlocking();
    if ( _error ) {
        // I guess an exception would be thrown in this case ?
        log_error(_("curl adaptor's fillCacheNonBlocking set _error rather then throwing an exception"));
        return 0; 
    }

    std::streamsize actuallyRead = std::fread(dst, 1, bytes, _cache);
    if ( _running ) {
        // if we're still running drop any eof flag
        // on the cache
        clearerr(_cache);
    }

    return actuallyRead;

}

/*public*/
bool
CurlStreamFile::eof() const
{
    bool ret = ( ! _running && feof(_cache) );

#ifdef GNASH_CURL_VERBOSE
    log_debug("eof() returning %d", ret);
#endif
    return ret;

}

/*public*/
std::streampos
CurlStreamFile::tell() const
{
    std::streampos ret = std::ftell(_cache);

#ifdef GNASH_CURL_VERBOSE
    log_debug("tell() returning %ld", ret);
#endif

    return ret;

}

/*public*/
bool
CurlStreamFile::seek(std::streampos pos)
{

    if ( pos < 0 ) {
        std::ostringstream os;
        os << "CurlStreamFile: can't seek to negative absolute position "
           << pos;
        throw IOException(os.str());
    }

#ifdef GNASH_CURL_WARN_SEEKSBACK
    if ( pos < tell() ) {
        log_debug("Warning: seek backward requested (%ld from %ld)",
            pos, tell());
    }
#endif

    fillCache(pos);
    if (_error) return false; // error can be set by fillCache

    if (_cached < pos) {
        log_error(_("Warning: could not cache enough bytes on seek: %d requested, %d cached"),
		  pos, _cached);
        return false; 
    }

    if (std::fseek(_cache, pos, SEEK_SET) == -1) {
        log_error(_("Warning: fseek failed"));
        return false;
    }
   
    return true;

}

/*public*/
void
CurlStreamFile::go_to_end()
{
    CURLMcode mcode;
    while (_running > 0) {
        do {
            mcode=curl_multi_perform(_mhandle, &_running);
        } while ( mcode == CURLM_CALL_MULTI_PERFORM );

        if ( mcode != CURLM_OK ) {
            throw IOException(curl_multi_strerror(mcode));
        }
	
        long code;
        curl_easy_getinfo(_handle, CURLINFO_RESPONSE_CODE, &code);
        if (code == 404) {	// file not found!
            throw IOException("File not found");
        }

    }

    if (std::fseek(_cache, 0, SEEK_END) == -1) {
        throw IOException("NetworkAdapter: fseek to end failed");
    } 
}

/*public*/
size_t
CurlStreamFile::size() const
{
    if ( ! _size ) {
        double size;
        CURLcode ret = curl_easy_getinfo(_handle,
                CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
        if (ret == CURLE_OK) {
            assert(size <= std::numeric_limits<size_t>::max());
            _size = static_cast<size_t>(size);
        }
    }

#ifdef GNASH_CURL_VERBOSE
    log_debug("get_stream_size() returning %lu", _size);
#endif

    return _size;

}

void
CurlSession::importCookies()
{
    const char* cookiesIn = std::getenv("GNASH_COOKIES_IN");
    if ( ! cookiesIn ) return; // nothing to do

    ////////////////////////////////////////////////////////////////
    //
    // WARNING: what we're doing here is an ugly hack
    //
    // We'll be creating a fake easy handle for the sole purpos
    // of importing cookies. Tests conducted against 7.15.5-CVS
    // resulted in this working if a CURLOPT_URL is given, even
    // if invalid (but non-0!), while wouldn't if NO CURLOPT_URL
    // is given, with both cases returning the same CURLcode on
    // _perform (URL using bad/illegal format or missing URL)
    //
    // TODO: instead, we should be reading the input file
    //       ourselves and use CURLOPT_COOKIELIST to send
    //       each line. Doing so should not require a
    //       _perform call.
    //
    ////////////////////////////////////////////////////////////////

    // Create a fake handle for purpose of importing data
    CURL* fakeHandle = curl_easy_init(); // errors to handle here ?
    CURLcode ccode;

    // Configure the fake handle to use the share (shared cookies in particular..)
    ccode = curl_easy_setopt(fakeHandle, CURLOPT_SHARE, getSharedHandle());
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // Configure the fake handle to read cookies from the specified file
    ccode = curl_easy_setopt(fakeHandle, CURLOPT_COOKIEFILE, cookiesIn);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // need to pass a non-zero URL string for COOKIEFILE to
    // be really parsed
    ccode = curl_easy_setopt(fakeHandle, CURLOPT_URL, "");
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // perform, to activate actual cookie file parsing
    //
    // NOTE: curl_easy_perform is expected to return an
    //       "URL using bad/illegal format or missing URL" error,
    //       there's no way to detect actual cookies import errors
    //       other then using curl debugging output routines
    //
    log_debug("Importing cookies from file '%s'", cookiesIn);
    curl_easy_perform(fakeHandle);

    curl_easy_cleanup(fakeHandle);

}

void
CurlSession::exportCookies()
{
    const char* cookiesOut = std::getenv("GNASH_COOKIES_OUT");
    if ( ! cookiesOut ) return; // nothing to do

    ////////////////////////////////////////////////////////////////
    //
    // WARNING: what we're doing here is an ugly hack
    //
    // We'll be creating a fake easy handle for the sole purpose
    // of exporting cookies. Tests conducted against 7.15.5-CVS
    // resulted in this working w/out a CURLOPT_URL and w/out a
    // curl_easy_perform.
    // 
    // NOTE: the "correct" way would be to use CURLOPT_COOKIELIST
    //       with the "FLUSH" special string as value, but that'd
    //       be only supported by version 7.17.1
    //
    ////////////////////////////////////////////////////////////////

    CURL* fakeHandle = curl_easy_init(); // errors to handle here ?
    CURLcode ccode;

    // Configure the fake handle to use the share (shared cookies in particular..)
    ccode = curl_easy_setopt(fakeHandle, CURLOPT_SHARE, getSharedHandle());
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // Configure the fake handle to write cookies to the specified file
    ccode = curl_easy_setopt(fakeHandle, CURLOPT_COOKIEJAR , cookiesOut);
    if ( ccode != CURLE_OK ) {
        throw GnashException(curl_easy_strerror(ccode));
    }

    // Cleanup, to trigger actual cookie file flushing
    log_debug("Exporting cookies file '%s'", cookiesOut);
    curl_easy_cleanup(fakeHandle);

}


} // anonymous namespace

//-------------------------------------------
// Exported interfaces
//-------------------------------------------

std::auto_ptr<IOChannel>
NetworkAdapter::makeStream(const std::string& url, const std::string& cachefile)
{
#ifdef GNASH_CURL_VERBOSE
    log_debug("making curl stream for %s", url);
#endif

    std::auto_ptr<IOChannel> stream;

    try {
        stream.reset(new CurlStreamFile(url, cachefile));
    }
    catch (const std::exception& ex) {
        log_error(_("curl stream: %s"), ex.what());
    }
    return stream;
}

std::auto_ptr<IOChannel>
NetworkAdapter::makeStream(const std::string& url, const std::string& postdata,
        const std::string& cachefile)
{
#ifdef GNASH_CURL_VERBOSE
    log_debug("making curl stream for %s", url);
#endif

    std::auto_ptr<IOChannel> stream;

    try {
        stream.reset(new CurlStreamFile(url, postdata, cachefile));
    }
    catch (const std::exception& ex) {
        log_error(_("curl stream: %s"), ex.what());
    }
    return stream;
}

std::auto_ptr<IOChannel>
NetworkAdapter::makeStream(const std::string& url, const std::string& postdata,
        const RequestHeaders& headers, const std::string& cachefile)
{

    std::auto_ptr<IOChannel> stream;

    try {
        stream.reset(new CurlStreamFile(url, postdata, headers, cachefile));
    }
    catch (const std::exception& ex) {
        log_error(_("curl stream: %s"), ex.what());
    }

    return stream;

}

const NetworkAdapter::ReservedNames&
NetworkAdapter::reservedNames()
{
    static const ReservedNames names = boost::assign::list_of
    ("Accept-Ranges")
    ("Age")
    ("Allow")
    ("Allowed")
    ("Connection")
    ("Content-Length")
    ("Content-Location")
    ("Content-Range")
    ("ETag")
    ("GET")
    ("Host")
    ("HEAD")
    ("Last-Modified")
    ("Locations")
    ("Max-Forwards")
    ("POST")
    ("Proxy-Authenticate")
    ("Proxy-Authorization")
    ("Public")
    ("Range")
    ("Retry-After")
    ("Server")
    ("TE")
    ("Trailer")
    ("Transfer-Encoding")
    ("Upgrade")
    ("URI")
    ("Vary")
    ("Via")
    ("Warning")
    ("WWW-Authenticate");

    return names;
}

} // namespace gnash

#endif // def USE_CURL

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
