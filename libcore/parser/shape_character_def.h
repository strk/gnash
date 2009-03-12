// shape.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Quadratic bezier outline shapes, the basis for most SWF rendering.


#ifndef GNASH_SHAPE_CHARACTER_DEF_H
#define GNASH_SHAPE_CHARACTER_DEF_H

#include "character_def.h" // for inheritance of shape_character_def
#include "smart_ptr.h" // GNASH_USE_GC
#include "Geometry.h"     // for path
#include "rect.h"      // for composition
#include "fill_style.h" // for fill style
#include "styles.h"     // for line style
#include "swf.h"

#include <vector> // for composition


namespace gnash {
	class SWFStream;
	class cxform;
	class SWFMatrix;
}

namespace gnash {

/// \brief
/// Represents the outline of one or more shapes, along with
/// information on fill and line styles.
//
class shape_character_def : public character_def
{
public:

    typedef std::vector<fill_style> FillStyles;
    typedef std::vector<line_style> LineStyles;
    typedef std::vector<path> Paths;

    shape_character_def();
    virtual ~shape_character_def();

    virtual void display(character* inst);

    /// Return true if the specified point is on the interior of our shape.
    //
    /// Incoming coords are local coords (twips).
    /// The SWFMatrix will be used for lines with non-scalable strokes.
    ///
    virtual bool point_test_local(boost::int32_t x, boost::int32_t y,
            const SWFMatrix& wm);

	virtual character* createDisplayObject(character* parent, int id);
	
    /// \brief
    /// Read a shape definition as included in DEFINEFONT*,
    /// DEFINESHAPE* or DEFINEMORPH* tag
    //
    /// @param in
    ///	The stream to read the shape from
    ///
    /// @param TagType
    ///	The SWF::TagType this shape definition is read for.
    ///	TODO: change to an actual SWF::TagType type
    ///
    /// @param with_style
    ///	If true, this definition includes bounds, fill styles and line styles.
    ///	Tipically, this is only false for DEFINEFONT* tags.
    ///	NOTE: if with_style is false, bounds of the shape will be computed
    ///	      rather then read.
    ///	TODO: drop this function, set based on TagType ?
    ///
    /// @param m
    ///	The movie definition corresponding to the SWF we/re parsing.
    ///	This is used to resolve bitmap characters for fill styles, never
    ///	used if with_style is false.
    void read(SWFStream& in, SWF::TagType tag, bool with_style,
            movie_definition& m);

    /// Get cached bounds of this shape.
    const rect&	get_bound() const { return _bound; }

    /// Compute bounds by looking at the component paths
    void compute_bound(rect& r, int swfVersion) const;

    const FillStyles& fillStyles() const { return _fill_styles; }
    const LineStyles& lineStyles() const { return _line_styles; }

    const Paths& paths() const { return _paths; }

    // morph uses this
    // Should this be verified?
    void set_bound(const rect& r) { _bound = r; }

    // Morph uses this.
    void addFillStyle(const fill_style& fs) {
        _fill_styles.push_back(fs);
    }

    void addLineStyle(const line_style& fs) {
        _line_styles.push_back(fs);
    }

protected:

#ifdef GNASH_USE_GC
    /// Mark reachable resources (for the GC)
    //
    /// Reachable resources are:
    ///	- Associated fill styles (_fill_styles).
    ///	  These are not actual resources, but may contain some.
    ///
    virtual void markReachableResources() const;
#endif // GNASH_USE_GC

    // derived morph classes changes these
    FillStyles _fill_styles;
    LineStyles _line_styles;
    Paths _paths;
    rect _bound;

    /// Copy a shape character definition
    shape_character_def(const shape_character_def& o);

private:

    /// Shape record flags
    enum ShapeRecordFlags {
        flagEnd = 0x00,
        flagMove = 0x01,
        flagFillStyle0Change = 0x02,
        flagFillStyle1Change = 0x04,
        flagLineStyleChange = 0x08,
        flagHasNewStyles = 0x10
    };
    
    // Don't assign to a shape character definition
    shape_character_def& operator= (const shape_character_def&);

};

}	// end namespace gnash


#endif // GNASH_SHAPE_CHARACTER_DEF_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
