// LoadableObject.cpp: abstraction of network-loadable AS object functions.
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

#include "RunResources.h"
#include "LoadableObject.h"
#include "log.h"
#include "Array_as.h"
#include "as_object.h"
#include "StreamProvider.h"
#include "URL.h"
#include "namedStrings.h"
#include "movie_root.h"
#include "VM.h"
#include "NativeFunction.h"
#include "utf8.h"
#include "fn_call.h"
#include "GnashAlgorithm.h"
#include "Global_as.h"
#include "IOChannel.h"

#include <sstream>
#include <map>
#include <boost/tokenizer.hpp>

namespace gnash {

namespace {
    as_value loadableobject_send(const fn_call& fn);
    as_value loadableobject_load(const fn_call& fn);
    as_value loadableobject_decode(const fn_call& fn);
    as_value loadableobject_sendAndLoad(const fn_call& fn);
    as_value loadableobject_getBytesTotal(const fn_call& fn);
    as_value loadableobject_getBytesLoaded(const fn_call& fn);
    as_value loadableobject_addRequestHeader(const fn_call& fn);
}

void
attachLoadableInterface(as_object& o, int flags)
{
    Global_as& gl = getGlobal(o);

	o.init_member("addRequestHeader", gl.createFunction(
	            loadableobject_addRequestHeader), flags);
	o.init_member("getBytesLoaded", gl.createFunction(
	            loadableobject_getBytesLoaded),flags);
	o.init_member("getBytesTotal", gl.createFunction(
                loadableobject_getBytesTotal), flags);
}

void
registerLoadableNative(as_object& o)
{
    VM& vm = getVM(o);

    vm.registerNative(loadableobject_load, 301, 0);
    vm.registerNative(loadableobject_send, 301, 1);
    vm.registerNative(loadableobject_sendAndLoad, 301, 2);

    /// This is only automatically used in LoadVars.
    vm.registerNative(loadableobject_decode, 301, 3);
}

/// Functors for use with foreachArray
namespace {

class WriteHeaders
{
public:

    WriteHeaders(NetworkAdapter::RequestHeaders& headers)
        :
        _headers(headers),
        _i(0)
    {}

    void operator()(const as_value& val)
    {
        // Store even elements and continue
        if (!(_i++ % 2)) {
            _key = val;
            return;
        }

        // Both elements apparently must be strings, or we move onto the 
        // next pair.
        if (!val.is_string() || !_key.is_string()) return;
        _headers[_key.to_string()] = val.to_string();
    }

private:
    as_value _key;
    NetworkAdapter::RequestHeaders _headers;
    size_t _i;
};

class GetHeaders
{
public:

    GetHeaders(as_object& target)
        :
        _target(target),
        _i(0)
    {}

