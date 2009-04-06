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

#ifndef GNASH_MORPH_SHAPE_H
#define GNASH_MORPH_SHAPE_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "DisplayObject.h"
#include "morph2_character_def.h"
#include <cassert>

namespace gnash {

/// A DisplayObject that tweens between two shapes.
//
/// A MorphShape has no properties of its own, but its inherited properties
/// may be read in AS3 using a reference to the object created with
/// getChildAt().
class MorphShape : public DisplayObject
{

public:
    
    typedef std::vector<fill_style> FillStyles;
    typedef std::vector<line_style> LineStyles;
    typedef std::vector<path> Paths;

    MorphShape(morph_character_def* def, DisplayObject* parent, int id);

	virtual void display();

    virtual rect getBounds() const {
        return _bounds;
    }
    
    virtual bool pointInShape(boost::int32_t  x, boost::int32_t  y) const;

    /// Advances the morphing process.
    virtual void advance();
    
    const FillStyles& fillStyles() const {
        return _fillStyles;
    }
    
    const LineStyles& lineStyles() const {
        return _lineStyles;
    }

    const Paths& paths() const {
        return _paths;
    }

protected:

#ifdef GNASH_USE_GC
	/// Mark reachable resources (for the GC)
	void markReachableResources() const
	{
		assert(isReachable());
        _def->setReachable();
		markDisplayObjectReachable();
	}
#endif

private:
    
    const boost::intrusive_ptr<morph_character_def> _def;
	
    FillStyles _fillStyles;
    LineStyles _lineStyles;
    Paths _paths;
    rect _bounds;

};


} // namespace gnash


#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
