// LoadableObject.h: abstraction of network-loadable AS object functions.
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

#ifndef GNASH_LOADABLE_OBJECT_H
#define GNASH_LOADABLE_OBJECT_H

#include <boost/noncopyable.hpp>
#include "as_object.h"
#include "smart_ptr.h"
#include "LoadThread.h"

namespace gnash {


/// Abstract class for loadable AS objects' interface
//
/// This is used for XML_as and LoadVars_as to abstract their identical
/// network loading functions.
///
/// It is a virtual base class because XML_as also inherits from XMLNode.
//
/// It may not be copied.
class LoadableObject : public virtual as_object, private boost::noncopyable
{
public:

    LoadableObject();

    virtual ~LoadableObject();

    /// Register methods as native for use by XML_as and LoadVars_as
    static void registerNative(as_object& global);

    /// Carry out the AS send() operation
    //
    /// @param urlstr   The URI to send the data to
    /// @param target   The target for the data (e.g. _self, _blank)
    /// @param post     Whether the data should be posted or not.
    //
    /// The success of the operation is irrelevant to AS.
    void send(const std::string& urlstr, const std::string& target,
            bool post);

    /// Carry out the AS sendAndLoad operation
    //
    /// @param urlstr   The URI to connect to for send and receive.
    ///                 This function checks for permission to load the URL.
    /// @param target   An as_object to load the data into using queueLoad,
    ///                 which only LoadableObjects should have.
    /// @param post     If true, POSTs data, otherwise GET.
    void sendAndLoad(const std::string& urlstr, as_object& target, bool post);

    /// Carry out the AS load operation
    //
    /// @param url      The URI to load from. Checks first for permission.
    void load(const std::string& url);

	size_t getBytesLoaded() const
	{
		return _bytesLoaded;
	}

	size_t getBytesTotal() const
	{
		return _bytesTotal;
	}  

    /// Overrides as_object::queueLoad to begin loading from a stream
    //
    /// @param str      The stream to load from. It is destroyed when
    ///                 we're finished with it.
    void queueLoad(std::auto_ptr<IOChannel> str);

    /// Shared AS methods for XML and LoadVars, which can be used
    /// interchangeably with each object in ActionScript.
    static as_value loadableobject_addRequestHeader(const fn_call& fn);

protected:

    /// Convert the Loadable Object to a string.
    //
    /// @param o        The ostream to write the string to.
    /// @param encode   Whether URL encoding is necessary. How this
    ///                 is done depends on the type of object.
    virtual void toString(std::ostream& o, bool encode) const = 0;

    typedef std::list<LoadThread*> LoadThreadList;

    /// Queue of load requests
    LoadThreadList _loadThreads;

    long _bytesLoaded;
    
    long _bytesTotal;

    /// The load checker interval timer used to make loads async
    unsigned int _loadCheckerTimer;

    /// Scan the LoadThread queue (_loadThreads) to see if any of
    /// them completed. If any did, invoke the onData event
    void checkLoads();

private:

    static as_value checkLoads_wrapper(const fn_call& fn);

};


}

#endif