    void operator()(const as_value& val)
    {
        // Store even elements and continue
        if (!(_i++ % 2)) {
            _key = val;
            return;
        }

        // Both elements apparently must be strings, or we move onto the 
        // next pair.
        if (!val.is_string() || !_key.is_string()) return;
        callMethod(&_target, NSV::PROP_PUSH, _key, val);
    }

private:
    as_value _key;
    as_object& _target;
    size_t _i;
};

as_value
loadableobject_getBytesLoaded(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> ptr = ensure<ValidThis>(fn);
    as_value bytesLoaded;
    ptr->get_member(NSV::PROP_uBYTES_LOADED, &bytesLoaded);
    return bytesLoaded;
}
    
as_value
loadableobject_getBytesTotal(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> ptr = ensure<ValidThis>(fn);
    as_value bytesTotal;
    ptr->get_member(NSV::PROP_uBYTES_TOTAL, &bytesTotal);
    return bytesTotal;
}

/// Can take either a two strings as arguments or an array of strings,
/// alternately header and value.
as_value
loadableobject_addRequestHeader(const fn_call& fn)
{
    
    as_value customHeaders;
    as_object* array;

    if (fn.this_ptr->get_member(NSV::PROP_uCUSTOM_HEADERS, &customHeaders))
    {
        array = toObject(customHeaders, getVM(fn));
        if (!array)
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("XML.addRequestHeader: XML._customHeaders "
                              "is not an object"));
            );
            return as_value();
        }
    }
    else {
        array = getGlobal(fn).createArray();
        // This property is always initialized on the first call to
        // addRequestHeaders. It has default properties.
        fn.this_ptr->init_member(NSV::PROP_uCUSTOM_HEADERS, array);
    }

    if (fn.nargs == 0)
    {
        // Return after having initialized the _customHeaders array.
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("XML.addRequestHeader requires at least "
                          "one argument"));
        );
        return as_value();
    }
    
    if (fn.nargs == 1) {
        // This must be an array (or something like it). Keys / values are
        // pushed in valid pairs to the _customHeaders array.    
        as_object* headerArray = toObject(fn.arg(0), getVM(fn));

        if (!headerArray) {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("XML.addRequestHeader: single argument "
                                "is not an array"));
            );
            return as_value();
        }

        GetHeaders gh(*array);
        foreachArray(*headerArray, gh);
        return as_value();
    }
        
    if (fn.nargs > 2)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("XML.addRequestHeader(%s): arguments after the"
                            "second will be discarded"), ss.str());
        );
    }
    
    // Push both to the _customHeaders array.
    const as_value& name = fn.arg(0);
    const as_value& val = fn.arg(1);
    
    // Both arguments must be strings.
    if (!name.is_string() || !val.is_string())
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("XML.addRequestHeader(%s): both arguments "
                        "must be a string"), ss.str());
        );
        return as_value(); 
    }

    callMethod(array, NSV::PROP_PUSH, name, val);
    
    return as_value();
}
/// Decode method (ASnative 301, 3) can be applied to any as_object.
as_value
loadableobject_decode(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) return as_value(false);

    typedef std::map<std::string, std::string> ValuesMap;
    ValuesMap vals;

    const int version = getSWFVersion(fn);
    const std::string qs = fn.arg(0).to_string(version);

    if (qs.empty()) return as_value();

    typedef boost::char_separator<char> Sep;
    typedef boost::tokenizer<Sep> Tok;
    Tok t1(qs, Sep("&"));

    VM& vm = getVM(fn);

    for (Tok::iterator tit=t1.begin(); tit!=t1.end(); ++tit) {

        const std::string& nameval = *tit;

        std::string name;
        std::string value;

        size_t eq = nameval.find("=");
        if (eq == std::string::npos) name = nameval;
        else {
            name = nameval.substr(0, eq);
            value = nameval.substr(eq + 1);
        }

        URL::decode(name);
        URL::decode(value);

        if (!name.empty()) ptr->set_member(getURI(vm, name), value);
    }

    return as_value(); 
}

