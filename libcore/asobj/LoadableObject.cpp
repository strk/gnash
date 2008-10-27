// LoadableObject.cpp: abstraction of network-loadable AS object functions.
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

#include "LoadableObject.h"
#include "log.h"
#include "array.h"
#include "as_object.h"
#include "StreamProvider.h"
#include "URL.h"
#include "namedStrings.h"
#include "VM.h"
#include "builtin_function.h"
#include "timers.h"
#include "utf8.h"
#include "fn_call.h"

#include <sstream>
#include <map>

namespace gnash {


LoadableObject::LoadableObject()
    :
    _bytesLoaded(-1),
    _bytesTotal(-1),
    _loadCheckerTimer(0)
{
}    


LoadableObject::~LoadableObject()
{
    for (LoadThreadList::iterator it = _loadThreads.begin(),
            e = _loadThreads.end(); it != e; ++it)
    {
        // Joins the thread
        delete *it;
    }

    if ( _loadCheckerTimer )
    {
        _vm.getRoot().clear_interval_timer(_loadCheckerTimer);
    }

}


void
LoadableObject::send(const std::string& urlstr, const std::string& target,
        bool post)
{
    movie_root& m = _vm.getRoot();

    // Encode the object for HTTP. If post is true,
    // XML should not be encoded. LoadVars is always
    // encoded.
    // TODO: test properly.
    std::ostringstream data;
    toString(data, !post);

    // Only GET and POST are possible here.
    MovieClip::VariablesMethod method = post ? MovieClip::METHOD_POST :
                                               MovieClip::METHOD_GET;

    m.getURL(urlstr, target, data.str(), method);

}


void
LoadableObject::sendAndLoad(const std::string& urlstr,
                            as_object& target, bool post)
{

    /// All objects get a loaded member, set to false.
    target.set_member(NSV::PROP_LOADED, false);

	URL url(urlstr, get_base_url());

	std::auto_ptr<IOChannel> str;
	if (post)
    {
        as_value customHeaders;

        NetworkAdapter::RequestHeaders headers;

        if (get_member(NSV::PROP_uCUSTOM_HEADERS, &customHeaders))
        {

            /// Read in our custom headers if they exist and are an
            /// array.
            Array_as* array = dynamic_cast<Array_as*>(
                            customHeaders.to_object().get());
                            
            if (array)
            {
                Array_as::const_iterator e = array->end();
                --e;

                for (Array_as::const_iterator i = array->begin(); i != e; ++i)
                {
                    // Only even indices can be a header.
                    if (i.index() % 2) continue;
                    if (! (*i).is_string()) continue;
                    
                    // Only the immediately following odd number can be 
                    // a value.
                    if (array->at(i.index() + 1).is_string())
                    {
                        const std::string& name = (*i).to_string();
                        const std::string& val =
                                    array->at(i.index() + 1).to_string();
                                    
                        // Values should overwrite existing ones.
                        headers[name] = val;
                    }
                    
                }
            }
        }

        as_value contentType;

        if (get_member(NSV::PROP_CONTENT_TYPE, &contentType))
        {
            // This should not overwrite anything set in 
            // LoadVars.addRequestHeader();
            headers.insert(std::make_pair("Content-Type", 
                        contentType.to_string()));
        }

        // Convert the object to a string to send. XML should
        // not be URL encoded for the POST method, LoadVars
        // is always URL encoded.
        std::ostringstream data;
        toString(data, false);

        /// It doesn't matter if there are no request headers.
        str = StreamProvider::getDefaultInstance().getStream(url,
                                                    data.str(), headers);
    }
	else
    {
        // Convert the object to a string to send. XML should
        // not be URL encoded for the GET method.
        std::ostringstream data;
        toString(data, true);

    	std::string getURL = urlstr + "?" + data.str();
        log_debug("Using GET method for sendAndLoad: %s", getURL);
        str = StreamProvider::getDefaultInstance().getStream(getURL);
    }

	if (!str.get()) 
	{
		log_error(_("Can't load from %s (security?)"), url.str());
		return;
		// TODO: check if this is correct
		//as_value nullValue; nullValue.set_null();
		//callMethod(VM::get().getStringTable().find(PROPNAME("onData")), nullValue);
	}

	log_security(_("Loading from url: '%s'"), url.str());
    target.queueLoad(str);
	
}

void
LoadableObject::load(const std::string& urlstr)
{
    // Set loaded property to false; will be updated (hopefully)
    // when loading is complete.
	set_member(NSV::PROP_LOADED, false);

	URL url(urlstr, get_base_url());

    // Checks whether access is allowed.
    std::auto_ptr<IOChannel> str(StreamProvider::getDefaultInstance().getStream(url));

	if (!str.get()) 
	{
		log_error(_("Can't load variables from %s (security?)"), url.str());
		return;
		// TODO: check if this is correct
		//as_value nullValue; nullValue.set_null();
		//callMethod(VM::get().getStringTable().find(PROPNAME("onData")), nullValue);
	}

	log_security(_("Loading from url: '%s'"), url.str());
    queueLoad(str);
}


void
LoadableObject::queueLoad(std::auto_ptr<IOChannel> str)
{

    bool startTimer = _loadThreads.empty();

    std::auto_ptr<LoadThread> lt ( new LoadThread(str) );

    // we push on the front to avoid invalidating
    // iterators when queueLoad is called as effect
    // of onData invocation.
    // Doing so also avoids processing queued load
    // request immediately
    // 
    _loadThreads.push_front(lt.release());

    if ( startTimer )
    {
        boost::intrusive_ptr<builtin_function> loadsChecker = 
            new builtin_function(&checkLoads_wrapper);

        std::auto_ptr<Timer> timer(new Timer);
        timer->setInterval(*loadsChecker, 50, this);
        _loadCheckerTimer = getVM().getRoot().add_interval_timer(timer, true);

    }

    _bytesLoaded = 0;
    _bytesTotal = -1;

}

as_value
LoadableObject::checkLoads_wrapper(const fn_call& fn)
{
	boost::intrusive_ptr<LoadableObject> ptr =
        ensureType<LoadableObject>(fn.this_ptr);
	ptr->checkLoads();
	return as_value();
}


void
LoadableObject::checkLoads()
{

    if (_loadThreads.empty()) return; // nothing to do

    for (LoadThreadList::iterator it=_loadThreads.begin();
            it != _loadThreads.end(); )
    {
        LoadThread* lt = *it;

        if ( lt->completed() )
        {
            size_t dataSize = _bytesTotal = _bytesLoaded = lt->getBytesTotal();

            boost::scoped_array<char> buf(new char[dataSize + 1]);
            size_t actuallyRead = lt->read(buf.get(), dataSize);
            if ( actuallyRead != dataSize )
			{
				// This would be either a bug of LoadThread or an expected
				// possibility which lacks documentation (thus a bug in
                // documentation)
				//
			}
            buf[actuallyRead] = '\0';

            // Strip BOM, if any.
            // See http://savannah.gnu.org/bugs/?19915
            utf8::TextEncoding encoding;
            // NOTE: the call below will possibly change 'xmlsize' parameter
            char* bufptr = utf8::stripBOM(buf.get(), dataSize, encoding);
            if ( encoding != utf8::encUTF8 && encoding != utf8::encUNSPECIFIED )
            {
                log_unimpl("%s to utf8 conversion in LoadVars input parsing", 
                        utf8::textEncodingName(encoding));
            }
            as_value dataVal(bufptr); // memory copy here (optimize?)

            it = _loadThreads.erase(it);
            delete lt; // supposedly joins the thread...

            // might push_front on the list..
            callMethod(NSV::PROP_ON_DATA, dataVal);

        }
        else
        {
            _bytesLoaded = lt->getBytesLoaded();
            ++it;
        }
    }

    if ( _loadThreads.empty() ) 
    {
        _vm.getRoot().clear_interval_timer(_loadCheckerTimer);
        _loadCheckerTimer=0;
    }
}


/// Can take either a two strings as arguments or an array of strings,
/// alternately header and value.
as_value
LoadableObject::loadableobject_addRequestHeader(const fn_call& fn)
{
    
    boost::intrusive_ptr<LoadableObject> ptr = ensureType<LoadableObject>(fn.this_ptr);   

    as_value customHeaders;
    as_object* array;

    if (ptr->get_member(NSV::PROP_uCUSTOM_HEADERS, &customHeaders))
    {
        array = customHeaders.to_object().get();
        if (!array)
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("XML.addRequestHeader: XML._customHeaders "
                              "is not an object"));
            );
            return as_value();
        }
    }
    else
    {
        array = new Array_as;
        // This property is always initialized on the first call to
        // addRequestHeaders.
        ptr->set_member(NSV::PROP_uCUSTOM_HEADERS, array);
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
    
    if (fn.nargs == 1)
    {
        // This must be an array. Keys / values are pushed in valid
        // pairs to the _customHeaders array.    
        boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
        Array_as* headerArray = dynamic_cast<Array_as*>(obj.get());

        if (!headerArray)
        {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror(_("XML.addRequestHeader: single argument "
                                "is not an array"));
            );
            return as_value();
        }

        Array_as::const_iterator e = headerArray->end();
        --e;

        for (Array_as::const_iterator i = headerArray->begin(); i != e; ++i)
        {
            // Only even indices can be a key, and they must be a string.
            if (i.index() % 2) continue;
            if (!(*i).is_string()) continue;
            
            // Only the immediately following odd number can be 
            // a value, and it must also be a string.
            const as_value& val = headerArray->at(i.index() + 1);
            if (val.is_string())
            {
                array->callMethod(NSV::PROP_PUSH, *i, val);
            }
        }
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

    array->callMethod(NSV::PROP_PUSH, name, val);
    
    return as_value();
}


