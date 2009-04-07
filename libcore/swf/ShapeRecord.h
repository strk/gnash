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

#ifndef GNASH_SWF_SHAPERECORD_H
#define GNASH_SWF_SHAPERECORD_H

#include "Geometry.h"
#include "fill_style.h"
#include "styles.h"
#include "rect.h"

#include <vector>


namespace gnash {
    class movie_definition;
}

namespace gnash {
namespace SWF {

/// Holds information needed to draw a shape.
//
/// This does not correspond exactly to parsed record in a SWF file, but
/// is used to create both mutable and immutable shapes.
//
/// A ShapeRecord should have enough methods to implement the AS3 Graphics
/// object (the drawing API of Shape and Sprite). This is restricted to
/// adding fills, paths and line styles (which must be constructed outside
/// this ShapeRecord before being added) and clearing everything. There
/// is no support for removing single elements.
//
/// ShapeRecord objects are not ref-counted, so they may be stack-allocated
/// or used in smart pointers.
class ShapeRecord
{
public:
    typedef std::vector<fill_style> FillStyles;
    typedef std::vector<line_style> LineStyles;
    typedef std::vector<Path> Paths;

    /// Construct a ShapeRecord.
    //
    /// Ideally all immutable ShapeRecords should be constructed with the
    /// ctor taking an SWFStream, but some tag formats do not allow this.
    ShapeRecord() {}

    /// Construct a ShapeRecord from a SWFStream.
    //
    /// This is useful for constructing immutable tags.
    ShapeRecord(SWFStream& in, SWF::TagType tag, movie_definition& m);

    /// Parse path data from a SWFStream.
    //
    /// This is used by DefineMorphShapeTag as part of parsing its
    /// more complex ShapeRecords.
    void read(SWFStream& in, SWF::TagType tag, movie_definition& m);

    const FillStyles& fillStyles() const {
        return _fillStyles;
    }
    
    const LineStyles& lineStyles() const {
        return _lineStyles;
    }

    const Paths& paths() const {
        return _paths;
    }

    const rect& getBounds() const {
        return _bounds;
    }

    /// For DynamicShape
    //
    /// TODO: rewrite DynamicShape to push paths when they're
    /// finished and drop this.
    Path& currentPath() {
        return _paths.back();
    }

    /// Reset all shape data.
    void clear() {
        _fillStyles.clear();
        _lineStyles.clear();
        _paths.clear();
        _bounds.set_null();
    }

    void addFillStyle(const fill_style& fs) {
        _fillStyles.push_back(fs);
    }

    void addPath(const Path& path) {
        _paths.push_back(path);
    }

    void addLineStyle(const line_style& ls) {
        _lineStyles.push_back(ls);
    }

    void setBounds(const rect& bounds) {
        _bounds = bounds;
    }

private:
    
    /// Shape record flags for use in parsing.
    enum ShapeRecordFlags {
        SHAPE_END = 0x00,
        SHAPE_MOVE = 0x01,
        SHAPE_FILLSTYLE0_CHANGE = 0x02,
        SHAPE_FILLSTYLE1_CHANGE = 0x04,
        SHAPE_LINESTYLE_CHANGE = 0x08,
        SHAPE_HAS_NEW_STYLES = 0x10
    };
    
    FillStyles _fillStyles;
    LineStyles _lineStyles;
    Paths _paths;
    rect _bounds;

};

} // namespace SWF
} // namespace gnash

#endif
