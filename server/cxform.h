// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef GNASHCXFORM_H
#define GNASHCXFORM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h" // for DSOEXPORT
#include <inttypes.h> // for uint8_t

#include <iosfwd>

namespace gnash {
	class rgba;
	class stream;
}

namespace gnash {

/// Color transform type, used by render handler
class DSOEXPORT cxform
{
public:
    /// [RGBA][multiply, add]
    float	m_[4][2];
    
    /// Initialize to the identity color transform (no transform)
    cxform();
    
    /// Concatenate c's transform onto ours. 
    //
    /// When transforming colors, c's transform is applied
    /// first, then ours.
    ///
    void concatenate(const cxform& c);
    
    /// Apply our transform to the given color; return the result.
    rgba transform(const rgba& in) const;
    
    /// Faster transform() method for loops (avoids creation of rgba object)
    void transform(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;    
    
    /// Read RGB from the SWF input stream.
    void read_rgb(stream* in);
    
    /// Read RGBA from the SWF input stream.
    void read_rgba(stream* in);
    
    /// Force component values to be in range.
    void clamp();
    
    /// Debug log.
    void print() const;
    
    /// Returns true when the cxform equals identity (no transform)
    bool is_identity() const;
    
    /// Returns true when the cxform leads to alpha == 0
    bool is_invisible() const;
    
    /// The identity color transform (no transform)
    static cxform	identity;
};




}	// namespace gnash

#endif // GNASHCXFORM_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