/// Returns true if the arguments are valid, otherwise false. The
/// success of the connection is irrelevant.
/// The second argument must be a loadable object (XML or LoadVars).
/// An optional third argument specifies the method ("GET", or by default
/// "POST"). The values are partly URL encoded if using GET.
as_value
LoadableObject::loadableobject_sendAndLoad(const fn_call& fn)
{
	boost::intrusive_ptr<LoadableObject> ptr =
	                ensureType<LoadableObject>(fn.this_ptr);

	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("sendAndLoad() requires at least two arguments"));
		);
		return as_value(false);
	}

	const std::string& urlstr = fn.arg(0).to_string();
	if ( urlstr.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("sendAndLoad(): invalid empty url"));
		);
		return as_value(false);
	}

	if (!fn.arg(1).is_object())
	{
		IF_VERBOSE_ASCODING_ERRORS(
		    log_aserror(_("sendAndLoad(): invalid target (must be an "
		    		    "XML or LoadVars object)"));
		);
		return as_value(false);
	}


	boost::intrusive_ptr<as_object> target = fn.arg(1).to_object();

	// Post by default, override by ActionScript third argument
	bool post = true;
	if ( fn.nargs > 2 && fn.arg(2).to_string() == "GET" ) post = false;

	ptr->sendAndLoad(urlstr, *target, post);
	return as_value(true);
}


as_value
LoadableObject::loadableobject_load(const fn_call& fn)
{
	boost::intrusive_ptr<LoadableObject> obj = ensureType<LoadableObject>(fn.this_ptr);

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

	obj->load(urlstr);
	return as_value(true);

}

    
as_value
LoadableObject::loadableobject_send(const fn_call& fn)
{
    boost::intrusive_ptr<LoadableObject> ptr =
        ensureType<LoadableObject>(fn.this_ptr);
 
    std::ostringstream os;
    fn.dump_args(os);
    log_debug("XML.send(%s) / LoadVars.send() TESTING", os.str());

    std::string target;
    std::string url;
    std::string method;

    switch (fn.nargs)
    {
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
    bool post = !noCaseCompare(method, "get");

    // Encode the data in the default way for the type.
    std::ostringstream data;

    ptr->send(url, target, post);
    return as_value(true);
}


}