/// Returns true if the arguments are valid, otherwise false. The
/// success of the connection is irrelevant.
/// The second argument must be a loadable object (XML or LoadVars).
/// An optional third argument specifies the method ("GET", or by default
/// "POST"). The values are partly URL encoded if using GET.
as_value
loadableobject_sendAndLoad(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if ( fn.nargs < 2 ) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("sendAndLoad() requires at least two arguments"));
        );
        return as_value(false);
    }

    const std::string& urlstr = fn.arg(0).to_string();
    if ( urlstr.empty() ) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("sendAndLoad(): invalid empty url"));
        );
        return as_value(false);
    }

    if (!fn.arg(1).is_object()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("sendAndLoad(): invalid target (must be an "
                        "XML or LoadVars object)"));
        );
        return as_value(false);
    }

    // TODO: if this isn't an XML or LoadVars, it won't work, but we should
    // check how far things get before it fails.
    as_object* target = toObject(fn.arg(1), getVM(fn));

    // According to the Flash 8 Cookbook (Joey Lott, Jeffrey Bardzell), p 427,
    // this method sends by GET unless overridden, and always by GET in the
    // standalone player. We have no tests for this, but a Twitter widget
    // gets Bad Request from the server if we send via POST.
    bool post = false;

    if (fn.nargs > 2) {
        const std::string& method = fn.arg(2).to_string();
        StringNoCaseEqual nc;
        post = nc(method, "post");
    }      

    const RunResources& ri = getRunResources(*obj);

    URL url(urlstr, ri.streamProvider().baseURL());

    std::auto_ptr<IOChannel> str;

    if (post) {
        as_value customHeaders;

        NetworkAdapter::RequestHeaders headers;

        if (obj->get_member(NSV::PROP_uCUSTOM_HEADERS, &customHeaders)) {

            /// Read in our custom headers if they exist and are an
            /// array.
            as_object* array = toObject(customHeaders, getVM(fn));
            if (array) {
                WriteHeaders wh(headers);
                foreachArray(*array, wh);
            }
        }

        as_value contentType;
        if (obj->get_member(NSV::PROP_CONTENT_TYPE, &contentType)) {
            // This should not overwrite anything set in 
            // LoadVars.addRequestHeader();
            headers.insert(std::make_pair("Content-Type", 
                        contentType.to_string()));
        }

        // Convert the object to a string to send. XML should
        // not be URL encoded for the POST method, LoadVars
        // is always URL encoded.
        const std::string& strval = as_value(obj).to_string();

        /// It doesn't matter if there are no request headers.
        str = ri.streamProvider().getStream(url, strval, headers);
    }
    else {
        // Convert the object to a string to send. XML should
        // not be URL encoded for the GET method.
        const std::string& dataString = as_value(obj).to_string();

        // Any data must be added to the existing querystring.
        if (!dataString.empty()) {

            std::string existingQS = url.querystring();
            if (!existingQS.empty()) existingQS += "&";

            url.set_querystring(existingQS + dataString);
        }

        log_debug("Using GET method for sendAndLoad: %s", url.str());
        str = ri.streamProvider().getStream(url.str());
    }

    log_security(_("Loading from url: '%s'"), url.str());
    
    movie_root& mr = getRoot(*obj);
    
    /// All objects get a loaded member, set to false.
    target->set_member(NSV::PROP_LOADED, false);

    mr.addLoadableObject(target, str);
    return as_value(true);
}


as_value
loadableobject_load(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    if ( fn.nargs < 1 )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("load() requires at least one argument"));
        );
        return as_value(false);
    }

    const std::string& urlstr = fn.arg(0).to_string();
    if ( urlstr.empty() )
    {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror(_("load(): invalid empty url"));
        );
        return as_value(false);
    }

    // Set loaded property to false; will be updated (hopefully)
    // when loading is complete.
    obj->set_member(NSV::PROP_LOADED, false);

    const RunResources& ri = getRunResources(*obj);

    URL url(urlstr, ri.streamProvider().baseURL());

    // Checks whether access is allowed.
    std::auto_ptr<IOChannel> str(ri.streamProvider().getStream(url));

    movie_root& mr = getRoot(fn);
    mr.addLoadableObject(obj, str);

    obj->set_member(NSV::PROP_uBYTES_LOADED, 0.0);
    obj->set_member(NSV::PROP_uBYTES_TOTAL, as_value());

    return as_value(true);

}

    
as_value
loadableobject_send(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
 
    std::string target;
    std::string url;
    std::string method;

    switch (fn.nargs) {
        case 0:
            return as_value(false);
        case 3:
            method = fn.arg(2).to_string();
        case 2:
            target = fn.arg(1).to_string();
        case 1:
            url = fn.arg(0).to_string();
            break;
    }

    StringNoCaseEqual noCaseCompare;
    
    // POST is the default in a browser, GET supposedly default
    // in a Flash test environment (whatever that is).
    MovieClip::VariablesMethod meth = noCaseCompare(method, "get") ?
        MovieClip::METHOD_GET : MovieClip::METHOD_POST;

    // Encode the data in the default way for the type.
    std::ostringstream data;

    movie_root& m = getRoot(fn);

    // Encode the object for HTTP. If post is true,
    // XML should not be encoded. LoadVars is always
    // encoded.
    // TODO: test properly.
    const std::string& str = as_value(obj).to_string();

    m.getURL(url, target, str, meth);

    return as_value(true);
}

} // anonymous namespace
} // namespace gnash
