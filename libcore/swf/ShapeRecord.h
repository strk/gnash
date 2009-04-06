

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

    const rect& bounds() const {
        return _bounds;
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
    
    /// Shape record flags
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
