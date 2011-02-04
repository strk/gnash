// DisplayObjectContainer.h: Container of DisplayObjects.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

// Stateful live Sprite instance

#ifndef GNASH_DISPLAYOBJECTCONTAINER_H
#define GNASH_DISPLAYOBJECTCONTAINER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" 
#endif

#include "DisplayList.h" 
#include "InteractiveObject.h"

#ifdef USE_SWFTREE
# include "tree.hh"
#endif

// Forward declarations
namespace gnash {
}

namespace gnash {

class DisplayObjectContainer : public InteractiveObject 
{

public:
    
    DisplayObjectContainer(as_object* object, DisplayObject* parent)
        :
        InteractiveObject(object, parent)
    {
        assert(object);
    }

    virtual ~DisplayObjectContainer();

    /// Remove the DisplayObject at the specified depth.
    //
    /// This is the implementation of the AS3-only method
    /// DisplayObjectContainer.removeChildAt().
    //
    /// @param index    The depth from which to remove a DisplayObject.
    /// @return         The removed DisplayObject (reflects the AS return)
    DisplayObject* removeChildAt(int index);
    
    /// Remove the specified child DisplayObject.
    //
    /// TODO: should be a function of DisplayObjectContainer
    /// This is the implementation of the AS3-only method
    /// DisplayObjectContainer.removeChild(), but can also be used for
    /// AS2.
    //
    /// @param obj      The DisplayObject to remove.
    /// @return         The removed DisplayObject (reflects the AS return)
    DisplayObject* removeChild(DisplayObject* obj);
    
    /// Add a child DisplayObject at the next suitable index (AS2: depth).
    //
    /// TODO: should be a function of DisplayObjectContainer
    /// This is the implementation of the AS3-only method
    /// DisplayObjectContainer.addChild(), but can also be used for
    /// AS2.
    //
    /// @param obj      The DisplayObject to add.
    /// @return         The added DisplayObject (reflects the AS return)
    DisplayObject* addChild(DisplayObject* obj);

    /// Add a child DisplayObject at the specified index (AS2: depth).
    //
    /// TODO: should be a function of DisplayObjectContainer
    /// This is the implementation of the AS3-only method
    /// DisplayObjectContainer.addChild(), but can also be used for
    /// AS2.
    //
    /// @param obj      The DisplayObject to add.
    /// @param index    The index (depth) at which to add the DisplayObject.
    /// @return         The added DisplayObject (reflects the AS return)
    DisplayObject* addChildAt(DisplayObject* obj, int index);

    size_t numChildren() const {
        return _displayList.size();
    }

#ifdef USE_SWFTREE
    // Override to append display list info, see dox in DisplayObject.h
    virtual InfoTree::iterator getMovieInfo(InfoTree& tr,
            InfoTree::iterator it);
#endif
    

protected:

    // TODO: make private and move all DisplayList functions to this class.
    DisplayList _displayList;

};

} // namespace gnash

#endif
