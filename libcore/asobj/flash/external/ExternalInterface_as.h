// ExternalInterface_as.h:  ActionScript "ExternalInterface" class, for Gnash.
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

#ifndef GNASH_ASOBJ_EXTERNALINTERFACE_H
#define GNASH_ASOBJ_EXTERNALINTERFACE_H

#include <string>
#include <vector>
#include <map>

namespace gnash {

class as_object;
class as_value;
class ObjectURI;
class Global_as;
class movie_root;
}

namespace gnash {

class ExternalInterface_as
{
public:
    ExternalInterface_as(as_object* owner);
    ~ExternalInterface_as();

#if 0
    // This is a flag that specifies whether exceptions in ActionScript
    // should be propogated to JavaScript in the browser.
    void setMarshallExceptions(bool x) { _marshallExceptions = x; };
    bool getMarshallExceptions() { return _marshallExceptions; };
    
    /// Add an ActionScript function as a callback by JavaScript
    // in the browser.
    bool addCallback(const std::string &name, as_object *method);

    ///
    bool addRootCallback(movie_root &mr);    

    virtual void update();

    // Parse the XML Invoke message.
    void processInvoke(const std::string &str);

    void setFD(int x) { _fd = x; };

    as_object *getCallback(const std::string &name);
#endif
    
private:
    int                 _fd;
    std::map<std::string, as_object *> _methods;
    bool		_marshallExceptions;
};

/// Initialize the global ExternalInterface class
void externalinterface_class_init(gnash::as_object& where,
                                  const gnash::ObjectURI& uri);

} // end of gnash namespace

// __GNASH_ASOBJ_EXTERNALINTERFACE_H__
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
