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

#ifndef GNASH_DRAG_STATE_H
#define GNASH_DRAG_STATE_H

#include <boost/optional.hpp>

#include "SWFRect.h"
#include "DisplayObject.h"

// Forward declarations
namespace gnash {
    class DisplayObject;
}

namespace gnash {

/// What is being dragged and how
class DragState
{
public:

    DragState(DisplayObject* d, bool lock)
        :
        _displayObject(d),
        _lock_centered(lock),
        _xoffset(0),
        _yoffset(0)
    {
    }

    bool isLockCentered() const {
        return _lock_centered;
    }

    /// Set displacement offset from origin
    /// at time of drag start.
    /// Coordinates are in stage space (twips)
    ///
    void setOffset(std::int32_t x, std::int32_t y) {
        _xoffset = x;
        _yoffset = y;
    }

    std::int32_t xOffset() const { return _xoffset; }
    std::int32_t yOffset() const { return _yoffset; }

    bool hasBounds() const {
        return (_bounds);
    }

    /// \brief
    /// Get the boundaries to constraint
    /// the drag into.
    //
    /// Coordinates of the rectangle are
    /// expected in TWIPS.
    ///
    /// Note that if hasBounds() is false
    /// the returned rectangle is the NULL
    /// rectangle - see SWFRect::is_null().
    ///
    const SWFRect& getBounds() const { return *_bounds; }

    /// \brief
    /// Set the boundaries to constraint
    /// the drag into.
    //
    /// Coordinates of the rectangle are
    /// expected in TWIPS.
    ///
    void setBounds(const SWFRect& bounds) {
        _bounds = bounds;
    }

    /// May return NULL !!
    DisplayObject* getCharacter() const {
        return _displayObject;
    }

    /// Reset drag state to its initial condition
    void reset() {
        _displayObject = nullptr;
        _bounds.reset();
        _lock_centered = false;
    }

    /// Mark DisplayObject as reachable (if any)
    void markReachableResources() const {
        if (_displayObject) _displayObject->setReachable();
    }

private:

    /// Boundaries to constrain the drag into.
    /// Coordinates in TWIPS.
    boost::optional<SWFRect> _bounds;

    DisplayObject* _displayObject;

    bool _lock_centered;

    /// Offsets of displacement from DisplayObject origin
    /// at time of drag start. These are used for non
    /// lock-centered dragging.
    /// Coordinates are in stage space (TWIPS)
    std::int32_t _xoffset;
    std::int32_t _yoffset;

};


} // namespace gnash

#endif // GNASH_DRAG_STATE_H
